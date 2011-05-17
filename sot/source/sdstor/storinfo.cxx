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
#include "precompiled_sot.hxx"

#include <sot/stg.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>


/************** class SvStorageInfoList **********************************
*************************************************************************/
PRV_SV_IMPL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)

const SvStorageInfo * SvStorageInfoList::Get( const String & rEleName )
{
    for( sal_uLong i = 0; i < Count(); i++ )
    {
        const SvStorageInfo & rType = GetObject( i );
        if( rType.GetName() == rEleName )
            return &rType;
    }
    return NULL;
}

/************** class SvStorageInfo **************************************
*************************************************************************/
sal_uLong ReadClipboardFormat( SvStream & rStm )
{
    sal_uInt32 nFormat = 0;
    sal_Int32 nLen = 0;
    rStm >> nLen;
    if( rStm.IsEof() )
        rStm.SetError( SVSTREAM_GENERALERROR );
    if( nLen > 0 )
    {
        // get a string name
        sal_Char * p = new sal_Char[ nLen ];
        if( rStm.Read( p, nLen ) == (sal_uLong) nLen )
        {
            nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii( p, short(nLen-1) ) );
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
        delete [] p;
    }
    else if( nLen == -1L )
        // Windows clipboard format
        // SV und Win stimmen ueberein (bis einschl. FORMAT_GDIMETAFILE)
        rStm >> nFormat;
    else if( nLen == -2L )
    {
        rStm >> nFormat;
        // Mac clipboard format
        // ??? not implemented
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    else if( nLen != 0 )
    {
        // unknown identifier
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    return nFormat;
}

void WriteClipboardFormat( SvStream & rStm, sal_uLong nFormat )
{
    // determine the clipboard format string
    String aCbFmt;
    if( nFormat > FORMAT_GDIMETAFILE )
        aCbFmt = SotExchange::GetFormatName( nFormat );
    if( aCbFmt.Len() )
    {
        ByteString aAsciiCbFmt( aCbFmt, RTL_TEXTENCODING_ASCII_US );
        rStm << (sal_Int32) (aAsciiCbFmt.Len() + 1);
        rStm << (const char *)aAsciiCbFmt.GetBuffer();
        rStm << (sal_uInt8) 0;
    }
    else if( nFormat )
        rStm << (sal_Int32) -1         // for Windows
             << (sal_Int32) nFormat;
    else
        rStm << (sal_Int32) 0;         // no clipboard format
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
