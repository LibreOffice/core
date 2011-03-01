/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
#include "EndMarker.hxx"
#include "ColorChanger.hxx"
#include "SectionWindow.hxx"
#include "helpids.hrc"
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>


#define CORNER_SPACE    5
//=====================================================================
namespace rptui
{
//=====================================================================
    DBG_NAME( rpt_OEndMarker )
OEndMarker::OEndMarker(Window* _pParent ,const ::rtl::OUString& _sColorEntry)
: OColorListener(_pParent,_sColorEntry)
{
    DBG_CTOR( rpt_OEndMarker,NULL);
    SetUniqueId(HID_RPT_ENDMARKER);
    ImplInitSettings();
}
// -----------------------------------------------------------------------------
OEndMarker::~OEndMarker()
{
    DBG_DTOR( rpt_OEndMarker,NULL);
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
    Gradient aGradient(GRADIENT_LINEAR,aStartColor,aEndColor);
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
