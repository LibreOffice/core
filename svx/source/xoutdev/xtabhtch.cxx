/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xtabhtch.cxx,v $
 * $Revision: 1.21 $
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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xoutx.hxx>
#include "dlgutil.hxx"
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

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

        uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( FALSE );
}

/************************************************************************/

BOOL XHatchList::Save()
{
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

// eof
