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

#include <config_wasm_strip.h>

#include <vcl/rendercontext/DrawModeFlags.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/event.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/relfld.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xtable.hxx>
#include <bitmaps.hlst>
#include <svx/dlgctrl.hxx>
#include <tools/debug.hxx>
#include <svxpixelctlaccessiblecontext.hxx>
#include <svtools/colorcfg.hxx>
#include <svxrectctaccessiblecontext.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdopath.hxx>
#include <sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/BitmapTools.hxx>

#define OUTPUT_DRAWMODE_COLOR       (DrawModeFlags::Default)
#define OUTPUT_DRAWMODE_CONTRAST    (DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

// Control for display and selection of the corner points and
// mid point of an object

Bitmap& SvxRectCtl::GetRectBitmap()
{
    if( maBitmap.IsEmpty() )
        InitRectBitmap();

    return maBitmap;
}

SvxRectCtl::SvxRectCtl(SvxTabPage* pPage)
    : m_pPage(pPage)
    , m_nBorderWidth(Application::GetDefaultDevice()->LogicToPixel(Size(200, 0), MapMode(MapUnit::Map100thMM)).Width())
    , m_eRP(RectPoint::MM)
    , m_eDefRP(RectPoint::MM)
    , m_nState(CTL_STATE::NONE)
    , mbCompleteDisable(false)
{
}

void SvxRectCtl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_approximate_digit_width() * 25,
               pDrawingArea->get_text_height() * 5);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    Resize_Impl(aSize);
}

void SvxRectCtl::SetControlSettings(RectPoint eRpt, sal_uInt16 nBorder)
{
    m_nBorderWidth = Application::GetDefaultDevice()->LogicToPixel(Size(nBorder, 0), MapMode(MapUnit::Map100thMM)).Width();
    m_eDefRP = eRpt;
    Resize();
}

SvxRectCtl::~SvxRectCtl()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    m_pAccContext.clear();
#endif
}

void SvxRectCtl::Resize()
{
    Resize_Impl(GetOutputSizePixel());
}

void SvxRectCtl::Resize_Impl(const Size &rSize)
{
    m_aPtLT = Point( 0 + m_nBorderWidth,  0 + m_nBorderWidth );
    m_aPtMT = Point( rSize.Width() / 2, 0 + m_nBorderWidth );
    m_aPtRT = Point( rSize.Width() - m_nBorderWidth, 0 + m_nBorderWidth );

    m_aPtLM = Point( 0 + m_nBorderWidth,  rSize.Height() / 2 );
    m_aPtMM = Point( rSize.Width() / 2, rSize.Height() / 2 );
    m_aPtRM = Point( rSize.Width() - m_nBorderWidth, rSize.Height() / 2 );

    m_aPtLB = Point( 0 + m_nBorderWidth,    rSize.Height() - m_nBorderWidth );
    m_aPtMB = Point( rSize.Width() / 2,   rSize.Height() - m_nBorderWidth );
    m_aPtRB = Point( rSize.Width() - m_nBorderWidth, rSize.Height() - m_nBorderWidth );

    Reset();
    StyleUpdated();
}

void SvxRectCtl::InitRectBitmap()
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    maBitmap = Bitmap(RID_SVXCTRL_RECTBTNS);

    // set bitmap-colors
    Color aColorAry1[7];
    Color aColorAry2[7];
    aColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );  // light-gray
    aColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );  // yellow
    aColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );  // white
    aColorAry1[3] = Color( 0x80, 0x80, 0x80 );  // dark-gray
    aColorAry1[4] = Color( 0x00, 0x00, 0x00 );  // black
    aColorAry1[5] = Color( 0x00, 0xFF, 0x00 );  // green
    aColorAry1[6] = Color( 0x00, 0x00, 0xFF );  // blue
    aColorAry2[0] = rStyles.GetDialogColor();       // background
    aColorAry2[1] = rStyles.GetWindowColor();
    aColorAry2[2] = rStyles.GetLightColor();
    aColorAry2[3] = rStyles.GetShadowColor();
    aColorAry2[4] = rStyles.GetDarkShadowColor();
    aColorAry2[5] = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    aColorAry2[6] = rStyles.GetDialogColor();

#ifdef DBG_UTIL
    static bool     bModify = false;
    bool&           rModify = bModify;
    if( rModify )
    {
        static int      n = 0;
        static sal_uInt8    r = 0xFF;
        static sal_uInt8    g = 0x00;
        static sal_uInt8    b = 0xFF;
        int&            rn = n;
        sal_uInt8&          rr = r;
        sal_uInt8&          rg = g;
        sal_uInt8&          rb = b;
        aColorAry2[ rn ] = Color( rr, rg, rb );
    }
#endif

    maBitmap.Replace( aColorAry1, aColorAry2, 7, nullptr );
}

void SvxRectCtl::StyleUpdated()
{
    maBitmap.SetEmpty(); // forces new creating of bitmap
    CustomWidgetController::StyleUpdated();
}

void SvxRectCtl::InitSettings(vcl::RenderContext& rRenderContext)
{
    svtools::ColorConfig aColorConfig;
    Color aTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);
    rRenderContext.SetTextColor(aTextColor);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(rStyleSettings.GetWindowColor());
}

// The clicked rectangle (3 x 3) is determined and the parent (dialog)
// is notified that the item was changed
bool SvxRectCtl::MouseButtonDown(const MouseEvent& rMEvt)
{
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        m_aPtNew = GetApproxLogPtFromPixPt( rMEvt.GetPosPixel() );
        m_eRP = GetRPFromPoint( m_aPtNew );
        SetActualRP( m_eRP );

        if (m_pPage)
            m_pPage->PointChanged(GetDrawingArea(), m_eRP);
    }
    return true;
}

