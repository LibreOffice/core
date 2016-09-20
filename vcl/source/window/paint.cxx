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

#include <config_features.h>

#include <vcl/window.hxx>
#include <vcl/dialog.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>
#include <vcl/settings.hxx>

#include <sal/types.h>

#include <window.h>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <svdata.hxx>
#include <comphelper/lok.hxx>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLHelper.hxx>
#endif

#define IMPL_PAINT_PAINT            ((sal_uInt16)0x0001)
#define IMPL_PAINT_PAINTALL         ((sal_uInt16)0x0002)
#define IMPL_PAINT_PAINTALLCHILDREN   ((sal_uInt16)0x0004)
#define IMPL_PAINT_PAINTCHILDREN      ((sal_uInt16)0x0008)
#define IMPL_PAINT_ERASE            ((sal_uInt16)0x0010)
#define IMPL_PAINT_CHECKRTL         ((sal_uInt16)0x0020)

// PaintBufferGuard

PaintBufferGuard::PaintBufferGuard(ImplFrameData* pFrameData, vcl::Window* pWindow)
    : mpFrameData(pFrameData),
    m_pWindow(pWindow),
    mbBackground(false),
    mnOutOffX(0),
    mnOutOffY(0)
{
    if (!pFrameData->mpBuffer)
        return;

    // transfer various settings
    // FIXME: this must disappear as we move to RenderContext only,
    // the painting must become state-less, so that no actual
    // vcl::Window setting affects this
    mbBackground = pFrameData->mpBuffer->IsBackground();
    if (pWindow->IsBackground())
    {
        maBackground = pFrameData->mpBuffer->GetBackground();
        pFrameData->mpBuffer->SetBackground(pWindow->GetBackground());
    }
    //else
        //SAL_WARN("vcl.window", "the root of the double-buffering hierarchy should not have a transparent background");

    PushFlags nFlags = PushFlags::NONE;
    nFlags |= PushFlags::CLIPREGION;
    nFlags |= PushFlags::FILLCOLOR;
    nFlags |= PushFlags::FONT;
    nFlags |= PushFlags::LINECOLOR;
    nFlags |= PushFlags::MAPMODE;
    maSettings = pFrameData->mpBuffer->GetSettings();
    nFlags |= PushFlags::REFPOINT;
    nFlags |= PushFlags::TEXTCOLOR;
    nFlags |= PushFlags::TEXTLINECOLOR;
    nFlags |= PushFlags::OVERLINECOLOR;
    nFlags |= PushFlags::TEXTFILLCOLOR;
    nFlags |= PushFlags::TEXTALIGN;
    nFlags |= PushFlags::RASTEROP;
    nFlags |= PushFlags::TEXTLAYOUTMODE;
    nFlags |= PushFlags::TEXTLANGUAGE;
    pFrameData->mpBuffer->Push(nFlags);
    pFrameData->mpBuffer->SetClipRegion(pWindow->GetClipRegion());
    pFrameData->mpBuffer->SetFillColor(pWindow->GetFillColor());
    pFrameData->mpBuffer->SetFont(pWindow->GetFont());
    pFrameData->mpBuffer->SetLineColor(pWindow->GetLineColor());
    pFrameData->mpBuffer->SetMapMode(pWindow->GetMapMode());
    pFrameData->mpBuffer->SetRefPoint(pWindow->GetRefPoint());
    pFrameData->mpBuffer->SetSettings(pWindow->GetSettings());
    pFrameData->mpBuffer->SetTextColor(pWindow->GetTextColor());
    pFrameData->mpBuffer->SetTextLineColor(pWindow->GetTextLineColor());
    pFrameData->mpBuffer->SetOverlineColor(pWindow->GetOverlineColor());
    pFrameData->mpBuffer->SetTextFillColor(pWindow->GetTextFillColor());
    pFrameData->mpBuffer->SetTextAlign(pWindow->GetTextAlign());
    pFrameData->mpBuffer->SetRasterOp(pWindow->GetRasterOp());
    pFrameData->mpBuffer->SetLayoutMode(pWindow->GetLayoutMode());
    pFrameData->mpBuffer->SetDigitLanguage(pWindow->GetDigitLanguage());

    mnOutOffX = pFrameData->mpBuffer->GetOutOffXPixel();
    mnOutOffY = pFrameData->mpBuffer->GetOutOffYPixel();
    pFrameData->mpBuffer->SetOutOffXPixel(pWindow->GetOutOffXPixel());
    pFrameData->mpBuffer->SetOutOffYPixel(pWindow->GetOutOffYPixel());
}

PaintBufferGuard::~PaintBufferGuard()
{
    if (!mpFrameData->mpBuffer)
        return;

    if (!m_aPaintRect.IsEmpty())
    {
        // copy the buffer content to the actual window
        // export VCL_DOUBLEBUFFERING_AVOID_PAINT=1 to see where we are
        // painting directly instead of using Invalidate()
        // [ie. everything you can see was painted directly to the
        // window either above or in eg. an event handler]
        if (!getenv("VCL_DOUBLEBUFFERING_AVOID_PAINT"))
        {
            // Make sure that the +1 value GetSize() adds to the size is in pixels.
            Size aPaintRectSize;
            if (m_pWindow->GetMapMode().GetMapUnit() == MAP_PIXEL)
            {
                aPaintRectSize = m_aPaintRect.GetSize();
            }
            else
            {
                Rectangle aRectanglePixel = m_pWindow->LogicToPixel(m_aPaintRect);
                aPaintRectSize = m_pWindow->PixelToLogic(aRectanglePixel.GetSize());
            }

            m_pWindow->DrawOutDev(m_aPaintRect.TopLeft(), aPaintRectSize, m_aPaintRect.TopLeft(), aPaintRectSize, *mpFrameData->mpBuffer.get());
        }
    }

    // Restore buffer state.
    mpFrameData->mpBuffer->SetOutOffXPixel(mnOutOffX);
    mpFrameData->mpBuffer->SetOutOffYPixel(mnOutOffY);

    mpFrameData->mpBuffer->Pop();
    mpFrameData->mpBuffer->SetSettings(maSettings);
    if (mbBackground)
        mpFrameData->mpBuffer->SetBackground(maBackground);
    else
        mpFrameData->mpBuffer->SetBackground();
}

void PaintBufferGuard::SetPaintRect(const Rectangle& rRectangle)
{
    m_aPaintRect = rRectangle;
}

vcl::RenderContext* PaintBufferGuard::GetRenderContext()
{
    if (mpFrameData->mpBuffer)
        return mpFrameData->mpBuffer;
    else
        return m_pWindow;
}

