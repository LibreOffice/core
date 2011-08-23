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

/* $Id: hgzip.h,v 1.5 2008-06-04 09:57:59 vg Exp $ */

#ifndef _HWPGZIP_H_
#define _HWPGZIP_H_

// DVO: add zlib/ prefix
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include "zlib/zlib.h"
#endif

class HStream;
/**
 * @short Structure for using z_stream
 */
struct gz_stream
{
    z_stream stream;
/**
 * The error code of z_stream operation
 */
    int      z_err;
/**
 * EOF of the input file
 */
    int      z_eof;
/**
 * Stream
 */
    HStream*     _inputstream;
/**
 * Input buffer
 */
    Byte     *inbuf;
/**
 * Output buffer
 */
    Byte     *outbuf;
/**
 * Crc32 of uncompressed data
 */
    uLong    crc;
/**
 * Stream
 */
    char     *msg;
/**
 * It becomes one when the input file type is not gz file
 */
    int      transparent;
/**
 * 'w' or 'r'
 */
    char     mode;
};

/**
 *  Opens a gzipped stream for reading.
 *    gz_open returns NULL if the stream could not be opened or if there was
 *  insufficient memory to allocate the (de)compression state; errno
 *  can be checked to distinguish the two cases (if errno is zero, the
 *  zlib error is Z_MEM_ERROR).
 * @param _stream Reference of stream object having binary data.
 */
gz_stream   *gz_open    ( HStream& _stream );
/**
 * Flushes all pending output if necessary, closes the compressed stream
 * and deallocates all the (de)compression state
 */
int     gz_close    ( gz_stream *file );
/**
 * Reads the given number of uncompressed bytes from the compressed stream
 * @param file Gzipped stream
 * @param buf Buffer to have the data to be read
 * @param len Length of data to be read
 * @returns The number of bytes actually read
 */
int     gz_read     ( gz_stream *file, voidp  buf, unsigned len );
/**
 * Flushes all pending output into the compressed file
 * gz_flush should be called only when strictly necessary because it can
 * degrade compression
 * @param flush Is as in the deflate() function
 */
int     gz_flush    ( gz_stream *file, int flush );
#endif                                            /* _HWPGZIP_H_ */
