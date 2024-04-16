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

#include <limits.h>

#include <o3tl/float_int_conversion.hxx>
#include <sal/log.hxx>

#include <tools/helpers.hxx>

#include <vcl/toolkit/dialog.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <vcl/scrollable.hxx>
#include <vcl/toolkit/scrbar.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/builder.hxx>
#include <o3tl/string_view.hxx>

#include <window.h>
#include <svdata.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <scrwnd.hxx>

#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace com::sun::star;

namespace vcl {

void Window::ShowFocus( const tools::Rectangle& rRect )
{
    if( mpWindowImpl->mbInShowFocus )
        return;
    mpWindowImpl->mbInShowFocus = true;

    ImplWinData* pWinData = ImplGetWinData();

    // native themeing suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbInPaint )
        {
            if ( mpWindowImpl->mbFocusVisible )
            {
                if ( *pWinData->mpFocusRect == rRect )
                {
                    mpWindowImpl->mbInShowFocus = false;
                    return;
                }

                ImplInvertFocus( *pWinData->mpFocusRect );
            }

            ImplInvertFocus( rRect );
        }
        pWinData->mpFocusRect = rRect;
        mpWindowImpl->mbFocusVisible = true;
    }
    else
    {
        if( ! mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = true;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInShowFocus = false;
}

void Window::HideFocus()
{

    if( mpWindowImpl->mbInHideFocus )
        return;
    mpWindowImpl->mbInHideFocus = true;

    // native themeing can suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbFocusVisible )
        {
            mpWindowImpl->mbInHideFocus = false;
            return;
        }

        if ( !mpWindowImpl->mbInPaint )
            ImplInvertFocus( *ImplGetWinData()->mpFocusRect );
        mpWindowImpl->mbFocusVisible = false;
    }
    else
    {
        if( mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = false;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInHideFocus = false;
}

void Window::ShowTracking( const tools::Rectangle& rRect, ShowTrackFlags nFlags )
{
    ImplWinData* pWinData = ImplGetWinData();

    if ( !mpWindowImpl->mbInPaint || !(nFlags & ShowTrackFlags::TrackWindow) )
    {
        if ( mpWindowImpl->mbTrackVisible )
        {
            if ( (*pWinData->mpTrackRect  == rRect) &&
                 (pWinData->mnTrackFlags    == nFlags) )
                return;

            InvertTracking( *pWinData->mpTrackRect, pWinData->mnTrackFlags );
        }

        InvertTracking( rRect, nFlags );
    }

    pWinData->mpTrackRect = rRect;
    pWinData->mnTrackFlags      = nFlags;
    mpWindowImpl->mbTrackVisible              = true;
}

void Window::HideTracking()
{
    if ( mpWindowImpl->mbTrackVisible )
    {
        ImplWinData* pWinData = ImplGetWinData();
        if ( !mpWindowImpl->mbInPaint || !(pWinData->mnTrackFlags & ShowTrackFlags::TrackWindow) )
            InvertTracking( *pWinData->mpTrackRect, pWinData->mnTrackFlags );
        mpWindowImpl->mbTrackVisible = false;
    }
}

void Window::InvertTracking( const tools::Rectangle& rRect, ShowTrackFlags nFlags )
{
    OutputDevice *pOutDev = GetOutDev();
    tools::Rectangle aRect( pOutDev->ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Normalize();

    SalGraphics* pGraphics;

    if ( nFlags & ShowTrackFlags::TrackWindow )
    {
        if ( !GetOutDev()->IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !GetOutDev()->mpGraphics )
        {
            if ( !pOutDev->AcquireGraphics() )
                return;
        }

        if ( GetOutDev()->mbInitClipRegion )
            GetOutDev()->InitClipRegion();

        if ( GetOutDev()->mbOutputClipped )
            return;

        pGraphics = GetOutDev()->mpGraphics;
    }
    else
    {
        pGraphics = ImplGetFrameGraphics();

        if ( nFlags & ShowTrackFlags::Clip )
        {
            vcl::Region aRegion( GetOutputRectPixel() );
            ImplClipBoundaries( aRegion, false, false );
            pOutDev->SelectClipRegion( aRegion, pGraphics );
        }
    }

    ShowTrackFlags nStyle = nFlags & ShowTrackFlags::StyleMask;
    if ( nStyle == ShowTrackFlags::Object )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SalInvert::TrackFrame, *GetOutDev() );
    else if ( nStyle == ShowTrackFlags::Split )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SalInvert::N50, *GetOutDev() );
    else
    {
        tools::Long nBorder = 1;
        if ( nStyle == ShowTrackFlags::Big )
            nBorder = 5;
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), nBorder, SalInvert::N50, *GetOutDev() );
        pGraphics->Invert( aRect.Left(), aRect.Bottom()-nBorder+1, aRect.GetWidth(), nBorder, SalInvert::N50, *GetOutDev() );
        pGraphics->Invert( aRect.Left(), aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SalInvert::N50, *GetOutDev() );
        pGraphics->Invert( aRect.Right()-nBorder+1, aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SalInvert::N50, *GetOutDev() );
    }
}

IMPL_LINK( Window, ImplTrackTimerHdl, Timer*, pTimer, void )
{
    if (!mpWindowImpl)
    {
        SAL_WARN("vcl", "ImplTrackTimerHdl has outlived dispose");
        return;
    }

    ImplSVData* pSVData = ImplGetSVData();

    // if Button-Repeat we have to change the timeout
    if ( pSVData->mpWinData->mnTrackFlags & StartTrackingFlags::ButtonRepeat )
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );

    // create Tracking-Event
    Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( GetOutDev()->ImplIsAntiparallel() )
    {
        // re-mirror frame pos at pChild
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aMousePos );
    }
    MouseEvent      aMEvt( ScreenToOutputPixel( aMousePos ),
                           mpWindowImpl->mpFrameData->mnClickCount, MouseEventModifiers::NONE,
                           mpWindowImpl->mpFrameData->mnMouseCode,
                           mpWindowImpl->mpFrameData->mnMouseCode );
    TrackingEvent   aTEvt( aMEvt, TrackingEventFlags::Repeat );
    Tracking( aTEvt );
}

void Window::SetUseFrameData(bool bUseFrameData)
{
    if (mpWindowImpl)
        mpWindowImpl->mbUseFrameData = bUseFrameData;
}

