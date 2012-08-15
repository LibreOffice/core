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


#include <string.h> // memset(), memcpy()
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

static sal_uInt8 cStgSignature[ 8 ] = { 0xD0,0xCF,0x11,0xE0,0xA1,0xB1,0x1A,0xE1 };

////////////////////////////// struct ClsId  /////////////////////////////

SvStream& operator >>( SvStream& r, ClsId& rId )
{
    r >> rId.n1
      >> rId.n2
      >> rId.n3
      >> rId.n4
      >> rId.n5
      >> rId.n6
      >> rId.n7
      >> rId.n8
      >> rId.n9
      >> rId.n10
      >> rId.n11;
    return r;
}

SvStream& operator <<( SvStream& r, const ClsId& rId )
{
    return
       r  << (sal_Int32) rId.n1
          << (sal_Int16) rId.n2
          << (sal_Int16) rId.n3
          << (sal_uInt8) rId.n4
          << (sal_uInt8) rId.n5
          << (sal_uInt8) rId.n6
          << (sal_uInt8) rId.n7
          << (sal_uInt8) rId.n8
          << (sal_uInt8) rId.n9
          << (sal_uInt8) rId.n10
          << (sal_uInt8) rId.n11;
}

///////////////////////////// class StgHeader ////////////////////////////

StgHeader::StgHeader()
: nVersion( 0 )
, nByteOrder( 0 )
, nPageSize( 0 )
, nDataPageSize( 0 )
, bDirty( 0 )
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
    bDirty = 0;
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
    for( short i = 0; i < 109; i++ )
        SetFATPage( i, STG_FREE );
}

sal_Bool StgHeader::Load( StgIo& rIo )
{
    sal_Bool bResult = sal_False;
    if ( rIo.GetStrm() )
    {
        SvStream& r = *rIo.GetStrm();
        bResult = Load( r );
        bResult = ( bResult && rIo.Good() );
    }

    return bResult;
}

sal_Bool StgHeader::Load( SvStream& r )
{
    r.Seek( 0L );
    r.Read( cSignature, 8 );
    r >> aClsId                     // 08 Class ID
      >> nVersion                   // 1A version number
      >> nByteOrder                 // 1C Unicode byte order indicator
      >> nPageSize                  // 1E 1 << nPageSize = block size
      >> nDataPageSize;             // 20 1 << this size == data block size
    r.SeekRel( 10 );
    r >> nFATSize                   // 2C total number of FAT pages
      >> nTOCstrm                   // 30 starting page for the TOC stream
      >> nReserved                  // 34
      >> nThreshold                 // 38 minimum file size for big data
      >> nDataFAT                   // 3C page # of 1st data FAT block
      >> nDataFATSize               // 40 # of data FATpages
      >> nMasterChain               // 44 chain to the next master block
      >> nMaster;                   // 48 # of additional master blocks
    for( short i = 0; i < 109; i++ )
        r >> nMasterFAT[ i ];

    return ( r.GetErrorCode() == ERRCODE_NONE && Check() );
}

sal_Bool StgHeader::Store( StgIo& rIo )
{
    if( !bDirty )
        return sal_True;
    SvStream& r = *rIo.GetStrm();
    r.Seek( 0L );
    r.Write( cSignature, 8 + 16 );
    r << nVersion                   // 1A version number
      << nByteOrder                 // 1C Unicode byte order indicator
      << nPageSize                  // 1E 1 << nPageSize = block size
      << nDataPageSize              // 20 1 << this size == data block size
      << (sal_Int32) 0 << (sal_Int32) 0 << (sal_Int16) 0
      << nFATSize                   // 2C total number of FAT pages
      << nTOCstrm                   // 30 starting page for the TOC stream
      << nReserved                  // 34
      << nThreshold                 // 38 minimum file size for big data
      << nDataFAT                   // 3C page # of 1st data FAT block
      << nDataFATSize               // 40 # of data FAT pages
      << nMasterChain               // 44 chain to the next master block
      << nMaster;                   // 48 # of additional master blocks
    for( short i = 0; i < 109; i++ )
        r << nMasterFAT[ i ];
    bDirty = !rIo.Good();
    return sal_Bool( !bDirty );
}

static bool lcl_wontoverflow(short shift)
{
    return shift >= 0 && shift < (short)sizeof(short) * 8 - 1;
}

// Perform thorough checks also on unknown variables
sal_Bool StgHeader::Check()
{
    return sal_Bool( memcmp( cSignature, cStgSignature, 8 ) == 0
            && (short) ( nVersion >> 16 ) == 3 )
            && nPageSize == 9
            && lcl_wontoverflow(nPageSize)
            && lcl_wontoverflow(nDataPageSize)
            && nFATSize > 0
            && nTOCstrm >= 0
            && nThreshold > 0
            && ( nDataFAT == -2 || ( nDataFAT >= 0 && nDataFATSize > 0 ) )
            && ( nMasterChain == -2 || nMasterChain >=0 )
            && nMaster >= 0;
}

sal_Int32 StgHeader::GetFATPage( short n ) const
{
    if( n >= 0 && n < 109 )
        return nMasterFAT[ n ];
    else
        return STG_EOF;
}

void StgHeader::SetFATPage( short n, sal_Int32 nb )
{
    if( n >= 0 && n < 109 )
    {
        if( nMasterFAT[ n ] != nb )
            bDirty = sal_True, nMasterFAT[ n ] = nb;
    }
}

