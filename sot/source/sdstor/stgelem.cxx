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
#include "sot/stg.hxx"
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
: nVersion( 0 )
, nByteOrder( 0 )
, nPageSize( 0 )
, nDataPageSize( 0 )
, bDirty( sal_uInt8(false) )
, nFATSize( 0 )
, nTOCstrm( 0 )
, nReserved( 0 )
, nThreshold( 0 )
, nDataFAT( 0 )
, nDataFATSize( 0 )
, nMasterChain( 0 )
, nMaster( 0 )
{
    memset( cSignature, 0, sizeof( cSignature ) );
    memset( &aClsId, 0, sizeof( ClsId ) );
    memset( cReserved, 0, sizeof( cReserved ) );
    memset( nMasterFAT, 0, sizeof( nMasterFAT ) );
}

void StgHeader::Init()
{
    memcpy( cSignature, cStgSignature, 8 );
    memset( &aClsId, 0, sizeof( ClsId ) );
    nVersion      = 0x0003003B;
    nByteOrder    = 0xFFFE;
    nPageSize     = 9;          // 512 bytes
    nDataPageSize = 6;          // 64 bytes
    bDirty = sal_uInt8(false);
    memset( cReserved, 0, sizeof( cReserved ) );
    nFATSize = 0;
    nTOCstrm = 0;
    nReserved = 0;
    nThreshold    = 4096;
    nDataFAT = 0;
    nDataFATSize  = 0;
    nMasterChain  = STG_EOF;

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
    r.Seek( 0L );
    r.Read( cSignature, 8 );
    ReadClsId( r, aClsId );         // 08 Class ID
    r.ReadInt32( nVersion )                   // 1A version number
     .ReadUInt16( nByteOrder )                 // 1C Unicode byte order indicator
     .ReadInt16( nPageSize )                  // 1E 1 << nPageSize = block size
     .ReadInt16( nDataPageSize );             // 20 1 << this size == data block size
    r.SeekRel( 10 );
    r.ReadInt32( nFATSize )                   // 2C total number of FAT pages
     .ReadInt32( nTOCstrm )                   // 30 starting page for the TOC stream
     .ReadInt32( nReserved )                  // 34
     .ReadInt32( nThreshold )                 // 38 minimum file size for big data
     .ReadInt32( nDataFAT )                   // 3C page # of 1st data FAT block
     .ReadInt32( nDataFATSize )               // 40 # of data FATpages
     .ReadInt32( nMasterChain )               // 44 chain to the next master block
     .ReadInt32( nMaster );                   // 48 # of additional master blocks
    for( short i = 0; i < cFATPagesInHeader; i++ )
        r.ReadInt32( nMasterFAT[ i ] );

    return (r.GetErrorCode() == ERRCODE_NONE) && Check();
}

bool StgHeader::Store( StgIo& rIo )
{
    if( !bDirty )
        return true;

    SvStream& r = *rIo.GetStrm();
    r.Seek( 0L );
    r.Write( cSignature, 8 );
    WriteClsId( r, aClsId );                   // 08 Class ID
    r.WriteInt32( nVersion )                   // 1A version number
     .WriteUInt16( nByteOrder )                 // 1C Unicode byte order indicator
     .WriteInt16( nPageSize )                  // 1E 1 << nPageSize = block size
     .WriteInt16( nDataPageSize )              // 20 1 << this size == data block size
     .WriteInt32( 0 ).WriteInt32( 0 ).WriteInt16( 0 )
     .WriteInt32( nFATSize )                   // 2C total number of FAT pages
     .WriteInt32( nTOCstrm )                   // 30 starting page for the TOC stream
     .WriteInt32( nReserved )                  // 34
     .WriteInt32( nThreshold )                 // 38 minimum file size for big data
     .WriteInt32( nDataFAT )                   // 3C page # of 1st data FAT block
     .WriteInt32( nDataFATSize )               // 40 # of data FAT pages
     .WriteInt32( nMasterChain )               // 44 chain to the next master block
     .WriteInt32( nMaster );                   // 48 # of additional master blocks
    for( short i = 0; i < cFATPagesInHeader; i++ )
        r.WriteInt32( nMasterFAT[ i ] );
    bDirty = sal_uInt8(!rIo.Good());
    return !bDirty;
}

