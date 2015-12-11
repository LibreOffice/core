/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>

#include <utility>

#include "thumbnailviewacc.hxx"

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <rtl/ustring.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <memory>

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

enum
{
    ITEM_OFFSET = 4,
    ITEM_OFFSET_DOUBLE = 6,
    NAME_LINE_OFF_X = 2,
    NAME_LINE_OFF_Y = 2,
    NAME_LINE_HEIGHT = 2,
    NAME_OFFSET = 2,
    SCROLL_OFFSET = 4
};

ThumbnailView::ThumbnailView (vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : Control( pParent, nWinStyle )
    , mpItemAttrs(new ThumbnailItemAttributes)
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::~ThumbnailView()
{
    disposeOnce();
}

void ThumbnailView::dispose()
{
    css::uno::Reference< css::lang::XComponent> xComponent(GetAccessible(false), css::uno::UNO_QUERY);

    if (xComponent.is())
        xComponent->dispose ();

    mpScrBar.disposeAndClear();
    delete mpItemAttrs;

    ImplDeleteItems();
    Control::dispose();
}

void ThumbnailView::MouseMove(const MouseEvent& rMEvt)
{
    size_t nItemCount = mFilteredItemList.size();
    Point aPoint = rMEvt.GetPosPixel();
    OUString aHelp;

    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *pItem = mFilteredItemList[i];

        if (pItem->mbVisible && !rMEvt.IsLeaveWindow() && pItem->getDrawArea().IsInside(aPoint))
        {
            aHelp = pItem->getHelpText();
        }

        Rectangle aToInvalidate(pItem->updateHighlight(pItem->mbVisible && !rMEvt.IsLeaveWindow(), aPoint));

        if (!aToInvalidate.IsEmpty() && IsReallyVisible() && IsUpdateMode())
            Invalidate(aToInvalidate);
    }

    if (mbShowTooltips)
        SetQuickHelpText(aHelp);
}

void ThumbnailView::AppendItem(ThumbnailViewItem *pItem)
{
    if (maFilterFunc(pItem))
    {
        // Save current start,end range, iterator might get invalidated
        size_t nSelStartPos = 0;
        ThumbnailViewItem *pSelStartItem = nullptr;

        if (mpStartSelRange != mFilteredItemList.end())
        {
            pSelStartItem = *mpStartSelRange;
            nSelStartPos = mpStartSelRange - mFilteredItemList.begin();
        }

        mFilteredItemList.push_back(pItem);
        mpStartSelRange = pSelStartItem != nullptr ? mFilteredItemList.begin() + nSelStartPos : mFilteredItemList.end();
    }

    mItemList.push_back(pItem);
}

void ThumbnailView::ImplInit()
{
    mpScrBar = nullptr;
    mnHeaderHeight = 0;
    mnItemWidth = 0;
    mnItemHeight = 0;
    mnItemPadding = 0;
    mnVisLines = 0;
    mnLines = 0;
    mnFineness = 5;
    mnFirstLine = 0;
    mnCols = 0;
    mbScroll = false;
    mbHasVisibleItems = false;
    mbShowTooltips = false;
    maFilterFunc = ViewFilterAll();
    maFillColor = GetSettings().GetStyleSettings().GetFieldColor();
    maTextColor = GetSettings().GetStyleSettings().GetWindowTextColor();
    maHighlightColor = GetSettings().GetStyleSettings().GetHighlightColor();
    maHighlightTextColor = GetSettings().GetStyleSettings().GetWindowTextColor();

    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
    mfHighlightTransparence = aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01;

    mpStartSelRange = mFilteredItemList.end();

    ApplySettings(*this);
}

void ThumbnailView::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ThumbnailViewItem *const pItem = mItemList[i];

        // deselect all current selected items and fire events
        if (pItem->isSelected())
        {
            pItem->setSelection(false);
            maItemStateHdl.Call(pItem);

            // fire accessible event???
        }

        if ( pItem->isVisible() && ImplHasAccessibleListeners() )
        {
            css::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mItemList.clear();
    mFilteredItemList.clear();

    mpStartSelRange = mFilteredItemList.end();
}

