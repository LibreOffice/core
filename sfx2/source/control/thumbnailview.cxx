/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>

#include "thumbnailviewacc.hxx"

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <rtl/ustring.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

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
    SCRBAR_OFFSET = 1,
    SCROLL_OFFSET = 4
};

ThumbnailView::ThumbnailView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : Control( pParent, nWinStyle ),
    maColor( pParent->GetBackground().GetColor() )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::ThumbnailView (Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : Control( pParent, rResId ),
    maColor( pParent->GetBackground().GetColor() )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::~ThumbnailView()
{
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
            xComponent(GetAccessible(sal_False),
                       com::sun::star::uno::UNO_QUERY);

    if (xComponent.is())
        xComponent->dispose ();

    delete mpScrBar;
    delete mpItemAttrs;

    ImplDeleteItems();
}

void ThumbnailView::ImplInit()
{
    mpScrBar            = NULL;
    mnHeaderHeight      = 0;
    mnItemWidth         = 0;
    mnItemHeight        = 0;
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
    mbScroll            = false;
    mbHasVisibleItems   = false;
    mbSelectionMode = false;
    maFilterFunc = ViewFilterAll();

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    mpProcessor = drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(*this, aNewViewInfos );

    ImplInitSettings( true, true, true );
}

void ThumbnailView::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ThumbnailViewItem *const pItem = mItemList[i];
        if ( pItem->isVisible() && ImplHasAccessibleListeners() )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mItemList.clear();
}

void ThumbnailView::ImplInitSettings( bool bFont, bool bForeground, bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetButtonTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
    }

    mpItemAttrs = new ThumbnailItemAttributes;
    mpItemAttrs->aFillColor = maColor.getBColor();
    mpItemAttrs->aHighlightColor = rStyleSettings.GetHighlightColor().getBColor();
    mpItemAttrs->aFontAttr = getFontAttributeFromVclFont(mpItemAttrs->aFontSize,GetFont(),false,true);
}

void ThumbnailView::ImplInitScrollBar()
{
    if ( GetStyle() & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ThumbnailView, ImplScrollHdl ) );
        }
        else
        {
            // adapt the width because of the changed settings
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->SetPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

void ThumbnailView::DrawItem (ThumbnailViewItem *pItem)
{
    Rectangle aRect = pItem->getDrawArea();

    if ( (aRect.GetHeight() > 0) && (aRect.GetWidth() > 0) )
        pItem->Paint(mpProcessor,mpItemAttrs);
}

void ThumbnailView::OnSelectionMode (bool bMode)
{
    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        mItemList[i]->setSelectionMode(bMode);

        if (mItemList[i]->isVisible())
            DrawItem(mItemList[i]);
    }
}

void ThumbnailView::OnItemDblClicked (ThumbnailViewItem*)
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ThumbnailView::CreateAccessible()
{
    return new ThumbnailViewAcc( this, mbIsTransientChildrenDisabled );
}

