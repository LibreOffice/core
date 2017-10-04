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

#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sfx2/dialoghelper.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/strings.hrc>
#include "bitmaps.hlst"
#include <svx/dlgctrl.hxx>
#include <svx/dialmgr.hxx>
#include <tools/poly.hxx>
#include <vcl/region.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include "svxpixelctlaccessiblecontext.hxx"
#include <svtools/colorcfg.hxx>
#include <svxrectctaccessiblecontext.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdopath.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/bitmapaccess.hxx>
#include <svx/xbtmpit.hxx>

#define OUTPUT_DRAWMODE_COLOR       (DrawModeFlags::Default)
#define OUTPUT_DRAWMODE_CONTRAST    (DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

// Control for display and selection of the corner points and
// mid point of an object

BitmapEx& SvxRectCtl::GetRectBitmap()
{
    if( !pBitmap )
        InitRectBitmap();

    return *pBitmap;
}

SvxRectCtl::SvxRectCtl(vcl::Window* pParent, RectPoint eRpt,
    sal_uInt16 nBorder, sal_uInt16 nCircle)
    : Control(pParent, WB_BORDER | WB_TABSTOP)
    , nBorderWidth(nBorder)
    , nRadius(nCircle)
    , eDefRP(eRpt)
    , pBitmap(nullptr)
    , m_nState(CTL_STATE::NONE)
    , mbCompleteDisable(false)
    , mbUpdateForeground(true)
    , mbUpdateBackground(true)
{
    SetMapMode(MapUnit::Map100thMM);
    Resize_Impl();
}

void SvxRectCtl::SetControlSettings(RectPoint eRpt, sal_uInt16 nBorder, sal_uInt16 nCircle)
{
    nBorderWidth = nBorder;
    nRadius = nCircle;
    eDefRP = eRpt;
    Resize_Impl();
}

Size SvxRectCtl::GetOptimalSize() const
{
    return LogicToPixel(Size(78, 39), MapUnit::MapAppFont);
}

VCL_BUILDER_FACTORY(SvxRectCtl)

SvxRectCtl::~SvxRectCtl()
{
    disposeOnce();
}

void SvxRectCtl::dispose()
{
    delete pBitmap;

    pAccContext.clear();
    Control::dispose();
}


void SvxRectCtl::Resize()
{
    Resize_Impl();
    Control::Resize();
}


void SvxRectCtl::Resize_Impl()
{
    aSize = GetOutputSize();

    aPtLT = Point( 0 + nBorderWidth,  0 + nBorderWidth );
    aPtMT = Point( aSize.Width() / 2, 0 + nBorderWidth );
    aPtRT = Point( aSize.Width() - nBorderWidth, 0 + nBorderWidth );

    aPtLM = Point( 0 + nBorderWidth,  aSize.Height() / 2 );
    aPtMM = Point( aSize.Width() / 2, aSize.Height() / 2 );
    aPtRM = Point( aSize.Width() - nBorderWidth, aSize.Height() / 2 );

    aPtLB = Point( 0 + nBorderWidth,    aSize.Height() - nBorderWidth );
    aPtMB = Point( aSize.Width() / 2,   aSize.Height() - nBorderWidth );
    aPtRB = Point( aSize.Width() - nBorderWidth, aSize.Height() - nBorderWidth );

    Reset();
    MarkToResetSettings(true, true);
    Invalidate();
}

void SvxRectCtl::InitRectBitmap()
{
    delete pBitmap;

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;

    pBitmap = new BitmapEx(RID_SVXCTRL_RECTBTNS);

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
    aColorAry2[5] = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
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

    pBitmap->Replace( aColorAry1, aColorAry2, 7 );
}


void SvxRectCtl::MarkToResetSettings(bool bUpdateForeground, bool bUpdateBackground)
{
    mbUpdateForeground = bUpdateForeground;
    mbUpdateBackground = bUpdateBackground;
    delete pBitmap;
    pBitmap = nullptr; // forces new creating of bitmap
}

void SvxRectCtl::InitSettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if (mbUpdateForeground)
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);

        if (IsControlForeground())
            aTextColor = GetControlForeground();
        rRenderContext.SetTextColor(aTextColor);
        mbUpdateForeground = false;
    }

    if (mbUpdateBackground)
    {
        if (IsControlBackground())
            SetBackground(GetControlBackground());
        else
            SetBackground(rStyleSettings.GetWindowColor());
        mbUpdateBackground = false;
    }
}

// The clicked rectangle (3 x 3) is determined and the parent (dialog)
// is notified that the item was changed
void SvxRectCtl::MouseButtonDown( const MouseEvent& rMEvt )
{
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        Point aPtLast = aPtNew;

        aPtNew = GetApproxLogPtFromPixPt( rMEvt.GetPosPixel() );

        Invalidate( tools::Rectangle( aPtLast - Point( nRadius, nRadius ),
                               aPtLast + Point( nRadius, nRadius ) ) );
        Invalidate( tools::Rectangle( aPtNew - Point( nRadius, nRadius ),
                               aPtNew + Point( nRadius, nRadius ) ) );
        eRP = GetRPFromPoint( aPtNew );

        SetActualRP( eRP );

        vcl::Window *pTabPage = getNonLayoutParent(this);
        if (pTabPage && WindowType::TABPAGE == pTabPage->GetType())
            static_cast<SvxTabPage*>(pTabPage)->PointChanged( this, eRP );
    }
}

