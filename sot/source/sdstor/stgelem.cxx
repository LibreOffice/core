/*************************************************************************
 *
 *  $RCSfile: stgelem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-16 14:08:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h> // memset(), memcpy()
#include <tools/intn.hxx>
#include "stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"

#pragma hdrstop

static BYTE cStgSignature[ 8 ] = { 0xD0,0xCF,0x11,0xE0,0xA1,0xB1,0x1A,0xE1 };

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
       r  << (INT32) rId.n1
          << (INT16) rId.n2
          << (INT16) rId.n3
          << (UINT8) rId.n4
          << (UINT8) rId.n5
          << (UINT8) rId.n6
          << (UINT8) rId.n7
          << (UINT8) rId.n8
          << (UINT8) rId.n9
          << (UINT8) rId.n10
          << (UINT8) rId.n11;
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

BOOL StgHeader::Load( StgIo& rIo )
{
    SvStream& r = *rIo.GetStrm();
    Load( r );
    return rIo.Good();
}

BOOL StgHeader::Load( SvStream& r )
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

BOOL StgHeader::Store( StgIo& rIo )
{
    if( !bDirty )
        return TRUE;
    SvStream& r = *rIo.GetStrm();
    r.Seek( 0L );
    r.Write( cSignature, 8 + 16 );
    r << nVersion                   // 1A version number
      << nByteOrder                 // 1C Unicode byte order indicator
      << nPageSize                  // 1E 1 << nPageSize = block size
      << nDataPageSize              // 20 1 << this size == data block size
      << (INT32) 0 << (INT32) 0 << (INT16) 0
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
    return BOOL( !bDirty );
}

// Perform thorough checks also on unknown variables

BOOL StgHeader::Check()
{
    return BOOL( memcmp( cSignature, cStgSignature, 8 ) == 0
            &&   (short) ( nVersion >> 16 ) == 3 );
}

INT32 StgHeader::GetFATPage( short n ) const
{
    if( n >= 0 && n < 109 )
        return nMasterFAT[ n ];
    else
        return STG_EOF;
}

void StgHeader::SetFATPage( short n, INT32 nb )
{
    if( n >= 0 && n < 109 )
    {
        if( nMasterFAT[ n ] != nb )
            bDirty = TRUE, nMasterFAT[ n ] = nb;
    }
}

void StgHeader::SetClassId( const ClsId& r )
{
    if( memcmp( &aClsId, &r, sizeof( ClsId ) ) )
        bDirty = TRUE, memcpy( &aClsId, &r, sizeof( ClsId ) );
}

void StgHeader::SetTOCStart( INT32 n )
{
    if( n != nTOCstrm ) bDirty = TRUE, nTOCstrm = n;
}

void StgHeader::SetDataFATStart( INT32 n )
{
    if( n != nDataFAT ) bDirty = TRUE, nDataFAT = n;
}

void StgHeader::SetDataFATSize( INT32 n )
{
    if( n != nDataFATSize ) bDirty = TRUE, nDataFATSize = n;
}

void StgHeader::SetFATSize( INT32 n )
{
    if( n != nFATSize ) bDirty = TRUE, nFATSize = n;
}

void StgHeader::SetFATChain( INT32 n )
{
    if( n != nMasterChain )
        bDirty = TRUE, nMasterChain = n;
}

void StgHeader::SetMasters( INT32 n )
{
    if( n != nMaster ) bDirty = TRUE, nMaster = n;
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
    BYTE* p = (BYTE*) rName.GetCharStr();
    for( USHORT i = 0; i < rName.Len(); i++, p++ )
    {
        // check each character and substitute blanks for illegal ones
        BYTE cChar = *p;
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
BOOL StgEntry::Init()
{
    memset( this, 0, sizeof (StgEntry) - sizeof( String ) );
    SetLeaf( STG_LEFT,  STG_FREE );
    SetLeaf( STG_RIGHT, STG_FREE );
    SetLeaf( STG_CHILD, STG_FREE );
    SetLeaf( STG_DATA,  STG_EOF );
    return TRUE;
}

static International* pInter = 0;

inline International& GetInternational()
{
    return pInter ? *pInter : *(pInter = new International);
}

BOOL StgEntry::SetName( const String& rName )
{
    const International& aInter = GetInternational();
    // make upper case character in current language
    aName = rName;
    aName.Erase( 31 );
    aInter.ToUpper( aName );
    //ToUnicode_Impl( aNameStr );

    int i;
    for( i = 0; i < aName.Len() && i < 32; i++ )
        nName[ i ] = rName.GetChar( i );
    while( i < 32 )
        nName[ i++ ] = 0;
    nNameLen = ( aName.Len() + 1 ) << 1;
    return TRUE;
}

INT32 StgEntry::GetLeaf( StgEntryRef eRef ) const
{
    INT32 n = -1;
    switch( eRef )
    {
        case STG_LEFT:  n = nLeft;  break;
        case STG_RIGHT: n = nRight; break;
        case STG_CHILD: n = nChild; break;
        case STG_DATA:  n = nPage1; break;
    }
    return n;
}

void StgEntry::SetLeaf( StgEntryRef eRef, INT32 nPage )
{
    switch( eRef )
    {
        case STG_LEFT:  nLeft  = nPage; break;
        case STG_RIGHT: nRight = nPage; break;
        case STG_CHILD: nChild = nPage; break;
        case STG_DATA:  nPage1 = nPage; break;
    }
}

const INT32* StgEntry::GetTime( StgEntryTime eTime ) const
{
    return( eTime == STG_MODIFIED ) ? nMtime : nAtime;
}

void StgEntry::SetTime( StgEntryTime eTime, INT32* pTime )
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
    UINT16 n = nNameLen;
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

BOOL StgEntry::Load( const void* pFrom )
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

    UINT16 n = nNameLen;
    if( n )
        n = ( n >> 1 ) - 1;
    if( n > 31 )
        return FALSE;

    aName = String( nName, n );
    const International& aInter = GetInternational();
    aInter.ToUpper( aName );
    return TRUE;
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

