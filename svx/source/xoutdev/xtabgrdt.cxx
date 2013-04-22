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

#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xgrscit.hxx>

using namespace com::sun::star;

class impXGradientList
{
private:
    VirtualDevice*          mpVirtualDevice;
    SdrModel*               mpSdrModel;
    SdrObject*              mpBackgroundObject;

public:
    impXGradientList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB)
    {}

    ~impXGradientList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
};

void XGradientList::impCreate()
{
    if(!mpData)
    {
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
        OSL_ENSURE(0 != pVirDev, "XGradientList: no VirtualDevice created!" );
        pVirDev->SetMapMode(MAP_100TH_MM);
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
        const Size aSize(pVirDev->PixelToLogic(rSize));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        pVirDev->SetBackground(rStyleSettings.GetFieldColor());

        SdrModel* pSdrModel = new SdrModel();
        OSL_ENSURE(0 != pSdrModel, "XGradientList: no SdrModel created!" );
        pSdrModel->GetItemPool().FreezeIdRanges();

        const Size aSinglePixel(pVirDev->PixelToLogic(Size(1, 1)));
        const Rectangle aBackgroundSize(aZero, Size(aSize.getWidth() - aSinglePixel.getWidth(), aSize.getHeight() - aSinglePixel.getHeight()));
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pBackgroundObject, "XGradientList: no BackgroundObject created!" );
        pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        pBackgroundObject->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));
        pBackgroundObject->SetMergedItem(XGradientStepCountItem(sal_uInt16((rSize.Width() + rSize.Height()) / 3)));

        mpData = new impXGradientList(pVirDev, pSdrModel, pBackgroundObject);
        OSL_ENSURE(0 != mpData, "XGradientList: data creation went wrong!" );
    }
}

void XGradientList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XGradientList::XGradientList( const String& rPath, XOutdevItemPool* pInPool )
:   XPropertyList( XGRADIENT_LIST, rPath, pInPool ),
    mpData(0)
{
}

XGradientList::~XGradientList()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return( (XGradientEntry*) XPropertyList::Remove( nIndex ) );
}

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
}

uno::Reference< container::XNameContainer > XGradientList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXGradientTable_createInstance( this ),
        uno::UNO_QUERY );
}

sal_Bool XGradientList::Create()
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

    return( sal_True );
}

Bitmap XGradientList::CreateBitmapForUI( long nIndex )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pBackgroundObject = mpData->getBackgroundObject();

    pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
    pBackgroundObject->SetMergedItem(XFillGradientItem(GetGradient(nIndex)->GetGradient()));

    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(pBackgroundObject);
    sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
    sdr::contact::DisplayInfo aDisplayInfo;

    pVD->Erase();
    aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    return pVD->GetBitmap(aZero, pVD->GetOutputSize());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