void Window::StartTracking( StartTrackingFlags nFlags )
{
    if (!mpWindowImpl)
        return;

    ImplSVData* pSVData = ImplGetSVData();
    VclPtr<vcl::Window> pTrackWin = mpWindowImpl->mbUseFrameData ?
        mpWindowImpl->mpFrameData->mpTrackWin :
        pSVData->mpWinData->mpTrackWin;

    if ( pTrackWin.get() != this )
    {
        if ( pTrackWin )
            pTrackWin->EndTracking( TrackingEventFlags::Cancel );
    }

    SAL_WARN_IF(pSVData->mpWinData->mpTrackTimer, "vcl", "StartTracking called while TrackerTimer still running");

    if ( !mpWindowImpl->mbUseFrameData &&
         (nFlags & (StartTrackingFlags::ScrollRepeat | StartTrackingFlags::ButtonRepeat)) )
    {
        pSVData->mpWinData->mpTrackTimer.reset(new AutoTimer("vcl::Window pSVData->mpWinData->mpTrackTimer"));

        if ( nFlags & StartTrackingFlags::ScrollRepeat )
            pSVData->mpWinData->mpTrackTimer->SetTimeout( MouseSettings::GetScrollRepeat() );
        else
            pSVData->mpWinData->mpTrackTimer->SetTimeout( MouseSettings::GetButtonStartRepeat() );
        pSVData->mpWinData->mpTrackTimer->SetInvokeHandler( LINK( this, Window, ImplTrackTimerHdl ) );
        pSVData->mpWinData->mpTrackTimer->Start();
    }

    if (mpWindowImpl->mbUseFrameData)
    {
        mpWindowImpl->mpFrameData->mpTrackWin = this;
    }
    else
    {
        pSVData->mpWinData->mpTrackWin   = this;
        pSVData->mpWinData->mnTrackFlags = nFlags;
        CaptureMouse();
    }
}

void Window::EndTracking( TrackingEventFlags nFlags )
{
    if (!mpWindowImpl)
        return;

    ImplSVData* pSVData = ImplGetSVData();
    VclPtr<vcl::Window> pTrackWin = mpWindowImpl->mbUseFrameData ?
        mpWindowImpl->mpFrameData->mpTrackWin :
        pSVData->mpWinData->mpTrackWin;

    if ( pTrackWin.get() != this )
        return;

    if ( !mpWindowImpl->mbUseFrameData && pSVData->mpWinData->mpTrackTimer )
        pSVData->mpWinData->mpTrackTimer.reset();

    mpWindowImpl->mpFrameData->mpTrackWin = pSVData->mpWinData->mpTrackWin = nullptr;
    pSVData->mpWinData->mnTrackFlags  = StartTrackingFlags::NONE;
    ReleaseMouse();

    // call EndTracking if required
    if (mpWindowImpl->mpFrameData)
    {
        Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
        if( GetOutDev()->ImplIsAntiparallel() )
        {
            // re-mirror frame pos at pChild
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aMousePos );
        }

        MouseEvent      aMEvt( ScreenToOutputPixel( aMousePos ),
                               mpWindowImpl->mpFrameData->mnClickCount, MouseEventModifiers::NONE,
                               mpWindowImpl->mpFrameData->mnMouseCode,
                               mpWindowImpl->mpFrameData->mnMouseCode );
        TrackingEvent   aTEvt( aMEvt, nFlags | TrackingEventFlags::End );
        // CompatTracking effectively
        if (!mpWindowImpl || mpWindowImpl->mbInDispose)
            return Window::Tracking( aTEvt );
        else
            return Tracking( aTEvt );
    }
}

bool Window::IsTracking() const
{
    if (!mpWindowImpl)
        return false;
    if (mpWindowImpl->mbUseFrameData && mpWindowImpl->mpFrameData)
    {
        return mpWindowImpl->mpFrameData->mpTrackWin == this;
    }
    if (!mpWindowImpl->mbUseFrameData && ImplGetSVData()->mpWinData)
    {
        return ImplGetSVData()->mpWinData->mpTrackWin == this;
    }
    return false;
}

void Window::StartAutoScroll( StartAutoScrollFlags nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->mpWinData->mpAutoScrollWin.get() != this )
    {
        if ( pSVData->mpWinData->mpAutoScrollWin )
            pSVData->mpWinData->mpAutoScrollWin->EndAutoScroll();
    }

    pSVData->mpWinData->mpAutoScrollWin = this;
    pSVData->mpWinData->mnAutoScrollFlags = nFlags;
    pSVData->maAppData.mpWheelWindow = VclPtr<ImplWheelWindow>::Create( this );
}

void Window::EndAutoScroll()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->mpWinData->mpAutoScrollWin.get() == this )
    {
        pSVData->mpWinData->mpAutoScrollWin = nullptr;
        pSVData->mpWinData->mnAutoScrollFlags = StartAutoScrollFlags::NONE;
        pSVData->maAppData.mpWheelWindow->ImplStop();
        pSVData->maAppData.mpWheelWindow.disposeAndClear();
    }
}

VclPtr<vcl::Window> Window::SaveFocus()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->mpWinData->mpFocusWin )
    {
        return pSVData->mpWinData->mpFocusWin;
    }
    else
        return nullptr;
}

void Window::EndSaveFocus(const VclPtr<vcl::Window>& xFocusWin)
{
    if (xFocusWin && !xFocusWin->isDisposed())
    {
        xFocusWin->GrabFocus();
    }
}

void Window::SetZoom( const Fraction& rZoom )
{
    if ( mpWindowImpl && mpWindowImpl->maZoom != rZoom )
    {
        mpWindowImpl->maZoom = rZoom;
        CompatStateChanged( StateChangedType::Zoom );
    }
}

void Window::SetZoomedPointFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont)
{
    const Fraction& rZoom = GetZoom();
    if (rZoom.GetNumerator() != rZoom.GetDenominator())
    {
        vcl::Font aFont(rFont);
        Size aSize = aFont.GetFontSize();
        aSize.setWidth(basegfx::fround<tools::Long>(double(aSize.Width() * rZoom)));
        aSize.setHeight(basegfx::fround<tools::Long>(double(aSize.Height() * rZoom)));
        aFont.SetFontSize(aSize);
        SetPointFont(rRenderContext, aFont);
    }
    else
    {
        SetPointFont(rRenderContext, rFont);
    }
}

tools::Long Window::CalcZoom( tools::Long nCalc ) const
{

    const Fraction& rZoom = GetZoom();
    if ( rZoom.GetNumerator() != rZoom.GetDenominator() )
    {
        double n = double(nCalc * rZoom);
        nCalc = basegfx::fround<tools::Long>(n);
    }
    return nCalc;
}

void Window::SetControlFont()
{
    if (mpWindowImpl && mpWindowImpl->mpControlFont)
    {
        mpWindowImpl->mpControlFont.reset();
        CompatStateChanged(StateChangedType::ControlFont);
    }
}

void Window::SetControlFont(const vcl::Font& rFont)
{
    if (rFont == vcl::Font())
    {
        SetControlFont();
        return;
    }

    if (mpWindowImpl->mpControlFont)
    {
        if (*mpWindowImpl->mpControlFont == rFont)
            return;
        *mpWindowImpl->mpControlFont = rFont;
    }
    else
        mpWindowImpl->mpControlFont = rFont;

    CompatStateChanged(StateChangedType::ControlFont);
}

