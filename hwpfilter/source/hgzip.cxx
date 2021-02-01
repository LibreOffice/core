/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "precompile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "hgzip.h"
#include "hstream.hxx"

#define Z_BUFSIZE   (1024 * 4)

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

static int get_byte(gz_stream * s);
static int destroy(gz_stream * s);
static uLong getLong(gz_stream * s);

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

    s = static_cast<gz_stream *>(ALLOC(sizeof(gz_stream)));
    if (!s)
        return Z_NULL;
    s->stream.zalloc = nullptr;
    s->stream.zfree = nullptr;
    s->stream.opaque = nullptr;
    s->stream.next_in = s->inbuf = Z_NULL;
    s->stream.next_out = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
//s->_inputstream = NULL;
    s->z_err = Z_OK;
    s->z_eof = 0;
    s->crc = crc32(0, Z_NULL, 0);
    s->msg = nullptr;

    s->mode = 'r';

//realking
    err = inflateInit2(&(s->stream), -MAX_WBITS);
    s->stream.next_in = s->inbuf = static_cast<Byte *>(ALLOC(Z_BUFSIZE));

    if (err != Z_OK || s->inbuf == Z_NULL)
    {
        destroy(s);
        return Z_NULL;
    }

    s->stream.avail_out = Z_BUFSIZE;

    errno = 0;
    s->_inputstream = &_stream;

    return s;
}


/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been successfully opened for reading.
*/
static int get_byte(gz_stream * s)
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
static int destroy(gz_stream * s)
{
    int err = Z_OK;

    if (!s)
        return Z_STREAM_ERROR;

    TRYFREE(s->msg);

    if (s->stream.state != nullptr)
    {
        err = inflateEnd(&(s->stream));
    }
    if (s->z_err < 0)
        err = s->z_err;

    TRYFREE(s->inbuf);
    TRYFREE(s);
    return err;
}


// typedef unsigned char  Byte
// typedef Byte Bytef;
/* ===========================================================================
   Reads the given number of uncompressed bytes from the compressed file.
   gz_read returns the number of bytes actually read (0 for end of file).
*/
size_t gz_read(gz_stream * file, voidp buf, unsigned len)
{
//printf("@@ gz_read : len : %d\t",len);
    gz_stream *s = file;
    Bytef *start = static_cast<Bytef *>(buf);                 /* starting point for crc computation */
    if (s == nullptr)
        return 0;

    if (s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO)
        return 0;
    if (s->z_err == Z_STREAM_END)
        return 0;                                 /* EOF */

    s->stream.next_out = static_cast<Bytef *>(buf);
    s->stream.avail_out = len;

    while (s->stream.avail_out != 0)
    {
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
            s->crc = crc32(s->crc, start, static_cast<uInt>(s->stream.next_out - start));
            start = s->stream.next_out;

            if (getLong(s) != s->crc || getLong(s) != s->stream.total_out)
            {
                s->z_err = Z_DATA_ERROR;
            }
            else if (s->z_err == Z_OK)
            {
                inflateReset(&(s->stream));
                s->crc = crc32(0, Z_NULL, 0);
            }
        }
        if (s->z_err != Z_OK || s->z_eof)
            break;
    }
    s->crc = crc32(s->crc, start, static_cast<uInt>(s->stream.next_out - start));
    return len - s->stream.avail_out;
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
    bool done = false;
    gz_stream *s = file;

    if (s == nullptr || s->mode != 'w')
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
            s->stream.next_out = nullptr;
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
static uLong getLong(gz_stream * s)
{
    uLong x = static_cast<unsigned char>(get_byte(s));

    x += static_cast<unsigned char>(get_byte(s)) << 8;
    x += static_cast<unsigned char>(get_byte(s)) << 16;
    x += static_cast<unsigned char>(get_byte(s)) << 24;
    if (s->z_eof)
    {
        s->z_err = Z_DATA_ERROR;
    }
    return x;
}


/* ===========================================================================
   Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state.
*/
int gz_close(gz_stream * file)
{
//  int err;
    gz_stream *s = file;

    if (s == nullptr)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