void ThumbnailView::CalculateItemPositions ()
{
    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mItemList.size();
    WinBits     nStyle = GetStyle();
    long        nNoneHeight = 0;
    long        nNoneSpace = 0;
    ScrollBar*  pDelScrBar = NULL;

    // consider the scrolling
    if ( nStyle & WB_VSCROLL )
        ImplInitScrollBar();
    else
    {
        if ( mpScrBar )
        {
            // delete ScrollBar not until later, to prevent recursive calls
            pDelScrBar = mpScrBar;
            mpScrBar = NULL;
        }
    }

    // calculate ScrollBar width
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;

    // calculate number of columns
    if ( !mnUserCols )
    {
        if ( mnUserItemWidth )
        {
            mnCols = (sal_uInt16)((aWinSize.Width()-nScrBarWidth+mnSpacing) / (mnUserItemWidth+mnSpacing));
            if ( !mnCols )
                mnCols = 1;
        }
        else
            mnCols = 1;
    }
    else
        mnCols = mnUserCols;

    // calculate number of rows
    mbScroll = false;
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;
    if ( !mnLines )
        mnLines = 1;

    long nCalcHeight = aWinSize.Height()-nNoneHeight;
    if ( mnUserVisLines )
        mnVisLines = mnUserVisLines;
    else if ( mnUserItemHeight )
    {
        mnVisLines = (nCalcHeight-nNoneSpace+mnSpacing) / (mnUserItemHeight+mnSpacing);
        if ( !mnVisLines )
            mnVisLines = 1;
    }
    else
        mnVisLines = mnLines;
    if ( mnLines > mnVisLines )
        mbScroll = true;
    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else
    {
        if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
            mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);
    }

    // calculate item size
    const long nColSpace  = (mnCols-1)*mnSpacing;
    const long nLineSpace = ((mnVisLines-1)*mnSpacing)+nNoneSpace;
    if ( mnUserItemWidth && !mnUserCols )
    {
        mnItemWidth = mnUserItemWidth;
        if ( mnItemWidth > aWinSize.Width()-nScrBarWidth-nColSpace )
            mnItemWidth = aWinSize.Width()-nScrBarWidth-nColSpace;
    }
    else
        mnItemWidth = (aWinSize.Width()-nScrBarWidth-nColSpace) / mnCols;
    if ( mnUserItemHeight && !mnUserVisLines )
    {
        mnItemHeight = mnUserItemHeight;
        if ( mnItemHeight > nCalcHeight-nNoneSpace )
            mnItemHeight = nCalcHeight-nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        mnItemHeight = nCalcHeight / mnVisLines;
    }

    // nothing is changed in case of too small items
    if ( (mnItemWidth <= 0) ||
         (mnItemHeight <= 2) ||
         !nItemCount )
    {
        mbHasVisibleItems = false;

        for ( size_t i = 0; i < nItemCount; i++ )
        {
            if (mItemList[i]->isVisible())
                maItemStateHdl.Call(mItemList[i]);

            mItemList[i]->show(false);
        }

        if ( mpScrBar )
            mpScrBar->Hide();
    }
    else
    {
        mbHasVisibleItems = true;

        // calculate offsets
        long nStartX = 0;
        long nStartY = mnHeaderHeight;

        // calculate and draw items
        long x = nStartX;
        long y = nStartY;

        // draw items
        size_t nTotalItems = mnFirstLine*mnCols + mnVisLines*mnCols;

        maItemListRect.Left() = x;
        maItemListRect.Top() = y + mnHeaderHeight;
        maItemListRect.Right() = x + mnCols*(mnItemWidth+mnSpacing) - mnSpacing - 1;
        maItemListRect.Bottom() = y + mnVisLines*(mnItemHeight+mnSpacing) - mnSpacing - 1;

        // If want also draw parts of items in the last line,
        // then we add one more line if parts of these line are
        // visible
        if ( y+(mnVisLines*(mnItemHeight+mnSpacing)) < aWinSize.Height() )
            nTotalItems += mnCols;
        maItemListRect.Bottom() = aWinSize.Height() - y;

        size_t nCurCount = 0;
        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ThumbnailViewItem *const pItem = mItemList[i];

            if (maFilterFunc(pItem) && nCurCount < nTotalItems)
            {
                if( !pItem->isVisible() && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aNewAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                if (!mItemList[i]->isVisible())
                    maItemStateHdl.Call(mItemList[i]);

                pItem->show(true);
                pItem->setDrawArea(Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
                pItem->calculateItemsPosition();

                if ( !((nCurCount+1) % mnCols) )
                {
                    x = nStartX;
                    y += mnItemHeight+mnSpacing;
                }
                else
                    x += mnItemWidth+mnSpacing;

                ++nCurCount;
            }
            else
            {
                if( pItem->isVisible() && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                if (mItemList[i]->isVisible())
                    maItemStateHdl.Call(mItemList[i]);

                pItem->show(false);
            }
        }

        // arrange ScrollBar, set values and show it
        if ( mpScrBar )
        {
            Point   aPos( aWinSize.Width()-nScrBarWidth+SCRBAR_OFFSET, 0 );
            Size    aSize( nScrBarWidth-SCRBAR_OFFSET, aWinSize.Height() );

            mpScrBar->SetPosSizePixel( aPos, aSize );
            mpScrBar->SetRangeMax( mnLines );
            mpScrBar->SetVisibleSize( mnVisLines );
            mpScrBar->SetThumbPos( (long)mnFirstLine );
            long nPageSize = mnVisLines;
            if ( nPageSize < 1 )
                nPageSize = 1;
            mpScrBar->SetPageSize( nPageSize );
        }
    }

    // delete ScrollBar
    delete pDelScrBar;
}

bool ThumbnailView::ImplScroll( const Point& rPos )
{
    if ( !mbScroll || !maItemListRect.IsInside(rPos) )
        return false;

    const long nScrollOffset = (mnItemHeight <= 16) ? SCROLL_OFFSET/2 : SCROLL_OFFSET;
    bool bScroll = false;

    if ( rPos.Y() <= maItemListRect.Top()+nScrollOffset )
    {
        if ( mnFirstLine > 0 )
        {
            --mnFirstLine;
            bScroll = true;
        }
    }
    else if ( rPos.Y() >= maItemListRect.Bottom()-nScrollOffset )
    {
        if ( mnFirstLine < static_cast<sal_uInt16>(mnLines-mnVisLines) )
        {
            ++mnFirstLine;
            bScroll = true;
        }
    }

    if ( !bScroll )
        return false;

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();

    return true;
}

size_t ThumbnailView::ImplGetItem( const Point& rPos, bool bMove ) const
{
    if ( !mbHasVisibleItems )
    {
        return THUMBNAILVIEW_ITEM_NOTFOUND;
    }

    if ( maItemListRect.IsInside( rPos ) )
    {
        for (size_t i = 0; i < mItemList.size(); ++i)
        {
            if (mItemList[i]->isVisible() && mItemList[i]->getDrawArea().IsInside(rPos))
                return i;
        }

        // return the previously selected item if spacing is set and
        // the mouse hasn't left the window yet
        if ( bMove && mnSpacing && mnHighItemId )
        {
            return GetItemPos( mnHighItemId );
        }
    }

    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

ThumbnailViewItem* ThumbnailView::ImplGetItem( size_t nPos )
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos] : NULL;
}

