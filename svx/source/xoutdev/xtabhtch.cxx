/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include "svx/dlgutil.hxx"
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/xlnclit.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

sal_Unicode const pszExtHatch[]  = {'s','o','h'};

char const aChckHatch[]  = { 0x04, 0x00, 'S','O','H','L'};  // < 5.2
char const aChckHatch0[] = { 0x04, 0x00, 'S','O','H','0'};  // = 5.2
char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };     // = 6.0

// -----------------
// class XHatchList
// -----------------

class impXHatchList
{
private:
    VirtualDevice*          mpVirtualDevice;
    SdrModel*               mpSdrModel;
    SdrObject*              mpBackgroundObject;
    SdrObject*              mpHatchObject;

public:
    impXHatchList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB, SdrObject* pH)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB),
        mpHatchObject(pH)
    {}

    ~impXHatchList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        SdrObject::Free(mpHatchObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
    SdrObject* getHatchObject() const { return mpHatchObject; }
};

void XHatchList::impCreate()
{
    if(!mpData)
    {
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
        OSL_ENSURE(0 != pVirDev, "XDashList: no VirtualDevice created!" );
        pVirDev->SetMapMode(MAP_100TH_MM);
        const Size aSize(pVirDev->PixelToLogic(Size(getUiBitmapWidth(), getUiBitmapHeight())));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        pVirDev->SetBackground(rStyleSettings.GetFieldColor());

        SdrModel* pSdrModel = new SdrModel();
        OSL_ENSURE(0 != pSdrModel, "XDashList: no SdrModel created!" );
        pSdrModel->GetItemPool().FreezeIdRanges();

        const Size aSinglePixel(pVirDev->PixelToLogic(Size(1, 1)));
        const Rectangle aBackgroundSize(aZero, Size(aSize.getWidth() - aSinglePixel.getWidth(), aSize.getHeight() - aSinglePixel.getHeight()));
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pBackgroundObject, "XDashList: no BackgroundObject created!" );
        pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
        pBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        pBackgroundObject->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));

        SdrObject* pHatchObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pHatchObject, "XDashList: no HatchObject created!" );
        pHatchObject->SetModel(pSdrModel);
        pHatchObject->SetMergedItem(XFillStyleItem(XFILL_HATCH));
        pHatchObject->SetMergedItem(XLineStyleItem(XLINE_NONE));

        mpData = new impXHatchList(pVirDev, pSdrModel, pBackgroundObject, pHatchObject);
        OSL_ENSURE(0 != mpData, "XDashList: data creation went wrong!" );
    }
}

void XHatchList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XHatchList::XHatchList(const String& rPath, XOutdevItemPool* pInPool)
:   XPropertyList(rPath, pInPool),
    mpData(0)
{
}

XHatchList::~XHatchList()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
{
    return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XHatchEntry* XHatchList::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyList::Remove(nIndex, 0);
}

XHatchEntry* XHatchList::GetHatch(long nIndex) const
{
    return (XHatchEntry*) XPropertyList::Get(nIndex, 0);
}

sal_Bool XHatchList::Load()
{
    if( mbListDirty )
    {
        mbListDirty = false;

        INetURLObject aURL( maPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !maPath.Len(), "invalid URL" );
            return sal_False;
        }

        aURL.Append( maName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

sal_Bool XHatchList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !maPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( maName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XHatchList::Create()
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

    return( sal_True );
}

Bitmap XHatchList::CreateBitmapForUI( long nIndex )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pHatchObject = mpData->getHatchObject();

    pHatchObject->SetMergedItem(XFillStyleItem(XFILL_HATCH));
    pHatchObject->SetMergedItem(XFillHatchItem(String(), GetHatch(nIndex)->GetHatch()));

    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(mpData->getBackgroundObject());
    aObjectVector.push_back(pHatchObject);
    sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
    sdr::contact::DisplayInfo aDisplayInfo;

    pVD->Erase();
    aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    return pVD->GetBitmap(aZero, pVD->GetOutputSize());
}

//////////////////////////////////////////////////////////////////////////////
// eof
