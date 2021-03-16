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


#include <sal/log.hxx>
#include <comphelper/string.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/vcllayout.hxx>
#include <vcl/status.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <config_features.h>
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

    VclPtr<VirtualDevice> mpVirDev;
};

StatusBar::ImplData::ImplData()
{
    mpVirDev = nullptr;
}

struct ImplStatusItem
{
    sal_uInt16                          mnId;
    StatusBarItemBits                   mnBits;
    tools::Long                                mnWidth;
    tools::Long                                mnOffset;
    tools::Long                                mnExtraWidth;
    tools::Long                                mnX;
    OUString                            maText;
    OUString                            maHelpText;
    OUString                            maQuickHelpText;
    OString                             maHelpId;
    void*                               mpUserData;
    bool                                mbVisible;
    OUString                            maAccessibleName;
    OUString                            maCommand;
    std::unique_ptr<SalLayout>          mxLayoutCache;
};

static tools::Long ImplCalcProgressWidth( sal_uInt16 nMax, tools::Long nSize )
{
    return ((nMax*(nSize+(nSize/2)))-(nSize/2)+(STATUSBAR_PRGS_OFFSET*2));
}

static Point ImplGetItemTextPos( const Size& rRectSize, const Size& rTextSize,
                                 StatusBarItemBits nStyle )
{
    tools::Long nX;
    tools::Long nY;
    tools::Long delta = (rTextSize.Height()/4) + 1;
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
    return !mbProgressMode && IsReallyVisible() && IsUpdateMode();
}

void StatusBar::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpImplData.reset(new ImplData);

    // default: RightAlign
    if ( !(nStyle & (WB_LEFT | WB_RIGHT)) )
        nStyle |= WB_RIGHT;

    Window::ImplInit( pParent, nStyle & ~WB_BORDER, nullptr );

    // remember WinBits
    mpImplData->mpVirDev = VclPtr<VirtualDevice>::Create( *this );
    mnCurItemId     = 0;
    mbFormat        = true;
    mbProgressMode  = false;
    mbInUserDraw    = false;
    mbAdjustHiDPI   = false;
    mnItemsWidth    = STATUSBAR_OFFSET_X;
    mnDX            = 0;
    mnDY            = 0;
    mnCalcHeight    = 0;
    mnTextY         = STATUSBAR_OFFSET_TEXTY;

    ImplInitSettings();

    SetOutputSizePixel( CalcWindowSizePixel() );
}

