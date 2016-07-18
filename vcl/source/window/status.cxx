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

#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/status.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLWrapper.hxx>
#endif
#include <svdata.hxx>
#include <window.h>

#define STATUSBAR_OFFSET_X      STATUSBAR_OFFSET
#define STATUSBAR_OFFSET_Y      2
#define STATUSBAR_OFFSET_TEXTY  3

#define STATUSBAR_PRGS_OFFSET   3
#define STATUSBAR_PRGS_COUNT    100
#define STATUSBAR_PRGS_MIN      5

class StatusBar::ImplData
{
public:
    ImplData();
    ~ImplData();

    VclPtr<VirtualDevice> mpVirDev;
    long                mnItemBorderWidth;
    bool                mbDrawItemFrames:1;
};

StatusBar::ImplData::ImplData()
{
    mpVirDev = nullptr;
    mbDrawItemFrames = false;
    mnItemBorderWidth = 0;
}

StatusBar::ImplData::~ImplData()
{
}

struct ImplStatusItem
{
    sal_uInt16              mnId;
    StatusBarItemBits   mnBits;
    long                mnWidth;
    long                mnOffset;
    long                mnExtraWidth;
    long                mnX;
    OUString            maText;
    OUString            maHelpText;
    OUString            maQuickHelpText;
    OString             maHelpId;
    void*               mpUserData;
    bool            mbVisible;
    OUString            maAccessibleName;
    OUString            maCommand;
};

inline long ImplCalcProgessWidth( sal_uInt16 nMax, long nSize )
{
    return ((nMax*(nSize+(nSize/2)))-(nSize/2)+(STATUSBAR_PRGS_OFFSET*2));
}

static Point ImplGetItemTextPos( const Size& rRectSize, const Size& rTextSize,
                                 StatusBarItemBits nStyle )
{
    long nX;
    long nY;
    long delta = (rTextSize.Height()/4) + 1;
    if( delta + rTextSize.Width() > rRectSize.Width() )
        delta = 0;

    if ( nStyle & StatusBarItemBits::Left )
        nX = delta;
    else if ( nStyle & StatusBarItemBits::Right )
        nX = rRectSize.Width()-rTextSize.Width()-delta;
    else // StatusBarItemBits::Center
        nX = (rRectSize.Width()-rTextSize.Width())/2;
    nY = (rRectSize.Height()-rTextSize.Height())/2 + 1;
    return Point( nX, nY );
}

bool StatusBar::ImplIsItemUpdate()
{
    if ( !mbProgressMode && mbVisibleItems && IsReallyVisible() && IsUpdateMode() )
        return true;
    else
        return false;
}

void StatusBar::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpImplData = new ImplData;

    // default: RightAlign
    if ( !(nStyle & (WB_LEFT | WB_RIGHT)) )
        nStyle |= WB_RIGHT;

    Window::ImplInit( pParent, nStyle & ~WB_BORDER, nullptr );

    // remember WinBits
    mpItemList      = new ImplStatusItemList;
    mpImplData->mpVirDev = VclPtr<VirtualDevice>::Create( *this );
    mnCurItemId     = 0;
    mbFormat        = true;
    mbVisibleItems  = true;
    mbProgressMode  = false;
    mbInUserDraw    = false;
    mbAdjustHiDPI   = false;
    mnItemsWidth    = STATUSBAR_OFFSET_X;
    mnDX            = 0;
    mnDY            = 0;
    mnCalcHeight    = 0;
    mnItemY         = STATUSBAR_OFFSET_Y;
    mnTextY         = STATUSBAR_OFFSET_TEXTY;

    ImplInitSettings();

    SetOutputSizePixel( CalcWindowSizePixel() );
}

StatusBar::StatusBar( vcl::Window* pParent, WinBits nStyle ) :
    Window( WINDOW_STATUSBAR )
{
    ImplInit( pParent, nStyle );
}

StatusBar::~StatusBar()
{
    disposeOnce();
}

void StatusBar::dispose()
{
    // delete all items
    for (ImplStatusItem* i : *mpItemList) {
        delete i;
    }
    delete mpItemList;

    // delete VirtualDevice
    mpImplData->mpVirDev.disposeAndClear();
    delete mpImplData;
    Window::dispose();
}

void StatusBar::AdjustItemWidthsForHiDPI()
{
    mbAdjustHiDPI = true;
}

void StatusBar::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetLineColor();

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetToolFont();
    if (IsControlFont())
        aFont.Merge(GetControlFont());
    SetZoomedPointFont(rRenderContext, aFont);

    Color aColor;
    if (IsControlForeground())
        aColor = GetControlForeground();
    else if (GetStyle() & WB_3DLOOK)
        aColor = rStyleSettings.GetButtonTextColor();
    else
        aColor = rStyleSettings.GetWindowTextColor();

    rRenderContext.SetTextColor(aColor);
    rRenderContext.SetTextFillColor();

    if (IsControlBackground())
        aColor = GetControlBackground();
    else if (GetStyle() & WB_3DLOOK)
        aColor = rStyleSettings.GetFaceColor();
    else
        aColor = rStyleSettings.GetWindowColor();
    rRenderContext.SetBackground(aColor);

    // NWF background
    if (!IsControlBackground() &&
          rRenderContext.IsNativeControlSupported(ControlType::WindowBackground, ControlPart::BackgroundWindow))
    {
        ImplGetWindowImpl()->mnNativeBackground = ControlPart::BackgroundWindow;
        EnableChildTransparentMode();
    }
}