vcl::Font Window::GetControlFont() const
{
    if (mpWindowImpl->mpControlFont)
        return *mpWindowImpl->mpControlFont;
    else
    {
        vcl::Font aFont;
        return aFont;
    }
}

void Window::ApplyControlFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont)
{
    vcl::Font aFont(rFont);
    if (IsControlFont())
        aFont.Merge(GetControlFont());
    SetZoomedPointFont(rRenderContext, aFont);
}

void Window::SetControlForeground()
{
    if (mpWindowImpl->mbControlForeground)
    {
        mpWindowImpl->maControlForeground = COL_TRANSPARENT;
        mpWindowImpl->mbControlForeground = false;
        CompatStateChanged(StateChangedType::ControlForeground);
    }
}

void Window::SetControlForeground(const Color& rColor)
{
    if (rColor.IsTransparent())
    {
        if (mpWindowImpl->mbControlForeground)
        {
            mpWindowImpl->maControlForeground = COL_TRANSPARENT;
            mpWindowImpl->mbControlForeground = false;
            CompatStateChanged(StateChangedType::ControlForeground);
        }
    }
    else
    {
        if (mpWindowImpl->maControlForeground != rColor)
        {
            mpWindowImpl->maControlForeground = rColor;
            mpWindowImpl->mbControlForeground = true;
            CompatStateChanged(StateChangedType::ControlForeground);
        }
    }
}

void Window::ApplyControlForeground(vcl::RenderContext& rRenderContext, const Color& rDefaultColor)
{
    Color aTextColor(rDefaultColor);
    if (IsControlForeground())
        aTextColor = GetControlForeground();
    rRenderContext.SetTextColor(aTextColor);
}

void Window::SetControlBackground()
{
    if (mpWindowImpl->mbControlBackground)
    {
        mpWindowImpl->maControlBackground = COL_TRANSPARENT;
        mpWindowImpl->mbControlBackground = false;
        CompatStateChanged(StateChangedType::ControlBackground);
    }
}

void Window::SetControlBackground(const Color& rColor)
{
    if (rColor.IsTransparent())
    {
        if (mpWindowImpl->mbControlBackground)
        {
            mpWindowImpl->maControlBackground = COL_TRANSPARENT;
            mpWindowImpl->mbControlBackground = false;
            CompatStateChanged(StateChangedType::ControlBackground);
        }
    }
    else
    {
        if (mpWindowImpl->maControlBackground != rColor)
        {
            mpWindowImpl->maControlBackground = rColor;
            mpWindowImpl->mbControlBackground = true;
            CompatStateChanged(StateChangedType::ControlBackground);
        }
    }
}

void Window::ApplyControlBackground(vcl::RenderContext& rRenderContext, const Color& rDefaultColor)
{
    Color aColor(rDefaultColor);
    if (IsControlBackground())
        aColor = GetControlBackground();
    rRenderContext.SetBackground(aColor);
}

Size Window::CalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;
    aSz.AdjustWidth(mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder );
    aSz.AdjustHeight(mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder );
    return aSz;
}

Size Window::CalcOutputSize( const Size& rWinSz ) const
{
    Size aSz = rWinSz;
    aSz.AdjustWidth( -(mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder) );
    aSz.AdjustHeight( -(mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder) );
    return aSz;
}

vcl::Font Window::GetDrawPixelFont(OutputDevice const * pDev) const
{
    vcl::Font aFont = GetPointFont(*GetOutDev());
    Size aFontSize = aFont.GetFontSize();
    MapMode aPtMapMode(MapUnit::MapPoint);
    aFontSize = pDev->LogicToPixel( aFontSize, aPtMapMode );
    aFont.SetFontSize( aFontSize );
    return aFont;
}

tools::Long Window::GetDrawPixel( OutputDevice const * pDev, tools::Long nPixels ) const
{
    tools::Long nP = nPixels;
    if ( pDev->GetOutDevType() != OUTDEV_WINDOW )
    {
        MapMode aMap( MapUnit::Map100thMM );
        Size aSz( nP, 0 );
        aSz = PixelToLogic( aSz, aMap );
        aSz = pDev->LogicToPixel( aSz, aMap );
        nP = aSz.Width();
    }
    return nP;
}

// returns how much was actually scrolled (so that abs(retval) <= abs(nN))
static double lcl_HandleScrollHelper( Scrollable* pScrl, double nN, bool isMultiplyByLineSize )
{
    if (!pScrl || !nN || pScrl->Inactive())
        return 0.0;

    tools::Long nNewPos = pScrl->GetThumbPos();
    double scrolled = nN;

    if ( nN == double(-LONG_MAX) )
        nNewPos += pScrl->GetPageSize();
    else if ( nN == double(LONG_MAX) )
        nNewPos -= pScrl->GetPageSize();
    else
    {
        // allowing both chunked and continuous scrolling
        if(isMultiplyByLineSize){
            nN*=pScrl->GetLineSize();
        }

        // compute how many quantized units to scroll
        tools::Long magnitude = o3tl::saturating_cast<tools::Long>(abs(nN));
        tools::Long change = copysign(magnitude, nN);

        nNewPos = nNewPos - change;

        scrolled = double(change);
        // convert back to chunked/continuous
        if(isMultiplyByLineSize){
            scrolled /= pScrl->GetLineSize();
        }
    }

    pScrl->DoScroll( nNewPos );

    return scrolled;
}