StatusBar::StatusBar( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::STATUSBAR ),
    mnLastProgressPaint_ms(osl_getGlobalTimer())
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
    mvItemList.clear();

    // delete VirtualDevice
    mpImplData->mpVirDev.disposeAndClear();
    mpImplData.reset();
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
    ApplyControlFont(rRenderContext, rStyleSettings.GetToolFont());

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
    tools::Long            nExtraWidth;
    tools::Long            nExtraWidth2;
    tools::Long            nX;
    sal_uInt16      nAutoSizeItems;
    bool            bChanged;

    do {
        // sum up widths
        nAutoSizeItems = 0;
        mnItemsWidth = STATUSBAR_OFFSET_X;
        bChanged = false;
        tools::Long nOffset = 0;
        for ( const auto & pItem : mvItemList ) {
            if ( pItem->mbVisible )
            {
                if ( pItem->mnBits & StatusBarItemBits::AutoSize ) {
                    nAutoSizeItems++;
                }

                mnItemsWidth += pItem->mnWidth + nOffset;
                nOffset = pItem->mnOffset;
            }
        }

        if ( mnDX > 0 && mnDX < mnItemsWidth )
        {
            // Total width of items is more than available width
            // Try to hide secondary elements, if any
            for ( auto & pItem : mvItemList )
            {
                if ( pItem->mbVisible && !(pItem->mnBits & StatusBarItemBits::Mandatory) )
                {
                    pItem->mbVisible = false;
                    bChanged = true;
                    break;
                }
            }
        }
        else if ( mnDX > mnItemsWidth )
        {
            // Width of statusbar is sufficient.
            // Try to restore hidden items, if any
            for ( auto & pItem : mvItemList )
            {
                if ( !pItem->mbVisible &&
                    !(pItem->mnBits & StatusBarItemBits::Mandatory) &&
                    pItem->mnWidth + nOffset + mnItemsWidth < mnDX )
                {
                    pItem->mbVisible = true;
                    bChanged = true;
                    break;
                }
            }
        }
    } while ( bChanged );

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

    for (auto & pItem : mvItemList) {
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

tools::Rectangle StatusBar::ImplGetItemRectPos( sal_uInt16 nPos ) const
{
    tools::Rectangle       aRect;
    ImplStatusItem* pItem = ( nPos < mvItemList.size() ) ? mvItemList[ nPos ].get() : nullptr;
    if ( pItem && pItem->mbVisible )
    {
        aRect.SetLeft( pItem->mnX );
        aRect.SetRight( aRect.Left() + pItem->mnWidth + pItem->mnExtraWidth );
        aRect.SetTop( STATUSBAR_OFFSET_Y );
        aRect.SetBottom( mnCalcHeight - STATUSBAR_OFFSET_Y );
    }

    return aRect;
}

sal_uInt16 StatusBar::ImplGetFirstVisiblePos() const
{
    for( size_t nPos = 0; nPos < mvItemList.size(); nPos++ )
    {
        ImplStatusItem* pItem = mvItemList[ nPos ].get();
        if ( pItem->mbVisible )
            return sal_uInt16(nPos);
    }

    return SAL_MAX_UINT16;
}

void StatusBar::ImplDrawText(vcl::RenderContext& rRenderContext)
{
    // prevent item box from being overwritten
    tools::Rectangle aTextRect;
    aTextRect.SetLeft( STATUSBAR_OFFSET_X + 1 );
    aTextRect.SetTop( mnTextY );
    if (GetStyle() & WB_RIGHT)
        aTextRect.SetRight( mnDX - mnItemsWidth - 1 );
    else
        aTextRect.SetRight( mnDX - 1 );
    if (aTextRect.Right() > aTextRect.Left())
    {
        // compute position
        OUString aStr = GetText();
        sal_Int32 nPos = aStr.indexOf('\n');
        if (nPos != -1)
            aStr = aStr.copy(0, nPos);

        aTextRect.SetBottom( aTextRect.Top()+GetTextHeight()+1 );

        rRenderContext.DrawText(aTextRect, aStr, DrawTextFlags::Left | DrawTextFlags::Top | DrawTextFlags::Clip | DrawTextFlags::EndEllipsis);
    }
}

void StatusBar::ImplDrawItem(vcl::RenderContext& rRenderContext, bool bOffScreen, sal_uInt16 nPos)
{
    tools::Rectangle aRect = ImplGetItemRectPos(nPos);

    if (aRect.IsEmpty())
        return;

    // compute output region
    ImplStatusItem* pItem = mvItemList[nPos].get();
    tools::Long nW = 1;
    tools::Rectangle aTextRect(aRect.Left() + nW, aRect.Top() + nW,
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

    // if the framework code is drawing status, let it do all the work
    if (!(pItem->mnBits & StatusBarItemBits::UserDraw))
    {
        SalLayout* pLayoutCache = pItem->mxLayoutCache.get();

        if(!pLayoutCache)
        {
            // update cache
            pItem->mxLayoutCache = rRenderContext.ImplLayout(pItem->maText, 0, -1);
            pLayoutCache = pItem->mxLayoutCache.get();
        }

        const SalLayoutGlyphs glyphs = pLayoutCache ? pLayoutCache->GetGlyphs() : SalLayoutGlyphs();
        Size aTextSize(rRenderContext.GetTextWidth(pItem->maText,0,-1,nullptr,&glyphs), rRenderContext.GetTextHeight());
        Point aTextPos = ImplGetItemTextPos(aTextRectSize, aTextSize, pItem->mnBits);

        if (bOffScreen)
        {
            mpImplData->mpVirDev->DrawText(
                        aTextPos,
                        pItem->maText,
                        0, -1, nullptr, nullptr,
                        &glyphs );
        }
        else
        {
            aTextPos.AdjustX(aTextRect.Left() );
            aTextPos.AdjustY(aTextRect.Top() );
            rRenderContext.DrawText(
                        aTextPos,
                        pItem->maText,
                        0, -1, nullptr, nullptr,
                        &glyphs );
        }
    }

    // call DrawItem if necessary
    if (pItem->mnBits & StatusBarItemBits::UserDraw)
    {
        if (bOffScreen)
        {
            mbInUserDraw = true;
            mpImplData->mpVirDev->EnableRTL( IsRTLEnabled() );
            UserDrawEvent aODEvt(this, mpImplData->mpVirDev, tools::Rectangle(Point(), aTextRectSize), pItem->mnId);
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

    if (nPos != ImplGetFirstVisiblePos())
    {
        // draw separator
        Point aFrom(aRect.TopLeft());
        aFrom.AdjustX( -4 );
        aFrom.AdjustY( 1 );
        Point aTo(aRect.BottomLeft());
        aTo.AdjustX( -4 );
        aTo.AdjustY( -1 );

        DecorationView aDecoView(&rRenderContext);
        aDecoView.DrawSeparator(aFrom, aTo);
    }

    if (!rRenderContext.ImplIsRecordLayout())
        CallEventListeners(VclEventId::StatusbarDrawItem, reinterpret_cast<void*>(pItem->mnId));
}

void DrawProgress(vcl::Window* pWindow, vcl::RenderContext& rRenderContext, const Point& rPos,
                  tools::Long nOffset, tools::Long nPrgsWidth, tools::Long nPrgsHeight,
                  sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                  const tools::Rectangle& rFramePosSize)
{
    if (rRenderContext.IsNativeControlSupported(ControlType::Progress, ControlPart::Entire))
    {
        bool bNeedErase = ImplGetSVData()->maNWFData.mbProgressNeedsErase;

        tools::Long nFullWidth = (nPrgsWidth + nOffset) * (10000 / nPercentCount);
        tools::Long nPerc = std::min<sal_uInt16>(nPercent2, 10000);
        ImplControlValue aValue(nFullWidth * nPerc / 10000);
        tools::Rectangle aDrawRect(rPos, Size(nFullWidth, nPrgsHeight));
        tools::Rectangle aControlRegion(aDrawRect);

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
                tools::Rectangle aRect(aTL, rFramePosSize.GetSize());
                pEraseWindow->Invalidate(aRect, InvalidateFlags::NoChildren     |
                                                InvalidateFlags::NoClipChildren |
                                                InvalidateFlags::Transparent);
                pEraseWindow->PaintImmediately();
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
        tools::Long nDX = nPrgsWidth + nOffset;
        tools::Long nLeft = rPos.X() + ((nPerc1 - 1) * nDX);
        tools::Rectangle aRect(nLeft, rPos.Y(), nLeft + nPrgsWidth, rPos.Y() + nPrgsHeight);

        do
        {
            rRenderContext.Erase(aRect);
            aRect.AdjustLeft( -nDX );
            aRect.AdjustRight( -nDX );
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
        tools::Long nDX = nPrgsWidth + nOffset;
        tools::Long nLeft = rPos.X() + (nPerc1 * nDX);
        tools::Rectangle aRect(nLeft, rPos.Y(), nLeft + nPrgsWidth, rPos.Y() + nPrgsHeight);

        do
        {
            rRenderContext.DrawRect(aRect);
            aRect.AdjustLeft(nDX );
            aRect.AdjustRight(nDX );
            nPerc1++;
        }
        while (nPerc1 < nPerc2);

        // if greater than 100%, set rectangle to blink
        if (nPercent2 > 10000)
        {
            // define on/off status
            if (((nPercent2 / nPercentCount) & 0x01) == (nPercentCount & 0x01))
            {
                aRect.AdjustLeft( -nDX );
                aRect.AdjustRight( -nDX );
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
    tools::Long nPrgsHeight = mnPrgsSize;
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
    maPrgsTxtPos.setX( STATUSBAR_OFFSET_X+1 );

    // calculate progress frame
    maPrgsFrameRect.SetLeft( maPrgsTxtPos.X()+aPrgsTxtSize.Width()+STATUSBAR_OFFSET );
    maPrgsFrameRect.SetTop( STATUSBAR_OFFSET_Y );
    maPrgsFrameRect.SetBottom( mnCalcHeight - STATUSBAR_OFFSET_Y );

    // calculate size of progress rects
    mnPrgsSize = maPrgsFrameRect.Bottom()-maPrgsFrameRect.Top()-(STATUSBAR_PRGS_OFFSET*2);
    sal_uInt16 nMaxPercent = STATUSBAR_PRGS_COUNT;

    tools::Long nMaxWidth = mnDX-STATUSBAR_OFFSET-1;

    // make smaller if there are too many rects
    while ( maPrgsFrameRect.Left()+ImplCalcProgressWidth( nMaxPercent, mnPrgsSize ) > nMaxWidth )
    {
        nMaxPercent--;
        if ( nMaxPercent <= STATUSBAR_PRGS_MIN )
            break;
    }
    maPrgsFrameRect.SetRight( maPrgsFrameRect.Left() + ImplCalcProgressWidth( nMaxPercent, mnPrgsSize ) );

    // save the divisor for later
    mnPercentCount = 10000 / nMaxPercent;
    bool bNativeOK = false;
    if( IsNativeControlSupported( ControlType::Progress, ControlPart::Entire ) )
    {
        ImplControlValue aValue;
        tools::Rectangle aControlRegion( tools::Rectangle( Point(), maPrgsFrameRect.GetSize() ) );
        tools::Rectangle aNativeControlRegion, aNativeContentRegion;
        if( (bNativeOK = GetNativeControlRegion( ControlType::Progress, ControlPart::Entire, aControlRegion,
                                                 ControlState::ENABLED, aValue,
                                                 aNativeControlRegion, aNativeContentRegion ) ) )
        {
            tools::Long nProgressHeight = aNativeControlRegion.GetHeight();
            if( nProgressHeight > maPrgsFrameRect.GetHeight() )
            {
                tools::Long nDelta = nProgressHeight - maPrgsFrameRect.GetHeight();
                maPrgsFrameRect.AdjustTop( -(nDelta - nDelta/2) );
                maPrgsFrameRect.AdjustBottom(nDelta/2 );
            }
            maPrgsTxtPos.setY( maPrgsFrameRect.Top() + (nProgressHeight - GetTextHeight())/2 );
        }
    }
    if( ! bNativeOK )
        maPrgsTxtPos.setY( mnTextY );
}

void StatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    // trigger toolbox only for left mouse button
    if ( !rMEvt.IsLeft() )
        return;

    Point  aMousePos = rMEvt.GetPosPixel();

    // search for clicked item
    for ( size_t i = 0; i < mvItemList.size(); ++i )
    {
        ImplStatusItem* pItem = mvItemList[ i ].get();
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

    // if there's no item, trigger Click or DoubleClick
    if ( rMEvt.GetClicks() == 2 )
        DoubleClick();
    else
        Click();
}

void StatusBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (mbFormat)
        ImplFormat();

    sal_uInt16 nItemCount = sal_uInt16( mvItemList.size() );

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
        if (GetStyle() & WB_RIGHT)
            ImplDrawText(rRenderContext);

        // draw items

        // Do offscreen only when we are not recording layout...
        bool bOffscreen = !rRenderContext.ImplIsRecordLayout();

        if (!bOffscreen)
            rRenderContext.Erase(rRect);

        for (sal_uInt16 i = 0; i < nItemCount; i++)
            ImplDrawItem(rRenderContext, bOffscreen, i);
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
        tools::Rectangle aItemRect = GetItemRect( nItemId );
        Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.SetLeft( aPt.X() );
        aItemRect.SetTop( aPt.Y() );
        aPt = OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.SetRight( aPt.X() );
        aItemRect.SetBottom( aPt.Y() );

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

    //invalidate layout cache
    for (auto & pItem : mvItemList)
    {
        pItem->mxLayoutCache.reset();
    }

}

void StatusBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if (  !((rDCEvt.GetType() == DataChangedEventType::DISPLAY         )
       || (rDCEvt.GetType() == DataChangedEventType::FONTS           )
       || (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION)
       || (  (rDCEvt.GetType() == DataChangedEventType::SETTINGS)
          && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE )
          ))
       )
        return;

    mbFormat = true;
    ImplInitSettings();
    tools::Long nFudge = GetTextHeight() / 4;
    for (auto & pItem : mvItemList)
    {
        tools::Long nWidth = GetTextWidth( pItem->maText ) + nFudge;
        if( nWidth > pItem->mnWidth + STATUSBAR_OFFSET )
            pItem->mnWidth = nWidth + STATUSBAR_OFFSET;

        pItem->mxLayoutCache.reset();
    }
    Size aSize = GetSizePixel();
    // do not disturb current width, since
    // CalcWindowSizePixel calculates a minimum width
    aSize.setHeight( CalcWindowSizePixel().Height() );
    SetSizePixel( aSize );
    Invalidate();
}

void StatusBar::Click()
{
    maClickHdl.Call( this );
}

void StatusBar::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

void StatusBar::UserDraw( const UserDrawEvent& )
{
}

void StatusBar::InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
                            StatusBarItemBits nBits,
                            tools::Long nOffset, sal_uInt16 nPos )
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
    if (mbAdjustHiDPI)
    {
        nWidth *= GetDPIScaleFactor();
    }
    tools::Long nFudge = GetTextHeight()/4;
    std::unique_ptr<ImplStatusItem> pItem(new ImplStatusItem);
    pItem->mnId             = nItemId;
    pItem->mnBits           = nBits;
    pItem->mnWidth          = static_cast<tools::Long>(nWidth)+nFudge+STATUSBAR_OFFSET;
    pItem->mnOffset         = nOffset;
    pItem->mpUserData       = nullptr;
    pItem->mbVisible        = true;

    // add item to list
    if ( nPos < mvItemList.size() ) {
        mvItemList.insert( mvItemList.begin() + nPos, std::move(pItem) );
    } else {
        mvItemList.push_back( std::move(pItem) );
    }

    mbFormat = true;
    if ( ImplIsItemUpdate() )
        Invalidate();

    CallEventListeners( VclEventId::StatusbarItemAdded, reinterpret_cast<void*>(nItemId) );
}

void StatusBar::RemoveItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        mvItemList.erase( mvItemList.begin() + nPos );

        mbFormat = true;
        if ( ImplIsItemUpdate() )
            Invalidate();

        CallEventListeners( VclEventId::StatusbarItemRemoved, reinterpret_cast<void*>(nItemId) );
    }
}

void StatusBar::ShowItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
    if ( !pItem->mbVisible )
    {
        pItem->mbVisible = true;

        mbFormat = true;
        if ( ImplIsItemUpdate() )
            Invalidate();

        CallEventListeners( VclEventId::StatusbarShowItem, reinterpret_cast<void*>(nItemId) );
    }
}