bool SvxRectCtl::KeyInput(const KeyEvent& rKeyEvt)
{
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if (IsCompletelyDisabled())
        return false;

    RectPoint eNewRP = m_eRP;

    switch( rKeyEvt.GetKeyCode().GetCode() )
    {
        case KEY_DOWN:
        {
            if( !(m_nState & CTL_STATE::NOVERT) )
                switch( eNewRP )
                {
                    case RectPoint::LT: eNewRP = RectPoint::LM; break;
                    case RectPoint::MT: eNewRP = RectPoint::MM; break;
                    case RectPoint::RT: eNewRP = RectPoint::RM; break;
                    case RectPoint::LM: eNewRP = RectPoint::LB; break;
                    case RectPoint::MM: eNewRP = RectPoint::MB; break;
                    case RectPoint::RM: eNewRP = RectPoint::RB; break;
                    default: ; //prevent warning
                }
        }
        break;
        case KEY_UP:
        {
            if( !(m_nState & CTL_STATE::NOVERT) )
                switch( eNewRP )
                {
                    case RectPoint::LM: eNewRP = RectPoint::LT; break;
                    case RectPoint::MM: eNewRP = RectPoint::MT; break;
                    case RectPoint::RM: eNewRP = RectPoint::RT; break;
                    case RectPoint::LB: eNewRP = RectPoint::LM; break;
                    case RectPoint::MB: eNewRP = RectPoint::MM; break;
                    case RectPoint::RB: eNewRP = RectPoint::RM; break;
                    default: ; //prevent warning
                }
        }
        break;
        case KEY_LEFT:
        {
            if( !(m_nState & CTL_STATE::NOHORZ) )
                switch( eNewRP )
                {
                    case RectPoint::MT: eNewRP = RectPoint::LT; break;
                    case RectPoint::RT: eNewRP = RectPoint::MT; break;
                    case RectPoint::MM: eNewRP = RectPoint::LM; break;
                    case RectPoint::RM: eNewRP = RectPoint::MM; break;
                    case RectPoint::MB: eNewRP = RectPoint::LB; break;
                    case RectPoint::RB: eNewRP = RectPoint::MB; break;
                    default: ; //prevent warning
                }
        }
        break;
        case KEY_RIGHT:
        {
            if( !(m_nState & CTL_STATE::NOHORZ) )
                switch( eNewRP )
                {
                    case RectPoint::LT: eNewRP = RectPoint::MT; break;
                    case RectPoint::MT: eNewRP = RectPoint::RT; break;
                    case RectPoint::LM: eNewRP = RectPoint::MM; break;
                    case RectPoint::MM: eNewRP = RectPoint::RM; break;
                    case RectPoint::LB: eNewRP = RectPoint::MB; break;
                    case RectPoint::MB: eNewRP = RectPoint::RB; break;
                    default: ; //prevent warning
                }
        }
        break;
        default:
            return false;
    }
    if( eNewRP != m_eRP )
    {
        SetActualRP( eNewRP );

        if (m_pPage)
            m_pPage->PointChanged(GetDrawingArea(), m_eRP);
    }
    return true;
}

// the control (rectangle with 9 circles)
void SvxRectCtl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    InitSettings(rRenderContext);

    Point aPtDiff(1, 1);

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();

    rRenderContext.SetLineColor(rStyles.GetDialogColor());
    rRenderContext.SetFillColor(rStyles.GetDialogColor());
    rRenderContext.DrawRect(tools::Rectangle(Point(0,0), rRenderContext.GetOutputSize()));

    if (IsEnabled())
        rRenderContext.SetLineColor(rStyles.GetLabelTextColor());
    else
        rRenderContext.SetLineColor(rStyles.GetShadowColor());

    rRenderContext.SetFillColor();

    if (!IsEnabled())
    {
        Color aOldCol = rRenderContext.GetLineColor();
        rRenderContext.SetLineColor(rStyles.GetLightColor());
        rRenderContext.DrawRect(tools::Rectangle(m_aPtLT + aPtDiff, m_aPtRB + aPtDiff));
        rRenderContext.SetLineColor(aOldCol);
    }
    rRenderContext.DrawRect(tools::Rectangle(m_aPtLT, m_aPtRB));

    rRenderContext.SetFillColor(rRenderContext.GetBackground().GetColor());

    Size aBtnSize(11, 11);
    Size aDstBtnSize(aBtnSize);
    Point aToCenter(aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1);
    Point aBtnPnt1(IsEnabled() ? 0 : 22, 0);
    Point aBtnPnt2(11, 0);
    Point aBtnPnt3(22, 0);

    bool bNoHorz = bool(m_nState & CTL_STATE::NOHORZ);
    bool bNoVert = bool(m_nState & CTL_STATE::NOVERT);

    Bitmap& rBitmap = GetRectBitmap();

    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if (IsCompletelyDisabled())
    {
        rRenderContext.DrawBitmap(m_aPtLT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtMT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtRT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtLM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtMM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtRM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtLB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtMB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtRB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap);
    }
    else
    {
        rRenderContext.DrawBitmap(m_aPtLT - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtMT - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtRT - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtLM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);

        // Center for rectangle and line
        rRenderContext.DrawBitmap(m_aPtMM - aToCenter, aDstBtnSize, aBtnPnt1, aBtnSize, rBitmap);

        rRenderContext.DrawBitmap(m_aPtRM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtLB - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtMB - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
        rRenderContext.DrawBitmap(m_aPtRB - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap);
    }

    // draw active button, avoid center pos for angle
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if (!IsCompletelyDisabled())
    {
        if (IsEnabled())
        {
            Point aCenterPt(m_aPtNew);
            aCenterPt -= aToCenter;

            rRenderContext.DrawBitmap(aCenterPt, aDstBtnSize, aBtnPnt2, aBtnSize, rBitmap);
        }
    }
}

