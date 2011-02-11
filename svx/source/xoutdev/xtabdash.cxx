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

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif
#include <vcl/svapp.hxx>

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflclit.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace com::sun::star;
using namespace rtl;

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[]  = {'s','o','d'};
char const aChckDash[]  = { 0x04, 0x00, 'S','O','D','L'};   // < 5.2
char const aChckDash0[] = { 0x04, 0x00, 'S','O','D','0'};   // = 5.2
char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };      // = 6.0

// -----------------
// class XDashTable
// -----------------

/*************************************************************************
|*
|* XDashTable::XDashTable()
|*
*************************************************************************/

XDashTable::XDashTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XDashTable::~XDashTable()
{
}

/************************************************************************/

XDashEntry* XDashTable::Replace(long nIndex, XDashEntry* pEntry )
{
    return (XDashEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XDashEntry* XDashTable::Remove(long nIndex)
{
    return (XDashEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XDashEntry* XDashTable::GetDash(long nIndex) const
{
    return (XDashEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

sal_Bool XDashTable::Load()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XDashTable::Save()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XDashTable::Create()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XDashTable::CreateBitmapsForUI()
{
    return( sal_False );
}

/************************************************************************/

Bitmap* XDashTable::CreateBitmapForUI( long /*nIndex*/, sal_Bool /*bDelete*/)
{
    return( NULL );
}

// ----------------
// class XDashList
// ----------------

class impXDashList
{
private:
    VirtualDevice*          mpVirtualDevice;
    SdrModel*               mpSdrModel;
    SdrObject*              mpBackgroundObject;
    SdrObject*              mpLineObject;

public:
    impXDashList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB, SdrObject* pL)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB),
        mpLineObject(pL)
    {}

    ~impXDashList()
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

void XDashList::impCreate()
{
    if(!mpData)
    {
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
        OSL_ENSURE(0 != pVirDev, "XDashList: no VirtualDevice created!" );
        pVirDev->SetMapMode(MAP_100TH_MM);
        const Size aSize(pVirDev->PixelToLogic(Size(BITMAP_WIDTH * 2, BITMAP_HEIGHT)));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        SdrModel* pSdrModel = new SdrModel();
        OSL_ENSURE(0 != pSdrModel, "XDashList: no SdrModel created!" );
        pSdrModel->GetItemPool().FreezeIdRanges();

        const Rectangle aBackgroundSize(aZero, aSize);
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pBackgroundObject, "XDashList: no BackgroundObject created!" );
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
        OSL_ENSURE(0 != pLineObject, "XDashList: no LineObject created!" );
        pLineObject->SetModel(pSdrModel);
        pLineObject->SetMergedItem(XLineStyleItem(XLINE_DASH));
        pLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));
        pLineObject->SetMergedItem(XLineWidthItem(30));

        mpData = new impXDashList(pVirDev, pSdrModel, pBackgroundObject, pLineObject);
        OSL_ENSURE(0 != mpData, "XDashList: data creation went wrong!" );
    }
}

void XDashList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XDashList::XDashList(const String& rPath, XOutdevItemPool* pInPool, sal_uInt16 nInitSize, sal_uInt16 nReSize)
:   XPropertyList(rPath, pInPool, nInitSize, nReSize),
    mpData(0)
{
    pBmpList = new List(nInitSize, nReSize);
}

XDashList::~XDashList()
{
    impDestroy();
}

XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
{
    return (XDashEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XDashEntry* XDashList::Remove(long nIndex)
{
    return (XDashEntry*) XPropertyList::Remove(nIndex, 0);
}

XDashEntry* XDashList::GetDash(long nIndex) const
{
    return (XDashEntry*) XPropertyList::Get(nIndex, 0);
}

sal_Bool XDashList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

sal_Bool XDashList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XDashList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_LINESTYLE ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XDashEntry(XDash(XDASH_RECT,1, 50,1, 50, 50),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XDashEntry(XDash(XDASH_RECT,1,500,1,500,500),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XDashEntry(XDash(XDASH_RECT,2, 50,3,250,120),aStr));

    return( sal_True );
}

sal_Bool XDashList::CreateBitmapsForUI()
{
    impCreate();

    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, sal_False );
        DBG_ASSERT( pBmp, "XDashList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }

    impDestroy();

    return( sal_True );
}

Bitmap* XDashList::CreateBitmapForUI( long nIndex, sal_Bool bDelete )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pLine = mpData->getLineObject();

    pLine->SetMergedItem(XLineStyleItem(XLINE_DASH));
    pLine->SetMergedItem(XLineDashItem(String(), GetDash(nIndex)->GetDash()));

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
