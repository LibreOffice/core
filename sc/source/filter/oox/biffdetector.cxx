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

#include "biffdetector.hxx"

#include <algorithm>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <rtl/strbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/ole/olestorage.hxx"

namespace oox {
namespace xls {
namespace BiffDetector {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::comphelper::MediaDescriptor;
using ::rtl::OStringBuffer;
using ::rtl::OUString;

BiffType detectStreamBiffVersion( BinaryInputStream& rInStream )
{
    BiffType eBiff = BIFF_UNKNOWN;
    if( !rInStream.isEof() && rInStream.isSeekable() && (rInStream.size() > 4) )
    {
        sal_Int64 nOldPos = rInStream.tell();
        rInStream.seekToStart();
        sal_uInt16 nBofId, nBofSize;
        rInStream >> nBofId >> nBofSize;

        if( (4 <= nBofSize) && (nBofSize <= 16) && (rInStream.tell() + nBofSize <= rInStream.size()) )
        {
            switch( nBofId )
            {
                case BIFF2_ID_BOF:
                    eBiff = BIFF2;
                break;
                case BIFF3_ID_BOF:
                    eBiff = BIFF3;
                break;
                case BIFF4_ID_BOF:
                    eBiff = BIFF4;
                break;
                case BIFF5_ID_BOF:
                {
                    if( 6 <= nBofSize )
                    {
                        sal_uInt16 nVersion;
                        rInStream >> nVersion;
                        // #i23425# #i44031# #i62752# there are some *really* broken documents out there...
                        switch( nVersion & 0xFF00 )
                        {
                            case 0:                 eBiff = BIFF5;  break;  // #i44031# #i62752#
                            case BIFF_BOF_BIFF2:    eBiff = BIFF2;  break;
                            case BIFF_BOF_BIFF3:    eBiff = BIFF3;  break;
                            case BIFF_BOF_BIFF4:    eBiff = BIFF4;  break;
                            case BIFF_BOF_BIFF5:    eBiff = BIFF5;  break;
                            case BIFF_BOF_BIFF8:    eBiff = BIFF8;  break;
                            default:    OSL_FAIL( OStringBuffer( "lclDetectStreamBiffVersion - unknown BIFF version: 0x" ).
                                append( static_cast< sal_Int32 >( nVersion ), 16 ).getStr() );
                        }
                    }
                }
                break;
                // else do nothing, no BIFF stream
            }
        }
        rInStream.seek( nOldPos );
    }
    return eBiff;
}

BiffType detectStorageBiffVersion( OUString& orWorkbookStreamName, const StorageRef& rxStorage )
{

    BiffType eBiff = BIFF_UNKNOWN;
    if( rxStorage.get() )
    {
        if( rxStorage->isStorage() )
        {
            // try to open the "Book" stream
            const OUString saBookName = CREATE_OUSTRING( "Book" );
            BinaryXInputStream aBookStrm5( rxStorage->openInputStream( saBookName ), true );
            BiffType eBookStrm5Biff = detectStreamBiffVersion( aBookStrm5 );

            // try to open the "Workbook" stream
            const OUString saWorkbookName = CREATE_OUSTRING( "Workbook" );
            BinaryXInputStream aBookStrm8( rxStorage->openInputStream( saWorkbookName ), true );
            BiffType eBookStrm8Biff = detectStreamBiffVersion( aBookStrm8 );

            // decide which stream to use
            if( (eBookStrm8Biff != BIFF_UNKNOWN) && ((eBookStrm5Biff == BIFF_UNKNOWN) || (eBookStrm8Biff > eBookStrm5Biff)) )
            {
                /*  Only "Workbook" stream exists; or both streams exist, and
                    "Workbook" has higher BIFF version than "Book" stream. */
                eBiff = eBookStrm8Biff;
                orWorkbookStreamName = saWorkbookName;
            }
            else if( eBookStrm5Biff != BIFF_UNKNOWN )
            {
                /*  Only "Book" stream exists; or both streams exist, and
                    "Book" has higher BIFF version than "Workbook" stream. */
                eBiff = eBookStrm5Biff;
                orWorkbookStreamName = saBookName;
            }
        }
        else
        {
            // no storage, try plain input stream from medium (even for BIFF5+)
            BinaryXInputStream aStrm( rxStorage->openInputStream( OUString() ), false );
            eBiff = detectStreamBiffVersion( aStrm );
            orWorkbookStreamName = OUString();
        }
    }

    return eBiff;
}

// ============================================================================

} // BiffDetector
} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