tools::Rectangle SvxRectCtl::GetFocusRect()
{
    tools::Rectangle aRet;
    if (HasFocus())
        aRet = CalculateFocusRectangle();
    return aRet;
}

// Convert RectPoint Point

const Point& SvxRectCtl::GetPointFromRP( RectPoint _eRP) const
{
    switch( _eRP )
    {
        case RectPoint::LT: return m_aPtLT;
        case RectPoint::MT: return m_aPtMT;
        case RectPoint::RT: return m_aPtRT;
        case RectPoint::LM: return m_aPtLM;
        case RectPoint::MM: return m_aPtMM;
        case RectPoint::RM: return m_aPtRM;
        case RectPoint::LB: return m_aPtLB;
        case RectPoint::MB: return m_aPtMB;
        case RectPoint::RB: return m_aPtRB;
    }
    return m_aPtMM; // default
}

Point SvxRectCtl::SetActualRPWithoutInvalidate( RectPoint eNewRP )
{
    Point aPtLast = m_aPtNew;
    m_aPtNew = GetPointFromRP( eNewRP );

    if( m_nState & CTL_STATE::NOHORZ )
        m_aPtNew.setX( m_aPtMM.X() );

    if( m_nState & CTL_STATE::NOVERT )
        m_aPtNew.setY( m_aPtMM.Y() );

    // fdo#74751 this fix reverse base point on RTL UI.
    bool bRTL = AllSettings::GetLayoutRTL();
    eNewRP = GetRPFromPoint( m_aPtNew, bRTL );

    m_eRP = eNewRP;

    return aPtLast;
}

void SvxRectCtl::GetFocus()
{
    Invalidate();

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    // Send accessibility event.
    if (m_pAccContext.is())
    {
        m_pAccContext->FireChildFocus(GetActualRP());
    }
#endif
}

void SvxRectCtl::LoseFocus()
{
    Invalidate();
}

Point SvxRectCtl::GetApproxLogPtFromPixPt( const Point& rPt ) const
{
    Point   aPt = rPt;
    tools::Long    x;
    tools::Long    y;

    Size aSize(GetOutputSizePixel());

    if( !( m_nState & CTL_STATE::NOHORZ ) )
    {
        if( aPt.X() < aSize.Width() / 3 )
            x = m_aPtLT.X();
        else if( aPt.X() < aSize.Width() * 2 / 3 )
            x = m_aPtMM.X();
        else
            x = m_aPtRB.X();
    }
    else
        x = m_aPtMM.X();

    if( !( m_nState & CTL_STATE::NOVERT ) )
    {
        if( aPt.Y() < aSize.Height() / 3 )
            y = m_aPtLT.Y();
        else if( aPt.Y() < aSize.Height() * 2 / 3 )
            y = m_aPtMM.Y();
        else
            y = m_aPtRB.Y();
    }
    else
            y = m_aPtMM.Y();

    return Point( x, y );
}


// Converts Point in RectPoint

RectPoint SvxRectCtl::GetRPFromPoint( Point aPt, bool bRTL ) const
{
    RectPoint rPoint = RectPoint::MM;  // default

    if (aPt == m_aPtLT) rPoint = bRTL ? RectPoint::RT : RectPoint::LT;
    else if( aPt == m_aPtMT) rPoint = RectPoint::MT;
    else if( aPt == m_aPtRT) rPoint = bRTL ? RectPoint::LT : RectPoint::RT;
    else if( aPt == m_aPtLM) rPoint = bRTL ? RectPoint::RM : RectPoint::LM;
    else if( aPt == m_aPtRM) rPoint = bRTL ? RectPoint::LM : RectPoint::RM;
    else if( aPt == m_aPtLB) rPoint = bRTL ? RectPoint::RB : RectPoint::LB;
    else if( aPt == m_aPtMB) rPoint = RectPoint::MB;
    else if( aPt == m_aPtRB) rPoint = bRTL ? RectPoint::LB : RectPoint::RB;

    return rPoint;
}

// Resets to the original state of the control

void SvxRectCtl::Reset()
{
    m_aPtNew = GetPointFromRP( m_eDefRP );
    m_eRP = m_eDefRP;
    Invalidate();
}

// Returns the currently selected RectPoint


void SvxRectCtl::SetActualRP( RectPoint eNewRP )
{
    SetActualRPWithoutInvalidate(eNewRP);

    Invalidate();

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    // notify accessibility object about change
    if (m_pAccContext.is())
        m_pAccContext->selectChild( eNewRP /* MT, bFireFocus */ );
#endif
}

void SvxRectCtl::SetState( CTL_STATE nState )
{
    m_nState = nState;

    Point aPtLast( GetPointFromRP( m_eRP ) );
    Point _aPtNew( aPtLast );

    if( m_nState & CTL_STATE::NOHORZ )
        _aPtNew.setX( m_aPtMM.X() );

    if( m_nState & CTL_STATE::NOVERT)
        _aPtNew.setY( m_aPtMM.Y() );

    m_eRP = GetRPFromPoint( _aPtNew );
    Invalidate();

    if (m_pPage)
        m_pPage->PointChanged(GetDrawingArea(), m_eRP);
}

tools::Rectangle SvxRectCtl::CalculateFocusRectangle() const
{
    Size        aDstBtnSize(15, 15);
    return tools::Rectangle( m_aPtNew - Point( aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1 ), aDstBtnSize );
}

tools::Rectangle SvxRectCtl::CalculateFocusRectangle( RectPoint eRectPoint ) const
{
    tools::Rectangle   aRet;
    RectPoint  eOldRectPoint = GetActualRP();

    if( eOldRectPoint == eRectPoint )
        aRet = CalculateFocusRectangle();
    else
    {
        SvxRectCtl* pThis = const_cast<SvxRectCtl*>(this);

        pThis->SetActualRPWithoutInvalidate( eRectPoint );      // no invalidation because it's only temporary!
        aRet = CalculateFocusRectangle();

        pThis->SetActualRPWithoutInvalidate( eOldRectPoint );   // no invalidation because nothing has changed!
    }

    return aRet;
}

