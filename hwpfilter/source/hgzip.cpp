/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "hgzip.h"
#include "hstream.h"

#ifndef local
#  define local static
#endif

#define Z_BUFSIZE   (1024 * 4)

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

#define ASCII_FLAG   0x01                         /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02                         /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04                         /* bit 2 set: extra field present */
#define ORIG_NAME    0x08                         /* bit 3 set: original file name present */
#define COMMENT      0x10                         /* bit 4 set: file comment present */
#define RESERVED     0xE0                         /* bits 5..7: reserved */

local int get_byte(gz_stream * s);
local int destroy(gz_stream * s);
local uLong getLong(gz_stream * s);

/* ===========================================================================
   Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb"). The file is given either by file descriptor
   or path name (if fd == -1).
     gz_open return NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).
*/
gz_stream *gz_open(HStream & _stream)
{
    int err;
    //int level = Z_DEFAULT_COMPRESSION;            /* compression level */

//  char        *p = (char*)mode;
    //char fmode[80];                               /* copy of mode, without the compression level */
    //char *m = fmode;
    gz_stream *s;

    s = (gz_stream *) ALLOC(sizeof(gz_stream));
    if (!s)
        return Z_NULL;
    s->stream.zalloc = (alloc_func) 0;
    s->stream.zfree = (free_func) 0;
    s->stream.opaque = (voidpf) 0;
    s->stream.next_in = s->inbuf = Z_NULL;
    s->stream.next_out = s->outbuf = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
//s->_inputstream = NULL;
    s->z_err = Z_OK;
    s->z_eof = 0;
    s->crc = crc32(0L, Z_NULL, 0);
    s->msg = NULL;
    s->transparent = 0;

    s->mode = 'r';

//realking
    err = inflateInit2(&(s->stream), -MAX_WBITS);
    s->stream.next_in = s->inbuf = (Byte *) ALLOC(Z_BUFSIZE);

    if (err != Z_OK || s->inbuf == Z_NULL)
    {
        return destroy(s), (gz_stream *) Z_NULL;
    }

    s->stream.avail_out = Z_BUFSIZE;

    errno = 0;
    s->_inputstream = &_stream;

    return (gz_stream *) s;
}


/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/
local int get_byte(gz_stream * s)
{
    if (s->z_eof)
        return EOF;
    if (s->stream.avail_in == 0)
    {
        errno = 0;

        s->stream.avail_in = s->_inputstream->readBytes(s->inbuf, Z_BUFSIZE);
        if (s->stream.avail_in == 0)
        {
            s->z_eof = 1;
            return EOF;
        }
        s->stream.next_in = s->inbuf;
    }
    s->stream.avail_in--;
    return *(s->stream.next_in)++;
}


/* ===========================================================================
 * Cleanup then free the given gz_stream. Return a zlib error code.
 * Try freeing in the reverse order of allocations.
 */
local int destroy(gz_stream * s)
{
    int err = Z_OK;

    if (!s)
        return Z_STREAM_ERROR;

    TRYFREE(s->msg);

    if (s->stream.state != NULL)
    {
        err = inflateEnd(&(s->stream));
    }
    if (s->z_err < 0)
        err = s->z_err;

    TRYFREE(s->inbuf);
    TRYFREE(s->outbuf);
    TRYFREE(s);
    return err;
}