void ThumbnailView::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    ApplyControlFont(*this, rStyleSettings.GetAppFont());
    ApplyControlForeground(*this, rStyleSettings.GetButtonTextColor());
    rRenderContext.SetTextFillColor();
    rRenderContext.SetBackground(maFillColor);

    mpItemAttrs->aFillColor = maFillColor.getBColor();
    mpItemAttrs->aTextColor = maTextColor.getBColor();
    mpItemAttrs->aHighlightColor = maHighlightColor.getBColor();
    mpItemAttrs->aHighlightTextColor = maHighlightTextColor.getBColor();
    mpItemAttrs->fHighlightTransparence = mfHighlightTransparence;
    mpItemAttrs->aFontAttr = getFontAttributeFromVclFont(mpItemAttrs->aFontSize,GetFont(),false,true);
    mpItemAttrs->nMaxTextLength = 0;
}

void ThumbnailView::ImplInitScrollBar()
{
    if ( GetStyle() & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = VclPtr<ScrollBar>::Create( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ThumbnailView, ImplScrollHdl ) );
        }
        else
        {
            // adapt the width because of the changed settings
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->setPosSizePixel( 0, 0, nScrBarWidth, 0, PosSizeFlags::Width );
        }
    }
}

void ThumbnailView::DrawItem(ThumbnailViewItem *pItem)
{
    if (pItem->isVisible())
    {
        Rectangle aRect = pItem->getDrawArea();

        if ((aRect.GetHeight() > 0) && (aRect.GetWidth() > 0))
            Invalidate(aRect);
    }
}

void ThumbnailView::OnItemDblClicked (ThumbnailViewItem*)
{
}

css::uno::Reference< css::accessibility::XAccessible > ThumbnailView::CreateAccessible()
{
    return new ThumbnailViewAcc( this, mbIsTransientChildrenDisabled );
}

