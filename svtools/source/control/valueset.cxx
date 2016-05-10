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
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>
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

void ValueSet::ImplInit()
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

    // #106446#, #106601# force mirroring of virtual device
    maVirDev->EnableRTL( GetParent()->IsRTLEnabled() );

    ImplInitSettings( true, true, true );
}

ValueSet::ValueSet( vcl::Window* pParent, WinBits nWinStyle, bool bDisableTransientChildren ) :
    Control( pParent, nWinStyle ),
    maVirDev( VclPtr<VirtualDevice>::Create(*this) ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

VCL_BUILDER_DECL_FACTORY(ValueSet)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<ValueSet>::Create(pParent, nWinBits);
}

ValueSet::ValueSet( vcl::Window* pParent, const ResId& rResId, bool bDisableTransientChildren ) :
    Control( pParent, rResId ),
    maVirDev( VclPtr<VirtualDevice>::Create(*this) ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

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
        ValueSetItem* pItem = mItemList[i];
        if ( pItem->mbVisible && ImplHasAccessibleListeners() )
        {
            Any aOldAny;
            Any aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
        }

        delete pItem;
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
    if (GetStyle() & WB_VSCROLL)
    {
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
}

void ValueSet::ImplFormatItem(vcl::RenderContext& rRenderContext, ValueSetItem* pItem, Rectangle aRect)
{
    WinBits nStyle = GetStyle();
    if (nStyle & WB_ITEMBORDER)
    {
        aRect.Left()   += 1;
        aRect.Top()    += 1;
        aRect.Right()  -= 1;
        aRect.Bottom() -= 1;

        if (nStyle & WB_FLATVALUESET)
        {
            sal_Int32 nBorder = (nStyle & WB_DOUBLEBORDER) ? 2 : 1;

            aRect.Left()   += nBorder;
            aRect.Top()    += nBorder;
            aRect.Right()  -= nBorder;
            aRect.Bottom() -= nBorder;
        }
        else
        {
            DecorationView aView(maVirDev.get());
            aRect = aView.DrawFrame(aRect, mnFrameStyle);
        }
    }

    if (pItem == mpNoneItem.get())
        pItem->maText = GetText();

    if ((aRect.GetHeight() > 0) && (aRect.GetWidth() > 0))
    {
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
            if (nStyle & WB_RADIOSEL)
            {
                aTxtPos.X() += 4;
                aTxtPos.Y() += 4;
            }
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
                UserDrawEvent aUDEvt(this, maVirDev.get(), aRect, pItem->mnId);
                UserDraw(aUDEvt);
            }
            else
            {
                Size aImageSize = pItem->maImage.GetSizePixel();
                Size  aRectSize = aRect.GetSize();
                Point aPos(aRect.Left(), aRect.Top());
                aPos.X() += (aRectSize.Width() - aImageSize.Width()) / 2;

                if (pItem->meType != VALUESETITEM_IMAGE_AND_TEXT)
                    aPos.Y() += (aRectSize.Height() - aImageSize.Height()) / 2;

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
}

Reference<XAccessible> ValueSet::CreateAccessible()
{
    return new ValueSetAcc( this, mbIsTransientChildrenDisabled );
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
        aWinSize.Height() -= nTxtHeight + NAME_OFFSET;

        if (!(nStyle & WB_FLATVALUESET))
        {
            mnTextOffset -= NAME_LINE_HEIGHT + NAME_LINE_OFF_Y;
            aWinSize.Height() -= NAME_LINE_HEIGHT + NAME_LINE_OFF_Y;
        }
    }
    else
        mnTextOffset = 0;

    // consider offset and size, if NoneField does exist
    if (nStyle & WB_NONEFIELD)
    {
        nNoneHeight = nTxtHeight + nOff;
        nNoneSpace = mnSpacing;
        if (nStyle & WB_RADIOSEL)
            nNoneHeight += 8;
    }
    else
    {
        nNoneHeight = 0;
        nNoneSpace = 0;

        if (mpNoneItem.get())
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

    // Init VirDev
    maVirDev->SetSettings(rRenderContext.GetSettings());
    maVirDev->SetBackground(rRenderContext.GetBackground());
    maVirDev->SetOutputSizePixel(aWinSize);

    // nothing is changed in case of too small items
    if ((mnItemWidth <= 0) ||
        (mnItemHeight <= ((nStyle & WB_ITEMBORDER) ? 4 : 2)) ||
        !nItemCount)
    {
        mbHasVisibleItems = false;

        if (nStyle & WB_NONEFIELD)
        {
            if (mpNoneItem.get())
            {
                mpNoneItem->mbVisible = false;
                mpNoneItem->maText = GetText();
            }
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
        maVirDev->SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // create NoSelection field and show it
        if (nStyle & WB_NONEFIELD)
        {
            if (mpNoneItem.get() == nullptr)
                mpNoneItem.reset(new ValueSetItem(*this));

            mpNoneItem->mnId = 0;
            mpNoneItem->meType = VALUESETITEM_NONE;
            mpNoneItem->mbVisible = true;
            maNoneItemRect.Left() = x;
            maNoneItemRect.Top()  = y;
            maNoneItemRect.Right() = maNoneItemRect.Left() + aWinSize.Width() - x - 1;
            maNoneItemRect.Bottom() = y + nNoneHeight - 1;

            ImplFormatItem(rRenderContext, mpNoneItem.get(), maNoneItemRect);

            y += nNoneHeight + nNoneSpace;
        }

        // draw items
        sal_uLong nFirstItem = static_cast<sal_uLong>(mnFirstLine) * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        maItemListRect.Left() = x;
        maItemListRect.Top() = y;
        maItemListRect.Right() = x + mnCols * (mnItemWidth + mnSpacing) - mnSpacing - 1;
        maItemListRect.Bottom() = y + mnVisLines * (mnItemHeight + mnSpacing) - mnSpacing - 1;

        if (!mbFullMode)
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if (y + (mnVisLines * (mnItemHeight + mnSpacing)) < aWinSize.Height())
                nLastItem += mnCols;
            maItemListRect.Bottom() = aWinSize.Height() - y;
        }
        for (size_t i = 0; i < nItemCount; i++)
        {
            ValueSetItem* pItem = mItemList[i];

            if (i >= nFirstItem && i < nLastItem)
            {
                if (!pItem->mbVisible && ImplHasAccessibleListeners())
                {
                    Any aOldAny;
                    Any aNewAny;

                    aNewAny <<= pItem->GetAccessible(mbIsTransientChildrenDisabled);
                    ImplFireAccessibleEvent(AccessibleEventId::CHILD, aOldAny, aNewAny);
                }

                pItem->mbVisible = true;
                ImplFormatItem(rRenderContext, pItem, Rectangle(Point(x, y), Size(mnItemWidth, mnItemHeight)));

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

                    aOldAny <<= pItem->GetAccessible(mbIsTransientChildrenDisabled);
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
                aPos.Y() = nStartY + nNoneHeight + 1;
                aSize.Height() = ((mnItemHeight + mnSpacing) * mnVisLines) - 2 - mnSpacing;
            }
            mxScrollBar->SetPosSizePixel(aPos, aSize);
            mxScrollBar->SetRangeMax(mnLines);
            mxScrollBar->SetVisibleSize(mnVisLines);
            mxScrollBar->SetThumbPos((long)mnFirstLine);
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
        rRenderContext.DrawRect(Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
        rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
    }
    else
    {
        nTxtOffset += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        rRenderContext.Erase(Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
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
    Rectangle aRect;
    if (nItemId)
    {
        const size_t nPos = GetItemPos( nItemId );
        pItem = mItemList[ nPos ];
        aRect = ImplGetItemRect( nPos );
    }
    else if (mpNoneItem.get())
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

    if (pItem->mbVisible)
    {
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
                    aDoubleColor = Color(COL_WHITE);
                    aSingleColor = Color(COL_BLACK);
                }
                else if (rBack.IsBright() && ! aDoubleColor.IsDark())
                {
                    aDoubleColor = Color(COL_BLACK);
                    aSingleColor = Color(COL_WHITE);
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
                rRenderContext.SetLineColor(mbBlackSel ? Color(COL_BLACK) : aDoubleColor);
                rRenderContext.DrawRect(aRect);
            }
        }
        else if (nStyle & WB_RADIOSEL)
        {
            aRect.Left()   += 3;
            aRect.Top()    += 3;
            aRect.Right()  -= 3;
            aRect.Bottom() -= 3;
            if (nStyle & WB_DOUBLEBORDER)
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
            }

            if (bFocus)
                ShowFocus(aRect);

            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;

            if (bDrawSel)
            {
                rRenderContext.SetLineColor(aDoubleColor);
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                rRenderContext.DrawRect(aRect);
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                rRenderContext.DrawRect(aRect);
            }
        }
        else
        {
            if (bDrawSel)
            {
                rRenderContext.SetLineColor(mbBlackSel ? Color(COL_BLACK) : aDoubleColor);
                rRenderContext.DrawRect(aRect);
            }
            if (mbDoubleSel)
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                if (bDrawSel)
                    rRenderContext.DrawRect(aRect);
            }
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            Rectangle aRect2 = aRect;
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            if (bDrawSel)
                rRenderContext.DrawRect(aRect);
            if (mbDoubleSel)
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
                if (bDrawSel)
                    rRenderContext.DrawRect(aRect);
            }

            if (bDrawSel)
            {
                rRenderContext.SetLineColor(mbBlackSel ? Color(COL_WHITE) : aSingleColor);
            }
            else
            {
                rRenderContext.SetLineColor(Color(COL_LIGHTGRAY));
            }
            rRenderContext.DrawRect(aRect2);

            if (bFocus)
                ShowFocus(aRect2);
        }

        ImplDrawItemText(rRenderContext, pItem->maText);
    }
}

void ValueSet::ImplHideSelect( sal_uInt16 nItemId )
{
    Rectangle aRect;

    const size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
        if ( !mItemList[nItemPos]->mbVisible )
        {
            return;
        }
        aRect = ImplGetItemRect(nItemPos);
    }
    else
    {
        if (mpNoneItem.get() == nullptr)
        {
            return;
        }
        aRect = maNoneItemRect;
    }

    HideFocus();
    const Point aPos  = aRect.TopLeft();
    const Size  aSize = aRect.GetSize();
    DrawOutDev( aPos, aSize, aPos, aSize, *maVirDev.get() );
}

