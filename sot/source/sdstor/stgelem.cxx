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


#include <string.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unotools/charclass.hxx>
#include <sot/stg.hxx>
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"

static const sal_uInt16 nMaxLegalStr = 31;

static const sal_uInt8 cStgSignature[ 8 ] = { 0xD0,0xCF,0x11,0xE0,0xA1,0xB1,0x1A,0xE1 };

////////////////////////////// struct ClsId

SvStream& ReadClsId( SvStream& r, ClsId& rId )
{
    r.ReadUInt32( rId.Data1 )
     .ReadUInt16( rId.Data2 )
     .ReadUInt16( rId.Data3 )
     .ReadUChar( rId.Data4[0] )
     .ReadUChar( rId.Data4[1] )
     .ReadUChar( rId.Data4[2] )
     .ReadUChar( rId.Data4[3] )
     .ReadUChar( rId.Data4[4] )
     .ReadUChar( rId.Data4[5] )
     .ReadUChar( rId.Data4[6] )
     .ReadUChar( rId.Data4[7] );
    return r;
}

SvStream& WriteClsId( SvStream& r, const ClsId& rId )
{
    return
       r .WriteUInt32( rId.Data1 )
         .WriteUInt16( rId.Data2 )
         .WriteUInt16( rId.Data3 )
         .WriteUChar( rId.Data4[0] )
         .WriteUChar( rId.Data4[1] )
         .WriteUChar( rId.Data4[2] )
         .WriteUChar( rId.Data4[3] )
         .WriteUChar( rId.Data4[4] )
         .WriteUChar( rId.Data4[5] )
         .WriteUChar( rId.Data4[6] )
         .WriteUChar( rId.Data4[7] );
}

///////////////////////////// class StgHeader

StgHeader::StgHeader()
: m_nVersion( 0 )
, m_nByteOrder( 0 )
, m_nPageSize( 0 )
, m_nDataPageSize( 0 )
, m_bDirty( sal_uInt8(false) )
, m_nFATSize( 0 )
, m_nTOCstrm( 0 )
, m_nReserved( 0 )
, m_nThreshold( 0 )
, m_nDataFAT( 0 )
, m_nDataFATSize( 0 )
, m_nMasterChain( 0 )
, m_nMaster( 0 )
{
    memset( m_cSignature, 0, sizeof( m_cSignature ) );
    memset( &m_aClsId, 0, sizeof( ClsId ) );
    memset( m_cReserved, 0, sizeof( m_cReserved ) );
    memset( m_nMasterFAT, 0, sizeof( m_nMasterFAT ) );
}

void StgHeader::Init()
{
    memcpy( m_cSignature, cStgSignature, 8 );
    memset( &m_aClsId, 0, sizeof( ClsId ) );
    m_nVersion      = 0x0003003B;
    m_nByteOrder    = 0xFFFE;
    m_nPageSize     = 9;          // 512 bytes
    m_nDataPageSize = 6;          // 64 bytes
    m_bDirty = sal_uInt8(false);
    memset( m_cReserved, 0, sizeof( m_cReserved ) );
    m_nFATSize = 0;
    m_nTOCstrm = 0;
    m_nReserved = 0;
    m_nThreshold    = 4096;
    m_nDataFAT = 0;
    m_nDataFATSize  = 0;
    m_nMasterChain  = STG_EOF;

    SetTOCStart( STG_EOF );
    SetDataFATStart( STG_EOF );
    for( short i = 0; i < cFATPagesInHeader; i++ )
        SetFATPage( i, STG_FREE );
}

bool StgHeader::Load( StgIo& rIo )
{
    bool bResult = false;
    if ( rIo.GetStrm() )
    {
        SvStream& r = *rIo.GetStrm();
        bResult = Load( r );
        bResult = ( bResult && rIo.Good() );
    }

    return bResult;
}

