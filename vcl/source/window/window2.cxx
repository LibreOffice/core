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
#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/dialog.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/timer.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/settings.hxx>

#include <window.h>
#include <outfont.hxx>
#include <outdev.h>
#include <svdata.hxx>
#include <impbmp.hxx>
#include <salbmp.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <scrwnd.hxx>

using namespace com::sun::star;

namespace vcl {

void Window::ShowFocus( const Rectangle& rRect )
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
                if ( *(pWinData->mpFocusRect) == rRect )
                {
                    mpWindowImpl->mbInShowFocus = false;
                    return;
                }

                ImplInvertFocus( *(pWinData->mpFocusRect) );
            }

            ImplInvertFocus( rRect );
        }
        if ( !pWinData->mpFocusRect )
            pWinData->mpFocusRect = new Rectangle( rRect );
        else
            *(pWinData->mpFocusRect) = rRect;
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
            ImplInvertFocus( *(ImplGetWinData()->mpFocusRect) );
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

void Window::ShowTracking( const Rectangle& rRect, sal_uInt16 nFlags )
{
    ImplWinData* pWinData = ImplGetWinData();

    if ( !mpWindowImpl->mbInPaint || !(nFlags & SHOWTRACK_WINDOW) )
    {
        if ( mpWindowImpl->mbTrackVisible )
        {
            if ( (*(pWinData->mpTrackRect)  == rRect) &&
                 (pWinData->mnTrackFlags    == nFlags) )
                return;

            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        }

        InvertTracking( rRect, nFlags );
    }

    if ( !pWinData->mpTrackRect )
        pWinData->mpTrackRect = new Rectangle( rRect );
    else
        *(pWinData->mpTrackRect) = rRect;
    pWinData->mnTrackFlags      = nFlags;
    mpWindowImpl->mbTrackVisible              = true;
}

void Window::HideTracking()
{
    if ( mpWindowImpl->mbTrackVisible )
    {
        ImplWinData* pWinData = ImplGetWinData();
        if ( !mpWindowImpl->mbInPaint || !(pWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        mpWindowImpl->mbTrackVisible = false;
    }
}

void Window::InvertTracking( const Rectangle& rRect, sal_uInt16 nFlags )
{
    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect( pOutDev->ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

    SalGraphics* pGraphics;

    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !pOutDev->AcquireGraphics() )
                return;
        }

        if ( mbInitClipRegion )
            InitClipRegion();

        if ( mbOutputClipped )
            return;

        pGraphics = mpGraphics;
    }
    else
    {
        pGraphics = ImplGetFrameGraphics();

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aPoint( mnOutOffX, mnOutOffY );
            vcl::Region aRegion( Rectangle( aPoint,
                                       Size( mnOutWidth, mnOutHeight ) ) );
            ImplClipBoundaries( aRegion, false, false );
            pOutDev->SelectClipRegion( aRegion, pGraphics );
        }
    }

    sal_uInt16 nStyle = nFlags & SHOWTRACK_STYLE;
    if ( nStyle == SHOWTRACK_OBJECT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_TRACKFRAME, this );
    else if ( nStyle == SHOWTRACK_SPLIT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_50, this );
    else
    {
        long nBorder = 1;
        if ( nStyle == SHOWTRACK_BIG )
            nBorder = 5;
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), nBorder, SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Left(), aRect.Bottom()-nBorder+1, aRect.GetWidth(), nBorder, SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Left(), aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Right()-nBorder+1, aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50, this );
    }
}

void Window::InvertTracking( const tools::Polygon& rPoly, sal_uInt16 nFlags )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    OutputDevice *pOutDev = GetOutDev();

    tools::Polygon aPoly( pOutDev->ImplLogicToDevicePixel( rPoly ) );

    SalGraphics* pGraphics;

    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !pOutDev->AcquireGraphics() )
                return;
        }

        if ( mbInitClipRegion )
            InitClipRegion();

        if ( mbOutputClipped )
            return;

        pGraphics = mpGraphics;
    }
    else
    {
        pGraphics = ImplGetFrameGraphics();

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aPoint( mnOutOffX, mnOutOffY );
            vcl::Region aRegion( Rectangle( aPoint,
                                       Size( mnOutWidth, mnOutHeight ) ) );
            ImplClipBoundaries( aRegion, false, false );
            pOutDev->SelectClipRegion( aRegion, pGraphics );
        }
    }

    const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aPoly.GetConstPointAry());
    pGraphics->Invert( nPoints, pPtAry, SAL_INVERT_TRACKFRAME, this );
}

