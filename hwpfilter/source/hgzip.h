/*************************************************************************
 *
 *  $RCSfile: hgzip.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:35:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hgzip.h,v 1.1 2003-10-15 14:35:46 dvo Exp $ */

#ifndef _HWPGZIP_H_
#define _HWPGZIP_H_

// DVO: add zlib/ prefix
#include "zlib/zlib.h"

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
 * Writes the given number of uncompressed bytes into the compressed file
 * This exists for future using.
 * @returns The number of bytes actually written
 */
int     gz_write    ( gz_stream *file, const voidp buf, unsigned len );
/**
 * Flushes all pending output into the compressed file
 * gz_flush should be called only when strictly necessary because it can
 * degrade compression
 * @param flush Is as in the deflate() function
 */
int     gz_flush    ( gz_stream *file, int flush );
/**
 * Not implemented.
 */
int     gz_seek     ( gz_stream *file, long offset, int whence );
/**
 * Not implemented.
 */
long        gz_tell     ( gz_stream *file );
#endif                                            /* _HWPGZIP_H_ */