class PaintHelper
{
private:
    VclPtr<vcl::Window> m_pWindow;
    vcl::Region* m_pChildRegion;
    Rectangle m_aSelectionRect;
    Rectangle m_aPaintRect;
    vcl::Region m_aPaintRegion;
    sal_uInt16 m_nPaintFlags;
    bool m_bPop : 1;
    bool m_bRestoreCursor : 1;
    bool m_bStartedBufferedPaint : 1; ///< This PaintHelper started a buffered paint, and should paint it on the screen when being destructed.
public:
    PaintHelper(vcl::Window* pWindow, sal_uInt16 nPaintFlags);
    void SetPop()
    {
        m_bPop = true;
    }
    void SetPaintRect(const Rectangle& rRect)
    {
        m_aPaintRect = rRect;
    }
    void SetSelectionRect(const Rectangle& rRect)
    {
        m_aSelectionRect = rRect;
    }
    void SetRestoreCursor(bool bRestoreCursor)
    {
        m_bRestoreCursor = bRestoreCursor;
    }
    bool GetRestoreCursor() const
    {
        return m_bRestoreCursor;
    }
    sal_uInt16 GetPaintFlags() const
    {
        return m_nPaintFlags;
    }
    vcl::Region& GetPaintRegion()
    {
        return m_aPaintRegion;
    }
    void DoPaint(const vcl::Region* pRegion);

    /// Start buffered paint: set it up to have the same settings as m_pWindow.
    void StartBufferedPaint();

    /// Paint the content of the buffer to the current m_pWindow.
    void PaintBuffer();

    ~PaintHelper();
};

PaintHelper::PaintHelper(vcl::Window *pWindow, sal_uInt16 nPaintFlags)
    : m_pWindow(pWindow)
    , m_pChildRegion(nullptr)
    , m_nPaintFlags(nPaintFlags)
    , m_bPop(false)
    , m_bRestoreCursor(false)
    , m_bStartedBufferedPaint(false)
{
}

void PaintHelper::StartBufferedPaint()
{
    ImplFrameData* pFrameData = m_pWindow->mpWindowImpl->mpFrameData;
    assert(!pFrameData->mbInBufferedPaint);

    pFrameData->mbInBufferedPaint = true;
    pFrameData->maBufferedRect = Rectangle();
    m_bStartedBufferedPaint = true;
}

void PaintHelper::PaintBuffer()
{
    ImplFrameData* pFrameData = m_pWindow->mpWindowImpl->mpFrameData;
    assert(pFrameData->mbInBufferedPaint);
    assert(m_bStartedBufferedPaint);

    PaintBufferGuard aGuard(pFrameData, m_pWindow);
    aGuard.SetPaintRect(pFrameData->maBufferedRect);
}

void PaintHelper::DoPaint(const vcl::Region* pRegion)
{
    WindowImpl* pWindowImpl = m_pWindow->ImplGetWindowImpl();

    vcl::Region* pWinChildClipRegion = m_pWindow->ImplGetWinChildClipRegion();
    ImplFrameData* pFrameData = m_pWindow->mpWindowImpl->mpFrameData;
    if (pWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL || pFrameData->mbInBufferedPaint)
    {
        pWindowImpl->maInvalidateRegion = *pWinChildClipRegion;
    }
    else
    {
        if (pRegion)
            pWindowImpl->maInvalidateRegion.Union( *pRegion );

        if (pWindowImpl->mpWinData && pWindowImpl->mbTrackVisible)
            /* #98602# need to repaint all children within the
           * tracking rectangle, so the following invert
           * operation takes places without traces of the previous
           * one.
           */
           pWindowImpl->maInvalidateRegion.Union(*pWindowImpl->mpWinData->mpTrackRect);

        if (pWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN)
            m_pChildRegion = new vcl::Region(pWindowImpl->maInvalidateRegion);
        pWindowImpl->maInvalidateRegion.Intersect(*pWinChildClipRegion);
    }
    pWindowImpl->mnPaintFlags = 0;
    if (!pWindowImpl->maInvalidateRegion.IsEmpty())
    {
#if HAVE_FEATURE_OPENGL
        VCL_GL_INFO("PaintHelper::DoPaint on " <<
                    typeid( *m_pWindow ).name() << " '" << m_pWindow->GetText() << "' begin");
#endif
        // double-buffering: setup the buffer if it does not exist
        if (!pFrameData->mbInBufferedPaint && m_pWindow->SupportsDoubleBuffering())
            StartBufferedPaint();

        // double-buffering: if this window does not support double-buffering,
        // but we are in the middle of double-buffered paint, we might be
        // losing information
        if (pFrameData->mbInBufferedPaint && !m_pWindow->SupportsDoubleBuffering())
            SAL_WARN("vcl.window", "non-double buffered window in the double-buffered hierarchy, painting directly: " << typeid(*m_pWindow.get()).name());

        if (pFrameData->mbInBufferedPaint && m_pWindow->SupportsDoubleBuffering())
        {
            // double-buffering
            PaintBufferGuard g(pFrameData, m_pWindow);
            m_pWindow->ApplySettings(*pFrameData->mpBuffer.get());

            m_pWindow->PushPaintHelper(this, *pFrameData->mpBuffer.get());
            m_pWindow->Paint(*pFrameData->mpBuffer.get(), m_aPaintRect);
            pFrameData->maBufferedRect.Union(m_aPaintRect);
        }
        else
        {
            // direct painting
            m_pWindow->ApplySettings(*m_pWindow);
            m_pWindow->PushPaintHelper(this, *m_pWindow);
            m_pWindow->Paint(*m_pWindow, m_aPaintRect);
        }
#if HAVE_FEATURE_OPENGL
        VCL_GL_INFO("PaintHelper::DoPaint end on " <<
                    typeid( *m_pWindow ).name() << " '" << m_pWindow->GetText() << "'");
#endif
    }
}