void StatusBar::ImplInitSettings()
{
    ApplySettings(*this);

    mpImplData->mpVirDev->SetFont(GetFont());
    mpImplData->mpVirDev->SetTextColor(GetTextColor());
    mpImplData->mpVirDev->SetTextAlign(GetTextAlign());
    mpImplData->mpVirDev->SetTextFillColor();
    mpImplData->mpVirDev->SetBackground(GetBackground());
}

void StatusBar::ImplFormat()
{
    ImplStatusItem* pItem;
    long            nExtraWidth;
    long            nExtraWidth2;
    long            nX;
    sal_uInt16          nAutoSizeItems = 0;

    // sum up widths
    mnItemsWidth = STATUSBAR_OFFSET_X;
    long nOffset = 0;
    for (ImplStatusItem* i : *mpItemList) {
        pItem = i;
        if ( pItem->mbVisible )
        {
            if ( pItem->mnBits & StatusBarItemBits::AutoSize ) {
                nAutoSizeItems++;
            }

            mnItemsWidth += pItem->mnWidth + nOffset;
            nOffset = pItem->mnOffset;
        }
    }

    if ( GetStyle() & WB_RIGHT )
    {
        // AutoSize isn't computed for right-alignment,
        // because we show the text that is declared by SetText on the left side
        nX              = mnDX - mnItemsWidth;
        nExtraWidth     = 0;
        nExtraWidth2    = 0;
    }
    else
    {
        mnItemsWidth += STATUSBAR_OFFSET_X;

        // calling AutoSize is potentially necessary for left-aligned text,
        if ( nAutoSizeItems && (mnDX > (mnItemsWidth - STATUSBAR_OFFSET)) )
        {
            nExtraWidth  = (mnDX - mnItemsWidth - 1) / nAutoSizeItems;
            nExtraWidth2 = (mnDX - mnItemsWidth - 1) % nAutoSizeItems;
        }
        else
        {
            nExtraWidth  = 0;
            nExtraWidth2 = 0;
        }
        nX = STATUSBAR_OFFSET_X;

        if( HasMirroredGraphics() && IsRTLEnabled() )
            nX += ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset;
    }

    for (ImplStatusItem* i : *mpItemList) {
        pItem = i;
        if ( pItem->mbVisible ) {
            if ( pItem->mnBits & StatusBarItemBits::AutoSize ) {
                pItem->mnExtraWidth = nExtraWidth;
                if ( nExtraWidth2 ) {
                    pItem->mnExtraWidth++;
                    nExtraWidth2--;
                }
            } else {
                pItem->mnExtraWidth = 0;
            }

            pItem->mnX = nX;
            nX += pItem->mnWidth + pItem->mnExtraWidth + pItem->mnOffset;
        }
    }

    mbFormat = false;
}

Rectangle StatusBar::ImplGetItemRectPos( sal_uInt16 nPos ) const
{
    Rectangle       aRect;
    ImplStatusItem* pItem;
    pItem = ( nPos < mpItemList->size() ) ? (*mpItemList)[ nPos ] : nullptr;
    if ( pItem )
    {
        if ( pItem->mbVisible )
        {
            aRect.Left()   = pItem->mnX;
            aRect.Right()  = aRect.Left() + pItem->mnWidth + pItem->mnExtraWidth;
            aRect.Top()    = mnItemY;
            aRect.Bottom() = mnCalcHeight - STATUSBAR_OFFSET_Y;
        }
    }

    return aRect;
}

sal_uInt16 StatusBar::ImplGetFirstVisiblePos() const
{
    for( size_t nPos = 0; nPos < mpItemList->size(); nPos++ )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];
        if ( pItem )
        {
            if ( pItem->mbVisible )
                return sal_uInt16(nPos);
        }
    }

    return SAL_MAX_UINT16;
}

void StatusBar::ImplDrawText(vcl::RenderContext& rRenderContext)
{
    // prevent item box from being overwritten
    Rectangle aTextRect;
    aTextRect.Left() = STATUSBAR_OFFSET_X + 1;
    aTextRect.Top() = mnTextY;
    if (mbVisibleItems && (GetStyle() & WB_RIGHT))
        aTextRect.Right() = mnDX - mnItemsWidth - 1;
    else
        aTextRect.Right() = mnDX - 1;
    if (aTextRect.Right() > aTextRect.Left())
    {
        // compute position
        OUString aStr = GetText();
        sal_Int32 nPos = aStr.indexOf('\n');
        if (nPos != -1)
            aStr = aStr.copy(0, nPos);

        aTextRect.Bottom() = aTextRect.Top()+GetTextHeight()+1;

        rRenderContext.DrawText(aTextRect, aStr, DrawTextFlags::Left | DrawTextFlags::Top | DrawTextFlags::Clip | DrawTextFlags::EndEllipsis);
    }
}