void ThumbnailView::CalculateItemPositions (bool bScrollBarUsed)
{
    if (!mnItemHeight || !mnItemWidth)
        return;

    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mFilteredItemList.size();
    WinBits     nStyle = GetStyle();
    VclPtr<ScrollBar>  pDelScrBar;

    // consider the scrolling
    if ( nStyle & WB_VSCROLL )
        ImplInitScrollBar();
    else
    {
        if ( mpScrBar )
        {
            // delete ScrollBar not until later, to prevent recursive calls
            pDelScrBar = mpScrBar;
            mpScrBar = nullptr;
        }
    }

    // calculate window scroll ratio
    float nScrollRatio;
    if( bScrollBarUsed && mpScrBar )
        nScrollRatio = static_cast<float>(mpScrBar->GetThumbPos()) /
                        static_cast<float>(mpScrBar->GetRangeMax()-2);
    else
        nScrollRatio = 0;

    // calculate ScrollBar width
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width();

    // calculate maximum number of visible columns
    mnCols = (sal_uInt16)((aWinSize.Width()-nScrBarWidth) / (mnItemWidth));

    if (!mnCols)
        mnCols = 1;

    // calculate maximum number of visible rows
    mnVisLines = (sal_uInt16)((aWinSize.Height()-mnHeaderHeight) / (mnItemHeight));

    // calculate empty space
    long nHSpace = aWinSize.Width()-nScrBarWidth - mnCols*mnItemWidth;
    long nVSpace = aWinSize.Height()-mnHeaderHeight - mnVisLines*mnItemHeight;
    long nHItemSpace = nHSpace / (mnCols+1);
    long nVItemSpace = nVSpace / (mnVisLines+1);

    // calculate maximum number of rows
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;

    if ( !mnLines )
        mnLines = 1;

    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
        mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);

    mbHasVisibleItems = true;

    long nItemHeightOffset = mnItemHeight + nVItemSpace;
    long nHiddenLines = (static_cast<long>(
        ( mnLines - 1 ) * nItemHeightOffset * nScrollRatio ) -
        nVItemSpace - mnHeaderHeight) /
        nItemHeightOffset;

    // calculate offsets
    long nStartX = nHItemSpace;
    long nStartY = nVItemSpace + mnHeaderHeight;

    // calculate and draw items
    long x = nStartX;
    long y = nStartY - ( mnLines - 1 ) * nItemHeightOffset * nScrollRatio +
        nHiddenLines * nItemHeightOffset;

    // draw items
    // Unless we are scrolling (via scrollbar) we just use the precalculated
    // mnFirstLine -- our nHiddenLines calculation takes into account only
    // what the user has done with the scrollbar but not any changes of selection
    // using the keyboard, meaning we could accidentally hide the selected item
    // if we believe the scrollbar (fdo#72287).
    size_t nFirstItem = (bScrollBarUsed ? nHiddenLines : mnFirstLine) * mnCols;
    size_t nLastItem = nFirstItem + (mnVisLines + 1) * mnCols;

    // If want also draw parts of items in the last line,
    // then we add one more line if parts of these line are
    // visible

    size_t nCurCount = 0;
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        ThumbnailViewItem *const pItem = mFilteredItemList[i];

        if ((nCurCount >= nFirstItem) && (nCurCount < nLastItem))
        {
            if( !pItem->isVisible())
            {
                if ( ImplHasAccessibleListeners() )
                {
                    css::uno::Any aOldAny, aNewAny;

                    aNewAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(true);

                maItemStateHdl.Call(pItem);
            }

            pItem->setDrawArea(Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
            pItem->calculateItemsPosition(mnThumbnailHeight,mnDisplayHeight,mnItemPadding,mpItemAttrs->nMaxTextLength,mpItemAttrs);

            if ( !((nCurCount+1) % mnCols) )
            {
                x = nStartX;
                y += mnItemHeight+nVItemSpace;
            }
            else
                x += mnItemWidth+nHItemSpace;
        }
        else
        {
            if( pItem->isVisible())
            {
                if ( ImplHasAccessibleListeners() )
                {
                    css::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);

                maItemStateHdl.Call(pItem);
            }

        }

        ++nCurCount;
    }

    // arrange ScrollBar, set values and show it
    if ( mpScrBar )
    {
        mnLines = (nCurCount+mnCols-1)/mnCols;

        // check if scroll is needed
        mbScroll = mnLines > mnVisLines;


        Point aPos( aWinSize.Width() - nScrBarWidth, mnHeaderHeight );
        Size aSize( nScrBarWidth, aWinSize.Height() - mnHeaderHeight );

        mpScrBar->SetPosSizePixel( aPos, aSize );
        mpScrBar->SetRangeMax( (nCurCount+mnCols-1)*mnFineness/mnCols);
        mpScrBar->SetVisibleSize( mnVisLines );
        if (!bScrollBarUsed)
            mpScrBar->SetThumbPos( (long)mnFirstLine*mnFineness );
        long nPageSize = mnVisLines;
        if ( nPageSize < 1 )
            nPageSize = 1;
        mpScrBar->SetPageSize( nPageSize );
        mpScrBar->Show( mbScroll );
    }

    // delete ScrollBar
    pDelScrBar.disposeAndClear();
}

size_t ThumbnailView::ImplGetItem( const Point& rPos ) const
{
    if ( !mbHasVisibleItems )
    {
        return THUMBNAILVIEW_ITEM_NOTFOUND;
    }

    for (size_t i = 0; i < mFilteredItemList.size(); ++i)
    {
        if (mFilteredItemList[i]->isVisible() && mFilteredItemList[i]->getDrawArea().IsInside(rPos))
            return i;
    }

    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

ThumbnailViewItem* ThumbnailView::ImplGetItem( size_t nPos )
{
    return ( nPos < mFilteredItemList.size() ) ? mFilteredItemList[nPos] : nullptr;
}

sal_uInt16 ThumbnailView::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        if ( mItemList[n]->isVisible() )
            ++nRet;
    }

    return nRet;
}

