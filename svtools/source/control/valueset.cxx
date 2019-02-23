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
#include <vcl/builderfactory.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/virdev.hxx>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include "valueimp.hxx"

#include <svtools/valueset.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::accessibility;

namespace
{

enum
{
    ITEM_OFFSET = 4,
    ITEM_OFFSET_DOUBLE = 6,
    NAME_LINE_OFF_X = 2,
    NAME_LINE_OFF_Y = 2,
    NAME_LINE_HEIGHT = 2,
    NAME_OFFSET = 2,
    SCRBAR_OFFSET = 1,
    SCROLL_OFFSET = 4
};

}

ValueSet::ValueSet( vcl::Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle ),
    maColor( COL_TRANSPARENT )
{
    mpNoneItem.reset(nullptr);
    mxScrollBar.reset(nullptr);

    mnItemWidth         = 0;
    mnItemHeight        = 0;
    mnTextOffset        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnSelItemId         = 0;
    mnHighItemId        = 0;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = DrawFrameStyle::NONE;
    mbFormat            = true;
    mbHighlight         = false;
    mbSelection         = false;
    mbNoSelection       = true;
    mbDrawSelection     = true;
    mbBlackSel          = false;
    mbDoubleSel         = false;
    mbScroll            = false;
    mbFullMode          = true;
    mbEdgeBlending      = false;
    mbHasVisibleItems   = false;

    ImplInitSettings( true, true, true );
}


VCL_BUILDER_FACTORY_CONSTRUCTOR(ValueSet, WB_TABSTOP)

ValueSet::~ValueSet()
{
    disposeOnce();
}

void ValueSet::dispose()
{
    Reference<XComponent> xComponent(GetAccessible(false), UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    ImplDeleteItems();
    mxScrollBar.disposeAndClear();
    Control::dispose();
}

void ValueSet::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ValueSetItem* pItem = mItemList[i].get();
        if ( pItem->mbVisible && ImplHasAccessibleListeners() )
        {
            Any aOldAny;
            Any aNewAny;

            aOldAny <<= pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
            ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
        }

        mItemList[i].reset();
    }

    mItemList.clear();
}

void ValueSet::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    ApplyControlFont(rRenderContext, rStyleSettings.GetAppFont());
    ApplyControlForeground(rRenderContext, rStyleSettings.GetButtonTextColor());
    SetTextFillColor();
    Color aColor;
    if (GetStyle() & WB_MENUSTYLEVALUESET)
        aColor = rStyleSettings.GetMenuColor();
    else if (IsEnabled() && (GetStyle() & WB_FLATVALUESET))
        aColor = rStyleSettings.GetWindowColor();
    else
        aColor = rStyleSettings.GetFaceColor();
    ApplyControlBackground(rRenderContext, aColor);
}

void ValueSet::ImplInitSettings(bool bFont, bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
    {
        ApplyControlFont(*this, rStyleSettings.GetAppFont());
    }

    if (bForeground || bFont)
    {
        ApplyControlForeground(*this, rStyleSettings.GetButtonTextColor());
        SetTextFillColor();
    }

    if (bBackground)
    {
        Color aColor;
        if (GetStyle() & WB_MENUSTYLEVALUESET)
            aColor = rStyleSettings.GetMenuColor();
        else if (IsEnabled() && (GetStyle() & WB_FLATVALUESET))
            aColor = rStyleSettings.GetWindowColor();
        else
            aColor = rStyleSettings.GetFaceColor();
        ApplyControlBackground(*this, aColor);
    }
}

void ValueSet::ImplInitScrollBar()
{
    if (!(GetStyle() & WB_VSCROLL))
        return;

    if (!mxScrollBar.get())
    {
        mxScrollBar.reset(VclPtr<ScrollBar>::Create(this, WB_VSCROLL | WB_DRAG));
        mxScrollBar->SetScrollHdl(LINK(this, ValueSet, ImplScrollHdl));
    }
    else
    {
        // adapt the width because of the changed settings
        long nScrBarWidth = Application::GetSettings().GetStyleSettings().GetScrollBarSize();
        mxScrollBar->setPosSizePixel(0, 0, nScrBarWidth, 0, PosSizeFlags::Width);
    }
}

void ValueSet::ImplFormatItem(vcl::RenderContext& rRenderContext, ValueSetItem* pItem, tools::Rectangle aRect)
{
    WinBits nStyle = GetStyle();
    if (nStyle & WB_ITEMBORDER)
    {
        aRect.AdjustLeft(1 );
        aRect.AdjustTop(1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );

        if (nStyle & WB_FLATVALUESET)
        {
            sal_Int32 nBorder = (nStyle & WB_DOUBLEBORDER) ? 2 : 1;

            aRect.AdjustLeft(nBorder );
            aRect.AdjustTop(nBorder );
            aRect.AdjustRight( -nBorder );
            aRect.AdjustBottom( -nBorder );
        }
        else
        {
            DecorationView aView(&rRenderContext);
            aRect = aView.DrawFrame(aRect, mnFrameStyle);
        }
    }

    if (pItem == mpNoneItem.get())
        pItem->maText = GetText();

    if (!((aRect.GetHeight() > 0) && (aRect.GetWidth() > 0)))
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (pItem == mpNoneItem.get())
    {
        rRenderContext.SetTextColor((nStyle & WB_MENUSTYLEVALUESET)
                                        ? rStyleSettings.GetMenuTextColor()
                                        : rStyleSettings.GetWindowTextColor());
        rRenderContext.SetTextFillColor();
        rRenderContext.SetFillColor((nStyle & WB_MENUSTYLEVALUESET)
                                        ? rStyleSettings.GetMenuColor()
                                        : rStyleSettings.GetWindowColor());
        rRenderContext.DrawRect(aRect);
        Point aTxtPos(aRect.Left() + 2, aRect.Top());
        long nTxtWidth = rRenderContext.GetTextWidth(pItem->maText);
        if ((aTxtPos.X() + nTxtWidth) > aRect.Right())
        {
            rRenderContext.SetClipRegion(vcl::Region(aRect));
            rRenderContext.DrawText(aTxtPos, pItem->maText);
            rRenderContext.SetClipRegion();
        }
        else
            rRenderContext.DrawText(aTxtPos, pItem->maText);
    }
    else if (pItem->meType == VALUESETITEM_COLOR)
    {
        rRenderContext.SetFillColor(pItem->maColor);
        rRenderContext.DrawRect(aRect);
    }
    else
    {
        if (IsColor())
            rRenderContext.SetFillColor(maColor);
        else if (nStyle & WB_MENUSTYLEVALUESET)
            rRenderContext.SetFillColor(rStyleSettings.GetMenuColor());
        else if (IsEnabled())
            rRenderContext.SetFillColor(rStyleSettings.GetWindowColor());
        else
            rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        rRenderContext.DrawRect(aRect);

        if (pItem->meType == VALUESETITEM_USERDRAW)
        {
            UserDrawEvent aUDEvt(this, &rRenderContext, aRect, pItem->mnId);
            UserDraw(aUDEvt);
        }
        else
        {
            Size aImageSize = pItem->maImage.GetSizePixel();
            Size  aRectSize = aRect.GetSize();
            Point aPos(aRect.Left(), aRect.Top());
            aPos.AdjustX((aRectSize.Width() - aImageSize.Width()) / 2 );

            if (pItem->meType != VALUESETITEM_IMAGE_AND_TEXT)
                aPos.AdjustY((aRectSize.Height() - aImageSize.Height()) / 2 );

            DrawImageFlags  nImageStyle  = DrawImageFlags::NONE;
            if (!IsEnabled())
                nImageStyle  |= DrawImageFlags::Disable;

            if (aImageSize.Width()  > aRectSize.Width() ||
                aImageSize.Height() > aRectSize.Height())
            {
                rRenderContext.SetClipRegion(vcl::Region(aRect));
                rRenderContext.DrawImage(aPos, pItem->maImage, nImageStyle);
                rRenderContext.SetClipRegion();
            }
            else
                rRenderContext.DrawImage(aPos, pItem->maImage, nImageStyle);

            if (pItem->meType == VALUESETITEM_IMAGE_AND_TEXT)
            {
                rRenderContext.SetFont(rRenderContext.GetFont());
                rRenderContext.SetTextColor((nStyle & WB_MENUSTYLEVALUESET) ? rStyleSettings.GetMenuTextColor() : rStyleSettings.GetWindowTextColor());
                rRenderContext.SetTextFillColor();

                long nTxtWidth = rRenderContext.GetTextWidth(pItem->maText);

                if (nTxtWidth > aRect.GetWidth())
                    rRenderContext.SetClipRegion(vcl::Region(aRect));

                rRenderContext.DrawText(Point(aRect.Left() + (aRect.GetWidth() - nTxtWidth) / 2,
                                              aRect.Bottom() - rRenderContext.GetTextHeight()),
                                        pItem->maText);

                if (nTxtWidth > aRect.GetWidth())
                    rRenderContext.SetClipRegion();
            }
        }
    }

    const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

    if (nEdgeBlendingPercent)
    {
        const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
        const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
        const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
        const BitmapEx aBlendFrame(createBlendFrame(aRect.GetSize(), nAlpha, rTopLeft, rBottomRight));

        if (!aBlendFrame.IsEmpty())
        {
            rRenderContext.DrawBitmapEx(aRect.TopLeft(), aBlendFrame);
        }
    }
}

Reference<XAccessible> ValueSet::CreateAccessible()
{
    return new ValueSetAcc( this );
}