namespace vcl
{

void RenderTools::DrawSelectionBackground(vcl::RenderContext& rRenderContext, vcl::Window& rWindow,
                                          const Rectangle& rRect, sal_uInt16 nHighlight,
                                          bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly,
                                          Color* pSelectionTextColor, long nCornerRadius, Color* pPaintColor)
{
    if (rRect.IsEmpty())
        return;

    bool bRoundEdges = nCornerRadius > 0;

    const StyleSettings& rStyles = rRenderContext.GetSettings().GetStyleSettings();

    // colors used for item highlighting
    Color aSelectionBorderColor(pPaintColor ? *pPaintColor : rStyles.GetHighlightColor());
    Color aSelectionFillColor(aSelectionBorderColor);

    bool bDark = rStyles.GetFaceColor().IsDark();
    bool bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderColor.GetLuminance();
    int c2 = rWindow.GetDisplayBackground().GetColor().GetLuminance();

    if (!bDark && !bBright && std::abs(c2 - c1) < (pPaintColor ? 40 : 75))
    {
        // constrast too low
        sal_uInt16 h, s, b;
        aSelectionFillColor.RGBtoHSB( h, s, b );
        if( b > 50 )    b -= 40;
        else            b += 40;
        aSelectionFillColor.SetColor( Color::HSBtoRGB( h, s, b ) );
        aSelectionBorderColor = aSelectionFillColor;
    }

    if (bRoundEdges)
    {
        if (aSelectionBorderColor.IsDark())
            aSelectionBorderColor.IncreaseLuminance(128);
        else
            aSelectionBorderColor.DecreaseLuminance(128);
    }

    Rectangle aRect(rRect);
    if (bDrawExtBorderOnly)
    {
        aRect.Left()   -= 1;
        aRect.Top()    -= 1;
        aRect.Right()  += 1;
        aRect.Bottom() += 1;
    }
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

    if (bDrawBorder)
        rRenderContext.SetLineColor(bDark ? Color(COL_WHITE) : (bBright ? Color(COL_BLACK) : aSelectionBorderColor));
    else
        rRenderContext.SetLineColor();

    sal_uInt16 nPercent = 0;
    if (!nHighlight)
    {
        if (bDark)
            aSelectionFillColor = COL_BLACK;
        else
            nPercent = 80;  // just checked (light)
    }
    else
    {
        if (bChecked && nHighlight == 2)
        {
            if (bDark)
                aSelectionFillColor = COL_LIGHTGRAY;
            else if (bBright)
            {
                aSelectionFillColor = COL_BLACK;
                rRenderContext.SetLineColor(COL_BLACK);
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 40 : 20; // selected, pressed or checked ( very dark )
        }
        else if (bChecked || nHighlight == 1)
        {
            if (bDark)
                aSelectionFillColor = COL_GRAY;
            else if (bBright)
            {
                aSelectionFillColor = COL_BLACK;
                rRenderContext.SetLineColor(COL_BLACK);
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 60 : 35; // selected, pressed or checked ( very dark )
        }
        else
        {
            if (bDark)
                aSelectionFillColor = COL_LIGHTGRAY;
            else if (bBright)
            {
                aSelectionFillColor = COL_BLACK;
                rRenderContext.SetLineColor(COL_BLACK);
                if (nHighlight == 3)
                    nPercent = 80;
                else
                    nPercent = 0;
            }
            else
                nPercent = 70; // selected ( dark )
        }
    }

    if (bDark && bDrawExtBorderOnly)
    {
        rRenderContext.SetFillColor();
        if (pSelectionTextColor)
            *pSelectionTextColor = rStyles.GetHighlightTextColor();
    }
    else
    {
        rRenderContext.SetFillColor(aSelectionFillColor);
        if (pSelectionTextColor)
        {
            Color aTextColor = rWindow.IsControlBackground() ? rWindow.GetControlForeground() : rStyles.GetButtonTextColor();
            Color aHLTextColor = rStyles.GetHighlightTextColor();
            int nTextDiff = std::abs(aSelectionFillColor.GetLuminance() - aTextColor.GetLuminance());
            int nHLDiff = std::abs(aSelectionFillColor.GetLuminance() - aHLTextColor.GetLuminance());
            *pSelectionTextColor = (nHLDiff >= nTextDiff) ? aHLTextColor : aTextColor;
        }
    }

    if (bDark)
    {
        rRenderContext.DrawRect(aRect);
    }
    else
    {
        if (bRoundEdges)
        {
            tools::Polygon aPoly(aRect, nCornerRadius, nCornerRadius);
            tools::PolyPolygon aPolyPoly(aPoly);
            rRenderContext.DrawTransparent(aPolyPoly, nPercent);
        }
        else
        {
            tools::Polygon aPoly(aRect);
            tools::PolyPolygon aPolyPoly(aPoly);
            rRenderContext.DrawTransparent(aPolyPoly, nPercent);
        }
    }

    rRenderContext.Pop(); // LINECOLOR | FILLCOLOR
}

void Window::PushPaintHelper(PaintHelper *pHelper, vcl::RenderContext& rRenderContext)
{
    pHelper->SetPop();

    if ( mpWindowImpl->mpCursor )
        pHelper->SetRestoreCursor(mpWindowImpl->mpCursor->ImplSuspend());

    mbInitClipRegion = true;
    mpWindowImpl->mbInPaint = true;

    // restore Paint-Region
    vcl::Region &rPaintRegion = pHelper->GetPaintRegion();
    rPaintRegion = mpWindowImpl->maInvalidateRegion;
    Rectangle aPaintRect = rPaintRegion.GetBoundRect();

    // - RTL - re-mirror paint rect and region at this window
    if (ImplIsAntiparallel())
    {
        rRenderContext.ReMirror(aPaintRect);
        rRenderContext.ReMirror(rPaintRegion);
    }
    aPaintRect = ImplDevicePixelToLogic(aPaintRect);
    mpWindowImpl->mpPaintRegion = &rPaintRegion;
    mpWindowImpl->maInvalidateRegion.SetEmpty();

    if ((pHelper->GetPaintFlags() & IMPL_PAINT_ERASE) && rRenderContext.IsBackground())
    {
        if (rRenderContext.IsClipRegion())
        {
            vcl::Region aOldRegion = rRenderContext.GetClipRegion();
            rRenderContext.SetClipRegion();
            Erase(rRenderContext);
            rRenderContext.SetClipRegion(aOldRegion);
        }
        else
            Erase(rRenderContext);
    }

    // #98943# trigger drawing of toolbox selection after all children are painted
    if (mpWindowImpl->mbDrawSelectionBackground)
        pHelper->SetSelectionRect(aPaintRect);
    pHelper->SetPaintRect(aPaintRect);
}

void Window::PopPaintHelper(PaintHelper *pHelper)
{
    if (mpWindowImpl->mpWinData)
    {
        if (mpWindowImpl->mbFocusVisible)
            ImplInvertFocus(*(mpWindowImpl->mpWinData->mpFocusRect));
    }
    mpWindowImpl->mbInPaint = false;
    mbInitClipRegion = true;
    mpWindowImpl->mpPaintRegion = nullptr;
    if (mpWindowImpl->mpCursor)
        mpWindowImpl->mpCursor->ImplResume(pHelper->GetRestoreCursor());
}

} /* namespace vcl */

PaintHelper::~PaintHelper()
{
    WindowImpl* pWindowImpl = m_pWindow->ImplGetWindowImpl();
    if (m_bPop)
    {
        m_pWindow->PopPaintHelper(this);
    }

    ImplFrameData* pFrameData = m_pWindow->mpWindowImpl->mpFrameData;
    if ( m_nPaintFlags & (IMPL_PAINT_PAINTALLCHILDREN | IMPL_PAINT_PAINTCHILDREN) )
    {
        // Paint from the bottom child window and frontward.
        vcl::Window* pTempWindow = pWindowImpl->mpLastChild;
        while (pTempWindow)
        {
            if (pTempWindow->mpWindowImpl->mbVisible)
                pTempWindow->ImplCallPaint(m_pChildRegion, m_nPaintFlags);
            pTempWindow = pTempWindow->mpWindowImpl->mpPrev;
        }
    }

    if ( pWindowImpl->mpWinData && pWindowImpl->mbTrackVisible && (pWindowImpl->mpWinData->mnTrackFlags & ShowTrackFlags::TrackWindow) )
        /* #98602# need to invert the tracking rect AFTER
        * the children have painted
        */
        m_pWindow->InvertTracking( *(pWindowImpl->mpWinData->mpTrackRect), pWindowImpl->mpWinData->mnTrackFlags );

    // double-buffering: paint in case we created the buffer, the children are
    // already painted inside
    if (m_bStartedBufferedPaint && pFrameData->mbInBufferedPaint)
    {
        PaintBuffer();
        pFrameData->mbInBufferedPaint = false;
        pFrameData->maBufferedRect = Rectangle();
    }

    // #98943# draw toolbox selection
    if( !m_aSelectionRect.IsEmpty() )
        m_pWindow->DrawSelectionBackground( m_aSelectionRect, 3, false, true );

    delete m_pChildRegion;
}

namespace vcl {

void Window::ImplCallPaint(const vcl::Region* pRegion, sal_uInt16 nPaintFlags)
{
    // call PrePaint. PrePaint may add to the invalidate region as well as
    // other parameters used below.
    PrePaint(*this);

    mpWindowImpl->mbPaintFrame = false;

    if (nPaintFlags & IMPL_PAINT_PAINTALLCHILDREN)
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDREN | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if (nPaintFlags & IMPL_PAINT_PAINTCHILDREN)
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN;
    if (nPaintFlags & IMPL_PAINT_ERASE)
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if (nPaintFlags & IMPL_PAINT_CHECKRTL)
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;
    if (!mpWindowImpl->mpFirstChild)
        mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDREN;

    if (mpWindowImpl->mbPaintDisabled)
    {
        if (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL)
            Invalidate(InvalidateFlags::NoChildren | InvalidateFlags::NoErase | InvalidateFlags::NoTransparent | InvalidateFlags::NoClipChildren);
        else if ( pRegion )
            Invalidate(*pRegion, InvalidateFlags::NoChildren | InvalidateFlags::NoErase | InvalidateFlags::NoTransparent | InvalidateFlags::NoClipChildren);

        // call PostPaint before returning
        PostPaint(*this);

        return;
    }

    nPaintFlags = mpWindowImpl->mnPaintFlags & ~(IMPL_PAINT_PAINT);

    PaintHelper aHelper(this, nPaintFlags);

    if (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT)
        aHelper.DoPaint(pRegion);
    else
        mpWindowImpl->mnPaintFlags = 0;

    // call PostPaint
    PostPaint(*this);
}

void Window::ImplCallOverlapPaint()
{
    // emit overlapping windows first
    vcl::Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mpWindowImpl->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // only then ourself
    if ( mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        // - RTL - notify ImplCallPaint to check for re-mirroring (CHECKRTL)
        //         because we were called from the Sal layer
        ImplCallPaint(nullptr, mpWindowImpl->mnPaintFlags /*| IMPL_PAINT_CHECKRTL */);
    }
}

void Window::ImplPostPaint()
{
    if ( !mpWindowImpl->mpFrameData->maPaintIdle.IsActive() )
        mpWindowImpl->mpFrameData->maPaintIdle.Start();
}

IMPL_LINK_NOARG_TYPED(Window, ImplHandlePaintHdl, Idle *, void)
{
    // save paint events until layout is done
    if (IsSystemWindow() && static_cast<const SystemWindow*>(this)->hasPendingLayout())
    {
        mpWindowImpl->mpFrameData->maPaintIdle.Start();
        return;
    }

    // save paint events until resizing or initial sizing done
    if (mpWindowImpl->mbFrame &&
        (mpWindowImpl->mpFrameData->maResizeIdle.IsActive() ||
         mpWindowImpl->mpFrame->PaintsBlocked()))
    {
        mpWindowImpl->mpFrameData->maPaintIdle.Start();
    }
    else if ( mpWindowImpl->mbReallyVisible )
    {
        ImplCallOverlapPaint();
    }
}

IMPL_LINK_NOARG_TYPED(Window, ImplHandleResizeTimerHdl, Idle *, void)
{
    if( mpWindowImpl->mbReallyVisible )
    {
        ImplCallResize();
        if( mpWindowImpl->mpFrameData->maPaintIdle.IsActive() )
        {
            mpWindowImpl->mpFrameData->maPaintIdle.Stop();
            mpWindowImpl->mpFrameData->maPaintIdle.Invoke( nullptr );
        }
    }
}

void Window::ImplInvalidateFrameRegion( const vcl::Region* pRegion, InvalidateFlags nFlags )
{
    // set PAINTCHILDREN for all parent windows till the first OverlapWindow
    if ( !ImplIsOverlapWindow() )
    {
        vcl::Window* pTempWindow = this;
        sal_uInt16 nTranspPaint = IsPaintTransparent() ? IMPL_PAINT_PAINT : 0;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN )
                break;
            pTempWindow->mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN | nTranspPaint;
            if( ! pTempWindow->IsPaintTransparent() )
                nTranspPaint = 0;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // set Paint-Flags
    mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & InvalidateFlags::Children )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDREN;
    if ( !(nFlags & InvalidateFlags::NoErase) )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // if not everything has to be redrawn, add the region to it
    if ( !(mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL) )
        mpWindowImpl->maInvalidateRegion.Union( *pRegion );

    // Handle transparent windows correctly: invalidate must be done on the first opaque parent
    if( ((IsPaintTransparent() && !(nFlags & InvalidateFlags::NoTransparent)) || (nFlags & InvalidateFlags::Transparent) )
            && ImplGetParent() )
    {
        vcl::Window *pParent = ImplGetParent();
        while( pParent && pParent->IsPaintTransparent() )
            pParent = pParent->ImplGetParent();
        if( pParent )
        {
            vcl::Region *pChildRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                // invalidate the whole child window region in the parent
                pChildRegion = ImplGetWinChildClipRegion();
            else
                // invalidate the same region in the parent that has to be repainted in the child
                pChildRegion = &mpWindowImpl->maInvalidateRegion;

            nFlags |= InvalidateFlags::Children;  // paint should also be done on all children
            nFlags &= ~InvalidateFlags::NoErase;  // parent should paint and erase to create proper background
            pParent->ImplInvalidateFrameRegion( pChildRegion, nFlags );
        }
    }
    ImplPostPaint();
}

void Window::ImplInvalidateOverlapFrameRegion( const vcl::Region& rRegion )
{
    vcl::Region aRegion = rRegion;

    ImplClipBoundaries( aRegion, true, true );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, InvalidateFlags::Children );

    // now we invalidate the overlapping windows
    vcl::Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplInvalidateParentFrameRegion( vcl::Region& rRegion )
{
    if ( mpWindowImpl->mbOverlapWin )
        mpWindowImpl->mpFrameWindow->ImplInvalidateOverlapFrameRegion( rRegion );
    else
    {
        if( ImplGetParent() )
            ImplGetParent()->ImplInvalidateFrameRegion( &rRegion, InvalidateFlags::Children );
    }
}

void Window::ImplInvalidate( const vcl::Region* pRegion, InvalidateFlags nFlags )
{
    // check what has to be redrawn
    bool bInvalidateAll = !pRegion;

    // take Transparent-Invalidate into account
    vcl::Window* pOpaqueWindow = this;
    if ( (mpWindowImpl->mbPaintTransparent && !(nFlags & InvalidateFlags::NoTransparent)) || (nFlags & InvalidateFlags::Transparent) )
    {
        vcl::Window* pTempWindow = pOpaqueWindow->ImplGetParent();
        while ( pTempWindow )
        {
            if ( !pTempWindow->IsPaintTransparent() )
            {
                pOpaqueWindow = pTempWindow;
                nFlags |= InvalidateFlags::Children;
                bInvalidateAll = false;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    // assemble region
    InvalidateFlags nOrgFlags = nFlags;
    if ( !(nFlags & (InvalidateFlags::Children | InvalidateFlags::NoChildren)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= InvalidateFlags::NoChildren;
        else
            nFlags |= InvalidateFlags::Children;
    }
    if ( (nFlags & InvalidateFlags::NoChildren) && mpWindowImpl->mpFirstChild )
        bInvalidateAll = false;
    if ( bInvalidateAll )
        ImplInvalidateFrameRegion( nullptr, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        vcl::Region      aRegion( aRect );
        if ( pRegion )
        {
            // --- RTL --- remirror region before intersecting it
            if ( ImplIsAntiparallel() )
            {
                const OutputDevice *pOutDev = GetOutDev();

                vcl::Region aRgn( *pRegion );
                pOutDev->ReMirror( aRgn );
                aRegion.Intersect( aRgn );
            }
            else
                aRegion.Intersect( *pRegion );
        }
        ImplClipBoundaries( aRegion, true, true );
        if ( nFlags & InvalidateFlags::NoChildren )
        {
            nFlags &= ~InvalidateFlags::Children;
            if ( !(nFlags & InvalidateFlags::NoClipChildren) )
            {
                if ( nOrgFlags & InvalidateFlags::NoChildren )
                    ImplClipAllChildren( aRegion );
                else
                {
                    if ( ImplClipChildren( aRegion ) )
                        nFlags |= InvalidateFlags::Children;
                }
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplInvalidateFrameRegion( &aRegion, nFlags );  // transparency is handled here, pOpaqueWindow not required
    }

    if ( nFlags & InvalidateFlags::Update )
        pOpaqueWindow->Update();        // start painting at the opaque parent
}

void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       bool bChildren )
{
    if ( (mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        vcl::Region aTempRegion = mpWindowImpl->maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        mpWindowImpl->maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChildren && (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN) )
    {
        vcl::Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, true );
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           bool bChildren )
{
    // also shift Paint-Region when paints need processing
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChildren );
    // Paint-Region should be shifted, as drawn by the parents
    if ( !ImplIsOverlapWindow() )
    {
        vcl::Region  aPaintAllRegion;
        vcl::Window* pPaintAllWindow = this;
        do
        {
            pPaintAllWindow = pPaintAllWindow->ImplGetParent();
            if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            {
                if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                {
                    aPaintAllRegion.SetEmpty();
                    break;
                }
                else
                    aPaintAllRegion.Union( pPaintAllWindow->mpWindowImpl->maInvalidateRegion );
            }
        }
        while ( !pPaintAllWindow->ImplIsOverlapWindow() );
        if ( !aPaintAllRegion.IsEmpty() )
        {
            aPaintAllRegion.Move( nHorzScroll, nVertScroll );
            InvalidateFlags nPaintFlags = InvalidateFlags::NONE;
            if ( bChildren )
                nPaintFlags |= InvalidateFlags::Children;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

void Window::ImplValidateFrameRegion( const vcl::Region* pRegion, ValidateFlags nFlags )
{
    if ( !pRegion )
        mpWindowImpl->maInvalidateRegion.SetEmpty();
    else
    {
        // when all child windows have to be drawn we need to invalidate them before doing so
        if ( (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN) && mpWindowImpl->mpFirstChild )
        {
            vcl::Region aChildRegion = mpWindowImpl->maInvalidateRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            {
                Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                aChildRegion = aRect;
            }
            vcl::Window* pChild = mpWindowImpl->mpFirstChild;
            while ( pChild )
            {
                pChild->Invalidate( aChildRegion, InvalidateFlags::Children | InvalidateFlags::NoTransparent );
                pChild = pChild->mpWindowImpl->mpNext;
            }
        }
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
        {
            Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            mpWindowImpl->maInvalidateRegion = aRect;
        }
        mpWindowImpl->maInvalidateRegion.Exclude( *pRegion );
    }
    mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALL;

    if ( nFlags & ValidateFlags::Children )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplValidateFrameRegion( pRegion, nFlags );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplValidate()
{
    // assemble region
    bool    bValidateAll = true;
    ValidateFlags nFlags = ValidateFlags::NONE;
    if ( GetStyle() & WB_CLIPCHILDREN )
        nFlags |= ValidateFlags::NoChildren;
    else
        nFlags |= ValidateFlags::Children;
    if ( (nFlags & ValidateFlags::NoChildren) && mpWindowImpl->mpFirstChild )
        bValidateAll = false;
    if ( bValidateAll )
        ImplValidateFrameRegion( nullptr, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        vcl::Region      aRegion( aRect );
        ImplClipBoundaries( aRegion, true, true );
        if ( nFlags & ValidateFlags::NoChildren )
        {
            nFlags &= ~ValidateFlags::Children;
            if ( ImplClipChildren( aRegion ) )
                nFlags |= ValidateFlags::Children;
        }
        if ( !aRegion.IsEmpty() )
            ImplValidateFrameRegion( &aRegion, nFlags );
    }
}

void Window::ImplUpdateAll()
{
    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        vcl::Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    // an update changes the OverlapWindow, such that for later paints
    // not too much has to be drawn, if ALLCHILDREN etc. is set
    vcl::Window* pWindow = ImplGetFirstOverlapWindow();
    pWindow->ImplCallOverlapPaint();

    if ( bFlush )
        Flush();
}

void Window::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
}

void Window::PostPaint(vcl::RenderContext& /*rRenderContext*/)
{
}

void Window::Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect)
{
    CallEventListeners(VCLEVENT_WINDOW_PAINT, const_cast<Rectangle *>(&rRect));
}

void Window::SetPaintTransparent( bool bTransparent )
{
    // transparency is not useful for frames as the background would have to be provided by a different frame
    if( bTransparent && mpWindowImpl->mbFrame )
        return;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetPaintTransparent( bTransparent );

    mpWindowImpl->mbPaintTransparent = bTransparent;
}

void Window::SetWindowRegionPixel()
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel();
    else if( mpWindowImpl->mbFrame )
    {
        mpWindowImpl->maWinRegion = vcl::Region(true);
        mpWindowImpl->mbWinRegion = false;
        mpWindowImpl->mpFrame->ResetClipRegion();
    }
    else
    {
        if ( mpWindowImpl->mbWinRegion )
        {
            mpWindowImpl->maWinRegion = vcl::Region(true);
            mpWindowImpl->mbWinRegion = false;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                vcl::Region      aRegion( aRect );
                ImplInvalidateParentFrameRegion( aRegion );
            }
        }
    }
}

void Window::SetWindowRegionPixel( const vcl::Region& rRegion )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel( rRegion );
    else if( mpWindowImpl->mbFrame )
    {
        if( !rRegion.IsNull() )
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = ! rRegion.IsEmpty();

            if( mpWindowImpl->mbWinRegion )
            {
                // set/update ClipRegion
                RectangleVector aRectangles;
                mpWindowImpl->maWinRegion.GetRegionRectangles(aRectangles);
                mpWindowImpl->mpFrame->BeginSetClipRegion(aRectangles.size());

                for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                {
                    mpWindowImpl->mpFrame->UnionClipRegion(
                        aRectIter->Left(),
                        aRectIter->Top(),
                        aRectIter->GetWidth(),       // orig nWidth was ((R - L) + 1), same as GetWidth does
                        aRectIter->GetHeight());     // same for height
                }

                mpWindowImpl->mpFrame->EndSetClipRegion();

                //long                nX;
                //long                nY;
                //long                nWidth;
                //long                nHeight;
                //sal_uLong               nRectCount;
                //ImplRegionInfo      aInfo;
                //sal_Bool                bRegionRect;

                //nRectCount = mpWindowImpl->maWinRegion.GetRectCount();
                //mpWindowImpl->mpFrame->BeginSetClipRegion( nRectCount );
                //bRegionRect = mpWindowImpl->maWinRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                //while ( bRegionRect )
                //{
                //    mpWindowImpl->mpFrame->UnionClipRegion( nX, nY, nWidth, nHeight );
                //    bRegionRect = mpWindowImpl->maWinRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                //}
                //mpWindowImpl->mpFrame->EndSetClipRegion();
            }
            else
                SetWindowRegionPixel();
        }
        else
            SetWindowRegionPixel();
    }
    else
    {
        if ( rRegion.IsNull() )
        {
            if ( mpWindowImpl->mbWinRegion )
            {
                mpWindowImpl->maWinRegion = vcl::Region(true);
                mpWindowImpl->mbWinRegion = false;
                ImplSetClipFlag();
            }
        }
        else
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = true;
            ImplSetClipFlag();
        }

        if ( IsReallyVisible() )
        {
            Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            vcl::Region      aRegion( aRect );
            ImplInvalidateParentFrameRegion( aRegion );
        }
    }
}

const vcl::Region& Window::GetWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetWindowRegionPixel();
    else
        return mpWindowImpl->maWinRegion;
}

bool Window::IsWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->IsWindowRegionPixel();
    else
        return mpWindowImpl->mbWinRegion;
}

vcl::Region Window::GetPaintRegion() const
{

    if ( mpWindowImpl->mpPaintRegion )
    {
        vcl::Region aRegion = *mpWindowImpl->mpPaintRegion;
        aRegion.Move( -mnOutOffX, -mnOutOffY );
        return PixelToLogic( aRegion );
    }
    else
    {
        vcl::Region aPaintRegion(true);
        return aPaintRegion;
    }
}

void Window::Invalidate( InvalidateFlags nFlags )
{
    if ( !comphelper::LibreOfficeKit::isActive() && (!IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight) )
        return;

    ImplInvalidate( nullptr, nFlags );
    LogicInvalidate(nullptr);
}

void Window::Invalidate( const Rectangle& rRect, InvalidateFlags nFlags )
{
    if ( !comphelper::LibreOfficeKit::isActive() && (!IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight) )
        return;

    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        vcl::Region aRegion( aRect );
        ImplInvalidate( &aRegion, nFlags );
        Rectangle aLogicRectangle(rRect);
        LogicInvalidate(&aLogicRectangle);
    }
}

void Window::Invalidate( const vcl::Region& rRegion, InvalidateFlags nFlags )
{
    if ( !comphelper::LibreOfficeKit::isActive() && (!IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight) )
        return;

    if ( rRegion.IsNull() )
    {
        ImplInvalidate( nullptr, nFlags );
        LogicInvalidate(nullptr);
    }
    else
    {
        vcl::Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
        {
            ImplInvalidate( &aRegion, nFlags );
            Rectangle aLogicRectangle = rRegion.GetBoundRect();
            LogicInvalidate(&aLogicRectangle);
        }
    }
}

void Window::Validate()
{
    if ( !comphelper::LibreOfficeKit::isActive() && (!IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight) )
        return;

    ImplValidate();
}

bool Window::HasPaintEvent() const
{

    if ( !mpWindowImpl->mbReallyVisible )
        return false;

    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
        return true;

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        return true;

    if ( !ImplIsOverlapWindow() )
    {
        const vcl::Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINTCHILDREN | IMPL_PAINT_PAINTALLCHILDREN) )
                return true;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return false;
}