bool Window::HandleScrollCommand( const CommandEvent& rCmd,
                                  Scrollable* pHScrl, Scrollable* pVScrl )
{
    bool bRet = false;

    if ( pHScrl || pVScrl )
    {
        switch( rCmd.GetCommand() )
        {
            case CommandEventId::StartAutoScroll:
            {
                StartAutoScrollFlags nFlags = StartAutoScrollFlags::NONE;
                if ( pHScrl )
                {
                    if ( (pHScrl->GetVisibleSize() < pHScrl->GetRangeMax()) &&
                         !pHScrl->Inactive() )
                        nFlags |= StartAutoScrollFlags::Horz;
                }
                if ( pVScrl )
                {
                    if ( (pVScrl->GetVisibleSize() < pVScrl->GetRangeMax()) &&
                         !pVScrl->Inactive() )
                        nFlags |= StartAutoScrollFlags::Vert;
                }

                if ( nFlags != StartAutoScrollFlags::NONE )
                {
                    StartAutoScroll( nFlags );
                    bRet = true;
                }
            }
            break;

            case CommandEventId::Wheel:
            {
                const CommandWheelData* pData = rCmd.GetWheelData();

                if ( pData && (CommandWheelMode::SCROLL == pData->GetMode()) )
                {
                    if (!pData->IsDeltaPixel())
                    {
                        double nScrollLines = pData->GetScrollLines();
                        double nLines;
                        double* partialScroll = pData->IsHorz()
                            ? &mpWindowImpl->mfPartialScrollX
                            : &mpWindowImpl->mfPartialScrollY;
                        if ( nScrollLines == COMMAND_WHEEL_PAGESCROLL )
                        {
                            if ( pData->GetDelta() < 0 )
                                nLines = double(-LONG_MAX);
                            else
                                nLines = double(LONG_MAX);
                        }
                        else
                            nLines = *partialScroll + pData->GetNotchDelta() * nScrollLines;
                        if ( nLines )
                        {
                            Scrollable* pScrl = pData->IsHorz() ? pHScrl : pVScrl;
                            double scrolled = lcl_HandleScrollHelper( pScrl, nLines, true );
                            *partialScroll = nLines - scrolled;
                            bRet = true;
                        }
                    }
                    else
                    {
                        // Mobile / touch scrolling section
                        const Point & deltaPoint = rCmd.GetMousePosPixel();

                        double deltaXInPixels = double(deltaPoint.X());
                        double deltaYInPixels = double(deltaPoint.Y());
                        Size winSize = GetOutputSizePixel();

                        if(pHScrl)
                        {
                            double visSizeX = double(pHScrl->GetVisibleSize());
                            double ratioX = deltaXInPixels / double(winSize.getWidth());
                            tools::Long deltaXInLogic = tools::Long(visSizeX * ratioX);
                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            tools::Long lineSizeX = pHScrl->GetLineSize();

                            if(lineSizeX)
                            {
                                deltaXInLogic /= lineSizeX;
                            }
                            else
                            {
                                deltaXInLogic = 0;
                            }
#endif
                            if ( deltaXInLogic)
                            {
#ifndef IOS
                                bool const isMultiplyByLineSize = true;
#else
                                bool const isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pHScrl, deltaXInLogic, isMultiplyByLineSize );
                                bRet = true;
                            }
                        }
                        if(pVScrl)
                        {
                            double visSizeY = double(pVScrl->GetVisibleSize());
                            double ratioY = deltaYInPixels / double(winSize.getHeight());
                            tools::Long deltaYInLogic = tools::Long(visSizeY * ratioY);

                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            tools::Long lineSizeY = pVScrl->GetLineSize();
                            if(lineSizeY)
                            {
                                deltaYInLogic /= lineSizeY;
                            }
                            else
                            {
                                deltaYInLogic = 0;
                            }
#endif
                            if ( deltaYInLogic )
                            {
#ifndef IOS
                                bool const isMultiplyByLineSize = true;
#else
                                bool const isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pVScrl, deltaYInLogic, isMultiplyByLineSize );

                                bRet = true;
                            }
                        }
                    }
                }
            }
            break;

            case CommandEventId::GesturePan:
            {
                if (pVScrl)
                {
                    const CommandGesturePanData* pData = rCmd.GetGesturePanData();
                    if (pData->meEventType == GestureEventPanType::Begin)
                    {
                        mpWindowImpl->mpFrameData->mnTouchPanPosition = pVScrl->GetThumbPos();
                    }
                    else if(pData->meEventType == GestureEventPanType::Update)
                    {
                        tools::Long nOriginalPosition = mpWindowImpl->mpFrameData->mnTouchPanPosition;
                        pVScrl->DoScroll(nOriginalPosition + (pData->mfOffset / pVScrl->GetVisibleSize()));
                    }
                    if (pData->meEventType == GestureEventPanType::End)
                    {
                        mpWindowImpl->mpFrameData->mnTouchPanPosition = -1;
                    }
                    bRet = true;
                }
                break;
            }

            case CommandEventId::AutoScroll:
            {
                const CommandScrollData* pData = rCmd.GetAutoScrollData();
                if ( pData && (pData->GetDeltaX() || pData->GetDeltaY()) )
                {
                    ImplHandleScroll( pHScrl, pData->GetDeltaX(),
                                      pVScrl, pData->GetDeltaY() );
                    bRet = true;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

void Window::ImplHandleScroll( Scrollable* pHScrl, double nX,
                               Scrollable* pVScrl, double nY )
{
    lcl_HandleScrollHelper( pHScrl, nX, true );
    lcl_HandleScrollHelper( pVScrl, nY, true );
}

DockingManager* Window::GetDockingManager()
{
    return ImplGetDockingManager();
}

void Window::EnableDocking( bool bEnable )
{
    // update list of dockable windows
    if( bEnable )
        ImplGetDockingManager()->AddWindow( this );
    else
        ImplGetDockingManager()->RemoveWindow( this );
}

// retrieves the list of owner draw decorated windows for this window hierarchy
::std::vector<VclPtr<vcl::Window> >& Window::ImplGetOwnerDrawList()
{
    return ImplGetTopmostFrameWindow()->mpWindowImpl->mpFrameData->maOwnerDrawList;
}

void Window::SetHelpId( const OUString& rHelpId )
{
    mpWindowImpl->maHelpId = rHelpId;
}

const OUString& Window::GetHelpId() const
{
    return mpWindowImpl->maHelpId;
}

// --------- old inline methods ---------------

vcl::Window* Window::ImplGetWindow() const
{
    if ( mpWindowImpl->mpClientWindow )
        return mpWindowImpl->mpClientWindow;
    else
        return const_cast<vcl::Window*>(this);
}

ImplFrameData* Window::ImplGetFrameData()
{
    return mpWindowImpl ? mpWindowImpl->mpFrameData : nullptr;
}

SalFrame* Window::ImplGetFrame() const
{
    return mpWindowImpl ? mpWindowImpl->mpFrame : nullptr;
}

weld::Window* Window::GetFrameWeld() const
{
    SalFrame* pFrame = ImplGetFrame();
    return pFrame ? pFrame->GetFrameWeld() : nullptr;
}

vcl::Window* Window::GetFrameWindow() const
{
    SalFrame* pFrame = ImplGetFrame();
    return pFrame ? pFrame->GetWindow() : nullptr;
}

vcl::Window* Window::ImplGetParent() const
{
    return mpWindowImpl ? mpWindowImpl->mpParent.get() : nullptr;
}

vcl::Window* Window::ImplGetClientWindow() const
{
    return mpWindowImpl ? mpWindowImpl->mpClientWindow.get() : nullptr;
}

vcl::Window* Window::ImplGetBorderWindow() const
{
    return mpWindowImpl ? mpWindowImpl->mpBorderWindow.get() : nullptr;
}

vcl::Window* Window::ImplGetFirstOverlapWindow()
{
    if (!mpWindowImpl)
    {
        return nullptr;
    }

    if ( mpWindowImpl->mbOverlapWin )
        return this;
    else
        return mpWindowImpl->mpOverlapWindow;
}

const vcl::Window* Window::ImplGetFirstOverlapWindow() const
{
    if (!mpWindowImpl)
    {
        return nullptr;
    }

    if ( mpWindowImpl->mbOverlapWin )
        return this;
    else
        return mpWindowImpl->mpOverlapWindow;
}

vcl::Window* Window::ImplGetFrameWindow() const
{
    return mpWindowImpl ? mpWindowImpl->mpFrameWindow.get() : nullptr;
}

bool Window::IsDockingWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbDockWin;
}

bool Window::ImplIsFloatingWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbFloatWin;
}

bool Window::ImplIsSplitter() const
{
    return mpWindowImpl && mpWindowImpl->mbSplitter;
}

bool Window::ImplIsPushButton() const
{
    return mpWindowImpl && mpWindowImpl->mbPushButton;
}

bool Window::ImplIsOverlapWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbOverlapWin;
}