void ValueSet::Format(vcl::RenderContext& rRenderContext)
{
    Size aWinSize(GetOutputSizePixel());
    size_t nItemCount = mItemList.size();
    WinBits nStyle = GetStyle();
    long nTxtHeight = rRenderContext.GetTextHeight();
    long nOff;
    long nNoneHeight;
    long nNoneSpace;
    VclPtr<ScrollBar> xDeletedScrollBar;

    // consider the scrolling
    if (nStyle & WB_VSCROLL)
        ImplInitScrollBar();
    else
    {
        xDeletedScrollBar = mxScrollBar;
        mxScrollBar.clear();
    }

    // calculate item offset
    if (nStyle & WB_ITEMBORDER)
    {
        if (nStyle & WB_DOUBLEBORDER)
            nOff = ITEM_OFFSET_DOUBLE;
        else
            nOff = ITEM_OFFSET;
    }
    else
        nOff = 0;

    // consider size, if NameField does exist
    if (nStyle & WB_NAMEFIELD)
    {
        mnTextOffset = aWinSize.Height() - nTxtHeight - NAME_OFFSET;
        aWinSize.AdjustHeight( -(nTxtHeight + NAME_OFFSET) );

        if (!(nStyle & WB_FLATVALUESET))
        {
            mnTextOffset -= NAME_LINE_HEIGHT + NAME_LINE_OFF_Y;
            aWinSize.AdjustHeight( -(NAME_LINE_HEIGHT + NAME_LINE_OFF_Y) );
        }
    }
    else
        mnTextOffset = 0;

    // consider offset and size, if NoneField does exist
    if (nStyle & WB_NONEFIELD)
    {
        nNoneHeight = nTxtHeight + nOff;
        nNoneSpace = mnSpacing;
    }
    else
    {
        nNoneHeight = 0;
        nNoneSpace = 0;

        if (mpNoneItem)
            mpNoneItem.reset(nullptr);
    }

    // calculate ScrollBar width
    long nScrBarWidth = 0;
    if (mxScrollBar.get())
        nScrBarWidth = mxScrollBar->GetSizePixel().Width() + SCRBAR_OFFSET;

    // calculate number of columns
    if (!mnUserCols)
    {
        if (mnUserItemWidth)
        {
            mnCols = static_cast<sal_uInt16>((aWinSize.Width() - nScrBarWidth + mnSpacing) / (mnUserItemWidth + mnSpacing));
            if (mnCols <= 0)
                mnCols = 1;
        }
        else
        {
            mnCols = 1;
        }
    }
    else
    {
        mnCols = mnUserCols;
    }

    // calculate number of rows
    mbScroll = false;

    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount) + mnCols - 1) / mnCols;
    if (mnLines <= 0)
        mnLines = 1;

    long nCalcHeight = aWinSize.Height() - nNoneHeight;
    if (mnUserVisLines)
    {
        mnVisLines = mnUserVisLines;
    }
    else if (mnUserItemHeight)
    {
        mnVisLines = (nCalcHeight - nNoneSpace + mnSpacing) / (mnUserItemHeight + mnSpacing);
        if (!mnVisLines)
            mnVisLines = 1;
    }
    else
    {
        mnVisLines = mnLines;
    }

    if (mnLines > mnVisLines)
        mbScroll = true;

    if (mnLines <= mnVisLines)
    {
        mnFirstLine = 0;
    }
    else
    {
        if (mnFirstLine > static_cast<sal_uInt16>(mnLines - mnVisLines))
            mnFirstLine = static_cast<sal_uInt16>(mnLines - mnVisLines);
    }

    // calculate item size
    const long nColSpace  = (mnCols - 1) * static_cast<long>(mnSpacing);
    const long nLineSpace = ((mnVisLines - 1) * mnSpacing) + nNoneSpace;
    if (mnUserItemWidth && !mnUserCols)
    {
        mnItemWidth = mnUserItemWidth;
        if (mnItemWidth > aWinSize.Width() - nScrBarWidth - nColSpace)
            mnItemWidth = aWinSize.Width() - nScrBarWidth - nColSpace;
    }
    else
        mnItemWidth = (aWinSize.Width() - nScrBarWidth-nColSpace) / mnCols;
    if (mnUserItemHeight && !mnUserVisLines)
    {
        mnItemHeight = mnUserItemHeight;
        if (mnItemHeight > nCalcHeight - nNoneSpace)
            mnItemHeight = nCalcHeight - nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        mnItemHeight = nCalcHeight / mnVisLines;
    }

    // nothing is changed in case of too small items
    if ((mnItemWidth <= 0) ||
        (mnItemHeight <= ((nStyle & WB_ITEMBORDER) ? 4 : 2)) ||
        !nItemCount)
    {
        mbHasVisibleItems = false;

        if ((nStyle & WB_NONEFIELD) && mpNoneItem)
        {
            mpNoneItem->mbVisible = false;
            mpNoneItem->maText = GetText();
        }

        for (size_t i = 0; i < nItemCount; i++)
        {
            mItemList[i]->mbVisible = false;
        }

        if (mxScrollBar.get())
            mxScrollBar->Hide();
    }
    else
    {
        mbHasVisibleItems = true;

        // determine Frame-Style
        if (nStyle & WB_DOUBLEBORDER)
            mnFrameStyle = DrawFrameStyle::DoubleIn;
        else
            mnFrameStyle = DrawFrameStyle::In;

        // determine selected color and width
        // if necessary change the colors, to make the selection
        // better detectable
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aHighColor(rStyleSettings.GetHighlightColor());
        if (((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
             (aHighColor.GetBlue() > 0x80)) ||
            ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
             (aHighColor.GetBlue() == 0x80)))
        {
            mbBlackSel = true;
        }
        else
        {
            mbBlackSel = false;
        }
        // draw the selection with double width if the items are bigger
        if ((nStyle & WB_DOUBLEBORDER) &&
            ((mnItemWidth >= 25) && (mnItemHeight >= 20)))
        {
            mbDoubleSel = true;
        }
        else
        {
            mbDoubleSel = false;
        }

        // calculate offsets
        long nStartX;
        long nStartY;
        if (mbFullMode)
        {
            long nAllItemWidth = (mnItemWidth * mnCols) + nColSpace;
            long nAllItemHeight = (mnItemHeight * mnVisLines) + nNoneHeight + nLineSpace;
            nStartX = (aWinSize.Width() - nScrBarWidth - nAllItemWidth) / 2;
            nStartY = (aWinSize.Height() - nAllItemHeight) / 2;
        }
        else
        {
            nStartX = 0;
            nStartY = 0;
        }

        // calculate and draw items
        rRenderContext.SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // create NoSelection field and show it
        if (nStyle & WB_NONEFIELD)
        {
            if (mpNoneItem == nullptr)
                mpNoneItem.reset(new ValueSetItem(*this));

            mpNoneItem->mnId = 0;
            mpNoneItem->meType = VALUESETITEM_NONE;
            mpNoneItem->mbVisible = true;
            maNoneItemRect.SetLeft( x );
            maNoneItemRect.SetTop( y );
            maNoneItemRect.SetRight( maNoneItemRect.Left() + aWinSize.Width() - x - 1 );
            maNoneItemRect.SetBottom( y + nNoneHeight - 1 );

            ImplFormatItem(rRenderContext, mpNoneItem.get(), maNoneItemRect);

            y += nNoneHeight + nNoneSpace;
        }

        // draw items
        sal_uLong nFirstItem = static_cast<sal_uLong>(mnFirstLine) * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        maItemListRect.SetLeft( x );
        maItemListRect.SetTop( y );
        maItemListRect.SetRight( x + mnCols * (mnItemWidth + mnSpacing) - mnSpacing - 1 );
        maItemListRect.SetBottom( y + mnVisLines * (mnItemHeight + mnSpacing) - mnSpacing - 1 );

        if (!mbFullMode)
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if (y + (mnVisLines * (mnItemHeight + mnSpacing)) < aWinSize.Height())
                nLastItem += mnCols;
            maItemListRect.SetBottom( aWinSize.Height() - y );
        }
        for (size_t i = 0; i < nItemCount; i++)
        {
            ValueSetItem* pItem = mItemList[i].get();

            if (i >= nFirstItem && i < nLastItem)
            {
                if (!pItem->mbVisible && ImplHasAccessibleListeners())
                {
                    Any aOldAny;
                    Any aNewAny;

                    aNewAny <<= pItem->GetAccessible(false/*bIsTransientChildrenDisabled*/);
                    ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
                }

                pItem->mbVisible = true;
                ImplFormatItem(rRenderContext, pItem, tools::Rectangle(Point(x, y), Size(mnItemWidth, mnItemHeight)));

                if (!((i + 1) % mnCols))
                {
                    x = nStartX;
                    y += mnItemHeight + mnSpacing;
                }
                else
                    x += mnItemWidth + mnSpacing;
            }
            else
            {
                if (pItem->mbVisible && ImplHasAccessibleListeners())
                {
                    Any aOldAny;
                    Any aNewAny;

                    aOldAny <<= pItem->GetAccessible(false/*bIsTransientChildrenDisabled*/);
                    ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
                }

                pItem->mbVisible = false;
            }
        }

        // arrange ScrollBar, set values and show it
        if (mxScrollBar.get())
        {
            Point   aPos(aWinSize.Width() - nScrBarWidth + SCRBAR_OFFSET, 0);
            Size    aSize(nScrBarWidth - SCRBAR_OFFSET, aWinSize.Height());
            // If a none field is visible, then we center the scrollbar
            if (nStyle & WB_NONEFIELD)
            {
                aPos.setY( nStartY + nNoneHeight + 1 );
                aSize.setHeight( ((mnItemHeight + mnSpacing) * mnVisLines) - 2 - mnSpacing );
            }
            mxScrollBar->SetPosSizePixel(aPos, aSize);
            mxScrollBar->SetRangeMax(mnLines);
            mxScrollBar->SetVisibleSize(mnVisLines);
            mxScrollBar->SetThumbPos(static_cast<long>(mnFirstLine));
            long nPageSize = mnVisLines;
            if (nPageSize < 1)
                nPageSize = 1;
            mxScrollBar->SetPageSize(nPageSize);
            mxScrollBar->Show();
        }
    }

    // waiting for the next since the formatting is finished
    mbFormat = false;

    xDeletedScrollBar.disposeAndClear();
}

void ValueSet::ImplDrawItemText(vcl::RenderContext& rRenderContext, const OUString& rText)
{
    if (!(GetStyle() & WB_NAMEFIELD))
        return;

    Size aWinSize(GetOutputSizePixel());
    long nTxtWidth = rRenderContext.GetTextWidth(rText);
    long nTxtOffset = mnTextOffset;

    // delete rectangle and show text
    if (GetStyle() & WB_FLATVALUESET)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        rRenderContext.DrawRect(tools::Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
        rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
    }
    else
    {
        nTxtOffset += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        rRenderContext.Erase(tools::Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
    }
    rRenderContext.DrawText(Point((aWinSize.Width() - nTxtWidth) / 2, nTxtOffset + (NAME_OFFSET / 2)), rText);
}

void ValueSet::ImplDrawSelect(vcl::RenderContext& rRenderContext)
{
    if (!IsReallyVisible())
        return;

    const bool bFocus = HasFocus();
    const bool bDrawSel = !((mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight));

    if (!bFocus && !bDrawSel)
    {
        ImplDrawItemText(rRenderContext, OUString());
        return;
    }

    ImplDrawSelect(rRenderContext, mnSelItemId, bFocus, bDrawSel);
    if (mbHighlight)
    {
        ImplDrawSelect(rRenderContext, mnHighItemId, bFocus, bDrawSel);
    }
}

void ValueSet::ImplDrawSelect(vcl::RenderContext& rRenderContext, sal_uInt16 nItemId, const bool bFocus, const bool bDrawSel )
{
    ValueSetItem* pItem;
    tools::Rectangle aRect;
    if (nItemId)
    {
        const size_t nPos = GetItemPos( nItemId );
        pItem = mItemList[ nPos ].get();
        aRect = ImplGetItemRect( nPos );
    }
    else if (mpNoneItem)
    {
        pItem = mpNoneItem.get();
        aRect = maNoneItemRect;
    }
    else if (bFocus && (pItem = ImplGetFirstItem()))
    {
        aRect = ImplGetItemRect(0);
    }
    else
    {
        return;
    }

    if (!pItem->mbVisible)
        return;

    // draw selection
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetFillColor();

    Color aDoubleColor(rStyleSettings.GetHighlightColor());
    Color aSingleColor(rStyleSettings.GetHighlightTextColor());
    if (!mbDoubleSel)
    {
        /*
        *  #99777# contrast enhancement for thin mode
        */
        const Wallpaper& rWall = GetDisplayBackground();
        if (!rWall.IsBitmap() && ! rWall.IsGradient())
        {
            const Color& rBack = rWall.GetColor();
            if (rBack.IsDark() && ! aDoubleColor.IsBright())
            {
                aDoubleColor = COL_WHITE;
                aSingleColor = COL_BLACK;
            }
            else if (rBack.IsBright() && ! aDoubleColor.IsDark())
            {
                aDoubleColor = COL_BLACK;
                aSingleColor = COL_WHITE;
            }
        }
    }

    // specify selection output
    WinBits nStyle = GetStyle();
    if (nStyle & WB_MENUSTYLEVALUESET)
    {
        if (bFocus)
            ShowFocus(aRect);

        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_BLACK : aDoubleColor);
            rRenderContext.DrawRect(aRect);
        }
    }
    else
    {
        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_BLACK : aDoubleColor);
            rRenderContext.DrawRect(aRect);
        }
        if (mbDoubleSel)
        {
            aRect.AdjustLeft( 1 );
            aRect.AdjustTop( 1 );
            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
            if (bDrawSel)
                rRenderContext.DrawRect(aRect);
        }
        aRect.AdjustLeft( 1 );
        aRect.AdjustTop( 1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );
        tools::Rectangle aRect2 = aRect;
        aRect.AdjustLeft( 1 );
        aRect.AdjustTop( 1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );
        if (bDrawSel)
            rRenderContext.DrawRect(aRect);
        if (mbDoubleSel)
        {
            aRect.AdjustLeft( 1 );
            aRect.AdjustTop( 1 );
            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
            if (bDrawSel)
                rRenderContext.DrawRect(aRect);
        }

        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_WHITE : aSingleColor);
        }
        else
        {
            rRenderContext.SetLineColor(COL_LIGHTGRAY);
        }
        rRenderContext.DrawRect(aRect2);

        if (bFocus)
            ShowFocus(aRect2);
    }

    ImplDrawItemText(rRenderContext, pItem->maText);
}