void ValueSet::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
{
    if ( mnHighItemId != nItemId )
    {
        // remember the old item to delete the previous selection
        sal_uInt16 nOldItem = mnHighItemId;
        mnHighItemId = nItemId;

        // don't draw the selection if nothing is selected
        if ( !bIsSelection && mbNoSelection )
            mbDrawSelection = false;

        // remove the old selection and draw the new one
        ImplHideSelect( nOldItem );
        Invalidate();
        mbDrawSelection = true;
    }
}

void ValueSet::ImplDraw(vcl::RenderContext& rRenderContext)
{
    if (mbFormat)
        Format(rRenderContext);

    HideFocus();

    Point aDefPos;
    Size aSize = maVirDev->GetOutputSizePixel();

    if (mxScrollBar.get() && mxScrollBar->IsVisible())
    {
        Point aScrPos = mxScrollBar->GetPosPixel();
        Size aScrSize = mxScrollBar->GetSizePixel();
        Point aTempPos(0, aScrPos.Y());
        Size aTempSize(aSize.Width(), aScrPos.Y());

        rRenderContext.DrawOutDev(aDefPos, aTempSize, aDefPos, aTempSize, *maVirDev.get());
        aTempSize.Width()   = aScrPos.X() - 1;
        aTempSize.Height()  = aScrSize.Height();
        rRenderContext.DrawOutDev(aTempPos, aTempSize, aTempPos, aTempSize, *maVirDev.get());
        aTempPos.Y()        = aScrPos.Y() + aScrSize.Height();
        aTempSize.Width()   = aSize.Width();
        aTempSize.Height()  = aSize.Height() - aTempPos.Y();
        rRenderContext.DrawOutDev(aTempPos, aTempSize, aTempPos, aTempSize, *maVirDev.get());
    }
    else
        rRenderContext.DrawOutDev(aDefPos, aSize, aDefPos, aSize, *maVirDev.get());

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
                aPos1.Y()++;
                aPos2.Y()++;
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

size_t ValueSet::ImplGetItem( const Point& rPos, bool bMove ) const
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

        // return the previously selected item if spacing is set and
        // the mouse hasn't left the window yet
        if (bMove && mnSpacing && mnHighItemId)
        {
            return GetItemPos( mnHighItemId );
        }
    }

    return VALUESET_ITEM_NOTFOUND;
}