ThumbnailViewItem* ThumbnailView::ImplGetVisibleItem( sal_uInt16 nVisiblePos )
{
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        ThumbnailViewItem *const pItem = mItemList[n];

        if ( pItem->isVisible() && !nVisiblePos-- )
            return pItem;
    }

    return nullptr;
}

void ThumbnailView::ImplFireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue )
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( false ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool ThumbnailView::ImplHasAccessibleListeners()
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( false ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK_TYPED( ThumbnailView,ImplScrollHdl, ScrollBar*, pScrollBar, void )
{
    if ( pScrollBar->GetDelta() )
    {
        CalculateItemPositions(true);

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::KeyInput( const KeyEvent& rKEvt )
{
    // Get the last selected item in the list
    size_t nLastPos = 0;
    bool bFoundLast = false;
    for ( long i = mFilteredItemList.size() - 1; !bFoundLast && i >= 0; --i )
    {
        ThumbnailViewItem* pItem = mFilteredItemList[i];
        if ( pItem->isSelected() )
        {
            nLastPos = i;
            bFoundLast = true;
        }
    }

    bool bValidRange = false;
    bool bHasSelRange = mpStartSelRange != mFilteredItemList.end();
    size_t nNextPos = nLastPos;
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    ThumbnailViewItem* pNext = nullptr;

    if (aKeyCode.IsShift() && bHasSelRange)
    {
        //If the last elemented selected is the start range position
        //search for the first selected item
        size_t nSelPos = mpStartSelRange - mFilteredItemList.begin();

        if (nLastPos == nSelPos)
        {
            while (nLastPos && mFilteredItemList[nLastPos-1]->isSelected())
                --nLastPos;
        }
    }

    switch ( aKeyCode.GetCode() )
    {
        case KEY_RIGHT:
            if (!mFilteredItemList.empty())
            {
                if ( bFoundLast && nLastPos + 1 < mFilteredItemList.size() )
                {
                    bValidRange = true;
                    nNextPos = nLastPos + 1;
                }

                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_LEFT:
            if (!mFilteredItemList.empty())
            {
                if ( nLastPos > 0 )
                {
                    bValidRange = true;
                    nNextPos = nLastPos - 1;
                }

                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_DOWN:
            if (!mFilteredItemList.empty())
            {
                if ( bFoundLast )
                {
                    //If we are in the second last row just go the one in
                    //the row below, if theres not row below just go to the
                    //last item but for the last row don't do anything.
                    if ( nLastPos + mnCols < mFilteredItemList.size( ) )
                    {
                        bValidRange = true;
                        nNextPos = nLastPos + mnCols;
                    }
                    else
                    {
                        int curRow = nLastPos/mnCols;

                        if (curRow < mnLines-1)
                            nNextPos = mFilteredItemList.size()-1;
                    }
                }

                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_UP:
            if (!mFilteredItemList.empty())
            {
                if ( nLastPos >= mnCols )
                {
                    bValidRange = true;
                    nNextPos = nLastPos - mnCols;
                }

                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_RETURN:
            {
                if ( bFoundLast )
                    OnItemDblClicked( mFilteredItemList[nLastPos] );
            }
            //fall-through
        default:
            Control::KeyInput( rKEvt );
    }

    if ( pNext )
    {
        if (aKeyCode.IsShift() && bValidRange)
        {
            std::pair<size_t,size_t> aRange;
            size_t nSelPos = mpStartSelRange - mFilteredItemList.begin();

            if (nLastPos < nSelPos)
            {
                if (nNextPos > nLastPos)
                {
                    if ( nNextPos > nSelPos)
                        aRange = std::make_pair(nLastPos,nNextPos);
                    else
                        aRange = std::make_pair(nLastPos,nNextPos-1);
                }
                else
                    aRange = std::make_pair(nNextPos,nLastPos-1);
            }
            else if (nLastPos == nSelPos)
            {
                if (nNextPos > nLastPos)
                    aRange = std::make_pair(nLastPos+1,nNextPos);
                else
                    aRange = std::make_pair(nNextPos,nLastPos-1);
            }
            else
            {
                if (nNextPos > nLastPos)
                    aRange = std::make_pair(nLastPos+1,nNextPos);
                else
                {
                    if ( nNextPos < nSelPos)
                        aRange = std::make_pair(nNextPos,nLastPos);
                    else
                        aRange = std::make_pair(nNextPos+1,nLastPos);
                }
            }

            for (size_t i = aRange.first; i <= aRange.second; ++i)
            {
                if (i != nSelPos)
                {
                    ThumbnailViewItem *pCurItem = mFilteredItemList[i];

                    pCurItem->setSelection(!pCurItem->isSelected());

                    if (pCurItem->isVisible())
                        DrawItem(pCurItem);

                    maItemStateHdl.Call(pCurItem);
                }
            }
        }
        else if (!aKeyCode.IsShift())
        {
            deselectItems();
            SelectItem(pNext->mnId);

            //Mark it as the selection range start position
            mpStartSelRange = mFilteredItemList.begin() + nNextPos;
        }

        MakeItemVisible(pNext->mnId);
    }
}

void ThumbnailView::MakeItemVisible( sal_uInt16 nItemId )
{
    // Get the item row
    size_t nPos = 0;
    bool bFound = false;
    for ( size_t i = 0; !bFound && i < mFilteredItemList.size(); ++i )
    {
        ThumbnailViewItem* pItem = mFilteredItemList[i];
        if ( pItem->mnId == nItemId )
        {
            nPos = i;
            bFound = true;
        }
    }
    sal_uInt16 nRow = mnCols ? nPos / mnCols : 0;

    // Move the visible rows as little as possible to include that one
    if ( nRow < mnFirstLine )
        mnFirstLine = nRow;
    else if ( nRow > mnFirstLine + mnVisLines )
        mnFirstLine = nRow - mnVisLines;

    CalculateItemPositions();
    Invalidate();
}

void ThumbnailView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
    {
        Control::MouseButtonDown( rMEvt );
        return;
    }

    size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
    ThumbnailViewItem* pItem = ImplGetItem(nPos);

    if ( !pItem )
    {
        deselectItems();
        Control::MouseButtonDown( rMEvt );
        return;
    }

    if ( rMEvt.GetClicks() == 2 )
    {
        OnItemDblClicked(pItem);
        return;
    }

    if ( rMEvt.GetClicks() == 1 )
    {
        if (rMEvt.IsMod1())
        {
            //Keep selected item group state and just invert current desired one state
            pItem->setSelection(!pItem->isSelected());

            //This one becomes the selection range start position if it changes its state to selected otherwise resets it
            mpStartSelRange = pItem->isSelected() ? mFilteredItemList.begin() + nPos : mFilteredItemList.end();
        }
        else if (rMEvt.IsShift() && mpStartSelRange != mFilteredItemList.end())
        {
            std::pair<size_t,size_t> aNewRange;
            aNewRange.first = mpStartSelRange - mFilteredItemList.begin();
            aNewRange.second = nPos;

            if (aNewRange.first > aNewRange.second)
                std::swap(aNewRange.first,aNewRange.second);

            //Deselect the ones outside of it
            for (size_t i = 0, n = mFilteredItemList.size(); i < n; ++i)
            {
                ThumbnailViewItem *pCurItem  = mFilteredItemList[i];

                if (pCurItem->isSelected() && (i < aNewRange.first || i > aNewRange.second))
                {
                    pCurItem->setSelection(false);

                    if (pCurItem->isVisible())
                        DrawItem(pCurItem);

                    maItemStateHdl.Call(pCurItem);
                }
            }

            size_t nSelPos = mpStartSelRange - mFilteredItemList.begin();

            //Select the items between start range and the selected item
            if (nSelPos != nPos)
            {
                int dir = nSelPos < nPos ? 1 : -1;
                size_t nCurPos = nSelPos + dir;

                while (nCurPos != nPos)
                {
                    ThumbnailViewItem *pCurItem  = mFilteredItemList[nCurPos];

                    if (!pCurItem->isSelected())
                    {
                        pCurItem->setSelection(true);

                        if (pCurItem->isVisible())
                            DrawItem(pCurItem);

                        maItemStateHdl.Call(pCurItem);
                    }

                    nCurPos += dir;
                }
            }

            pItem->setSelection(true);
        }
        else
        {
            //If we got a group of selected items deselect the rest and only keep the desired one
            //mark items as not selected to not fire unnecessary change state events.
            pItem->setSelection(false);
            deselectItems();
            pItem->setSelection(true);

            //Mark as initial selection range position and reset end one
            mpStartSelRange = mFilteredItemList.begin() + nPos;
        }

        if (pItem->isSelected())
        {
            bool bClickOnTitle = pItem->getTextArea().IsInside(rMEvt.GetPosPixel());
            pItem->setEditTitle(bClickOnTitle);
        }

        if (!pItem->isHighlighted())
            DrawItem(pItem);

        maItemStateHdl.Call(pItem);

        //fire accessible event??
    }
}

void ThumbnailView::MouseButtonUp( const MouseEvent& rMEvt )
{
    Control::MouseButtonUp( rMEvt );
}

void ThumbnailView::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == CommandEventId::Wheel) ||
         (rCEvt.GetCommand() == CommandEventId::StartAutoScroll) ||
         (rCEvt.GetCommand() == CommandEventId::AutoScroll) )
    {
        if ( HandleScrollCommand( rCEvt, nullptr, mpScrBar ) )
            return;
    }

    Control::Command( rCEvt );
}

void ThumbnailView::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    size_t nItemCount = mItemList.size();

    // Draw background
    drawinglayer::primitive2d::Primitive2DContainer aSeq(1);
    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonColorPrimitive2D(
                    B2DPolyPolygon( ::tools::Polygon(Rectangle(Point(), GetOutputSizePixel()), 0, 0).getB2DPolygon()),
                    maFillColor.getBColor()));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));
    pProcessor->process(aSeq);

    // draw items
    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *const pItem = mItemList[i];

        if (pItem->isVisible())
        {
            pItem->Paint(pProcessor.get(), mpItemAttrs);
        }
    }

    if (mpScrBar && mpScrBar->IsVisible())
        mpScrBar->Invalidate(rRect);
}