static bool lcl_wontoverflow(short shift)
{
    return shift >= 0 && shift < (short)sizeof(short) * 8 - 1;
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
    return  memcmp( cSignature, cStgSignature, 8 ) == 0
            && (short) ( nVersion >> 16 ) == 3
            && nPageSize == 9
            && lcl_wontoverflow(nPageSize)
            && lcl_wontoverflow(nDataPageSize)
            && nFATSize > 0
            && nTOCstrm >= 0
            && nThreshold > 0
            && ( isKnownSpecial(nDataFAT) || ( nDataFAT >= 0 && nDataFATSize > 0 ) )
            && ( isKnownSpecial(nMasterChain) || nMasterChain >=0 )
            && nMaster >= 0;
}

sal_Int32 StgHeader::GetFATPage( short n ) const
{
    if( n >= 0 && n < cFATPagesInHeader )
        return nMasterFAT[ n ];
    else
        return STG_EOF;
}

void StgHeader::SetFATPage( short n, sal_Int32 nb )
{
    if( n >= 0 && n < cFATPagesInHeader )
    {
        if( nMasterFAT[ n ] != nb )
            bDirty = sal_uInt8(true), nMasterFAT[ n ] = nb;
    }
}

void StgHeader::SetTOCStart( sal_Int32 n )
{
    if( n != nTOCstrm ) bDirty = sal_uInt8(true), nTOCstrm = n;
}

void StgHeader::SetDataFATStart( sal_Int32 n )
{
    if( n != nDataFAT ) bDirty = sal_uInt8(true), nDataFAT = n;
}

void StgHeader::SetDataFATSize( sal_Int32 n )
{
    if( n != nDataFATSize ) bDirty = sal_uInt8(true), nDataFATSize = n;
}

void StgHeader::SetFATSize( sal_Int32 n )
{
    if( n != nFATSize ) bDirty = sal_uInt8(true), nFATSize = n;
}

void StgHeader::SetFATChain( sal_Int32 n )
{
    if( n != nMasterChain )
        bDirty = sal_uInt8(true), nMasterChain = n;
}

void StgHeader::SetMasters( sal_Int32 n )
{
    if( n != nMaster ) bDirty = sal_uInt8(true), nMaster = n;
}

///////////////////////////// class StgEntry

bool StgEntry::Init()
{
    memset( nName, 0, sizeof( nName ) );
    nNameLen = 0;
    cType = 0;
    cFlags = 0;
    nLeft = 0;
    nRight = 0;
    nChild = 0;
    memset( &aClsId, 0, sizeof( aClsId ) );
    nFlags = 0;
    nMtime[0] = 0; nMtime[1] = 0;
    nAtime[0] = 0; nAtime[1] = 0;
    nPage1 = 0;
    nSize = 0;
    nUnknown = 0;

    SetLeaf( STG_LEFT,  STG_FREE );
    SetLeaf( STG_RIGHT, STG_FREE );
    SetLeaf( STG_CHILD, STG_FREE );
    SetLeaf( STG_DATA,  STG_EOF );
    return true;
}

static OUString ToUpperUnicode( const OUString & rStr )
{
    // I don't know the locale, so en_US is hopefully fine
    static CharClass aCC( LanguageTag( com::sun::star::lang::Locale( "en", "US", "" )) );
    return aCC.uppercase( rStr );
}

bool StgEntry::SetName( const OUString& rName )
{
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( rName );
    if(aName.getLength() > nMaxLegalStr)
    {
        aName = aName.copy(0, nMaxLegalStr);
    }

    sal_uInt16 i;
    for( i = 0; i < rName.getLength() && i <= nMaxLegalStr; i++ )
    {
        nName[ i ] = rName[ i ];
    }
    while (i <= nMaxLegalStr)
    {
        nName[ i++ ] = 0;
    }
    nNameLen = ( rName.getLength() + 1 ) << 1;
    return true;
}

sal_Int32 StgEntry::GetLeaf( StgEntryRef eRef ) const
{
    sal_Int32 n = -1;
    switch( eRef )
    {
    case STG_LEFT:  n = nLeft;  break;
    case STG_RIGHT: n = nRight; break;
    case STG_CHILD: n = nChild; break;
    case STG_DATA:  n = nPage1; break;
    }
    return n;
}

void StgEntry::SetLeaf( StgEntryRef eRef, sal_Int32 nPage )
{
    switch( eRef )
    {
    case STG_LEFT:  nLeft  = nPage; break;
    case STG_RIGHT: nRight = nPage; break;
    case STG_CHILD: nChild = nPage; break;
    case STG_DATA:  nPage1 = nPage; break;
    }
}