void StatusBar::HideItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
    if ( pItem->mbVisible )
    {
        pItem->mbVisible = false;

        mbFormat = true;
        if ( ImplIsItemUpdate() )
            Invalidate();

        CallEventListeners( VclEventId::StatusbarHideItem, reinterpret_cast<void*>(nItemId) );
    }
}

bool StatusBar::IsItemVisible( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mbVisible;
    else
        return false;
}

void StatusBar::Clear()
{
    // delete all items
    mvItemList.clear();

    mbFormat = true;
    if ( ImplIsItemUpdate() )
        Invalidate();

    CallEventListeners( VclEventId::StatusbarAllItemsRemoved );
}

sal_uInt16 StatusBar::GetItemCount() const
{
    return static_cast<sal_uInt16>(mvItemList.size());
}

sal_uInt16 StatusBar::GetItemId( sal_uInt16 nPos ) const
{
    if ( nPos < mvItemList.size() )
        return mvItemList[ nPos ]->mnId;
    return 0;
}

sal_uInt16 StatusBar::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mvItemList.size(); i < n; ++i ) {
        if ( mvItemList[ i ]->mnId == nItemId ) {
            return sal_uInt16( i );
        }
    }

    return STATUSBAR_ITEM_NOTFOUND;
}

sal_uInt16 StatusBar::GetItemId( const Point& rPos ) const
{
    if ( !mbFormat )
    {
        sal_uInt16 nItemCount = GetItemCount();
        sal_uInt16 nPos;
        for ( nPos = 0; nPos < nItemCount; nPos++ )
        {
            // get rectangle
            tools::Rectangle aRect = ImplGetItemRectPos( nPos );
            if ( aRect.IsInside( rPos ) )
                return mvItemList[ nPos ]->mnId;
        }
    }

    return 0;
}