void StatusBar::ImplDrawItem(vcl::RenderContext& rRenderContext, bool bOffScreen, sal_uInt16 nPos)
{
    Rectangle aRect = ImplGetItemRectPos(nPos);

    if (aRect.IsEmpty())
        return;

    // compute output region
    ImplStatusItem* pItem = (*mpItemList)[nPos];
    long nW = mpImplData->mnItemBorderWidth + 1;
    Rectangle aTextRect(aRect.Left() + nW, aRect.Top() + nW,
                        aRect.Right() - nW, aRect.Bottom() - nW);

    Size aTextRectSize(aTextRect.GetSize());

    if (bOffScreen)
    {
        mpImplData->mpVirDev->SetOutputSizePixel(aTextRectSize);
    }
    else
    {
        vcl::Region aRegion(aTextRect);
        rRenderContext.SetClipRegion(aRegion);
    }

    // print text
    Size aTextSize(rRenderContext.GetTextWidth(pItem->maText), rRenderContext.GetTextHeight());
    Point aTextPos = ImplGetItemTextPos(aTextRectSize, aTextSize, pItem->mnBits);
    if (bOffScreen)
    {
        mpImplData->mpVirDev->DrawText(aTextPos, pItem->maText);
    }
    else
    {
        aTextPos.X() += aTextRect.Left();
        aTextPos.Y() += aTextRect.Top();
        rRenderContext.DrawText(aTextPos, pItem->maText);
    }

    // call DrawItem if necessary
    if (pItem->mnBits & StatusBarItemBits::UserDraw)
    {
        if (bOffScreen)
        {
            mbInUserDraw = true;
            mpImplData->mpVirDev->EnableRTL( IsRTLEnabled() );
            UserDrawEvent aODEvt(this, mpImplData->mpVirDev, Rectangle(Point(), aTextRectSize), pItem->mnId);
            UserDraw(aODEvt);
            mpImplData->mpVirDev->EnableRTL(false);
            mbInUserDraw = false;
        }
        else
        {
            UserDrawEvent aODEvt(this, &rRenderContext, aTextRect, pItem->mnId);
            UserDraw(aODEvt);
        }
    }

    if (bOffScreen)
        rRenderContext.DrawOutDev(aTextRect.TopLeft(), aTextRectSize, Point(), aTextRectSize, *mpImplData->mpVirDev);
    else
        rRenderContext.SetClipRegion();

    // show frame
    if (mpImplData->mbDrawItemFrames)
    {
        if (!(pItem->mnBits & StatusBarItemBits::Flat))
        {
            DrawFrameStyle nStyle;

            if (pItem->mnBits & StatusBarItemBits::In)
                nStyle = DrawFrameStyle::In;
            else
                nStyle = DrawFrameStyle::Out;

            DecorationView aDecoView(&rRenderContext);
            aDecoView.DrawFrame(aRect, nStyle);
        }
    }
    else if (nPos != ImplGetFirstVisiblePos())
    {
        // draw separator
        Point aFrom(aRect.TopLeft());
        aFrom.X() -= 4;
        aFrom.Y()++;
        Point aTo(aRect.BottomLeft());
        aTo.X() -= 4;
        aTo.Y()--;

        DecorationView aDecoView(&rRenderContext);
        aDecoView.DrawSeparator(aFrom, aTo);
    }

    if (!rRenderContext.ImplIsRecordLayout())
        CallEventListeners(VCLEVENT_STATUSBAR_DRAWITEM, reinterpret_cast<void*>(pItem->mnId));
}