void ValueSet::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
{
    if ( mnHighItemId == nItemId )
        return;

    mnHighItemId = nItemId;

    // don't draw the selection if nothing is selected
    if ( !bIsSelection && mbNoSelection )
        mbDrawSelection = false;

    // remove the old selection and draw the new one
    Invalidate();
    mbDrawSelection = true;
}

void ValueSet::ImplDraw(vcl::RenderContext& rRenderContext)
{
    Format(rRenderContext);
    HideFocus();

    // draw parting line to the Namefield
    if (GetStyle() & WB_NAMEFIELD)
    {
        if (!(GetStyle() & WB_FLATVALUESET))
        {
            const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
            Size aWinSize(GetOutputSizePixel());
            Point aPos1(NAME_LINE_OFF_X, mnTextOffset + NAME_LINE_OFF_Y);
            Point aPos2(aWinSize.Width() - (NAME_LINE_OFF_X * 2), mnTextOffset + NAME_LINE_OFF_Y);
            if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
            {
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawLine(aPos1, aPos2);
                aPos1.AdjustY( 1 );
                aPos2.AdjustY( 1 );
                rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            }
            else
                rRenderContext.SetLineColor(rStyleSettings.GetWindowTextColor());
            rRenderContext.DrawLine(aPos1, aPos2);
        }
    }

    ImplDrawSelect(rRenderContext);
}

bool ValueSet::ImplScroll(const Point& rPos)
{
    if (!mbScroll || !maItemListRect.IsInside(rPos))
        return false;

    const long nScrollOffset = (mnItemHeight <= 16) ? SCROLL_OFFSET / 2 : SCROLL_OFFSET;
    bool bScroll = false;

    if (rPos.Y() <= maItemListRect.Top() + nScrollOffset)
    {
        if (mnFirstLine > 0)
        {
            --mnFirstLine;
            bScroll = true;
        }
    }
    else if (rPos.Y() >= maItemListRect.Bottom() - nScrollOffset)
    {
        if (mnFirstLine < static_cast<sal_uInt16>(mnLines - mnVisLines))
        {
            ++mnFirstLine;
            bScroll = true;
        }
    }

    if (!bScroll)
        return false;

    mbFormat = true;
    Invalidate();
    return true;
}

size_t ValueSet::ImplGetItem( const Point& rPos ) const
{
    if (!mbHasVisibleItems)
    {
        return VALUESET_ITEM_NOTFOUND;
    }

    if (mpNoneItem.get() && maNoneItemRect.IsInside(rPos))
    {
        return VALUESET_ITEM_NONEITEM;
    }

    if (maItemListRect.IsInside(rPos))
    {
        const int xc = rPos.X() - maItemListRect.Left();
        const int yc = rPos.Y() - maItemListRect.Top();
        // The point is inside the area of item list,
        // let's find the containing item.
        const int col = xc / (mnItemWidth + mnSpacing);
        const int x = xc % (mnItemWidth + mnSpacing);
        const int row = yc / (mnItemHeight + mnSpacing);
        const int y = yc % (mnItemHeight + mnSpacing);

        if (x < mnItemWidth && y < mnItemHeight)
        {
            // the point is inside item rect and not inside spacing
            const size_t item = (mnFirstLine + row) * static_cast<size_t>(mnCols) + col;
            if (item < mItemList.size())
            {
                return item;
            }
        }
    }

    return VALUESET_ITEM_NOTFOUND;
}

ValueSetItem* ValueSet::ImplGetItem( size_t nPos )
{
    if (nPos == VALUESET_ITEM_NONEITEM)
        return mpNoneItem.get();
    else
        return (nPos < mItemList.size()) ? mItemList[nPos].get() : nullptr;
}

ValueSetItem* ValueSet::ImplGetFirstItem()
{
    return !mItemList.empty() ? mItemList[0].get() : nullptr;
}

sal_uInt16 ValueSet::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        if ( mItemList[n]->mbVisible )
            ++nRet;
    }

    return nRet;
}

void ValueSet::ImplFireAccessibleEvent( short nEventId, const Any& rOldValue, const Any& rNewValue )
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( false ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool ValueSet::ImplHasAccessibleListeners()
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( false ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK( ValueSet,ImplScrollHdl, ScrollBar*, pScrollBar, void )
{
    sal_uInt16 nNewFirstLine = static_cast<sal_uInt16>(pScrollBar->GetThumbPos());
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        Invalidate();
    }
}

IMPL_LINK_NOARG(ValueSet, ImplTimerHdl, Timer *, void)
{
    ImplTracking( GetPointerPosPixel(), true );
}

void ValueSet::ImplTracking( const Point& rPos, bool bRepeat )
{
    if ( bRepeat || mbSelection )
    {
        if ( ImplScroll( rPos ) && mbSelection )
        {
            maTimer.SetInvokeHandler( LINK( this, ValueSet, ImplTimerHdl ) );
            maTimer.SetTimeout( MouseSettings::GetScrollRepeat() );
            maTimer.Start();
        }
    }

    ValueSetItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem )
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET || GetStyle() & WB_FLATVALUESET )
            mbHighlight = true;

        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET || GetStyle() & WB_FLATVALUESET )
            mbHighlight = true;

        ImplHighlightItem( mnSelItemId, false );
    }
}

void ValueSet::ImplEndTracking( const Point& rPos, bool bCancel )
{
    ValueSetItem* pItem;

    // restore the old status in case of termination
    if ( bCancel )
        pItem = nullptr;
    else
        pItem = ImplGetItem( ImplGetItem( rPos ) );

    if ( pItem )
    {
        SelectItem( pItem->mnId );
        if ( !mbSelection && !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        mbHighlight = false;
        mbSelection = false;
        Select();
    }
    else
    {
        ImplHighlightItem( mnSelItemId, false );
        mbHighlight = false;
        mbSelection = false;
    }
}

void ValueSet::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    if ( rMouseEvent.IsLeft() )
    {
        ValueSetItem* pItem = ImplGetItem( ImplGetItem( rMouseEvent.GetPosPixel() ) );
        if ( mbSelection )
        {
            mbHighlight = true;
            if ( pItem )
            {
                mnHighItemId = mnSelItemId;
                ImplHighlightItem( pItem->mnId );
            }

            return;
        }
        else
        {
            if ( pItem && !rMouseEvent.IsMod2() )
            {
                if ( rMouseEvent.GetClicks() == 1 )
                {
                    mbHighlight  = true;
                    mnHighItemId = mnSelItemId;
                    ImplHighlightItem( pItem->mnId );
                    StartTracking( StartTrackingFlags::ScrollRepeat );
                }
                else if ( rMouseEvent.GetClicks() == 2 )
                    maDoubleClickHdl.Call( this );

                return;
            }
        }
    }

    Control::MouseButtonDown( rMouseEvent );
}

void ValueSet::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    // because of SelectionMode
    if ( rMouseEvent.IsLeft() && mbSelection )
        ImplEndTracking( rMouseEvent.GetPosPixel(), false );
    else
        Control::MouseButtonUp( rMouseEvent );
}

void ValueSet::MouseMove( const MouseEvent& rMouseEvent )
{
    // because of SelectionMode
    if ( mbSelection || (GetStyle() & WB_MENUSTYLEVALUESET) || (GetStyle() & WB_FLATVALUESET))
        ImplTracking( rMouseEvent.GetPosPixel(), false );
    Control::MouseMove( rMouseEvent );
}

void ValueSet::Tracking( const TrackingEvent& rTrackingEvent )
{
    Point aMousePos = rTrackingEvent.GetMouseEvent().GetPosPixel();

    if ( rTrackingEvent.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTrackingEvent.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTrackingEvent.IsTrackingRepeat() );
}

void ValueSet::KeyInput( const KeyEvent& rKeyEvent )
{
    size_t nLastItem = mItemList.size();

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKeyEvent );
        return;
    }

    if (mbFormat)
        Invalidate();

    --nLastItem;

    const size_t nCurPos
        = mnSelItemId ? GetItemPos(mnSelItemId) : (mpNoneItem ? VALUESET_ITEM_NONEITEM : 0);
    size_t nItemPos = VALUESET_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch (rKeyEvent.GetKeyCode().GetCode())
    {
        case KEY_HOME:
            nItemPos = mpNoneItem ? VALUESET_ITEM_NONEITEM : 0;
            break;

        case KEY_END:
            nItemPos = nLastItem;
            break;

        case KEY_LEFT:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos)
                {
                    nItemPos = nCurPos-1;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
            }
            break;

        case KEY_RIGHT:
            if (nCurPos < nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = 0;
                }
                else
                {
                    nItemPos = nCurPos+1;
                }
            }
            break;

        case KEY_PAGEUP:
            if (rKeyEvent.GetKeyCode().IsShift() || rKeyEvent.GetKeyCode().IsMod1() || rKeyEvent.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKeyEvent );
                return;
            }
            nVStep *= mnVisLines;
            [[fallthrough]];
        case KEY_UP:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos == nLastItem)
                {
                    const size_t nCol = mnCols ? nLastItem % mnCols : 0;
                    if (nCol < mnCurCol)
                    {
                        // Move to previous row/page, keeping the old column
                        nVStep -= mnCurCol - nCol;
                    }
                }
                if (nCurPos >= nVStep)
                {
                    // Go up of a whole page
                    nItemPos = nCurPos-nVStep;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
                else if (nCurPos > mnCols)
                {
                    // Go to same column in first row
                    nItemPos = nCurPos % mnCols;
                }
            }
            break;

        case KEY_PAGEDOWN:
            if (rKeyEvent.GetKeyCode().IsShift() || rKeyEvent.GetKeyCode().IsMod1() || rKeyEvent.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKeyEvent );
                return;
            }
            nVStep *= mnVisLines;
            [[fallthrough]];
        case KEY_DOWN:
            if (nCurPos != nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = nVStep-mnCols+mnCurCol;
                }
                else
                {
                    nItemPos = nCurPos+nVStep;
                }
                if (nItemPos > nLastItem)
                {
                    nItemPos = nLastItem;
                }
            }
            break;

        case KEY_RETURN:
            if (GetStyle() & WB_NO_DIRECTSELECT)
            {
                Select();
                break;
            }
            [[fallthrough]];
        default:
            Control::KeyInput( rKeyEvent );
            return;
    }

    // This point is reached only if key travelling was used,
    // in which case selection mode should be switched off
    EndSelection();

    if ( nItemPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nItemPos!=VALUESET_ITEM_NONEITEM && nItemPos<nLastItem )
    {
        // update current column only in case of a new position
        // which is also not a "specially" handled one.
        mnCurCol = mnCols ? nItemPos % mnCols : 0;
    }
    const sal_uInt16 nItemId = (nItemPos != VALUESET_ITEM_NONEITEM) ? GetItemId( nItemPos ) : 0;
    if ( nItemId != mnSelItemId )
    {
        SelectItem( nItemId );
        if (!(GetStyle() & WB_NO_DIRECTSELECT))
        {
            // select only if WB_NO_DIRECTSELECT is not set
            Select();
        }
    }
}

void ValueSet::Command( const CommandEvent& rCommandEvent )
{
    if ( rCommandEvent.GetCommand() == CommandEventId::Wheel ||
         rCommandEvent.GetCommand() == CommandEventId::StartAutoScroll ||
         rCommandEvent.GetCommand() == CommandEventId::AutoScroll )
    {
        if ( HandleScrollCommand( rCommandEvent, nullptr, mxScrollBar.get() ) )
            return;
    }

    Control::Command( rCommandEvent );
}

void ValueSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (GetStyle() & WB_FLATVALUESET)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
    }

    ImplDraw(rRenderContext);
}

void ValueSet::GetFocus()
{
    SAL_INFO("svtools", "value set getting focus");
    Invalidate();
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(GetAccessible(false));
    if (pAcc)
        pAcc->GetFocus();
}

void ValueSet::LoseFocus()
{
    SAL_INFO("svtools", "value set losing focus");
    if (!mbNoSelection || !mnSelItemId)
        HideFocus();
    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( false ) );
    if( pAcc )
        pAcc->LoseFocus();
}

void ValueSet::Resize()
{
    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    Control::Resize();
}

