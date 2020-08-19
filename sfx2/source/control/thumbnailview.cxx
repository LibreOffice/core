/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/thumbnailview.hxx>
#include <thumbnailviewitem.hxx>

#include <utility>

#include "thumbnailviewacc.hxx"

#include <basegfx/color/bcolortools.hxx>
#include <comphelper/processfactory.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <memory>

using namespace basegfx;
using namespace basegfx::utils;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

constexpr int gnFineness = 5;

ThumbnailView::ThumbnailView (vcl::Window *pParent)
    : Control( pParent, WB_TABSTOP )
    , mpItemAttrs(new ThumbnailItemAttributes)
{
    ImplInit();
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
    mpItemAttrs.reset();

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

        ::tools::Rectangle aToInvalidate(pItem->updateHighlight(pItem->mbVisible && !rMEvt.IsLeaveWindow(), aPoint));

        if (!aToInvalidate.IsEmpty() && IsReallyVisible() && IsUpdateMode())
            Invalidate(aToInvalidate);
    }

    if (mbShowTooltips)
        SetQuickHelpText(aHelp);
}

void ThumbnailView::AppendItem(std::unique_ptr<ThumbnailViewItem> pItem)
{
    if (maFilterFunc(pItem.get()))
    {
        // Save current start,end range, iterator might get invalidated
        size_t nSelStartPos = 0;
        ThumbnailViewItem *pSelStartItem = nullptr;

        if (mpStartSelRange != mFilteredItemList.end())
        {
            pSelStartItem = *mpStartSelRange;
            nSelStartPos = mpStartSelRange - mFilteredItemList.begin();
        }

        mFilteredItemList.push_back(pItem.get());
        mpStartSelRange = pSelStartItem != nullptr ? mFilteredItemList.begin() + nSelStartPos : mFilteredItemList.end();
    }

    mItemList.push_back(std::move(pItem));
}

