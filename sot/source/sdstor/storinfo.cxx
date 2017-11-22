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
#include <memory>

/************** class SvStorageInfo **************************************
*************************************************************************/
SotClipboardFormatId ReadClipboardFormat( SvStream & rStm )
{
    SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
    sal_Int32 nLen = 0;
    rStm.ReadInt32( nLen );
    if( rStm.eof() )
        rStm.SetError( SVSTREAM_GENERALERROR );
    if( nLen > 0 )
    {
        // get a string name
        std::unique_ptr<sal_Char[]> p(new( ::std::nothrow ) sal_Char[ nLen ]);
        if (p && rStm.ReadBytes(p.get(), nLen) == static_cast<std::size_t>(nLen))
        {
            nFormat = SotExchange::RegisterFormatName(OUString(p.get(), nLen-1, RTL_TEXTENCODING_ASCII_US));
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
    }
    else if( nLen == -1 )
    {
        // Windows clipboard format
        // SV and Win match (up to and including SotClipboardFormatId::GDIMETAFILE)
        sal_uInt32 nTmp;
        rStm.ReadUInt32( nTmp );
        nFormat = static_cast<SotClipboardFormatId>(nTmp);
    }
    else if( nLen == -2 )
    {
        sal_uInt32 nTmp;
        rStm.ReadUInt32( nTmp );
        nFormat = static_cast<SotClipboardFormatId>(nTmp);
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

void WriteClipboardFormat( SvStream & rStm, SotClipboardFormatId nFormat )
{
    // determine the clipboard format string
    OUString aCbFmt;
    if( nFormat > SotClipboardFormatId::GDIMETAFILE )
        aCbFmt = SotExchange::GetFormatName( nFormat );
    if( !aCbFmt.isEmpty() )
    {
        OString aAsciiCbFmt(OUStringToOString(aCbFmt,
                                              RTL_TEXTENCODING_ASCII_US));
        rStm.WriteInt32( aAsciiCbFmt.getLength() + 1 );
        rStm.WriteCharPtr( aAsciiCbFmt.getStr() );
        rStm.WriteUChar( 0 );
    }
    else if( nFormat != SotClipboardFormatId::NONE )
    {
        rStm.WriteInt32( -1 )         // for Windows
            .WriteInt32( static_cast<sal_Int32>(nFormat) );
    }
    else
    {
        rStm.WriteInt32( 0 );         // no clipboard format
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
