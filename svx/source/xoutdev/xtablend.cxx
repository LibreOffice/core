/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xtablend.cxx,v $
 * $Revision: 1.19 $
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

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xoutx.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtLineEnd[]   = {'s','o','e'};

static char const aChckLEnd[]  = { 0x04, 0x00, 'S','O','E','L'};    // < 5.2
static char const aChckLEnd0[] = { 0x04, 0x00, 'S','O','E','0'};    // = 5.2
static char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };       // = 6.0

// --------------------
// class XLineEndTable
// --------------------

/*************************************************************************
|*
|* XLineEndTable::XLineEndTable()
|*
*************************************************************************/

XLineEndTable::XLineEndTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XLineEndTable::~XLineEndTable()
{
}

/************************************************************************/

XLineEndEntry* XLineEndTable::Replace(long nIndex, XLineEndEntry* pEntry )
{
    return (XLineEndEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XLineEndEntry* XLineEndTable::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XLineEndEntry* XLineEndTable::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XLineEndTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndTable::Create()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XLineEndTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

BOOL XLineEndTable::CreateBitmapsForUI()
{
    return( FALSE );
}

// --------------------
// class XLineEndList
// --------------------

/*************************************************************************
|*
|* XLineEndList::XLineEndList()
|*
*************************************************************************/

XLineEndList::XLineEndList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize),
                pVD             ( NULL ),
                pXOut           ( NULL ),
                pXFSet          ( NULL ),
                pXLSet          ( NULL )
{
    pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XLineEndList::~XLineEndList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
    if( pXLSet ) delete pXLSet;
}

/************************************************************************/

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
    return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XLineEndList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

/************************************************************************/

BOOL XLineEndList::Create()
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aTriangle), SVX_RESSTR( RID_SVXSTR_ARROW ) ) );

    basegfx::B2DPolygon aSquare;
    aSquare.append(basegfx::B2DPoint(0.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 10.0));
    aSquare.append(basegfx::B2DPoint(0.0, 10.0));
    aSquare.setClosed(true);
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aSquare), SVX_RESSTR( RID_SVXSTR_SQUARE ) ) );

    basegfx::B2DPolygon aCircle(basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 100.0));
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aCircle), SVX_RESSTR( RID_SVXSTR_CIRCLE ) ) );

    return( TRUE );
}

/************************************************************************/

BOOL XLineEndList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XLineEndList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
    if( pXLSet ){ delete pXLSet; pXLSet = NULL; }

    return( TRUE );
}

/************************************************************************/

Bitmap* XLineEndList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    Point   aZero;
    Size    aVDSize;

    if( !pVD ) // und pXOut und pXFSet und pXLSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein VirtualDevice erzeugen!" );
        pVD->SetMapMode( MAP_100TH_MM );
        aVDSize = pVD->PixelToLogic( Size( BITMAP_WIDTH * 2, BITMAP_HEIGHT ) );
        pVD->SetOutputSize( aVDSize );

        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        pVD->SetFillColor( rStyles.GetFieldColor() );
        pVD->SetLineColor( rStyles.GetFieldColor() );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XFillAttrSetItem erzeugen!" );
        pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
        const StyleSettings& rStyleSettings = pVD->GetSettings().GetStyleSettings();
        pXFSet->GetItemSet().Put( XFillColorItem( String(), rStyleSettings.GetFieldColor() ) );

        pXLSet = new XLineAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XLineAttrSetItem erzeugen!" );
        pXLSet->GetItemSet().Put( XLineStartWidthItem( aVDSize.Height() ) );
        pXLSet->GetItemSet().Put( XLineEndWidthItem( aVDSize.Height() ) );
        pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( rStyles.GetFieldTextColor().GetColor() ) ) );

    }
    else
        aVDSize = pVD->GetOutputSize();

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
//-/    pXOut->SetLineAttr( *pXLSet );

    pXOut->SetLineAttr( pXLSet->GetItemSet() );
//-/    pXOut->SetFillAttr( *pXFSet );

    pXOut->SetFillAttr( pXFSet->GetItemSet() );
    pXOut->DrawRect( Rectangle( aZero, aVDSize ) );

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_SOLID ) );
    pXLSet->GetItemSet().Put( XLineStartItem( String(), GetLineEnd( nIndex )->GetLineEnd() ) );
    pXLSet->GetItemSet().Put( XLineEndItem( String(), GetLineEnd( nIndex )->GetLineEnd() ) );

//-/    pXOut->SetLineAttr( *pXLSet );
    pXOut->SetLineAttr( pXLSet->GetItemSet() );

    pXOut->DrawLine( Point( 0, aVDSize.Height() / 2 ),
                     Point( aVDSize.Width(), aVDSize.Height() / 2 ) );

    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, aVDSize ) );

    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( bDelete )
    {
        if( pVD )   { delete pVD;   pVD = NULL;     }
        if( pXOut ) { delete pXOut; pXOut = NULL;   }
        if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
        if( pXLSet ){ delete pXLSet; pXLSet = NULL; }
    }

    return( pBitmap );
}

// eof