IMPL_LINK_TYPED( Window, ImplTrackTimerHdl, Timer*, pTimer, void )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Bei Button-Repeat muessen wir den Timeout umsetzen
    if ( pSVData->maWinData.mnTrackFlags & StartTrackingFlags::ButtonRepeat )
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );

    // Tracking-Event erzeugen
    Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // - RTL - re-mirror frame pos at pChild
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aMousePos );
    }
    MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                           mpWindowImpl->mpFrameData->mnClickCount, MouseEventModifiers::NONE,
                           mpWindowImpl->mpFrameData->mnMouseCode,
                           mpWindowImpl->mpFrameData->mnMouseCode );
    TrackingEvent   aTEvt( aMEvt, TrackingEventFlags::Repeat );
    Tracking( aTEvt );
}

void Window::StartTracking( StartTrackingFlags nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin.get() != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel );
    }

    if ( nFlags & (StartTrackingFlags::ScrollRepeat | StartTrackingFlags::ButtonRepeat) )
    {
        pSVData->maWinData.mpTrackTimer = new AutoTimer;

        if ( nFlags & StartTrackingFlags::ScrollRepeat )
            pSVData->maWinData.mpTrackTimer->SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
        else
            pSVData->maWinData.mpTrackTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
        pSVData->maWinData.mpTrackTimer->SetTimeoutHdl( LINK( this, Window, ImplTrackTimerHdl ) );
        pSVData->maWinData.mpTrackTimer->Start();
    }

    pSVData->maWinData.mpTrackWin   = this;
    pSVData->maWinData.mnTrackFlags = nFlags;
    CaptureMouse();
}

void Window::EndTracking( TrackingEventFlags nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin.get() == this )
    {
        // due to DbgChkThis in brackets, as the window could be destroyed
        // in the handler
        {

        if ( pSVData->maWinData.mpTrackTimer )
        {
            delete pSVData->maWinData.mpTrackTimer;
            pSVData->maWinData.mpTrackTimer = nullptr;
        }

        pSVData->maWinData.mpTrackWin    = nullptr;
        pSVData->maWinData.mnTrackFlags  = StartTrackingFlags::NONE;
        ReleaseMouse();
        }

        // call EndTracking if required
        if ( !(nFlags & TrackingEventFlags::DontCallHdl) )
        {
            Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
            if( ImplIsAntiparallel() )
            {
                // - RTL - re-mirror frame pos at pChild
                const OutputDevice *pOutDev = GetOutDev();
                pOutDev->ReMirror( aMousePos );
            }

            MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                                   mpWindowImpl->mpFrameData->mnClickCount, MouseEventModifiers::NONE,
                                   mpWindowImpl->mpFrameData->mnMouseCode,
                                   mpWindowImpl->mpFrameData->mnMouseCode );
            TrackingEvent   aTEvt( aMEvt, nFlags | TrackingEventFlags::End );
            Tracking( aTEvt );
        }
    }
}

bool Window::IsTracking() const
{
    return (ImplGetSVData()->maWinData.mpTrackWin == this);
}

void Window::StartAutoScroll( StartAutoScrollFlags nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpAutoScrollWin.get() != this )
    {
        if ( pSVData->maWinData.mpAutoScrollWin )
            pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
    }

    pSVData->maWinData.mpAutoScrollWin = this;
    pSVData->maWinData.mnAutoScrollFlags = nFlags;
    pSVData->maAppData.mpWheelWindow = VclPtr<ImplWheelWindow>::Create( this );
}