bool StgHeader::Load( SvStream& r )
{
    r.Seek( 0 );
    r.ReadBytes( m_cSignature, 8 );
    ReadClsId( r, m_aClsId );         // 08 Class ID
    r.ReadInt32( m_nVersion )                   // 1A version number
     .ReadUInt16( m_nByteOrder )                 // 1C Unicode byte order indicator
     .ReadInt16( m_nPageSize )                  // 1E 1 << nPageSize = block size
     .ReadInt16( m_nDataPageSize );             // 20 1 << this size == data block size
    if (!r.good())
        return false;
    if (!checkSeek(r, r.Tell() + 10))
        return false;
    r.ReadInt32( m_nFATSize )                   // 2C total number of FAT pages
     .ReadInt32( m_nTOCstrm )                   // 30 starting page for the TOC stream
     .ReadInt32( m_nReserved )                  // 34
     .ReadInt32( m_nThreshold )                 // 38 minimum file size for big data
     .ReadInt32( m_nDataFAT )                   // 3C page # of 1st data FAT block
     .ReadInt32( m_nDataFATSize )               // 40 # of data FATpages
     .ReadInt32( m_nMasterChain )               // 44 chain to the next master block
     .ReadInt32( m_nMaster );                   // 48 # of additional master blocks
    for(sal_Int32 & i : m_nMasterFAT)
        r.ReadInt32( i );

    return r.good() && Check();
}

bool StgHeader::Store( StgIo& rIo )
{
    if( !m_bDirty )
        return true;

    SvStream& r = *rIo.GetStrm();
    r.Seek( 0 );
    r.WriteBytes( m_cSignature, 8 );
    WriteClsId( r, m_aClsId );                   // 08 Class ID
    r.WriteInt32( m_nVersion )                   // 1A version number
     .WriteUInt16( m_nByteOrder )                 // 1C Unicode byte order indicator
     .WriteInt16( m_nPageSize )                  // 1E 1 << nPageSize = block size
     .WriteInt16( m_nDataPageSize )              // 20 1 << this size == data block size
     .WriteInt32( 0 ).WriteInt32( 0 ).WriteInt16( 0 )
     .WriteInt32( m_nFATSize )                   // 2C total number of FAT pages
     .WriteInt32( m_nTOCstrm )                   // 30 starting page for the TOC stream
     .WriteInt32( m_nReserved )                  // 34
     .WriteInt32( m_nThreshold )                 // 38 minimum file size for big data
     .WriteInt32( m_nDataFAT )                   // 3C page # of 1st data FAT block
     .WriteInt32( m_nDataFATSize )               // 40 # of data FAT pages
     .WriteInt32( m_nMasterChain )               // 44 chain to the next master block
     .WriteInt32( m_nMaster );                   // 48 # of additional master blocks
    for(sal_Int32 i : m_nMasterFAT)
        r.WriteInt32( i );
    m_bDirty = sal_uInt8(!rIo.Good());
    return !m_bDirty;
}

static bool lcl_wontoverflow(short shift)
{
    return shift >= 0 && shift < short(sizeof(short)) * 8 - 1;
}

static bool isKnownSpecial(sal_Int32 nLocation)
{
    return (nLocation == STG_FREE ||
            nLocation == STG_EOF ||
            nLocation == STG_FAT ||
            nLocation == STG_MASTER);
}

// Perform thorough checks also on unknown variables
bool StgHeader::Check()
{
    return  memcmp( m_cSignature, cStgSignature, 8 ) == 0
            && static_cast<short>( m_nVersion >> 16 ) == 3
            && m_nPageSize == 9
            && lcl_wontoverflow(m_nPageSize)
            && lcl_wontoverflow(m_nDataPageSize)
            && m_nFATSize > 0
            && m_nTOCstrm >= 0
            && m_nThreshold > 0
            && ( isKnownSpecial(m_nDataFAT) || ( m_nDataFAT >= 0 && m_nDataFATSize > 0 ) )
            && ( isKnownSpecial(m_nMasterChain) || m_nMasterChain >=0 )
            && m_nMaster >= 0;
}

sal_Int32 StgHeader::GetFATPage( short n ) const
{
    if( n >= 0 && n < cFATPagesInHeader )
        return m_nMasterFAT[ n ];
    else
        return STG_EOF;
}

void StgHeader::SetFATPage( short n, sal_Int32 nb )
{
    if( n >= 0 && n < cFATPagesInHeader )
    {
        if( m_nMasterFAT[ n ] != nb )
        {
            m_bDirty = sal_uInt8(true);
            m_nMasterFAT[ n ] = nb;
        }
    }
}

void StgHeader::SetTOCStart( sal_Int32 n )
{
    if( n != m_nTOCstrm )
    {
        m_bDirty = sal_uInt8(true);
        m_nTOCstrm = n;
    }
}

