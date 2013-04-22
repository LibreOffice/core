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

#include <vcl/svapp.hxx>
#include <svl/itemset.hxx>

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
#include <svx/xlnwtit.hxx>

using namespace com::sun::star;

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
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
        const Size aSize(pVirDev->PixelToLogic(Size(rSize.Width() * 2, rSize.Height())));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        pVirDev->SetBackground(rStyleSettings.GetFieldColor());

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
        const basegfx::B2DPoint aEnd(aSize.Width() - 1, aSize.Height() / 2);
        basegfx::B2DPolygon aPolygon;
        aPolygon.append(aStart);
        aPolygon.append(aEnd);
        SdrObject* pLineObject = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPolygon));
        OSL_ENSURE(0 != pLineObject, "XLineEndList: no LineObject created!" );
        pLineObject->SetModel(pSdrModel);
        const Size aLineWidth(pVirDev->PixelToLogic(Size(rStyleSettings.GetListBoxPreviewDefaultLineWidth(), 0)));
        pLineObject->SetMergedItem(XLineWidthItem(aLineWidth.getWidth()));
        const sal_uInt32 nArrowHeight((aSize.Height() * 8) / 10);
        pLineObject->SetMergedItem(XLineStartWidthItem(nArrowHeight));
        pLineObject->SetMergedItem(XLineEndWidthItem(nArrowHeight));
        pLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));

        mpData = new impXLineEndList(pVirDev, pSdrModel, pBackgroundObject, pLineObject);
        OSL_ENSURE(0 != mpData, "XLineEndList: data creation went wrong!" );
    }
}

void XLineEndList::impDestroy()
{
    delete mpData;
    mpData = NULL;
}

XLineEndList::XLineEndList( const String& rPath, XOutdevItemPool* _pXPool )
    : XPropertyList( XLINE_END_LIST, rPath, _pXPool ),
      mpData(NULL)
{
}

XLineEndList::~XLineEndList()
{
    impDestroy();
}

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyList::Remove(nIndex);
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

uno::Reference< container::XNameContainer > XLineEndList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
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

    return sal_True;
}

Bitmap XLineEndList::CreateBitmapForUI( long nIndex )
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

    pVD->Erase();
    aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    return pVD->GetBitmap(aZero, pVD->GetOutputSize());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