void Window::EndAutoScroll()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpAutoScrollWin.get() == this )
    {
        pSVData->maWinData.mpAutoScrollWin = nullptr;
        pSVData->maWinData.mnAutoScrollFlags = StartAutoScrollFlags::NONE;
        pSVData->maAppData.mpWheelWindow->ImplStop();
        pSVData->maAppData.mpWheelWindow->doLazyDelete();
        pSVData->maAppData.mpWheelWindow = nullptr;
    }
}

sal_uIntPtr Window::SaveFocus()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpFocusWin )
    {
        ImplFocusDelData* pDelData = new ImplFocusDelData;
        pSVData->maWinData.mpFocusWin->ImplAddDel( pDelData );
        pDelData->mpFocusWin = pSVData->maWinData.mpFocusWin;
        return reinterpret_cast<sal_uIntPtr>(pDelData);
    }
    else
        return 0;
}

bool Window::EndSaveFocus( sal_uIntPtr nSaveId, bool bRestore )
{
    if ( !nSaveId )
        return false;
    else
    {
        bool                bOK = true;
        ImplFocusDelData*   pDelData = reinterpret_cast<ImplFocusDelData*>(nSaveId);
        if ( !pDelData->IsDead() )
        {
            pDelData->mpFocusWin->ImplRemoveDel( pDelData );
            if ( bRestore )
                pDelData->mpFocusWin->GrabFocus();
        }
        else
            bOK = !bRestore;
        delete pDelData;
        return bOK;
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

inline long WinFloatRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}

void Window::SetZoomedPointFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont)
{
    const Fraction& rZoom = GetZoom();
    if (rZoom.GetNumerator() != rZoom.GetDenominator())
    {
        vcl::Font aFont(rFont);
        Size aSize = aFont.GetSize();
        double n = double(aSize.Width());
        n *= double(rZoom.GetNumerator());
        n /= double(rZoom.GetDenominator());
        aSize.Width() = WinFloatRound(n);
        n = double(aSize.Height());
        n *= double(rZoom.GetNumerator());
        n /= double(rZoom.GetDenominator());
        aSize.Height() = WinFloatRound(n);
        aFont.SetSize(aSize);
        SetPointFont(rRenderContext, aFont);

        // Use another font if the representation is to be scaled,
        // and the actual font is not scalable
        FontMetric aMetric = rRenderContext.GetFontMetric();
        long nFontDiff = std::abs(rRenderContext.GetFont().GetSize().Height() - aMetric.GetSize().Height());
        if ((aMetric.GetType() == TYPE_RASTER) && (nFontDiff >= 2))
        {
            DefaultFontType nType;
            if (aMetric.GetPitch() == PITCH_FIXED)
                nType = DefaultFontType::FIXED;
            else
                nType = DefaultFontType::UI_SANS;
            vcl::Font aTempFont = OutputDevice::GetDefaultFont(nType, rRenderContext.GetSettings().GetLanguageTag().getLanguageType(), GetDefaultFontFlags::NONE);
            aFont.SetName(aTempFont.GetFamilyName());
            SetPointFont(rRenderContext, aFont);
        }
    }
    else
    {
        SetPointFont(rRenderContext, rFont);
    }
}

long Window::CalcZoom( long nCalc ) const
{

    const Fraction& rZoom = GetZoom();
    if ( rZoom.GetNumerator() != rZoom.GetDenominator() )
    {
        double n = (double)nCalc;
        n *= (double)rZoom.GetNumerator();
        n /= (double)rZoom.GetDenominator();
        nCalc = WinFloatRound( n );
    }
    return nCalc;
}

void Window::SetControlFont()
{
    if (mpWindowImpl && mpWindowImpl->mpControlFont)
    {
        delete mpWindowImpl->mpControlFont;
        mpWindowImpl->mpControlFont = nullptr;
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
        mpWindowImpl->mpControlFont = new vcl::Font(rFont);

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
        mpWindowImpl->maControlForeground = Color(COL_TRANSPARENT);
        mpWindowImpl->mbControlForeground = false;
        CompatStateChanged(StateChangedType::ControlForeground);
    }
}

