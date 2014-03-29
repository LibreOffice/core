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
#include "sot/storinfo.hxx"

#ifdef _MSC_VER
#pragma warning(disable: 4342)
#endif
///////////////////////// class StgInternalStream

StgInternalStream::StgInternalStream( BaseStorage& rStg, const OUString& rName, bool bWr )
{
    bIsWritable = true;
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

sal_uInt64 StgInternalStream::SeekPos(sal_uInt64 const nPos)
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

///////////////////////// class StgCompObjStream

StgCompObjStream::StgCompObjStream( BaseStorage& rStg, bool bWr )
    : StgInternalStream( rStg, OUString("\1CompObj"), bWr )
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
}

bool StgCompObjStream::Load()
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
    aUserName = "";
    if( GetError() != SVSTREAM_OK )
        return false;
    Seek( 8L );     // skip the first part
    sal_Int32 nMarker = 0;
    ReadInt32( nMarker );
    if( nMarker == -1L )
    {
        ReadClsId( *this, aClsId );
        sal_Int32 nLen1 = 0;
        ReadInt32( nLen1 );
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
                //https://issues.apache.org/ooo/attachment.cgi?id=68668
                //for a good edge-case example
                aUserName = nStrLen ? OUString( p, nStrLen, RTL_TEXTENCODING_MS_1252 ) : OUString();
                nCbFormat = ReadClipboardFormat( *this );
            }
            else
                SetError( SVSTREAM_GENERALERROR );
            delete [] p;
        }
    }
    return GetError() == SVSTREAM_OK;
}

bool StgCompObjStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return false;
    Seek( 0L );
    OString aAsciiUserName(OUStringToOString(aUserName, RTL_TEXTENCODING_MS_1252));
    WriteInt16( (sal_Int16) 1 );          // Version?
    WriteInt16( (sal_Int16) -2 );                     // 0xFFFE = Byte Order Indicator
    WriteInt32( (sal_Int32) 0x0A03 );         // Windows 3.10
    WriteInt32( (sal_Int32) -1L );
    WriteClsId( *this, aClsId );             // Class ID
    WriteInt32( (sal_Int32) (aAsciiUserName.getLength() + 1) );
    WriteCharPtr( (const char *)aAsciiUserName.getStr() );
    WriteUChar( (sal_uInt8) 0 );             // string terminator
    WriteClipboardFormat( *this, nCbFormat );
    WriteInt32( (sal_Int32) 0 );             // terminator
    Commit();
    return GetError() == SVSTREAM_OK;
}

/////////////////////////// class StgOleStream

StgOleStream::StgOleStream( BaseStorage& rStg, bool bWr )
    : StgInternalStream( rStg, OUString("\1Ole"), bWr )
{
    nFlags = 0;
}

bool StgOleStream::Load()
{
    nFlags = 0;
    if( GetError() != SVSTREAM_OK )
        return false;

    sal_Int32 version = 0;
    Seek( 0L );
    ReadInt32( version ).ReadUInt32( nFlags );
    return GetError() == SVSTREAM_OK;
}

bool StgOleStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return false;

    Seek( 0L );
    WriteInt32( (sal_Int32) 0x02000001 );         // OLE version, format
    WriteInt32( (sal_Int32) nFlags );             // Object flags
    WriteInt32( (sal_Int32) 0 );                  // Update Options
    WriteInt32( (sal_Int32) 0 );                  // reserved
    WriteInt32( (sal_Int32) 0 );                 // Moniker 1
    Commit();
    return GetError() == SVSTREAM_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