void ThumbnailView::GetFocus()
{
    // Select the first item if nothing selected
    int nSelected = -1;
    for (size_t i = 0, n = mItemList.size(); i < n && nSelected == -1; ++i)
    {
        if (mItemList[i]->isSelected())
            nSelected = i;
    }

    if (nSelected == -1 && mItemList.size() > 0)
    {
        SelectItem(1);
    }

    // Tell the accessible object that we got the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( false ) );
    if( pAcc )
        pAcc->GetFocus();

    Control::GetFocus();
}

void ThumbnailView::LoseFocus()
{
    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( false ) );
    if( pAcc )
        pAcc->LoseFocus();
}

void ThumbnailView::Resize()
{
    Control::Resize();
    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Text )
    {
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        Invalidate();
    }
    else if ( (nType == StateChangedType::Style) || (nType == StateChangedType::Enable) )
    {
        Invalidate();
    }
}

void ThumbnailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        Invalidate();
    }
}

void ThumbnailView::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    if ( nPos < mFilteredItemList.size() ) {

        // delete item from the thumbnail list
        for (size_t i = 0, n = mItemList.size(); i < n; ++i)
        {
            if (mItemList[i]->mnId == nItemId)
            {
                mItemList.erase(mItemList.begin()+i);
                break;
            }
        }

        // delete item from the filter item list
        ThumbnailValueItemList::iterator it = mFilteredItemList.begin();
        ::std::advance( it, nPos );

        if ((*it)->isSelected())
        {
            (*it)->setSelection(false);
            maItemStateHdl.Call(*it);
        }

        delete *it;
        mFilteredItemList.erase( it );
        mpStartSelRange = mFilteredItemList.end();
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::updateItems (const std::vector<ThumbnailViewItem*> &items)
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    mItemList = items;

    filterItems(maFilterFunc);
}

