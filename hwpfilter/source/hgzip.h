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

#pragma once


#include <zlib.h>

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
 * Crc32 of uncompressed data
 */
    uLong    crc;
/**
 * Stream
 */
    char     *msg;
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
size_t     gz_read     ( gz_stream *file, voidp  buf, unsigned len );
/**
 * Flushes all pending output into the compressed file
 * gz_flush should be called only when strictly necessary because it can
 * degrade compression
 * @param flush Is as in the deflate() function
 */
int     gz_flush    ( gz_stream *file, int flush );


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
