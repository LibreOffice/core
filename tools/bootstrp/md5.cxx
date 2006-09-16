/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: md5.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:48:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "md5.hxx"

#include <cstddef>
#include <stdio.h>

#include <string.hxx>

#ifdef WNT
#define FILE_OPEN_READ  "rb"
#else
#define FILE_OPEN_READ  "r"
#endif

rtlDigestError calc_md5_checksum( const char *filename, ByteString &aChecksum )
{
    sal_uInt8 checksum[RTL_DIGEST_LENGTH_MD5];
    rtlDigestError  error = rtl_Digest_E_None;

    FILE *fp = fopen( filename, FILE_OPEN_READ );

    if ( fp )
    {
        rtlDigest digest = rtl_digest_createMD5();

        if ( digest )
        {
            size_t          nBytesRead;
            sal_uInt8       buffer[0x1000];

            while ( rtl_Digest_E_None == error &&
                0 != (nBytesRead = fread( buffer, 1, sizeof(buffer), fp )) )
            {
                error = rtl_digest_updateMD5( digest, buffer, nBytesRead );
            }

            if ( rtl_Digest_E_None == error )
            {
                error = rtl_digest_getMD5( digest, checksum, sizeof(checksum) );
            }

            rtl_digest_destroyMD5( digest );

            for ( std::size_t i = 0; i < sizeof(checksum); i++ )
            {
                if ( checksum[i] < 16 )
                    aChecksum.Append( "0" );
                aChecksum += ByteString::CreateFromInt32( checksum[i], 16 );
            }
        }

        fclose( fp );
    }
    else
        error = rtl_Digest_E_Unknown;

    return error;
}