void ThumbnailView::ImplInit()
{
    mpScrBar = nullptr;
    mnItemWidth = 0;
    mnItemHeight = 0;
    mnItemPadding = 0;
    mnVisLines = 0;
    mnLines = 0;
    mnFirstLine = 0;
    mnCols = 0;
    mbScroll = false;
    mbHasVisibleItems = false;
    mbShowTooltips = false;
    maFilterFunc = ViewFilterAll();
    maFillColor = GetSettings().GetStyleSettings().GetFieldColor();
    maTextColor = GetSettings().GetStyleSettings().GetWindowTextColor();
    maHighlightColor = GetSettings().GetStyleSettings().GetHighlightColor();
    maHighlightTextColor = GetSettings().GetStyleSettings().GetHighlightTextColor();
    maSelectHighlightColor = GetSettings().GetStyleSettings().GetActiveColor();
    maSelectHighlightTextColor = GetSettings().GetStyleSettings().GetActiveTextColor();

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
        ThumbnailViewItem *const pItem = mItemList[i].get();

        // deselect all current selected items and fire events
        if (pItem->isSelected())
        {
            pItem->setSelection(false);
            // fire accessible event???
        }

        if ( pItem->isVisible() && ImplHasAccessibleListeners() )
        {
            css::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( false );
            ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        mItemList[i].reset();
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
    mpItemAttrs->aSelectHighlightColor = maSelectHighlightColor.getBColor();
    mpItemAttrs->aSelectHighlightTextColor = maSelectHighlightTextColor.getBColor();
    mpItemAttrs->fHighlightTransparence = mfHighlightTransparence;
    mpItemAttrs->aFontAttr = getFontAttributeFromVclFont(mpItemAttrs->aFontSize,GetFont(),false,true);
    mpItemAttrs->nMaxTextLength = 0;
}

void ThumbnailView::DrawItem(ThumbnailViewItem const *pItem)
{
    if (pItem->isVisible())
    {
        ::tools::Rectangle aRect = pItem->getDrawArea();

        if (!aRect.IsEmpty())
            Invalidate(aRect);
    }
}

void ThumbnailView::OnItemDblClicked (ThumbnailViewItem*)
{
}

css::uno::Reference< css::accessibility::XAccessible > ThumbnailView::CreateAccessible()
{
    return new ThumbnailViewAcc( this );
}

css::uno::Reference< css::accessibility::XAccessible > ThumbnailView::getAccessible()
{
    return GetAccessible();
}

void ThumbnailView::CalculateItemPositions (bool bScrollBarUsed)
{
    if (!mnItemHeight || !mnItemWidth)
        return;

    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mFilteredItemList.size();
    WinBits     nStyle = GetStyle();
    VclPtr<ScrollBar>  pDelScrBar;
    long        nScrBarWidth = 0;

    // consider the scrolling
    if ( nStyle & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = VclPtr<ScrollBar>::Create( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ThumbnailView, ImplScrollHdl ) );
        }

        // adapt the width because of the changed settings
        nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    }
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
                        static_cast<float>(mpScrBar->GetRangeMax() - mpScrBar->GetVisibleSize());
    else
        nScrollRatio = 0;

    // calculate maximum number of visible columns
    mnCols = static_cast<sal_uInt16>((aWinSize.Width()-nScrBarWidth) / mnItemWidth);

    if (!mnCols)
        mnCols = 1;

    // calculate maximum number of visible rows
    mnVisLines = static_cast<sal_uInt16>(aWinSize.Height() / mnItemHeight);

    // calculate empty space
    long nHSpace = aWinSize.Width()-nScrBarWidth - mnCols*mnItemWidth;
    long nVSpace = aWinSize.Height() - mnVisLines*mnItemHeight;
    long nHItemSpace = nHSpace / (mnCols+1);
    long nVItemSpace = nVSpace / (mnVisLines+1);

    // calculate maximum number of rows
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;

    if ( !mnLines )
        mnLines = 1;

    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else if ( mnFirstLine > o3tl::make_unsigned(mnLines-mnVisLines) )
        mnFirstLine = static_cast<sal_uInt16>(mnLines-mnVisLines);

    mbHasVisibleItems = true;

    long nFullSteps = (mnLines > mnVisLines) ? mnLines - mnVisLines + 1 : 1;

    long nItemHeightOffset = mnItemHeight + nVItemSpace;
    long nHiddenLines = static_cast<long>((nFullSteps - 1) * nScrollRatio);

    // calculate offsets
    long nStartX = nHItemSpace;
    long nStartY = nVItemSpace;

    // calculate and draw items
    long x = nStartX;
    long y = nStartY - ((nFullSteps - 1) * nScrollRatio - nHiddenLines) * nItemHeightOffset;

    // draw items
    // Unless we are scrolling (via scrollbar) we just use the precalculated
    // mnFirstLine -- our nHiddenLines calculation takes into account only
    // what the user has done with the scrollbar but not any changes of selection
    // using the keyboard, meaning we could accidentally hide the selected item
    // if we believe the scrollbar (fdo#72287).
    size_t nFirstItem = (bScrollBarUsed ? nHiddenLines : mnFirstLine) * mnCols;
    size_t nLastItem = nFirstItem + (mnVisLines + 1) * mnCols;

    // If want also draw parts of items in the last line,
    // then we add one more line if parts of this line are visible

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

                    aNewAny <<= pItem->GetAccessible( false );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(true);
            }

            pItem->setDrawArea(::tools::Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
            pItem->calculateItemsPosition(mnThumbnailHeight,mnItemPadding,mpItemAttrs->nMaxTextLength,mpItemAttrs.get());

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

                    aOldAny <<= pItem->GetAccessible( false );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);
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


        Point aPos( aWinSize.Width() - nScrBarWidth, 0 );
        Size aSize( nScrBarWidth, aWinSize.Height() );

        mpScrBar->SetPosSizePixel( aPos, aSize );
        mpScrBar->SetRangeMax(mnLines * gnFineness);
        mpScrBar->SetVisibleSize(mnVisLines * gnFineness);
        if (!bScrollBarUsed)
            mpScrBar->SetThumbPos( static_cast<long>(mnFirstLine)*gnFineness );
        long nPageSize = mnVisLines;
        if ( nPageSize < 1 )
            nPageSize = 1;
        mpScrBar->SetPageSize( nPageSize );
        mpScrBar->Show( mbScroll );
        mpScrBar->Enable( mbScroll );
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
        ThumbnailViewItem *const pItem = mItemList[n].get();

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