ValueSetItem* ValueSet::ImplGetItem( size_t nPos )
{
    if (nPos == VALUESET_ITEM_NONEITEM)
        return mpNoneItem.get();
    else
        return (nPos < mItemList.size()) ? mItemList[nPos] : nullptr;
}

ValueSetItem* ValueSet::ImplGetFirstItem()
{
    return mItemList.size() ? mItemList[0] : nullptr;
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

IMPL_LINK_TYPED( ValueSet,ImplScrollHdl, ScrollBar*, pScrollBar, void )
{
    sal_uInt16 nNewFirstLine = (sal_uInt16)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        Invalidate();
    }
}

IMPL_LINK_NOARG_TYPED(ValueSet, ImplTimerHdl, Timer *, void)
{
    ImplTracking( GetPointerPosPixel(), true );
}

void ValueSet::ImplTracking( const Point& rPos, bool bRepeat )
{
    if ( bRepeat || mbSelection )
    {
        if ( ImplScroll( rPos ) )
        {
            if ( mbSelection )
            {
                maTimer.SetTimeoutHdl( LINK( this, ValueSet, ImplTimerHdl ) );
                maTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
                maTimer.Start();
            }
        }
    }

    ValueSetItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem )
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
            mbHighlight = true;

        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
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
                    DoubleClick();

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
    if ( mbSelection || (GetStyle() & WB_MENUSTYLEVALUESET) )
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

    const size_t nCurPos = mnSelItemId ? GetItemPos(mnSelItemId)
                                       : (mpNoneItem.get() ? VALUESET_ITEM_NONEITEM : 0);
    size_t nItemPos = VALUESET_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch (rKeyEvent.GetKeyCode().GetCode())
    {
        case KEY_HOME:
            nItemPos = mpNoneItem.get() ? VALUESET_ITEM_NONEITEM : 0;
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
                else if (mpNoneItem.get())
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
            SAL_FALLTHROUGH;
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
                else if (mpNoneItem.get())
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
            SAL_FALLTHROUGH;
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
            SAL_FALLTHROUGH;
        default:
            Control::KeyInput( rKeyEvent );
            return;
    }

    // This point is reached only if key travelling was used,
    // in which case selection mode should be switched off
    EndSelection();

    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
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

