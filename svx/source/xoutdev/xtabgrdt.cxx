/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabgrdt.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:26:02 $
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

// include ---------------------------------------------------------------

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

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <xflgrit.hxx>
#endif

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtGradient[]  = {'s','o','g'};

char const aChckGradient[]  = { 0x04, 0x00, 'S','O','G','L'};   // < 5.2
char const aChckGradient0[] = { 0x04, 0x00, 'S','O','G','0'};   // = 5.2
char const aChckXML[]       = { '<', '?', 'x', 'm', 'l' };      // = 6.0

// ---------------------
// class XGradientTable
// ---------------------

/*************************************************************************
|*
|* XGradientTable::XGradientTable()
|*
*************************************************************************/

XGradientTable::XGradientTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XGradientTable::~XGradientTable()
{
}

/************************************************************************/

XGradientEntry* XGradientTable::Replace(long nIndex, XGradientEntry* pEntry )
{
    return (XGradientEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XGradientEntry* XGradientTable::Remove(long nIndex)
{
    return (XGradientEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XGradientEntry* XGradientTable::GetGradient(long nIndex) const
{
    return (XGradientEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XGradientTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XGradientTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XGradientTable::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 rOut << (long)0;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XGradientEntry* pEntry = (XGradientEntry*)aTable.First();
//BFS01
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XGradient& rGradient = pEntry->GetGradient();
//BFS01     rOut << (long)rGradient.GetGradientStyle();
//BFS01     rOut << rGradient.GetStartColor().GetRed();
//BFS01     rOut << rGradient.GetStartColor().GetGreen();
//BFS01     rOut << rGradient.GetStartColor().GetBlue();
//BFS01     rOut << rGradient.GetEndColor().GetRed();
//BFS01     rOut << rGradient.GetEndColor().GetGreen();
//BFS01     rOut << rGradient.GetEndColor().GetBlue();
//BFS01     rOut << rGradient.GetAngle();
//BFS01     rOut << (ULONG)rGradient.GetBorder();
//BFS01     rOut << (ULONG)rGradient.GetXOffset();
//BFS01     rOut << (ULONG)rGradient.GetYOffset();
//BFS01     pEntry = (XGradientEntry*)aTable.Next();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XGradientTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XGradientEntry* pEntry = NULL;
//BFS01 long        nType;
//BFS01 long        nCount;
//BFS01 long        nIndex;
//BFS01 XubString       aName;
//BFS01
//BFS01 long    nStyle;
//BFS01 USHORT  nRed;
//BFS01 USHORT  nGreen;
//BFS01 USHORT  nBlue;
//BFS01 Color   aStart;
//BFS01 Color   aEnd;
//BFS01 long    nAngle;
//BFS01 ULONG   nBorder;
//BFS01 ULONG   nXOfs;
//BFS01 ULONG   nYOfs;
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
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01
//BFS01         aStart = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01
//BFS01         aEnd = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01
//BFS01         rIn >> nAngle;
//BFS01         rIn >> nBorder;
//BFS01         rIn >> nXOfs;
//BFS01         rIn >> nYOfs;
//BFS01
//BFS01         XGradient aGradient(aStart, aEnd, (XGradientStyle)nStyle, nAngle,
//BFS01                             (USHORT)nXOfs, (USHORT)nYOfs, (USHORT)nBorder);
//BFS01         pEntry = new XGradientEntry (aGradient, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// --------------------
// class XGradientList
// --------------------

/*************************************************************************
|*
|* XGradientList::XGradientList()
|*
*************************************************************************/

XGradientList::XGradientList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList   ( rPath, pInPool, nInitSize, nReSize),
                pVD             ( NULL ),
                pXOut           ( NULL ),
                pXFSet          ( NULL )
{
    pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XGradientList::~XGradientList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
}

/************************************************************************/

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

/************************************************************************/

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return( (XGradientEntry*) XPropertyList::Remove( nIndex, 0 ) );
}

/************************************************************************/

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
}

/************************************************************************/

BOOL XGradientList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

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
//BFS01         SvStream* pStream = aMedium.GetInStream();
//BFS01         if( !pStream )
//BFS01             return( FALSE );
//BFS01
//BFS01         char aCheck[6];
//BFS01         pStream->Read( aCheck, 6 );
//BFS01
//BFS01         // Handelt es sich um die gew"unschte Tabelle?
//BFS01         if( memcmp( aCheck, aChckGradient, sizeof( aChckGradient ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckGradient0, sizeof( aChckGradient0 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XGradientList::Save()
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
        aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckGradient0, 4 );
    pStream->WriteByteString(String( pszChckGradient0, 4 ));

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

BOOL XGradientList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_GRADIENT ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLACK  ),RGB_Color(COL_WHITE  ),XGRAD_LINEAR    ,    0,10,10, 0,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLUE   ),RGB_Color(COL_RED    ),XGRAD_AXIAL     ,  300,20,20,10,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_RED    ),RGB_Color(COL_YELLOW ),XGRAD_RADIAL    ,  600,30,30,20,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_YELLOW ),RGB_Color(COL_GREEN  ),XGRAD_ELLIPTICAL,  900,40,40,30,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_GREEN  ),RGB_Color(COL_MAGENTA),XGRAD_SQUARE    , 1200,50,50,40,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_MAGENTA),RGB_Color(COL_YELLOW ),XGRAD_RECT      , 1900,60,60,50,100,100),aStr));

    return( TRUE );
}