void Window::SetControlForeground(const Color& rColor)
{
    if (rColor.GetTransparency())
    {
        if (mpWindowImpl->mbControlForeground)
        {
            mpWindowImpl->maControlForeground = Color(COL_TRANSPARENT);
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
        mpWindowImpl->maControlBackground = Color(COL_TRANSPARENT);
        mpWindowImpl->mbControlBackground = false;
        CompatStateChanged(StateChangedType::ControlBackground);
    }
}

void Window::SetControlBackground(const Color& rColor)
{
    if (rColor.GetTransparency())
    {
        if (mpWindowImpl->mbControlBackground)
        {
            mpWindowImpl->maControlBackground = Color(COL_TRANSPARENT);
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
    aSz.Width()  += mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() += mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
    return aSz;
}

Size Window::CalcOutputSize( const Size& rWinSz ) const
{
    Size aSz = rWinSz;
    aSz.Width()  -= mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() -= mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
    return aSz;
}

vcl::Font Window::GetDrawPixelFont(OutputDevice* pDev) const
{
    vcl::Font aFont = GetPointFont(*pDev);
    Size aFontSize = aFont.GetSize();
    MapMode aPtMapMode(MAP_POINT);
    aFontSize = pDev->LogicToPixel( aFontSize, aPtMapMode );
    aFont.SetSize( aFontSize );
    return aFont;
}

long Window::GetDrawPixel( OutputDevice* pDev, long nPixels ) const
{
    long nP = nPixels;
    if ( pDev->GetOutDevType() != OUTDEV_WINDOW )
    {
        MapMode aMap( MAP_100TH_MM );
        Size aSz( nP, 0 );
        aSz = PixelToLogic( aSz, aMap );
        aSz = pDev->LogicToPixel( aSz, aMap );
        nP = aSz.Width();
    }
    return nP;
}

static void lcl_HandleScrollHelper( ScrollBar* pScrl, long nN, bool isMultiplyByLineSize )
{
    if ( pScrl && nN && pScrl->IsEnabled() && pScrl->IsInputEnabled() && ! pScrl->IsInModalMode() )
    {
        long nNewPos = pScrl->GetThumbPos();

        if ( nN == -LONG_MAX )
            nNewPos += pScrl->GetPageSize();
        else if ( nN == LONG_MAX )
            nNewPos -= pScrl->GetPageSize();
        else
        {
            // allowing both chunked and continuous scrolling
            if(isMultiplyByLineSize){
                nN*=pScrl->GetLineSize();
            }

            const double fVal = (double)(nNewPos - nN);

            if ( fVal < LONG_MIN )
                nNewPos = LONG_MIN;
            else if ( fVal > LONG_MAX )
                nNewPos = LONG_MAX;
            else
                nNewPos = (long)fVal;
        }

        pScrl->DoScroll( nNewPos );
    }

}

bool Window::HandleScrollCommand( const CommandEvent& rCmd,
                                  ScrollBar* pHScrl, ScrollBar* pVScrl )
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
                         pHScrl->IsEnabled() && pHScrl->IsInputEnabled() && ! pHScrl->IsInModalMode() )
                        nFlags |= StartAutoScrollFlags::Horz;
                }
                if ( pVScrl )
                {
                    if ( (pVScrl->GetVisibleSize() < pVScrl->GetRangeMax()) &&
                         pVScrl->IsEnabled() && pVScrl->IsInputEnabled() && ! pVScrl->IsInModalMode() )
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
                        sal_uLong nScrollLines = pData->GetScrollLines();
                        long nLines;
                        if ( nScrollLines == COMMAND_WHEEL_PAGESCROLL )
                        {
                            if ( pData->GetDelta() < 0 )
                                nLines = -LONG_MAX;
                            else
                                nLines = LONG_MAX;
                        }
                        else
                            nLines = pData->GetNotchDelta() * (long)nScrollLines;
                        if ( nLines )
                        {
                            ImplHandleScroll( nullptr,
                                          0L,
                                          pData->IsHorz() ? pHScrl : pVScrl,
                                          nLines );
                            bRet = true;
                        }
                    }
                    else
                    {
                        // Mobile / touch scrolling section
                        const Point & deltaPoint = rCmd.GetMousePosPixel();

                        double deltaXInPixels = double(deltaPoint.X());
                        double deltaYInPixels = double(deltaPoint.Y());
                        Size winSize = this->GetOutputSizePixel();

                        if(pHScrl)
                        {
                            double visSizeX = double(pHScrl->GetVisibleSize());
                            double ratioX = deltaXInPixels / double(winSize.getWidth());
                            long deltaXInLogic = long(visSizeX * ratioX);
                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            long lineSizeX = pHScrl->GetLineSize();

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
                                bool isMultiplyByLineSize = true;
#else
                                bool isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pHScrl, deltaXInLogic, isMultiplyByLineSize );
                                bRet = true;
                            }
                        }
                        if(pVScrl)
                        {
                            double visSizeY = double(pVScrl->GetVisibleSize());
                            double ratioY = deltaYInPixels / double(winSize.getHeight());
                            long deltaYInLogic = long(visSizeY * ratioY);

                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            long lineSizeY = pVScrl->GetLineSize();
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
                                bool isMultiplyByLineSize = true;
#else
                                bool isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pVScrl, deltaYInLogic, isMultiplyByLineSize );

                                bRet = true;
                            }
                        }
                    }
                }
            }
            break;

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