void Window::ImplSetMouseTransparent( bool bTransparent )
{
    if (mpWindowImpl)
        mpWindowImpl->mbMouseTransparent = bTransparent;
}

void Window::SetCompoundControl( bool bCompound )
{
    if (mpWindowImpl)
        mpWindowImpl->mbCompoundControl = bCompound;
}

WinBits Window::GetStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnStyle : 0;
}

WinBits Window::GetPrevStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnPrevStyle : 0;
}

WindowExtendedStyle Window::GetExtendedStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnExtendedStyle : WindowExtendedStyle::NONE;
}

void Window::SetType( WindowType nType )
{
    if (mpWindowImpl)
        mpWindowImpl->mnType = nType;
}

WindowType Window::GetType() const
{
    if (mpWindowImpl)
        return mpWindowImpl->mnType;
    else
        return WindowType::NONE;
}

Dialog* Window::GetParentDialog() const
{
    const vcl::Window *pWindow = this;

    while( pWindow )
    {
        if( pWindow->IsDialog() )
            break;

        pWindow = pWindow->GetParent();
    }

    return const_cast<Dialog *>(dynamic_cast<const Dialog*>(pWindow));
}

bool Window::IsSystemWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbSysWin;
}

bool Window::IsDialog() const
{
    return mpWindowImpl && mpWindowImpl->mbDialog;
}

bool Window::IsMenuFloatingWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbMenuFloatingWindow;
}

bool Window::IsToolbarFloatingWindow() const
{
    return mpWindowImpl && mpWindowImpl->mbToolbarFloatingWindow;
}

bool Window::IsNativeFrame() const
{
    if( mpWindowImpl->mbFrame )
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menus!) are closeable
        if( mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return true;
        else
            return false;
    else
        return false;
}

void Window::EnableAllResize()
{
    mpWindowImpl->mbAllResize = true;
}

void Window::EnableChildTransparentMode( bool bEnable )
{
    mpWindowImpl->mbChildTransparent = bEnable;
}

bool Window::IsChildTransparentModeEnabled() const
{
    return mpWindowImpl && mpWindowImpl->mbChildTransparent;
}

bool Window::IsMouseTransparent() const
{
    return mpWindowImpl && mpWindowImpl->mbMouseTransparent;
}

bool Window::IsPaintTransparent() const
{
    return mpWindowImpl && mpWindowImpl->mbPaintTransparent;
}

void Window::SetDialogControlStart( bool bStart )
{
    mpWindowImpl->mbDlgCtrlStart = bStart;
}

bool Window::IsDialogControlStart() const
{
    return mpWindowImpl && mpWindowImpl->mbDlgCtrlStart;
}

void Window::SetDialogControlFlags( DialogControlFlags nFlags )
{
    mpWindowImpl->mnDlgCtrlFlags = nFlags;
}

DialogControlFlags Window::GetDialogControlFlags() const
{
    return mpWindowImpl->mnDlgCtrlFlags;
}

const InputContext& Window::GetInputContext() const
{
    return mpWindowImpl->maInputContext;
}

bool Window::IsControlFont() const
{
    return bool(mpWindowImpl->mpControlFont);
}

const Color& Window::GetControlForeground() const
{
    return mpWindowImpl->maControlForeground;
}

bool Window::IsControlForeground() const
{
    return mpWindowImpl->mbControlForeground;
}

const Color& Window::GetControlBackground() const
{
    return mpWindowImpl->maControlBackground;
}

bool Window::IsControlBackground() const
{
    return mpWindowImpl->mbControlBackground;
}

bool Window::IsInPaint() const
{
    return mpWindowImpl && mpWindowImpl->mbInPaint;
}

vcl::Window* Window::GetParent() const
{
    return mpWindowImpl ? mpWindowImpl->mpRealParent.get() : nullptr;
}

bool Window::IsVisible() const
{
    return mpWindowImpl && mpWindowImpl->mbVisible;
}

bool Window::IsReallyVisible() const
{
    return mpWindowImpl && mpWindowImpl->mbReallyVisible;
}

bool Window::IsReallyShown() const
{
    return mpWindowImpl && mpWindowImpl->mbReallyShown;
}

bool Window::IsInInitShow() const
{
    return mpWindowImpl->mbInInitShow;
}

bool Window::IsEnabled() const
{
    return mpWindowImpl && !mpWindowImpl->mbDisabled;
}

bool Window::IsInputEnabled() const
{
    return mpWindowImpl && !mpWindowImpl->mbInputDisabled;
}

bool Window::IsAlwaysEnableInput() const
{
    return mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled;
}

ActivateModeFlags Window::GetActivateMode() const
{
    return mpWindowImpl->mnActivateMode;

}

bool Window::IsAlwaysOnTopEnabled() const
{
    return mpWindowImpl->mbAlwaysOnTop;
}

bool Window::IsDefaultPos() const
{
    return mpWindowImpl->mbDefPos;
}

bool Window::IsDefaultSize() const
{
    return mpWindowImpl->mbDefSize;
}

Point Window::GetOffsetPixelFrom(const vcl::Window& rWindow) const
{
    return Point(GetOutOffXPixel() - rWindow.GetOutOffXPixel(), GetOutOffYPixel() - rWindow.GetOutOffYPixel());
}

void Window::EnablePaint( bool bEnable )
{
    mpWindowImpl->mbPaintDisabled = !bEnable;
}

bool Window::IsPaintEnabled() const
{
    return !mpWindowImpl->mbPaintDisabled;
}

bool Window::IsUpdateMode() const
{
    return !mpWindowImpl->mbNoUpdate;
}

void Window::SetParentUpdateMode( bool bUpdate )
{
    mpWindowImpl->mbNoParentUpdate = !bUpdate;
}

bool Window::IsActive() const
{
    return mpWindowImpl->mbActive;
}

GetFocusFlags Window::GetGetFocusFlags() const
{
    return mpWindowImpl->mnGetFocusFlags;
}

bool Window::IsCompoundControl() const
{
    return mpWindowImpl && mpWindowImpl->mbCompoundControl;
}