void DrawProgress(vcl::Window* pWindow, vcl::RenderContext& rRenderContext, const Point& rPos,
                  long nOffset, long nPrgsWidth, long nPrgsHeight,
                  sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                  const Rectangle& rFramePosSize)
{
    if (rRenderContext.IsNativeControlSupported(ControlType::Progress, ControlPart::Entire))
    {
        bool bNeedErase = ImplGetSVData()->maNWFData.mbProgressNeedsErase;

        long nFullWidth = (nPrgsWidth + nOffset) * (10000 / nPercentCount);
        long nPerc = (nPercent2 > 10000) ? 10000 : nPercent2;
        ImplControlValue aValue(nFullWidth * long(nPerc) / 10000);
        Rectangle aDrawRect(rPos, Size(nFullWidth, nPrgsHeight));
        Rectangle aControlRegion(aDrawRect);

        if(bNeedErase)
        {
            vcl::Window* pEraseWindow = pWindow;
            while (pEraseWindow->IsPaintTransparent() && !pEraseWindow->ImplGetWindowImpl()->mbFrame)
            {
                pEraseWindow = pEraseWindow->ImplGetWindowImpl()->mpParent;
            }

            if (pEraseWindow == pWindow)
            {
                // restore background of pWindow
                rRenderContext.Erase(rFramePosSize);
            }
            else
            {
                // restore transparent background
                Point aTL(pWindow->OutputToAbsoluteScreenPixel(rFramePosSize.TopLeft()));
                aTL = pEraseWindow->AbsoluteScreenToOutputPixel(aTL);
                Rectangle aRect(aTL, rFramePosSize.GetSize());
                pEraseWindow->Invalidate(aRect, InvalidateFlags::NoChildren     |
                                                InvalidateFlags::NoClipChildren |
                                                InvalidateFlags::Transparent);
                pEraseWindow->Update();
            }
            rRenderContext.Push(PushFlags::CLIPREGION);
            rRenderContext.IntersectClipRegion(rFramePosSize);
        }

        bool bNativeOK = rRenderContext.DrawNativeControl(ControlType::Progress, ControlPart::Entire, aControlRegion,
                                                          ControlState::ENABLED, aValue, OUString());
        if (bNeedErase)
            rRenderContext.Pop();
        if (bNativeOK)
            return;
    }

    // precompute values
    sal_uInt16 nPerc1 = nPercent1 / nPercentCount;
    sal_uInt16 nPerc2 = nPercent2 / nPercentCount;

    if (nPerc1 > nPerc2)
    {
        // support progress that can also decrease

        // compute rectangle
        long nDX = nPrgsWidth + nOffset;
        long nLeft = rPos.X() + ((nPerc1 - 1) * nDX);
        Rectangle aRect(nLeft, rPos.Y(), nLeft + nPrgsWidth, rPos.Y() + nPrgsHeight);

        do
        {
            rRenderContext.Erase(aRect);
            aRect.Left()  -= nDX;
            aRect.Right() -= nDX;
            nPerc1--;
        }
        while (nPerc1 > nPerc2);
    }
    else if (nPerc1 < nPerc2)
    {
        // draw Percent rectangle
        // if Percent2 greater than 100%, adapt values
        if (nPercent2 > 10000)
        {
            nPerc2 = 10000 / nPercentCount;
            if (nPerc1 >= nPerc2)
                nPerc1 = nPerc2 - 1;
        }

        // compute rectangle
        long nDX = nPrgsWidth + nOffset;
        long nLeft = rPos.X() + (nPerc1 * nDX);
        Rectangle aRect(nLeft, rPos.Y(), nLeft + nPrgsWidth, rPos.Y() + nPrgsHeight);

        do
        {
            rRenderContext.DrawRect(aRect);
            aRect.Left()  += nDX;
            aRect.Right() += nDX;
            nPerc1++;
        }
        while (nPerc1 < nPerc2);

        // if greater than 100%, set rectangle to blink
        if (nPercent2 > 10000)
        {
            // define on/off status
            if (((nPercent2 / nPercentCount) & 0x01) == (nPercentCount & 0x01))
            {
                aRect.Left()  -= nDX;
                aRect.Right() -= nDX;
                rRenderContext.Erase(aRect);
            }
        }
    }
}

void StatusBar::ImplDrawProgress(vcl::RenderContext& rRenderContext, sal_uInt16 nPercent2)
{
    bool bNative = rRenderContext.IsNativeControlSupported(ControlType::Progress, ControlPart::Entire);
    // bPaint: draw text also, else only update progress
    rRenderContext.DrawText(maPrgsTxtPos, maPrgsTxt);
    if (!bNative)
    {
        DecorationView aDecoView(&rRenderContext);
        aDecoView.DrawFrame(maPrgsFrameRect, DrawFrameStyle::In);
    }

    Point aPos(maPrgsFrameRect.Left() + STATUSBAR_PRGS_OFFSET,
               maPrgsFrameRect.Top()  + STATUSBAR_PRGS_OFFSET);
    long nPrgsHeight = mnPrgsSize;
    if (bNative)
    {
        aPos = maPrgsFrameRect.TopLeft();
        nPrgsHeight = maPrgsFrameRect.GetHeight();
    }
    DrawProgress(this, rRenderContext, aPos, mnPrgsSize / 2, mnPrgsSize, nPrgsHeight,
                 0, nPercent2 * 100, mnPercentCount, maPrgsFrameRect);
}