rtl::Reference<comphelper::OAccessible> SvxRectCtl::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    m_pAccContext = new SvxRectCtlAccessibleContext(this);
#endif
    return m_pAccContext;
}

RectPoint SvxRectCtl::GetApproxRPFromPixPt( const css::awt::Point& r ) const
{
    return GetRPFromPoint( GetApproxLogPtFromPixPt( Point( r.X, r.Y ) ) );
}

// CompletelyDisabled() added to have a disabled state for SvxRectCtl
void SvxRectCtl::DoCompletelyDisable(bool bNew)
{
    mbCompleteDisable = bNew;
    Invalidate();
}

void SvxRectCtl::SaveValue()
{
    m_eDefRP = m_eRP;
}

bool SvxRectCtl::IsValueModified() const
{
    return m_eDefRP != m_eRP;
}

// Control for editing bitmaps

rtl::Reference<comphelper::OAccessible> SvxPixelCtl::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (!m_xAccess.is())
        m_xAccess = new SvxPixelCtlAccessible(this);
#endif
    return m_xAccess;
}

tools::Long SvxPixelCtl::PointToIndex(const Point &aPt) const
{
    tools::Long nX = aPt.X() * nLines / m_aRectSize.Width();
    tools::Long nY = aPt.Y() * nLines / m_aRectSize.Height();

    return nX + nY * nLines ;
}

Point SvxPixelCtl::IndexToPoint(tools::Long nIndex) const
{
    DBG_ASSERT(nIndex >= 0 && nIndex < nSquares ," Check Index");

    sal_Int32 nXIndex = nIndex % nLines;
    sal_Int32 nYIndex = nIndex / nLines;

    Point aPtTl;
    aPtTl.setY( m_aRectSize.Height() * nYIndex / nLines + 1 );
    aPtTl.setX( m_aRectSize.Width() * nXIndex / nLines + 1 );

    return aPtTl;
}

tools::Long SvxPixelCtl::GetFocusPosIndex() const
{
    return m_aFocusPosition.getX() + m_aFocusPosition.getY() * nLines ;
}

tools::Long SvxPixelCtl::ShowPosition( const Point &rPt)
{
    sal_Int32 nX = rPt.X() * nLines / m_aRectSize.Width();
    sal_Int32 nY = rPt.Y() * nLines / m_aRectSize.Height();

    ChangePixel( nX + nY * nLines );

    //Solution:Set new focus position and repaint
    m_aFocusPosition.setX(nX);
    m_aFocusPosition.setY(nY);
    Invalidate(tools::Rectangle(Point(0,0),m_aRectSize));

    if (m_pPage)
        m_pPage->PointChanged(GetDrawingArea(), RectPoint::MM ); // RectPoint is dummy

    return GetFocusPosIndex();

}

SvxPixelCtl::SvxPixelCtl(SvxTabPage* pPage)
    : m_pPage(pPage)
    , m_bPaintable(true)
    , m_aFocusPosition(0,0)
{
    maPixelData.fill(0);
}

void SvxPixelCtl::Resize()
{
    CustomWidgetController::Resize();
    m_aRectSize = GetOutputSizePixel();
}

void SvxPixelCtl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 25,
                                   pDrawingArea->get_text_height() * 10);
}

SvxPixelCtl::~SvxPixelCtl()
{
}

// Changes the foreground or Background color

void SvxPixelCtl::ChangePixel( sal_uInt16 nPixel )
{
    if( maPixelData[nPixel] == 0 )
        maPixelData[nPixel] = 1; //  could be extended to more colors
    else
        maPixelData[nPixel] = 0;
}

// The clicked rectangle is identified, to change its color

bool SvxPixelCtl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!m_aRectSize.Width() || !m_aRectSize.Height())
        return true;

    //Grab focus when click in window
    if (!HasFocus())
    {
        GrabFocus();
    }

    tools::Long nIndex = ShowPosition(rMEvt.GetPosPixel());

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if(m_xAccess.is())
    {
        m_xAccess->NotifyChild(nIndex,true, true);
    }
#else
    (void)nIndex;
#endif

    return true;
}

tools::Rectangle SvxPixelCtl::GetFocusRect()
{
    tools::Rectangle aRet;
    //Draw visual focus when has focus
    if (HasFocus())
        aRet = implCalFocusRect(m_aFocusPosition);
    return aRet;
}

// Draws the Control (Rectangle with nine circles)
void SvxPixelCtl::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    if (!m_aRectSize.Width() || !m_aRectSize.Height())
        return;

    sal_uInt16 i, j, nTmp;
    Point aPtTl, aPtBr;

    if (m_bPaintable)
    {
        // Draw lines
        rRenderContext.SetLineColor(Color());
        for (i = 1; i < nLines; i++)
        {
            // horizontal
            nTmp = static_cast<sal_uInt16>(m_aRectSize.Height() * i / nLines);
            rRenderContext.DrawLine(Point(0, nTmp), Point(m_aRectSize.Width(), nTmp));
            // vertically
            nTmp = static_cast<sal_uInt16>( m_aRectSize.Width() * i / nLines );
            rRenderContext.DrawLine(Point(nTmp, 0), Point(nTmp, m_aRectSize.Height()));
        }

        //Draw Rectangles (squares)
        rRenderContext.SetLineColor();
        sal_uInt16 nLastPixel = maPixelData[0] ? 0 : 1;

        for (i = 0; i < nLines; i++)
        {
            aPtTl.setY( m_aRectSize.Height() * i / nLines + 1 );
            aPtBr.setY( m_aRectSize.Height() * (i + 1) / nLines - 1 );

            for (j = 0; j < nLines; j++)
            {
                aPtTl.setX( m_aRectSize.Width() * j / nLines + 1 );
                aPtBr.setX( m_aRectSize.Width() * (j + 1) / nLines - 1 );

                if (maPixelData[i * nLines + j] != nLastPixel)
                {
                    nLastPixel = maPixelData[i * nLines + j];
                    // Change color: 0 -> Background color
                    rRenderContext.SetFillColor(nLastPixel ? m_aPixelColor : m_aBackgroundColor);
                }
                rRenderContext.DrawRect(tools::Rectangle(aPtTl, aPtBr));
            }
        }
    }
    else
    {
        rRenderContext.SetBackground(Wallpaper(COL_LIGHTGRAY));
        rRenderContext.SetLineColor(COL_LIGHTRED);
        rRenderContext.DrawLine(Point(0, 0), Point(m_aRectSize.Width(), m_aRectSize.Height()));
        rRenderContext.DrawLine(Point(0, m_aRectSize.Height()), Point(m_aRectSize.Width(), 0));
    }
}