bool Window::IsWait() const
{
    return (mpWindowImpl->mnWaitCount != 0);
}

vcl::Cursor* Window::GetCursor() const
{
    if (!mpWindowImpl)
        return nullptr;
    return mpWindowImpl->mpCursor;
}

const Fraction& Window::GetZoom() const
{
    return mpWindowImpl->maZoom;
}

bool Window::IsZoom() const
{
    return mpWindowImpl->maZoom.GetNumerator() != mpWindowImpl->maZoom.GetDenominator();
}

void Window::SetHelpText( const OUString& rHelpText )
{
    mpWindowImpl->maHelpText = rHelpText;
    mpWindowImpl->mbHelpTextDynamic = true;
}

void Window::SetQuickHelpText( const OUString& rHelpText )
{
    if (mpWindowImpl)
        mpWindowImpl->maQuickHelpText = rHelpText;
}

const OUString& Window::GetQuickHelpText() const
{
    return mpWindowImpl->maQuickHelpText;
}

bool Window::IsCreatedWithToolkit() const
{
    return mpWindowImpl->mbCreatedWithToolkit;
}

void Window::SetCreatedWithToolkit( bool b )
{
    mpWindowImpl->mbCreatedWithToolkit = b;
}

PointerStyle Window::GetPointer() const
{
    return mpWindowImpl->maPointer;
}

VCLXWindow* Window::GetWindowPeer() const
{
    return mpWindowImpl ? mpWindowImpl->mpVCLXWindow : nullptr;
}

void Window::SetPosPixel( const Point& rNewPos )
{
    setPosSizePixel( rNewPos.X(), rNewPos.Y(), 0, 0, PosSizeFlags::Pos );
}

void Window::SetSizePixel( const Size& rNewSize )
{
    setPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(),
                     PosSizeFlags::Size );
}

void Window::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    setPosSizePixel( rNewPos.X(), rNewPos.Y(),
                     rNewSize.Width(), rNewSize.Height());
}

void Window::SetOutputSizePixel( const Size& rNewSize )
{
    SetSizePixel( Size( rNewSize.Width()+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                        rNewSize.Height()+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder ) );
}

//When a widget wants to renegotiate layout, get toplevel parent dialog and call
//resize on it. Mark all intermediate containers (or container-alike) widgets
//as dirty for the size remains unchanged, but layout changed circumstances
namespace
{
    bool queue_ungrouped_resize(vcl::Window const *pOrigWindow)
    {
        bool bSomeoneCares = false;

        vcl::Window *pWindow = pOrigWindow->GetParent();
        if (pWindow)
        {
            if (isContainerWindow(*pWindow))
            {
                bSomeoneCares = true;
            }
            else if (pWindow->GetType() == WindowType::TABCONTROL)
            {
                bSomeoneCares = true;
            }
            pWindow->queue_resize();
        }

        return bSomeoneCares;
    }
}

void Window::InvalidateSizeCache()
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnOptimalWidthCache = -1;
    pWindowImpl->mnOptimalHeightCache = -1;
}

static bool HasParentDockingWindow(const vcl::Window* pWindow)
{
    while( pWindow )
    {
        if( pWindow->IsDockingWindow() )
            return true;

        pWindow = pWindow->GetParent();
    }

    return false;
}

void Window::queue_resize(StateChangedType eReason)
{
    if (isDisposed())
        return;

    bool bSomeoneCares = queue_ungrouped_resize(this);

    if (eReason != StateChangedType::Visible)
    {
        InvalidateSizeCache();
    }

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->m_xSizeGroup && pWindowImpl->m_xSizeGroup->get_mode() != VclSizeGroupMode::NONE)
    {
        std::set<VclPtr<vcl::Window> > &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
        for (VclPtr<vcl::Window> const & pOther : rWindows)
        {
            if (pOther == this)
                continue;
            queue_ungrouped_resize(pOther);
        }
    }

    if (bSomeoneCares && !isDisposed())
    {
        //fdo#57090 force a resync of the borders of the borderwindow onto this
        //window in case they have changed
        vcl::Window* pBorderWindow = ImplGetBorderWindow();
        if (pBorderWindow)
            pBorderWindow->Resize();
    }
    if (VclPtr<vcl::Window> pParent = GetParentWithLOKNotifier())
    {
        Size aSize = GetSizePixel();
        if (!aSize.IsEmpty() && !pParent->IsInInitShow()
            && (GetParentDialog() || HasParentDockingWindow(this)))
            LogicInvalidate(nullptr);
    }
}

namespace
{
    VclAlign toAlign(std::u16string_view rValue)
    {
        VclAlign eRet = VclAlign::Fill;

        if (rValue == u"fill")
            eRet = VclAlign::Fill;
        else if (rValue == u"start")
            eRet = VclAlign::Start;
        else if (rValue == u"end")
            eRet = VclAlign::End;
        else if (rValue == u"center")
            eRet = VclAlign::Center;
        return eRet;
    }
}

bool Window::set_font_attribute(const OUString &rKey, std::u16string_view rValue)
{
    if (rKey == "weight")
    {
        vcl::Font aFont(GetControlFont());
        if (rValue == u"thin")
            aFont.SetWeight(WEIGHT_THIN);
        else if (rValue == u"ultralight")
            aFont.SetWeight(WEIGHT_ULTRALIGHT);
        else if (rValue == u"light")
            aFont.SetWeight(WEIGHT_LIGHT);
        else if (rValue == u"book")
            aFont.SetWeight(WEIGHT_SEMILIGHT);
        else if (rValue == u"normal")
            aFont.SetWeight(WEIGHT_NORMAL);
        else if (rValue == u"medium")
            aFont.SetWeight(WEIGHT_MEDIUM);
        else if (rValue == u"semibold")
            aFont.SetWeight(WEIGHT_SEMIBOLD);
        else if (rValue == u"bold")
            aFont.SetWeight(WEIGHT_BOLD);
        else if (rValue == u"ultrabold")
            aFont.SetWeight(WEIGHT_ULTRABOLD);
        else
            aFont.SetWeight(WEIGHT_BLACK);
        SetControlFont(aFont);
    }
    else if (rKey == "style")
    {
        vcl::Font aFont(GetControlFont());
        if (rValue == u"normal")
            aFont.SetItalic(ITALIC_NONE);
        else if (rValue == u"oblique")
            aFont.SetItalic(ITALIC_OBLIQUE);
        else if (rValue == u"italic")
            aFont.SetItalic(ITALIC_NORMAL);
        SetControlFont(aFont);
    }
    else if (rKey == "underline")
    {
        vcl::Font aFont(GetControlFont());
        aFont.SetUnderline(toBool(rValue) ? LINESTYLE_SINGLE : LINESTYLE_NONE);
        SetControlFont(aFont);
    }
    else if (rKey == "scale")
    {
        // if no control font was set yet, take the underlying font from the device
        vcl::Font aFont(IsControlFont() ? GetControlFont() : GetPointFont(*GetOutDev()));
        aFont.SetFontHeight(aFont.GetFontHeight() * o3tl::toDouble(rValue));
        SetControlFont(aFont);
    }
    else if (rKey == "size")
    {
        vcl::Font aFont(GetControlFont());
        sal_Int32 nHeight = o3tl::toInt32(rValue) / 1000;
        aFont.SetFontHeight(nHeight);
        SetControlFont(aFont);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled font attribute: " << rKey);
        return false;
    }
    return true;
}

