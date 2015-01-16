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


#include <sot/stg.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>
#include <boost/scoped_array.hpp>

/************** class SvStorageInfo **************************************
*************************************************************************/
sal_uLong ReadClipboardFormat( SvStream & rStm )
{
    sal_uInt32 nFormat = 0;
    sal_Int32 nLen = 0;
    rStm.ReadInt32( nLen );
    if( rStm.IsEof() )
        rStm.SetError( SVSTREAM_GENERALERROR );
    if( nLen > 0 )
    {
        // get a string name
        boost::scoped_array<sal_Char> p(new( ::std::nothrow ) sal_Char[ nLen ]);
        if( p && rStm.Read( p.get(), nLen ) == (sal_uLong) nLen )
        {
            nFormat = SotExchange::RegisterFormatName(OUString(p.get(), nLen-1, RTL_TEXTENCODING_ASCII_US));
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
    }
    else if( nLen == -1L )
        // Windows clipboard format
        // SV und Win stimmen ueberein (bis einschl. FORMAT_GDIMETAFILE)
        rStm.ReadUInt32( nFormat );
    else if( nLen == -2L )
    {
        rStm.ReadUInt32( nFormat );
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
    OUString aCbFmt;
    if( nFormat > FORMAT_GDIMETAFILE )
        aCbFmt = SotExchange::GetFormatName( nFormat );
    if( !aCbFmt.isEmpty() )
    {
        OString aAsciiCbFmt(OUStringToOString(aCbFmt,
                                              RTL_TEXTENCODING_ASCII_US));
        rStm.WriteInt32( aAsciiCbFmt.getLength() + 1 );
        rStm.WriteCharPtr( (const char *)aAsciiCbFmt.getStr() );
        rStm.WriteUChar( 0 );
    }
    else if( nFormat )
    {
        rStm.WriteInt32( -1 )         // for Windows
            .WriteInt32( nFormat );
    }
    else
    {
        rStm.WriteInt32( 0 );         // no clipboard format
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