//Calculate visual focus rectangle via focus position
tools::Rectangle SvxPixelCtl::implCalFocusRect( const Point& aPosition )
{
    tools::Long nLeft,nTop,nRight,nBottom;
    tools::Long i,j;
    i = aPosition.Y();
    j = aPosition.X();
    nLeft = m_aRectSize.Width() * j / nLines + 1;
    nRight = m_aRectSize.Width() * (j + 1) / nLines - 1;
    nTop = m_aRectSize.Height() * i / nLines + 1;
    nBottom = m_aRectSize.Height() * (i + 1) / nLines - 1;
    return tools::Rectangle(nLeft,nTop,nRight,nBottom);
}

//Solution:Keyboard function
bool SvxPixelCtl::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bIsMod = aKeyCode.IsShift() || aKeyCode.IsMod1() || aKeyCode.IsMod2();

    if( !bIsMod )
    {
        Point aRepaintPoint( m_aRectSize.Width() *( m_aFocusPosition.getX() - 1)/ nLines - 1,
                             m_aRectSize.Height() *( m_aFocusPosition.getY() - 1)/ nLines -1
                            );
        Size  aRepaintSize( m_aRectSize.Width() *3/ nLines + 2,m_aRectSize.Height() *3/ nLines + 2);
        tools::Rectangle aRepaintRect( aRepaintPoint, aRepaintSize );
        bool bFocusPosChanged=false;
        switch(nCode)
        {
            case KEY_LEFT:
                if(m_aFocusPosition.getX() >= 1)
                {
                    m_aFocusPosition.setX( m_aFocusPosition.getX() - 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_RIGHT:
                if( m_aFocusPosition.getX() < (nLines - 1) )
                {
                    m_aFocusPosition.setX( m_aFocusPosition.getX() + 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_UP:
                if(m_aFocusPosition.getY() >= 1)
                {
                    m_aFocusPosition.setY( m_aFocusPosition.getY() - 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_DOWN:
                if( m_aFocusPosition.getY() < ( nLines - 1 ) )
                {
                    m_aFocusPosition.setY( m_aFocusPosition.getY() + 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_SPACE:
                ChangePixel( sal_uInt16(m_aFocusPosition.getX() + m_aFocusPosition.getY() * nLines) );
                Invalidate( implCalFocusRect(m_aFocusPosition) );
                break;
            default:
                return CustomWidgetController::KeyInput( rKEvt );
        }
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        if(m_xAccess.is())
        {
            tools::Long nIndex = GetFocusPosIndex();
            switch(nCode)
            {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_UP:
            case KEY_DOWN:
                if (bFocusPosChanged)
                {
                    m_xAccess->NotifyChild(nIndex,false,false);
                }
                break;
            case KEY_SPACE:
                m_xAccess->NotifyChild(nIndex,false,true);
                break;
            default:
                break;
            }
        }
#else
        (void)bFocusPosChanged;
#endif
        return true;
    }
    else
    {
        return CustomWidgetController::KeyInput( rKEvt );
    }
}

//Draw focus when get focus
void SvxPixelCtl::GetFocus()
{
    Invalidate(implCalFocusRect(m_aFocusPosition));

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (m_xAccess.is())
    {
        m_xAccess->NotifyChild(GetFocusPosIndex(),true,false);
    }
#endif
}

void SvxPixelCtl::LoseFocus()
{
    Invalidate();
}

void SvxPixelCtl::SetXBitmap(const Bitmap& rBitmap)
{
    if (vcl::bitmap::isHistorical8x8(rBitmap, m_aBackgroundColor, m_aPixelColor))
    {
        for (sal_uInt16 i = 0; i < nSquares; i++)
        {
            Color aColor = rBitmap.GetPixelColor(i%8, i/8);
            maPixelData[i] = (aColor == m_aBackgroundColor) ? 0 : 1;
        }
    }
}

// Returns a specific pixel

sal_uInt8 SvxPixelCtl::GetBitmapPixel( const sal_uInt16 nPixel ) const
{
    return maPixelData[nPixel];
}

// Resets to the original state of the control

void SvxPixelCtl::Reset()
{
    // clear pixel area
    maPixelData.fill(0);
    Invalidate();
}

SvxLineLB::SvxLineLB(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
    , mbAddStandardFields(true)
{
}

void SvxLineLB::setAddStandardFields(bool bNew)
{
    if(getAddStandardFields() != bNew)
    {
        mbAddStandardFields = bNew;
    }
}

// Fills the listbox (provisional) with strings

void SvxLineLB::Fill( const XDashListRef &pList )
{
    m_xControl->clear();

    if( !pList.is() )
        return;

    ScopedVclPtrInstance< VirtualDevice > pVD;

    if(getAddStandardFields())
    {
        // entry for 'none'
        m_xControl->append_text(pList->GetStringForUiNoLine());

        // entry for solid line
        const Bitmap aBitmap = pList->GetBitmapForUISolidLine();
        const Size aBmpSize(aBitmap.GetSizePixel());
        pVD->SetOutputSizePixel(aBmpSize, false);
        pVD->DrawBitmap(Point(), aBitmap);
        m_xControl->append(u""_ustr, pList->GetStringForUiSolidLine(), *pVD);
    }

    // entries for dashed lines

    tools::Long nCount = pList->Count();
    m_xControl->freeze();

    for( tools::Long i = 0; i < nCount; i++ )
    {
        const XDashEntry* pEntry = pList->GetDash(i);
        const Bitmap aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
        {
            const Size aBmpSize(aBitmap.GetSizePixel());
            pVD->SetOutputSizePixel(aBmpSize, false);
            pVD->DrawBitmap(Point(), aBitmap);
            m_xControl->append(u""_ustr, pEntry->GetName(), *pVD);
        }
        else
        {
            m_xControl->append_text(pEntry->GetName());
        }
    }

    m_xControl->thaw();
}

void SvxLineLB::Append( const XDashEntry& rEntry, const Bitmap& rBitmap )
{
    if (!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;

        const Size aBmpSize(rBitmap.GetSizePixel());
        pVD->SetOutputSizePixel(aBmpSize, false);
        pVD->DrawBitmap(Point(), rBitmap);
        m_xControl->append(u""_ustr, rEntry.GetName(), *pVD);
    }
    else
    {
        m_xControl->append_text(rEntry.GetName());
    }
}

void SvxLineLB::Modify(const XDashEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap)
{
    m_xControl->remove(nPos);

    if (!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;

        const Size aBmpSize(rBitmap.GetSizePixel());
        pVD->SetOutputSizePixel(aBmpSize, false);
        pVD->DrawBitmap(Point(), rBitmap);
        m_xControl->insert(nPos, rEntry.GetName(), nullptr, nullptr, pVD);
    }
    else
    {
        m_xControl->insert_text(nPos, rEntry.GetName());
    }
}

SvxLineEndLB::SvxLineEndLB(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
{
}

void SvxLineEndLB::Fill( const XLineEndListRef &pList, bool bStart )
{
    if( !pList.is() )
        return;

    tools::Long nCount = pList->Count();
    ScopedVclPtrInstance< VirtualDevice > pVD;
    m_xControl->freeze();

    for( tools::Long i = 0; i < nCount; i++ )
    {
        const XLineEndEntry* pEntry = pList->GetLineEnd(i);
        const Bitmap aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
        {
            const Size aBmpSize(aBitmap.GetSizePixel());
            pVD->SetOutputSizePixel(Size(aBmpSize.Width() / 2, aBmpSize.Height()), false);
            pVD->DrawBitmap(bStart ? Point() : Point(-aBmpSize.Width() / 2, 0), aBitmap);
            m_xControl->append(u""_ustr, pEntry->GetName(), *pVD);
        }
        else
            m_xControl->append_text(pEntry->GetName());
    }

    m_xControl->thaw();
}

void SvxLineEndLB::Append( const XLineEndEntry& rEntry, const Bitmap& rBitmap )
{
    if(!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;

        const Size aBmpSize(rBitmap.GetSizePixel());
        pVD->SetOutputSizePixel(Size(aBmpSize.Width() / 2, aBmpSize.Height()), false);
        pVD->DrawBitmap(Point(-aBmpSize.Width() / 2, 0), rBitmap);
        m_xControl->append(u""_ustr, rEntry.GetName(), *pVD);
    }
    else
    {
        m_xControl->append_text(rEntry.GetName());
    }
}

void SvxLineEndLB::Modify( const XLineEndEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap )
{
    m_xControl->remove(nPos);

    if(!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;

        const Size aBmpSize(rBitmap.GetSizePixel());
        pVD->SetOutputSizePixel(Size(aBmpSize.Width() / 2, aBmpSize.Height()), false);
        pVD->DrawBitmap(Point(-aBmpSize.Width() / 2, 0), rBitmap);
        m_xControl->insert(nPos, rEntry.GetName(), nullptr, nullptr, pVD);
    }
    else
    {
        m_xControl->insert_text(nPos, rEntry.GetName());
    }
}

void SvxXLinePreview::Resize()
{
    SvxPreviewBase::Resize();

    const Size aOutputSize(GetOutputSize());
    const sal_Int32 nDistance(500);
    const sal_Int32 nAvailableLength(aOutputSize.Width() - (4 * nDistance));

    // create DrawObjectA
    const sal_Int32 aYPosA(aOutputSize.Height() / 2);
    const basegfx::B2DPoint aPointA1( nDistance,  aYPosA);
    const basegfx::B2DPoint aPointA2( aPointA1.getX() + ((nAvailableLength * 14) / 20), aYPosA );
    basegfx::B2DPolygon aPolygonA;
    aPolygonA.append(aPointA1);
    aPolygonA.append(aPointA2);
    mpLineObjA->SetPathPoly(basegfx::B2DPolyPolygon(aPolygonA));

    // create DrawObjectB
    const sal_Int32 aYPosB1((aOutputSize.Height() * 3) / 4);
    const sal_Int32 aYPosB2((aOutputSize.Height() * 1) / 4);
    const basegfx::B2DPoint aPointB1( aPointA2.getX() + nDistance,  aYPosB1);
    const basegfx::B2DPoint aPointB2( aPointB1.getX() + ((nAvailableLength * 2) / 20), aYPosB2 );
    const basegfx::B2DPoint aPointB3( aPointB2.getX() + ((nAvailableLength * 2) / 20), aYPosB1 );
    basegfx::B2DPolygon aPolygonB;
    aPolygonB.append(aPointB1);
    aPolygonB.append(aPointB2);
    aPolygonB.append(aPointB3);
    mpLineObjB->SetPathPoly(basegfx::B2DPolyPolygon(aPolygonB));

    // create DrawObjectC
    basegfx::B2DPolygon aPolygonC;
    const basegfx::B2DPoint aPointC1( aPointB3.getX() + nDistance,  aYPosB1);
    const basegfx::B2DPoint aPointC2( aPointC1.getX() + ((nAvailableLength * 1) / 20), aYPosB2 );
    const basegfx::B2DPoint aPointC3( aPointC2.getX() + ((nAvailableLength * 1) / 20), aYPosB1 );
    aPolygonC.append(aPointC1);
    aPolygonC.append(aPointC2);
    aPolygonC.append(aPointC3);
    mpLineObjC->SetPathPoly(basegfx::B2DPolyPolygon(aPolygonC));
}

SvxXLinePreview::SvxXLinePreview()
    : mpGraphic(nullptr)
    , mbWithSymbol(false)
{
}

void SvxXLinePreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    SvxPreviewBase::SetDrawingArea(pDrawingArea);

    mpLineObjA = new SdrPathObj(getModel(), SdrObjKind::Line);
    mpLineObjB = new SdrPathObj(getModel(), SdrObjKind::PolyLine);
    mpLineObjC = new SdrPathObj(getModel(), SdrObjKind::PolyLine);

    Resize();
    Invalidate();
}

SvxXLinePreview::~SvxXLinePreview()
{
}

void SvxXLinePreview::SetSymbol(Graphic* p,const Size& s)
{
    mpGraphic = p;
    maSymbolSize = s;
}

void SvxXLinePreview::ResizeSymbol(const Size& s)
{
    if ( s != maSymbolSize )
    {
        maSymbolSize = s;
        Invalidate();
    }
}

void SvxXLinePreview::SetLineAttributes(const SfxItemSet& rItemSet)
{
    // Set ItemSet at objects
    mpLineObjA->SetMergedItemSet(rItemSet);

    // At line joints, do not use arrows
    SfxItemSet aTempSet(rItemSet);
    aTempSet.ClearItem(XATTR_LINESTART);
    aTempSet.ClearItem(XATTR_LINEEND);

    mpLineObjB->SetMergedItemSet(aTempSet);
    mpLineObjC->SetMergedItemSet(aTempSet);
}

void SvxXLinePreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    LocalPrePaint(rRenderContext);

    // paint objects to buffer device
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(mpLineObjA.get());
    aObjectVector.push_back(mpLineObjB.get());
    aObjectVector.push_back(mpLineObjC.get());

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), std::move(aObjectVector), nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    if ( mbWithSymbol && mpGraphic )
    {
        const Size aOutputSize(GetOutputSize());
        Point aPos( aOutputSize.Width() / 3, aOutputSize.Height() / 2 );
        aPos.AdjustX( -(maSymbolSize.Width() / 2) );
        aPos.AdjustY( -(maSymbolSize.Height() / 2) );
        mpGraphic->Draw(getBufferDevice(), aPos, maSymbolSize);
    }

    LocalPostPaint(rRenderContext);
}

SvxXShadowPreview::SvxXShadowPreview()
{
}

void SvxXShadowPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    SvxPreviewBase::SetDrawingArea(pDrawingArea);
    InitSettings();

    // prepare size
    Size aSize = GetPreviewSize().GetSize();
    aSize.setWidth( aSize.Width() / 3 );
    aSize.setHeight( aSize.Height() / 3 );

    // create RectangleObject
    const tools::Rectangle aObjectSize( Point( aSize.Width(), aSize.Height() ), aSize );
    mpRectangleObject = new SdrRectObj(
        getModel(),
        aObjectSize);

    // create ShadowObject
    const tools::Rectangle aShadowSize( Point( aSize.Width(), aSize.Height() ), aSize );
    mpRectangleShadow = new SdrRectObj(
        getModel(),
        aShadowSize);
}

SvxXShadowPreview::~SvxXShadowPreview()
{
}

void SvxXShadowPreview::SetRectangleAttributes(const SfxItemSet& rItemSet)
{
    mpRectangleObject->SetMergedItemSet(rItemSet, true);
    mpRectangleObject->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
}

void SvxXShadowPreview::SetShadowAttributes(const SfxItemSet& rItemSet)
{
    mpRectangleShadow->SetMergedItemSet(rItemSet, true);
    mpRectangleShadow->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
}

void SvxXShadowPreview::SetShadowPosition(const Point& rPos)
{
    maShadowOffset = rPos;
}

void SvxXShadowPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::MAPMODE);
    rRenderContext.SetMapMode(MapMode(MapUnit::Map100thMM));

    LocalPrePaint(rRenderContext);

    // prepare size
    Size aSize = rRenderContext.GetOutputSize();
    aSize.setWidth( aSize.Width() / 3 );
    aSize.setHeight( aSize.Height() / 3 );

    tools::Rectangle aObjectRect(Point(aSize.Width(), aSize.Height()), aSize);
    mpRectangleObject->SetSnapRect(aObjectRect);
    aObjectRect.Move(maShadowOffset.X(), maShadowOffset.Y());
    mpRectangleShadow->SetSnapRect(aObjectRect);

    sdr::contact::SdrObjectVector aObjectVector;

    aObjectVector.push_back(mpRectangleShadow.get());
    aObjectVector.push_back(mpRectangleObject.get());

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), std::move(aObjectVector), nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    LocalPostPaint(rRenderContext);
}

void SvxPreviewBase::InitSettings()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    svtools::ColorConfig aColorConfig;
    Color aTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);
    getBufferDevice().SetTextColor(aTextColor);

    getBufferDevice().SetBackground(rStyleSettings.GetWindowColor());

    getBufferDevice().SetDrawMode(rStyleSettings.GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);

    Invalidate();
}