void SvxRectCtl::KeyInput( const KeyEvent& rKeyEvt )
{
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if(!IsCompletelyDisabled())
    {
        RectPoint eNewRP = eRP;

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
                Control::KeyInput( rKeyEvt );
                return;
        }
        if( eNewRP != eRP )
        {
            SetActualRP( eNewRP );

            vcl::Window *pTabPage = getNonLayoutParent(this);
            if (pTabPage && WindowType::TABPAGE == pTabPage->GetType())
                static_cast<SvxTabPage*>(pTabPage)->PointChanged(this, eRP);

            SetFocusRect();
        }
    }
}


void SvxRectCtl::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::ControlForeground )
        MarkToResetSettings(true, false);
    else if ( nType == StateChangedType::ControlBackground )
        MarkToResetSettings(false, true);

    Window::StateChanged( nType );
}


void SvxRectCtl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        MarkToResetSettings(true, true);
    else
        Window::DataChanged( rDCEvt );
}

// the control (rectangle with 9 circles)

void SvxRectCtl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    InitSettings(rRenderContext);

    Point aPtDiff(PixelToLogic(Point(1, 1)));

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
        rRenderContext.DrawRect(tools::Rectangle(aPtLT + aPtDiff, aPtRB + aPtDiff));
        rRenderContext.SetLineColor(aOldCol);
    }
    rRenderContext.DrawRect(tools::Rectangle(aPtLT, aPtRB));

    rRenderContext.SetFillColor(rRenderContext.GetBackground().GetColor());

    Size aBtnSize(11, 11);
    Size aDstBtnSize(PixelToLogic(aBtnSize));
    Point aToCenter(aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1);
    Point aBtnPnt1(IsEnabled() ? 0 : 22, 0);
    Point aBtnPnt2(11, 0);
    Point aBtnPnt3(22, 0);

    bool bNoHorz = bool(m_nState & CTL_STATE::NOHORZ);
    bool bNoVert = bool(m_nState & CTL_STATE::NOVERT);

    BitmapEx& rBitmap = GetRectBitmap();

    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if (IsCompletelyDisabled())
    {
        rRenderContext.DrawBitmap(aPtLT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtMT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtRT - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtLM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtMM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtRM - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtLB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtMB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtRB - aToCenter, aDstBtnSize, aBtnPnt3, aBtnSize, rBitmap.GetBitmap());
    }
    else
    {
        rRenderContext.DrawBitmap(aPtLT - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtMT - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtRT - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtLM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());

        // Center for rectangle and line
        rRenderContext.DrawBitmap(aPtMM - aToCenter, aDstBtnSize, aBtnPnt1, aBtnSize, rBitmap.GetBitmap());

        rRenderContext.DrawBitmap(aPtRM - aToCenter, aDstBtnSize, bNoHorz?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtLB - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtMB - aToCenter, aDstBtnSize, bNoVert?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
        rRenderContext.DrawBitmap(aPtRB - aToCenter, aDstBtnSize, (bNoHorz || bNoVert)?aBtnPnt3:aBtnPnt1, aBtnSize, rBitmap.GetBitmap());
    }

    // draw active button, avoid center pos for angle
    // CompletelyDisabled() added to have a disabled state for SvxRectCtl
    if (!IsCompletelyDisabled())
    {
        if (IsEnabled())
        {
            Point aCenterPt(aPtNew);
            aCenterPt -= aToCenter;

            rRenderContext.DrawBitmap(aCenterPt, aDstBtnSize, aBtnPnt2, aBtnSize, rBitmap.GetBitmap());
        }
    }
}

// Convert RectPoint Point

const Point& SvxRectCtl::GetPointFromRP( RectPoint _eRP) const
{
    switch( _eRP )
    {
        case RectPoint::LT: return aPtLT;
        case RectPoint::MT: return aPtMT;
        case RectPoint::RT: return aPtRT;
        case RectPoint::LM: return aPtLM;
        case RectPoint::MM: return aPtMM;
        case RectPoint::RM: return aPtRM;
        case RectPoint::LB: return aPtLB;
        case RectPoint::MB: return aPtMB;
        case RectPoint::RB: return aPtRB;
    }
    return aPtMM; // default
}


void SvxRectCtl::SetFocusRect()
{
    HideFocus();

    ShowFocus( CalculateFocusRectangle() );
}

Point SvxRectCtl::SetActualRPWithoutInvalidate( RectPoint eNewRP )
{
    Point aPtLast = aPtNew;
    aPtNew = GetPointFromRP( eNewRP );

    if( m_nState & CTL_STATE::NOHORZ )
        aPtNew.X() = aPtMM.X();

    if( m_nState & CTL_STATE::NOVERT )
        aPtNew.Y() = aPtMM.Y();

    // fdo#74751 this fix reverse base point on RTL UI.
    bool bRTL = AllSettings::GetLayoutRTL();
    eNewRP = GetRPFromPoint( aPtNew, bRTL );

    eDefRP = eNewRP;
    eRP = eNewRP;

    return aPtLast;
}

void SvxRectCtl::GetFocus()
{
    SetFocusRect();
    // Send the accessible focused event
    Control::GetFocus();
    // Send accessibility event.
    if(pAccContext.is())
    {
        pAccContext->FireChildFocus(GetActualRP());
    }
}