void StgHeader::SetDataFATStart( sal_Int32 n )
{
    if( n != m_nDataFAT )
    {
        m_bDirty = sal_uInt8(true);
        m_nDataFAT = n;
    }
}

void StgHeader::SetDataFATSize( sal_Int32 n )
{
    if( n != m_nDataFATSize )
    {
        m_bDirty = sal_uInt8(true);
        m_nDataFATSize = n;
    }
}

void StgHeader::SetFATSize( sal_Int32 n )
{
    if( n != m_nFATSize )
    {
        m_bDirty = sal_uInt8(true);
        m_nFATSize = n;
    }
}

void StgHeader::SetFATChain( sal_Int32 n )
{
    if( n != m_nMasterChain )
    {
        m_bDirty = sal_uInt8(true);
        m_nMasterChain = n;
    }
}

void StgHeader::SetMasters( sal_Int32 n )
{
    if( n != m_nMaster )
    {
        m_bDirty = sal_uInt8(true);
        m_nMaster = n;
    }
}

///////////////////////////// class StgEntry

void StgEntry::Init()
{
    memset( m_nName, 0, sizeof( m_nName ) );
    m_nNameLen = 0;
    m_cType = 0;
    m_cFlags = 0;
    m_nLeft = 0;
    m_nRight = 0;
    m_nChild = 0;
    memset( &m_aClsId, 0, sizeof( m_aClsId ) );
    m_nFlags = 0;
    m_nMtime[0] = 0; m_nMtime[1] = 0;
    m_nAtime[0] = 0; m_nAtime[1] = 0;
    m_nPage1 = 0;
    m_nSize = 0;
    m_nUnknown = 0;

    SetLeaf( STG_LEFT,  STG_FREE );
    SetLeaf( STG_RIGHT, STG_FREE );
    SetLeaf( STG_CHILD, STG_FREE );
    SetLeaf( STG_DATA,  STG_EOF );
}

static OUString ToUpperUnicode( const OUString & rStr )
{
    // I don't know the locale, so en_US is hopefully fine
    static CharClass aCC( LanguageTag( css::lang::Locale( "en", "US", "" )) );
    return aCC.uppercase( rStr );
}

void StgEntry::SetName( const OUString& rName )
{
    // I don't know the locale, so en_US is hopefully fine
    m_aName = ToUpperUnicode( rName );
    if(m_aName.getLength() > nMaxLegalStr)
    {
        m_aName = m_aName.copy(0, nMaxLegalStr);
    }

    sal_Int32 i;
    for( i = 0; i < rName.getLength() && i <= nMaxLegalStr; i++ )
    {
        m_nName[ i ] = rName[ i ];
    }
    while (i <= nMaxLegalStr)
    {
        m_nName[ i++ ] = 0;
    }
    m_nNameLen = ( rName.getLength() + 1 ) << 1;
}

sal_Int32 StgEntry::GetLeaf( StgEntryRef eRef ) const
{
    sal_Int32 n = -1;
    switch( eRef )
    {
    case STG_LEFT:  n = m_nLeft;  break;
    case STG_RIGHT: n = m_nRight; break;
    case STG_CHILD: n = m_nChild; break;
    case STG_DATA:  n = m_nPage1; break;
    }
    return n;
}

void StgEntry::SetLeaf( StgEntryRef eRef, sal_Int32 nPage )
{
    switch( eRef )
    {
    case STG_LEFT:  m_nLeft  = nPage; break;
    case STG_RIGHT: m_nRight = nPage; break;
    case STG_CHILD: m_nChild = nPage; break;
    case STG_DATA:  m_nPage1 = nPage; break;
    }
}

void StgEntry::SetClassId( const ClsId& r )
{
    memcpy( &m_aClsId, &r, sizeof( ClsId ) );
}

void StgEntry::GetName( OUString& rName ) const
{
    sal_uInt16 n = m_nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;
    rName = OUString(m_nName, n);
}

// Compare two entries. Do this case-insensitive.

sal_Int32 StgEntry::Compare( const StgEntry& r ) const
{
    if (r.m_nNameLen != m_nNameLen)
        return r.m_nNameLen > m_nNameLen ? 1 : -1;
    else
        return r.m_aName.compareTo(m_aName);
}

// These load/store operations are a bit more complicated,
// since they have to copy their contents into a packed structure.

