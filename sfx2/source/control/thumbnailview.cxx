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
    SCROLL_OFFSET = 4
};

ThumbnailView::ThumbnailView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : Control( pParent, nWinStyle )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

ThumbnailView::ThumbnailView (Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : Control( pParent, rResId )
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
    delete mpProcessor;

    ImplDeleteItems();
}

void ThumbnailView::ImplInit()
{
    mpScrBar            = NULL;
    mnHeaderHeight      = 0;
    mnItemWidth         = 0;
    mnItemHeight        = 0;
    mnItemPadding = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnFirstLine         = 0;
    mnScrBarOffset = 1;
    mnHighItemId        = 0;
    mnCols              = 0;
    mnSpacing           = 0;
    mbScroll            = false;
    mbHasVisibleItems   = false;
    maFilterFunc = ViewFilterAll();
    maColor = GetSettings().GetStyleSettings().GetFieldColor();

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

        // deselect all current selected items and fire events
        if (pItem->isSelected())
        {
            pItem->setSelection(false);
            maItemStateHdl.Call(pItem);

            // fire accessible event???
        }

        if ( pItem->isVisible() && ImplHasAccessibleListeners() )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mItemList.clear();
    mFilteredItemList.clear();
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
        Color aColor = rStyleSettings.GetFieldColor();
        SetBackground( aColor );
    }


    mpItemAttrs = new ThumbnailItemAttributes;
    mpItemAttrs->aFillColor = maColor.getBColor();
    mpItemAttrs->aHighlightColor = rStyleSettings.GetHighlightColor().getBColor();
    mpItemAttrs->aFontAttr = getFontAttributeFromVclFont(mpItemAttrs->aFontSize,GetFont(),false,true);
    mpItemAttrs->nMaxTextLenght = -1;
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
            mpScrBar->setPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

void ThumbnailView::DrawItem (ThumbnailViewItem *pItem)
{
    if (pItem->isVisible())
    {
        Rectangle aRect = pItem->getDrawArea();

        if ( (aRect.GetHeight() > 0) && (aRect.GetWidth() > 0) )
            pItem->Paint(mpProcessor,mpItemAttrs);
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
    if (!mnItemHeight || !mnItemWidth)
        return;

    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mItemList.size();
    WinBits     nStyle = GetStyle();
    ScrollBar*  pDelScrBar = NULL;

    mFilteredItemList.clear();

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
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+mnScrBarOffset;

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
    else
    {
        if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
            mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);
    }

    mbHasVisibleItems = true;

    // calculate offsets
    long nStartX = nHItemSpace;
    long nStartY = nVItemSpace + mnHeaderHeight;

    // calculate and draw items
    long x = nStartX;
    long y = nStartY;

    // draw items
    size_t nFirstItem = mnFirstLine * mnCols;
    size_t nLastItem = nFirstItem + (mnVisLines * mnCols);

    maItemListRect.Left() = x;
    maItemListRect.Top() = y;
    maItemListRect.Right() = x + mnCols*(mnItemWidth+nHItemSpace) - nHItemSpace - 1;
    maItemListRect.Bottom() = y + mnVisLines*(mnItemHeight+nVItemSpace) - nVItemSpace - 1;

    // If want also draw parts of items in the last line,
    // then we add one more line if parts of these line are
    // visible

    size_t nCurCount = 0;
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        ThumbnailViewItem *const pItem = mItemList[i];

        if (maFilterFunc(pItem))
        {
            mFilteredItemList.push_back(pItem);
            if ((nCurCount >= nFirstItem) && (nCurCount < nLastItem))
            {
                if( !pItem->isVisible())
                {
                    if ( ImplHasAccessibleListeners() )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;

                        aNewAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                        ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                    }

                    pItem->show(true);

                    maItemStateHdl.Call(pItem);
                }

                pItem->setDrawArea(Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ));
                pItem->calculateItemsPosition(mnThumbnailHeight,mnDisplayHeight,mnItemPadding,mpItemAttrs->nMaxTextLenght,mpItemAttrs);

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
                        ::com::sun::star::uno::Any aOldAny, aNewAny;

                        aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                        ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                    }

                    pItem->show(false);

                    maItemStateHdl.Call(pItem);
                }

            }

            ++nCurCount;
        }
        else
        {
            if( pItem->isVisible())
            {
                if ( ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->show(false);

                maItemStateHdl.Call(pItem);
            }
        }
    }

    // arrange ScrollBar, set values and show it
    if ( mpScrBar )
    {
        mnLines = (nCurCount+mnCols-1)/mnCols;

        // check if scroll is needed
        mbScroll = mnLines > mnVisLines;


        Point aPos( aWinSize.Width() - nScrBarWidth - mnScrBarOffset, mnHeaderHeight );
        Size aSize( nScrBarWidth - mnScrBarOffset, aWinSize.Height() - mnHeaderHeight );

        mpScrBar->SetPosSizePixel( aPos, aSize );
        mpScrBar->SetRangeMax( (nCurCount+mnCols-1)/mnCols);
        mpScrBar->SetVisibleSize( mnVisLines );
        mpScrBar->SetThumbPos( (long)mnFirstLine );
        long nPageSize = mnVisLines;
        if ( nPageSize < 1 )
            nPageSize = 1;
        mpScrBar->SetPageSize( nPageSize );
        mpScrBar->Show( mbScroll );
    }

    // delete ScrollBar
    delete pDelScrBar;
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
    sal_uInt16 nNewFirstLine = mnFirstLine;

    if (pScrollBar->GetDelta() > 0)
        nNewFirstLine += 1;
    else
        nNewFirstLine -= 1;

    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;

        CalculateItemPositions();

        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    return 0;
}