ThumbnailViewItem* ThumbnailView::ImplGetFirstItem()
{
    return mItemList.empty() ? NULL : mItemList[0];
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

    return NULL;
}

void ThumbnailView::ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

bool ThumbnailView::ImplHasAccessibleListeners()
{
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

IMPL_LINK( ThumbnailView,ImplScrollHdl, ScrollBar*, pScrollBar )
{
    sal_uInt16 nNewFirstLine = (sal_uInt16)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;

        // Recalculate item positions
        Size aWinSize = GetOutputSizePixel();
        size_t nItemCount = mItemList.size();

        // calculate offsets
        long nStartX = 0;
        long nStartY = 0;

        // calculate items
        long x = nStartX;
        long y = nStartY;

        // draw items
        sal_uLong nFirstItem = mnFirstLine * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        // If want also draw parts of items in the last line,
        // then we add one more line if parts of these line are
        // visible
        if ( y+(mnVisLines*(mnItemHeight+mnSpacing)) < aWinSize.Height() )
            nLastItem += mnCols;

        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ThumbnailViewItem *const pItem = mItemList[i];

            if ( (i >= nFirstItem) && (i < nLastItem) )
            {
                if (!mItemList[i]->isVisible())
                    maItemStateHdl.Call(mItemList[i]);

                pItem->show(true);
                pItem->setDrawArea(Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
                pItem->calculateItemsPosition();

                if ( !((i+1) % mnCols) )
                {
                    x = nStartX;
                    y += mnItemHeight+mnSpacing;
                }
                else
                    x += mnItemWidth+mnSpacing;
            }
            else
            {
                if (mItemList[i]->isVisible())
                    maItemStateHdl.Call(mItemList[i]);

                pItem->show(false);
            }
        }

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    return 0;
}

IMPL_LINK_NOARG(ThumbnailView, ImplTimerHdl)
{
    ImplTracking( GetPointerPosPixel(), true );
    return 0;
}

void ThumbnailView::ImplTracking( const Point& rPos, bool bRepeat )
{
    if ( bRepeat )
    {
        if ( ImplScroll( rPos ) )
        {
        }
    }
}

void ThumbnailView::ImplEndTracking( const Point& rPos, bool bCancel )
{
}

IMPL_LINK (ThumbnailView, OnFolderSelected, ThumbnailViewItem*, pItem)
{
    return 0;
}

void ThumbnailView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        ThumbnailViewItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );

        if (pItem && pItem->isVisible())
        {
            if ( !rMEvt.IsMod2() )
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    if (mbSelectionMode)
                    {
                        pItem->setSelection(!pItem->isSelected());

                        if (!pItem->isHighlighted())
                            DrawItem(pItem);

                        maItemStateHdl.Call(pItem);
                    }
                    else
                    {
                        if (pItem->isInsideTitle(rMEvt.GetPosPixel()))
                        {
                            pItem->setSelection(!pItem->isSelected());

                            if (!pItem->isHighlighted())
                                DrawItem(pItem);

                            maItemStateHdl.Call(pItem);
                        }

                        //StartTracking( STARTTRACK_SCROLLREPEAT );
                    }
                }
                else if ( rMEvt.GetClicks() == 2 )
                    OnItemDblClicked(pItem);
            }

            return;
        }
    }

    Control::MouseButtonDown( rMEvt );
}