void Window::Update()
{
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Update();
        return;
    }

    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        vcl::Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    // First we should skip all windows which are Paint-Transparent
    vcl::Window* pUpdateWindow = this;
    vcl::Window* pWindow = pUpdateWindow;
    while ( !pWindow->ImplIsOverlapWindow() )
    {
        if ( !pWindow->mpWindowImpl->mbPaintTransparent )
        {
            pUpdateWindow = pWindow;
            break;
        }
        pWindow = pWindow->ImplGetParent();
    }
    // In order to limit drawing, an update only draws the window which
    // has PAINTALLCHILDREN set
    pWindow = pUpdateWindow;
    do
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // if there is something to paint, trigger a Paint
    if ( pUpdateWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        VclPtr<vcl::Window> xWindow(this);

        // trigger an update also for system windows on top of us,
        // otherwise holes would remain
         vcl::Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpWindowImpl->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpWindowImpl->mpNext;
         }

        pUpdateWindow->ImplCallPaint(nullptr, pUpdateWindow->mpWindowImpl->mnPaintFlags);

        if (xWindow->IsDisposed())
           return;
        bFlush = true;
    }

    if ( bFlush )
        Flush();
}

void Window::ImplPaintToDevice( OutputDevice* i_pTargetOutDev, const Point& i_rPos )
{
    bool bRVisible = mpWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyVisible = mpWindowImpl->mbVisible;
    bool bDevOutput = mbDevOutput;
    mbDevOutput = true;

    const OutputDevice *pOutDev = GetOutDev();
    long nOldDPIX = pOutDev->GetDPIX();
    long nOldDPIY = pOutDev->GetDPIY();
    mnDPIX = i_pTargetOutDev->GetDPIX();
    mnDPIY = i_pTargetOutDev->GetDPIY();
    bool bOutput = IsOutputEnabled();
    EnableOutput();

    SAL_WARN_IF( GetMapMode().GetMapUnit() != MAP_PIXEL, "vcl", "MapMode must be PIXEL based" );
    if ( GetMapMode().GetMapUnit() != MAP_PIXEL )
        return;

    // preserve graphicsstate
    Push();
    vcl::Region aClipRegion( GetClipRegion() );
    SetClipRegion();

    GDIMetaFile* pOldMtf = GetConnectMetaFile();
    GDIMetaFile aMtf;
    SetConnectMetaFile( &aMtf );

    // put a push action to metafile
    Push();
    // copy graphics state to metafile
    vcl::Font aCopyFont = GetFont();
    if( nOldDPIX != mnDPIX || nOldDPIY != mnDPIY )
    {
        aCopyFont.SetFontHeight( aCopyFont.GetFontHeight() * mnDPIY / nOldDPIY );
        aCopyFont.SetAverageFontWidth( aCopyFont.GetAverageFontWidth() * mnDPIX / nOldDPIX );
    }
    SetFont( aCopyFont );
    SetTextColor( GetTextColor() );
    if( IsLineColor() )
        SetLineColor( GetLineColor() );
    else
        SetLineColor();
    if( IsFillColor() )
        SetFillColor( GetFillColor() );
    else
        SetFillColor();
    if( IsTextLineColor() )
        SetTextLineColor( GetTextLineColor() );
    else
        SetTextLineColor();
    if( IsOverlineColor() )
        SetOverlineColor( GetOverlineColor() );
    else
        SetOverlineColor();
    if( IsTextFillColor() )
        SetTextFillColor( GetTextFillColor() );
    else
        SetTextFillColor();
    SetTextAlign( GetTextAlign() );
    SetRasterOp( GetRasterOp() );
    if( IsRefPoint() )
        SetRefPoint( GetRefPoint() );
    else
        SetRefPoint();
    SetLayoutMode( GetLayoutMode() );
    SetDigitLanguage( GetDigitLanguage() );
    Rectangle aPaintRect( Point( 0, 0 ), GetOutputSizePixel() );
    aClipRegion.Intersect( aPaintRect );
    SetClipRegion( aClipRegion );

    // do the actual paint

    // background
    if( ! IsPaintTransparent() && IsBackground() && ! (GetParentClipMode() & ParentClipMode::NoClip ) )
        Erase(*this);
    // foreground
    Paint(*this, aPaintRect);
    // put a pop action to metafile
    Pop();

    SetConnectMetaFile( pOldMtf );
    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;

    // paint metafile to VDev
    VclPtrInstance<VirtualDevice> pMaskedDevice(*i_pTargetOutDev,
                                                DeviceFormat::DEFAULT,
                                                DeviceFormat::DEFAULT);
    pMaskedDevice->SetOutputSizePixel( GetOutputSizePixel() );
    pMaskedDevice->EnableRTL( IsRTLEnabled() );
    aMtf.WindStart();
    aMtf.Play( pMaskedDevice );
    BitmapEx aBmpEx( pMaskedDevice->GetBitmapEx( Point( 0, 0 ), pMaskedDevice->GetOutputSizePixel() ) );
    i_pTargetOutDev->DrawBitmapEx( i_rPos, aBmpEx );
    // get rid of virtual device now so they don't pile up during recursive calls
    pMaskedDevice.disposeAndClear();

    for( vcl::Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext )
    {
        if( pChild->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame && pChild->IsVisible() )
        {
            long nDeltaX = pChild->mnOutOffX - mnOutOffX;

            if( pOutDev->HasMirroredGraphics() )
                nDeltaX = mnOutWidth - nDeltaX - pChild->mnOutWidth;
            long nDeltaY = pChild->GetOutOffYPixel() - GetOutOffYPixel();
            Point aPos( i_rPos );
            Point aDelta( nDeltaX, nDeltaY );
            aPos += aDelta;
            pChild->ImplPaintToDevice( i_pTargetOutDev, aPos );
        }
    }

    // restore graphics state
    Pop();

    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;
    mbDevOutput = bDevOutput;
    mnDPIX = nOldDPIX;
    mnDPIY = nOldDPIY;
}