IMPL_LINK( ThumbnailView,ImplScrollHdl, ScrollBar*, pScrollBar, void )
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
        //If the last element selected is the start range position
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
                    //the row below, if there's not row below just go to the
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
            [[fallthrough]];
        default:
            Control::KeyInput( rKEvt );
    }

    if ( !pNext )
        return;

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

    if(rMEvt.GetClicks() != 1)
        return;

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

    if (!pItem->isHighlighted())
        DrawItem(pItem);

    //fire accessible event??
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

void ThumbnailView::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    size_t nItemCount = mItemList.size();

    // Draw background
    drawinglayer::primitive2d::Primitive2DContainer aSeq(1);
    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonColorPrimitive2D(
                    B2DPolyPolygon( ::tools::Polygon(::tools::Rectangle(Point(), GetOutputSizePixel()), 0, 0).getB2DPolygon()),
                    maFillColor.getBColor()));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));
    pProcessor->process(aSeq);

    // draw items
    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *const pItem = mItemList[i].get();

        if (pItem->isVisible())
        {
            pItem->Paint(pProcessor.get(), mpItemAttrs.get());
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

    if (nSelected == -1 && !mItemList.empty())
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

void ThumbnailView::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::updateItems (std::vector<std::unique_ptr<ThumbnailViewItem>> items)
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    mItemList = std::move(items);

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
    if (pItem->isSelected())
        return;

    pItem->setSelection(true);

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    bool bNewOut = IsReallyVisible() && IsUpdateMode();

    // if necessary scroll to the visible area
    if (mbScroll && nItemId && mnCols)
    {
        sal_uInt16 nNewLine = static_cast<sal_uInt16>(nItemPos / mnCols);
        if ( nNewLine < mnFirstLine )
        {
            mnFirstLine = nNewLine;
        }
        else if ( mnVisLines != 0 && nNewLine > o3tl::make_unsigned(mnFirstLine+mnVisLines-1) )
        {
            mnFirstLine = static_cast<sal_uInt16>(nNewLine-mnVisLines+1);
        }
    }

    if ( bNewOut )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }

    if( !ImplHasAccessibleListeners() )
        return;

    // focus event (select)
    ThumbnailViewItemAcc* pItemAcc = ThumbnailViewItemAcc::getImplementation( pItem->GetAccessible( false ) );

    if( pItemAcc )
    {
        css::uno::Any aOldAny, aNewAny;
        aNewAny <<= css::uno::Reference< css::uno::XInterface >(
            static_cast< ::cppu::OWeakObject* >( pItemAcc ));
        ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
    }

    // selection event
    css::uno::Any aOldAny, aNewAny;
    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
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
    for (std::unique_ptr<ThumbnailViewItem>& p : mItemList)
    {
        if (p->isSelected())
        {
            p->setSelection(false);
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
        ThumbnailViewItem *const pItem = mItemList[i].get();

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

                    aOldAny <<= pItem->GetAccessible( false );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);
                pItem->setSelection(false);
            }
        }
    }

    mpStartSelRange = bHasSelRange ? mFilteredItemList.begin()  + nSelPos : mFilteredItemList.end();
    CalculateItemPositions();

    Invalidate();
}