void StgHeader::SetTOCStart( sal_Int32 n )
{
    if( n != nTOCstrm ) bDirty = sal_True, nTOCstrm = n;
}

void StgHeader::SetDataFATStart( sal_Int32 n )
{
    if( n != nDataFAT ) bDirty = sal_True, nDataFAT = n;
}

void StgHeader::SetDataFATSize( sal_Int32 n )
{
    if( n != nDataFATSize ) bDirty = sal_True, nDataFATSize = n;
}

void StgHeader::SetFATSize( sal_Int32 n )
{
    if( n != nFATSize ) bDirty = sal_True, nFATSize = n;
}

void StgHeader::SetFATChain( sal_Int32 n )
{
    if( n != nMasterChain )
        bDirty = sal_True, nMasterChain = n;
}

void StgHeader::SetMasters( sal_Int32 n )
{
    if( n != nMaster ) bDirty = sal_True, nMaster = n;
}

///////////////////////////// class StgEntry /////////////////////////////

sal_Bool StgEntry::Init()
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
    return sal_True;
}

static String ToUpperUnicode( const String & rStr )
{
    // I don't know the locale, so en_US is hopefully fine
    /*
    com.sun.star.lang.Locale aLocale;
    aLocale.Language = OUString("en");
    aLocale.Country  = OUString("US");
    */
    static rtl::OUString aEN("en");
    static rtl::OUString aUS("US");
    static CharClass aCC( com::sun::star::lang::Locale( aEN, aUS, rtl::OUString() ) );
    return aCC.uppercase( rStr );
}

sal_Bool StgEntry::SetName( const String& rName )
{
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( rName );
    aName.Erase( nMaxLegalStr );

    int i;
    for( i = 0; i < aName.Len() && i < 32; i++ )
        nName[ i ] = rName.GetChar( sal_uInt16( i ));
    while( i < 32 )
        nName[ i++ ] = 0;
    nNameLen = ( aName.Len() + 1 ) << 1;
    return sal_True;
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

void StgEntry::GetName( String& rName ) const
{
    sal_uInt16 n = nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;
    rName = rtl::OUString(nName, n);
}

// Compare two entries. Do this case-insensitive.

short StgEntry::Compare( const StgEntry& r ) const
{
    /*
    short nRes = r.nNameLen - nNameLen;
    if( !nRes ) return strcmp( r.aName, aName );
    else return nRes;
    */
    sal_Int32 nRes = r.nNameLen - nNameLen;
    if( !nRes )
        nRes = r.aName.CompareTo( aName );
    return (short)nRes;
    //return aName.CompareTo( r.aName );
}

// These load/store operations are a bit more complicated,
// since they have to copy their contents into a packed structure.

sal_Bool StgEntry::Load( const void* pFrom, sal_uInt32 nBufSize )
{
    if ( nBufSize < 128 )
        return sal_False;

    SvMemoryStream r( (sal_Char*) pFrom, nBufSize, STREAM_READ );
    for( short i = 0; i < 32; i++ )
        r >> nName[ i ];            // 00 name as WCHAR
    r >> nNameLen                   // 40 size of name in bytes including 00H
      >> cType                      // 42 entry type
      >> cFlags                     // 43 0 or 1 (tree balance?)
      >> nLeft                      // 44 left node entry
      >> nRight                     // 48 right node entry
      >> nChild                     // 4C 1st child entry if storage
      >> aClsId                     // 50 class ID (optional)
      >> nFlags                     // 60 state flags(?)
      >> nMtime[ 0 ]                // 64 modification time
      >> nMtime[ 1 ]                // 64 modification time
      >> nAtime[ 0 ]                // 6C creation and access time
      >> nAtime[ 1 ]                // 6C creation and access time
      >> nPage1                     // 74 starting block (either direct or translated)
      >> nSize                      // 78 file size
      >> nUnknown;                  // 7C unknown

    sal_uInt16 n = nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;

    if (n > nMaxLegalStr)
        return sal_False;

    if ((nSize < 0 && cType != STG_STORAGE) || (nPage1 < 0 && nPage1 != -2))
    {
        // the size makes no sense for the substorage
        // TODO/LATER: actually the size should be an unsigned value, but in this case it would mean a stream of more than 2Gb
        return sal_False;
    }

    aName = rtl::OUString( nName, n );
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( aName );
    aName.Erase( nMaxLegalStr );

    return sal_True;
}

void StgEntry::Store( void* pTo )
{
    SvMemoryStream r( (sal_Char *)pTo, 128, STREAM_WRITE );
    for( short i = 0; i < 32; i++ )
        r << nName[ i ];            // 00 name as WCHAR
    r << nNameLen                   // 40 size of name in bytes including 00H
      << cType                      // 42 entry type
      << cFlags                     // 43 0 or 1 (tree balance?)
      << nLeft                      // 44 left node entry
      << nRight                     // 48 right node entry
      << nChild                     // 4C 1st child entry if storage;
      << aClsId                     // 50 class ID (optional)
      << nFlags                     // 60 state flags(?)
      << nMtime[ 0 ]                // 64 modification time
      << nMtime[ 1 ]                // 64 modification time
      << nAtime[ 0 ]                // 6C creation and access time
      << nAtime[ 1 ]                // 6C creation and access time
      << nPage1                     // 74 starting block (either direct or translated)
      << nSize                      // 78 file size
      << nUnknown;                  // 7C unknown
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