tools::Rectangle StatusBar::GetItemRect( sal_uInt16 nItemId ) const
{
    tools::Rectangle aRect;

    if ( !mbFormat )
    {
        sal_uInt16 nPos = GetItemPos( nItemId );
        if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        {
            // get rectangle and subtract frame
            aRect = ImplGetItemRectPos( nPos );
            tools::Long nW = 1;
            aRect.AdjustTop(nW-1 );
            aRect.AdjustBottom( -(nW-1) );
            aRect.AdjustLeft(nW );
            aRect.AdjustRight( -nW );
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
            ImplStatusItem* pItem = mvItemList[ nPos ].get();
            tools::Rectangle aRect = ImplGetItemRectPos( nPos );
            tools::Long nW = 1;
            tools::Rectangle           aTextRect( aRect.Left()+nW, aRect.Top()+nW,
                                           aRect.Right()-nW, aRect.Bottom()-nW );
            Point aPos = ImplGetItemTextPos( aTextRect.GetSize(),
                                             Size( GetTextWidth( pItem->maText ), GetTextHeight() ),
                                             pItem->mnBits );
            if ( !mbInUserDraw )
            {
                aPos.AdjustX(aTextRect.Left() );
                aPos.AdjustY(aTextRect.Top() );
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
        return mvItemList[ nPos ]->mnWidth;

    return 0;
}

StatusBarItemBits StatusBar::GetItemBits( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mnBits;

    return StatusBarItemBits::NONE;
}

tools::Long StatusBar::GetItemOffset( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mnOffset;

    return 0;
}

void StatusBar::SetItemText( sal_uInt16 nItemId, const OUString& rText, int nCharsWidth )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = mvItemList[ nPos ].get();

    if ( pItem->maText == rText )
        return;

    pItem->maText = rText;

    // adjust item width - see also DataChanged()
    tools::Long nFudge = GetTextHeight()/4;

    tools::Long nWidth;
    if (nCharsWidth != -1)
    {
        std::unique_ptr<SalLayout> pSalLayout = ImplLayout("0",0,-1);
        const SalLayoutGlyphs glyphs = pSalLayout ? pSalLayout->GetGlyphs() : SalLayoutGlyphs();
        nWidth = GetTextWidth("0",0,-1,nullptr,&glyphs);
        nWidth = nWidth * nCharsWidth + nFudge;
    }
    else
    {
        std::unique_ptr<SalLayout> pSalLayout = ImplLayout(pItem->maText,0,-1);
        const SalLayoutGlyphs glyphs = pSalLayout ? pSalLayout->GetGlyphs() : SalLayoutGlyphs();
        nWidth = GetTextWidth( pItem->maText,0,-1,nullptr,&glyphs) + nFudge;
        // Store the calculated layout.
        pItem->mxLayoutCache = std::move(pSalLayout);
    }

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
        tools::Rectangle aRect = ImplGetItemRectPos(nPos);
        Invalidate(aRect);
        PaintImmediately();
    }
}

const OUString& StatusBar::GetItemText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert( nPos != STATUSBAR_ITEM_NOTFOUND );

    return mvItemList[ nPos ]->maText;
}