void ValueSet::RequestHelp( const HelpEvent& rHelpEvent )
{
    if (rHelpEvent.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON))
    {
        Point aPos = ScreenToOutputPixel( rHelpEvent.GetMousePosPixel() );
        size_t nItemPos = ImplGetItem( aPos );
        if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            tools::Rectangle aItemRect = ImplGetItemRect( nItemPos );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.SetLeft( aPt.X() );
            aItemRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.SetRight( aPt.X() );
            aItemRect.SetBottom( aPt.Y() );
            Help::ShowQuickHelp( this, aItemRect, GetItemText( ImplGetItem( nItemPos )->mnId ) );
            return;
        }
    }

    Control::RequestHelp( rHelpEvent );
}

void ValueSet::StateChanged(StateChangedType nType)
{
    Control::StateChanged(nType);

    if (nType == StateChangedType::InitShow)
    {
        if (mbFormat)
            Invalidate();
    }
    else if (nType == StateChangedType::UpdateMode)
    {
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
    }
    else if (nType == StateChangedType::Text)
    {
        if (mpNoneItem.get() && !mbFormat && IsReallyVisible() && IsUpdateMode())
        {
            Invalidate(maNoneItemRect);
        }
    }
    else if ((nType == StateChangedType::Zoom) ||
             (nType == StateChangedType::ControlFont))
    {
        ImplInitSettings(true, false, false);
        Invalidate();
    }
    else if (nType == StateChangedType::ControlForeground)
    {
        ImplInitSettings(false, true, false);
        Invalidate();
    }
    else if (nType == StateChangedType::ControlBackground)
    {
        ImplInitSettings(false, false, true);
        Invalidate();
    }
    else if ((nType == StateChangedType::Style) || (nType == StateChangedType::Enable))
    {
        mbFormat = true;
        ImplInitSettings(false, false, true);
        Invalidate();
    }
}

void ValueSet::DataChanged( const DataChangedEvent& rDataChangedEvent )
{
    Control::DataChanged( rDataChangedEvent );

    if ( rDataChangedEvent.GetType() == DataChangedEventType::FONTS ||
         rDataChangedEvent.GetType() == DataChangedEventType::DISPLAY ||
         rDataChangedEvent.GetType() == DataChangedEventType::FONTSUBSTITUTION ||
         (rDataChangedEvent.GetType() == DataChangedEventType::SETTINGS &&
          rDataChangedEvent.GetFlags() & AllSettingsFlags::STYLE) )
    {
        mbFormat = true;
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

void ValueSet::Select()
{
    maSelectHdl.Call( this );
}

void ValueSet::UserDraw( const UserDrawEvent& )
{
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage )
{
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    ImplInsertItem( std::move(pItem), VALUESET_APPEND );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const OUString& rText, size_t nPos,
                           bool bShowLegend )
{
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = bShowLegend ? VALUESETITEM_IMAGE_AND_TEXT : VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), nPos );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor,
                           const OUString& rText )
{
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), VALUESET_APPEND );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    ImplInsertItem( std::move(pItem), nPos );
}

void ValueSet::ImplInsertItem( std::unique_ptr<ValueSetItem> pItem, const size_t nPos )
{
    DBG_ASSERT( pItem->mnId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( pItem->mnId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    if ( nPos < mItemList.size() ) {
        mItemList.insert( mItemList.begin() + nPos, std::move(pItem) );
    } else {
        mItemList.push_back( std::move(pItem) );
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

tools::Rectangle ValueSet::ImplGetItemRect( size_t nPos ) const
{
    const size_t nVisibleBegin = static_cast<size_t>(mnFirstLine)*mnCols;
    const size_t nVisibleEnd = nVisibleBegin + static_cast<size_t>(mnVisLines)*mnCols;

    // Check if the item is inside the range of the displayed ones,
    // taking into account that last row could be incomplete
    if ( nPos<nVisibleBegin || nPos>=nVisibleEnd || nPos>=mItemList.size() )
        return tools::Rectangle();

    nPos -= nVisibleBegin;

    const size_t row = mnCols ? nPos/mnCols : 0;
    const size_t col = mnCols ? nPos%mnCols : 0;
    const long x = maItemListRect.Left()+col*(mnItemWidth+mnSpacing);
    const long y = maItemListRect.Top()+row*(mnItemHeight+mnSpacing);

    return tools::Rectangle( Point(x, y), Size(mnItemWidth, mnItemHeight) );
}

void ValueSet::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        mItemList.erase( mItemList.begin() + nPos );
    }

    // reset variables
    if ( (mnHighItemId == nItemId) || (mnSelItemId == nItemId) )
    {
        mnCurCol        = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = true;
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ValueSet::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = true;

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

size_t ValueSet::GetItemCount() const
{
    return mItemList.size();
}

size_t ValueSet::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mItemList.size(); i < n; ++i ) {
        if ( mItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return VALUESET_ITEM_NOTFOUND;
}

sal_uInt16 ValueSet::GetItemId( size_t nPos ) const
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos]->mnId : 0 ;
}

sal_uInt16 ValueSet::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

tools::Rectangle ValueSet::GetItemRect( sal_uInt16 nItemId ) const
{
    const size_t nPos = GetItemPos( nItemId );

    if ( nPos!=VALUESET_ITEM_NOTFOUND && mItemList[nPos]->mbVisible )
        return ImplGetItemRect( nPos );

    return tools::Rectangle();
}

void ValueSet::EnableFullItemMode( bool bFullMode )
{
    mbFullMode = bFullMode;
}

void ValueSet::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ValueSet::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ValueSet::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ValueSet::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

/**
 * An inelegant method; sets the item width & height such that
 * all of the included items and their labels fit; if we can
 * calculate that.
 */
void ValueSet::RecalculateItemSizes()
{
    Size aLargestItem = GetLargestItemSize();

    if ( mnUserItemWidth != aLargestItem.Width() ||
         mnUserItemHeight != aLargestItem.Height() )
    {
        mnUserItemWidth = aLargestItem.Width();
        mnUserItemHeight = aLargestItem.Height();
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ValueSet::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == VALUESET_ITEM_NOTFOUND )
            return;
    }

    if ( !((mnSelItemId != nItemId) || mbNoSelection) )
        return;

    const sal_uInt16 nOldItem = mnSelItemId;
    mnSelItemId = nItemId;
    mbNoSelection = false;

    bool bNewOut = !mbFormat && IsReallyVisible() && IsUpdateMode();
    bool bNewLine = false;

    // if necessary scroll to the visible area
    if (mbScroll && mnCols)
    {
        sal_uInt16 nNewLine = static_cast<sal_uInt16>(nItemPos / mnCols);
        if ( nNewLine < mnFirstLine )
        {
            mnFirstLine = nNewLine;
            bNewLine = true;
        }
        else if ( nNewLine > static_cast<sal_uInt16>(mnFirstLine+mnVisLines-1) )
        {
            mnFirstLine = static_cast<sal_uInt16>(nNewLine-mnVisLines+1);
            bNewLine = true;
        }
    }

    if ( bNewOut )
    {
        if ( bNewLine )
        {
            // redraw everything if the visible area has changed
            mbFormat = true;
            Invalidate();
        }
        else
        {
            // remove old selection and draw the new one
            Invalidate();
        }
    }

    if( ImplHasAccessibleListeners() )
    {
        // focus event (deselect)
        if( nOldItem )
        {
            const size_t nPos = GetItemPos( nItemId );

            if( nPos != VALUESET_ITEM_NOTFOUND )
            {
                ValueItemAcc* pItemAcc = ValueItemAcc::getImplementation(
                    mItemList[nPos]->GetAccessible( false/*bIsTransientChildrenDisabled*/ ) );

                if( pItemAcc )
                {
                    Any aOldAny;
                    Any aNewAny;
                    aOldAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
                    ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
            }
        }

        // focus event (select)
        const size_t nPos = GetItemPos( mnSelItemId );

        ValueSetItem* pItem;
        if( nPos != VALUESET_ITEM_NOTFOUND )
            pItem = mItemList[nPos].get();
        else
            pItem = mpNoneItem.get();

        ValueItemAcc* pItemAcc = nullptr;
        if (pItem != nullptr)
            pItemAcc = ValueItemAcc::getImplementation( pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ ) );

        if( pItemAcc )
        {
            Any aOldAny;
            Any aNewAny;
            aNewAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
            ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny);
        }

        // selection event
        Any aOldAny;
        Any aNewAny;
        ImplFireAccessibleEvent(AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny);
    }
    maHighlightHdl.Call(this);
}

void ValueSet::SetNoSelection()
{
    mbNoSelection   = true;
    mbHighlight     = false;
    mbSelection     = false;

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void ValueSet::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos].get();
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const tools::Rectangle aRect = ImplGetItemRect(nPos);
        Invalidate(aRect);
    }
    else
        mbFormat = true;
}

Image ValueSet::GetItemImage( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maImage;
    else
        return Image();
}

void ValueSet::SetItemColor( sal_uInt16 nItemId, const Color& rColor )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos].get();
    pItem->meType  = VALUESETITEM_COLOR;
    pItem->maColor = rColor;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const tools::Rectangle aRect = ImplGetItemRect(nPos);
        Invalidate( aRect );
    }
    else
        mbFormat = true;
}

Color ValueSet::GetItemColor( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maColor;
    else
        return Color();
}

void ValueSet::SetItemData( sal_uInt16 nItemId, void* pData )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos].get();
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            const tools::Rectangle aRect = ImplGetItemRect(nPos);
            Invalidate(aRect);
        }
        else
            mbFormat = true;
    }
}

void* ValueSet::GetItemData( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->mpData;
    else
        return nullptr;
}

void ValueSet::SetItemText(sal_uInt16 nItemId, const OUString& rText)
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;


    ValueSetItem* pItem = mItemList[nPos].get();
    if (pItem->maText == rText)
        return;
    // Remember old and new name for accessibility event.
    Any aOldName;
    Any aNewName;
    aOldName <<= pItem->maText;
    aNewName <<= rText;

    pItem->maText = rText;

    if (!mbFormat && IsReallyVisible() && IsUpdateMode())
    {
        sal_uInt16 nTempId = mnSelItemId;

        if (mbHighlight)
            nTempId = mnHighItemId;

        if (nTempId == nItemId)
            Invalidate();
    }

    if (ImplHasAccessibleListeners())
    {
        Reference<XAccessible> xAccessible(pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/));
        ValueItemAcc* pValueItemAcc = static_cast<ValueItemAcc*>(xAccessible.get());
        pValueItemAcc->FireAccessibleEvent(AccessibleEventId::NAME_CHANGED, aOldName, aNewName);
    }
}

OUString ValueSet::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return OUString();
}

void ValueSet::SetColor( const Color& rColor )
{
    maColor  = rColor;
    mbFormat = true;
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void ValueSet::SetExtraSpacing( sal_uInt16 nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;

        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ValueSet::StartSelection()
{
    mbHighlight  = true;
    mbSelection  = true;
    mnHighItemId = mnSelItemId;
}

void ValueSet::EndSelection()
{
    if ( mbHighlight )
    {
        if ( IsTracking() )
            EndTracking( TrackingEventFlags::Cancel );

        ImplHighlightItem( mnSelItemId );
        mbHighlight = false;
    }
    mbSelection = false;
}

void ValueSet::SetFormat()
{
    mbFormat = true;
}

void ValueSet::StartDrag( const CommandEvent& rEvent, vcl::Region& rRegion )
{
    if ( rEvent.GetCommand() != CommandEventId::StartDrag )
        return;

    // if necessary abort an existing action
    EndSelection();

    // Check out if the clicked on page is selected. If this is not the
    // case set it as the current item. We only check mouse actions since
    // drag-and-drop can also be triggered by the keyboard
    sal_uInt16 nSelId;
    if ( rEvent.IsMouseEvent() )
        nSelId = GetItemId( rEvent.GetMousePosPixel() );
    else
        nSelId = mnSelItemId;

    // don't activate dragging if no item was clicked on
    if ( !nSelId )
        return;

    // Check out if the page was selected. If not set as current page and
    // call select.
    if ( nSelId != mnSelItemId )
    {
        SelectItem( nSelId );
        Update();
        Select();
    }

    vcl::Region aRegion;

    // assign region
    rRegion = aRegion;
}

Size ValueSet::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines ) const
{
    size_t nCalcCols = nDesireCols;
    size_t nCalcLines = nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = mnUserCols;
        else
            nCalcCols = 1;
    }

    if ( !nCalcLines )
    {
        nCalcLines = mnVisLines;

        if ( mbFormat )
        {
            if ( mnUserVisLines )
                nCalcLines = mnUserVisLines;
            else
            {
                // Floor( (M+N-1)/N )==Ceiling( M/N )
                nCalcLines = (mItemList.size()+nCalcCols-1) / nCalcCols;
                if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width() * nCalcCols, rItemSize.Height() * nCalcLines );
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        n;

    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.AdjustWidth(n * nCalcCols );
        aSize.AdjustHeight(n * nCalcLines );
    }
    else
        n = 0;

    if ( mnSpacing )
    {
        aSize.AdjustWidth(mnSpacing * (nCalcCols - 1) );
        aSize.AdjustHeight(mnSpacing * (nCalcLines - 1) );
    }

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.AdjustHeight(nTxtHeight + NAME_OFFSET );
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.AdjustHeight(NAME_LINE_HEIGHT + NAME_LINE_OFF_Y );
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.AdjustHeight(nTxtHeight + n + mnSpacing );
    }

    // sum possible ScrollBar width
    aSize.AdjustWidth(GetScrollWidth() );

    return aSize;
}