bool Window::set_property(const OUString &rKey, const OUString &rValue)
{
    if ((rKey == "label") || (rKey == "title") || (rKey == "text") )
    {
        SetText(BuilderUtils::convertMnemonicMarkup(rValue));
    }
    else if (rKey == "visible")
        Show(toBool(rValue));
    else if (rKey == "sensitive")
        Enable(toBool(rValue));
    else if (rKey == "resizable")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_SIZEABLE;
        if (toBool(rValue))
            nBits |= WB_SIZEABLE;
        SetStyle(nBits);
    }
    else if (rKey == "xalign")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

        float f = rValue.toFloat();
        assert(f == 0.0 || f == 1.0 || f == 0.5);
        if (f == 0.0)
            nBits |= WB_LEFT;
        else if (f == 1.0)
            nBits |= WB_RIGHT;
        else if (f == 0.5)
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey == "justification")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

        if (rValue == "left")
            nBits |= WB_LEFT;
        else if (rValue == "right")
            nBits |= WB_RIGHT;
        else if (rValue == "center")
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey == "yalign")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_TOP | WB_VCENTER | WB_BOTTOM);

        float f = rValue.toFloat();
        assert(f == 0.0 || f == 1.0 || f == 0.5);
        if (f == 0.0)
            nBits |= WB_TOP;
        else if (f == 1.0)
            nBits |= WB_BOTTOM;
        else if (f == 0.5)
            nBits |= WB_VCENTER;

        SetStyle(nBits);
    }
    else if (rKey == "wrap")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_WORDBREAK;
        if (toBool(rValue))
            nBits |= WB_WORDBREAK;
        SetStyle(nBits);
    }
    else if (rKey == "height-request")
        set_height_request(rValue.toInt32());
    else if (rKey == "width-request")
        set_width_request(rValue.toInt32());
    else if (rKey == "hexpand")
        set_hexpand(toBool(rValue));
    else if (rKey == "vexpand")
        set_vexpand(toBool(rValue));
    else if (rKey == "halign")
        set_halign(toAlign(rValue));
    else if (rKey == "valign")
        set_valign(toAlign(rValue));
    else if (rKey == "tooltip-markup")
        SetQuickHelpText(rValue);
    else if (rKey == "tooltip-text")
        SetQuickHelpText(rValue);
    else if (rKey == "border-width")
        set_border_width(rValue.toInt32());
    else if (rKey == "margin-start" || rKey == "margin-left")
    {
        assert(rKey == "margin-start" && "margin-left deprecated in favor of margin-start");
        set_margin_start(rValue.toInt32());
    }
    else if (rKey == "margin-end" || rKey == "margin-right")
    {
        assert(rKey == "margin-end" && "margin-right deprecated in favor of margin-end");
        set_margin_end(rValue.toInt32());
    }
    else if (rKey == "margin-top")
        set_margin_top(rValue.toInt32());
    else if (rKey == "margin-bottom")
        set_margin_bottom(rValue.toInt32());
    else if (rKey == "hscrollbar-policy")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOHSCROLL|WB_HSCROLL);
        if (rValue == "always")
            nBits |= WB_HSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOHSCROLL;
        SetStyle(nBits);
    }
    else if (rKey == "vscrollbar-policy")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOVSCROLL|WB_VSCROLL);
        if (rValue == "always")
            nBits |= WB_VSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOVSCROLL;
        SetStyle(nBits);
    }
    else if (rKey == "accessible-name")
    {
        SetAccessibleName(rValue);
    }
    else if (rKey == "accessible-description")
    {
        SetAccessibleDescription(rValue);
    }
    else if (rKey == "accessible-role")
    {
        sal_Int16 role = BuilderUtils::getRoleFromName(rValue);
        if (role != com::sun::star::accessibility::AccessibleRole::UNKNOWN)
            SetAccessibleRole(role);
    }
    else if (rKey == "use-markup")
    {
        //https://live.gnome.org/GnomeGoals/RemoveMarkupInMessages
        SAL_WARN_IF(toBool(rValue), "vcl.layout", "Use pango attributes instead of mark-up");
    }
    else if (rKey == "has-focus")
    {
        if (toBool(rValue))
            GrabFocus();
    }
    else if (rKey == "can-focus")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_TABSTOP|WB_NOTABSTOP);
        if (toBool(rValue))
            nBits |= WB_TABSTOP;
        else
            nBits |= WB_NOTABSTOP;
        SetStyle(nBits);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled property: " << rKey);
        return false;
    }
    return true;
}

void Window::set_height_request(sal_Int32 nHeightRequest)
{
    if (!mpWindowImpl)
        return;

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();

    if ( pWindowImpl->mnHeightRequest != nHeightRequest )
    {
        pWindowImpl->mnHeightRequest = nHeightRequest;
        queue_resize();
    }
}

void Window::set_width_request(sal_Int32 nWidthRequest)
{
    if (!mpWindowImpl)
        return;

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();

    if ( pWindowImpl->mnWidthRequest != nWidthRequest )
    {
        pWindowImpl->mnWidthRequest = nWidthRequest;
        queue_resize();
    }
}

Size Window::get_ungrouped_preferred_size() const
{
    Size aRet(get_width_request(), get_height_request());
    if (aRet.Width() == -1 || aRet.Height() == -1)
    {
        //cache gets blown away by queue_resize
        WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
        if (pWindowImpl->mnOptimalWidthCache == -1 || pWindowImpl->mnOptimalHeightCache == -1)
        {
            Size aOptimal(GetOptimalSize());
            pWindowImpl->mnOptimalWidthCache = aOptimal.Width();
            pWindowImpl->mnOptimalHeightCache = aOptimal.Height();
        }

        if (aRet.Width() == -1)
            aRet.setWidth( pWindowImpl->mnOptimalWidthCache );
        if (aRet.Height() == -1)
            aRet.setHeight( pWindowImpl->mnOptimalHeightCache );
    }
    return aRet;
}

