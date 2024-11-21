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
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <mutex>

// #i73602#
// #i74769#, #i75172#
// #i4219#

namespace SvtOptionsDrawinglayer
{

Color GetStripeColorA()
{
    return Color(ColorTransparency, officecfg::Office::Common::Drawinglayer::StripeColorA::get());
}

Color GetStripeColorB()
{
    return Color(ColorTransparency, officecfg::Office::Common::Drawinglayer::StripeColorB::get());
}

bool IsAntiAliasing()
{
    return drawinglayer::geometry::ViewInformation2D::getGlobalAntiAliasing();
}

/**
  * Some code like to turn this stuff on and off during a drawing operation
  * so it can "tunnel" information down through several layers,
  * so we don't want to actually do a config write all the time.
  */
void SetAntiAliasing( bool bOn, bool bTemporary )
{
    drawinglayer::geometry::ViewInformation2D::setGlobalAntiAliasing(bOn, bTemporary);
}


bool IsSnapHorVerLinesToDiscrete()
{
    return IsAntiAliasing() && officecfg::Office::Common::Drawinglayer::SnapHorVerLinesToDiscrete::get();
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