// Note that when called for CommandEventId::Wheel above, despite its name,
// pVScrl isn't necessarily the vertical scroll bar. Depending on
// whether the scroll is horizontal or vertical, it is either the
// horizontal or vertical scroll bar. nY is correspondingly either
// the horizontal or vertical scroll amount.

void Window::ImplHandleScroll( ScrollBar* pHScrl, long nX,
                               ScrollBar* pVScrl, long nY )
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

// retrieves the list of owner draw decorated windows for this window hiearchy
::std::vector<VclPtr<vcl::Window> >& Window::ImplGetOwnerDrawList()
{
    return ImplGetTopmostFrameWindow()->mpWindowImpl->mpFrameData->maOwnerDrawList;
}

void Window::SetHelpId( const OString& rHelpId )
{
    mpWindowImpl->maHelpId = rHelpId;
}

const OString& Window::GetHelpId() const
{
    return mpWindowImpl->maHelpId;
}

void Window::SetUniqueId( const OString& rUniqueId )
{
    mpWindowImpl->maUniqId = rUniqueId;
}

const OString& Window::GetUniqueId() const
{
    return mpWindowImpl->maUniqId;
}

// --------- old inline methods ---------------

vcl::Window* Window::ImplGetWindow()
{
    if ( mpWindowImpl->mpClientWindow )
        return mpWindowImpl->mpClientWindow;
    else
        return this;
}

ImplFrameData* Window::ImplGetFrameData()
{
    return mpWindowImpl ? mpWindowImpl->mpFrameData : nullptr;
}

SalFrame* Window::ImplGetFrame() const
{
    return mpWindowImpl ? mpWindowImpl->mpFrame : nullptr;
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
    if ( mpWindowImpl->mbOverlapWin )
        return this;
    else
        return mpWindowImpl->mpOverlapWindow;
}

