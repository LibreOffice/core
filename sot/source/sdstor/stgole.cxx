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

#include "rtl/string.h"
#include "stgole.hxx"
#include "sot/storinfo.hxx"     // Read/WriteClipboardFormat()

#ifdef _MSC_VER
#pragma warning(disable: 4342)
#endif
///////////////////////// class StgInternalStream ////////////////////////

StgInternalStream::StgInternalStream
    ( BaseStorage& rStg, const String& rName, sal_Bool bWr )
{
    bIsWritable = sal_True;
    sal_uInt16 nMode = bWr
                 ? STREAM_WRITE | STREAM_SHARE_DENYALL
                 : STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE;
    pStrm = rStg.OpenStream( rName, nMode );

    // set the error code right here in the stream
    SetError( rStg.GetError() );
    SetBufferSize( 1024 );
}

StgInternalStream::~StgInternalStream()
{
    delete pStrm;
}

sal_uLong StgInternalStream::GetData( void* pData, sal_uLong nSize )
{
    if( pStrm )
    {
        nSize = pStrm->Read( pData, nSize );
        SetError( pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

sal_uLong StgInternalStream::PutData( const void* pData, sal_uLong nSize )
{
    if( pStrm )
    {
        nSize = pStrm->Write( pData, nSize );
        SetError( pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

sal_uLong StgInternalStream::SeekPos( sal_uLong nPos )
{
    return pStrm ? pStrm->Seek( nPos ) : 0;
}

void StgInternalStream::FlushData()
{
    if( pStrm )
    {
        pStrm->Flush();
        SetError( pStrm->GetError() );
    }
}

void StgInternalStream::Commit()
{
    Flush();
    pStrm->Commit();
}

///////////////////////// class StgCompObjStream /////////////////////////

StgCompObjStream::StgCompObjStream( BaseStorage& rStg, sal_Bool bWr )
            : StgInternalStream( rStg, rtl::OUString("\1CompObj"), bWr )
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
}

sal_Bool StgCompObjStream::Load()
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
    aUserName.Erase();
    if( GetError() != SVSTREAM_OK )
        return sal_False;
    Seek( 8L );     // skip the first part
    sal_Int32 nMarker = 0;
    *this >> nMarker;
    if( nMarker == -1L )
    {
        *this >> aClsId;
        sal_Int32 nLen1 = 0;
        *this >> nLen1;
        if ( nLen1 > 0 )
        {
            // higher bits are ignored
            sal_uLong nStrLen = ::std::min( nLen1, (sal_Int32)0xFFFE );

            sal_Char* p = new sal_Char[ nStrLen+1 ];
            p[nStrLen] = 0;
            if( Read( p, nStrLen ) == nStrLen )
            {
                //The encoding here is "ANSI", which is pretty useless seeing as
                //the actual codepage used doesn't seem to be specified/stored
                //anywhere :-(. Might as well pick 1252 and be consistent on
                //all platforms and envs
                //http://www.openoffice.org/nonav/issues/showattachment.cgi/68668/Orginal%20Document.doc
                //for a good edge-case example
                aUserName = nStrLen ? String( p, RTL_TEXTENCODING_MS_1252 ) : String();
                nCbFormat = ReadClipboardFormat( *this );
            }
            else
                SetError( SVSTREAM_GENERALERROR );
            delete [] p;
        }
    }
    return sal_Bool( GetError() == SVSTREAM_OK );
}

sal_Bool StgCompObjStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return sal_False;
    Seek( 0L );
    rtl::OString aAsciiUserName(rtl::OUStringToOString(aUserName, RTL_TEXTENCODING_MS_1252));
    *this << (sal_Int16) 1          // Version?
              << (sal_Int16) -2                     // 0xFFFE = Byte Order Indicator
              << (sal_Int32) 0x0A03         // Windows 3.10
              << (sal_Int32) -1L
              << aClsId             // Class ID
              << (sal_Int32) (aAsciiUserName.getLength() + 1)
              << (const char *)aAsciiUserName.getStr()
              << (sal_uInt8) 0;             // string terminator
    WriteClipboardFormat( *this, nCbFormat );
    *this << (sal_Int32) 0;             // terminator
    Commit();
    return sal_Bool( GetError() == SVSTREAM_OK );
}

/////////////////////////// class StgOleStream ///////////////////////////

StgOleStream::StgOleStream( BaseStorage& rStg, sal_Bool bWr )
            : StgInternalStream( rStg, rtl::OUString("\1Ole"), bWr )
{
    nFlags = 0;
}

sal_Bool StgOleStream::Load()
{
    nFlags = 0;
    if( GetError() != SVSTREAM_OK )
        return sal_False;
    sal_Int32 version = 0;
    Seek( 0L );
    *this >> version >> nFlags;
    return sal_Bool( GetError() == SVSTREAM_OK );
}

sal_Bool StgOleStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return sal_False;
    Seek( 0L );
    *this << (sal_Int32) 0x02000001         // OLE version, format
          << (sal_Int32) nFlags             // Object flags
          << (sal_Int32) 0                  // Update Options
          << (sal_Int32) 0                  // reserved
          << (sal_Int32) 0;                 // Moniker 1
    Commit();
    return sal_Bool( GetError() == SVSTREAM_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