Size ValueSet::CalcItemSizePixel( const Size& rItemSize) const
{
    Size aSize = rItemSize;

    WinBits nStyle = GetStyle();
    if ( nStyle & WB_ITEMBORDER )
    {
        long n;

        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.AdjustWidth(n );
        aSize.AdjustHeight(n );
    }

    return aSize;
}

long ValueSet::GetScrollWidth() const
{
    if (GetStyle() & WB_VSCROLL)
    {
        ValueSet* pValueSet = const_cast<ValueSet*>(this);
        if (!mxScrollBar)
        {
            pValueSet->ImplInitScrollBar();
        }
        pValueSet->Invalidate();
        return mxScrollBar->GetSizePixel().Width() + SCRBAR_OFFSET;
    }
    else
        return 0;
}

void ValueSet::SetHighlightHdl( const Link<ValueSet*,void>& rLink )
{
    maHighlightHdl = rLink;
}

Size ValueSet::GetLargestItemSize()
{
    Size aLargestItem;

    for (std::unique_ptr<ValueSetItem>& pItem : mItemList)
    {
        if (!pItem->mbVisible)
            continue;

        if (pItem->meType != VALUESETITEM_IMAGE &&
            pItem->meType != VALUESETITEM_IMAGE_AND_TEXT)
        {
            // handle determining an optimal size for this case
            continue;
        }

        Size aSize = pItem->maImage.GetSizePixel();
        if (pItem->meType == VALUESETITEM_IMAGE_AND_TEXT)
        {
            aSize.AdjustHeight(3 * NAME_LINE_HEIGHT + GetTextHeight());
            aSize.setWidth( std::max(aSize.Width(),
                                     GetTextWidth(pItem->maText) + NAME_OFFSET) );
        }

        aLargestItem.setWidth( std::max(aLargestItem.Width(), aSize.Width()) );
        aLargestItem.setHeight( std::max(aLargestItem.Height(), aSize.Height()) );
    }

    return aLargestItem;
}

Size ValueSet::GetOptimalSize() const
{
    return CalcWindowSizePixel(
        const_cast<ValueSet *>(this)->GetLargestItemSize());
}

void ValueSet::SetEdgeBlending(bool bNew)
{
    if(mbEdgeBlending != bNew)
    {
        mbEdgeBlending = bNew;
        mbFormat = true;

        if(IsReallyVisible() && IsUpdateMode())
        {
            Invalidate();
        }
    }
}

SvtValueSet::SvtValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow)
    : maVirDev( VclPtr<VirtualDevice>::Create())
    , mxScrolledWindow(std::move(pScrolledWindow))
    , mnHighItemId(0)
    , maColor(COL_TRANSPARENT)
    , mnStyle(0)
    , mbFormat(true)
    , mbHighlight(false)
{
    maVirDev->SetBackground(Application::GetSettings().GetStyleSettings().GetFaceColor());

    mpNoneItem.reset(nullptr);

    mnItemWidth         = 0;
    mnItemHeight        = 0;
    mnTextOffset        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnSelItemId         = 0;
    mnSavedItemId       = -1;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = DrawFrameStyle::NONE;
    mbNoSelection       = true;
    mbDrawSelection     = true;
    mbBlackSel          = false;
    mbDoubleSel         = false;
    mbScroll            = false;
    mbEdgeBlending      = false;
    mbHasVisibleItems   = false;

    if (mxScrolledWindow)
    {
        mxScrolledWindow->set_user_managed_scrolling();
        mxScrolledWindow->connect_vadjustment_changed(LINK(this, SvtValueSet, ImplScrollHdl));
    }
}

void SvtValueSet::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    // #106446#, #106601# force mirroring of virtual device
    maVirDev->EnableRTL(pDrawingArea->get_direction());
}

Reference<XAccessible> SvtValueSet::CreateAccessible()
{
    if (!mxAccessible)
        mxAccessible.set(new SvtValueSetAcc(this));
    return mxAccessible;
}

SvtValueSet::~SvtValueSet()
{
    Reference<XComponent> xComponent(mxAccessible, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    ImplDeleteItems();
}

void SvtValueSet::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        SvtValueSetItem* pItem = mItemList[i].get();
        if ( pItem->mbVisible && ImplHasAccessibleListeners() )
        {
            Any aOldAny;
            Any aNewAny;

            aOldAny <<= pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ );
            ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
        }

        mItemList[i].reset();
    }

    mItemList.clear();
}

void SvtValueSet::Select()
{
    maSelectHdl.Call( this );
}

void SvtValueSet::UserDraw( const UserDrawEvent& )
{
}

size_t SvtValueSet::ImplGetItem( const Point& rPos ) const
{
    if (!mbHasVisibleItems)
    {
        return VALUESET_ITEM_NOTFOUND;
    }

    if (mpNoneItem.get() && maNoneItemRect.IsInside(rPos))
    {
        return VALUESET_ITEM_NONEITEM;
    }

    if (maItemListRect.IsInside(rPos))
    {
        const int xc = rPos.X() - maItemListRect.Left();
        const int yc = rPos.Y() - maItemListRect.Top();
        // The point is inside the area of item list,
        // let's find the containing item.
        const int col = xc / (mnItemWidth + mnSpacing);
        const int x = xc % (mnItemWidth + mnSpacing);
        const int row = yc / (mnItemHeight + mnSpacing);
        const int y = yc % (mnItemHeight + mnSpacing);

        if (x < mnItemWidth && y < mnItemHeight)
        {
            // the point is inside item rect and not inside spacing
            const size_t item = (mnFirstLine + row) * static_cast<size_t>(mnCols) + col;
            if (item < mItemList.size())
            {
                return item;
            }
        }
    }

    return VALUESET_ITEM_NOTFOUND;
}

SvtValueSetItem* SvtValueSet::ImplGetItem( size_t nPos )
{
    if (nPos == VALUESET_ITEM_NONEITEM)
        return mpNoneItem.get();
    else
        return (nPos < mItemList.size()) ? mItemList[nPos].get() : nullptr;
}

SvtValueSetItem* SvtValueSet::ImplGetFirstItem()
{
    return !mItemList.empty() ? mItemList[0].get() : nullptr;
}

sal_uInt16 SvtValueSet::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        if ( mItemList[n]->mbVisible )
            ++nRet;
    }

    return nRet;
}

void SvtValueSet::ImplFireAccessibleEvent( short nEventId, const Any& rOldValue, const Any& rNewValue )
{
    SvtValueSetAcc* pAcc = SvtValueSetAcc::getImplementation(mxAccessible);

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool SvtValueSet::ImplHasAccessibleListeners()
{
    SvtValueSetAcc* pAcc = SvtValueSetAcc::getImplementation(mxAccessible);
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK(SvtValueSet, ImplScrollHdl, weld::ScrolledWindow&, rScrollWin, void)
{
    auto nNewFirstLine = rScrollWin.vadjustment_get_value();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        Invalidate();
    }
}

void SvtValueSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (GetStyle() & WB_FLATVALUESET)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        long nOffY = maVirDev->GetOutputSizePixel().Height();
        Size aWinSize(GetOutputSizePixel());
        rRenderContext.DrawRect(tools::Rectangle(Point(0, nOffY ), Point( aWinSize.Width(), aWinSize.Height())));
    }

    ImplDraw(rRenderContext);
}

void SvtValueSet::GetFocus()
{
    SAL_INFO("svtools", "value set getting focus");
    Invalidate();
    CustomWidgetController::GetFocus();

    // Tell the accessible object that we got the focus.
    SvtValueSetAcc* pAcc = SvtValueSetAcc::getImplementation(mxAccessible);
    if (pAcc)
        pAcc->GetFocus();
}

void SvtValueSet::LoseFocus()
{
    SAL_INFO("svtools", "value set losing focus");
    Invalidate();
    CustomWidgetController::LoseFocus();

    // Tell the accessible object that we lost the focus.
    SvtValueSetAcc* pAcc = SvtValueSetAcc::getImplementation(mxAccessible);
    if( pAcc )
        pAcc->LoseFocus();
}

void SvtValueSet::Resize()
{
    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    CustomWidgetController::Resize();
}

bool SvtValueSet::KeyInput( const KeyEvent& rKeyEvent )
{
    size_t nLastItem = mItemList.size();

    if ( !nLastItem || !ImplGetFirstItem() )
        return CustomWidgetController::KeyInput(rKeyEvent);

    if (mbFormat)
        Invalidate();

    --nLastItem;

    const size_t nCurPos
        = mnSelItemId ? GetItemPos(mnSelItemId) : (mpNoneItem ? VALUESET_ITEM_NONEITEM : 0);
    size_t nItemPos = VALUESET_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch (rKeyEvent.GetKeyCode().GetCode())
    {
        case KEY_HOME:
            nItemPos = mpNoneItem ? VALUESET_ITEM_NONEITEM : 0;
            break;

        case KEY_END:
            nItemPos = nLastItem;
            break;

        case KEY_LEFT:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos)
                {
                    nItemPos = nCurPos-1;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
            }
            break;

        case KEY_RIGHT:
            if (nCurPos < nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = 0;
                }
                else
                {
                    nItemPos = nCurPos+1;
                }
            }
            break;

        case KEY_PAGEUP:
            if (rKeyEvent.GetKeyCode().IsShift() || rKeyEvent.GetKeyCode().IsMod1() || rKeyEvent.GetKeyCode().IsMod2())
            {
                return CustomWidgetController::KeyInput(rKeyEvent);
            }
            nVStep *= mnVisLines;
            [[fallthrough]];
        case KEY_UP:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos == nLastItem)
                {
                    const size_t nCol = mnCols ? nLastItem % mnCols : 0;
                    if (nCol < mnCurCol)
                    {
                        // Move to previous row/page, keeping the old column
                        nVStep -= mnCurCol - nCol;
                    }
                }
                if (nCurPos >= nVStep)
                {
                    // Go up of a whole page
                    nItemPos = nCurPos-nVStep;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
                else if (nCurPos > mnCols)
                {
                    // Go to same column in first row
                    nItemPos = nCurPos % mnCols;
                }
            }
            break;

        case KEY_PAGEDOWN:
            if (rKeyEvent.GetKeyCode().IsShift() || rKeyEvent.GetKeyCode().IsMod1() || rKeyEvent.GetKeyCode().IsMod2())
            {
                return CustomWidgetController::KeyInput(rKeyEvent);
            }
            nVStep *= mnVisLines;
            [[fallthrough]];
        case KEY_DOWN:
            if (nCurPos != nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = nVStep-mnCols+mnCurCol;
                }
                else
                {
                    nItemPos = nCurPos+nVStep;
                }
                if (nItemPos > nLastItem)
                {
                    nItemPos = nLastItem;
                }
            }
            break;

        case KEY_RETURN:
            if (GetStyle() & WB_NO_DIRECTSELECT)
            {
                Select();
                break;
            }
            [[fallthrough]];
        default:
            return CustomWidgetController::KeyInput(rKeyEvent);
    }

    if ( nItemPos == VALUESET_ITEM_NOTFOUND )
        return true;

    if ( nItemPos!=VALUESET_ITEM_NONEITEM && nItemPos<nLastItem )
    {
        // update current column only in case of a new position
        // which is also not a "specially" handled one.
        mnCurCol = mnCols ? nItemPos % mnCols : 0;
    }
    const sal_uInt16 nItemId = (nItemPos != VALUESET_ITEM_NONEITEM) ? GetItemId( nItemPos ) : 0;
    if ( nItemId != mnSelItemId )
    {
        SelectItem( nItemId );
        if (!(GetStyle() & WB_NO_DIRECTSELECT))
        {
            // select only if WB_NO_DIRECTSELECT is not set
            Select();
        }
    }

    return true;
}