const vcl::Window* Window::ImplGetFirstOverlapWindow() const
{
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

Point Window::ImplOutputToFrame( const Point& rPos )
{
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

Point Window::ImplFrameToOutput( const Point& rPos )
{
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

void Window::SetCompoundControl( bool bCompound )
{
    if (mpWindowImpl)
        mpWindowImpl->mbCompoundControl = bCompound;
}

void Window::IncrementLockCount()
{
    mpWindowImpl->mnLockCount++;
}

void Window::DecrementLockCount()
{
    mpWindowImpl->mnLockCount--;
}

WinBits Window::GetStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnStyle : 0;
}

WinBits Window::GetPrevStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnPrevStyle : 0;
}

WinBits Window::GetExtendedStyle() const
{
    return mpWindowImpl ? mpWindowImpl->mnExtendedStyle : 0;
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
        return 0;
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

void Window::EnableAllResize( bool bEnable )
{
    mpWindowImpl->mbAllResize = bEnable;
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
    return (mpWindowImpl->mpControlFont != nullptr);
}

Color Window::GetControlForeground() const
{
    return mpWindowImpl->maControlForeground;
}

bool Window::IsControlForeground() const
{
    return mpWindowImpl->mbControlForeground;
}

Color Window::GetControlBackground() const
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
    return mpWindowImpl->mbCompoundControl;
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

void Window::SetData( void* pNewData )
{
    mpWindowImpl->mpUserData = pNewData;
}

void* Window::GetData() const
{
    return mpWindowImpl->mpUserData;
}

bool Window::IsCreatedWithToolkit() const
{
    return mpWindowImpl->mbCreatedWithToolkit;
}

void Window::SetCreatedWithToolkit( bool b )
{
    mpWindowImpl->mbCreatedWithToolkit = b;
}

const Pointer& Window::GetPointer() const
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
    bool queue_ungrouped_resize(vcl::Window *pOrigWindow)
    {
        bool bSomeoneCares = false;

        vcl::Window *pWindow = pOrigWindow->GetParent();
        if (pWindow)
        {
            if (isContainerWindow(*pWindow))
            {
                bSomeoneCares = true;
            }
            else if (pWindow->GetType() == WINDOW_TABCONTROL)
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
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnOptimalWidthCache = -1;
    pWindowImpl->mnOptimalHeightCache = -1;
}

void Window::queue_resize(StateChangedType eReason)
{
    if (IsDisposed())
        return;

    bool bSomeoneCares = queue_ungrouped_resize(this);

    if (eReason != StateChangedType::Visible)
    {
        InvalidateSizeCache();
    }

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    if (pWindowImpl->m_xSizeGroup && pWindowImpl->m_xSizeGroup->get_mode() != VCL_SIZE_GROUP_NONE)
    {
        std::set<VclPtr<vcl::Window> > &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
        for (auto aI = rWindows.begin(), aEnd = rWindows.end(); aI != aEnd; ++aI)
        {
            vcl::Window *pOther = *aI;
            if (pOther == this)
                continue;
            queue_ungrouped_resize(pOther);
        }
    }

    if (bSomeoneCares && !mpWindowImpl->mbInDispose)
    {
        //fdo#57090 force a resync of the borders of the borderwindow onto this
        //window in case they have changed
        vcl::Window* pBorderWindow = ImplGetBorderWindow();
        if (pBorderWindow)
            pBorderWindow->Resize();
    }
}

namespace
{
    VclAlign toAlign(const OString &rValue)
    {
        VclAlign eRet = VCL_ALIGN_FILL;

        if (rValue == "fill")
            eRet = VCL_ALIGN_FILL;
        else if (rValue == "start")
            eRet = VCL_ALIGN_START;
        else if (rValue == "end")
            eRet = VCL_ALIGN_END;
        else if (rValue == "center")
            eRet = VCL_ALIGN_CENTER;
        return eRet;
    }
}

bool Window::set_font_attribute(const OString &rKey, const OString &rValue)
{
    if (rKey == "weight")
    {
        vcl::Font aFont(GetControlFont());
        if (rValue == "thin")
            aFont.SetWeight(WEIGHT_THIN);
        else if (rValue == "ultralight")
            aFont.SetWeight(WEIGHT_ULTRALIGHT);
        else if (rValue == "light")
            aFont.SetWeight(WEIGHT_LIGHT);
        else if (rValue == "book")
            aFont.SetWeight(WEIGHT_SEMILIGHT);
        else if (rValue == "normal")
            aFont.SetWeight(WEIGHT_NORMAL);
        else if (rValue == "medium")
            aFont.SetWeight(WEIGHT_MEDIUM);
        else if (rValue == "semibold")
            aFont.SetWeight(WEIGHT_SEMIBOLD);
        else if (rValue == "bold")
            aFont.SetWeight(WEIGHT_BOLD);
        else if (rValue == "ultrabold")
            aFont.SetWeight(WEIGHT_ULTRABOLD);
        else
            aFont.SetWeight(WEIGHT_BLACK);
        SetControlFont(aFont);
    }
    else if (rKey == "style")
    {
        vcl::Font aFont(GetControlFont());
        if (rValue == "normal")
            aFont.SetItalic(ITALIC_NONE);
        else if (rValue == "oblique")
            aFont.SetItalic(ITALIC_OBLIQUE);
        else if (rValue == "italic")
            aFont.SetItalic(ITALIC_NORMAL);
        SetControlFont(aFont);
    }
    else if (rKey == "underline")
    {
        vcl::Font aFont(GetControlFont());
        aFont.SetUnderline(toBool(rValue) ? UNDERLINE_SINGLE : UNDERLINE_NONE);
        SetControlFont(aFont);
    }
    else if (rKey == "size")
    {
        vcl::Font aFont(GetControlFont());
        sal_Int32 nHeight = rValue.toInt32() / 1000;
        aFont.SetHeight(nHeight);
        SetControlFont(aFont);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled font attribute: " << rKey.getStr());
        return false;
    }
    return true;
}

bool Window::set_property(const OString &rKey, const OString &rValue)
{
    if ((rKey == "label") || (rKey == "title") || (rKey == "text") )
    {
        SetText(OStringToOUString(VclBuilder::convertMnemonicMarkup(rValue), RTL_TEXTENCODING_UTF8));
    }
    else if (rKey == "visible")
        Show(toBool(rValue));
    else if (rKey == "sensitive")
        Enable(toBool(rValue));
    else if (rKey == "resizable")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_SIZEABLE);
        if (toBool(rValue))
            nBits |= WB_SIZEABLE;
        SetStyle(nBits);
    }
    else if (rKey == "xalign")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

        float f = rValue.toFloat();
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
        if (f == 0.0)
            nBits |= WB_TOP;
        else if (f == 1.0)
            nBits |= WB_BOTTOM;
        else if (f == 0.5)
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey == "wrap")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_WORDBREAK);
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
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "tooltip-text")
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "border-width")
        set_border_width(rValue.toInt32());
    else if (rKey == "margin-left")
        set_margin_left(rValue.toInt32());
    else if (rKey == "margin-right")
        set_margin_right(rValue.toInt32());
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
        SetAccessibleName(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    }
    else if (rKey == "accessible-description")
    {
        SetAccessibleDescription(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
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
    else
    {
        SAL_INFO("vcl.layout", "unhandled property: " << rKey.getStr());
        return false;
    }
    return true;
}

void Window::set_height_request(sal_Int32 nHeightRequest)
{
    if (!mpWindowImpl)
        return;

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

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

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

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
        WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
        if (pWindowImpl->mnOptimalWidthCache == -1 || pWindowImpl->mnOptimalHeightCache == -1)
        {
            Size aOptimal(GetOptimalSize());
            pWindowImpl->mnOptimalWidthCache = aOptimal.Width();
            pWindowImpl->mnOptimalHeightCache = aOptimal.Height();
        }

        if (aRet.Width() == -1)
            aRet.Width() = pWindowImpl->mnOptimalWidthCache;
        if (aRet.Height() == -1)
            aRet.Height() = pWindowImpl->mnOptimalHeightCache;
    }
    return aRet;
}