size_t ThumbnailView::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mFilteredItemList.size(); i < n; ++i ) {
        if ( mFilteredItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

sal_uInt16 ThumbnailView::GetItemId( size_t nPos ) const
{
    return ( nPos < mFilteredItemList.size() ) ? mFilteredItemList[nPos]->mnId : 0 ;
}

sal_uInt16 ThumbnailView::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

sal_uInt16 ThumbnailView::getNextItemId() const
{
    return mItemList.empty() ? 1 : mItemList.back()->mnId + 1;
}

void ThumbnailView::setItemMaxTextLength(sal_uInt32 nLength)
{
    mpItemAttrs->nMaxTextLength = nLength;
}

void ThumbnailView::setItemDimensions(long itemWidth, long thumbnailHeight, long displayHeight, int itemPadding)
{
    mnItemWidth = itemWidth + 2*itemPadding;
    mnThumbnailHeight = thumbnailHeight;
    mnDisplayHeight = displayHeight;
    mnItemPadding = itemPadding;
    mnItemHeight = mnDisplayHeight + mnThumbnailHeight + 2*itemPadding;
}

void ThumbnailView::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    ThumbnailViewItem* pItem = mFilteredItemList[nItemPos];
    if (!pItem->isSelected())
    {
        pItem->setSelection(true);
        maItemStateHdl.Call(pItem);

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        bool bNewOut = IsReallyVisible() && IsUpdateMode();

        // if necessary scroll to the visible area
        if ( mbScroll && nItemId )
        {
            sal_uInt16 nNewLine = (sal_uInt16)(nItemPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
            }
            else if ( nNewLine > (sal_uInt16)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (sal_uInt16)(nNewLine-mnVisLines+1);
            }
        }

        if ( bNewOut )
        {
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }

        if( ImplHasAccessibleListeners() )
        {
            // focus event (select)
            ThumbnailViewItemAcc* pItemAcc = ThumbnailViewItemAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                css::uno::Any aOldAny, aNewAny;
                if( !mbIsTransientChildrenDisabled )
                {
                    aNewAny <<= css::uno::Reference< css::uno::XInterface >(
                        static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
                else
                {
                    aNewAny <<= css::accessibility::AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent( css::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                }
            }

            // selection event
            css::uno::Any aOldAny, aNewAny;
            ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
        }
    }
}

bool ThumbnailView::IsItemSelected( sal_uInt16 nItemId ) const
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return false;

    ThumbnailViewItem* pItem = mFilteredItemList[nItemPos];
    return pItem->isSelected();
}

void ThumbnailView::deselectItems()
{
    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        if (mItemList[i]->isSelected())
        {
            mItemList[i]->setEditTitle(false);
            mItemList[i]->setSelection(false);

            maItemStateHdl.Call(mItemList[i]);
        }
    }

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void ThumbnailView::ShowTooltips( bool bShowTooltips )
{
    mbShowTooltips = bShowTooltips;
}

void ThumbnailView::filterItems(const std::function<bool (const ThumbnailViewItem*)> &func)
{
    mnFirstLine = 0;        // start at the top of the list instead of the current position
    maFilterFunc = func;

    size_t nSelPos = 0;
    bool bHasSelRange = false;
    ThumbnailViewItem *curSel = mpStartSelRange != mFilteredItemList.end() ? *mpStartSelRange : nullptr;

    mFilteredItemList.clear();

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        ThumbnailViewItem *const pItem = mItemList[i];

        if (maFilterFunc(pItem))
        {
            if (curSel == pItem)
            {
                nSelPos = i;
                bHasSelRange = true;
            }

            mFilteredItemList.push_back(pItem);
        }
        else
        {
            if( pItem->isVisible())
            {
                if ( ImplHasAccessibleListeners() )
                {
                    css::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);
                pItem->setSelection(false);

                maItemStateHdl.Call(pItem);
            }
        }
    }

    mpStartSelRange = bHasSelRange ? mFilteredItemList.begin()  + nSelPos : mFilteredItemList.end();
    CalculateItemPositions();

    Invalidate();
}

void ThumbnailView::sortItems(const std::function<bool (const ThumbnailViewItem*, const ThumbnailViewItem*)> &func)
{
    std::sort(mItemList.begin(),mItemList.end(),func);

    CalculateItemPositions();

    Invalidate();
}

bool ThumbnailView::renameItem(ThumbnailViewItem*, const OUString&)
{
    // Do nothing by default
    return false;
}

BitmapEx ThumbnailView::readThumbnail(const OUString &msURL)
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    // Load the thumbnail from a template document.
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    try
    {
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory = embed::StorageFactory::create(xContext);

        uno::Sequence<uno::Any> aArgs (2);
        aArgs[0] <<= msURL;
        aArgs[1] <<= embed::ElementModes::READ;
        uno::Reference<embed::XStorage> xDocStorage (
            xStorageFactory->createInstanceWithArguments(aArgs),
            uno::UNO_QUERY);

        try
        {
            if (xDocStorage.is())
            {
                uno::Reference<embed::XStorage> xStorage (
                    xDocStorage->openStorageElement(
                        "Thumbnails",
                        embed::ElementModes::READ));
                if (xStorage.is())
                {
                    uno::Reference<io::XStream> xThumbnailCopy (
                        xStorage->cloneStreamElement("thumbnail.png"));
                    if (xThumbnailCopy.is())
                        xIStream = xThumbnailCopy->getInputStream();
                }
            }
        }
        catch (const uno::Exception& rException)
        {
            OSL_TRACE (
                "caught exception while trying to access Thumbnail/thumbnail.png of %s: %s",
                OUStringToOString(msURL,
                    RTL_TEXTENCODING_UTF8).getStr(),
                OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }

        try
        {
            // An (older) implementation had a bug - The storage
            // name was "Thumbnail" instead of "Thumbnails".  The
            // old name is still used as fallback but this code can
            // be removed soon.
            if ( ! xIStream.is())
            {
                uno::Reference<embed::XStorage> xStorage (
                    xDocStorage->openStorageElement( "Thumbnail",
                        embed::ElementModes::READ));
                if (xStorage.is())
                {
                    uno::Reference<io::XStream> xThumbnailCopy (
                        xStorage->cloneStreamElement("thumbnail.png"));
                    if (xThumbnailCopy.is())
                        xIStream = xThumbnailCopy->getInputStream();
                }
            }
        }
        catch (const uno::Exception& rException)
        {
            OSL_TRACE (
                "caught exception while trying to access Thumbnails/thumbnail.png of %s: %s",
                OUStringToOString(msURL,
                    RTL_TEXTENCODING_UTF8).getStr(),
                OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    catch (const uno::Exception& rException)
    {
        OSL_TRACE (
            "caught exception while trying to access tuhmbnail of %s: %s",
            OUStringToOString(msURL,
                RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
    }

    // Extract the image from the stream.
    BitmapEx aThumbnail;
    if (xIStream.is())
    {
        std::unique_ptr<SvStream> pStream (
            ::utl::UcbStreamHelper::CreateStream (xIStream));
        vcl::PNGReader aReader (*pStream);
        aThumbnail = aReader.Read ();
    }

    // Note that the preview is returned without scaling it to the desired
    // width.  This gives the caller the chance to take advantage of a
    // possibly larger resolution then was asked for.
    return aThumbnail;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