void SvtValueSet::ImplTracking(const Point& rPos)
{
    SvtValueSetItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem )
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET || GetStyle() & WB_FLATVALUESET )
            mbHighlight = true;

        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET || GetStyle() & WB_FLATVALUESET )
            mbHighlight = true;

        ImplHighlightItem( mnSelItemId, false );
    }
}

bool SvtValueSet::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    if ( rMouseEvent.IsLeft() )
    {
        SvtValueSetItem* pItem = ImplGetItem( ImplGetItem( rMouseEvent.GetPosPixel() ) );
        if (pItem && !rMouseEvent.IsMod2())
        {
            if (rMouseEvent.GetClicks() == 1)
            {
                SelectItem( pItem->mnId );
                if (!(GetStyle() & WB_NOPOINTERFOCUS))
                    GrabFocus();
                Select();
            }
            else if ( rMouseEvent.GetClicks() == 2 )
                maDoubleClickHdl.Call( this );

            return true;
        }
    }

    return CustomWidgetController::MouseButtonDown( rMouseEvent );
}

bool SvtValueSet::MouseMove(const MouseEvent& rMouseEvent)
{
    // because of SelectionMode
    if ((GetStyle() & WB_MENUSTYLEVALUESET) || (GetStyle() & WB_FLATVALUESET))
        ImplTracking(rMouseEvent.GetPosPixel());
    return CustomWidgetController::MouseMove(rMouseEvent);
}

void SvtValueSet::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        mItemList.erase( mItemList.begin() + nPos );
    }

    // reset variables
    if (mnHighItemId == nItemId || mnSelItemId == nItemId)
    {
        mnCurCol        = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = true;
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void SvtValueSet::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = true;

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

size_t SvtValueSet::GetItemCount() const
{
    return mItemList.size();
}

size_t SvtValueSet::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mItemList.size(); i < n; ++i ) {
        if ( mItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return VALUESET_ITEM_NOTFOUND;
}

sal_uInt16 SvtValueSet::GetItemId( size_t nPos ) const
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos]->mnId : 0 ;
}

sal_uInt16 SvtValueSet::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

tools::Rectangle SvtValueSet::GetItemRect( sal_uInt16 nItemId ) const
{
    const size_t nPos = GetItemPos( nItemId );

    if ( nPos!=VALUESET_ITEM_NOTFOUND && mItemList[nPos]->mbVisible )
        return ImplGetItemRect( nPos );

    return tools::Rectangle();
}

tools::Rectangle SvtValueSet::ImplGetItemRect( size_t nPos ) const
{
    const size_t nVisibleBegin = static_cast<size_t>(mnFirstLine)*mnCols;
    const size_t nVisibleEnd = nVisibleBegin + static_cast<size_t>(mnVisLines)*mnCols;

    // Check if the item is inside the range of the displayed ones,
    // taking into account that last row could be incomplete
    if ( nPos<nVisibleBegin || nPos>=nVisibleEnd || nPos>=mItemList.size() )
        return tools::Rectangle();

    nPos -= nVisibleBegin;

    const size_t row = mnCols ? nPos/mnCols : 0;
    const size_t col = mnCols ? nPos%mnCols : 0;
    const long x = maItemListRect.Left()+col*(mnItemWidth+mnSpacing);
    const long y = maItemListRect.Top()+row*(mnItemHeight+mnSpacing);

    return tools::Rectangle( Point(x, y), Size(mnItemWidth, mnItemHeight) );
}

void SvtValueSet::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
{
    if ( mnHighItemId == nItemId )
        return;

    // remember the old item to delete the previous selection
    mnHighItemId = nItemId;

    // don't draw the selection if nothing is selected
    if ( !bIsSelection && mbNoSelection )
        mbDrawSelection = false;

    // remove the old selection and draw the new one
    Invalidate();
    mbDrawSelection = true;
}

void SvtValueSet::ImplDraw(vcl::RenderContext& rRenderContext)
{
    if (mbFormat)
        Format(rRenderContext);

    Point aDefPos;
    Size aSize = maVirDev->GetOutputSizePixel();

    rRenderContext.DrawOutDev(aDefPos, aSize, aDefPos, aSize, *maVirDev);

    // draw parting line to the Namefield
    if (GetStyle() & WB_NAMEFIELD)
    {
        if (!(GetStyle() & WB_FLATVALUESET))
        {
            const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
            Size aWinSize(GetOutputSizePixel());
            Point aPos1(NAME_LINE_OFF_X, mnTextOffset + NAME_LINE_OFF_Y);
            Point aPos2(aWinSize.Width() - (NAME_LINE_OFF_X * 2), mnTextOffset + NAME_LINE_OFF_Y);
            if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
            {
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawLine(aPos1, aPos2);
                aPos1.AdjustY( 1 );
                aPos2.AdjustY( 1 );
                rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            }
            else
                rRenderContext.SetLineColor(rStyleSettings.GetWindowTextColor());
            rRenderContext.DrawLine(aPos1, aPos2);
        }
    }

    ImplDrawSelect(rRenderContext);
}

void SvtValueSet::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == VALUESET_ITEM_NOTFOUND )
            return;
    }

    if ( !((mnSelItemId != nItemId) || mbNoSelection) )
        return;

    const sal_uInt16 nOldItem = mnSelItemId;
    mnSelItemId = nItemId;
    mbNoSelection = false;

    bool bNewOut = !mbFormat && IsReallyVisible() && IsUpdateMode();
    bool bNewLine = false;

    // if necessary scroll to the visible area
    if (mbScroll && nItemId && mnCols)
    {
        sal_uInt16 nNewLine = static_cast<sal_uInt16>(nItemPos / mnCols);
        if ( nNewLine < mnFirstLine )
        {
            mnFirstLine = nNewLine;
            bNewLine = true;
        }
        else if ( nNewLine > static_cast<sal_uInt16>(mnFirstLine+mnVisLines-1) )
        {
            mnFirstLine = static_cast<sal_uInt16>(nNewLine-mnVisLines+1);
            bNewLine = true;
        }
    }

    if ( bNewOut )
    {
        if ( bNewLine )
        {
            // redraw everything if the visible area has changed
            mbFormat = true;
        }
        Invalidate();
    }

    if( ImplHasAccessibleListeners() )
    {
        // focus event (deselect)
        if( nOldItem )
        {
            const size_t nPos = GetItemPos( nItemId );

            if( nPos != VALUESET_ITEM_NOTFOUND )
            {
                SvtValueItemAcc* pItemAcc = SvtValueItemAcc::getImplementation(
                    mItemList[nPos]->GetAccessible( false/*bIsTransientChildrenDisabled*/ ) );

                if( pItemAcc )
                {
                    Any aOldAny;
                    Any aNewAny;
                    aOldAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
                    ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
            }
        }

        // focus event (select)
        const size_t nPos = GetItemPos( mnSelItemId );

        SvtValueSetItem* pItem;
        if( nPos != VALUESET_ITEM_NOTFOUND )
            pItem = mItemList[nPos].get();
        else
            pItem = mpNoneItem.get();

        SvtValueItemAcc* pItemAcc = nullptr;
        if (pItem != nullptr)
            pItemAcc = SvtValueItemAcc::getImplementation( pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/ ) );

        if( pItemAcc )
        {
            Any aOldAny;
            Any aNewAny;
            aNewAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
            ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny);
        }

        // selection event
        Any aOldAny;
        Any aNewAny;
        ImplFireAccessibleEvent(AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny);
    }
}