Size Window::get_preferred_size() const
{
    Size aRet(get_ungrouped_preferred_size());

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    if (pWindowImpl->m_xSizeGroup)
    {
        const VclSizeGroupMode eMode = pWindowImpl->m_xSizeGroup->get_mode();
        if (eMode != VCL_SIZE_GROUP_NONE)
        {
            const bool bIgnoreInHidden = pWindowImpl->m_xSizeGroup->get_ignore_hidden();
            const std::set<VclPtr<vcl::Window> > &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
            for (auto aI = rWindows.begin(), aEnd = rWindows.end(); aI != aEnd; ++aI)
            {
                const vcl::Window *pOther = *aI;
                if (pOther == this)
                    continue;
                if (bIgnoreInHidden && !pOther->IsVisible())
                    continue;
                Size aOtherSize = pOther->get_ungrouped_preferred_size();
                if (eMode == VCL_SIZE_GROUP_BOTH || eMode == VCL_SIZE_GROUP_HORIZONTAL)
                    aRet.Width() = std::max(aRet.Width(), aOtherSize.Width());
                if (eMode == VCL_SIZE_GROUP_BOTH || eMode == VCL_SIZE_GROUP_VERTICAL)
                    aRet.Height() = std::max(aRet.Height(), aOtherSize.Height());
            }
        }
    }

    return aRet;
}