void ThumbnailView::MouseButtonUp( const MouseEvent& rMEvt )
{
    Control::MouseButtonUp( rMEvt );
}

void ThumbnailView::MouseMove( const MouseEvent& rMEvt )
{
    ThumbnailViewItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );

    if (pItem)
    {
        if (mnHighItemId != pItem->mnId && pItem->isVisible())
        {
            size_t nPos = GetItemPos(mnHighItemId);

            if (nPos != THUMBNAILVIEW_ITEM_NOTFOUND)
            {
                ThumbnailViewItem *pOld = mItemList[nPos];

                pOld->setHighlight(false);

                if (!pOld->isSelected())
                    DrawItem(pOld);
            }

            mnHighItemId = pItem->mnId;
            pItem->setHighlight(true);

            if (!pItem->isSelected())
                DrawItem(pItem);
        }
    }
    else
    {
        if (mnHighItemId)
        {
            size_t nPos = GetItemPos(mnHighItemId);

            if (nPos != THUMBNAILVIEW_ITEM_NOTFOUND)
            {
                ThumbnailViewItem *pOld = mItemList[nPos];

                pOld->setHighlight(false);

                if (!pOld->isSelected())
                    DrawItem(pOld);
            }

            mnHighItemId = 0;
        }
    }

    Control::MouseMove( rMEvt );
}

void ThumbnailView::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