// typedef unsigned char  Byte
// typedef Byte  FAR Bytef;
/* ===========================================================================
   Reads the given number of uncompressed bytes from the compressed file.
   gz_read returns the number of bytes actually read (0 for end of file).
*/
int gz_read(gz_stream * file, voidp buf, unsigned len)
{
//printf("@@ gz_read : len : %d\t",len);
    gz_stream *s = (gz_stream *) file;
    Bytef *start = (Bytef *) buf;                 /* starting point for crc computation */
    Byte *next_out;                               /* == stream.next_out but not forced far (for MSDOS) */
    if (s == NULL)
        return Z_STREAM_ERROR;

    if (s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO)
        return -1;
    if (s->z_err == Z_STREAM_END)
        return 0;                                 /* EOF */

    s->stream.next_out = next_out = (Bytef *) buf;
    s->stream.avail_out = len;

    while (s->stream.avail_out != 0)
    {
        if (s->transparent)
        {
/* Copy first the lookahead bytes: */
            uInt n = s->stream.avail_in;

            if (n > s->stream.avail_out)
                n = s->stream.avail_out;
            if (n > 0)
            {
                memcpy(s->stream.next_out, s->stream.next_in, n);
                next_out += n;
                s->stream.next_out = next_out;
                s->stream.next_in += n;
                s->stream.avail_out -= n;
                s->stream.avail_in -= n;
            }
            if (s->stream.avail_out > 0)
            {
                s->stream.avail_out -=
                    s->_inputstream->readBytes(next_out, s->stream.avail_out);
            }
            return (int) (len - s->stream.avail_out);
        }
        if (s->stream.avail_in == 0 && !s->z_eof)
        {

            errno = 0;
            s->stream.avail_in = s->_inputstream->readBytes(s->inbuf, Z_BUFSIZE);
            if (s->stream.avail_in == 0)
            {
                s->z_eof = 1;
                break;
            }
            s->stream.next_in = s->inbuf;
        }
        s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

        if (s->z_err == Z_STREAM_END)
        {
/* Check CRC and original size */
            s->crc = crc32(s->crc, start, (uInt) (s->stream.next_out - start));
            start = s->stream.next_out;

            if (getLong(s) != s->crc || getLong(s) != s->stream.total_out)
            {
                s->z_err = Z_DATA_ERROR;
            }
            else if (s->z_err == Z_OK)
            {
                inflateReset(&(s->stream));
                s->crc = crc32(0L, Z_NULL, 0);
            }
        }
        if (s->z_err != Z_OK || s->z_eof)
            break;
    }
    s->crc = crc32(s->crc, start, (uInt) (s->stream.next_out - start));
    return (int) (len - s->stream.avail_out);
}

/* ===========================================================================
     Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function.
     gz_flush should be called only when strictly necessary because it can
   degrade compression.
*/
int gz_flush(gz_stream * file, int flush)
{
    uInt len;
    int done = 0;
    gz_stream *s = (gz_stream *) file;

    if (s == NULL || s->mode != 'w')
        return Z_STREAM_ERROR;

    s->stream.avail_in = 0;                       /* should be zero already anyway */

    for (;;)
    {
        len = Z_BUFSIZE - s->stream.avail_out;
        if (len != 0)
        {
/*
      if ((uInt)fwrite(s->outbuf, 1, len, s->file) != len) {
    s->z_err = Z_ERRNO;
    return Z_ERRNO;
      }
      */
            s->stream.next_out = s->outbuf;
            s->stream.avail_out = Z_BUFSIZE;
        }
        if (done)
            break;
        s->z_err = deflate(&(s->stream), flush);

/* deflate has finished flushing only when it hasn't used up
 * all the available space in the output buffer:
 */
        done = (s->stream.avail_out != 0 || s->z_err == Z_STREAM_END);

        if (s->z_err != Z_OK && s->z_err != Z_STREAM_END)
            break;
    }
    return s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local uLong getLong(gz_stream * s)
{
    uLong x = (uLong) get_byte(s);
    int c;

    x += ((uLong) get_byte(s)) << 8;
    x += ((uLong) get_byte(s)) << 16;
    c = get_byte(s);
    if (c == EOF)
        s->z_err = Z_DATA_ERROR;
    x += ((uLong) c) << 24;
    return x;
}


/* ===========================================================================
   Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state.
*/
int gz_close(gz_stream * file)
{
//  int err;
    gz_stream *s = (gz_stream *) file;

    if (s == NULL)
        return Z_STREAM_ERROR;
#if 0
    if (s->mode == 'w')
    {
        err = gz_flush(file, Z_FINISH);
        if (err != Z_OK)
            return destroy(s);
        putLong(s->file, s->crc);
        putLong(s->file, s->stream.total_in);
    }
#endif
    return destroy(s);
}