bool ThumbnailViewBase::renameItem(ThumbnailViewItem*, const OUString&)
{
    // Do nothing by default
    return false;
}

ThumbnailViewBase::~ThumbnailViewBase()
{
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
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("sfx",
                "caught exception while trying to access Thumbnail/thumbnail.png of " << msURL);
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
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("sfx",
                "caught exception while trying to access Thumbnails/thumbnail.png of " << msURL);
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sfx",
            "caught exception while trying to access thumbnail of "
            << msURL);
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

SfxThumbnailView::SfxThumbnailView(std::unique_ptr<weld::ScrolledWindow> xWindow, std::unique_ptr<weld::Menu> xMenu)
    : mnThumbnailHeight(0)
    , mnDisplayHeight(0)
    , mpItemAttrs(new ThumbnailItemAttributes)
    , mxScrolledWindow(std::move(xWindow))
    , mxContextMenu(std::move(xMenu))
{
    mxScrolledWindow->set_user_managed_scrolling();
    ImplInit();
    mxScrolledWindow->connect_vadjustment_changed(LINK(this, SfxThumbnailView, ImplScrollHdl));
}

SfxThumbnailView::~SfxThumbnailView()
{
    css::uno::Reference< css::lang::XComponent> xComponent(mxAccessible, css::uno::UNO_QUERY);

    if (xComponent.is())
        xComponent->dispose();

    mpItemAttrs.reset();

    ImplDeleteItems();
}

bool SfxThumbnailView::MouseMove(const MouseEvent& rMEvt)
{
    size_t nItemCount = mFilteredItemList.size();
    Point aPoint = rMEvt.GetPosPixel();

    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *pItem = mFilteredItemList[i];
        ::tools::Rectangle aToInvalidate(pItem->updateHighlight(pItem->mbVisible && !rMEvt.IsLeaveWindow(), aPoint));
        if (!aToInvalidate.IsEmpty() && IsReallyVisible() && IsUpdateMode())
            Invalidate(aToInvalidate);
    }

    return true;
}

OUString SfxThumbnailView::RequestHelp(tools::Rectangle& rHelpRect)
{
    if (!mbShowTooltips)
        return OUString();

    Point aPos = rHelpRect.TopLeft();
    size_t nItemCount = mFilteredItemList.size();
    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *pItem = mFilteredItemList[i];
        if (!pItem->mbVisible)
            continue;
        const tools::Rectangle& rDrawArea = pItem->getDrawArea();
        if (pItem->mbVisible && rDrawArea.IsInside(aPos))
        {
            rHelpRect = rDrawArea;
            return pItem->getHelpText();
        }
    }

    return OUString();
}

void SfxThumbnailView::AppendItem(std::unique_ptr<ThumbnailViewItem> pItem)
{
    if (maFilterFunc(pItem.get()))
    {
        // Save current start,end range, iterator might get invalidated
        size_t nSelStartPos = 0;
        ThumbnailViewItem *pSelStartItem = nullptr;

        if (mpStartSelRange != mFilteredItemList.end())
        {
            pSelStartItem = *mpStartSelRange;
            nSelStartPos = mpStartSelRange - mFilteredItemList.begin();
        }

        mFilteredItemList.push_back(pItem.get());
        mpStartSelRange = pSelStartItem != nullptr ? mFilteredItemList.begin() + nSelStartPos : mFilteredItemList.end();
    }

    mItemList.push_back(std::move(pItem));
}

