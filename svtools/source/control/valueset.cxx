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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <comphelper/base64.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/virdev.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include "valueimp.hxx"

#include <svtools/valueset.hxx>
#include <boost/property_tree/ptree.hpp>

#include <uiobject.hxx>

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
};

}

ValueSet::ValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow)
    : maVirDev( VclPtr<VirtualDevice>::Create())
    , mxScrolledWindow(std::move(pScrolledWindow))
    , mnHighItemId(0)
    , maColor(COL_TRANSPARENT)
    , mnStyle(0)
    , mbFormat(true)
    , mbHighlight(false)
{
    maVirDev->SetBackground(Application::GetSettings().GetStyleSettings().GetFaceColor());

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
    mbFullMode          = true;
    mbEdgeBlending      = false;
    mbHasVisibleItems   = false;

    if (mxScrolledWindow)
    {
        mxScrolledWindow->set_user_managed_scrolling();
        mxScrolledWindow->connect_vadjustment_changed(LINK(this, ValueSet, ImplScrollHdl));
    }
}

void ValueSet::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    // #106446#, #106601# force mirroring of virtual device
    maVirDev->EnableRTL(pDrawingArea->get_direction());
}

Reference<XAccessible> ValueSet::CreateAccessible()
{
    if (!mxAccessible)
        mxAccessible.set(new ValueSetAcc(this));
    return mxAccessible;
}

ValueSet::~ValueSet()
{
    Reference<XComponent> xComponent(mxAccessible, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    ImplDeleteItems();
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

void ValueSet::Select()
{
    maSelectHdl.Call( this );
}

void ValueSet::UserDraw( const UserDrawEvent& )
{
}

size_t ValueSet::ImplGetItem( const Point& rPos ) const
{
    if (!mbHasVisibleItems)
    {
        return VALUESET_ITEM_NOTFOUND;
    }

    if (mpNoneItem && maNoneItemRect.IsInside(rPos))
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
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(mxAccessible);

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool ValueSet::ImplHasAccessibleListeners()
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(mxAccessible);
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK(ValueSet, ImplScrollHdl, weld::ScrolledWindow&, rScrollWin, void)
{
    auto nNewFirstLine = rScrollWin.vadjustment_get_value();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        Invalidate();
    }
}

void ValueSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
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

void ValueSet::GetFocus()
{
    SAL_INFO("svtools", "value set getting focus");
    Invalidate();
    CustomWidgetController::GetFocus();

    // Tell the accessible object that we got the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(mxAccessible);
    if (pAcc)
        pAcc->GetFocus();
}

void ValueSet::LoseFocus()
{
    SAL_INFO("svtools", "value set losing focus");
    Invalidate();
    CustomWidgetController::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation(mxAccessible);
    if( pAcc )
        pAcc->LoseFocus();
}

void ValueSet::Resize()
{
    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    CustomWidgetController::Resize();
}

bool ValueSet::KeyInput( const KeyEvent& rKeyEvent )
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

void ValueSet::ImplTracking(const Point& rPos)
{
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

bool ValueSet::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    if ( rMouseEvent.IsLeft() )
    {
        ValueSetItem* pItem = ImplGetItem( ImplGetItem( rMouseEvent.GetPosPixel() ) );
        if (pItem && !rMouseEvent.IsMod2())
        {
            if (rMouseEvent.GetClicks() == 1)
            {
                SelectItem( pItem->mnId );
                if (!(GetStyle() & WB_NOPOINTERFOCUS))
                    GrabFocus();
            }
            else if ( rMouseEvent.GetClicks() == 2 )
                maDoubleClickHdl.Call( this );

            return true;
        }
    }

    return CustomWidgetController::MouseButtonDown( rMouseEvent );
}

bool ValueSet::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    if (rMouseEvent.IsLeft() && !rMouseEvent.IsMod2())
    {
        Select();
        return true;
    }

    return CustomWidgetController::MouseButtonUp( rMouseEvent );
}

bool ValueSet::MouseMove(const MouseEvent& rMouseEvent)
{
    // because of SelectionMode
    if ((GetStyle() & WB_MENUSTYLEVALUESET) || (GetStyle() & WB_FLATVALUESET))
        ImplTracking(rMouseEvent.GetPosPixel());
    return CustomWidgetController::MouseMove(rMouseEvent);
}

void ValueSet::QueueReformat()
{
    queue_resize();
    RecalcScrollBar();
    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
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
    if (mnHighItemId == nItemId || mnSelItemId == nItemId)
    {
        mnCurCol        = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = true;
    }

    QueueReformat();
}

void ValueSet::RecalcScrollBar()
{
    // reset scrolled window state to initial value
    // so it will get configured to the right adjustment
    WinBits nStyle = GetStyle();
    if (mxScrolledWindow && (nStyle & WB_VSCROLL))
        mxScrolledWindow->set_vpolicy(VclPolicyType::NEVER);
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

    RecalcScrollBar();

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

void ValueSet::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
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

void ValueSet::ImplDraw(vcl::RenderContext& rRenderContext)
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
        QueueReformat();
    }
}