void SvxRectCtl::LoseFocus()
{
    HideFocus();
}


Point SvxRectCtl::GetApproxLogPtFromPixPt( const Point& rPt ) const
{
    Point   aPt = PixelToLogic( rPt );
    long    x;
    long    y;

    if( !( m_nState & CTL_STATE::NOHORZ ) )
    {
        if( aPt.X() < aSize.Width() / 3 )
            x = aPtLT.X();
        else if( aPt.X() < aSize.Width() * 2 / 3 )
            x = aPtMM.X();
        else
            x = aPtRB.X();
    }
    else
        x = aPtMM.X();

    if( !( m_nState & CTL_STATE::NOVERT ) )
    {
        if( aPt.Y() < aSize.Height() / 3 )
            y = aPtLT.Y();
        else if( aPt.Y() < aSize.Height() * 2 / 3 )
            y = aPtMM.Y();
        else
            y = aPtRB.Y();
    }
    else
            y = aPtMM.Y();

    return Point( x, y );
}


// Converts Point in RectPoint

RectPoint SvxRectCtl::GetRPFromPoint( Point aPt, bool bRTL ) const
{
    RectPoint rPoint = RectPoint::MM;  // default

    if     ( aPt == aPtLT) rPoint = bRTL ? RectPoint::RT : RectPoint::LT;
    else if( aPt == aPtMT) rPoint = RectPoint::MT;
    else if( aPt == aPtRT) rPoint = bRTL ? RectPoint::LT : RectPoint::RT;
    else if( aPt == aPtLM) rPoint = bRTL ? RectPoint::RM : RectPoint::LM;
    else if( aPt == aPtRM) rPoint = bRTL ? RectPoint::LM : RectPoint::RM;
    else if( aPt == aPtLB) rPoint = bRTL ? RectPoint::RB : RectPoint::LB;
    else if( aPt == aPtMB) rPoint = RectPoint::MB;
    else if( aPt == aPtRB) rPoint = bRTL ? RectPoint::LB : RectPoint::RB;

    return rPoint;
}

// Resets to the original state of the control

void SvxRectCtl::Reset()
{
    aPtNew = GetPointFromRP( eDefRP );
    eRP = eDefRP;
    Invalidate();
}

// Returns the currently selected RectPoint


void SvxRectCtl::SetActualRP( RectPoint eNewRP )
{
    Point aPtLast( SetActualRPWithoutInvalidate( eNewRP ) );

    Invalidate( tools::Rectangle( aPtLast - Point( nRadius, nRadius ), aPtLast + Point( nRadius, nRadius ) ) );
    Invalidate( tools::Rectangle( aPtNew - Point( nRadius, nRadius ), aPtNew + Point( nRadius, nRadius ) ) );

    // notify accessibility object about change
    if( pAccContext.is() )
        pAccContext->selectChild( eNewRP /* MT, bFireFocus */ );
}

void SvxRectCtl::SetState( CTL_STATE nState )
{
    m_nState = nState;

    Point aPtLast( GetPointFromRP( eRP ) );
    Point _aPtNew( aPtLast );

    if( m_nState & CTL_STATE::NOHORZ )
        _aPtNew.X() = aPtMM.X();

    if( m_nState & CTL_STATE::NOVERT)
        _aPtNew.Y() = aPtMM.Y();

    eRP = GetRPFromPoint( _aPtNew );
    Invalidate();

    vcl::Window *pTabPage = getNonLayoutParent(this);
    if (pTabPage && WindowType::TABPAGE == pTabPage->GetType())
        static_cast<SvxTabPage*>(pTabPage)->PointChanged(this, eRP);
}

tools::Rectangle SvxRectCtl::CalculateFocusRectangle() const
{
    Size        aDstBtnSize( PixelToLogic( Size( 15, 15 ) ) );
    return tools::Rectangle( aPtNew - Point( aDstBtnSize.Width() >> 1, aDstBtnSize.Height() >> 1 ), aDstBtnSize );
}

tools::Rectangle SvxRectCtl::CalculateFocusRectangle( RectPoint eRectPoint ) const
{
    tools::Rectangle   aRet;
    RectPoint  eOldRectPoint = GetActualRP();

    if( eOldRectPoint == eRectPoint )
        aRet = CalculateFocusRectangle();
    else
    {
        SvxRectCtl* pThis = const_cast< SvxRectCtl* >( this );

        pThis->SetActualRPWithoutInvalidate( eRectPoint );      // no invalidation because it's only temporary!
        aRet = CalculateFocusRectangle();

        pThis->SetActualRPWithoutInvalidate( eOldRectPoint );   // no invalidation because nothing has changed!
    }

    return aRet;
}