void SvtValueSet::SetNoSelection()
{
    mbNoSelection   = true;
    mbHighlight     = false;

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void SvtValueSet::SetStyle(WinBits nStyle)
{
    if (nStyle != mnStyle)
    {
        mnStyle = nStyle;
        mbFormat = true;
        Invalidate();
    }
}

void SvtValueSet::Format(vcl::RenderContext const & rRenderContext)
{
    Size aWinSize(GetOutputSizePixel());
    size_t nItemCount = mItemList.size();
    WinBits nStyle = GetStyle();
    long nTxtHeight = rRenderContext.GetTextHeight();
    long nOff;
    long nNoneHeight;
    long nNoneSpace;

    if (mxScrolledWindow && !(nStyle & WB_VSCROLL) && mxScrolledWindow->get_vpolicy() != VclPolicyType::NEVER)
    {
        mxScrolledWindow->set_vpolicy(VclPolicyType::NEVER);
        Size aPrefSize(GetDrawingArea()->get_preferred_size());
        GetDrawingArea()->set_size_request(aPrefSize.Width() + GetScrollWidth(), aPrefSize.Height());
    }

    // calculate item offset
    if (nStyle & WB_ITEMBORDER)
    {
        if (nStyle & WB_DOUBLEBORDER)
            nOff = ITEM_OFFSET_DOUBLE;
        else
            nOff = ITEM_OFFSET;
    }
    else
        nOff = 0;

    // consider size, if NameField does exist
    if (nStyle & WB_NAMEFIELD)
    {
        mnTextOffset = aWinSize.Height() - nTxtHeight - NAME_OFFSET;
        aWinSize.AdjustHeight( -(nTxtHeight + NAME_OFFSET) );

        if (!(nStyle & WB_FLATVALUESET))
        {
            mnTextOffset -= NAME_LINE_HEIGHT + NAME_LINE_OFF_Y;
            aWinSize.AdjustHeight( -(NAME_LINE_HEIGHT + NAME_LINE_OFF_Y) );
        }
    }
    else
        mnTextOffset = 0;

    // consider offset and size, if NoneField does exist
    if (nStyle & WB_NONEFIELD)
    {
        nNoneHeight = nTxtHeight + nOff;
        nNoneSpace = mnSpacing;
    }
    else
    {
        nNoneHeight = 0;
        nNoneSpace = 0;

        if (mpNoneItem)
            mpNoneItem.reset(nullptr);
    }

    // calculate number of columns
    if (!mnUserCols)
    {
        if (mnUserItemWidth)
        {
            mnCols = static_cast<sal_uInt16>((aWinSize.Width() - mnSpacing) / (mnUserItemWidth + mnSpacing));
            if (mnCols <= 0)
                mnCols = 1;
        }
        else
        {
            mnCols = 1;
        }
    }
    else
    {
        mnCols = mnUserCols;
    }

    // calculate number of rows
    mbScroll = false;

    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount) + mnCols - 1) / mnCols;
    if (mnLines <= 0)
        mnLines = 1;

    long nCalcHeight = aWinSize.Height() - nNoneHeight;
    if (mnUserVisLines)
    {
        mnVisLines = mnUserVisLines;
    }
    else if (mnUserItemHeight)
    {
        mnVisLines = (nCalcHeight - nNoneSpace + mnSpacing) / (mnUserItemHeight + mnSpacing);
        if (!mnVisLines)
            mnVisLines = 1;
    }
    else
    {
        mnVisLines = mnLines;
    }

    if (mnLines > mnVisLines)
        mbScroll = true;

    if (mnLines <= mnVisLines)
    {
        mnFirstLine = 0;
    }
    else
    {
        if (mnFirstLine > static_cast<sal_uInt16>(mnLines - mnVisLines))
            mnFirstLine = static_cast<sal_uInt16>(mnLines - mnVisLines);
    }

    // calculate item size
    const long nColSpace  = (mnCols - 1) * static_cast<long>(mnSpacing);
    const long nLineSpace = ((mnVisLines - 1) * mnSpacing) + nNoneSpace;
    if (mnUserItemWidth && !mnUserCols)
    {
        mnItemWidth = mnUserItemWidth;
        if (mnItemWidth > aWinSize.Width() - nColSpace)
            mnItemWidth = aWinSize.Width() - nColSpace;
    }
    else
        mnItemWidth = (aWinSize.Width() - nColSpace) / mnCols;
    if (mnUserItemHeight && !mnUserVisLines)
    {
        mnItemHeight = mnUserItemHeight;
        if (mnItemHeight > nCalcHeight - nNoneSpace)
            mnItemHeight = nCalcHeight - nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        mnItemHeight = nCalcHeight / mnVisLines;
    }

    // Init VirDev
    maVirDev->SetSettings(rRenderContext.GetSettings());
    maVirDev->SetOutputSizePixel(aWinSize);

    // nothing is changed in case of too small items
    if ((mnItemWidth <= 0) ||
        (mnItemHeight <= ((nStyle & WB_ITEMBORDER) ? 4 : 2)) ||
        !nItemCount)
    {
        mbHasVisibleItems = false;

        if ((nStyle & WB_NONEFIELD) && mpNoneItem)
        {
            mpNoneItem->mbVisible = false;
            mpNoneItem->maText = GetText();
        }

        for (size_t i = 0; i < nItemCount; i++)
        {
            mItemList[i]->mbVisible = false;
        }

        if (mxScrolledWindow && mxScrolledWindow->get_vpolicy() != VclPolicyType::NEVER)
        {
            mxScrolledWindow->set_vpolicy(VclPolicyType::NEVER);
            Size aPrefSize(GetDrawingArea()->get_preferred_size());
            GetDrawingArea()->set_size_request(aPrefSize.Width() + GetScrollWidth(), aPrefSize.Height());
        }
    }
    else
    {
        mbHasVisibleItems = true;

        // determine Frame-Style
        if (nStyle & WB_DOUBLEBORDER)
            mnFrameStyle = DrawFrameStyle::DoubleIn;
        else
            mnFrameStyle = DrawFrameStyle::In;

        // determine selected color and width
        // if necessary change the colors, to make the selection
        // better detectable
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aHighColor(rStyleSettings.GetHighlightColor());
        if (((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
             (aHighColor.GetBlue() > 0x80)) ||
            ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
             (aHighColor.GetBlue() == 0x80)))
        {
            mbBlackSel = true;
        }
        else
        {
            mbBlackSel = false;
        }
        // draw the selection with double width if the items are bigger
        if ((nStyle & WB_DOUBLEBORDER) &&
            ((mnItemWidth >= 25) && (mnItemHeight >= 20)))
        {
            mbDoubleSel = true;
        }
        else
        {
            mbDoubleSel = false;
        }

        // calculate offsets
        long nStartX;
        long nStartY;
        long nAllItemWidth = (mnItemWidth * mnCols) + nColSpace;
        long nAllItemHeight = (mnItemHeight * mnVisLines) + nNoneHeight + nLineSpace;
        nStartX = (aWinSize.Width() - nAllItemWidth) / 2;
        nStartY = (aWinSize.Height() - nAllItemHeight) / 2;

        // calculate and draw items
        maVirDev->SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // create NoSelection field and show it
        if (nStyle & WB_NONEFIELD)
        {
            if (!mpNoneItem)
                mpNoneItem.reset(new SvtValueSetItem(*this));

            mpNoneItem->mnId = 0;
            mpNoneItem->meType = VALUESETITEM_NONE;
            mpNoneItem->mbVisible = true;
            maNoneItemRect.SetLeft( x );
            maNoneItemRect.SetTop( y );
            maNoneItemRect.SetRight( maNoneItemRect.Left() + aWinSize.Width() - x - 1 );
            maNoneItemRect.SetBottom( y + nNoneHeight - 1 );

            ImplFormatItem(rRenderContext, mpNoneItem.get(), maNoneItemRect);

            y += nNoneHeight + nNoneSpace;
        }

        // draw items
        sal_uLong nFirstItem = static_cast<sal_uLong>(mnFirstLine) * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        maItemListRect.SetLeft( x );
        maItemListRect.SetTop( y );
        maItemListRect.SetRight( x + mnCols * (mnItemWidth + mnSpacing) - mnSpacing - 1 );
        maItemListRect.SetBottom( y + mnVisLines * (mnItemHeight + mnSpacing) - mnSpacing - 1 );

        for (size_t i = 0; i < nItemCount; i++)
        {
            SvtValueSetItem* pItem = mItemList[i].get();

            if (i >= nFirstItem && i < nLastItem)
            {
                if (!pItem->mbVisible && ImplHasAccessibleListeners())
                {
                    Any aOldAny;
                    Any aNewAny;

                    aNewAny <<= pItem->GetAccessible(false/*bIsTransientChildrenDisabled*/);
                    ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
                }

                pItem->mbVisible = true;
                ImplFormatItem(rRenderContext, pItem, tools::Rectangle(Point(x, y), Size(mnItemWidth, mnItemHeight)));

                if (!((i + 1) % mnCols))
                {
                    x = nStartX;
                    y += mnItemHeight + mnSpacing;
                }
                else
                    x += mnItemWidth + mnSpacing;
            }
            else
            {
                if (pItem->mbVisible && ImplHasAccessibleListeners())
                {
                    Any aOldAny;
                    Any aNewAny;

                    aOldAny <<= pItem->GetAccessible(false/*bIsTransientChildrenDisabled*/);
                    ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
                }

                pItem->mbVisible = false;
            }
        }

        // arrange ScrollBar, set values and show it
        if (mxScrolledWindow && (nStyle & WB_VSCROLL) && mxScrolledWindow->get_vpolicy() != VclPolicyType::ALWAYS)
        {
            long nPageSize = mnVisLines;
            if (nPageSize < 1)
                nPageSize = 1;
            mxScrolledWindow->vadjustment_configure(mnFirstLine, 0, mnLines, 1,
                                                    mnVisLines, nPageSize);
            mxScrolledWindow->set_vpolicy(VclPolicyType::ALWAYS);
            Size aPrefSize(GetDrawingArea()->get_preferred_size());
            GetDrawingArea()->set_size_request(aPrefSize.Width() - GetScrollWidth(), aPrefSize.Height());
        }
    }

    // waiting for the next since the formatting is finished
    mbFormat = false;
}

void SvtValueSet::ImplDrawSelect(vcl::RenderContext& rRenderContext)
{
    if (!IsReallyVisible())
        return;

    const bool bFocus = HasFocus();
    const bool bDrawSel = !((mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight));

    if (!bFocus && !bDrawSel)
    {
        ImplDrawItemText(rRenderContext, OUString());
        return;
    }

    ImplDrawSelect(rRenderContext, mnSelItemId, bFocus, bDrawSel);
    if (mbHighlight)
    {
        ImplDrawSelect(rRenderContext, mnHighItemId, bFocus, bDrawSel);
    }
}

void SvtValueSet::ImplDrawSelect(vcl::RenderContext& rRenderContext, sal_uInt16 nItemId, const bool bFocus, const bool bDrawSel )
{
    SvtValueSetItem* pItem;
    tools::Rectangle aRect;
    if (nItemId)
    {
        const size_t nPos = GetItemPos( nItemId );
        pItem = mItemList[ nPos ].get();
        aRect = ImplGetItemRect( nPos );
    }
    else if (mpNoneItem)
    {
        pItem = mpNoneItem.get();
        aRect = maNoneItemRect;
    }
    else if (bFocus && (pItem = ImplGetFirstItem()))
    {
        aRect = ImplGetItemRect(0);
    }
    else
    {
        return;
    }

    if (!pItem->mbVisible)
        return;

    // draw selection
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetFillColor();

    Color aDoubleColor(rStyleSettings.GetHighlightColor());
    Color aSingleColor(rStyleSettings.GetHighlightTextColor());
    if (!mbDoubleSel)
    {
        /*
        *  #99777# contrast enhancement for thin mode
        */
        const Wallpaper& rWall = maVirDev->GetBackground();
        if (!rWall.IsBitmap() && ! rWall.IsGradient())
        {
            const Color& rBack = rWall.GetColor();
            if (rBack.IsDark() && ! aDoubleColor.IsBright())
            {
                aDoubleColor = COL_WHITE;
                aSingleColor = COL_BLACK;
            }
            else if (rBack.IsBright() && !aDoubleColor.IsDark())
            {
                aDoubleColor = COL_BLACK;
                aSingleColor = COL_WHITE;
            }
        }
    }

    // specify selection output
    WinBits nStyle = GetStyle();
    if (nStyle & WB_MENUSTYLEVALUESET)
    {
        if (bFocus)
            DrawFocusRect(rRenderContext, aRect);
        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_BLACK : aDoubleColor);
            rRenderContext.DrawRect(aRect);
        }
    }
    else
    {
        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_BLACK : aDoubleColor);
            rRenderContext.DrawRect(aRect);
        }
        if (mbDoubleSel)
        {
            aRect.AdjustLeft( 1 );
            aRect.AdjustTop( 1 );
            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
            if (bDrawSel)
                rRenderContext.DrawRect(aRect);
        }
        aRect.AdjustLeft( 1 );
        aRect.AdjustTop( 1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );
        tools::Rectangle aRect2 = aRect;
        aRect.AdjustLeft( 1 );
        aRect.AdjustTop( 1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );
        if (bDrawSel)
            rRenderContext.DrawRect(aRect);
        if (mbDoubleSel)
        {
            aRect.AdjustLeft( 1 );
            aRect.AdjustTop( 1 );
            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
            if (bDrawSel)
                rRenderContext.DrawRect(aRect);
        }

        if (bDrawSel)
        {
            rRenderContext.SetLineColor(mbBlackSel ? COL_WHITE : aSingleColor);
        }
        else
        {
            rRenderContext.SetLineColor(COL_LIGHTGRAY);
        }
        rRenderContext.DrawRect(aRect2);
        if (bFocus)
            DrawFocusRect(rRenderContext, aRect2);
    }

    ImplDrawItemText(rRenderContext, pItem->maText);
}