Size Window::get_preferred_size() const
{
    Size aRet(get_ungrouped_preferred_size());

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->m_xSizeGroup)
    {
        const VclSizeGroupMode eMode = pWindowImpl->m_xSizeGroup->get_mode();
        if (eMode != VclSizeGroupMode::NONE)
        {
            const bool bIgnoreInHidden = pWindowImpl->m_xSizeGroup->get_ignore_hidden();
            const std::set<VclPtr<vcl::Window> > &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
            for (auto const& window : rWindows)
            {
                const vcl::Window *pOther = window;
                if (pOther == this)
                    continue;
                if (bIgnoreInHidden && !pOther->IsVisible())
                    continue;
                Size aOtherSize = pOther->get_ungrouped_preferred_size();
                if (eMode == VclSizeGroupMode::Both || eMode == VclSizeGroupMode::Horizontal)
                    aRet.setWidth( std::max(aRet.Width(), aOtherSize.Width()) );
                if (eMode == VclSizeGroupMode::Both || eMode == VclSizeGroupMode::Vertical)
                    aRet.setHeight( std::max(aRet.Height(), aOtherSize.Height()) );
            }
        }
    }

    return aRet;
}

VclAlign Window::get_halign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->meHalign;
}

void Window::set_halign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->meHalign = eAlign;
}

VclAlign Window::get_valign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->meValign;
}

void Window::set_valign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->meValign = eAlign;
}

bool Window::get_hexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbHexpand;
}

void Window::set_hexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbHexpand = bExpand;
}

bool Window::get_vexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbVexpand;
}

void Window::set_vexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbVexpand = bExpand;
}

bool Window::get_expand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbExpand;
}

void Window::set_expand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbExpand = bExpand;
}

VclPackType Window::get_pack_type() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mePackType;
}

void Window::set_pack_type(VclPackType ePackType)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mePackType = ePackType;
}

sal_Int32 Window::get_padding() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnPadding;
}

void Window::set_padding(sal_Int32 nPadding)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnPadding = nPadding;
}

bool Window::get_fill() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbFill;
}

void Window::set_fill(bool bFill)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbFill = bFill;
}

sal_Int32 Window::get_grid_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnGridWidth;
}

void Window::set_grid_width(sal_Int32 nCols)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnGridWidth = nCols;
}

sal_Int32 Window::get_grid_left_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnGridLeftAttach;
}

void Window::set_grid_left_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnGridLeftAttach = nAttach;
}

sal_Int32 Window::get_grid_height() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnGridHeight;
}

void Window::set_grid_height(sal_Int32 nRows)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnGridHeight = nRows;
}

sal_Int32 Window::get_grid_top_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnGridTopAttach;
}

void Window::set_grid_top_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnGridTopAttach = nAttach;
}

void Window::set_border_width(sal_Int32 nBorderWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mnBorderWidth = nBorderWidth;
}

sal_Int32 Window::get_border_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnBorderWidth;
}

void Window::set_margin_start(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->mnMarginLeft != nWidth)
    {
        pWindowImpl->mnMarginLeft = nWidth;
        queue_resize();
    }
}

sal_Int32 Window::get_margin_start() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnMarginLeft;
}

void Window::set_margin_end(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->mnMarginRight != nWidth)
    {
        pWindowImpl->mnMarginRight = nWidth;
        queue_resize();
    }
}

sal_Int32 Window::get_margin_end() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnMarginRight;
}

void Window::set_margin_top(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->mnMarginTop != nWidth)
    {
        pWindowImpl->mnMarginTop = nWidth;
        queue_resize();
    }
}

sal_Int32 Window::get_margin_top() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnMarginTop;
}

void Window::set_margin_bottom(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    if (pWindowImpl->mnMarginBottom != nWidth)
    {
        pWindowImpl->mnMarginBottom = nWidth;
        queue_resize();
    }
}

sal_Int32 Window::get_margin_bottom() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnMarginBottom;
}

sal_Int32 Window::get_height_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnHeightRequest;
}

sal_Int32 Window::get_width_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mnWidthRequest;
}

bool Window::get_secondary() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbSecondary;
}

void Window::set_secondary(bool bSecondary)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbSecondary = bSecondary;
}

bool Window::get_non_homogeneous() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    return pWindowImpl->mbNonHomogeneous;
}

void Window::set_non_homogeneous(bool bNonHomogeneous)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    pWindowImpl->mbNonHomogeneous = bNonHomogeneous;
}

void Window::add_to_size_group(const std::shared_ptr<VclSizeGroup>& xGroup)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    //To-Do, multiple groups
    pWindowImpl->m_xSizeGroup = xGroup;
    pWindowImpl->m_xSizeGroup->insert(this);
    if (VclSizeGroupMode::NONE != pWindowImpl->m_xSizeGroup->get_mode())
        queue_resize();
}

void Window::remove_from_all_size_groups()
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl.get() : mpWindowImpl.get();
    //To-Do, multiple groups
    if (pWindowImpl->m_xSizeGroup)
    {
        if (VclSizeGroupMode::NONE != pWindowImpl->m_xSizeGroup->get_mode())
            queue_resize();
        pWindowImpl->m_xSizeGroup->erase(this);
        pWindowImpl->m_xSizeGroup.reset();
    }
}

void Window::add_mnemonic_label(FixedText *pLabel)
{
    std::vector<VclPtr<FixedText> >& v = mpWindowImpl->m_aMnemonicLabels;
    if (std::find(v.begin(), v.end(), VclPtr<FixedText>(pLabel)) != v.end())
        return;
    v.emplace_back(pLabel);
    pLabel->set_mnemonic_widget(this);
}

void Window::remove_mnemonic_label(FixedText *pLabel)
{
    std::vector<VclPtr<FixedText> >& v = mpWindowImpl->m_aMnemonicLabels;
    auto aFind = std::find(v.begin(), v.end(), VclPtr<FixedText>(pLabel));
    if (aFind == v.end())
        return;
    v.erase(aFind);
    pLabel->set_mnemonic_widget(nullptr);
}

const std::vector<VclPtr<FixedText> >& Window::list_mnemonic_labels() const
{
    return mpWindowImpl->m_aMnemonicLabels;
}

} /* namespace vcl */

void InvertFocusRect(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    const int nBorder = 1;
    rRenderContext.Invert(tools::Rectangle(Point(rRect.Left(), rRect.Top()), Size(rRect.GetWidth(), nBorder)), InvertFlags::N50);
    rRenderContext.Invert(tools::Rectangle(Point(rRect.Left(), rRect.Bottom()-nBorder+1), Size(rRect.GetWidth(), nBorder)), InvertFlags::N50);
    rRenderContext.Invert(tools::Rectangle(Point(rRect.Left(), rRect.Top()+nBorder), Size(nBorder, rRect.GetHeight()-(nBorder*2))), InvertFlags::N50);
    rRenderContext.Invert(tools::Rectangle(Point(rRect.Right()-nBorder+1, rRect.Top()+nBorder), Size(nBorder, rRect.GetHeight()-(nBorder*2))), InvertFlags::N50);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