void StatusBar::ImplCalcProgressRect()
{
    // calculate text size
    Size aPrgsTxtSize( GetTextWidth( maPrgsTxt ), GetTextHeight() );
    maPrgsTxtPos.X()    = STATUSBAR_OFFSET_X+1;

    // calculate progress frame
    maPrgsFrameRect.Left()      = maPrgsTxtPos.X()+aPrgsTxtSize.Width()+STATUSBAR_OFFSET;
    maPrgsFrameRect.Top()       = mnItemY;
    maPrgsFrameRect.Bottom()    = mnCalcHeight - STATUSBAR_OFFSET_Y;

    // calculate size of progress rects
    mnPrgsSize = maPrgsFrameRect.Bottom()-maPrgsFrameRect.Top()-(STATUSBAR_PRGS_OFFSET*2);
    sal_uInt16 nMaxPercent = STATUSBAR_PRGS_COUNT;

    long nMaxWidth = mnDX-STATUSBAR_OFFSET-1;

    // make smaller if there are too many rects
    while ( maPrgsFrameRect.Left()+ImplCalcProgessWidth( nMaxPercent, mnPrgsSize ) > nMaxWidth )
    {
        nMaxPercent--;
        if ( nMaxPercent <= STATUSBAR_PRGS_MIN )
            break;
    }
    maPrgsFrameRect.Right() = maPrgsFrameRect.Left() + ImplCalcProgessWidth( nMaxPercent, mnPrgsSize );

    // save the divisor for later
    mnPercentCount = 10000 / nMaxPercent;
    bool bNativeOK = false;
    if( IsNativeControlSupported( ControlType::Progress, ControlPart::Entire ) )
    {
        ImplControlValue aValue;
        Rectangle aControlRegion( Rectangle( (const Point&)Point(), maPrgsFrameRect.GetSize() ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;
        if( (bNativeOK = GetNativeControlRegion( ControlType::Progress, ControlPart::Entire, aControlRegion,
                                                 ControlState::ENABLED, aValue, OUString(),
                                                 aNativeControlRegion, aNativeContentRegion ) ) )
        {
            long nProgressHeight = aNativeControlRegion.GetHeight();
            if( nProgressHeight > maPrgsFrameRect.GetHeight() )
            {
                long nDelta = nProgressHeight - maPrgsFrameRect.GetHeight();
                maPrgsFrameRect.Top() -= (nDelta - nDelta/2);
                maPrgsFrameRect.Bottom() += nDelta/2;
            }
            maPrgsTxtPos.Y() = maPrgsFrameRect.Top() + (nProgressHeight - GetTextHeight())/2;
        }
    }
    if( ! bNativeOK )
        maPrgsTxtPos.Y()    = mnTextY;
}

void StatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    // trigger toolbox only for left mouse button
    if ( rMEvt.IsLeft() )
    {
        if ( mbVisibleItems )
        {
            Point  aMousePos = rMEvt.GetPosPixel();

            // search for clicked item
            for ( size_t i = 0; i < mpItemList->size(); ++i )
            {
                ImplStatusItem* pItem = (*mpItemList)[ i ];
                // check item for being clicked
                if ( ImplGetItemRectPos( sal_uInt16(i) ).IsInside( aMousePos ) )
                {
                    mnCurItemId = pItem->mnId;
                    if ( rMEvt.GetClicks() == 2 )
                        DoubleClick();
                    else
                        Click();
                    mnCurItemId = 0;

                    // Item found
                    return;
                }
            }
        }

        // if there's no item, trigger Click or DoubleClick
        if ( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else
            Click();
    }
}

void StatusBar::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (mbFormat)
        ImplFormat();

    sal_uInt16 nItemCount = sal_uInt16( mpItemList->size() );

    if (mbProgressMode)
    {
        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aProgressColor = rStyleSettings.GetHighlightColor();
        if (aProgressColor == rStyleSettings.GetFaceColor())
            aProgressColor = rStyleSettings.GetDarkShadowColor();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(aProgressColor);

        ImplDrawProgress(rRenderContext, mnPercent);

        rRenderContext.Pop();
    }
    else
    {
        // draw text
        if (!mbVisibleItems || (GetStyle() & WB_RIGHT))
            ImplDrawText(rRenderContext);

        // draw items
        if (mbVisibleItems)
        {
            // Do offscreen only when we are not recording layout..
            bool bOffscreen = !rRenderContext.ImplIsRecordLayout();

            // tdf#94213 - un-necessary virtual-device in GL mode
            // causes context switch & hence flicker during sizing.
#if HAVE_FEATURE_OPENGL
            if( OpenGLWrapper::isVCLOpenGLEnabled() )
                bOffscreen = false;
#endif

            if (!bOffscreen)
                rRenderContext.Erase(rRect);

            for (sal_uInt16 i = 0; i < nItemCount; i++)
                ImplDrawItem(rRenderContext, bOffscreen, i);
        }
    }

    // draw line at the top of the status bar (to visually distinguish it from
    // shell / docking area)
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawLine(Point(0, 0), Point(mnDX-1, 0));
}

void StatusBar::Resize()
{
    // save width and height
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width() - ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset;
    mnDY = aSize.Height();
    mnCalcHeight = mnDY;

    mnItemY = STATUSBAR_OFFSET_Y;
    mnTextY = (mnCalcHeight-GetTextHeight())/2;

    // provoke re-formatting
    mbFormat = true;

    if ( mbProgressMode )
        ImplCalcProgressRect();

    Invalidate();
}

void StatusBar::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help in status bar
    if( rHEvt.KeyboardActivated() )
        return;

    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        Rectangle aItemRect = GetItemRect( nItemId );
        Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();

        if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
        {
            OUString aStr = GetHelpText( nItemId );
            Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
            return;
        }
        else if ( rHEvt.GetMode() & HelpEventMode::QUICK )
        {
            OUString aStr(GetQuickHelpText(nItemId));
            // show quickhelp if available
            if (!aStr.isEmpty())
            {
                Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
            aStr = GetItemText( nItemId );
            // show a quick help if item text doesn't fit
            if ( GetTextWidth( aStr ) > aItemRect.GetWidth() )
            {
                Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HelpEventMode::EXTENDED )
        {
            OUString aCommand = GetItemCommand( nItemId );
            OString aHelpId( GetHelpId( nItemId ) );

            if ( !aCommand.isEmpty() || !aHelpId.isEmpty() )
            {
                // show help text if there is one
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                {
                    if ( !aCommand.isEmpty() )
                        pHelp->Start( aCommand, this );
                    else if ( !aHelpId.isEmpty() )
                        pHelp->Start( OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ), this );
                }
                return;
            }
        }
    }

    Window::RequestHelp( rHEvt );
}

void StatusBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
        ImplFormat();
    else if ( nType == StateChangedType::UpdateMode )
        Invalidate();
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        mbFormat = true;
        ImplInitSettings();
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings();
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void StatusBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if (  (rDCEvt.GetType() == DataChangedEventType::DISPLAY         )
       || (rDCEvt.GetType() == DataChangedEventType::FONTS           )
       || (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION)
       || (  (rDCEvt.GetType() == DataChangedEventType::SETTINGS)
          && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE )
          )
       )
    {
        mbFormat = true;
        ImplInitSettings();
        long nFudge = GetTextHeight() / 4;
        for (ImplStatusItem* pItem : *mpItemList)
        {
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( nWidth > pItem->mnWidth + STATUSBAR_OFFSET )
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;
        }
        Size aSize = GetSizePixel();
        // do not disturb current width, since
        // CalcWindowSizePixel calculates a minimum width
        aSize.Height() = CalcWindowSizePixel().Height();
        SetSizePixel( aSize );
        Invalidate();
    }
}

void StatusBar::Click()
{
    CallEventListeners( VCLEVENT_STATUSBAR_CLICK );
    maClickHdl.Call( this );
}

void StatusBar::DoubleClick()
{
    CallEventListeners( VCLEVENT_STATUSBAR_DOUBLECLICK );
    maDoubleClickHdl.Call( this );
}

void StatusBar::UserDraw( const UserDrawEvent& )
{
}

void StatusBar::InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
                            StatusBarItemBits nBits,
                            long nOffset, sal_uInt16 nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "StatusBar::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != STATUSBAR_ITEM_NOTFOUND, "vcl",
                "StatusBar::InsertItem(): ItemId already exists" );

    // default: IN and CENTER
    if ( !(nBits & (StatusBarItemBits::In | StatusBarItemBits::Out | StatusBarItemBits::Flat)) )
        nBits |= StatusBarItemBits::In;
    if ( !(nBits & (StatusBarItemBits::Left | StatusBarItemBits::Right | StatusBarItemBits::Center)) )
        nBits |= StatusBarItemBits::Center;

    // create item
    if (mbAdjustHiDPI && GetDPIScaleFactor() != 1)
    {
        nWidth *= GetDPIScaleFactor();
    }
    long nFudge = GetTextHeight()/4;
    ImplStatusItem* pItem   = new ImplStatusItem;
    pItem->mnId             = nItemId;
    pItem->mnBits           = nBits;
    pItem->mnWidth          = (long)nWidth+nFudge+STATUSBAR_OFFSET;
    pItem->mnOffset         = nOffset;
    pItem->mpUserData       = nullptr;
    pItem->mbVisible        = true;

    // add item to list
    if ( nPos < mpItemList->size() ) {
        mpItemList->insert( mpItemList->begin() + nPos, pItem );
    } else {
        mpItemList->push_back( pItem );
    }

    mbFormat = true;
    if ( ImplIsItemUpdate() )
        Invalidate();

    CallEventListeners( VCLEVENT_STATUSBAR_ITEMADDED, reinterpret_cast<void*>(nItemId) );
}

void StatusBar::RemoveItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        delete (*mpItemList)[ nPos ];
        mpItemList->erase( mpItemList->begin() + nPos );

        mbFormat = true;
        if ( ImplIsItemUpdate() )
            Invalidate();

        CallEventListeners( VCLEVENT_STATUSBAR_ITEMREMOVED, reinterpret_cast<void*>(nItemId) );
    }
}

void StatusBar::ShowItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];
        if ( !pItem->mbVisible )
        {
            pItem->mbVisible = true;

            mbFormat = true;
            if ( ImplIsItemUpdate() )
                Invalidate();

            CallEventListeners( VCLEVENT_STATUSBAR_SHOWITEM, reinterpret_cast<void*>(nItemId) );
        }
    }
}

void StatusBar::HideItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];
        if ( pItem->mbVisible )
        {
            pItem->mbVisible = false;

            mbFormat = true;
            if ( ImplIsItemUpdate() )
                Invalidate();

            CallEventListeners( VCLEVENT_STATUSBAR_HIDEITEM, reinterpret_cast<void*>(nItemId) );
        }
    }
}

bool StatusBar::IsItemVisible( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mbVisible;
    else
        return false;
}

void StatusBar::Clear()
{
    // delete all items
    for (ImplStatusItem* i : *mpItemList) {
        delete i;
    }
    mpItemList->clear();

    mbFormat = true;
    if ( ImplIsItemUpdate() )
        Invalidate();

    CallEventListeners( VCLEVENT_STATUSBAR_ALLITEMSREMOVED );
}

sal_uInt16 StatusBar::GetItemCount() const
{
    return (sal_uInt16)mpItemList->size();
}

sal_uInt16 StatusBar::GetItemId( sal_uInt16 nPos ) const
{
    if ( nPos < mpItemList->size() )
        return (*mpItemList)[ nPos ]->mnId;
    return 0;
}

sal_uInt16 StatusBar::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        if ( (*mpItemList)[ i ]->mnId == nItemId ) {
            return sal_uInt16( i );
        }
    }

    return STATUSBAR_ITEM_NOTFOUND;
}

sal_uInt16 StatusBar::GetItemId( const Point& rPos ) const
{
    if ( AreItemsVisible() && !mbFormat )
    {
        sal_uInt16 nItemCount = GetItemCount();
        sal_uInt16 nPos;
        for ( nPos = 0; nPos < nItemCount; nPos++ )
        {
            // get rectangle
            Rectangle aRect = ImplGetItemRectPos( nPos );
            if ( aRect.IsInside( rPos ) )
                return (*mpItemList)[ nPos ]->mnId;
        }
    }

    return 0;
}