void SfxThumbnailView::ImplInit()
{
    mnItemWidth = 0;
    mnItemHeight = 0;
    mnItemPadding = 0;
    mnVisLines = 0;
    mnLines = 0;
    mnFirstLine = 0;
    mnCols = 0;
    mbScroll = false;
    mbHasVisibleItems = false;
    mbShowTooltips = false;
    mbIsMultiSelectionEnabled = true;
    maFilterFunc = ViewFilterAll();

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    maFillColor = rSettings.GetFieldColor();
    maTextColor = rSettings.GetWindowTextColor();
    maHighlightColor = rSettings.GetHighlightColor();
    maHighlightTextColor = rSettings.GetHighlightTextColor();
    maSelectHighlightColor = rSettings.GetActiveColor();
    maSelectHighlightTextColor = rSettings.GetActiveTextColor();

    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
    mfHighlightTransparence = aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01;

    mpStartSelRange = mFilteredItemList.end();

    mpItemAttrs->aFillColor = maFillColor.getBColor();
    mpItemAttrs->aTextColor = maTextColor.getBColor();
    mpItemAttrs->aHighlightColor = maHighlightColor.getBColor();
    mpItemAttrs->aHighlightTextColor = maHighlightTextColor.getBColor();
    mpItemAttrs->aSelectHighlightColor = maSelectHighlightColor.getBColor();
    mpItemAttrs->aSelectHighlightTextColor = maSelectHighlightTextColor.getBColor();
    mpItemAttrs->fHighlightTransparence = mfHighlightTransparence;

    mpItemAttrs->nMaxTextLength = 0;
}

void SfxThumbnailView::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ThumbnailViewItem *const pItem = mItemList[i].get();

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

            aOldAny <<= pItem->GetAccessible( false );
            ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        mItemList[i].reset();
    }

    mItemList.clear();
    mFilteredItemList.clear();

    mpStartSelRange = mFilteredItemList.end();
}

void SfxThumbnailView::DrawItem(ThumbnailViewItem const *pItem)
{
    if (pItem->isVisible())
    {
        ::tools::Rectangle aRect = pItem->getDrawArea();

        if (!aRect.IsEmpty())
            Invalidate(aRect);
    }
}

void SfxThumbnailView::OnItemDblClicked (ThumbnailViewItem*)
{
}

css::uno::Reference< css::accessibility::XAccessible > SfxThumbnailView::CreateAccessible()
{
    mxAccessible.set(new SfxThumbnailViewAcc(this));
    return mxAccessible;
}

css::uno::Reference< css::accessibility::XAccessible > SfxThumbnailView::getAccessible()
{
    return mxAccessible;
}

