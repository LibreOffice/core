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

#include <XPropertyTable.hxx>

#include <vcl/virdev.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <memory>

using namespace com::sun::star;

XGradientList::XGradientList( const OUString& rPath, const OUString& rReferer )
:   XPropertyList( XPropertyListType::Gradient, rPath, rReferer )
{
}

XGradientList::~XGradientList()
{
}

void XGradientList::Replace(std::unique_ptr<XGradientEntry> pEntry, tools::Long nIndex)
{
    XPropertyList::Replace(std::move(pEntry), nIndex);
}

XGradientEntry* XGradientList::GetGradient(tools::Long nIndex) const
{
    return static_cast<XGradientEntry*>( XPropertyList::Get( nIndex ) );
}

uno::Reference< container::XNameContainer > XGradientList::createInstance()
{
    return SvxUnoXGradientTable_createInstance( *this );
}

bool XGradientList::Create()
{
    OUStringBuffer aStr(SvxResId(RID_SVXSTR_GRADIENT) + " 1");
    sal_Int32 nLen = aStr.getLength() - 1;

    // XGradient() default already creates [COL_BLACK, COL_WHITE] as defaults
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(),aStr.toString()));

    aStr[nLen] = '2';
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(basegfx::BColorStops(COL_BLUE.getBColor(),    COL_RED.getBColor()),     css::awt::GradientStyle_AXIAL     ,  300_deg10,20,20,10,100,100),aStr.toString()));
    aStr[nLen] = '3';
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(basegfx::BColorStops(COL_RED.getBColor(),     COL_YELLOW.getBColor()),  css::awt::GradientStyle_RADIAL    ,  600_deg10,30,30,20,100,100),aStr.toString()));
    aStr[nLen] = '4';
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(basegfx::BColorStops(COL_YELLOW.getBColor(),  COL_GREEN.getBColor()),   css::awt::GradientStyle_ELLIPTICAL,  900_deg10,40,40,30,100,100),aStr.toString()));
    aStr[nLen] = '5';
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(basegfx::BColorStops(COL_GREEN.getBColor(),   COL_MAGENTA.getBColor()), css::awt::GradientStyle_SQUARE    , 1200_deg10,50,50,40,100,100),aStr.toString()));
    aStr[nLen] = '6';
    Insert(std::make_unique<XGradientEntry>(basegfx::BGradient(basegfx::BColorStops(COL_MAGENTA.getBColor(), COL_YELLOW.getBColor()),  css::awt::GradientStyle_RECT      , 1900_deg10,60,60,50,100,100),aStr.toString()));

    return true;
}

Bitmap XGradientList::CreateBitmap( tools::Long nIndex, const Size& rSize ) const
{
    Bitmap aRetval;

    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        // prepare polygon geometry for rectangle
        basegfx::B2DPolygon aRectangle(
            basegfx::utils::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, rSize.Width(), rSize.Height())));

        const basegfx::BGradient& rGradient = GetGradient(nIndex)->GetGradient();
        basegfx::BColorStops aColorStops(rGradient.GetColorStops());

        if (rGradient.GetStartIntens() != 100 || rGradient.GetEndIntens() != 100)
        {
            // Need to do the (old, crazy) blend against black
            aColorStops.blendToIntensity(
                    rGradient.GetStartIntens() * 0.01,
                rGradient.GetEndIntens() * 0.01,
                basegfx::BColor()); // COL_BLACK
        }

        drawinglayer::attribute::FillGradientAttribute aFillGradient(
            rGradient.GetGradientStyle(),
            static_cast<double>(rGradient.GetBorder()) * 0.01,
            static_cast<double>(rGradient.GetXOffset()) * 0.01,
            static_cast<double>(rGradient.GetYOffset()) * 0.01,
            toRadians(rGradient.GetAngle()),
            aColorStops);

        const drawinglayer::primitive2d::Primitive2DReference aGradientPrimitive(
            new drawinglayer::primitive2d::PolyPolygonGradientPrimitive2D(
                basegfx::B2DPolyPolygon(aRectangle),
                std::move(aFillGradient)));

        const basegfx::BColor aBlack(0.0, 0.0, 0.0);
        const drawinglayer::primitive2d::Primitive2DReference aBlackRectanglePrimitive(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                std::move(aRectangle),
                aBlack));

        // prepare VirtualDevice
        ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        pVirtualDevice->SetOutputSizePixel(rSize);
        pVirtualDevice->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient
            : DrawModeFlags::Default);

        // create processor and draw primitives
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            *pVirtualDevice,
            aNewViewInformation2D));

        drawinglayer::primitive2d::Primitive2DContainer aSequence {
            aGradientPrimitive,
            aBlackRectanglePrimitive };

        pProcessor2D->process(aSequence);
        pProcessor2D.reset();

        // get result bitmap and scale
        aRetval = pVirtualDevice->GetBitmap(Point(0, 0), pVirtualDevice->GetOutputSizePixel());
    }

    return aRetval;
}

Bitmap XGradientList::CreateBitmapForUI(tools::Long nIndex)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    return CreateBitmap(nIndex, rSize);
}

Bitmap XGradientList::GetBitmapForPreview(tools::Long nIndex, const Size& rSize)
{
    return CreateBitmap(nIndex, rSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
