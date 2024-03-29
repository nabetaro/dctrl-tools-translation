/*  dctrl-tools - Debian control file inspection tools
    Copyright © 2003, 2005 Antti-Juhani Kaijanaho

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FSAF_H
#define FSAF_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* FAST (MOSTLY) SEQUENTIAL-ACCESS FILE LAYER */

struct fsaf_private {
	char *fname;
	int fd;
	char * buf;
	size_t buf_capacity;
	size_t buf_offset;
	size_t buf_size;
	size_t invalid_mark;
	size_t eof_mark; /* can be (size_t)(-1) if not reached yet */
#ifdef _POSIX_MAPPED_FILES
	bool mapped;
	size_t topread; /* marks the top of what has been read ahead */
#endif
};

typedef struct fsaf_private FSAF;

/* True if FSAF should not use mmap even if it is possible. */
extern bool fsaf_mmap;

/* Open a FSAF for the given fd. Only read access is supported for
 * now.  The whole file is initially valid.  */
FSAF * fsaf_fdopen(int fd, char const *fname);

/* Close the given FSAF.  This DOES NOT close the underlying fd.  */
void fsaf_close(FSAF *);

/* Return a pointer to a buffer containing len bytes from the FSAF
 * starting at offset.  Note that if offset is below the invalid mark,
 * we return b = NULL.  This is a CHEAP operation, if the full range
 * has already been accessed.  The buffer may be invalidated by any
 * subsequent call to this function. */
struct fsaf_read_rv {
	char const * b;
	size_t len;
} fsaf_read(FSAF *, size_t offset, size_t len);

/* Behaves like fsaf_read except that the result is put in a malloc'd
 * zero-terminated buffer.  NULL return value indicates either memory
 * allocation failure or that the read was below the invalid mark.  */
static inline
char * fsaf_getas(FSAF * fp, size_t offset, size_t len)
{
	struct fsaf_read_rv r = fsaf_read(fp, offset, len);
	if (r.b == 0) return 0;
	char * rv = malloc(r.len+1);
	if (rv == 0) return 0;
	memcpy(rv, r.b, r.len);
	rv[r.len] = 0;
	return rv;
}

static inline
int fsaf_getc(FSAF * fp, size_t offset)
{
	if (offset >= fp->eof_mark) return -1;
	struct fsaf_read_rv r = fsaf_read(fp, offset, 1);
	if (offset >= fp->eof_mark) return -1;
	assert(r.len == 1);
	return (unsigned char)r.b[0];
}


/* Invalidate all bytes in FSAF up to and excluding offset.  */
void fsaf_invalidate(FSAF *, size_t offset);

/* Return an upper bound for the end of file, either the smallest
 * offset beyond eof or (size_t)(-1). */
static inline
size_t fsaf_eof(FSAF * fp) { return fp->eof_mark; }


#endif /* FSAF_H */