void SfxThumbnailView::CalculateItemPositions(bool bScrollBarUsed)
{
    if (!mnItemHeight || !mnItemWidth)
        return;

    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mFilteredItemList.size();

    // calculate window scroll ratio
    float nScrollRatio;
    if (bScrollBarUsed)
        nScrollRatio = static_cast<float>(mxScrolledWindow->vadjustment_get_value()) /
                        static_cast<float>(mxScrolledWindow->vadjustment_get_upper()-2);
    else
        nScrollRatio = 0;

    // calculate ScrollBar width
    long nScrBarWidth = mxScrolledWindow->get_vscroll_width();

    // calculate maximum number of visible columns
    mnCols = static_cast<sal_uInt16>((aWinSize.Width()-nScrBarWidth) / mnItemWidth);

    if (!mnCols)
        mnCols = 1;

    // calculate maximum number of visible rows
    mnVisLines = static_cast<sal_uInt16>(aWinSize.Height() / mnItemHeight);

    // calculate empty space
    long nHSpace = aWinSize.Width()-nScrBarWidth - mnCols*mnItemWidth;
    long nVSpace = aWinSize.Height() - mnVisLines*mnItemHeight;
    long nHItemSpace = nHSpace / (mnCols+1);
    long nVItemSpace = nVSpace / (mnVisLines+1);

    // calculate maximum number of rows
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;

    if ( !mnLines )
        mnLines = 1;

    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else if ( mnFirstLine > o3tl::make_unsigned(mnLines-mnVisLines) )
        mnFirstLine = static_cast<sal_uInt16>(mnLines-mnVisLines);

    mbHasVisibleItems = true;

    long nItemHeightOffset = mnItemHeight + nVItemSpace;
    long nHiddenLines = (static_cast<long>(
        ( mnLines - 1 ) * nItemHeightOffset * nScrollRatio ) -
        nVItemSpace ) /
        nItemHeightOffset;

    // calculate offsets
    long nStartX = nHItemSpace;
    long nStartY = nVItemSpace;

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
    // then we add one more line if parts of this line are visible

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

                    aNewAny <<= pItem->GetAccessible( false );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(true);

                maItemStateHdl.Call(pItem);
            }

            pItem->setDrawArea(::tools::Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
            pItem->calculateItemsPosition(mnThumbnailHeight,mnItemPadding,mpItemAttrs->nMaxTextLength,mpItemAttrs.get());

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

                    aOldAny <<= pItem->GetAccessible( false );
                    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);

                maItemStateHdl.Call(pItem);
            }

        }

        ++nCurCount;
    }

    // arrange ScrollBar, set values and show it
    mnLines = (nCurCount+mnCols-1)/mnCols;

    // check if scroll is needed
    mbScroll = mnLines > mnVisLines;

    mxScrolledWindow->vadjustment_set_upper((nCurCount+mnCols-1)*gnFineness/mnCols);
    mxScrolledWindow->vadjustment_set_page_size(mnVisLines);
    if (!bScrollBarUsed)
        mxScrolledWindow->vadjustment_set_value(static_cast<long>(mnFirstLine)*gnFineness);
    long nPageSize = mnVisLines;
    if ( nPageSize < 1 )
        nPageSize = 1;
    mxScrolledWindow->vadjustment_set_page_increment(nPageSize);
    mxScrolledWindow->set_vpolicy(mbScroll ? VclPolicyType::ALWAYS : VclPolicyType::NEVER);
}

size_t SfxThumbnailView::ImplGetItem( const Point& rPos ) const
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

ThumbnailViewItem* SfxThumbnailView::ImplGetItem( size_t nPos )
{
    return ( nPos < mFilteredItemList.size() ) ? mFilteredItemList[nPos] : nullptr;
}

sal_uInt16 SfxThumbnailView::ImplGetVisibleItemCount() const
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

ThumbnailViewItem* SfxThumbnailView::ImplGetVisibleItem( sal_uInt16 nVisiblePos )
{
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        ThumbnailViewItem *const pItem = mItemList[n].get();

        if ( pItem->isVisible() && !nVisiblePos-- )
            return pItem;
    }

    return nullptr;
}

void SfxThumbnailView::ImplFireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue )
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation(mxAccessible);

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool SfxThumbnailView::ImplHasAccessibleListeners()
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation(mxAccessible);
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK_NOARG(SfxThumbnailView, ImplScrollHdl, weld::ScrolledWindow&, void)
{
    CalculateItemPositions(true);
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

bool SfxThumbnailView::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = true;

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
        //If the last element selected is the start range position
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
                    //the row below, if there's not row below just go to the
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
            [[fallthrough]];
        default:
            bHandled = CustomWidgetController::KeyInput(rKEvt);
    }

    if ( pNext  && mbIsMultiSelectionEnabled)
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
    else if(pNext && !mbIsMultiSelectionEnabled)
    {
        deselectItems();
        SelectItem(pNext->mnId);
        MakeItemVisible(pNext->mnId);
    }
    return bHandled;
}

void SfxThumbnailView::MakeItemVisible( sal_uInt16 nItemId )
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