SvxPreviewBase::SvxPreviewBase()
    : mpModel(new SdrModel(nullptr, nullptr, true))
{
}

void SvxPreviewBase::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(getPreviewStripSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);

    mpBufferDevice = VclPtr<VirtualDevice>::Create(pDrawingArea->get_ref_device());
    mpBufferDevice->SetMapMode(MapMode(MapUnit::Map100thMM));
}

SvxPreviewBase::~SvxPreviewBase()
{
    mpModel.reset();
    mpBufferDevice.disposeAndClear();
}

void SvxPreviewBase::LocalPrePaint(vcl::RenderContext const & rRenderContext)
{
    // init BufferDevice
    if (mpBufferDevice->GetOutputSizePixel() != GetOutputSizePixel())
        mpBufferDevice->SetOutputSizePixel(GetOutputSizePixel());
    mpBufferDevice->SetAntialiasing(rRenderContext.GetAntialiasing());

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if (rStyleSettings.GetPreviewUsesCheckeredBackground())
    {
        const Point aNull(0, 0);
        static const sal_uInt32 nLen(8);
        static const Color aW(COL_WHITE);
        static const Color aG(0xef, 0xef, 0xef);
        const bool bWasEnabled(mpBufferDevice->IsMapModeEnabled());

        mpBufferDevice->EnableMapMode(false);
        mpBufferDevice->DrawCheckered(aNull, mpBufferDevice->GetOutputSizePixel(), nLen, aW, aG);
        mpBufferDevice->EnableMapMode(bWasEnabled);
    }
    else
    {
        mpBufferDevice->Erase();
    }
}

