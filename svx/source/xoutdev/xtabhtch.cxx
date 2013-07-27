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
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xpool.hxx>

#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

XHatchList::XHatchList(const String& rPath)
  : XPropertyList( XHATCH_LIST, rPath )
{
}

XHatchList::~XHatchList()
{
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
    return (XHatchEntry*) XPropertyList::Get(nIndex);
}

uno::Reference< container::XNameContainer > XHatchList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
}

bool XHatchList::Create()
{
    XubString aStr( SVX_RESSTR( RID_SVXSTR_HATCH ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLACK),XHATCH_SINGLE,100,  0),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_RED  ),XHATCH_DOUBLE, 80,450),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLUE ),XHATCH_TRIPLE,120,  0),aStr));

    return true;
}

Bitmap XHatchList::CreateBitmapForUI( long nIndex )
{
    Bitmap aRetval;
    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

        // prepare polygon geometry for rectangle
        const basegfx::B2DPolygon aRectangle(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, rSize.Width(), rSize.Height())));

        const XHatch& rHatch = GetHatch(nIndex)->GetHatch();
        drawinglayer::attribute::HatchStyle aHatchStyle(drawinglayer::attribute::HATCHSTYLE_TRIPLE);

        switch(rHatch.GetHatchStyle())
        {
            case XHATCH_SINGLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_SINGLE;
                break;
            }
            case XHATCH_DOUBLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_DOUBLE;
                break;
            }
            default :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_TRIPLE; // XHATCH_TRIPLE
                break;
            }
        }

        const basegfx::B2DHomMatrix aScaleMatrix(OutputDevice::LogicToLogic(MAP_100TH_MM, MAP_PIXEL));
        const basegfx::B2DVector aScaleVector(aScaleMatrix * basegfx::B2DVector(1.0, 0.0));
        const double fScaleValue(aScaleVector.getLength());

        const drawinglayer::attribute::FillHatchAttribute aFillHatch(
            aHatchStyle,
            (double)rHatch.GetDistance() * fScaleValue,
            (double)rHatch.GetAngle() * F_PI1800,
            rHatch.GetColor().getBColor(),
            3, // same default as VCL, a minimum of three discrete units (pixels) offset
            false);

        const basegfx::BColor aBlack(0.0, 0.0, 0.0);
        const drawinglayer::primitive2d::Primitive2DReference aHatchPrimitive(
            new drawinglayer::primitive2d::PolyPolygonHatchPrimitive2D(
                basegfx::B2DPolyPolygon(aRectangle),
                aBlack,
                aFillHatch));

        const drawinglayer::primitive2d::Primitive2DReference aBlackRectanglePrimitive(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aRectangle,
                aBlack));

        // prepare VirtualDevice
        VirtualDevice aVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        aVirtualDevice.SetOutputSizePixel(rSize);
        aVirtualDevice.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        if(rStyleSettings.GetPreviewUsesCheckeredBackground())
        {
            const Point aNull(0, 0);
            static const sal_uInt32 nLen(8);
            static const Color aW(COL_WHITE);
            static const Color aG(0xef, 0xef, 0xef);
            aVirtualDevice.DrawCheckered(aNull, rSize, nLen, aW, aG);
        }
        else
        {
            aVirtualDevice.SetBackground(rStyleSettings.GetFieldColor());
            aVirtualDevice.Erase();
        }

        // create processor and draw primitives
        drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            aVirtualDevice,
            aNewViewInformation2D);

        if(pProcessor2D)
        {
            drawinglayer::primitive2d::Primitive2DSequence aSequence(2);

            aSequence[0] = aHatchPrimitive;
            aSequence[1] = aBlackRectanglePrimitive;
            pProcessor2D->process(aSequence);
            delete pProcessor2D;
        }

        // get result bitmap and scale
        aRetval = aVirtualDevice.GetBitmap(Point(0, 0), aVirtualDevice.GetOutputSizePixel());
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