void SvtValueSet::ImplFormatItem(vcl::RenderContext const & rRenderContext, SvtValueSetItem* pItem, tools::Rectangle aRect)
{
    WinBits nStyle = GetStyle();
    if (nStyle & WB_ITEMBORDER)
    {
        aRect.AdjustLeft(1 );
        aRect.AdjustTop(1 );
        aRect.AdjustRight( -1 );
        aRect.AdjustBottom( -1 );

        if (nStyle & WB_FLATVALUESET)
        {
            sal_Int32 nBorder = (nStyle & WB_DOUBLEBORDER) ? 2 : 1;

            aRect.AdjustLeft(nBorder );
            aRect.AdjustTop(nBorder );
            aRect.AdjustRight( -nBorder );
            aRect.AdjustBottom( -nBorder );
        }
        else
        {
            DecorationView aView(maVirDev.get());
            aRect = aView.DrawFrame(aRect, mnFrameStyle);
        }
    }

    if (pItem == mpNoneItem.get())
        pItem->maText = GetText();

    if (!((aRect.GetHeight() > 0) && (aRect.GetWidth() > 0)))
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (pItem == mpNoneItem.get())
    {
        maVirDev->SetFont(rRenderContext.GetFont());
        maVirDev->SetTextColor((nStyle & WB_MENUSTYLEVALUESET) ? rStyleSettings.GetMenuTextColor() : rStyleSettings.GetWindowTextColor());
        maVirDev->SetTextFillColor();
        maVirDev->SetFillColor((nStyle & WB_MENUSTYLEVALUESET) ? rStyleSettings.GetMenuColor() : rStyleSettings.GetWindowColor());
        maVirDev->DrawRect(aRect);
        Point aTxtPos(aRect.Left() + 2, aRect.Top());
        long nTxtWidth = rRenderContext.GetTextWidth(pItem->maText);
        if ((aTxtPos.X() + nTxtWidth) > aRect.Right())
        {
            maVirDev->SetClipRegion(vcl::Region(aRect));
            maVirDev->DrawText(aTxtPos, pItem->maText);
            maVirDev->SetClipRegion();
        }
        else
            maVirDev->DrawText(aTxtPos, pItem->maText);
    }
    else if (pItem->meType == VALUESETITEM_COLOR)
    {
        maVirDev->SetFillColor(pItem->maColor);
        maVirDev->DrawRect(aRect);
    }
    else
    {
        if (IsColor())
            maVirDev->SetFillColor(maColor);
        else if (nStyle & WB_MENUSTYLEVALUESET)
            maVirDev->SetFillColor(rStyleSettings.GetMenuColor());
        else if (IsEnabled())
            maVirDev->SetFillColor(rStyleSettings.GetWindowColor());
        else
            maVirDev->SetFillColor(rStyleSettings.GetFaceColor());
        maVirDev->DrawRect(aRect);

        if (pItem->meType == VALUESETITEM_USERDRAW)
        {
            UserDrawEvent aUDEvt(nullptr, maVirDev.get(), aRect, pItem->mnId);
            UserDraw(aUDEvt);
        }
        else
        {
            Size aImageSize = pItem->maImage.GetSizePixel();
            Size  aRectSize = aRect.GetSize();
            Point aPos(aRect.Left(), aRect.Top());
            aPos.AdjustX((aRectSize.Width() - aImageSize.Width()) / 2 );

            if (pItem->meType != VALUESETITEM_IMAGE_AND_TEXT)
                aPos.AdjustY((aRectSize.Height() - aImageSize.Height()) / 2 );

            DrawImageFlags  nImageStyle  = DrawImageFlags::NONE;
            if (!IsEnabled())
                nImageStyle  |= DrawImageFlags::Disable;

            if (aImageSize.Width()  > aRectSize.Width() ||
                aImageSize.Height() > aRectSize.Height())
            {
                maVirDev->SetClipRegion(vcl::Region(aRect));
                maVirDev->DrawImage(aPos, pItem->maImage, nImageStyle);
                maVirDev->SetClipRegion();
            }
            else
                maVirDev->DrawImage(aPos, pItem->maImage, nImageStyle);

            if (pItem->meType == VALUESETITEM_IMAGE_AND_TEXT)
            {
                maVirDev->SetFont(rRenderContext.GetFont());
                maVirDev->SetTextColor((nStyle & WB_MENUSTYLEVALUESET) ? rStyleSettings.GetMenuTextColor() : rStyleSettings.GetWindowTextColor());
                maVirDev->SetTextFillColor();

                long nTxtWidth = maVirDev->GetTextWidth(pItem->maText);

                if (nTxtWidth > aRect.GetWidth())
                    maVirDev->SetClipRegion(vcl::Region(aRect));

                maVirDev->DrawText(Point(aRect.Left() +
                                         (aRect.GetWidth() - nTxtWidth) / 2,
                                         aRect.Bottom() - maVirDev->GetTextHeight()),
                                   pItem->maText);

                if (nTxtWidth > aRect.GetWidth())
                    maVirDev->SetClipRegion();
            }
        }
    }

    const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

    if (nEdgeBlendingPercent)
    {
        const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
        const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
        const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
        const BitmapEx aBlendFrame(createBlendFrame(aRect.GetSize(), nAlpha, rTopLeft, rBottomRight));

        if (!aBlendFrame.IsEmpty())
        {
            maVirDev->DrawBitmapEx(aRect.TopLeft(), aBlendFrame);
        }
    }
}

void SvtValueSet::ImplDrawItemText(vcl::RenderContext& rRenderContext, const OUString& rText)
{
    if (!(GetStyle() & WB_NAMEFIELD))
        return;

    Size aWinSize(GetOutputSizePixel());
    long nTxtWidth = rRenderContext.GetTextWidth(rText);
    long nTxtOffset = mnTextOffset;

    // delete rectangle and show text
    if (GetStyle() & WB_FLATVALUESET)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        rRenderContext.DrawRect(tools::Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
        rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
    }
    else
    {
        nTxtOffset += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        rRenderContext.SetBackground(Application::GetSettings().GetStyleSettings().GetFaceColor());
        rRenderContext.Erase(tools::Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
    }
    rRenderContext.DrawText(Point((aWinSize.Width() - nTxtWidth) / 2, nTxtOffset + (NAME_OFFSET / 2)), rText);
}

void SvtValueSet::StyleUpdated()
{
    mbFormat = true;
    CustomWidgetController::StyleUpdated();
}

void SvtValueSet::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = true;
        queue_resize();
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
    }
}

void SvtValueSet::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    SvtValueSetItem* pItem = mItemList[nPos].get();
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const tools::Rectangle aRect = ImplGetItemRect(nPos);
        Invalidate(aRect);
    }
    else
        mbFormat = true;
}

Color SvtValueSet::GetItemColor( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maColor;
    else
        return Color();
}

Size SvtValueSet::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines ) const
{
    size_t nCalcCols = nDesireCols;
    size_t nCalcLines = nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = mnUserCols;
        else
            nCalcCols = 1;
    }

    if ( !nCalcLines )
    {
        nCalcLines = mnVisLines;

        if ( mbFormat )
        {
            if ( mnUserVisLines )
                nCalcLines = mnUserVisLines;
            else
            {
                // Floor( (M+N-1)/N )==Ceiling( M/N )
                nCalcLines = (mItemList.size()+nCalcCols-1) / nCalcCols;
                if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width() * nCalcCols, rItemSize.Height() * nCalcLines );
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        n;

    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.AdjustWidth(n * nCalcCols );
        aSize.AdjustHeight(n * nCalcLines );
    }
    else
        n = 0;

    if ( mnSpacing )
    {
        aSize.AdjustWidth(mnSpacing * (nCalcCols - 1) );
        aSize.AdjustHeight(mnSpacing * (nCalcLines - 1) );
    }

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.AdjustHeight(nTxtHeight + NAME_OFFSET );
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.AdjustHeight(NAME_LINE_HEIGHT + NAME_LINE_OFF_Y );
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.AdjustHeight(nTxtHeight + n + mnSpacing );
    }

    return aSize;
}

void SvtValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage )
{
    std::unique_ptr<SvtValueSetItem> pItem(new SvtValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    ImplInsertItem( std::move(pItem), VALUESET_APPEND );
}

void SvtValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const OUString& rText, size_t nPos )
{
    std::unique_ptr<SvtValueSetItem> pItem(new SvtValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), nPos );
}

void SvtValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    std::unique_ptr<SvtValueSetItem> pItem(new SvtValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    ImplInsertItem( std::move(pItem), nPos );
}

void SvtValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor,
                           const OUString& rText )
{
    std::unique_ptr<SvtValueSetItem> pItem(new SvtValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), VALUESET_APPEND );
}

void SvtValueSet::ImplInsertItem( std::unique_ptr<SvtValueSetItem> pItem, const size_t nPos )
{
    DBG_ASSERT( pItem->mnId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( pItem->mnId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    if ( nPos < mItemList.size() ) {
        mItemList.insert( mItemList.begin() + nPos, std::move(pItem) );
    } else {
        mItemList.push_back( std::move(pItem) );
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

int SvtValueSet::GetScrollWidth() const
{
    if (mxScrolledWindow)
        return mxScrolledWindow->get_vscroll_width();
    return 0;
}

void SvtValueSet::SetEdgeBlending(bool bNew)
{
    if(mbEdgeBlending != bNew)
    {
        mbEdgeBlending = bNew;
        mbFormat = true;

        if (GetDrawingArea() && IsReallyVisible() && IsUpdateMode())
        {
            Invalidate();
        }
    }
}

Size SvtValueSet::CalcItemSizePixel( const Size& rItemSize) const
{
    Size aSize = rItemSize;

    WinBits nStyle = GetStyle();
    if ( nStyle & WB_ITEMBORDER )
    {
        long n;

        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.AdjustWidth(n );
        aSize.AdjustHeight(n );
    }

    return aSize;
}

void SvtValueSet::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void SvtValueSet::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

//method to set accessible when the style is user draw.
void SvtValueSet::InsertItem( sal_uInt16 nItemId, const OUString& rText, size_t nPos  )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );
    std::unique_ptr<SvtValueSetItem> pItem(new SvtValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), nPos );
}

void SvtValueSet::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

OUString SvtValueSet::RequestHelp(tools::Rectangle& rHelpRect)
{
    Point aPos = rHelpRect.TopLeft();
    const size_t nItemPos = ImplGetItem( aPos );
    OUString sRet;
    if (nItemPos != VALUESET_ITEM_NOTFOUND)
    {
        rHelpRect = ImplGetItemRect(nItemPos);
        sRet = GetItemText(ImplGetItem(nItemPos)->mnId);
    }
    return sRet;
}

OUString SvtValueSet::GetItemText(sal_uInt16 nItemId) const
{
    const size_t nPos = GetItemPos(nItemId);

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return OUString();
}

void SvtValueSet::SetExtraSpacing( sal_uInt16 nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;

        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void SvtValueSet::SetFormat()
{
    mbFormat = true;
}

void SvtValueSet::SetItemData( sal_uInt16 nItemId, void* pData )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    SvtValueSetItem* pItem = mItemList[nPos].get();
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            const tools::Rectangle aRect = ImplGetItemRect(nPos);
            Invalidate(aRect);
        }
        else
            mbFormat = true;
    }
}

void* SvtValueSet::GetItemData( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->mpData;
    else
        return nullptr;
}

void SvtValueSet::SetItemText(sal_uInt16 nItemId, const OUString& rText)
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    SvtValueSetItem* pItem = mItemList[nPos].get();

    // Remember old and new name for accessibility event.
    Any aOldName;
    Any aNewName;
    OUString sString (pItem->maText);
    aOldName <<= sString;
    sString = rText;
    aNewName <<= sString;

    pItem->maText = rText;

    if (!mbFormat && IsReallyVisible() && IsUpdateMode())
    {
        sal_uInt16 nTempId = mnSelItemId;

        if (mbHighlight)
            nTempId = mnHighItemId;

        if (nTempId == nItemId)
            Invalidate();
    }

    if (ImplHasAccessibleListeners())
    {
        Reference<XAccessible> xAccessible(pItem->GetAccessible( false/*bIsTransientChildrenDisabled*/));
        SvtValueItemAcc* pValueItemAcc = static_cast<SvtValueItemAcc*>(xAccessible.get());
        pValueItemAcc->FireAccessibleEvent(AccessibleEventId::NAME_CHANGED, aOldName, aNewName);
    }
}

Size SvtValueSet::GetLargestItemSize()
{
    Size aLargestItem;

    for (std::unique_ptr<SvtValueSetItem>& pItem : mItemList)
    {
        if (!pItem->mbVisible)
            continue;

        if (pItem->meType != VALUESETITEM_IMAGE &&
            pItem->meType != VALUESETITEM_IMAGE_AND_TEXT)
        {
            // handle determining an optimal size for this case
            continue;
        }

        Size aSize = pItem->maImage.GetSizePixel();
        if (pItem->meType == VALUESETITEM_IMAGE_AND_TEXT)
        {
            aSize.AdjustHeight(3 * NAME_LINE_HEIGHT +
                maVirDev->GetTextHeight() );
            aSize.setWidth( std::max(aSize.Width(),
                                     maVirDev->GetTextWidth(pItem->maText) + NAME_OFFSET) );
        }

        aLargestItem.setWidth( std::max(aLargestItem.Width(), aSize.Width()) );
        aLargestItem.setHeight( std::max(aLargestItem.Height(), aSize.Height()) );
    }

    return aLargestItem;
}

void SvtValueSet::SetOptimalSize()
{
    Size aLargestSize(GetLargestItemSize());
    aLargestSize.setWidth(std::max(aLargestSize.Width(), mnUserItemWidth));
    aLargestSize.setHeight(std::max(aLargestSize.Height(), mnUserItemHeight));
    Size aPrefSize(CalcWindowSizePixel(aLargestSize));
    GetDrawingArea()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
}

Image SvtValueSet::GetItemImage(sal_uInt16 nItemId) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maImage;
    else
        return Image();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