void ValueSet::SetFirstLine(sal_uInt16 nNewFirstLine)
{
    if (nNewFirstLine != mnFirstLine)
    {
        mnFirstLine = nNewFirstLine;
        if (mxScrolledWindow)
            mxScrolledWindow->vadjustment_set_value(mnFirstLine);
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

    if (weld::DrawingArea* pNeedsFormatToScroll = !mnCols ? GetDrawingArea() : nullptr)
    {
        Format(pNeedsFormatToScroll->get_ref_device());
        // reset scrollbar so its set to the later calculated mnFirstLine on
        // the next Format
        RecalcScrollBar(); // reset scrollbar so its set to the later calculated
    }

    // if necessary scroll to the visible area
    if (mbScroll && nItemId && mnCols)
    {
        sal_uInt16 nNewLine = static_cast<sal_uInt16>(nItemPos / mnCols);
        if ( nNewLine < mnFirstLine )
        {
            SetFirstLine(nNewLine);
            bNewLine = true;
        }
        else if ( nNewLine > o3tl::make_unsigned(mnFirstLine+mnVisLines-1) )
        {
            SetFirstLine(static_cast<sal_uInt16>(nNewLine-mnVisLines+1));
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

    if( !ImplHasAccessibleListeners() )
        return;

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

void ValueSet::SetNoSelection()
{
    mbNoSelection   = true;
    mbHighlight     = false;

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void ValueSet::SetStyle(WinBits nStyle)
{
    if (nStyle != mnStyle)
    {
        mnStyle = nStyle;
        mbFormat = true;
        Invalidate();
    }
}

void ValueSet::Format(vcl::RenderContext const & rRenderContext)
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
        mpNoneItem.reset();
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
        SetFirstLine(0);
    }
    else
    {
        if (mnFirstLine > o3tl::make_unsigned(mnLines - mnVisLines))
            SetFirstLine(static_cast<sal_uInt16>(mnLines - mnVisLines));
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
        if (mbFullMode)
        {
            long nAllItemWidth = (mnItemWidth * mnCols) + nColSpace;
            long nAllItemHeight = (mnItemHeight * mnVisLines) + nNoneHeight + nLineSpace;
            nStartX = (aWinSize.Width() - nAllItemWidth) / 2;
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
            if (!mpNoneItem)
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

void ValueSet::ImplFormatItem(vcl::RenderContext const & rRenderContext, ValueSetItem* pItem, tools::Rectangle aRect)
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

    if ((aRect.GetHeight() <= 0) || (aRect.GetWidth() <= 0))
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
        rRenderContext.SetBackground(Application::GetSettings().GetStyleSettings().GetFaceColor());
        rRenderContext.Erase(tools::Rectangle(Point(0, nTxtOffset), Point(aWinSize.Width(), aWinSize.Height())));
    }
    rRenderContext.DrawText(Point((aWinSize.Width() - nTxtWidth) / 2, nTxtOffset + (NAME_OFFSET / 2)), rText);
}

void ValueSet::StyleUpdated()
{
    mbFormat = true;
    CustomWidgetController::StyleUpdated();
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
        QueueReformat();
    }
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

    return aSize;
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

void ValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
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

    QueueReformat();
}

int ValueSet::GetScrollWidth() const
{
    if (mxScrolledWindow)
        return mxScrolledWindow->get_vscroll_width();
    return 0;
}

void ValueSet::SetEdgeBlending(bool bNew)
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

void ValueSet::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        QueueReformat();
    }
}

void ValueSet::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        QueueReformat();
    }
}

//method to set accessible when the style is user draw.
void ValueSet::InsertItem( sal_uInt16 nItemId, const OUString& rText, size_t nPos  )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );
    std::unique_ptr<ValueSetItem> pItem(new ValueSetItem( *this ));
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    pItem->maText   = rText;
    ImplInsertItem( std::move(pItem), nPos );
}

void ValueSet::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        QueueReformat();
    }
}

OUString ValueSet::RequestHelp(tools::Rectangle& rHelpRect)
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

OUString ValueSet::GetItemText(sal_uInt16 nItemId) const
{
    const size_t nPos = GetItemPos(nItemId);

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return OUString();
}

void ValueSet::SetExtraSpacing( sal_uInt16 nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;
        QueueReformat();
    }
}

void ValueSet::SetFormat()
{
    mbFormat = true;
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
        ValueItemAcc* pValueItemAcc = static_cast<ValueItemAcc*>(xAccessible.get());
        pValueItemAcc->FireAccessibleEvent(AccessibleEventId::NAME_CHANGED, aOldName, aNewName);
    }
}

Size ValueSet::GetLargestItemSize()
{
    Size aLargestItem;

    for (const std::unique_ptr<ValueSetItem>& pItem : mItemList)
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

void ValueSet::SetOptimalSize()
{
    Size aLargestSize(GetLargestItemSize());
    aLargestSize.setWidth(std::max(aLargestSize.Width(), mnUserItemWidth));
    aLargestSize.setHeight(std::max(aLargestSize.Height(), mnUserItemHeight));
    Size aPrefSize(CalcWindowSizePixel(aLargestSize));
    GetDrawingArea()->set_size_request(aPrefSize.Width(), aPrefSize.Height());
}

Image ValueSet::GetItemImage(sal_uInt16 nItemId) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maImage;
    else
        return Image();
}

void ValueSet::SetColor(const Color& rColor)
{
    maColor  = rColor;
    mbFormat = true;
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void ValueSet::Show()
{
    if (mxScrolledWindow)
        mxScrolledWindow->show();
    CustomWidgetController::Show();
}

void ValueSet::Hide()
{
    CustomWidgetController::Hide();
    if (mxScrolledWindow)
        mxScrolledWindow->hide();
}

FactoryFunction ValueSet::GetUITestFactory() const
{
    return ValueSetUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
