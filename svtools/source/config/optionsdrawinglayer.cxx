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

#include <svtools/optionsdrawinglayer.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <officecfg/Office/Common.hxx>
#include <mutex>

// #i73602#
// #i74769#, #i75172#
// #i4219#

namespace SvtOptionsDrawinglayer
{

bool IsOverlayBuffer()
{
    return officecfg::Office::Common::Drawinglayer::OverlayBuffer::get();
}

bool IsPaintBuffer()
{
    return officecfg::Office::Common::Drawinglayer::PaintBuffer::get();
}

Color GetStripeColorA()
{
    return Color(ColorTransparency, officecfg::Office::Common::Drawinglayer::StripeColorA::get());
}

Color GetStripeColorB()
{
    return Color(ColorTransparency, officecfg::Office::Common::Drawinglayer::StripeColorB::get());
}

sal_uInt16 GetStripeLength()
{
    return officecfg::Office::Common::Drawinglayer::StripeLength::get();
}

bool IsOverlayBuffer_Calc()
{
    return officecfg::Office::Common::Drawinglayer::OverlayBuffer_Calc::get();
}

bool IsOverlayBuffer_Writer()
{
    return officecfg::Office::Common::Drawinglayer::OverlayBuffer_Writer::get();
}

bool IsOverlayBuffer_DrawImpress()
{
    return officecfg::Office::Common::Drawinglayer::OverlayBuffer_DrawImpress::get();
}

// #i74769#, #i75172#
bool IsPaintBuffer_Calc()
{
    return officecfg::Office::Common::Drawinglayer::PaintBuffer_Calc::get();
}

bool IsPaintBuffer_Writer()
{
    return officecfg::Office::Common::Drawinglayer::PaintBuffer_Writer::get();
}

bool IsPaintBuffer_DrawImpress()
{
    return officecfg::Office::Common::Drawinglayer::PaintBuffer_DrawImpress::get();
}

// #i4219#
sal_uInt32 GetMaximumPaperWidth()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperWidth::get();
}

sal_uInt32 GetMaximumPaperHeight()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperHeight::get();
}

sal_uInt32 GetMaximumPaperLeftMargin()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperLeftMargin::get();
}

sal_uInt32 GetMaximumPaperRightMargin()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperRightMargin::get();
}

sal_uInt32 GetMaximumPaperTopMargin()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperTopMargin::get();
}

sal_uInt32 GetMaximumPaperBottomMargin()
{
    return officecfg::Office::Common::Drawinglayer::MaximumPaperBottomMargin::get();
}

static std::mutex gaAntiAliasMutex;
static bool gbAntiAliasingInit = false;
static bool gbAntiAliasing = false;
static bool gbAllowAAInit = false;
static bool gbAllowAA = false;

bool IsAAPossibleOnThisSystem()
{
    std::scoped_lock aGuard(gaAntiAliasMutex);
    if (!gbAllowAAInit)
    {
        gbAllowAAInit = true;
        gbAllowAA = Application::GetDefaultDevice()->SupportsOperation( OutDevSupportType::TransparentRect );
    }
    return gbAllowAA;
}


bool IsAntiAliasing()
{
    bool bAntiAliasing;
    {
        std::scoped_lock aGuard(gaAntiAliasMutex);
        if (!gbAntiAliasingInit)
        {
            gbAntiAliasingInit = true;
            gbAntiAliasing = officecfg::Office::Common::Drawinglayer::AntiAliasing::get();
        }
        bAntiAliasing = gbAntiAliasing;
    }
    return bAntiAliasing && IsAAPossibleOnThisSystem();
}

/**
  * Some code like to turn this stuff on and off during a drawing operation
  * so it can "tunnel" information down through several layers,
  * so we don't want to actually do a config write all the time.
  */
void SetAntiAliasing( bool bOn, bool bTemporary )
{
    std::scoped_lock aGuard(gaAntiAliasMutex);
    if (!bTemporary)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch =
                comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::Drawinglayer::AntiAliasing::set(bOn, batch);
        batch->commit();
    }
    gbAntiAliasing = bOn;
}


bool IsSnapHorVerLinesToDiscrete()
{
    return IsAntiAliasing() && officecfg::Office::Common::Drawinglayer::SnapHorVerLinesToDiscrete::get();
}

bool IsSolidDragCreate()
{
    return officecfg::Office::Common::Drawinglayer::SolidDragCreate::get();
}

bool IsRenderDecoratedTextDirect()
{
    return officecfg::Office::Common::Drawinglayer::RenderDecoratedTextDirect::get();
}

bool IsRenderSimpleTextDirect()
{
    return officecfg::Office::Common::Drawinglayer::RenderSimpleTextDirect::get();
}

sal_uInt32 GetQuadratic3DRenderLimit()
{
    return officecfg::Office::Common::Drawinglayer::Quadratic3DRenderLimit::get();
}

sal_uInt32 GetQuadraticFormControlRenderLimit()
{
    return officecfg::Office::Common::Drawinglayer::QuadraticFormControlRenderLimit::get();
}

// #i97672# selection settings
bool IsTransparentSelection()
{
    return officecfg::Office::Common::Drawinglayer::TransparentSelection::get();
}

sal_uInt16 GetTransparentSelectionPercent()
{
    sal_uInt16 aRetval = officecfg::Office::Common::Drawinglayer::TransparentSelectionPercent::get();

    // crop to range [10% .. 90%]
    if(aRetval < 10)
    {
        aRetval = 10;
    }

    if(aRetval > 90)
    {
        aRetval = 90;
    }

    return aRetval;
}

sal_uInt16 GetSelectionMaximumLuminancePercent()
{
    sal_uInt16 aRetval = officecfg::Office::Common::Drawinglayer::SelectionMaximumLuminancePercent::get();

    // crop to range [0% .. 100%]
    if(aRetval > 90)
    {
        aRetval = 90;
    }

    return aRetval;
}

Color getHilightColor()
{
    Color aRetval(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    const basegfx::BColor aSelection(aRetval.getBColor());
    const double fLuminance(aSelection.luminance());
    const double fMaxLum(GetSelectionMaximumLuminancePercent() / 100.0);

    if(fLuminance > fMaxLum)
    {
        const double fFactor(fMaxLum / fLuminance);
        const basegfx::BColor aNewSelection(
            aSelection.getRed() * fFactor,
            aSelection.getGreen() * fFactor,
            aSelection.getBlue() * fFactor);

        aRetval = Color(aNewSelection);
    }

    return aRetval;
}

} // namespace SvtOptionsDrawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