Rectangle StatusBar::GetItemRect( sal_uInt16 nItemId ) const
{
    Rectangle aRect;

    if ( AreItemsVisible() && !mbFormat )
    {
        sal_uInt16 nPos = GetItemPos( nItemId );
        if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        {
            // get rectangle and subtract frame
            aRect = ImplGetItemRectPos( nPos );
            long nW = mpImplData->mnItemBorderWidth+1;
            aRect.Top() += nW-1;
            aRect.Bottom() -= nW-1;
            aRect.Left() += nW;
            aRect.Right() -= nW;
            return aRect;
        }
    }

    return aRect;
}

Point StatusBar::GetItemTextPos( sal_uInt16 nItemId ) const
{
    if ( !mbFormat )
    {
        sal_uInt16 nPos = GetItemPos( nItemId );
        if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        {
            // get rectangle
            ImplStatusItem* pItem = (*mpItemList)[ nPos ];
            Rectangle aRect = ImplGetItemRectPos( nPos );
            long nW = mpImplData->mnItemBorderWidth + 1;
            Rectangle           aTextRect( aRect.Left()+nW, aRect.Top()+nW,
                                           aRect.Right()-nW, aRect.Bottom()-nW );
            Point aPos = ImplGetItemTextPos( aTextRect.GetSize(),
                                             Size( GetTextWidth( pItem->maText ), GetTextHeight() ),
                                             pItem->mnBits );
            if ( !mbInUserDraw )
            {
                aPos.X() += aTextRect.Left();
                aPos.Y() += aTextRect.Top();
            }
            return aPos;
        }
    }

    return Point();
}

sal_uLong StatusBar::GetItemWidth( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mnWidth;

    return 0;
}

StatusBarItemBits StatusBar::GetItemBits( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mnBits;

    return StatusBarItemBits::NONE;
}

long StatusBar::GetItemOffset( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mnOffset;

    return 0;
}

void StatusBar::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];

        if ( pItem->maText != rText )
        {
            pItem->maText = rText;

            // adjust item width - see also DataChanged()
            long nFudge = GetTextHeight()/4;
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( (nWidth > pItem->mnWidth + STATUSBAR_OFFSET) ||
                ((nWidth < pItem->mnWidth) && (mnDX - STATUSBAR_OFFSET) < mnItemsWidth  ))
            {
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;
                ImplFormat();
                Invalidate();
            }

            // re-draw item if StatusBar is visible and UpdateMode active
            if ( pItem->mbVisible && !mbFormat && ImplIsItemUpdate() )
            {
                Rectangle aRect = ImplGetItemRectPos(nPos);
                Invalidate(aRect);
                Flush();
            }
        }
    }
}

const OUString& StatusBar::GetItemText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert( nPos != STATUSBAR_ITEM_NOTFOUND );

    return (*mpItemList)[ nPos ]->maText;
}

void StatusBar::SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];

        if ( pItem->maCommand != rCommand )
            pItem->maCommand = rCommand;
    }
}

const OUString StatusBar::GetItemCommand( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->maCommand;

    return OUString();
}

void StatusBar::SetItemData( sal_uInt16 nItemId, void* pNewData )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];
        pItem->mpUserData = pNewData;

        // call Draw-Item if it's a User-Item
        if ( (pItem->mnBits & StatusBarItemBits::UserDraw) && pItem->mbVisible &&
             !mbFormat && ImplIsItemUpdate() )
        {
            Rectangle aRect = ImplGetItemRectPos(nPos);
            Invalidate(aRect, InvalidateFlags::NoErase);
            Flush();
        }
    }
}

void* StatusBar::GetItemData( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mpUserData;

    return nullptr;
}

void StatusBar::RedrawItem(sal_uInt16 nItemId)
{
    if ( mbFormat )
        return;

    sal_uInt16 nPos = GetItemPos(nItemId);
    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = (*mpItemList)[ nPos ];
    if (pItem && (pItem->mnBits & StatusBarItemBits::UserDraw) &&
        pItem->mbVisible && ImplIsItemUpdate())
    {
        Rectangle aRect = ImplGetItemRectPos(nPos);
        Invalidate(aRect);
        Flush();
    }
}

void StatusBar::SetHelpText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        (*mpItemList)[ nPos ]->maHelpText = rText;
}

const OUString& StatusBar::GetHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    ImplStatusItem* pItem = (*mpItemList)[ nPos ];
    if ( pItem->maHelpText.isEmpty() && ( !pItem->maHelpId.isEmpty() || !pItem->maCommand.isEmpty() ))
    {
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
        {
            if ( !pItem->maCommand.isEmpty() )
                pItem->maHelpText = pHelp->GetHelpText( pItem->maCommand, this );
            if ( pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty() )
                pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }
    }

    return pItem->maHelpText;
}

void StatusBar::SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        (*mpItemList)[ nPos ]->maQuickHelpText = rText;
}

const OUString& StatusBar::GetQuickHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    ImplStatusItem* pItem = (*mpItemList)[ nPos ];
    return pItem->maQuickHelpText;
}

