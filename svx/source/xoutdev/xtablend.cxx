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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>

#include <vcl/svapp.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
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

#include <svx/svdorect.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;

using ::rtl::OUString;

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
                            sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
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

sal_Bool XLineEndTable::Load()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XLineEndTable::Save()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XLineEndTable::Create()
{
    return( sal_False );
}

/************************************************************************/

Bitmap* XLineEndTable::CreateBitmapForUI( long /*nIndex*/, sal_Bool /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

sal_Bool XLineEndTable::CreateBitmapsForUI()
{
    return( sal_False );
}

// --------------------
// class XLineEndList
// --------------------

class impXLineEndList
{
private:
    VirtualDevice*          mpVirtualDevice;
    SdrModel*               mpSdrModel;
    SdrObject*              mpBackgroundObject;
    SdrObject*              mpLineObject;

public:
    impXLineEndList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB, SdrObject* pL)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB),
        mpLineObject(pL)
    {}

    ~impXLineEndList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        SdrObject::Free(mpLineObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
    SdrObject* getLineObject() const { return mpLineObject; }
};

void XLineEndList::impCreate()
{
    if(!mpData)
    {
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
        OSL_ENSURE(0 != pVirDev, "XLineEndList: no VirtualDevice created!" );
        pVirDev->SetMapMode(MAP_100TH_MM);
        const Size aSize(pVirDev->PixelToLogic(Size(BITMAP_WIDTH * 2, BITMAP_HEIGHT)));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        SdrModel* pSdrModel = new SdrModel();
        OSL_ENSURE(0 != pSdrModel, "XLineEndList: no SdrModel created!" );
        pSdrModel->GetItemPool().FreezeIdRanges();

        const Rectangle aBackgroundSize(aZero, aSize);
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pBackgroundObject, "XLineEndList: no BackgroundObject created!" );
        pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_NONE));
        pBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));

        const basegfx::B2DPoint aStart(0, aSize.Height() / 2);
        const basegfx::B2DPoint aEnd(aSize.Width(), aSize.Height() / 2);
        basegfx::B2DPolygon aPolygon;
        aPolygon.append(aStart);
        aPolygon.append(aEnd);
        SdrObject* pLineObject = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPolygon));
        OSL_ENSURE(0 != pLineObject, "XLineEndList: no LineObject created!" );
        pLineObject->SetModel(pSdrModel);
        pLineObject->SetMergedItem(XLineStartWidthItem(aSize.Height()));
        pLineObject->SetMergedItem(XLineEndWidthItem(aSize.Height()));
        pLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));

        mpData = new impXLineEndList(pVirDev, pSdrModel, pBackgroundObject, pLineObject);
        OSL_ENSURE(0 != mpData, "XLineEndList: data creation went wrong!" );
    }
}

void XLineEndList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XLineEndList::XLineEndList(const String& rPath, XOutdevItemPool* _pXPool, sal_uInt16 nInitSize, sal_uInt16 nReSize)
:   XPropertyList(rPath, _pXPool, nInitSize, nReSize),
    mpData(0)
{
    pBmpList = new List(nInitSize, nReSize);
}

XLineEndList::~XLineEndList()
{
    impDestroy();
}

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
    return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyList::Remove(nIndex, 0);
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

sal_Bool XLineEndList::Load()
{
    if( bListDirty )
    {
        bListDirty = sal_False;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return sal_False;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

sal_Bool XLineEndList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XLineEndList::Create()
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

    return( sal_True );
}

sal_Bool XLineEndList::CreateBitmapsForUI()
{
    impCreate();

    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, sal_False );
        OSL_ENSURE(0 != pBmp, "XLineEndList: Bitmap(UI) could not be created!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }

    impDestroy();

    return( sal_True );
}

Bitmap* XLineEndList::CreateBitmapForUI( long nIndex, sal_Bool bDelete )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pLine = mpData->getLineObject();

    pLine->SetMergedItem(XLineStyleItem(XLINE_SOLID));
    pLine->SetMergedItem(XLineStartItem(String(), GetLineEnd(nIndex)->GetLineEnd()));
    pLine->SetMergedItem(XLineEndItem(String(), GetLineEnd(nIndex)->GetLineEnd()));

    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(mpData->getBackgroundObject());
    aObjectVector.push_back(pLine);
    sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    Bitmap* pBitmap = new Bitmap(pVD->GetBitmap(aZero, pVD->GetOutputSize()));

    if(bDelete)
    {
        impDestroy();
    }

    return pBitmap;
}

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
