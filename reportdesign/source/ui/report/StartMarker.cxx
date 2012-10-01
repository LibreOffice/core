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
#include "StartMarker.hxx"
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include "RptResId.hrc"
#include "ModuleHelper.hxx"
#include "ColorChanger.hxx"
#include "ReportDefines.hxx"
#include "SectionWindow.hxx"
#include "helpids.hrc"
#include <vcl/help.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/smplhint.hxx>

#define CORNER_SPACE     5

//=====================================================================
namespace rptui
{
//=====================================================================

Image*  OStartMarker::s_pDefCollapsed       = NULL;
Image*  OStartMarker::s_pDefExpanded        = NULL;
oslInterlockedCount OStartMarker::s_nImageRefCount  = 0;

DBG_NAME( rpt_OStartMarker )
// -----------------------------------------------------------------------------
OStartMarker::OStartMarker(OSectionWindow* _pParent,const ::rtl::OUString& _sColorEntry)
: OColorListener(_pParent,_sColorEntry)
,m_aVRuler(this,WB_VERT)
,m_aText(this,WB_HYPHENATION)
,m_aImage(this,WB_LEFT|WB_TOP|WB_SCALE)
,m_pParent(_pParent)
,m_bShowRuler(sal_True)
{
    DBG_CTOR( rpt_OStartMarker,NULL);
    SetUniqueId(HID_RPT_STARTMARKER);

    osl_atomic_increment(&s_nImageRefCount);
    initDefaultNodeImages();
    ImplInitSettings();
    m_aText.SetHelpId(HID_RPT_START_TITLE);
    m_aText.SetPaintTransparent(sal_True);
    m_aImage.SetHelpId(HID_RPT_START_IMAGE);
    m_aText.Show();
    m_aImage.Show();
    m_aVRuler.Show();
    m_aVRuler.Activate();
    m_aVRuler.SetPagePos(0);
    m_aVRuler.SetBorders();
    m_aVRuler.SetIndents();
    m_aVRuler.SetMargin1();
    m_aVRuler.SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aVRuler.SetUnit(MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH);
    EnableChildTransparentMode( sal_True );
    SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    SetPaintTransparent( sal_True );
}
// -----------------------------------------------------------------------------
OStartMarker::~OStartMarker()
{
    DBG_DTOR( rpt_OStartMarker,NULL);
    if ( osl_atomic_decrement(&s_nImageRefCount) == 0 )
    {
        DELETEZ(s_pDefCollapsed);
        DELETEZ(s_pDefExpanded);
    }
}
// -----------------------------------------------------------------------------
sal_Int32 OStartMarker::getMinHeight() const
{
    Fraction aExtraWidth(long(2*REPORT_EXTRA_SPACE));
    aExtraWidth *= GetMapMode().GetScaleX();
    return LogicToPixel(Size(0,m_aText.GetTextHeight())).Height() + (long)aExtraWidth;
}
// -----------------------------------------------------------------------------
void OStartMarker::Paint( const Rectangle& rRect )
{
    (void)rRect;
    Size aSize = GetOutputSizePixel();
    long nSize = aSize.Width();
    const long nCornerWidth = long(CORNER_SPACE * (double)GetMapMode().GetScaleX());

    if ( isCollapsed() )
    {
        SetClipRegion();
    }
    else
    {
        const long nVRulerWidth = m_aVRuler.GetSizePixel().Width();
        nSize = aSize.Width() - nVRulerWidth;
        aSize.Width() += nCornerWidth;
        SetClipRegion(Region(PixelToLogic(Rectangle(Point(),Size(nSize,aSize.Height())))));
    }

    const Point aGcc3WorkaroundTemporary;
    Rectangle aWholeRect(aGcc3WorkaroundTemporary,aSize);
    {
        const ColorChanger aColors( this, m_nTextBoundaries, m_nColor );
        PolyPolygon aPoly;
        aPoly.Insert(Polygon(aWholeRect,nCornerWidth,nCornerWidth));

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
    }
    if ( m_bMarked )
    {
        const long nCornerHeight = long(CORNER_SPACE * (double)GetMapMode().GetScaleY());
        Rectangle aRect( Point(nCornerWidth,nCornerHeight),
                         Size(aSize.Width() - nCornerWidth - nCornerWidth,aSize.Height() - nCornerHeight - nCornerHeight));
        ColorChanger aColors( this, COL_WHITE, COL_WHITE );
        DrawPolyLine(Polygon(PixelToLogic(aRect)),LineInfo(LINE_SOLID,2 ));
    }
}
// -----------------------------------------------------------------------------
void OStartMarker::setColor()
{
    const Color aColor(m_nColor);
    Color aTextColor = GetTextColor();
    if ( aColor.GetLuminance() < 128 )
        aTextColor = COL_WHITE;
    m_aText.SetTextColor(aTextColor);
    m_aText.SetLineColor(m_nColor);
}
// -----------------------------------------------------------------------
void OStartMarker::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
        return;

    Point aPos( rMEvt.GetPosPixel());