Reference< XAccessible > SvxRectCtl::CreateAccessible()
{
    vcl::Window*                     pParent = GetAccessibleParentWindow();

    DBG_ASSERT( pParent, "-SvxRectCtl::CreateAccessible(): No Parent!" );

    Reference< XAccessible >    xAccParent  = pParent->GetAccessible();
    if( xAccParent.is() )
    {
        pAccContext = new SvxRectCtlAccessibleContext( xAccParent, *this );

        SetActualRP( GetActualRP() );

        return pAccContext.get();
    }
    else
        return Reference< XAccessible >();
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

// Control for editing bitmaps

css::uno::Reference< css::accessibility::XAccessible > SvxPixelCtl::CreateAccessible()
{
    if(!m_xAccess.is())
    {
        m_xAccess = new SvxPixelCtlAccessible(*this);
    }
    return m_xAccess.get();
}

//Logic Pixel
long SvxPixelCtl::PointToIndex(const Point &aPt) const
{
    long nX = aPt.X() * nLines / aRectSize.Width();
    long nY = aPt.Y() * nLines / aRectSize.Height();

    return nX + nY * nLines ;
}

Point SvxPixelCtl::IndexToPoint(long nIndex) const
{
    DBG_ASSERT(nIndex >= 0 && nIndex < nSquares ," Check Index");

    sal_Int32 nXIndex = nIndex % nLines;
    sal_Int32 nYIndex = nIndex / nLines;

    Point aPtTl;
    aPtTl.Y() = aRectSize.Height() * nYIndex / nLines + 1;
    aPtTl.X() = aRectSize.Width() * nXIndex / nLines + 1;

    return aPtTl;
}

long SvxPixelCtl::GetFocusPosIndex() const
{
    return aFocusPosition.getX() + aFocusPosition.getY() * nLines ;
}

long SvxPixelCtl::ShowPosition( const Point &pt)
{
    Point aPt = PixelToLogic( pt );

    sal_Int32 nX = aPt.X() * nLines / aRectSize.Width();
    sal_Int32 nY = aPt.Y() * nLines / aRectSize.Height();

    ChangePixel( nX + nY * nLines );

    //Solution:Set new focus position and repaint
    //Invalidate( Rectangle( aPtTl, aPtBr ) );
    aFocusPosition.setX(nX);
    aFocusPosition.setY(nY);
    Invalidate(tools::Rectangle(Point(0,0),aRectSize));

    vcl::Window *pTabPage = getNonLayoutParent(this);
    if (pTabPage && WindowType::TABPAGE == pTabPage->GetType())
        static_cast<SvxTabPage*>(pTabPage)->PointChanged( this, RectPoint::MM ); // RectPoint is dummy

    return GetFocusPosIndex();

}

SvxPixelCtl::SvxPixelCtl(vcl::Window* pParent, sal_uInt16 nNumber)
    : Control(pParent, WB_BORDER)
    , nLines(nNumber)
    , bPaintable(true)
    , aFocusPosition(0,0)
{
    assert(nLines); // can't have no lines
    SetPixelColor( Color( COL_BLACK ) );
    SetBackgroundColor( Color( COL_WHITE ) );
    SetLineColor( Color( COL_LIGHTGRAY ) );

    nSquares = nLines * nLines;
    pPixel = new sal_uInt16[ nSquares ];
    memset(pPixel, 0, nSquares * sizeof(sal_uInt16));
}

void SvxPixelCtl::Resize()
{
    Control::Resize();
    aRectSize = GetOutputSize();
}

Size SvxPixelCtl::GetOptimalSize() const
{
    return LogicToPixel(Size(72, 72), MapUnit::MapAppFont);
}

VCL_BUILDER_FACTORY_ARGS(SvxPixelCtl, 8)

SvxPixelCtl::~SvxPixelCtl( )
{
    disposeOnce();
}

void SvxPixelCtl::dispose()
{
    delete []pPixel;
    Control::dispose();
}

// Changes the foreground or Background color

void SvxPixelCtl::ChangePixel( sal_uInt16 nPixel )
{
    if( *( pPixel + nPixel) == 0 )
        *( pPixel + nPixel) = 1; //  could be extended to more colors
    else
        *( pPixel + nPixel) = 0;
}

// The clicked rectangle is identified, to change its color

void SvxPixelCtl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!aRectSize.Width() || !aRectSize.Height())
        return;

    //Grab focus when click in window
    if (!HasFocus())
    {
        GrabFocus();
    }

    long nIndex = ShowPosition(rMEvt.GetPosPixel());

    if(m_xAccess.is())
    {
        m_xAccess->NotifyChild(nIndex,true, true);
    }
}

// Draws the Control (Rectangle with nine circles)

void SvxPixelCtl::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    if (!aRectSize.Width() || !aRectSize.Height())
        return;

    sal_uInt16 i, j, nTmp;
    Point aPtTl, aPtBr;

    if (bPaintable)
    {
        // Draw lines
        rRenderContext.SetLineColor(Color());
        for (i = 1; i < nLines; i++)
        {
            // horizontal
            nTmp = (sal_uInt16) (aRectSize.Height() * i / nLines);
            rRenderContext.DrawLine(Point(0, nTmp), Point(aRectSize.Width(), nTmp));
            // vertically
            nTmp = (sal_uInt16) ( aRectSize.Width() * i / nLines );
            rRenderContext.DrawLine(Point(nTmp, 0), Point(nTmp, aRectSize.Height()));
        }

        //Draw Rectangles (squares)
        rRenderContext.SetLineColor();
        sal_uInt16 nLastPixel = *pPixel ? 0 : 1;

        for (i = 0; i < nLines; i++)
        {
            aPtTl.Y() = aRectSize.Height() * i / nLines + 1;
            aPtBr.Y() = aRectSize.Height() * (i + 1) / nLines - 1;

            for (j = 0; j < nLines; j++)
            {
                aPtTl.X() = aRectSize.Width() * j / nLines + 1;
                aPtBr.X() = aRectSize.Width() * (j + 1) / nLines - 1;

                if (*(pPixel + i * nLines + j) != nLastPixel)
                {
                    nLastPixel = *(pPixel + i * nLines + j);
                    // Change color: 0 -> Background color
                    rRenderContext.SetFillColor(nLastPixel ? aPixelColor : aBackgroundColor);
                }
                rRenderContext.DrawRect(tools::Rectangle(aPtTl, aPtBr));
            }
        }
        //Draw visual focus when has focus
        if (HasFocus())
        {
            ShowFocus(implCalFocusRect(aFocusPosition));
        }
    }
    else
    {
        rRenderContext.SetBackground(Wallpaper(Color(COL_LIGHTGRAY)));
        rRenderContext.SetLineColor(Color(COL_LIGHTRED));
        rRenderContext.DrawLine(Point(0, 0), Point(aRectSize.Width(), aRectSize.Height()));
        rRenderContext.DrawLine(Point(0, aRectSize.Height()), Point(aRectSize.Width(), 0));
    }
}

