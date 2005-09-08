/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: base64.cpp,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:43:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
