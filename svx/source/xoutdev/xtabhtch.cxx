/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "svx/XPropertyTable.hxx"
#include <vcl/svapp.hxx>

#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
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
        const Size aSize(pVirDev->PixelToLogic(rStyleSettings.GetListBoxPreviewDefaultPixelSize()));
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
    delete mpData;
    mpData = NULL;
}

XHatchList::XHatchList(const String& rPath, XOutdevItemPool* pInPool)
  : XPropertyList( XHATCH_LIST, rPath, pInPool ),
    mpData(0)
{
}

XHatchList::~XHatchList()
{
    delete mpData;
    mpData = NULL;
}

XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
{
    return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XHatchEntry* XHatchList::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyList::Remove(nIndex);
}

XHatchEntry* XHatchList::GetHatch(long nIndex) const
{
    return (XHatchEntry*) XPropertyList::Get(nIndex, 0);
}

uno::Reference< container::XNameContainer > XHatchList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