void StatusBar::SetHelpId( sal_uInt16 nItemId, const OString& rHelpId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        (*mpItemList)[ nPos ]->maHelpId = rHelpId;
}

OString StatusBar::GetHelpId( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    OString aRet;
    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];
        if ( !pItem->maHelpId.isEmpty() )
            aRet = pItem->maHelpId;
        else
            aRet = OUStringToOString( pItem->maCommand, RTL_TEXTENCODING_UTF8 );
    }

    return aRet;
}

void StatusBar::StartProgressMode( const OUString& rText )
{
    SAL_WARN_IF( mbProgressMode, "vcl", "StatusBar::StartProgressMode(): progress mode is active" );

    mbProgressMode  = true;
    mnPercent       = 0;
    maPrgsTxt       = rText;

    // compute size
    ImplCalcProgressRect();

    // trigger Paint, which draws text and frame
    if ( IsReallyVisible() )
    {
        Invalidate();
        Flush();
    }
}

void StatusBar::SetProgressValue( sal_uInt16 nNewPercent )
{
    SAL_WARN_IF( !mbProgressMode, "vcl", "StatusBar::SetProgressValue(): no progress mode" );
    SAL_WARN_IF( nNewPercent > 100, "vcl", "StatusBar::SetProgressValue(): nPercent > 100" );

    if ( mbProgressMode
    &&   IsReallyVisible()
    &&   (!mnPercent || (mnPercent != nNewPercent)) )
    {
        Invalidate(maPrgsFrameRect, InvalidateFlags::NONE);
        Flush();
    }
    mnPercent = nNewPercent;
}

void StatusBar::EndProgressMode()
{
    SAL_WARN_IF( !mbProgressMode, "vcl", "StatusBar::EndProgressMode(): no progress mode" );

    mbProgressMode = false;
    maPrgsTxt.clear();

    if ( IsReallyVisible() )
    {
        Invalidate();
        Flush();
    }
}

void StatusBar::SetText(const OUString& rText)
{
    if ((!mbVisibleItems || (GetStyle() & WB_RIGHT)) && !mbProgressMode && IsReallyVisible() && IsUpdateMode())
    {
        if (mbFormat)
        {
            Invalidate();
            Window::SetText(rText);
        }
        else
        {
            Invalidate();
            Window::SetText(rText);
            Flush();
        }
    }
    else if (mbProgressMode)
    {
        maPrgsTxt = rText;
        if (IsReallyVisible())
        {
            Invalidate();
            Flush();
        }
    }
    else
    {
        Window::SetText(rText);
    }
}

Size StatusBar::CalcWindowSizePixel() const
{
    size_t  i = 0;
    size_t  nCount = mpItemList->size();
    long    nOffset = 0;
    long    nCalcWidth = (STATUSBAR_OFFSET_X*2);
    long    nCalcHeight;

    while ( i < nCount )
    {
        ImplStatusItem* pItem = (*mpItemList)[ i ];
        nCalcWidth += pItem->mnWidth + nOffset;
        nOffset = pItem->mnOffset;
        i++;
    }

    long nMinHeight = GetTextHeight();
    const long nBarTextOffset = STATUSBAR_OFFSET_TEXTY*2;
    long nProgressHeight = nMinHeight + nBarTextOffset;

    if( IsNativeControlSupported( ControlType::Progress, ControlPart::Entire ) )
    {
        ImplControlValue aValue;
        Rectangle aControlRegion( (const Point&)Point(), Size( nCalcWidth, nMinHeight ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;
        if( GetNativeControlRegion( ControlType::Progress, ControlPart::Entire,
                    aControlRegion, ControlState::ENABLED, aValue, OUString(),
                    aNativeControlRegion, aNativeContentRegion ) )
        {
            nProgressHeight = aNativeControlRegion.GetHeight();
        }
    }

    if( mpImplData->mbDrawItemFrames &&
        IsNativeControlSupported( ControlType::Frame, ControlPart::Border ) )
    {
        ImplControlValue aControlValue( static_cast<long>(DrawFrameFlags::NoDraw) );
        Rectangle aBound, aContent;
        Rectangle aNatRgn( Point( 0, 0 ), Size( 150, 50 ) );
        if( GetNativeControlRegion(ControlType::Frame, ControlPart::Border,
                    aNatRgn, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
        {
            mpImplData->mnItemBorderWidth =
                ( aBound.GetHeight() - aContent.GetHeight() ) / 2;
        }
    }

    nCalcHeight = nMinHeight+nBarTextOffset + 2*mpImplData->mnItemBorderWidth;
    if( nCalcHeight < nProgressHeight+2 )
        nCalcHeight = nProgressHeight+2;

    return Size( nCalcWidth, nCalcHeight );
}

void StatusBar::SetAccessibleName( sal_uInt16 nItemId, const OUString& rName )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = (*mpItemList)[ nPos ];

        if ( pItem->maAccessibleName != rName )
        {
            pItem->maAccessibleName = rName;
            CallEventListeners( VCLEVENT_STATUSBAR_NAMECHANGED, reinterpret_cast<void*>(pItem->mnId) );
        }
    }
}

const OUString& StatusBar::GetAccessibleName( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    return (*mpItemList)[ nPos ]->maAccessibleName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