bool SfxThumbnailView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!rMEvt.IsLeft())
    {
        return CustomWidgetController::MouseButtonDown( rMEvt );
    }

    size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
    ThumbnailViewItem* pItem = ImplGetItem(nPos);

    if ( !pItem )
    {
        deselectItems();
        return CustomWidgetController::MouseButtonDown( rMEvt );
    }

    if ( rMEvt.GetClicks() == 2 )
    {
        OnItemDblClicked(pItem);
        return true;
    }

    if ( rMEvt.GetClicks() == 1 && !mbIsMultiSelectionEnabled )
    {
        deselectItems();
        pItem->setSelection(!pItem->isSelected());

        if (!pItem->isHighlighted())
            DrawItem(pItem);

        maItemStateHdl.Call(pItem);
    }
    else if(rMEvt.GetClicks() == 1)
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

        if (!pItem->isHighlighted())
            DrawItem(pItem);

        maItemStateHdl.Call(pItem);

        //fire accessible event??
    }
    return true;
}

void SfxThumbnailView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);

    if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
    {
        OutputDevice& rDevice = pDrawingArea->get_ref_device();
        pDefaultDevice->SetPointFont(rDevice, pDrawingArea->get_font());
        mpItemAttrs->aFontAttr = getFontAttributeFromVclFont(mpItemAttrs->aFontSize, rDevice.GetFont(), false, true);
    }

    SetOutputSizePixel(pDrawingArea->get_preferred_size());
}

void SfxThumbnailView::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& /*rRect*/)
{
    rRenderContext.Push(PushFlags::ALL);

    rRenderContext.SetTextFillColor();
    rRenderContext.SetBackground(maFillColor);

    size_t nItemCount = mItemList.size();

    // Draw background
    drawinglayer::primitive2d::Primitive2DContainer aSeq(1);
    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonColorPrimitive2D(
                    B2DPolyPolygon( ::tools::Polygon(::tools::Rectangle(Point(), GetOutputSizePixel()), 0, 0).getB2DPolygon()),
                    maFillColor.getBColor()));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));
    pProcessor->process(aSeq);

    // draw items
    for (size_t i = 0; i < nItemCount; i++)
    {
        ThumbnailViewItem *const pItem = mItemList[i].get();
        if (!pItem->isVisible())
            continue;
        pItem->Paint(pProcessor.get(), mpItemAttrs.get());
    }

    rRenderContext.Pop();
}

void SfxThumbnailView::GetFocus()
{
    // Select the first item if nothing selected
    int nSelected = -1;
    for (size_t i = 0, n = mItemList.size(); i < n && nSelected == -1; ++i)
    {
        if (mItemList[i]->isSelected())
            nSelected = i;
    }

    if (nSelected == -1 && !mItemList.empty())
    {
        SelectItem(1);
    }

    // Tell the accessible object that we got the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation(mxAccessible);
    if( pAcc )
        pAcc->GetFocus();

    CustomWidgetController::GetFocus();
}

void SfxThumbnailView::LoseFocus()
{
    CustomWidgetController::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation(mxAccessible);
    if( pAcc )
        pAcc->LoseFocus();
}