void SvxPreviewBase::LocalPostPaint(vcl::RenderContext& rRenderContext)
{
    // copy to front (in pixel mode)
    const bool bWasEnabledSrc(mpBufferDevice->IsMapModeEnabled());
    const bool bWasEnabledDst(rRenderContext.IsMapModeEnabled());
    const Point aEmptyPoint;

    mpBufferDevice->EnableMapMode(false);
    rRenderContext.EnableMapMode(false);

    rRenderContext.DrawOutDev(aEmptyPoint, GetOutputSizePixel(),
                              aEmptyPoint, GetOutputSizePixel(),
                              *mpBufferDevice);

    mpBufferDevice->EnableMapMode(bWasEnabledSrc);
    rRenderContext.EnableMapMode(bWasEnabledDst);
}

void SvxPreviewBase::StyleUpdated()
{
    InitSettings();
    CustomWidgetController::StyleUpdated();
}

SvxXRectPreview::SvxXRectPreview()
{
}

tools::Rectangle SvxPreviewBase::GetPreviewSize() const
{
    tools::Rectangle aObjectSize(Point(), getBufferDevice().PixelToLogic(GetOutputSizePixel()));
    return aObjectSize;
}

void SvxXRectPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    SvxPreviewBase::SetDrawingArea(pDrawingArea);
    InitSettings();

    // create RectangleObject
    mpRectangleObject = new SdrRectObj(getModel(), GetPreviewSize());
}

