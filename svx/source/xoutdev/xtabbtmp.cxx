/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabbtmp.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:25:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_LIGHT

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SVX_XPROPERTYTABLE_HXX
#include "XPropertyTable.hxx"
#endif

#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtBitmap[]  = {'s','o','b'};

static char const aChckBitmap[]  = { 0x04, 0x00, 'S','O','B','L'};  // very old
static char const aChckBitmap0[] = { 0x04, 0x00, 'S','O','B','0'};  // old
static char const aChckBitmap1[] = { 0x04, 0x00, 'S','O','B','1'};  // = 5.2
static char const aChckXML[]     = { 'P', 'K', 0x03, 0x04 };        // = 6.0

// -------------------
// class XBitmapTable
// -------------------

/*************************************************************************
|*
|* XBitmapTable::XBitmapTable()
|*
*************************************************************************/

XBitmapTable::XBitmapTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XBitmapTable::~XBitmapTable()
{
}

/************************************************************************/

XBitmapEntry* XBitmapTable::Replace(long nIndex, XBitmapEntry* pEntry )
{
    return (XBitmapEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XBitmapEntry* XBitmapTable::Remove(long nIndex)
{
    return (XBitmapEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XBitmapEntry* XBitmapTable::GetBitmap(long nIndex) const
{
    return (XBitmapEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XBitmapTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XBitmapTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XBitmapTable::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 // Version statt Tabellentyp, um auch alte Tabellen zu lesen
//BFS01 rOut << (long) -1;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XBitmapEntry* pEntry = (XBitmapEntry*)aTable.First();
//BFS01
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // Versionsverwaltung: Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     rOut << pEntry->GetXBitmap().GetBitmap();
//BFS01     pEntry = (XBitmapEntry*)aTable.Next();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XBitmapTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XBitmapEntry* pEntry = NULL;
//BFS01 long        nType;
//BFS01 long        nCount;
//BFS01 long        nIndex;
//BFS01 String      aName;
//BFS01 Bitmap      aBitmap;
//BFS01
//BFS01 rIn >> nType;
//BFS01
//BFS01 // gesamte Tabelle?
//BFS01 if (nType == 0)
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         rIn >> aBitmap;
//BFS01
//BFS01         pEntry = new XBitmapEntry (aBitmap, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 else // Version ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         rIn >> aBitmap;
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         pEntry = new XBitmapEntry (aBitmap, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// ------------------
// class XBitmapList
// ------------------

/*************************************************************************
|*
|* XBitmapList::XBitmapList()
|*
*************************************************************************/

XBitmapList::XBitmapList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize)
{
    // pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XBitmapList::~XBitmapList()
{
}

/************************************************************************/

XBitmapEntry* XBitmapList::Replace(XBitmapEntry* pEntry, long nIndex )
{
    return (XBitmapEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XBitmapEntry* XBitmapList::Remove(long nIndex)
{
    return (XBitmapEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XBitmapEntry* XBitmapList::GetBitmap(long nIndex) const
{
    return (XBitmapEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XBitmapList::Load()
{
//BFS01#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return FALSE;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

//BFS01     // check if file exists, SfxMedium shows an errorbox else
//BFS01     {
//BFS01         com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xHandler;
//BFS01         SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );
//BFS01
//BFS01         sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);
//BFS01
//BFS01         if( pIStm )
//BFS01             delete pIStm;
//BFS01
//BFS01         if( !bOk )
//BFS01             return sal_False;
//BFS01     }

//BFS01     {
//BFS01         SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE, TRUE );
//BFS01
//BFS01         SvStream* pStream = aMedium.GetInStream();
//BFS01         if( !pStream )
//BFS01             return( FALSE );
//BFS01
//BFS01         char aCheck[6];
//BFS01         pStream->Read( aCheck, 6 );
//BFS01
//BFS01         // Handelt es sich um die gew"unschte Tabelle?
//BFS01         if( memcmp( aCheck, aChckBitmap, sizeof( aChckBitmap ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckBitmap0, sizeof( aChckBitmap0 ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckBitmap1, sizeof( aChckBitmap1 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapList::Save()
{
//BFS01#ifndef SVX_LIGHT

    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckBitmap1, 4 );
    pStream->WriteByteString(String( pszChckBitmap1, 4 ));
    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
*/
//BFS01#else
//BFS01 return FALSE;
//BFS01#endif
}

/************************************************************************/
// Umgestellt am 27.07.95 auf XBitmap

BOOL XBitmapList::Create()
{
    // Array der Bitmap
    //-----------------------
    // 00 01 02 03 04 05 06 07
    // 08 09 10 11 12 13 14 15
    // 16 17 18 19 20 21 22 23
    // 24 25 26 27 28 29 30 31
    // 32 33 34 35 36 37 38 39
    // 40 41 42 43 44 45 46 47
    // 48 49 50 51 52 53 54 55
    // 56 57 58 59 60 61 62 63

    String  aStr( SVX_RES( RID_SVXSTR_BITMAP ) );
    Color   aColWhite( RGB_Color( COL_WHITE ) );
    xub_StrLen nLen;
    USHORT  aArray[64];

    memset( aArray, 0, sizeof( aArray ) );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XBitmapEntry( XOBitmap( aArray, aColWhite, aColWhite ), aStr ) );

    aArray[ 0] = 1; aArray[ 9] = 1; aArray[18] = 1; aArray[27] = 1;
    aArray[36] = 1; aArray[45] = 1; aArray[54] = 1; aArray[63] = 1;
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_BLACK ), aColWhite ), aStr ) );

    aArray[ 7] = 1; aArray[14] = 1; aArray[21] = 1; aArray[28] = 1;
    aArray[35] = 1; aArray[42] = 1; aArray[49] = 1; aArray[56] = 1;
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_LIGHTRED ), aColWhite ), aStr ) );

    aArray[24] = 1; aArray[25] = 1; aArray[26] = 1;
    aArray[29] = 1; aArray[30] = 1; aArray[31] = 1;
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_LIGHTBLUE ), aColWhite ), aStr ) );

    return( TRUE );
}