void ThumbnailView::KeyInput( const KeyEvent& rKEvt )
{
    size_t nLastItem = mItemList.size();

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    --nLastItem;
    const size_t nCurPos = mnSelItemId ? GetItemPos( mnSelItemId ) : 0;
    size_t nItemPos = THUMBNAILVIEW_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            nItemPos = 0;
            break;

        case KEY_END:
            nItemPos = nLastItem;
            break;

        case KEY_LEFT:
            if (nCurPos != THUMBNAILVIEW_ITEM_NONEITEM)
            {
                if (nCurPos)
                {
                    nItemPos = nCurPos-1;
                }
            }
            break;

        case KEY_RIGHT:
            if (nCurPos < nLastItem)
                nItemPos = nCurPos+1;

            break;

        case KEY_PAGEUP:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_UP:
            if (nCurPos != THUMBNAILVIEW_ITEM_NONEITEM)
            {
                if (nCurPos == nLastItem)
                {
                    const size_t nCol = nLastItem % mnCols;
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
                else if (nCurPos > mnCols)
                {
                    // Go to same column in first row
                    nItemPos = nCurPos % mnCols;
                }
            }
            break;

        case KEY_PAGEDOWN:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_DOWN:
            if (nCurPos != nLastItem)
            {
                if (nCurPos == THUMBNAILVIEW_ITEM_NONEITEM)
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
            // intentional fall-through
        default:
            Control::KeyInput( rKEvt );
            return;
    }

    // This point is reached only if key travelling was used,
    // in which case selection mode should be switched off

    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
    {
        if ( nItemPos!=THUMBNAILVIEW_ITEM_NONEITEM && nItemPos<nLastItem )
        {
            // update current column only in case of a new position
            // which is also not a "specially" handled one.
            mnCurCol = nItemPos % mnCols;
        }
        const sal_uInt16 nItemId = (nItemPos != THUMBNAILVIEW_ITEM_NONEITEM) ? GetItemId( nItemPos ) : 0;
        if ( nItemId != mnSelItemId )
        {
            SelectItem( nItemId );
        }
    }
}

void ThumbnailView::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_WHEEL) ||
         (rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
         (rCEvt.GetCommand() == COMMAND_AUTOSCROLL) )
    {
        if ( HandleScrollCommand( rCEvt, NULL, mpScrBar ) )
            return;
    }

    Control::Command( rCEvt );
}

void ThumbnailView::Paint( const Rectangle &aRect)
{
    size_t      nItemCount = mItemList.size();

    // Draw background
    Primitive2DSequence aSeq(1);
    aSeq[0] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(Polygon(aRect,5,5).getB2DPolygon()),
                                        maColor.getBColor()));

    mpProcessor->process(aSeq);

    // draw items
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        ThumbnailViewItem *const pItem = mItemList[i];

        if ( pItem->isVisible() )
            DrawItem(pItem);
    }

    if ( mpScrBar && mpScrBar->IsVisible() )
        mpScrBar->Invalidate();
}

void ThumbnailView::GetFocus()
{
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->GetFocus();
}

void ThumbnailView::LoseFocus()
{
    if (mnHighItemId)
    {
        size_t nPos = GetItemPos(mnHighItemId);

        if (nPos != THUMBNAILVIEW_ITEM_NOTFOUND)
        {
            ThumbnailViewItem *pOld = mItemList[nPos];

            pOld->setHighlight(false);

            if (!pOld->isSelected())
                DrawItem(pOld);
        }

        mnHighItemId = 0;
    }

    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->LoseFocus();
}

void ThumbnailView::Resize()
{
    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    Control::Resize();
}

void ThumbnailView::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
    else if ( (nType == STATE_CHANGE_STYLE) || (nType == STATE_CHANGE_ENABLE) )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void ThumbnailView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

void ThumbnailView::InsertItem( sal_uInt16 nItemId, const BitmapEx& rImage,
                           const rtl::OUString& rText, size_t nPos )
{
    ThumbnailViewItem* pItem = new ThumbnailViewItem( *this, this );
    pItem->mnId     = nItemId;
    pItem->maPreview1 = rImage;
    pItem->maText   = rText;
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnFolderSelected));
    ImplInsertItem( pItem, nPos );
}

void ThumbnailView::ImplInsertItem( ThumbnailViewItem *const pItem, const size_t nPos )
{
    assert(pItem->mnId); // "ItemId == 0"
    assert(GetItemPos( pItem->mnId ) == THUMBNAILVIEW_ITEM_NOTFOUND); // ItemId already exists

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        mItemList.insert( it, pItem );
    } else {
        mItemList.push_back( pItem );
    }

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
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
    mnCurCol        = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

size_t ThumbnailView::GetItemCount() const
{
    return mItemList.size();
}