void SvxXRectPreview::Resize()
{
    rtl::Reference<SdrObject> pOrigObject = mpRectangleObject;
    if (pOrigObject)
    {
        mpRectangleObject = new SdrRectObj(getModel(), GetPreviewSize());
        SetAttributes(pOrigObject->GetMergedItemSet());
        pOrigObject.clear();
    }
    SvxPreviewBase::Resize();
}

SvxXRectPreview::~SvxXRectPreview()
{
}

void SvxXRectPreview::SetAttributes(const SfxItemSet& rItemSet)
{
    mpRectangleObject->SetMergedItemSet(rItemSet, true);
    mpRectangleObject->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
}

void SvxXRectPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::MAPMODE);
    rRenderContext.SetMapMode(MapMode(MapUnit::Map100thMM));
    LocalPrePaint(rRenderContext);

    sdr::contact::SdrObjectVector aObjectVector;

    aObjectVector.push_back(mpRectangleObject.get());

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), std::move(aObjectVector), nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    LocalPostPaint(rRenderContext);
}

void limitWidthForSidebar(weld::SpinButton& rSpinButton)
{
    // space is limited in the sidebar, so limit MetricSpinButtons to a width of 4 digits
    const int nMaxDigits = 4;
    rSpinButton.set_width_chars(std::min(rSpinButton.get_width_chars(), nMaxDigits));
}

void limitWidthForSidebar(SvxRelativeField& rMetricSpinButton)
{
    weld::SpinButton& rSpinButton = rMetricSpinButton.get_widget();
    limitWidthForSidebar(rSpinButton);
}

void padWidthForSidebar(weld::Toolbar& rToolbar, const css::uno::Reference<css::frame::XFrame>& rFrame)
{
    static int nColumnWidth = -1;
    static vcl::ImageType eSize;
    if (nColumnWidth != -1 && eSize != rToolbar.get_icon_size())
        nColumnWidth = -1;
    if (nColumnWidth == -1)
    {
        // use the, filled-in by dispatcher, width of measurewidth as the width
        // of a "standard" column in a two column panel
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&rToolbar, u"svx/ui/measurewidthbar.ui"_ustr));
        std::unique_ptr<weld::Toolbar> xToolbar1(xBuilder->weld_toolbar(u"measurewidth1"_ustr));
        ToolbarUnoDispatcher aDispatcher1(*xToolbar1, *xBuilder, rFrame);
        std::unique_ptr<weld::Toolbar> xToolbar2(xBuilder->weld_toolbar(u"measurewidth2"_ustr));
        ToolbarUnoDispatcher aDispatcher2(*xToolbar2, *xBuilder, rFrame);
        nColumnWidth = std::max(xToolbar1->get_preferred_size().Width(), xToolbar2->get_preferred_size().Width());
        eSize = rToolbar.get_icon_size();
    }
    rToolbar.set_size_request(nColumnWidth, -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