void StatusBar::SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mvItemList[ nPos ].get();

        if ( pItem->maCommand != rCommand )
            pItem->maCommand = rCommand;
    }
}

OUString StatusBar::GetItemCommand( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->maCommand;

    return OUString();
}

void StatusBar::SetItemData( sal_uInt16 nItemId, void* pNewData )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
    // invalidate cache
    pItem->mxLayoutCache.reset();
    pItem->mpUserData = pNewData;

    // call Draw-Item if it's a User-Item
    if ( (pItem->mnBits & StatusBarItemBits::UserDraw) && pItem->mbVisible &&
         !mbFormat && ImplIsItemUpdate() )
    {
        tools::Rectangle aRect = ImplGetItemRectPos(nPos);
        Invalidate(aRect, InvalidateFlags::NoErase);
        PaintImmediately();
    }
}

void* StatusBar::GetItemData( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mpUserData;

    return nullptr;
}

void StatusBar::RedrawItem(sal_uInt16 nItemId)
{
    if ( mbFormat )
        return;

    sal_uInt16 nPos = GetItemPos(nItemId);
    if ( nPos == STATUSBAR_ITEM_NOTFOUND )
        return;

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
    if ((pItem->mnBits & StatusBarItemBits::UserDraw) &&
        pItem->mbVisible && ImplIsItemUpdate())
    {
        tools::Rectangle aRect = ImplGetItemRectPos(nPos);
        Invalidate(aRect);
        PaintImmediately();
    }
}