void ValueSet::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    if (GetStyle() & WB_FLATVALUESET)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        long nOffY = maVirDev->GetOutputSizePixel().Height();
        Size aWinSize(GetOutputSizePixel());
        rRenderContext.DrawRect(Rectangle(Point(0, nOffY ), Point( aWinSize.Width(), aWinSize.Height())));
    }

    ImplDraw(rRenderContext);
}

void ValueSet::GetFocus()
{
    OSL_TRACE ("value set getting focus");
    Invalidate();
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(GetAccessible(false));
    if (pAcc)
        pAcc->GetFocus();
}

void ValueSet::LoseFocus()
{
    OSL_TRACE ("value set losing focus");
    if ( mbNoSelection && mnSelItemId )
        ImplHideSelect( mnSelItemId );
    else
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
    if ( (rHelpEvent.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON)) == HelpEventMode::QUICK )
    {
        Point aPos = ScreenToOutputPixel( rHelpEvent.GetMousePosPixel() );
        size_t nItemPos = ImplGetItem( aPos );
        if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            Rectangle aItemRect = ImplGetItemRect( nItemPos );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
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

void ValueSet::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

void ValueSet::UserDraw( const UserDrawEvent& )
{
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    ImplInsertItem( pItem, nPos );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const OUString& rText, size_t nPos,
                           bool bShowLegend )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = bShowLegend ? VALUESETITEM_IMAGE_AND_TEXT : VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor,
                           const OUString& rText, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
}

void ValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    ImplInsertItem( pItem, nPos );
}