void StgEntry::SetClassId( const ClsId& r )
{
    memcpy( &aClsId, &r, sizeof( ClsId ) );
}

void StgEntry::GetName( OUString& rName ) const
{
    sal_uInt16 n = nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;
    rName = OUString(nName, n);
}

// Compare two entries. Do this case-insensitive.

sal_Int32 StgEntry::Compare( const StgEntry& r ) const
{
    if (r.nNameLen != nNameLen)
        return r.nNameLen > nNameLen ? 1 : -1;
    else
        return r.aName.compareTo(aName);
}

// These load/store operations are a bit more complicated,
// since they have to copy their contents into a packed structure.

bool StgEntry::Load(const void* pFrom, sal_uInt32 nBufSize, sal_uInt64 nUnderlyingStreamSize)
{
    if ( nBufSize < 128 )
        return false;

    SvMemoryStream r( const_cast<void *>(pFrom), nBufSize, StreamMode::READ );
    for( short i = 0; i < 32; i++ )
        r.ReadUInt16( nName[ i ] );            // 00 name as WCHAR
    r.ReadUInt16( nNameLen )                   // 40 size of name in bytes including 00H
     .ReadUChar( cType )                      // 42 entry type
     .ReadUChar( cFlags )                     // 43 0 or 1 (tree balance?)
     .ReadInt32( nLeft )                      // 44 left node entry
     .ReadInt32( nRight )                     // 48 right node entry
     .ReadInt32( nChild );                    // 4C 1st child entry if storage
    ReadClsId( r, aClsId );         // 50 class ID (optional)
    r.ReadInt32( nFlags )                     // 60 state flags(?)
     .ReadInt32( nMtime[ 0 ] )                // 64 modification time
     .ReadInt32( nMtime[ 1 ] )                // 64 modification time
     .ReadInt32( nAtime[ 0 ] )                // 6C creation and access time
     .ReadInt32( nAtime[ 1 ] )                // 6C creation and access time
     .ReadInt32( nPage1 )                     // 74 starting block (either direct or translated)
     .ReadInt32( nSize )                      // 78 file size
     .ReadInt32( nUnknown );                  // 7C unknown

    sal_uInt16 n = nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;

    if (n > nMaxLegalStr)
        return false;

    if (cType != STG_STORAGE)
    {
        if (nPage1 < 0 && !isKnownSpecial(nPage1))
        {
            //bad pageid
            return false;
        }
        if (nSize < 0)
        {
            // the size makes no sense for the substorage
            // TODO/LATER: actually the size should be an unsigned value, but
            // in this case it would mean a stream of more than 2Gb
            return false;
        }
        if (static_cast<sal_uInt64>(nSize) > nUnderlyingStreamSize)
        {
            // surely an entry cannot be larger than the underlying file
            return false;
        }

    }

    aName = OUString(nName , n);
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( aName );
    if(aName.getLength() > nMaxLegalStr)
    {
        aName = aName.copy(0, nMaxLegalStr);
    }

    return true;
}

void StgEntry::Store( void* pTo )
{
    SvMemoryStream r( pTo, 128, StreamMode::WRITE );
    for( short i = 0; i < 32; i++ )
        r.WriteUInt16( nName[ i ] );            // 00 name as WCHAR
    r.WriteUInt16( nNameLen )                   // 40 size of name in bytes including 00H
     .WriteUChar( cType )                      // 42 entry type
     .WriteUChar( cFlags )                     // 43 0 or 1 (tree balance?)
     .WriteInt32( nLeft )                      // 44 left node entry
     .WriteInt32( nRight )                     // 48 right node entry
     .WriteInt32( nChild );                    // 4C 1st child entry if storage;
    WriteClsId( r, aClsId );                   // 50 class ID (optional)
    r.WriteInt32( nFlags )                     // 60 state flags(?)
     .WriteInt32( nMtime[ 0 ] )                // 64 modification time
     .WriteInt32( nMtime[ 1 ] )                // 64 modification time
     .WriteInt32( nAtime[ 0 ] )                // 6C creation and access time
     .WriteInt32( nAtime[ 1 ] )                // 6C creation and access time
     .WriteInt32( nPage1 )                     // 74 starting block (either direct or translated)
     .WriteInt32( nSize )                      // 78 file size
     .WriteInt32( nUnknown );                  // 7C unknown
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