/************************************************************************/

BOOL XGradientList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XGradientList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }

    return( FALSE );
}

/************************************************************************/

Bitmap* XGradientList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    if( !pVD ) // und pXOut und pXFSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XGradientList: Konnte kein VirtualDevice erzeugen!" );
        pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XGradientList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XGradientList: Konnte kein XFillAttrSetItem erzeugen!" );
        pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_GRADIENT ) );
    }

    pXFSet->GetItemSet().Put(
    XFillGradientItem( pXPool, GetGradient( nIndex )->GetGradient() ) );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

    // #73550#
    pXOut->OverrideLineColor( Color( COL_BLACK ) );

    Size aVDSize = pVD->GetOutputSizePixel();
    pXOut->DrawRect( Rectangle( Point(), aVDSize ) );
    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( Point(), aVDSize ) );

    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( bDelete )
    {
        if( pVD )   { delete pVD;   pVD = NULL;     }
        if( pXOut ) { delete pXOut; pXOut = NULL;   }
        if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
    }
    return( pBitmap );
}

/************************************************************************/

//BFS01SvStream& XGradientList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 XGradientEntry* pEntry = NULL;
//BFS01
//BFS01 // Kennung
//BFS01 rOut << (long) -2;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // Versionsverwaltung: Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     pEntry = Get(nIndex);
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XGradient& rGradient = pEntry->GetGradient();
//BFS01     rOut << (long)rGradient.GetGradientStyle();
//BFS01
//BFS01     USHORT nCol = rGradient.GetStartColor().GetRed();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     nCol = rGradient.GetStartColor().GetGreen();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     nCol = rGradient.GetStartColor().GetBlue();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01
//BFS01     nCol = rGradient.GetEndColor().GetRed();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     nCol = rGradient.GetEndColor().GetGreen();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     nCol = rGradient.GetEndColor().GetBlue();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     rOut << rGradient.GetAngle();
//BFS01     rOut << (ULONG)rGradient.GetBorder();
//BFS01     rOut << (ULONG)rGradient.GetXOffset();
//BFS01     rOut << (ULONG)rGradient.GetYOffset();
//BFS01     rOut << (ULONG)rGradient.GetStartIntens();
//BFS01     rOut << (ULONG)rGradient.GetEndIntens();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01XubString& XGradientList::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( USHORT i=0; i<(RID_SVXSTR_GRDT_DEF_END-RID_SVXSTR_GRDT_DEF_START+1) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_GRDT_DEF_START + i );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_GRDT_START + i ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XGradientList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01 XGradientEntry* pEntry = NULL;
//BFS01 long        nCheck;
//BFS01 long        nCount;
//BFS01 XubString       aName;
//BFS01
//BFS01 long    nStyle;
//BFS01 USHORT  nRed;
//BFS01 USHORT  nGreen;
//BFS01 USHORT  nBlue;
//BFS01 Color   aStart;
//BFS01 Color   aEnd;
//BFS01 long    nAngle;
//BFS01 ULONG   nBorder;
//BFS01 ULONG   nXOfs;
//BFS01 ULONG   nYOfs;
//BFS01 ULONG   nStartIntens;
//BFS01 ULONG   nEndIntens;
//BFS01
//BFS01 // Kennung oder Anzahl
//BFS01 rIn >> nCheck;
//BFS01
//BFS01 if( nCheck >= 0 )
//BFS01 {
//BFS01     nCount = nCheck;
//BFS01
//BFS01     for (long nIndex = 0; nIndex < nCount; nIndex++)
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aStart = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aEnd = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01
//BFS01         rIn >> nAngle;
//BFS01         rIn >> nBorder;
//BFS01         rIn >> nXOfs;
//BFS01         rIn >> nYOfs;
//BFS01
//BFS01         nStartIntens = 100L;
//BFS01         nEndIntens = 100L;
//BFS01
//BFS01         XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
//BFS01                              (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
//BFS01                              (USHORT) nStartIntens, (USHORT) nEndIntens );
//BFS01         pEntry = new XGradientEntry (aGradient, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 else if( nCheck == -1L )
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nIndex = 0; nIndex < nCount; nIndex++)
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aStart = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aEnd = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01
//BFS01         rIn >> nAngle;
//BFS01         rIn >> nBorder;
//BFS01         rIn >> nXOfs;
//BFS01         rIn >> nYOfs;
//BFS01
//BFS01         rIn >> nStartIntens;
//BFS01         rIn >> nEndIntens;
//BFS01
//BFS01         XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
//BFS01                              (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
//BFS01                              (USHORT) nStartIntens, (USHORT) nEndIntens );
//BFS01         pEntry = new XGradientEntry (aGradient, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 else // ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nIndex = 0; nIndex < nCount; nIndex++)
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aStart = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         aEnd = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01
//BFS01         rIn >> nAngle;
//BFS01         rIn >> nBorder;
//BFS01         rIn >> nXOfs;
//BFS01         rIn >> nYOfs;
//BFS01
//BFS01         rIn >> nStartIntens;
//BFS01         rIn >> nEndIntens;
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
//BFS01                              (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
//BFS01                              (USHORT) nStartIntens, (USHORT) nEndIntens );
//BFS01         pEntry = new XGradientEntry (aGradient, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// eof