/************************************************************************/

BOOL XBitmapList::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XBitmapList::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XBitmapList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Version !!!
//BFS01 rOut << (long)-2;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XBitmapEntry* pEntry = NULL;
//BFS01
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // Versionsverwaltung: Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     pEntry = Get(nIndex);
//BFS01     XOBitmap aXOBitmap( pEntry->GetXBitmap() );
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     rOut << (INT16) aXOBitmap.GetBitmapStyle();
//BFS01     rOut << (INT16) aXOBitmap.GetBitmapType();
//BFS01
//BFS01     if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
//BFS01     {
//BFS01         rOut << aXOBitmap.GetBitmap();
//BFS01     }
//BFS01     else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
//BFS01     {
//BFS01         USHORT* pArray = aXOBitmap.GetPixelArray();
//BFS01         for( USHORT i = 0; i < 64; i++ )
//BFS01             rOut << (USHORT) *( pArray + i );
//BFS01
//BFS01         rOut << aXOBitmap.GetPixelColor();
//BFS01         rOut << aXOBitmap.GetBackgroundColor();
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01XubString& XBitmapList::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( USHORT i=0; i<(RID_SVXSTR_BMP_DEF_END-RID_SVXSTR_BMP_DEF_START+1) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_BMP_DEF_START + i );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_BMP_START + i ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XBitmapList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01 XBitmapEntry* pEntry = NULL;
//BFS01 long        nCount;
//BFS01 String      aName;
//BFS01
//BFS01 rIn >> nCount; // Version oder Anzahl ?
//BFS01
//BFS01 if( nCount >= 0 ) // Erste Version
//BFS01 {
//BFS01     for( long nIndex = 0; nIndex < nCount; nIndex++ )
//BFS01     {
//BFS01         // Behandlung der alten Bitmaps
//BFS01         Bitmap  aBmp;
//BFS01         XOBitmap    aXOBitmap;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> aBmp;
//BFS01
//BFS01         aXOBitmap.SetBitmap( aBmp );
//BFS01         aXOBitmap.SetBitmapStyle( XBITMAP_TILE );
//BFS01
//BFS01         if( aBmp.GetSizePixel().Width() == 8 &&
//BFS01             aBmp.GetSizePixel().Height() == 8 )
//BFS01         {
//BFS01             aXOBitmap.SetBitmapType( XBITMAP_8X8 );
//BFS01             aXOBitmap.Bitmap2Array();
//BFS01         }
//BFS01         else
//BFS01             aXOBitmap.SetBitmapType( XBITMAP_IMPORT );
//BFS01
//BFS01         pEntry = new XBitmapEntry( aXOBitmap, aName );
//BFS01         Insert( pEntry, nIndex );
//BFS01     }
//BFS01 }
//BFS01 else if( nCount == -1 ) // Zweite Version
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for( long nIndex = 0; nIndex < nCount; nIndex++ )
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         XOBitmap aXOBitmap;
//BFS01         INT16   iTmp;
//BFS01
//BFS01         rIn >> iTmp;
//BFS01         aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
//BFS01         rIn >> iTmp;
//BFS01         aXOBitmap.SetBitmapType( (XBitmapType) iTmp );
//BFS01
//BFS01         if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
//BFS01         {
//BFS01             Bitmap aBmp;
//BFS01             rIn >> aBmp;
//BFS01             aXOBitmap.SetBitmap( aBmp );
//BFS01         }
//BFS01         else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
//BFS01         {
//BFS01             USHORT* pArray = new USHORT[ 64 ];
//BFS01             Color   aColor;
//BFS01
//BFS01             for( USHORT i = 0; i < 64; i++ )
//BFS01                 rIn >> *( pArray + i );
//BFS01             aXOBitmap.SetPixelArray( pArray );
//BFS01
//BFS01             rIn >> aColor;
//BFS01             aXOBitmap.SetPixelColor( aColor );
//BFS01             rIn >> aColor;
//BFS01             aXOBitmap.SetBackgroundColor( aColor );
//BFS01
//BFS01             delete []pArray;
//BFS01         }
//BFS01
//BFS01         pEntry = new XBitmapEntry( aXOBitmap, aName );
//BFS01         Insert( pEntry, nIndex );
//BFS01     }
//BFS01 }
//BFS01 else // Version ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for( long nIndex = 0; nIndex < nCount; nIndex++ )
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         XOBitmap aXOBitmap;
//BFS01         INT16   iTmp;
//BFS01
//BFS01         rIn >> iTmp;
//BFS01         aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
//BFS01         rIn >> iTmp;
//BFS01         aXOBitmap.SetBitmapType( (XBitmapType) iTmp );
//BFS01
//BFS01         if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
//BFS01         {
//BFS01             Bitmap aBmp;
//BFS01             rIn >> aBmp;
//BFS01             aXOBitmap.SetBitmap( aBmp );
//BFS01         }
//BFS01         else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
//BFS01         {
//BFS01             USHORT* pArray = new USHORT[ 64 ];
//BFS01             Color   aColor;
//BFS01
//BFS01             for( USHORT i = 0; i < 64; i++ )
//BFS01                 rIn >> *( pArray + i );
//BFS01             aXOBitmap.SetPixelArray( pArray );
//BFS01
//BFS01             rIn >> aColor;
//BFS01             aXOBitmap.SetPixelColor( aColor );
//BFS01             rIn >> aColor;
//BFS01             aXOBitmap.SetBackgroundColor( aColor );
//BFS01
//BFS01             delete []pArray;
//BFS01         }
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         pEntry = new XBitmapEntry( aXOBitmap, aName );
//BFS01         Insert( pEntry, nIndex );
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return( rIn );
//BFS01}


// eof
