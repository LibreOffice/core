/*************************************************************************
 *
 *  $RCSfile: md5.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-14 08:39:07 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "md5.hxx"

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

            for ( int i = 0; i < sizeof(checksum); i++ )
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
