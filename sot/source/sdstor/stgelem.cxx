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
{
    memset( this, 0, sizeof( StgHeader ) );
}

void StgHeader::Init()
{
    memset( this, 0, sizeof( StgHeader ) );
    memcpy( cSignature, cStgSignature, 8 );
    nVersion      = 0x0003003B;
    nByteOrder    = 0xFFFE;
    nPageSize     = 9;          // 512 bytes
    nDataPageSize = 6;          // 64 bytes
    nThreshold    = 4096;
    nDataFATSize  = 0;
    nMasterChain  = STG_EOF;
    SetTOCStart( STG_EOF );
    SetDataFATStart( STG_EOF );
    for( short i = 0; i < 109; i++ )
        SetFATPage( i, STG_FREE );
}

sal_Bool StgHeader::Load( StgIo& rIo )
{
    SvStream& r = *rIo.GetStrm();
    Load( r );
    return rIo.Good();
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
    return r.GetErrorCode() == ERRCODE_NONE;
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
            && lcl_wontoverflow(nPageSize)
            && lcl_wontoverflow(nDataPageSize);
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

void StgHeader::SetClassId( const ClsId& r )
{
    if( memcmp( &aClsId, &r, sizeof( ClsId ) ) )
        bDirty = sal_True, memcpy( &aClsId, &r, sizeof( ClsId ) );
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

// This class is only a wrapper around teh dir entry structure
// which retrieves and sets data.

// The name must be smaller than 32 chars. Conversion into Unicode
// is easy, since the 1st 256 characters of the Windows ANSI set
// equal the 1st 256 Unicode characters.
/*
void ToUnicode_Impl( String& rName )
{
    rName.Erase( 32 );
    rName.Convert( ::GetSystemCharSet(), CHARSET_ANSI );
    // brute force is OK
    sal_uInt8* p = (sal_uInt8*) rName.GetCharStr();
    for( sal_uInt16 i = 0; i < rName.Len(); i++, p++ )
    {
        // check each character and substitute blanks for illegal ones
        sal_uInt8 cChar = *p;
        if( cChar == '!' || cChar == ':' || cChar == '\\' || cChar == '/' )
            *p = ' ';
    }
}
*/
/*
static void FromUnicode( String& rName )
{
    rName.Convert( CHARSET_ANSI, ::GetSystemCharSet() );
}
*/
sal_Bool StgEntry::Init()
{
    memset( this, 0, sizeof (StgEntry) - sizeof( String ) );
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
    aLocale.Language = OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
    aLocale.Country  = OUString(RTL_CONSTASCII_USTRINGPARAM("US"));
    */
    static rtl::OUString aEN(RTL_CONSTASCII_USTRINGPARAM("en"));
    static rtl::OUString aUS(RTL_CONSTASCII_USTRINGPARAM("US"));
    static CharClass aCC( com::sun::star::lang::Locale( aEN, aUS, rtl::OUString() ) );
    return aCC.toUpper( rStr, 0, rStr.Len() );
}


sal_Bool StgEntry::SetName( const String& rName )
{
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( rName );
    aName.Erase( 31 );

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

const sal_Int32* StgEntry::GetTime( StgEntryTime eTime ) const
{
    return( eTime == STG_MODIFIED ) ? nMtime : nAtime;
}

void StgEntry::SetTime( StgEntryTime eTime, sal_Int32* pTime )
{
    if( eTime == STG_MODIFIED )
        nMtime[ 0 ] = *pTime++, nMtime[ 1 ] = *pTime;
    else
        nAtime[ 0 ] = *pTime++, nAtime[ 1 ] = *pTime;
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
    rName = String( nName, n );
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

sal_Bool StgEntry::Load( const void* pFrom )
{
    SvMemoryStream r( (sal_Char*) pFrom, 128, STREAM_READ );
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
    if( n > 31 || (nSize < 0 && cType != STG_STORAGE) )
    {
        // the size makes no sence for the substorage
        // TODO/LATER: actually the size should be an unsigned value, but in this case it would mean a stream of more than 2Gb
        return sal_False;
    }

    aName = String( nName, n );
    // I don't know the locale, so en_US is hopefully fine
    aName = ToUpperUnicode( aName );
    aName.Erase( 31 );

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
