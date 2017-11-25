/**
 *	ratched - TLS connection router that performs a man-in-the-middle attack
 *	Copyright (C) 2017-2017 Johannes Bauer
 *
 *	This file is part of ratched.
 *
 *	ratched is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; this program is ONLY licensed under
 *	version 3 of the License, later versions are explicitly excluded.
 *
 *	ratched is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with ratched; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Johannes Bauer <JohannesBauer@gmx.de>
**/

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "logging.h"
#include "tools.h"

bool pathtok(const char *path, bool (*callback)(const char *path, void *arg), void *arg) {
	char *strcopy = strdup(path);
	if (!strcopy) {
		logmsg(LLVL_ERROR, "Unable to strdup(3) path: %s", strerror(errno));
		return false;
	}

	bool success = true;
	char *origstring = strcopy;
	char *string = strcopy;
	char *saveptr = NULL;
	char *next;
	while ((next = strtok_r(string, "/", &saveptr)) != NULL) {
		if (!string) {
			/* This is not the first run, restore last '/' */
			next[-1] = '/';
		}
		if (!callback(origstring, arg)) {
			success = false;
			break;
		}
		string = NULL;
	}

	free(origstring);
	return success;
}

static bool mkdir_callback(const char *path, void *arg) {
	if (mkdir(path, 0700) == -1) {
		/* Ignore error if already exists */
		if (errno == EEXIST) {
			return true;
		}

		/* Otherwise, abort. */
		logmsg(LLVL_ERROR, "mkdir(2) of %s failed: %s", path, strerror(errno));
		return false;
	}
	return true;
}

bool makedirs(const char *path) {
	return pathtok(path, mkdir_callback, NULL);
}

bool strxcat(char *dest, int bufsize, ...) {
	if (bufsize < 1) {
		return false;
	}

	/* Reserve at least one byte for zero-termination */
	bufsize--;

	bool success = true;
	va_list ap;
	va_start(ap, bufsize);
	const char *next_string;
	while ((next_string = va_arg(ap, const char*)) != NULL) {
		for (int i = 0; next_string[i]; i++) {
			if (bufsize == 0) {
				success = false;
				break;
			}
			*dest = next_string[i];
			dest++;
			bufsize--;
		}
	}
	*dest = 0;
	va_end(ap);
	return success;
}
