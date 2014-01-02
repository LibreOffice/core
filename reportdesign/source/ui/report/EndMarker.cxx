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
#include "EndMarker.hxx"
#include "ColorChanger.hxx"
#include "SectionWindow.hxx"
#include "helpids.hrc"

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>


#define CORNER_SPACE    5
//=====================================================================
namespace rptui
{
//=====================================================================
OEndMarker::OEndMarker(Window* _pParent ,const OUString& _sColorEntry)
: OColorListener(_pParent,_sColorEntry)
{
    SetUniqueId(HID_RPT_ENDMARKER);
    ImplInitSettings();
}
// -----------------------------------------------------------------------------
OEndMarker::~OEndMarker()
{
}
// -----------------------------------------------------------------------------
void OEndMarker::Paint( const Rectangle& /*rRect*/ )
{
    Fraction aCornerSpace(long(CORNER_SPACE));
    aCornerSpace *= GetMapMode().GetScaleX();
    const long nCornerSpace = aCornerSpace;

    Size aSize = GetSizePixel();
    aSize.Width() += nCornerSpace;
    Rectangle aWholeRect(Point(-nCornerSpace,0),aSize);
    PolyPolygon aPoly;
    aPoly.Insert(Polygon(aWholeRect,nCornerSpace,nCornerSpace));

    Color aStartColor(m_nColor);
    aStartColor.IncreaseLuminance(10);
    sal_uInt16 nHue = 0;
    sal_uInt16 nSat = 0;
    sal_uInt16 nBri = 0;
    aStartColor.RGBtoHSB(nHue, nSat, nBri);
    nSat += 40;
    Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
    Gradient aGradient(GradientStyle_LINEAR,aStartColor,aEndColor);
    aGradient.SetSteps(static_cast<sal_uInt16>(aSize.Height()));

    DrawGradient(PixelToLogic(aPoly) ,aGradient);
    if ( m_bMarked )
    {
        Rectangle aRect( Point(-nCornerSpace,nCornerSpace),
                         Size(aSize.Width()- nCornerSpace,aSize.Height() - nCornerSpace- nCornerSpace));
        ColorChanger aColors( this, COL_WHITE, COL_WHITE );
        DrawPolyLine(Polygon(PixelToLogic(aRect)),LineInfo(LINE_SOLID,2));
    }
}
// -----------------------------------------------------------------------
void OEndMarker::ImplInitSettings()
{
    EnableChildTransparentMode( sal_True );
    SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    SetPaintTransparent( sal_True );

    SetBackground( Wallpaper( svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor)  );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetShadowColor() );
}
// -----------------------------------------------------------------------
void OEndMarker::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
        return;
    static_cast<OSectionWindow*>(GetParent())->showProperties();
}
// =======================================================================
}
// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