void Window::PaintToDevice( OutputDevice* pDev, const Point& rPos, const Size& /*rSize*/ )
{
    // FIXME: scaling: currently this is for pixel copying only

    SAL_WARN_IF(  pDev->HasMirroredGraphics(), "vcl", "PaintToDevice to mirroring graphics" );
    SAL_WARN_IF(  pDev->IsRTLEnabled(), "vcl", "PaintToDevice to mirroring device" );

    vcl::Window* pRealParent = nullptr;
    if( ! mpWindowImpl->mbVisible )
    {
        vcl::Window* pTempParent = ImplGetDefaultWindow();
        pTempParent->EnableChildTransparentMode();
        pRealParent = GetParent();
        SetParent( pTempParent );
        // trigger correct visibility flags for children
        Show();
        Hide();
    }

    bool bVisible = mpWindowImpl->mbVisible;
    mpWindowImpl->mbVisible = true;

    if( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->ImplPaintToDevice( pDev, rPos );
    else
        ImplPaintToDevice( pDev, rPos );

    mpWindowImpl->mbVisible = bVisible;

    if( pRealParent )
        SetParent( pRealParent );
}

void Window::Erase(vcl::RenderContext& rRenderContext)
{
    if (!IsDeviceOutputNecessary() || ImplIsRecordLayout())
        return;

    bool bNativeOK = false;

    ControlPart aCtrlPart = ImplGetWindowImpl()->mnNativeBackground;
    if (aCtrlPart != ControlPart::NONE && ! IsControlBackground())
    {
        Rectangle aCtrlRegion(Point(), GetOutputSizePixel());
        ControlState nState = ControlState::NONE;

        if (IsEnabled())
            nState |= ControlState::ENABLED;

        bNativeOK = rRenderContext.DrawNativeControl(ControlType::WindowBackground, aCtrlPart, aCtrlRegion,
                                                     nState, ImplControlValue(), OUString());
    }

    if (mbBackground && !bNativeOK)
    {
        RasterOp eRasterOp = GetRasterOp();
        if (eRasterOp != RasterOp::OverPaint)
            SetRasterOp(RasterOp::OverPaint);
        rRenderContext.DrawWallpaper(0, 0, mnOutWidth, mnOutHeight, maBackground);
        if (eRasterOp != RasterOp::OverPaint)
            rRenderContext.SetRasterOp(eRasterOp);
    }

    if (mpAlphaVDev)
        mpAlphaVDev->Erase();
}

void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, ScrollFlags nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplSuspend();

    ScrollFlags nOrgFlags = nFlags;
    if ( !(nFlags & (ScrollFlags::Children | ScrollFlags::NoChildren)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= ScrollFlags::NoChildren;
        else
            nFlags |= ScrollFlags::Children;
    }

    vcl::Region  aInvalidateRegion;
    bool    bScrollChildren(nFlags & ScrollFlags::Children);

    if ( !mpWindowImpl->mpFirstChild )
        bScrollChildren = false;

    OutputDevice *pOutDev = GetOutDev();

    // --- RTL --- check if this window requires special action
    bool bReMirror = ( ImplIsAntiparallel() );

    Rectangle aRectMirror( rRect );
    if( bReMirror )
    {
        // --- RTL --- make sure the invalidate region of this window is
        // computed in the same coordinate space as the one from the overlap windows
        pOutDev->ReMirror( aRectMirror );
    }

    // adapt paint areas
    ImplMoveAllInvalidateRegions( aRectMirror, nHorzScroll, nVertScroll, bScrollChildren );

    ImplCalcOverlapRegion( aRectMirror, aInvalidateRegion, !bScrollChildren, false );

    // --- RTL ---
    // if the scrolling on the device is performed in the opposite direction
    // then move the overlaps in that direction to compute the invalidate region
    // on the correct side, i.e., revert nHorzScroll
    if (!aInvalidateRegion.IsEmpty())
    {
        aInvalidateRegion.Move(bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll);
    }

    Rectangle aDestRect(aRectMirror);
    aDestRect.Move(bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll);
    vcl::Region aWinInvalidateRegion(aRectMirror);
    if (!SupportsDoubleBuffering())
    {
        // There will be no CopyArea() call below, so invalidate the
        // whole visible area, not only the smaller one that was just
        // scrolled in.
        aWinInvalidateRegion.Exclude(aDestRect);
    }

    aInvalidateRegion.Union(aWinInvalidateRegion);

    Point aPoint( mnOutOffX, mnOutOffY );
    vcl::Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
    if ( nFlags & ScrollFlags::Clip )
        aRegion.Intersect( rRect );
    if ( mpWindowImpl->mbWinRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

    aRegion.Exclude( aInvalidateRegion );

    ImplClipBoundaries( aRegion, false, true );
    if ( !bScrollChildren )
    {
        if ( nOrgFlags & ScrollFlags::NoChildren )
            ImplClipAllChildren( aRegion );
        else
            ImplClipChildren( aRegion );
    }
    if ( mbClipRegion && (nFlags & ScrollFlags::UseClipRegion) )
        aRegion.Intersect( maRegion );
    if ( !aRegion.IsEmpty() )
    {
        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & ShowTrackFlags::TrackWindow) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }
#ifndef IOS
        // This seems completely unnecessary with tiled rendering, and
        // causes the "AquaSalGraphics::copyArea() for non-layered
        // graphics" message. Presumably we should bypass this on all
        // platforms when dealing with a "window" that uses tiled
        // rendering at the moment. Unclear how to figure that out,
        // though. Also unclear whether we actually could just not
        // create a "frame window", whatever that exactly is, in the
        // tiled rendering case, or at least for platforms where tiles
        // rendering is all there is.

        SalGraphics* pGraphics = ImplGetFrameGraphics();
        // The invalidation area contains the area what would be copied here,
        // so avoid copying in case of double buffering.
        if (pGraphics && !SupportsDoubleBuffering())
        {
            if( bReMirror )
            {
                // --- RTL --- frame coordinates require re-mirroring
                pOutDev->ReMirror( aRegion );
            }

            pOutDev->SelectClipRegion( aRegion, pGraphics );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 this );
        }
#endif
        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & ShowTrackFlags::TrackWindow) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }
    }

    if ( !aInvalidateRegion.IsEmpty() )
    {
        // --- RTL --- the invalidate region for this windows is already computed in frame coordinates
        // so it has to be re-mirrored before calling the Paint-handler
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;

        InvalidateFlags nPaintFlags = InvalidateFlags::Children;
        if ( !bScrollChildren )
        {
            if ( nOrgFlags & ScrollFlags::NoChildren )
                ImplClipAllChildren( aInvalidateRegion );
            else
                ImplClipChildren( aInvalidateRegion );
        }
        ImplInvalidateFrameRegion( &aInvalidateRegion, nPaintFlags );
    }

    if ( bScrollChildren )
    {
        vcl::Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            Point aPos = pWindow->GetPosPixel();
            aPos += Point( nHorzScroll, nVertScroll );
            pWindow->SetPosPixel( aPos );

            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }

    if ( nFlags & ScrollFlags::Update )
        Update();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplResume();
}

} /* namespace vcl */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