    const Size aOutputSize = GetOutputSizePixel();
    if( aPos.X() > aOutputSize.Width() || aPos.Y() > aOutputSize.Height() )
        return;
    Rectangle aRect(m_aImage.GetPosPixel(),m_aImage.GetSizePixel());
    if ( rMEvt.GetClicks() == 2 || aRect.IsInside( aPos ) )
    {
        m_bCollapsed = !m_bCollapsed;

        changeImage();

        m_aVRuler.Show(!m_bCollapsed && m_bShowRuler);
        if ( m_aCollapsedLink.IsSet() )
            m_aCollapsedLink.Call(this);
    }

    m_pParent->showProperties();
}
// -----------------------------------------------------------------------------
void OStartMarker::changeImage()
{
    Image* pImage = m_bCollapsed ? s_pDefCollapsed : s_pDefExpanded;
    m_aImage.SetImage(*pImage);
}
// -----------------------------------------------------------------------
void OStartMarker::initDefaultNodeImages()
{
    if ( !s_pDefCollapsed )
    {
        s_pDefCollapsed     = new Image( ModuleRes( RID_IMG_TREENODE_COLLAPSED      ) );
        s_pDefExpanded      = new Image( ModuleRes( RID_IMG_TREENODE_EXPANDED       ) );
    }

    Image* pImage = m_bCollapsed ? s_pDefCollapsed : s_pDefExpanded;
    m_aImage.SetImage(*pImage);
    m_aImage.SetMouseTransparent(sal_True);
    m_aImage.SetBackground();
    m_aText.SetBackground();
    m_aText.SetMouseTransparent(sal_True);
}
// -----------------------------------------------------------------------
void OStartMarker::ImplInitSettings()
{
    SetBackground( );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
    setColor();
}
//------------------------------------------------------------------------------
void OStartMarker::Resize()
{
    const Size aOutputSize( GetOutputSizePixel() );
    const long nOutputWidth  = aOutputSize.Width();
    const long nOutputHeight = aOutputSize.Height();

    const long nVRulerWidth = m_aVRuler.GetSizePixel().Width();
    const Point aRulerPos(nOutputWidth - nVRulerWidth,0);
    m_aVRuler.SetPosSizePixel(aRulerPos,Size(nVRulerWidth,nOutputHeight));

    Size aImageSize = m_aImage.GetImage().GetSizePixel();
    const MapMode& rMapMode = GetMapMode();
    aImageSize.Width() = long(aImageSize.Width() * (double)rMapMode.GetScaleX());
    aImageSize.Height() = long(aImageSize.Height() * (double)rMapMode.GetScaleY());

    Fraction aExtraWidth(long(REPORT_EXTRA_SPACE));
    aExtraWidth *= rMapMode.GetScaleX();

    Point aPos(aImageSize.Width() + (long)(aExtraWidth + aExtraWidth), aExtraWidth);
    const long nHeight = ::std::max<sal_Int32>(nOutputHeight - 2*aPos.Y(),LogicToPixel(Size(0,m_aText.GetTextHeight())).Height());
    m_aText.SetPosSizePixel(aPos,Size(aRulerPos.X() - aPos.X(),nHeight));

    aPos.X() = aExtraWidth;
    aPos.Y() += static_cast<sal_Int32>((LogicToPixel(Size(0,m_aText.GetTextHeight())).Height() - aImageSize.Height()) * 0.5) ;
    m_aImage.SetPosSizePixel(aPos,aImageSize);
}
// -----------------------------------------------------------------------------
void OStartMarker::setTitle(const String& _sTitle)
{
    m_aText.SetText(_sTitle);
}
// -----------------------------------------------------------------------------
void OStartMarker::Notify(SfxBroadcaster & rBc, SfxHint const & rHint)
{
    OColorListener::Notify(rBc, rHint);
    if (rHint.ISA(SfxSimpleHint)
        && (static_cast< SfxSimpleHint const & >(rHint).GetId()
            == SFX_HINT_COLORS_CHANGED))
    {
        setColor();
        Invalidate(INVALIDATE_CHILDREN);
    }
}
//----------------------------------------------------------------------------
void OStartMarker::showRuler(sal_Bool _bShow)
{
    m_bShowRuler = _bShow;
    m_aVRuler.Show(!m_bCollapsed && m_bShowRuler);
}
//------------------------------------------------------------------------------
void OStartMarker::RequestHelp( const HelpEvent& rHEvt )
{
    if( m_aText.GetText().Len())
    {
        // Hilfe anzeigen
        Rectangle aItemRect(rHEvt.GetMousePosPixel(),Size(GetSizePixel().Width(),getMinHeight()));
        Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();
        if( rHEvt.GetMode() == HELPMODE_BALLOON )
            Help::ShowBalloon( this, aItemRect.Center(), aItemRect, m_aText.GetText());
        else
            Help::ShowQuickHelp( this, aItemRect, m_aText.GetText() );
    }
}
// -----------------------------------------------------------------------------
void OStartMarker::setCollapsed(sal_Bool _bCollapsed)
{
    OColorListener::setCollapsed(_bCollapsed);
    showRuler(_bCollapsed);
    changeImage();
}
// -----------------------------------------------------------------------
void OStartMarker::zoom(const Fraction& _aZoom)
{
    setZoomFactor(_aZoom,*this);
    m_aVRuler.SetZoom(_aZoom);
    setZoomFactor(_aZoom,m_aText);
    Resize();
    Invalidate();
}
// -----------------------------------------------------------------------
// =======================================================================
}
// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