void SfxThumbnailView::Resize()
{
    CustomWidgetController::Resize();
    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void SfxThumbnailView::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    if ( nPos < mFilteredItemList.size() ) {

        // keep it alive until after we have deleted it from the filter item list
        std::unique_ptr<ThumbnailViewItem> xKeepAliveViewItem;

        // delete item from the thumbnail list
        for (auto it = mItemList.begin(); it != mItemList.end(); ++it)
        {
            if ((*it)->mnId == nItemId)
            {
                xKeepAliveViewItem = std::move(*it);
                mItemList.erase(it);
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

        mFilteredItemList.erase( it );
        mpStartSelRange = mFilteredItemList.end();
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void SfxThumbnailView::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void SfxThumbnailView::updateItems (std::vector<std::unique_ptr<ThumbnailViewItem>> items)
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;

    mItemList = std::move(items);

    filterItems(maFilterFunc);
}

size_t SfxThumbnailView::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mFilteredItemList.size(); i < n; ++i ) {
        if ( mFilteredItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

sal_uInt16 SfxThumbnailView::GetItemId( size_t nPos ) const
{
    return ( nPos < mFilteredItemList.size() ) ? mFilteredItemList[nPos]->mnId : 0 ;
}

sal_uInt16 SfxThumbnailView::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

sal_uInt16 SfxThumbnailView::getNextItemId() const
{
    return mItemList.empty() ? 1 : mItemList.back()->mnId + 1;
}

void SfxThumbnailView::setItemMaxTextLength(sal_uInt32 nLength)
{
    mpItemAttrs->nMaxTextLength = nLength;
}

void SfxThumbnailView::setItemDimensions(long itemWidth, long thumbnailHeight, long displayHeight, int itemPadding)
{
    mnItemWidth = itemWidth + 2*itemPadding;
    mnThumbnailHeight = thumbnailHeight;
    mnDisplayHeight = displayHeight;
    mnItemPadding = itemPadding;
    mnItemHeight = mnDisplayHeight + mnThumbnailHeight + 2*itemPadding;
}

void SfxThumbnailView::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    ThumbnailViewItem* pItem = mFilteredItemList[nItemPos];
    if (pItem->isSelected())
        return;

    pItem->setSelection(true);
    maItemStateHdl.Call(pItem);

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    bool bNewOut = IsReallyVisible() && IsUpdateMode();

    // if necessary scroll to the visible area
    if (mbScroll && nItemId && mnCols)
    {
        sal_uInt16 nNewLine = static_cast<sal_uInt16>(nItemPos / mnCols);
        if ( nNewLine < mnFirstLine )
        {
            mnFirstLine = nNewLine;
        }
        else if ( mnVisLines != 0 && nNewLine > o3tl::make_unsigned(mnFirstLine+mnVisLines-1) )
        {
            mnFirstLine = static_cast<sal_uInt16>(nNewLine-mnVisLines+1);
        }
    }

    if ( bNewOut )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }

    if( !ImplHasAccessibleListeners() )
        return;

    // focus event (select)
    ThumbnailViewItemAcc* pItemAcc = ThumbnailViewItemAcc::getImplementation( pItem->GetAccessible( false ) );

    if( pItemAcc )
    {
        css::uno::Any aOldAny, aNewAny;
        aNewAny <<= css::uno::Reference< css::uno::XInterface >(
            static_cast< ::cppu::OWeakObject* >( pItemAcc ));
        ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
    }

    // selection event
    css::uno::Any aOldAny, aNewAny;
    ImplFireAccessibleEvent( css::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
}

bool SfxThumbnailView::IsItemSelected( sal_uInt16 nItemId ) const
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return false;

    ThumbnailViewItem* pItem = mFilteredItemList[nItemPos];
    return pItem->isSelected();
}

void SfxThumbnailView::deselectItems()
{
    for (std::unique_ptr<ThumbnailViewItem>& p : mItemList)
    {
        if (p->isSelected())
        {
            p->setSelection(false);

            maItemStateHdl.Call(p.get());
        }
    }

    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();
}

void SfxThumbnailView::ShowTooltips( bool bShowTooltips )
{
    mbShowTooltips = bShowTooltips;
}

void SfxThumbnailView::SetMultiSelectionEnabled( bool bIsMultiSelectionEnabled )
{
    mbIsMultiSelectionEnabled = bIsMultiSelectionEnabled;
}

void SfxThumbnailView::filterItems(const std::function<bool (const ThumbnailViewItem*)> &func)
{
    mnFirstLine = 0;        // start at the top of the list instead of the current position
    maFilterFunc = func;

    size_t nSelPos = 0;
    bool bHasSelRange = false;
    ThumbnailViewItem *curSel = mpStartSelRange != mFilteredItemList.end() ? *mpStartSelRange : nullptr;

    mFilteredItemList.clear();

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        ThumbnailViewItem *const pItem = mItemList[i].get();

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

                    aOldAny <<= pItem->GetAccessible( false );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