IMPL_LINK (ThumbnailView, OnItemSelected, ThumbnailViewItem*, pItem)
{
    maItemStateHdl.Call(pItem);
    return 0;
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

    KeyCode aKeyCode = rKEvt.GetKeyCode();
    ThumbnailViewItem* pNext = NULL;
    switch ( aKeyCode.GetCode() )
    {
        case KEY_RIGHT:
            {
                size_t nNextPos = nLastPos;
                if ( bFoundLast && nLastPos < mFilteredItemList.size( ) - 1 )
                    nNextPos = nLastPos + 1;
                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_LEFT:
            {
                size_t nNextPos = nLastPos;
                if ( nLastPos > 0 )
                    nNextPos = nLastPos - 1;
                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_DOWN:
            {
                size_t nNextPos = nLastPos;
                if ( bFoundLast && nLastPos < mFilteredItemList.size( ) - mnCols )
                    nNextPos = nLastPos + mnCols;
                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_UP:
            {
                size_t nNextPos = nLastPos;
                if ( nLastPos >= mnCols )
                    nNextPos = nLastPos - mnCols;
                pNext = mFilteredItemList[nNextPos];
            }
            break;
        case KEY_RETURN:
            {
                if ( bFoundLast )
                    OnItemDblClicked( mFilteredItemList[nLastPos] );
            }
        default:
            Control::KeyInput( rKEvt );
    }

    if ( pNext )
    {
        deselectItems();
        SelectItem(pNext->mnId);
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
    sal_uInt16 nRow = nPos / mnCols;

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
    if ( rMEvt.IsLeft() )
    {
        ThumbnailViewItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );

        if (pItem && pItem->isVisible())
        {
            if ( rMEvt.GetClicks() == 1 )
            {
                if (rMEvt.IsMod1())
                {
                    //Keep selected item group state and just invert current desired one state
                    pItem->setSelection(!pItem->isSelected());
                }
                else
                {
                    //If we got a group of selected items deselect the rest and only keep the desired one
                    //mark items as not selected to not fire unnecessary change state events.
                    pItem->setSelection(false);
                    deselectItems();
                    pItem->setSelection(true);
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
            else if ( rMEvt.GetClicks() == 2 )
            {
                Rectangle aRect(pItem->getDrawArea());

                if (aRect.IsInside(rMEvt.GetPosPixel()))
                    OnItemDblClicked(pItem);
            }

            return;
        }
        else if (!pItem)
            deselectItems( );
    }

    Control::MouseButtonDown( rMEvt );
}

void ThumbnailView::MouseButtonUp( const MouseEvent& rMEvt )
{
    Control::MouseButtonUp( rMEvt );
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
        mpScrBar->Paint(aRect);
}

void ThumbnailView::GetFocus()
{
    if(GETFOCUS_TAB & GetGetFocusFlags())
    {
        // Select the first item if nothing selected
        int nSelected = -1;
        for (size_t i = 0, n = mItemList.size(); i < n && nSelected == -1; ++i)
        {
            if (mItemList[i]->isSelected())
                nSelected = i;
        }

        if ( nSelected == -1 && mItemList.size( ) > 0 )
        {
            SelectItem( 1 );
        }
    }

    // Tell the accessible object that we got the focus.
    ThumbnailViewAcc* pAcc = ThumbnailViewAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->GetFocus();

    Control::GetFocus();
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
    Control::Resize();
    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
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

void ThumbnailView::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );

        if ((*it)->isSelected())
        {
            (*it)->setSelection(false);
            maItemStateHdl.Call(*it);
        }

        delete *it;
        mItemList.erase( it );
    }

    // reset variables
    if ( mnHighItemId == nItemId )
    {
        mnHighItemId    = 0;
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
    mnHighItemId    = 0;

    CalculateItemPositions();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::updateItems (const std::vector<ThumbnailViewItem*> &items)
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;
    mnHighItemId    = 0;

    mItemList = items;

    CalculateItemPositions();

    Invalidate();
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

sal_uInt16 ThumbnailView::getNextItemId() const
{
    return mItemList.empty() ? 1 : mItemList.back()->mnId + 1;
}

void ThumbnailView::setItemMaxTextLength(sal_uInt32 nLength)
{
    mpItemAttrs->nMaxTextLenght = nLength;
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

    ThumbnailViewItem* pItem = mItemList[nItemPos];
    if (!pItem->isSelected())
    {
        mItemList[nItemPos]->setSelection(true);
        maItemStateHdl.Call(mItemList[nItemPos]);

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
            ThumbnailViewAcc* pItemAcc = ThumbnailViewAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

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

void ThumbnailView::DeselectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return;

    ThumbnailViewItem* pItem = mItemList[nItemPos];
    if (pItem->isSelected())
    {
        mItemList[nItemPos]->setSelection(false);
        maItemStateHdl.Call(mItemList[nItemPos]);

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        // TODO Trigger event in accessible object?
    }
}

bool ThumbnailView::IsItemSelected( sal_uInt16 nItemId ) const
{
    size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos == THUMBNAILVIEW_ITEM_NOTFOUND )
        return false;

    ThumbnailViewItem* pItem = mItemList[nItemPos];
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

OUString ThumbnailView::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != THUMBNAILVIEW_ITEM_NOTFOUND )
        return mItemList[nPos]->maTitle;

    return OUString();
}

void ThumbnailView::SetColor( const Color& rColor )
{
    maColor = rColor;
    mpItemAttrs->aFillColor = rColor.getBColor();

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

void ThumbnailView::filterItems (const boost::function<bool (const ThumbnailViewItem*) > &func)
{
    mnFirstLine = 0;        // start at the top of the list instead of the current position
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

bool ThumbnailView::renameItem(ThumbnailViewItem*, rtl::OUString)
{
    // Do nothing by default
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