bool StgEntry::Load(const void* pFrom, sal_uInt32 nBufSize, sal_uInt64 nUnderlyingStreamSize)
{
    if ( nBufSize < 128 )
        return false;

    SvMemoryStream r( const_cast<void *>(pFrom), nBufSize, StreamMode::READ );
    for(sal_Unicode & i : m_nName)
        r.ReadUtf16( i );             // 00 name as WCHAR
    r.ReadUInt16( m_nNameLen )                   // 40 size of name in bytes including 00H
     .ReadUChar( m_cType )                      // 42 entry type
     .ReadUChar( m_cFlags )                     // 43 0 or 1 (tree balance?)
     .ReadInt32( m_nLeft )                      // 44 left node entry
     .ReadInt32( m_nRight )                     // 48 right node entry
     .ReadInt32( m_nChild );                    // 4C 1st child entry if storage
    ReadClsId( r, m_aClsId );         // 50 class ID (optional)
    r.ReadInt32( m_nFlags )                     // 60 state flags(?)
     .ReadInt32( m_nMtime[ 0 ] )                // 64 modification time
     .ReadInt32( m_nMtime[ 1 ] )                // 64 modification time
     .ReadInt32( m_nAtime[ 0 ] )                // 6C creation and access time
     .ReadInt32( m_nAtime[ 1 ] )                // 6C creation and access time
     .ReadInt32( m_nPage1 )                     // 74 starting block (either direct or translated)
     .ReadInt32( m_nSize )                      // 78 file size
     .ReadInt32( m_nUnknown );                  // 7C unknown

    sal_uInt16 n = m_nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;

    if (n > nMaxLegalStr)
        return false;

    if (m_cType != STG_STORAGE)
    {
        if (m_nPage1 < 0 && !isKnownSpecial(m_nPage1))
        {
            //bad pageid
            return false;
        }
        if (m_cType == STG_EMPTY)
        {
            /*
             tdf#112399 opens fine in MSOffice 2013 despite a massive m_nSize field

             Free (unused) directory entries are marked with Object Type 0x0
             (unknown or unallocated). The entire directory entry must consist of
             all zeroes except for the child, right sibling, and left sibling
             pointers, which must be initialized to NOSTREAM (0xFFFFFFFF).
            */
            m_nSize = 0;
        }
        if (m_nSize < 0)
        {
            // the size makes no sense for the substorage
            // TODO/LATER: actually the size should be an unsigned value, but
            // in this case it would mean a stream of more than 2Gb
            return false;
        }
        if (static_cast<sal_uInt64>(m_nSize) > nUnderlyingStreamSize)
        {
            // surely an entry cannot be larger than the underlying file
            return false;
        }

    }

    m_aName = OUString(m_nName , n);
    // I don't know the locale, so en_US is hopefully fine
    m_aName = ToUpperUnicode( m_aName );
    if(m_aName.getLength() > nMaxLegalStr)
    {
        m_aName = m_aName.copy(0, nMaxLegalStr);
    }

    return true;
}

void StgEntry::Store( void* pTo )
{
    SvMemoryStream r( pTo, 128, StreamMode::WRITE );
    for(sal_Unicode i : m_nName)
        r.WriteUInt16( i );            // 00 name as WCHAR
    r.WriteUInt16( m_nNameLen )                   // 40 size of name in bytes including 00H
     .WriteUChar( m_cType )                      // 42 entry type
     .WriteUChar( m_cFlags )                     // 43 0 or 1 (tree balance?)
     .WriteInt32( m_nLeft )                      // 44 left node entry
     .WriteInt32( m_nRight )                     // 48 right node entry
     .WriteInt32( m_nChild );                    // 4C 1st child entry if storage;
    WriteClsId( r, m_aClsId );                   // 50 class ID (optional)
    r.WriteInt32( m_nFlags )                     // 60 state flags(?)
     .WriteInt32( m_nMtime[ 0 ] )                // 64 modification time
     .WriteInt32( m_nMtime[ 1 ] )                // 64 modification time
     .WriteInt32( m_nAtime[ 0 ] )                // 6C creation and access time
     .WriteInt32( m_nAtime[ 1 ] )                // 6C creation and access time
     .WriteInt32( m_nPage1 )                     // 74 starting block (either direct or translated)
     .WriteInt32( m_nSize )                      // 78 file size
     .WriteInt32( m_nUnknown );                  // 7C unknown
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
