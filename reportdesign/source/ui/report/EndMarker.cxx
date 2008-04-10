/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EndMarker.cxx,v $
 * $Revision: 1.4 $
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
#include "ViewsWindow.hxx"
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
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
    Size aSize = GetSizePixel();
    aSize.Width() += CORNER_SPACE;
    Rectangle aWholeRect(Point(-CORNER_SPACE,0),aSize);
    PolyPolygon aPoly;
    aPoly.Insert(Polygon(aWholeRect,CORNER_SPACE,CORNER_SPACE));

    Color aStartColor(m_nColor);
    aStartColor.IncreaseLuminance(10);
    USHORT nHue = 0;
    USHORT nSat = 0;
    USHORT nBri = 0;
    aStartColor.RGBtoHSB(nHue, nSat, nBri);
    nSat += 40;
    Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
    Gradient aGradient(GRADIENT_LINEAR,aStartColor,aEndColor);
    aGradient.SetSteps(static_cast<USHORT>(aSize.Height()));

    DrawGradient(aPoly ,aGradient);
    if ( m_bMarked )
    {
        Rectangle aRect( Point(-CORNER_SPACE,CORNER_SPACE),
                         Size(aSize.Width()- CORNER_SPACE,aSize.Height() - CORNER_SPACE- CORNER_SPACE));
        ColorChanger aColors( this, COL_WHITE, COL_WHITE );
        DrawPolyLine(Polygon(aRect),LineInfo(LINE_SOLID,2));
    }
}
// -----------------------------------------------------------------------
void OEndMarker::ImplInitSettings()
{
    //SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    /*SetBackground( );*/
    SetBackground( Wallpaper( m_aColorConfig.GetColorValue(::svtools::APPBACKGROUND).nColor ) );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetShadowColor() );
    //SetTextFillColor( Application::GetSettings().GetStyleSettings().GetDarkShadowColor() );
}
// -----------------------------------------------------------------------
void OEndMarker::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
        return;
    static_cast<OViewsWindow*>(GetParent())->showProperties(this);
}
// =======================================================================
}
// =======================================================================