void StatusBar::SetHelpText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        mvItemList[ nPos ]->maHelpText = rText;
}

const OUString& StatusBar::GetHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
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
        mvItemList[ nPos ]->maQuickHelpText = rText;
}

const OUString& StatusBar::GetQuickHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    ImplStatusItem* pItem = mvItemList[ nPos ].get();
    return pItem->maQuickHelpText;
}

void StatusBar::SetHelpId( sal_uInt16 nItemId, const OString& rHelpId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        mvItemList[ nPos ]->maHelpId = rHelpId;
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
        PaintImmediately();
    }
}

void StatusBar::SetProgressValue( sal_uInt16 nNewPercent )
{
    SAL_WARN_IF( !mbProgressMode, "vcl", "StatusBar::SetProgressValue(): no progress mode" );
    SAL_WARN_IF( nNewPercent > 100, "vcl", "StatusBar::SetProgressValue(): nPercent > 100" );

    bool bInvalidate = mbProgressMode && IsReallyVisible() && (!mnPercent || (mnPercent != nNewPercent));

    mnPercent = nNewPercent;

    if (bInvalidate)
    {
        // Rate limit how often we paint, otherwise in some loading scenarios we can spend significant
        // time just painting progress bars.
        sal_uInt32 nTime_ms = osl_getGlobalTimer();
        if ((nTime_ms - mnLastProgressPaint_ms) > 100)
        {
            Invalidate(maPrgsFrameRect);
            PaintImmediately();
            mnLastProgressPaint_ms = nTime_ms;
        }
    }
}

