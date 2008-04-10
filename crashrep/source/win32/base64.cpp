/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: base64.cpp,v $
 * $Revision: 1.4 $
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

#include <stdio.h>
#include <string.h>
#include "base64.h"

static const char base64_tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

extern "C" size_t base64_encode( FILE *fin, FILE *fout )
{
    size_t nBytesRead = 0;
    size_t nLineLength = 0;
    size_t nBytesWritten = 0;

    size_t nBytes = 0;

    do
    {
        unsigned char in_buffer[3];

        memset( in_buffer, 0, sizeof(in_buffer) );
        nBytes = fread( in_buffer, 1, sizeof(in_buffer), fin );
        nBytesRead += nBytes;

        if ( nBytes )
        {
            unsigned long value =
                ((unsigned long)in_buffer[0]) << 16 |
                ((unsigned long)in_buffer[1]) << 8 |
                ((unsigned long)in_buffer[2]) << 0;

            unsigned char out_buffer[4];

            memset( out_buffer, '=', sizeof(out_buffer) );

            out_buffer[0] = base64_tab[(value >> 18) & 0x3F];
            out_buffer[1] = base64_tab[(value >> 12) & 0x3F];

            if ( nBytes > 1 )
            {
                out_buffer[2] = base64_tab[(value >> 6) & 0x3F];
                if ( nBytes > 2 )
                    out_buffer[3] = base64_tab[(value >> 0) & 0x3F];
            }

            if ( nLineLength >= 76 )
            {
                fputs( "\n", fout );
                nLineLength = 0;
            }

            nBytesWritten += fwrite( out_buffer, 1, sizeof(out_buffer), fout );
            nLineLength += sizeof(out_buffer);
        }
    } while ( nBytes );

    return nBytesWritten;
}
