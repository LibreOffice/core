/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "md5.hxx"

#include <cstddef>
#include <stdio.h>

#include <tools/string.hxx>

#ifdef WNT
#define FILE_OPEN_READ  "rb"
#else
#define FILE_OPEN_READ  "r"
#endif

// Extended calc_md5_checksum to recognize Windows executables and libraries. To
// create the same md5 checksum for a (code/data) identical file it ignores a different
// date and header checksum. Please see crashrep/source/win32/soreport.cpp
// where the same method is also used. The crash reporter uses the MD5
// checksums to transfer them to the crash database. You have to make sure that both
// methods use the same algorithm otherwise there could be problems with stack reports.

void normalize_pe_image(sal_uInt8* buffer, size_t nBufferSize)
{
    const int OFFSET_PE_OFFSET = 0x3c;
    const int OFFSET_COFF_TIMEDATESTAMP = 4;
    const int PE_SIGNATURE_SIZE = 4;
    const int COFFHEADER_SIZE = 20;
    const int OFFSET_PE_OPTIONALHEADER_CHECKSUM = 64;

    // Check the header part of the file buffer
    if (buffer[0] == sal_uInt8('M') && buffer[1] == sal_uInt8('Z'))
    {
        unsigned long PEHeaderOffset = (long)buffer[OFFSET_PE_OFFSET];
        if (PEHeaderOffset < nBufferSize-4)
        {
            if ( buffer[PEHeaderOffset+0] == sal_uInt8('P') &&
                 buffer[PEHeaderOffset+1] == sal_uInt8('E') &&
                 buffer[PEHeaderOffset+2] == 0 &&
                 buffer[PEHeaderOffset+3] == 0 )
            {
                PEHeaderOffset += PE_SIGNATURE_SIZE;
                if (PEHeaderOffset+OFFSET_COFF_TIMEDATESTAMP < nBufferSize-4)
                {
                    // Set timedatestamp and checksum fields to a normalized
                    // value to enforce the same MD5 checksum for identical
                    // Windows  executables/libraries.
                    buffer[PEHeaderOffset+OFFSET_COFF_TIMEDATESTAMP+0] = 0;
                    buffer[PEHeaderOffset+OFFSET_COFF_TIMEDATESTAMP+1] = 0;
                    buffer[PEHeaderOffset+OFFSET_COFF_TIMEDATESTAMP+2] = 0;
                    buffer[PEHeaderOffset+OFFSET_COFF_TIMEDATESTAMP+3] = 0;
                }

                if (PEHeaderOffset+COFFHEADER_SIZE+OFFSET_PE_OPTIONALHEADER_CHECKSUM < nBufferSize-4)
                {
                    // Set checksum to a normalized value
                    buffer[PEHeaderOffset+COFFHEADER_SIZE+OFFSET_PE_OPTIONALHEADER_CHECKSUM] = 0;
                    buffer[PEHeaderOffset+COFFHEADER_SIZE+OFFSET_PE_OPTIONALHEADER_CHECKSUM+1] = 0;
                    buffer[PEHeaderOffset+COFFHEADER_SIZE+OFFSET_PE_OPTIONALHEADER_CHECKSUM+2] = 0;
                    buffer[PEHeaderOffset+COFFHEADER_SIZE+OFFSET_PE_OPTIONALHEADER_CHECKSUM+3] = 0;
                }
            }
        }
    }
}

rtlDigestError calc_md5_checksum( const char *filename, ByteString &aChecksum )
{
    const size_t BUFFER_SIZE  = 0x1000;
    const size_t MINIMAL_SIZE = 512;

    sal_uInt8 checksum[RTL_DIGEST_LENGTH_MD5];
    rtlDigestError  error = rtl_Digest_E_None;

    FILE *fp = fopen( filename, FILE_OPEN_READ );

    if ( fp )
    {
        rtlDigest digest = rtl_digest_createMD5();

        if ( digest )
        {
            size_t          nBytesRead;
            sal_uInt8       buffer[BUFFER_SIZE];
            bool            bHeader(true);

            while ( rtl_Digest_E_None == error &&
                0 != (nBytesRead = fread( buffer, 1, sizeof(buffer), fp )) )
            {
                if (bHeader)
                {
                    bHeader = false;
                    if (nBytesRead >= MINIMAL_SIZE && buffer[0] == sal_uInt8('M') && buffer[1] == sal_uInt8('Z') )
                        normalize_pe_image(buffer, nBytesRead);
                }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