void StatusBar::EndProgressMode()
{
    SAL_WARN_IF( !mbProgressMode, "vcl", "StatusBar::EndProgressMode(): no progress mode" );

    mbProgressMode = false;
    maPrgsTxt.clear();

    if ( IsReallyVisible() )
    {
        Invalidate();
        PaintImmediately();
    }
}

void StatusBar::SetText(const OUString& rText)
{
    if ((GetStyle() & WB_RIGHT) && !mbProgressMode && IsReallyVisible() && IsUpdateMode())
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
            PaintImmediately();
        }
    }
    else if (mbProgressMode)
    {
        maPrgsTxt = rText;
        if (IsReallyVisible())
        {
            Invalidate();
            PaintImmediately();
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
    size_t  nCount = mvItemList.size();
    tools::Long    nOffset = 0;
    tools::Long    nCalcWidth = STATUSBAR_OFFSET_X*2;
    tools::Long    nCalcHeight;

    while ( i < nCount )
    {
        ImplStatusItem* pItem = mvItemList[ i ].get();
        nCalcWidth += pItem->mnWidth + nOffset;
        nOffset = pItem->mnOffset;
        i++;
    }

    tools::Long nMinHeight = GetTextHeight();
    const tools::Long nBarTextOffset = STATUSBAR_OFFSET_TEXTY*2;
    tools::Long nProgressHeight = nMinHeight + nBarTextOffset;

    if( IsNativeControlSupported( ControlType::Progress, ControlPart::Entire ) )
    {
        ImplControlValue aValue;
        tools::Rectangle aControlRegion( Point(), Size( nCalcWidth, nMinHeight ) );
        tools::Rectangle aNativeControlRegion, aNativeContentRegion;
        if( GetNativeControlRegion( ControlType::Progress, ControlPart::Entire,
                    aControlRegion, ControlState::ENABLED, aValue,
                    aNativeControlRegion, aNativeContentRegion ) )
        {
            nProgressHeight = aNativeControlRegion.GetHeight();
        }
    }

    nCalcHeight = nMinHeight+nBarTextOffset;
    if( nCalcHeight < nProgressHeight+2 )
        nCalcHeight = nProgressHeight+2;

    return Size( nCalcWidth, nCalcHeight );
}

void StatusBar::SetAccessibleName( sal_uInt16 nItemId, const OUString& rName )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mvItemList[ nPos ].get();

        if ( pItem->maAccessibleName != rName )
        {
            pItem->maAccessibleName = rName;
            CallEventListeners( VclEventId::StatusbarNameChanged, reinterpret_cast<void*>(pItem->mnId) );
        }
    }
}

const OUString& StatusBar::GetAccessibleName( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    assert ( nPos != STATUSBAR_ITEM_NOTFOUND );

    return mvItemList[ nPos ]->maAccessibleName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