size_t ThumbnailView::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mItemList.size(); i < n; ++i ) {
        if ( mItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return THUMBNAILVIEW_ITEM_NOTFOUND;
}

sal_uInt16 ThumbnailView::GetItemId( size_t nPos ) const
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos]->mnId : 0 ;
}

sal_uInt16 ThumbnailView::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

void ThumbnailView::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;

        CalculateItemPositions();

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;

        CalculateItemPositions();

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;

        CalculateItemPositions();

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;

        CalculateItemPositions();

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

void ThumbnailView::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
            return;
    }

    if ( mnSelItemId != nItemId)
    {
        sal_uInt16 nOldItem = mnSelItemId ? mnSelItemId : 1;
        mnSelItemId = nItemId;

        bool bNewOut = IsReallyVisible() && IsUpdateMode();
        bool bNewLine = false;

        // if necessary scroll to the visible area
        if ( mbScroll && nItemId )
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
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }

        if( ImplHasAccessibleListeners() )
        {
            // focus event (deselect)
            if( nOldItem )
            {
                const size_t nPos = GetItemPos( nItemId );

                if( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
                {
                    ThumbnailViewAcc* pItemAcc = ThumbnailViewAcc::getImplementation(
                        mItemList[nPos]->GetAccessible( mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;
                        if( !mbIsTransientChildrenDisabled )
                        {
                            aOldAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                                static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                            ImplFireAccessibleEvent (::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                        }
                    }
                }
            }

            // focus event (select)
            const size_t nPos = GetItemPos( mnSelItemId );

            ThumbnailViewItem* pItem = NULL;
            if( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
                pItem = mItemList[nPos];

            ThumbnailViewAcc* pItemAcc = NULL;
            if (pItem != NULL)
                pItemAcc = ThumbnailViewAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                ::com::sun::star::uno::Any aOldAny, aNewAny;
                if( !mbIsTransientChildrenDisabled )
                {
                    aNewAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                        static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
                else
                {
                    aNewAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                }
            }

            // selection event
            ::com::sun::star::uno::Any aOldAny, aNewAny;
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
        }
    }
}

rtl::OUString ThumbnailView::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return rtl::OUString();
}

void ThumbnailView::SetColor( const Color& rColor )
{
    maColor = rColor;
    mpItemAttrs->aFillColor = rColor.getBColor();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

bool ThumbnailView::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return false;

    // if necessary abort an existing action

    // Check out if the the clicked on page is selected. If this is not the
    // case set it as the current item. We only check mouse actions since
    // drag-and-drop can also be triggered by the keyboard
    sal_uInt16 nSelId;
    if ( rCEvt.IsMouseEvent() )
        nSelId = GetItemId( rCEvt.GetMousePosPixel() );
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
    }

    Region aRegion;

    // assign region
    rRegion = aRegion;

    return true;
}

Size ThumbnailView::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines )
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

    Size        aSize( rItemSize.Width()*nCalcCols, rItemSize.Height()*nCalcLines );

    if ( mnSpacing )
    {
        aSize.Width()  += mnSpacing*(nCalcCols-1);
        aSize.Height() += mnSpacing*(nCalcLines-1);
    }

    // sum possible ScrollBar width
    aSize.Width() += GetScrollWidth();

    return aSize;
}

long ThumbnailView::GetScrollWidth() const
{
    if ( GetStyle() & WB_VSCROLL )
    {
        ((ThumbnailView*)this)->ImplInitScrollBar();
        return mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;
    }
    else
        return 0;
}

void ThumbnailView::setSelectionMode (bool mode)
{
    mbSelectionMode = mode;

    OnSelectionMode(mode);
}

void ThumbnailView::filterItems (const boost::function<bool (const ThumbnailViewItem*) > &func)
{
    maFilterFunc = func;

    CalculateItemPositions();

    Invalidate();
}

void ThumbnailView::sortItems (const boost::function<bool (const ThumbnailViewItem*, const ThumbnailViewItem*) > &func)
{
    std::sort(mItemList.begin(),mItemList.end(),func);

    CalculateItemPositions();

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