VclAlign Window::get_halign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->meHalign;
}

void Window::set_halign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->meHalign = eAlign;
}

VclAlign Window::get_valign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->meValign;
}

void Window::set_valign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->meValign = eAlign;
}

bool Window::get_hexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbHexpand;
}

void Window::set_hexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbHexpand = bExpand;
}

bool Window::get_vexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbVexpand;
}

void Window::set_vexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbVexpand = bExpand;
}

bool Window::get_expand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbExpand;
}

void Window::set_expand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbExpand = bExpand;
}

VclPackType Window::get_pack_type() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mePackType;
}

void Window::set_pack_type(VclPackType ePackType)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mePackType = ePackType;
}

sal_Int32 Window::get_padding() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnPadding;
}

void Window::set_padding(sal_Int32 nPadding)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnPadding = nPadding;
}

bool Window::get_fill() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbFill;
}

void Window::set_fill(bool bFill)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbFill = bFill;
}

sal_Int32 Window::get_grid_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridWidth;
}

void Window::set_grid_width(sal_Int32 nCols)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridWidth = nCols;
}

sal_Int32 Window::get_grid_left_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridLeftAttach;
}

void Window::set_grid_left_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridLeftAttach = nAttach;
}

sal_Int32 Window::get_grid_height() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridHeight;
}

void Window::set_grid_height(sal_Int32 nRows)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridHeight = nRows;
}

sal_Int32 Window::get_grid_top_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridTopAttach;
}

void Window::set_grid_top_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridTopAttach = nAttach;
}

void Window::set_border_width(sal_Int32 nBorderWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnBorderWidth = nBorderWidth;
}

sal_Int32 Window::get_border_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnBorderWidth;
}

void Window::set_margin_left(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginLeft = nWidth;
}

sal_Int32 Window::get_margin_left() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginLeft;
}

void Window::set_margin_right(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginRight = nWidth;
}

sal_Int32 Window::get_margin_right() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginRight;
}

void Window::set_margin_top(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginTop = nWidth;
}

sal_Int32 Window::get_margin_top() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginTop;
}

void Window::set_margin_bottom(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginBottom = nWidth;
}

sal_Int32 Window::get_margin_bottom() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginBottom;
}

sal_Int32 Window::get_height_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnHeightRequest;
}

sal_Int32 Window::get_width_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnWidthRequest;
}

bool Window::get_secondary() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbSecondary;
}

void Window::set_secondary(bool bSecondary)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbSecondary = bSecondary;
}

bool Window::get_non_homogeneous() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbNonHomogeneous;
}

void Window::set_non_homogeneous(bool bNonHomogeneous)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbNonHomogeneous = bNonHomogeneous;
}

void Window::add_to_size_group(std::shared_ptr<VclSizeGroup> xGroup)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    //To-Do, multiple groups
    pWindowImpl->m_xSizeGroup = xGroup;
    pWindowImpl->m_xSizeGroup->insert(this);
    if (VCL_SIZE_GROUP_NONE != pWindowImpl->m_xSizeGroup->get_mode())
        queue_resize();
}

void Window::remove_from_all_size_groups()
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    //To-Do, multiple groups
    if (pWindowImpl->m_xSizeGroup)
    {
        if (VCL_SIZE_GROUP_NONE != pWindowImpl->m_xSizeGroup->get_mode())
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
    v.push_back(pLabel);
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

std::vector<VclPtr<FixedText> > Window::list_mnemonic_labels() const
{
    return mpWindowImpl->m_aMnemonicLabels;
}

} /* namespace vcl */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