void ValueSet::ImplInsertItem( ValueSetItem *const pItem, const size_t nPos )
{
    DBG_ASSERT( pItem->mnId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( pItem->mnId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        mItemList.insert( it, pItem );
    } else {
        mItemList.push_back( pItem );
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

Rectangle ValueSet::ImplGetItemRect( size_t nPos ) const
{
    const size_t nVisibleBegin = static_cast<size_t>(mnFirstLine)*mnCols;
    const size_t nVisibleEnd = nVisibleBegin + static_cast<size_t>(mnVisLines)*mnCols;

    // Check if the item is inside the range of the displayed ones,
    // taking into account that last row could be incomplete
    if ( nPos<nVisibleBegin || nPos>=nVisibleEnd || nPos>=mItemList.size() )
        return Rectangle();

    nPos -= nVisibleBegin;

    const size_t row = mnCols ? nPos/mnCols : 0;
    const size_t col = mnCols ? nPos%mnCols : 0;
    const long x = maItemListRect.Left()+col*(mnItemWidth+mnSpacing);
    const long y = maItemListRect.Top()+row*(mnItemHeight+mnSpacing);

    return Rectangle( Point(x, y), Size(mnItemWidth, mnItemHeight) );
}

void ValueSet::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        delete *it;
        mItemList.erase( it );
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

Rectangle ValueSet::GetItemRect( sal_uInt16 nItemId ) const
{
    const size_t nPos = GetItemPos( nItemId );

    if ( nPos!=VALUESET_ITEM_NOTFOUND && mItemList[nPos]->mbVisible )
        return ImplGetItemRect( nPos );

    return Rectangle();
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

//method to set accessible when the style is user draw.
void ValueSet::InsertItem( sal_uInt16 nItemId, const OUString& rText, size_t nPos  )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
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

    if ( (mnSelItemId != nItemId) || mbNoSelection )
    {
        sal_uInt16 nOldItem = mnSelItemId ? mnSelItemId : 1;
        mnSelItemId = nItemId;
        mbNoSelection = false;

        bool bNewOut = !mbFormat && IsReallyVisible() && IsUpdateMode();
        bool bNewLine = false;

        // if necessary scroll to the visible area
        if (mbScroll && nItemId && mnCols)
        {
            sal_uInt16 nNewLine = (sal_uInt16)(nItemPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
                bNewLine = true;
            }
            else if ( nNewLine > (sal_uInt16)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (sal_uInt16)(nNewLine-mnVisLines+1);
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
                ImplHideSelect( nOldItem );
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
                        mItemList[nPos]->GetAccessible( mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        Any aOldAny;
                        Any aNewAny;
                        if( !mbIsTransientChildrenDisabled )
                        {
                            aOldAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
                            ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny);
                        }
                    }
                }
            }

            // focus event (select)
            const size_t nPos = GetItemPos( mnSelItemId );

            ValueSetItem* pItem;
            if( nPos != VALUESET_ITEM_NOTFOUND )
                pItem = mItemList[nPos];
            else
                pItem = mpNoneItem.get();

            ValueItemAcc* pItemAcc = nullptr;
            if (pItem != nullptr)
                pItemAcc = ValueItemAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                Any aOldAny;
                Any aNewAny;
                if( !mbIsTransientChildrenDisabled )
                {
                    aNewAny <<= Reference<XInterface>(static_cast<cppu::OWeakObject*>(pItemAcc));
                    ImplFireAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny);
                }
                else
                {
                    aNewAny <<= AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny);
                }
            }

            // selection event
            Any aOldAny;
            Any aNewAny;
            ImplFireAccessibleEvent(AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny);
        }
        maHighlightHdl.Call(this);
    }
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

    ValueSetItem* pItem = mItemList[nPos];
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const Rectangle aRect = ImplGetItemRect(nPos);
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

    ValueSetItem* pItem = mItemList[nPos];
    pItem->meType  = VALUESETITEM_COLOR;
    pItem->maColor = rColor;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const Rectangle aRect = ImplGetItemRect(nPos);
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

    ValueSetItem* pItem = mItemList[nPos];
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            const Rectangle aRect = ImplGetItemRect(nPos);
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


    ValueSetItem* pItem = mItemList[nPos];

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
        Reference<XAccessible> xAccessible(pItem->GetAccessible( mbIsTransientChildrenDisabled));
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

void ValueSet::SetFormat(bool bFormat)
{
    mbFormat = bFormat;
}

bool ValueSet::StartDrag( const CommandEvent& rEvent, vcl::Region& rRegion )
{
    if ( rEvent.GetCommand() != CommandEventId::StartDrag )
        return false;

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
        return false;

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

    return true;
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

        aSize.Width()  += n * nCalcCols;
        aSize.Height() += n * nCalcLines;
    }
    else
        n = 0;

    if ( mnSpacing )
    {
        aSize.Width()  += mnSpacing * (nCalcCols - 1);
        aSize.Height() += mnSpacing * (nCalcLines - 1);
    }

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.Height() += nTxtHeight + NAME_OFFSET;
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.Height() += NAME_LINE_HEIGHT + NAME_LINE_OFF_Y;
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.Height() += nTxtHeight + n + mnSpacing;
        if ( nStyle & WB_RADIOSEL )
            aSize.Height() += 8;
    }

    // sum possible ScrollBar width
    aSize.Width() += GetScrollWidth();

    return aSize;
}

Size ValueSet::CalcItemSizePixel( const Size& rItemSize, bool bOut ) const
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

        if ( bOut )
        {
            aSize.Width()  += n;
            aSize.Height() += n;
        }
        else
        {
            aSize.Width()  -= n;
            aSize.Height() -= n;
        }
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

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        const ValueSetItem* pItem = mItemList[i];
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
            aSize.Height() += 3 * NAME_LINE_HEIGHT +
                maVirDev->GetTextHeight();
            aSize.Width() = std::max(aSize.Width(),
                                     maVirDev->GetTextWidth(pItem->maText) + NAME_OFFSET);
        }

        aLargestItem.Width() = std::max(aLargestItem.Width(), aSize.Width());
        aLargestItem.Height() = std::max(aLargestItem.Height(), aSize.Height());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