//Calculate visual focus rectangle via focus position
tools::Rectangle SvxPixelCtl::implCalFocusRect( const Point& aPosition )
{
    long nLeft,nTop,nRight,nBottom;
    long i,j;
    i = aPosition.Y();
    j = aPosition.X();
    nLeft = aRectSize.Width() * j / nLines + 1;
    nRight = aRectSize.Width() * (j + 1) / nLines - 1;
    nTop = aRectSize.Height() * i / nLines + 1;
    nBottom = aRectSize.Height() * (i + 1) / nLines - 1;
    return tools::Rectangle(nLeft,nTop,nRight,nBottom);
}

//Solution:Keyboard function
void SvxPixelCtl::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bIsMod = aKeyCode.IsShift() || aKeyCode.IsMod1() || aKeyCode.IsMod2();

    if( !bIsMod )
    {
        Point aRepaintPoint( aRectSize.Width() *( aFocusPosition.getX() - 1)/ nLines - 1,
                             aRectSize.Height() *( aFocusPosition.getY() - 1)/ nLines -1
                            );
        Size  aRepaintSize( aRectSize.Width() *3/ nLines + 2,aRectSize.Height() *3/ nLines + 2);
        tools::Rectangle aRepaintRect( aRepaintPoint, aRepaintSize );
        bool bFocusPosChanged=false;
        switch(nCode)
        {
            case KEY_LEFT:
                if(aFocusPosition.getX() >= 1)
                {
                    aFocusPosition.setX( aFocusPosition.getX() - 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_RIGHT:
                if( aFocusPosition.getX() < (nLines - 1) )
                {
                    aFocusPosition.setX( aFocusPosition.getX() + 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_UP:
                if(aFocusPosition.getY() >= 1)
                {
                    aFocusPosition.setY( aFocusPosition.getY() - 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_DOWN:
                if( aFocusPosition.getY() < ( nLines - 1 ) )
                {
                    aFocusPosition.setY( aFocusPosition.getY() + 1 );
                    Invalidate(aRepaintRect);
                    bFocusPosChanged=true;
                }
                break;
            case KEY_SPACE:
                ChangePixel( sal_uInt16(aFocusPosition.getX() + aFocusPosition.getY() * nLines) );
                Invalidate( implCalFocusRect(aFocusPosition) );
                break;
            default:
                Control::KeyInput( rKEvt );
                return;
        }
        if(m_xAccess.is())
        {
            long nIndex = GetFocusPosIndex();
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
    }
    else
    {
        Control::KeyInput( rKEvt );
    }
}

//Draw focus when get focus
void SvxPixelCtl::GetFocus()
{
    Invalidate(implCalFocusRect(aFocusPosition));

    if(m_xAccess.is())
    {
        m_xAccess->NotifyChild(GetFocusPosIndex(),true,false);
    }

    Control::GetFocus();
}

//Hide focus when lose focus
void SvxPixelCtl::LoseFocus()
{
    HideFocus();
    if (m_xAccess.is())
    {
        m_xAccess->LoseFocus();
    }
    Control::LoseFocus();
}

void SvxPixelCtl::SetXBitmap( const BitmapEx& rBitmapEx )
{
    BitmapColor aBack;
    BitmapColor aFront;

    if(isHistorical8x8(rBitmapEx, aBack, aFront))
    {
        Bitmap aBitmap(rBitmapEx.GetBitmap());
        Bitmap::ScopedReadAccess pRead(aBitmap);

        aBackgroundColor = aBack;
        aPixelColor = aFront;

        for(sal_uInt16 i(0); i < nSquares; i++)
        {
            const BitmapColor aColor(pRead->GetColor(i/8, i%8));

            if(aColor == aBack)
            {
                *( pPixel + i ) = 0;
            }
            else
            {
                *( pPixel + i ) = 1;
            }
        }
    }
}

// Returns a specific pixel

sal_uInt16 SvxPixelCtl::GetBitmapPixel( const sal_uInt16 nPixel )
{
    return *( pPixel + nPixel );
}

// Resets to the original state of the control

void SvxPixelCtl::Reset()
{
    // clear pixel area
    memset(pPixel, 0, nSquares * sizeof(sal_uInt16));
    Invalidate();
}

// Fills the listbox (provisional) with strings

HatchingLB::HatchingLB( vcl::Window* pParent, WinBits nWinStyle)
: ListBox( pParent, nWinStyle )
{
    SetEdgeBlending(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeHatchingLB(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinStyle = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    VclPtrInstance<HatchingLB> pListBox(pParent, nWinStyle);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

// Fills the listbox (provisional) with strings

GradientLB::GradientLB( vcl::Window* pParent, WinBits aWB)
: ListBox( pParent, aWB )
{
    SetEdgeBlending(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeGradientLB(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinStyle = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    VclPtrInstance<GradientLB> pListBox(pParent, nWinStyle);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

// BitmapLB Constructor

BitmapLB::BitmapLB( vcl::Window* pParent, WinBits aWB)
:   ListBox( pParent, aWB )
{
    SetEdgeBlending(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeBitmapLB(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinStyle = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    VclPtrInstance<BitmapLB> pListBox(pParent, nWinStyle);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void FillTypeLB::Fill()
{
    SetUpdateMode( false );

    InsertEntry( SvxResId(RID_SVXSTR_INVISIBLE) );
    InsertEntry( SvxResId(RID_SVXSTR_COLOR) );
    InsertEntry( SvxResId(RID_SVXSTR_GRADIENT) );
    InsertEntry( SvxResId(RID_SVXSTR_HATCH) );
    InsertEntry( SvxResId(RID_SVXSTR_BITMAP) );
    InsertEntry( SvxResId(RID_SVXSTR_PATTERN) );

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

LineLB::LineLB(vcl::Window* pParent, WinBits aWB)
:   ListBox(pParent, aWB),
    mbAddStandardFields(true)
{
    // No EdgeBlending for LineStyle/Dash SetEdgeBlending(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeLineLB(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinBits |= WB_BORDER;
    VclPtrInstance<LineLB> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void LineLB::setAddStandardFields(bool bNew)
{
    if(getAddStandardFields() != bNew)
    {
        mbAddStandardFields = bNew;
    }
}

// Fills the listbox (provisional) with strings

void LineLB::Fill( const XDashListRef &pList )
{
    Clear();

    if( !pList.is() )
        return;

    if(getAddStandardFields())
    {
        // entry for 'none'
        InsertEntry(pList->GetStringForUiNoLine());

        // entry for solid line
        InsertEntry(pList->GetStringForUiSolidLine(),
                Image(pList->GetBitmapForUISolidLine()));
    }

    // entries for dashed lines

    long nCount = pList->Count();
    SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XDashEntry* pEntry = pList->GetDash(i);
        const Bitmap aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
        {
            InsertEntry(pEntry->GetName(), Image(aBitmap));
        }
        else
            InsertEntry( pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

void LineLB::Append( const XDashEntry& rEntry, const Bitmap& rBitmap )
{
    if(!rBitmap.IsEmpty())
    {
        InsertEntry(rEntry.GetName(), Image(rBitmap));
    }
    else
    {
        InsertEntry( rEntry.GetName() );
    }

    AdaptDropDownLineCountToMaximum();
}

void LineLB::Modify( const XDashEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap )
{
    RemoveEntry( nPos );

    if(!rBitmap.IsEmpty())
    {
        InsertEntry( rEntry.GetName(), Image(rBitmap), nPos );
    }
    else
    {
        InsertEntry( rEntry.GetName(), nPos );
    }
}

// Fills the listbox (provisional) with strings

LineEndLB::LineEndLB( vcl::Window* pParent, WinBits aWB )
    : ListBox( pParent, aWB )
{
    // No EdgeBlending for LineEnds SetEdgeBlending(true);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeLineEndLB(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinBits |= WB_BORDER;
    VclPtrInstance<LineEndLB> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

void LineEndLB::Fill( const XLineEndListRef &pList, bool bStart )
{
    if( !pList.is() )
        return;

    long nCount = pList->Count();
    ScopedVclPtrInstance< VirtualDevice > pVD;
    SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XLineEndEntry* pEntry = pList->GetLineEnd(i);
        const Bitmap aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
        {
            Size aBmpSize( aBitmap.GetSizePixel() );
            pVD->SetOutputSizePixel( aBmpSize, false );
            pVD->DrawBitmap( Point(), aBitmap );
            InsertEntry( pEntry->GetName(),
                Image(pVD->GetBitmap(
                    (bStart) ? Point() : Point(aBmpSize.Width() / 2, 0),
                    Size(aBmpSize.Width() / 2, aBmpSize.Height()))));
        }
        else
            InsertEntry( pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

void LineEndLB::Append( const XLineEndEntry& rEntry, const Bitmap& rBitmap )
{
    if(!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;
        const Size aBmpSize(rBitmap.GetSizePixel());

        pVD->SetOutputSizePixel(aBmpSize, false);
        pVD->DrawBitmap(Point(), rBitmap);
        InsertEntry(
            rEntry.GetName(),
            Image(pVD->GetBitmap(
                Point(),
                Size(aBmpSize.Width() / 2, aBmpSize.Height()))));
    }
    else
    {
        InsertEntry(rEntry.GetName());
    }

    AdaptDropDownLineCountToMaximum();
}

void LineEndLB::Modify( const XLineEndEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap )
{
    RemoveEntry( nPos );

    if(!rBitmap.IsEmpty())
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;
        const Size aBmpSize(rBitmap.GetSizePixel());

        pVD->SetOutputSizePixel(aBmpSize, false);
        pVD->DrawBitmap(Point(), rBitmap);
        InsertEntry(
            rEntry.GetName(),
            Image(pVD->GetBitmap(
                    Point(),
                    Size(aBmpSize.Width() / 2, aBmpSize.Height()))),
            nPos);
    }
    else
    {
        InsertEntry(rEntry.GetName(), nPos);
    }
}


void SvxPreviewBase::InitSettings(bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if(bForeground)
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);

        if(IsControlForeground())
        {
            aTextColor = GetControlForeground();
        }

        getBufferDevice().SetTextColor(aTextColor);
    }

    if(bBackground)
    {
        if(IsControlBackground())
        {
            getBufferDevice().SetBackground(GetControlBackground());
        }
        else
        {
            getBufferDevice().SetBackground(rStyleSettings.GetWindowColor());
        }
    }

    // do not paint background self, it gets painted buffered
    SetControlBackground();
    SetBackground();

    Invalidate();
}

SvxPreviewBase::SvxPreviewBase(vcl::Window* pParent)
    : Control(pParent, WB_BORDER)
    , mpModel(new SdrModel())
    , mpBufferDevice(VclPtr<VirtualDevice>::Create(*this))
{
    //  Draw the control's border as a flat thin black line.
    SetBorderStyle(WindowBorderStyle::MONO);
    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
    SetMapMode(MapUnit::Map100thMM);

    // init model
    mpModel->GetItemPool().FreezeIdRanges();
}

SvxPreviewBase::~SvxPreviewBase()
{
    disposeOnce();
}

void SvxPreviewBase::dispose()
{
    delete mpModel;
    mpBufferDevice.disposeAndClear();
    Control::dispose();
}

void SvxPreviewBase::LocalPrePaint(vcl::RenderContext const & rRenderContext)
{
    // init BufferDevice
    if (mpBufferDevice->GetOutputSizePixel() != GetOutputSizePixel())
    {
        mpBufferDevice->SetDrawMode(rRenderContext.GetDrawMode());
        mpBufferDevice->SetSettings(rRenderContext.GetSettings());
        mpBufferDevice->SetAntialiasing(rRenderContext.GetAntialiasing());
        mpBufferDevice->SetOutputSizePixel(GetOutputSizePixel());
        mpBufferDevice->SetMapMode(rRenderContext.GetMapMode());
    }

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
    const bool bWasEnabledDst(IsMapModeEnabled());
    const Point aEmptyPoint;

    mpBufferDevice->EnableMapMode(false);
    rRenderContext.EnableMapMode(false);

    rRenderContext.DrawOutDev(aEmptyPoint, GetOutputSizePixel(),
                              aEmptyPoint, GetOutputSizePixel(),
                              *mpBufferDevice);

    mpBufferDevice->EnableMapMode(bWasEnabledSrc);
    rRenderContext.EnableMapMode(bWasEnabledDst);
}

void SvxPreviewBase::StateChanged(StateChangedType nType)
{
    Control::StateChanged(nType);

    if(StateChangedType::ControlForeground == nType)
    {
        InitSettings(true, false);
    }
    else if(StateChangedType::ControlBackground == nType)
    {
        InitSettings(false, true);
    }
}

void SvxPreviewBase::DataChanged(const DataChangedEvent& rDCEvt)
{
    SetDrawMode(GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);

    if((DataChangedEventType::SETTINGS == rDCEvt.GetType()) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        InitSettings(true, true);
    }
    else
    {
        Control::DataChanged(rDCEvt);
    }
}

void SvxXLinePreview::Resize()
{
    SvxPreviewBase::Resize();

    const Size aOutputSize(GetOutputSize());
    const sal_Int32 nDistance(500);
    const sal_Int32 nAvailableLength(aOutputSize.Width() - (4 * nDistance));

    // create DrawObectA
    const sal_Int32 aYPosA(aOutputSize.Height() / 2);
    const basegfx::B2DPoint aPointA1( nDistance,  aYPosA);
    const basegfx::B2DPoint aPointA2( aPointA1.getX() + ((nAvailableLength * 14) / 20), aYPosA );
    basegfx::B2DPolygon aPolygonA;
    aPolygonA.append(aPointA1);
    aPolygonA.append(aPointA2);
    mpLineObjA->SetPathPoly(basegfx::B2DPolyPolygon(aPolygonA));

    // create DrawObectB
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

    // create DrawObectC
    basegfx::B2DPolygon aPolygonC;
    const basegfx::B2DPoint aPointC1( aPointB3.getX() + nDistance,  aYPosB1);
    const basegfx::B2DPoint aPointC2( aPointC1.getX() + ((nAvailableLength * 1) / 20), aYPosB2 );
    const basegfx::B2DPoint aPointC3( aPointC2.getX() + ((nAvailableLength * 1) / 20), aYPosB1 );
    aPolygonC.append(aPointC1);
    aPolygonC.append(aPointC2);
    aPolygonC.append(aPointC3);
    mpLineObjC->SetPathPoly(basegfx::B2DPolyPolygon(aPolygonC));
}

SvxXLinePreview::SvxXLinePreview(vcl::Window* pParent)
    : SvxPreviewBase(pParent)
    , mpLineObjA(nullptr)
    , mpLineObjB(nullptr)
    , mpLineObjC(nullptr)
    , mpGraphic(nullptr)
    , mbWithSymbol(false)
{
    InitSettings( true, true );

    mpLineObjA = new SdrPathObj(OBJ_LINE);
    mpLineObjA->SetModel(&getModel());

    mpLineObjB = new SdrPathObj(OBJ_PLIN);
    mpLineObjB->SetModel(&getModel());

    mpLineObjC = new SdrPathObj(OBJ_PLIN);
    mpLineObjC->SetModel(&getModel());
}

VCL_BUILDER_FACTORY(SvxXLinePreview)

Size SvxXLinePreview::GetOptimalSize() const
{
    return getPreviewStripSize(this);
}

SvxXLinePreview::~SvxXLinePreview()
{
    disposeOnce();
}

void SvxXLinePreview::dispose()
{
    SdrObject *pFoo = mpLineObjA;
    SdrObject::Free( pFoo );
    pFoo = mpLineObjB;
    SdrObject::Free( pFoo );
    pFoo = mpLineObjC;
    SdrObject::Free( pFoo );
    SvxPreviewBase::dispose();
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
    aObjectVector.push_back(mpLineObjA);
    aObjectVector.push_back(mpLineObjB);
    aObjectVector.push_back(mpLineObjC);

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), aObjectVector, nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    if ( mbWithSymbol && mpGraphic )
    {
        const Size aOutputSize(GetOutputSize());
        Point aPos = Point( aOutputSize.Width() / 3, aOutputSize.Height() / 2 );
        aPos.X() -= maSymbolSize.Width() / 2;
        aPos.Y() -= maSymbolSize.Height() / 2;
        mpGraphic->Draw(&getBufferDevice(), aPos, maSymbolSize);
    }

    LocalPostPaint(rRenderContext);
}

SvxXRectPreview::SvxXRectPreview(vcl::Window* pParent)
    : SvxPreviewBase(pParent)
    , mpRectangleObject(nullptr)
{
    InitSettings(true, true);

    // create RectangleObject
    const tools::Rectangle aObjectSize(Point(), GetOutputSize());
    mpRectangleObject = new SdrRectObj(aObjectSize);
    mpRectangleObject->SetModel(&getModel());
}

void SvxXRectPreview::Resize()
{
    const tools::Rectangle aObjectSize(Point(), GetOutputSize());
    SdrObject *pOrigObject = mpRectangleObject;
    if (pOrigObject)
    {
        mpRectangleObject = new SdrRectObj(aObjectSize);
        mpRectangleObject->SetModel(&getModel());
        SetAttributes(pOrigObject->GetMergedItemSet());
        SdrObject::Free(pOrigObject);
    }
    SvxPreviewBase::Resize();
}

VCL_BUILDER_FACTORY(SvxXRectPreview)

SvxXRectPreview::~SvxXRectPreview()
{
    disposeOnce();
}

void SvxXRectPreview::dispose()
{
    SdrObject::Free(mpRectangleObject);
    SvxPreviewBase::dispose();
}

void SvxXRectPreview::SetAttributes(const SfxItemSet& rItemSet)
{
    mpRectangleObject->SetMergedItemSet(rItemSet, true);
    mpRectangleObject->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
}

void SvxXRectPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    LocalPrePaint(rRenderContext);

    sdr::contact::SdrObjectVector aObjectVector;

    aObjectVector.push_back(mpRectangleObject);

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), aObjectVector, nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    LocalPostPaint(rRenderContext);
}

SvxXShadowPreview::SvxXShadowPreview( vcl::Window* pParent )
    : SvxPreviewBase(pParent)
    , mpRectangleObject(nullptr)
    , mpRectangleShadow(nullptr)
{
    InitSettings(true, true);

    // prepare size
    Size aSize = GetOutputSize();
    aSize.Width() = aSize.Width() / 3;
    aSize.Height() = aSize.Height() / 3;

    // create RectangleObject
    const tools::Rectangle aObjectSize( Point( aSize.Width(), aSize.Height() ), aSize );
    mpRectangleObject = new SdrRectObj(aObjectSize);
    mpRectangleObject->SetModel(&getModel());

    // create ShadowObject
    const tools::Rectangle aShadowSize( Point( aSize.Width(), aSize.Height() ), aSize );
    mpRectangleShadow = new SdrRectObj(aShadowSize);
    mpRectangleShadow->SetModel(&getModel());
}

VCL_BUILDER_FACTORY(SvxXShadowPreview)

SvxXShadowPreview::~SvxXShadowPreview()
{
    disposeOnce();
}

void SvxXShadowPreview::dispose()
{
    SdrObject::Free(mpRectangleObject);
    SdrObject::Free(mpRectangleShadow);
    SvxPreviewBase::dispose();
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
    LocalPrePaint(rRenderContext);

    // prepare size
    Size aSize = rRenderContext.GetOutputSize();
    aSize.Width() = aSize.Width() / 3;
    aSize.Height() = aSize.Height() / 3;

    tools::Rectangle aObjectRect(Point(aSize.Width(), aSize.Height()), aSize);
    mpRectangleObject->SetSnapRect(aObjectRect);
    aObjectRect.Move(maShadowOffset.X(), maShadowOffset.Y());
    mpRectangleShadow->SetSnapRect(aObjectRect);

    sdr::contact::SdrObjectVector aObjectVector;

    aObjectVector.push_back(mpRectangleShadow);
    aObjectVector.push_back(mpRectangleObject);

    sdr::contact::ObjectContactOfObjListPainter aPainter(getBufferDevice(), aObjectVector, nullptr);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    LocalPostPaint(rRenderContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
