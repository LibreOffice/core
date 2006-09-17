/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabhtch.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:26:17 $
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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
#include "dlgutil.hxx"

#ifndef _SVX_XFLHTIT_HXX //autogen
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

sal_Unicode const pszExtHatch[]  = {'s','o','h'};

char const aChckHatch[]  = { 0x04, 0x00, 'S','O','H','L'};  // < 5.2
char const aChckHatch0[] = { 0x04, 0x00, 'S','O','H','0'};  // = 5.2
char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };     // = 6.0

// ------------------
// class XHatchTable
// ------------------

/*************************************************************************
|*
|* XHatchTable::XHatchTable()
|*
*************************************************************************/

XHatchTable::XHatchTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XHatchTable::~XHatchTable()
{
}

/************************************************************************/

XHatchEntry* XHatchTable::Replace(long nIndex, XHatchEntry* pEntry )
{
    return (XHatchEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XHatchEntry* XHatchTable::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XHatchEntry* XHatchTable::GetHatch(long nIndex) const
{
    return (XHatchEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XHatchTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XHatchTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XHatchTable::ImpStore( SvStream& rOut )
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
//BFS01 XHatchEntry* pEntry = (XHatchEntry*)aTable.First();;
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XHatch& rHatch = pEntry->GetHatch();
//BFS01     rOut << (long)rHatch.GetHatchStyle();
//BFS01     rOut << rHatch.GetColor().GetRed();
//BFS01     rOut << rHatch.GetColor().GetGreen();
//BFS01     rOut << rHatch.GetColor().GetBlue();
//BFS01     rOut << rHatch.GetDistance();
//BFS01     rOut << rHatch.GetAngle();
//BFS01     pEntry = (XHatchEntry*)aTable.Next();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XHatchTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XHatchEntry* pEntry = NULL;
//BFS01 long        nType;
//BFS01 long        nCount;
//BFS01 long        nIndex;
//BFS01 XubString   aName;
//BFS01
//BFS01 long        nStyle;
//BFS01 USHORT      nRed;
//BFS01 USHORT      nGreen;
//BFS01 USHORT      nBlue;
//BFS01 long        nDistance;
//BFS01 long        nAngle;
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
//BFS01         rIn >> nDistance;
//BFS01         rIn >> nAngle;
//BFS01
//BFS01         Color aColor ( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue);
//BFS01         XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
//BFS01         pEntry = new XHatchEntry (aHatch, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// -----------------
// class XHatchList
// -----------------

/*************************************************************************
|*
|* XHatchList::XHatchList()
|*
*************************************************************************/

XHatchList::XHatchList( const String& rPath,
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

XHatchList::~XHatchList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
}

/************************************************************************/

XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
{
    return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XHatchEntry* XHatchList::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XHatchEntry* XHatchList::GetHatch(long nIndex) const
{
    return (XHatchEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XHatchList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

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
//BFS01         if( memcmp( aCheck, aChckHatch, sizeof( aChckHatch ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckHatch0, sizeof( aChckHatch0 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XHatchList::Save()
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
        aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckHatch0, 4 );
    pStream->WriteByteString(String( pszChckHatch0, 4 ));

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

BOOL XHatchList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_HATCH ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLACK),XHATCH_SINGLE,100,  0),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_RED  ),XHATCH_DOUBLE, 80,450),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLUE ),XHATCH_TRIPLE,120,  0),aStr));

    return( TRUE );
}

/************************************************************************/

BOOL XHatchList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XHatchList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }

    return( TRUE );
}

/************************************************************************/

Bitmap* XHatchList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    Point   aZero;

    if( !pVD ) // und pXOut und pXFSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XHatchList: Konnte kein VirtualDevice erzeugen!" );
        //pVD->SetMapMode( MAP_100TH_MM );
        //pVD->SetOutputSize( pVD->PixelToLogic( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) ) );
        pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XHatchList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XHatchList: Konnte kein XFillAttrSetItem erzeugen!" );
    }

    if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() != 0 )
        pVD->SetDrawMode( OUTPUT_DRAWMODE_CONTRAST );
    else
        pVD->SetDrawMode( OUTPUT_DRAWMODE_COLOR );

    // Damit die Schraffuren mit Rahmen angezeigt werden:
    // MapMode-Aenderungen (100th mm <--> Pixel)
    Size aPixelSize = pVD->GetOutputSizePixel();
    pVD->SetMapMode( MAP_PIXEL );

    pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
    pXFSet->GetItemSet().Put( XFillColorItem( String(), RGB_Color( COL_WHITE ) ) );

//-/    pXOut->SetFillAttr( *pXFSet );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

    // #73550#
    pXOut->OverrideLineColor( Color( COL_BLACK ) );

    pXOut->DrawRect( Rectangle( aZero, aPixelSize ) );

    pVD->SetMapMode( MAP_100TH_MM );
    Size aVDSize = pVD->GetOutputSize();
    // 1 Pixel (Rahmen) abziehen
    aVDSize.Width() -= (long) ( aVDSize.Width() / aPixelSize.Width() + 1 );
    aVDSize.Height() -= (long) ( aVDSize.Height() / aPixelSize.Height() + 1 );

    pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_HATCH ) );
    pXFSet->GetItemSet().Put( XFillHatchItem( String(), GetHatch( nIndex )->GetHatch() ) );

//-/    pXOut->SetFillAttr( *pXFSet );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

    pXOut->DrawRect( Rectangle( aZero, aVDSize ) );

    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, pVD->GetOutputSize() ) );

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

//BFS01SvStream& XHatchList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Version statt Anzahl, um auch alte Tabellen zu lesen
//BFS01 rOut << (long) -1;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XHatchEntry* pEntry = NULL;
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
//BFS01     XHatch& rHatch = pEntry->GetHatch();
//BFS01     rOut << (long)rHatch.GetHatchStyle();
//BFS01     USHORT nCol = rHatch.GetColor().GetRed();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01
//BFS01     nCol = rHatch.GetColor().GetGreen();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01
//BFS01     nCol = rHatch.GetColor().GetBlue();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     rOut << rHatch.GetDistance();
//BFS01     rOut << rHatch.GetAngle();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01XubString& XHatchList::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( USHORT i=0; i<(RID_SVXSTR_HATCH_DEF_END-RID_SVXSTR_HATCH_DEF_START+1) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_HATCH_DEF_START + i );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_HATCH_START + i ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XHatchList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01 XHatchEntry* pEntry = NULL;
//BFS01 long        nCount;
//BFS01 XubString       aName;
//BFS01
//BFS01 long        nStyle;
//BFS01 USHORT      nRed;
//BFS01 USHORT      nGreen;
//BFS01 USHORT      nBlue;
//BFS01 long        nDistance;
//BFS01 long        nAngle;
//BFS01 Color       aColor;
//BFS01
//BFS01 rIn >> nCount;
//BFS01
//BFS01 if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
//BFS01 {
//BFS01     for( long nIndex = 0; nIndex < nCount; nIndex++ )
//BFS01     {
//BFS01         // UNICODE:rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01         rIn >> nDistance;
//BFS01         rIn >> nAngle;
//BFS01
//BFS01         aColor = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
//BFS01         pEntry = new XHatchEntry (aHatch, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 else // ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01
//BFS01     for( long nIndex = 0; nIndex < nCount; nIndex++ )
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
//BFS01         rIn >> nDistance;
//BFS01         rIn >> nAngle;
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         aColor = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
//BFS01         pEntry = new XHatchEntry (aHatch, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// eof
