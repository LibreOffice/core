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
#include <EndMarker.hxx>
#include <ColorChanger.hxx>
#include <SectionWindow.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/event.hxx>


#define CORNER_SPACE    5

namespace rptui
{

OEndMarker::OEndMarker(vcl::Window* _pParent ,const OUString& _sColorEntry)
    : OColorListener(_pParent, _sColorEntry)
{
    ImplInitSettings();
}

OEndMarker::~OEndMarker()
{
}

void OEndMarker::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    Fraction aCornerSpace(long(CORNER_SPACE));
    aCornerSpace *= rRenderContext.GetMapMode().GetScaleX();
    const long nCornerSpace = long(aCornerSpace);

    Size aSize = GetSizePixel();
    aSize.AdjustWidth(nCornerSpace );
    tools::Rectangle aWholeRect(Point(-nCornerSpace,0),aSize);
    tools::PolyPolygon aPoly;
    aPoly.Insert( tools::Polygon(aWholeRect,nCornerSpace,nCornerSpace));

    Color aStartColor(m_nColor);
    aStartColor.IncreaseLuminance(10);
    sal_uInt16 nHue = 0;
    sal_uInt16 nSat = 0;
    sal_uInt16 nBri = 0;
    aStartColor.RGBtoHSB(nHue, nSat, nBri);
    nSat += 40;
    Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
    Gradient aGradient(GradientStyle::Linear, aStartColor, aEndColor);
    aGradient.SetSteps(static_cast<sal_uInt16>(aSize.Height()));

    rRenderContext.DrawGradient(PixelToLogic(aPoly), aGradient);
    if (m_bMarked)
    {
        tools::Rectangle aRect(Point(-nCornerSpace, nCornerSpace),
                         Size(aSize.Width() - nCornerSpace,
                              aSize.Height() - nCornerSpace - nCornerSpace));
        ColorChanger aColors(this, COL_WHITE, COL_WHITE);
        rRenderContext.DrawPolyLine( tools::Polygon(PixelToLogic(aRect)), LineInfo(LineStyle::Solid, 2));
    }
}

void OEndMarker::ImplInitSettings()
{
    EnableChildTransparentMode();
    SetParentClipMode( ParentClipMode::NoClip );
    SetPaintTransparent( true );

    SetBackground( Wallpaper( svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor)  );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetShadowColor() );
}

void OEndMarker::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
        return;
    static_cast<OSectionWindow*>(GetParent())->showProperties();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
