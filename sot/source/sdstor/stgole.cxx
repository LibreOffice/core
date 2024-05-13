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

#include <memory>
#include <algorithm>

#include "stgelem.hxx"
#include "stgole.hxx"
#include <o3tl/safeint.hxx>
#include <sot/storinfo.hxx>

///////////////////////// class StgInternalStream

StgInternalStream::StgInternalStream( BaseStorage& rStg, const OUString& rName, bool bWr )
{
    m_isWritable = true;
    StreamMode nMode = bWr
                 ? StreamMode::WRITE | StreamMode::SHARE_DENYALL
                 : StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE;
    m_pStrm.reset( rStg.OpenStream( rName, nMode ) );

    // set the error code right here in the stream
    SetError( rStg.GetError() );
    SetBufferSize( 1024 );
}

StgInternalStream::~StgInternalStream()
{
}

std::size_t StgInternalStream::GetData(void* pData, std::size_t nSize)
{
    if( m_pStrm )
    {
        nSize = m_pStrm->Read( pData, nSize );
        SetError( m_pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

std::size_t StgInternalStream::PutData(const void* pData, std::size_t nSize)
{
    if( m_pStrm )
    {
        nSize = m_pStrm->Write( pData, nSize );
        SetError( m_pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

sal_uInt64 StgInternalStream::SeekPos(sal_uInt64 const nPos)
{
    return m_pStrm ? m_pStrm->Seek( nPos ) : 0;
}

void StgInternalStream::FlushData()
{
    if( m_pStrm )
    {
        m_pStrm->Flush();
        SetError( m_pStrm->GetError() );
    }
}

void StgInternalStream::Commit()
{
    Flush();
    m_pStrm->Commit();
}

///////////////////////// class StgCompObjStream

StgCompObjStream::StgCompObjStream( BaseStorage& rStg, bool bWr )
    : StgInternalStream( rStg, u"\1CompObj"_ustr, bWr )
{
}

bool StgCompObjStream::Load()
{
    memset( &m_aClsId, 0, sizeof( ClsId ) );
    m_nCbFormat = SotClipboardFormatId::NONE;
    m_aUserName.clear();
    if( GetError() != ERRCODE_NONE )
        return false;
    Seek( 8 );     // skip the first part
    sal_Int32 nMarker = 0;
    ReadInt32( nMarker );
    if( nMarker == -1 )
    {
        ReadClsId( *this, m_aClsId );
        sal_Int32 nLen1 = 0;
        ReadInt32( nLen1 );
        if ( nLen1 > 0 )
        {
            // higher bits are ignored
            sal_Int32 nStrLen = ::std::min( nLen1, sal_Int32(0xFFFE) );

            std::unique_ptr<char[]> p(new char[ nStrLen+1 ]);
            p[nStrLen] = 0;
            if (ReadBytes( p.get(), nStrLen ) == o3tl::make_unsigned(nStrLen))
            {
                //The encoding here is "ANSI", which is pretty useless seeing as
                //the actual codepage used doesn't seem to be specified/stored
                //anywhere :-(. Might as well pick 1252 and be consistent on
                //all platforms and envs
                //https://bz.apache.org/ooo/attachment.cgi?id=68668
                //for a good edge-case example
                m_aUserName = OUString(p.get(), nStrLen, RTL_TEXTENCODING_MS_1252);
                m_nCbFormat = ReadClipboardFormat( *this );
            }
            else
                SetError( SVSTREAM_GENERALERROR );
        }
    }
    return GetError() == ERRCODE_NONE;
}

bool StgCompObjStream::Store()
{
    if( GetError() != ERRCODE_NONE )
        return false;
    Seek( 0 );
    OString aAsciiUserName(OUStringToOString(m_aUserName, RTL_TEXTENCODING_MS_1252));
    WriteInt16( 1 );          // Version?
    WriteInt16( -2 );                     // 0xFFFE = Byte Order Indicator
    WriteInt32( 0x0A03 );         // Windows 3.10
    WriteInt32( -1 );
    WriteClsId( *this, m_aClsId );             // Class ID
    WriteInt32( aAsciiUserName.getLength() + 1 );
    WriteOString( aAsciiUserName );
    WriteUChar( 0 );             // string terminator
    WriteClipboardFormat( *this, m_nCbFormat );
    WriteInt32( 0 );             // terminator
    Commit();
    return GetError() == ERRCODE_NONE;
}

/////////////////////////// class StgOleStream

StgOleStream::StgOleStream( BaseStorage& rStg )
    : StgInternalStream( rStg, u"\1Ole"_ustr, true )
{
}

bool StgOleStream::Store()
{
    if( GetError() != ERRCODE_NONE )
        return false;

    Seek( 0 );
    WriteInt32( 0x02000001 );         // OLE version, format
    WriteInt32( 0 );             // Object flags
    WriteInt32( 0 );                  // Update Options
    WriteInt32( 0 );                  // reserved
    WriteInt32( 0 );                 // Moniker 1
    Commit();
    return GetError() == ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
