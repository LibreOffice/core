/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EndMarker.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:50:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_ENDMARKER_HXX
#include "EndMarker.hxx"
#endif
#ifndef RPTUI_COLORCHANGER_HXX
#include "ColorChanger.hxx"
#endif
#ifndef RPTUI_VIEWSWINDOW_HXX
#include "ViewsWindow.hxx"
#endif
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
