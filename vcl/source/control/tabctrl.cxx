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
#include <sal/log.hxx>

#include <vcl/notebookbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/builderfactory.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>

#include <controldata.hxx>
#include <svdata.hxx>
#include <window.h>

#include <unordered_map>
#include <vector>

struct ImplTabItem
{
    sal_uInt16          mnId;
    VclPtr<TabPage>     mpTabPage;
    OUString            maText;
    OUString            maFormatText;
    OUString            maHelpText;
    OString             maHelpId;
    OString             maTabName;
    tools::Rectangle    maRect;
    sal_uInt16          mnLine;
    bool                mbFullVisible;
    bool                mbEnabled;
    Image               maTabImage;

    ImplTabItem()
    : mnId( 0 ), mpTabPage( nullptr ),
      mnLine( 0 ), mbFullVisible( false ), mbEnabled( true )
    {}
};

struct ImplTabCtrlData
{
    std::unordered_map< int, int >        maLayoutPageIdToLine;
    std::unordered_map< int, int >        maLayoutLineToPageId;
    Point                           maItemsOffset;       // offset of the tabitems
    std::vector< ImplTabItem >      maItemList;
    VclPtr<ListBox>                 mpListBox;
};

// for the Tab positions
#define TAB_PAGERECT        0xFFFF
#define HAMBURGER_DIM       28

void TabControl::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mbLayoutDirty = true;

    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Control::ImplInit( pParent, nStyle, nullptr );

    mnLastWidth                 = 0;
    mnLastHeight                = 0;
    mnActPageId                 = 0;
    mnCurPageId                 = 0;
    mbFormat                    = true;
    mbRestoreHelpId             = false;
    mbSmallInvalidate           = false;
    mpTabCtrlData.reset(new ImplTabCtrlData);
    mpTabCtrlData->mpListBox    = nullptr;

    ImplInitSettings( true );

    if( nStyle & WB_DROPDOWN )
    {
        mpTabCtrlData->mpListBox = VclPtr<ListBox>::Create( this, WB_DROPDOWN );
        mpTabCtrlData->mpListBox->SetPosSizePixel( Point( 0, 0 ), Size( 200, 20 ) );
        mpTabCtrlData->mpListBox->SetSelectHdl( LINK( this, TabControl, ImplListBoxSelectHdl ) );
        mpTabCtrlData->mpListBox->Show();
    }

    // if the tabcontrol is drawn (ie filled) by a native widget, make sure all controls will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(ControlType::TabPane, ControlPart::Entire) )
        EnableChildTransparentMode();

    if (pParent && pParent->IsDialog())
        pParent->AddChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );
}

const vcl::Font& TabControl::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetTabFont();
}

const Color& TabControl::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetTabTextColor();
}

void TabControl::ImplInitSettings( bool bBackground )
{
    Control::ImplInitSettings();

    if ( bBackground )
    {
        vcl::Window* pParent = GetParent();
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled()
            || IsNativeControlSupported(ControlType::TabPane, ControlPart::Entire)
            || IsNativeControlSupported(ControlType::TabItem, ControlPart::Entire) ) )

        {
            // set transparent mode for NWF tabcontrols to have
            // the background always cleared properly
            EnableChildTransparentMode();
            SetParentClipMode( ParentClipMode::NoClip );
            SetPaintTransparent( true );
            SetBackground();
            ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode();
            SetPaintTransparent( false );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

void TabControl::ImplFreeLayoutData()
{
    if( HasLayoutData() )
    {
        ImplClearLayoutData();
        mpTabCtrlData->maLayoutPageIdToLine.clear();
        mpTabCtrlData->maLayoutLineToPageId.clear();
    }
}

TabControl::TabControl( vcl::Window* pParent, WinBits nStyle ) :
    Control( WindowType::TABCONTROL )
{
    ImplInit( pParent, nStyle );
    SAL_INFO( "vcl", "*** TABCONTROL no notabs? " << (( GetStyle() & WB_NOBORDER ) ? "true" : "false") );
}

TabControl::~TabControl()
{
    disposeOnce();
}

void TabControl::dispose()
{
    Window *pParent = GetParent();
    if (pParent && pParent->IsDialog())
        GetParent()->RemoveChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );

    ImplFreeLayoutData();

    // delete TabCtrl data
    if (mpTabCtrlData)
        mpTabCtrlData->mpListBox.disposeAndClear();
    mpTabCtrlData.reset();
    Control::dispose();
}

ImplTabItem* TabControl::ImplGetItem( sal_uInt16 nId ) const
{
    for (auto & item : mpTabCtrlData->maItemList)
    {
        if( item.mnId == nId )
            return &item;
    }

    return nullptr;
}

Size TabControl::ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth )
{
    pItem->maFormatText = pItem->maText;
    Size aSize( GetCtrlTextWidth( pItem->maFormatText ), GetTextHeight() );
    Size aImageSize( 0, 0 );
    if( !!pItem->maTabImage )
    {
        aImageSize = pItem->maTabImage.GetSizePixel();
        if( !pItem->maFormatText.isEmpty() )
            aImageSize.AdjustWidth(GetTextHeight()/4 );
    }
    aSize.AdjustWidth(aImageSize.Width() );
    if( aImageSize.Height() > aSize.Height() )
        aSize.setHeight( aImageSize.Height() );

    aSize.AdjustWidth(TAB_TABOFFSET_X*2 );
    aSize.AdjustHeight(TAB_TABOFFSET_Y*2 );

    tools::Rectangle aCtrlRegion( Point( 0, 0 ), aSize );
    tools::Rectangle aBoundingRgn, aContentRgn;
    const TabitemValue aControlValue(tools::Rectangle(TAB_TABOFFSET_X, TAB_TABOFFSET_Y,
                                               aSize.Width() - TAB_TABOFFSET_X * 2,
                                               aSize.Height() - TAB_TABOFFSET_Y * 2));
    if(GetNativeControlRegion( ControlType::TabItem, ControlPart::Entire, aCtrlRegion,
                                           ControlState::ENABLED, aControlValue,
                                           aBoundingRgn, aContentRgn ) )
    {
        return aContentRgn.GetSize();
    }

    // For languages with short names (e.g. Chinese), because the space is
    // normally only one pixel per char
    if ( pItem->maFormatText.getLength() < TAB_EXTRASPACE_X )
        aSize.AdjustWidth(TAB_EXTRASPACE_X-pItem->maFormatText.getLength() );

    // shorten Text if needed
    if ( aSize.Width()+4 >= nMaxWidth )
    {
        OUString aAppendStr("...");
        pItem->maFormatText += aAppendStr;
        do
        {
            if (pItem->maFormatText.getLength() > aAppendStr.getLength())
                pItem->maFormatText = pItem->maFormatText.replaceAt( pItem->maFormatText.getLength()-aAppendStr.getLength()-1, 1, "" );
            aSize.setWidth( GetCtrlTextWidth( pItem->maFormatText ) );
            aSize.AdjustWidth(aImageSize.Width() );
            aSize.AdjustWidth(TAB_TABOFFSET_X*2 );
        }
        while ( (aSize.Width()+4 >= nMaxWidth) && (pItem->maFormatText.getLength() > aAppendStr.getLength()) );
        if ( aSize.Width()+4 >= nMaxWidth )
        {
            pItem->maFormatText = ".";
            aSize.setWidth( 1 );
        }
    }

    if( pItem->maFormatText.isEmpty() )
    {
        if( aSize.Height() < aImageSize.Height()+4 ) //leave space for focus rect
            aSize.setHeight( aImageSize.Height()+4 );
    }

    return aSize;
}

// Feel free to move this to some more general place for reuse
// http://en.wikipedia.org/wiki/Word_wrap#Minimum_raggedness
// Mostly based on Alexey Frunze's nifty example at
// http://stackoverflow.com/questions/9071205/balanced-word-wrap-minimum-raggedness-in-php
namespace MinimumRaggednessWrap
{
    static std::deque<size_t> GetEndOfLineIndexes(const std::vector<sal_Int32>& rWidthsOf, sal_Int32 nLineWidth)
    {
        ++nLineWidth;

        size_t nWidthsCount = rWidthsOf.size();
        std::vector<sal_Int32> aCosts(nWidthsCount * nWidthsCount);

        // cost function c(i, j) that computes the cost of a line consisting of
        // the words Word[i] to Word[j]
        for (size_t i = 0; i < nWidthsCount; ++i)
        {
            for (size_t j = 0; j < nWidthsCount; ++j)
            {
                if (j >= i)
                {
                    sal_Int32 c = nLineWidth - (j - i);
                    for (size_t k = i; k <= j; ++k)
                        c -= rWidthsOf[k];
                    c = (c >= 0) ? c * c : SAL_MAX_INT32;
                    aCosts[j * nWidthsCount + i] = c;
                }
                else
                {
                    aCosts[j * nWidthsCount + i] = SAL_MAX_INT32;
                }
            }
        }

        std::vector<sal_Int32> aFunction(nWidthsCount);
        std::vector<sal_Int32> aWrapPoints(nWidthsCount);

        // f(j) in aFunction[], collect wrap points in aWrapPoints[]
        for (size_t j = 0; j < nWidthsCount; ++j)
        {
            aFunction[j] = aCosts[j * nWidthsCount];
            if (aFunction[j] == SAL_MAX_INT32)
            {
                for (size_t k = 0; k < j; ++k)
                {
                    sal_Int32 s;
                    if (aFunction[k] == SAL_MAX_INT32 || aCosts[j * nWidthsCount + k + 1] == SAL_MAX_INT32)
                        s = SAL_MAX_INT32;
                    else
                        s = aFunction[k] + aCosts[j * nWidthsCount + k + 1];
                    if (aFunction[j] > s)
                    {
                        aFunction[j] = s;
                        aWrapPoints[j] = k + 1;
                    }
                }
            }
        }

        std::deque<size_t> aSolution;

        // no solution
        if (aFunction[nWidthsCount - 1] == SAL_MAX_INT32)
            return aSolution;

        // optimal solution
        size_t j = nWidthsCount - 1;
        while (true)
        {
            aSolution.push_front(j);
            if (!aWrapPoints[j])
                break;
            j = aWrapPoints[j] - 1;
        }

        return aSolution;
    }
};

bool TabControl::ImplPlaceTabs( long nWidth )
{
    if ( nWidth <= 0 )
        return false;
    if ( mpTabCtrlData->maItemList.empty() )
        return false;

    long nMaxWidth = nWidth;

    const long nOffsetX = 2 + GetItemsOffset().X();
    const long nOffsetY = 2 + GetItemsOffset().Y();

    //fdo#66435 throw Knuth/Tex minimum raggedness algorithm at the problem
    //of ugly bare tabs on lines of their own

    //collect widths
    std::vector<sal_Int32> aWidths;
    for (auto & item : mpTabCtrlData->maItemList)
    {
        aWidths.push_back(ImplGetItemSize( &item, nMaxWidth ).Width());
    }

    //aBreakIndexes will contain the indexes of the last tab on each row
    std::deque<size_t> aBreakIndexes(MinimumRaggednessWrap::GetEndOfLineIndexes(aWidths, nMaxWidth - nOffsetX - 2));

    nMaxWidth -= GetItemsOffset().X();

    long nX = nOffsetX;
    long nY = nOffsetY;

    sal_uInt16 nLines = 0;
    sal_uInt16 nCurLine = 0;

    long nLineWidthAry[100];
    sal_uInt16 nLinePosAry[101];
    nLineWidthAry[0] = 0;
    nLinePosAry[0] = 0;

    size_t nIndex = 0;
    sal_uInt16 nPos = 0;

    for (auto & item : mpTabCtrlData->maItemList)
    {
        Size aSize = ImplGetItemSize( &item, nMaxWidth );

        bool bNewLine = false;
        if (!aBreakIndexes.empty() && nIndex > aBreakIndexes.front())
        {
            aBreakIndexes.pop_front();
            bNewLine = true;
        }

        if ( bNewLine && (nWidth > 2+nOffsetX) )
        {
            if ( nLines == 99 )
                break;

            nX = nOffsetX;
            nY += aSize.Height();
            nLines++;
            nLineWidthAry[nLines] = 0;
            nLinePosAry[nLines] = nPos;
        }

        tools::Rectangle aNewRect( Point( nX, nY ), aSize );
        if ( mbSmallInvalidate && (item.maRect != aNewRect) )
            mbSmallInvalidate = false;
        item.maRect = aNewRect;
        item.mnLine = nLines;
        item.mbFullVisible = true;

        nLineWidthAry[nLines] += aSize.Width();
        nX += aSize.Width();

        if ( item.mnId == mnCurPageId )
            nCurLine = nLines;

        ++nPos;
        ++nIndex;
    }

    if ( nLines )
    { // two or more lines
        long nLineHeightAry[100];
        long nIH = mpTabCtrlData->maItemList[0].maRect.Bottom()-2;

        for ( sal_uInt16 i = 0; i < nLines+1; i++ )
        {
            if ( i <= nCurLine )
                nLineHeightAry[i] = nIH*(nLines-(nCurLine-i)) + GetItemsOffset().Y();
            else
                nLineHeightAry[i] = nIH*(i-nCurLine-1) + GetItemsOffset().Y();
        }

        nLinePosAry[nLines+1] = static_cast<sal_uInt16>(mpTabCtrlData->maItemList.size());

        long nDX = 0;
        long nModDX = 0;
        long nIDX = 0;

        sal_uInt16 i = 0;
        sal_uInt16 n = 0;

        for (auto & item : mpTabCtrlData->maItemList)
        {
            if ( i == nLinePosAry[n] )
            {
                if ( n == nLines+1 )
                    break;

                nIDX = 0;
                if( nLinePosAry[n+1]-i > 0 )
                {
                    nDX = ( nWidth - nOffsetX - nLineWidthAry[n] ) / ( nLinePosAry[n+1] - i );
                    nModDX = ( nWidth - nOffsetX - nLineWidthAry[n] ) % ( nLinePosAry[n+1] - i );
                }
                else
                {
                    // FIXME: this is a case of tabctrl way too small
                    nDX = 0;
                    nModDX = 0;
                }
                n++;
            }

            item.maRect.AdjustLeft(nIDX );
            item.maRect.AdjustRight(nIDX + nDX );
            item.maRect.SetTop( nLineHeightAry[n-1] );
            item.maRect.SetBottom( nLineHeightAry[n-1] + nIH );
            nIDX += nDX;

            if ( nModDX )
            {
                nIDX++;
                item.maRect.AdjustRight( 1 );
                nModDX--;
            }

            i++;
        }
    }
    else
    { // only one line
        if(ImplGetSVData()->maNWFData.mbCenteredTabs)
        {
            int nRightSpace = nMaxWidth;//space left on the right by the tabs
            for (auto const& item : mpTabCtrlData->maItemList)
            {
                nRightSpace -= item.maRect.Right()-item.maRect.Left();
            }
            for (auto & item : mpTabCtrlData->maItemList)
            {
                item.maRect.AdjustLeft(nRightSpace / 2 );
                item.maRect.AdjustRight(nRightSpace / 2 );
            }
        }
    }

    return true;
}

tools::Rectangle TabControl::ImplGetTabRect( sal_uInt16 nItemPos, long nWidth, long nHeight )
{
    Size aWinSize = Control::GetOutputSizePixel();
    if ( nWidth < 0 )
        nWidth = aWinSize.Width();
    if ( nHeight < 0 )
        nHeight = aWinSize.Height();

    if ( mpTabCtrlData->maItemList.empty() )
    {
        long nW = nWidth-TAB_OFFSET*2;
        long nH = nHeight-TAB_OFFSET*2;
        return (nW > 0 && nH > 0)
        ? tools::Rectangle( Point( TAB_OFFSET, TAB_OFFSET ), Size( nW, nH ) )
        : tools::Rectangle();
    }

    if ( nItemPos == TAB_PAGERECT )
    {
        sal_uInt16 nLastPos;
        if ( mnCurPageId )
            nLastPos = GetPagePos( mnCurPageId );
        else
            nLastPos = 0;

        tools::Rectangle aRect = ImplGetTabRect( nLastPos, nWidth, nHeight );
        long nW = nWidth-TAB_OFFSET*2;
        long nH = nHeight-aRect.Bottom()-TAB_OFFSET*2;
        aRect = (nW > 0 && nH > 0)
        ? tools::Rectangle( Point( TAB_OFFSET, aRect.Bottom()+TAB_OFFSET ), Size( nW, nH ) )
        : tools::Rectangle();
        return aRect;
    }

    nWidth -= 1;

    if ( (nWidth <= 0) || (nHeight <= 0) )
        return tools::Rectangle();

    if ( mbFormat || (mnLastWidth != nWidth) || (mnLastHeight != nHeight) )
    {
        vcl::Font aFont( GetFont() );
        aFont.SetTransparent( true );
        SetFont( aFont );

        bool bRet = ImplPlaceTabs( nWidth );
        if ( !bRet )
            return tools::Rectangle();

        mnLastWidth     = nWidth;
        mnLastHeight    = nHeight;
        mbFormat        = false;
    }

    return size_t(nItemPos) < mpTabCtrlData->maItemList.size() ? mpTabCtrlData->maItemList[nItemPos].maRect : tools::Rectangle();
}

void TabControl::ImplChangeTabPage( sal_uInt16 nId, sal_uInt16 nOldId )
{
    ImplFreeLayoutData();

    ImplTabItem*    pOldItem = ImplGetItem( nOldId );
    ImplTabItem*    pItem = ImplGetItem( nId );
    TabPage*        pOldPage = pOldItem ? pOldItem->mpTabPage.get() : nullptr;
    TabPage*        pPage = pItem ? pItem->mpTabPage.get() : nullptr;
    vcl::Window*    pCtrlParent = GetParent();

    if ( IsReallyVisible() && IsUpdateMode() )
    {
        sal_uInt16 nPos = GetPagePos( nId );
        tools::Rectangle aRect = ImplGetTabRect( nPos );

        if ( !pOldItem || !pItem || (pOldItem->mnLine != pItem->mnLine) )
        {
            aRect.SetLeft( 0 );
            aRect.SetTop( 0 );
            aRect.SetRight( Control::GetOutputSizePixel().Width() );
        }
        else
        {
            aRect.AdjustLeft( -3 );
            aRect.AdjustTop( -2 );
            aRect.AdjustRight(3 );
            Invalidate( aRect );
            nPos = GetPagePos( nOldId );
            aRect = ImplGetTabRect( nPos );
            aRect.AdjustLeft( -3 );
            aRect.AdjustTop( -2 );
            aRect.AdjustRight(3 );
        }
        Invalidate( aRect );
    }

    if ( pOldPage == pPage )
        return;

    tools::Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );

    if ( pOldPage )
    {
        if ( mbRestoreHelpId )
            pCtrlParent->SetHelpId( OString() );
        pOldPage->DeactivatePage();
    }

    if ( pPage )
    {
        if ( GetStyle() & WB_NOBORDER )
        {
            tools::Rectangle aRectNoTab(Point(0, 0), GetSizePixel());
            pPage->SetPosSizePixel( aRectNoTab.TopLeft(), aRectNoTab.GetSize() );
        }
        else
            pPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );

        // activate page here so the controls can be switched
        // also set the help id of the parent window to that of the tab page
        if ( GetHelpId().isEmpty() )
        {
            mbRestoreHelpId = true;
            pCtrlParent->SetHelpId( pPage->GetHelpId() );
        }

        pPage->ActivatePage();
        pPage->Show();

        if ( pOldPage && pOldPage->HasChildPathFocus() )
        {
            vcl::Window* pFirstChild = pPage->ImplGetDlgWindow( 0, GetDlgWindowType::First );
            if ( pFirstChild )
                pFirstChild->ImplControlFocus( GetFocusFlags::Init );
            else
                GrabFocus();
        }
    }

    if ( pOldPage )
        pOldPage->Hide();

    // Invalidate the same region that will be send to NWF
    // to always allow for bitmap caching
    // see Window::DrawNativeControl()
    if( IsNativeControlSupported( ControlType::TabPane, ControlPart::Entire ) )
    {
        aRect.AdjustLeft( -(TAB_OFFSET) );
        aRect.AdjustTop( -(TAB_OFFSET) );
        aRect.AdjustRight(TAB_OFFSET );
        aRect.AdjustBottom(TAB_OFFSET );
    }

    Invalidate( aRect );
}

bool TabControl::ImplPosCurTabPage()
{
    // resize/position current TabPage
    ImplTabItem* pItem = ImplGetItem( GetCurPageId() );
    if ( pItem && pItem->mpTabPage )
    {
        if (  GetStyle() & WB_NOBORDER )
        {
            tools::Rectangle aRectNoTab(Point(0, 0), GetSizePixel());
            pItem->mpTabPage->SetPosSizePixel( aRectNoTab.TopLeft(), aRectNoTab.GetSize() );
            return true;
        }
        tools::Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        pItem->mpTabPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        return true;
    }

    return false;
}

void TabControl::ImplActivateTabPage( bool bNext )
{
    sal_uInt16 nCurPos = GetPagePos( GetCurPageId() );

    if ( bNext )
        nCurPos = (nCurPos + 1) % GetPageCount();
    else
    {
        if ( !nCurPos )
            nCurPos = GetPageCount()-1;
        else
            nCurPos--;
    }

    SelectTabPage( GetPageId( nCurPos ) );
}

void TabControl::ImplShowFocus()
{
    if ( !GetPageCount() || mpTabCtrlData->mpListBox )
        return;

    sal_uInt16                   nCurPos     = GetPagePos( mnCurPageId );
    tools::Rectangle                aRect       = ImplGetTabRect( nCurPos );
    const ImplTabItem&       rItem       = mpTabCtrlData->maItemList[ nCurPos ];
    Size                     aTabSize    = aRect.GetSize();
    Size aImageSize( 0, 0 );
    long                     nTextHeight = GetTextHeight();
    long                     nTextWidth  = GetCtrlTextWidth( rItem.maFormatText );
    sal_uInt16                   nOff;

    if ( !(GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::Mono) )
        nOff = 1;
    else
        nOff = 0;

    if( !! rItem.maTabImage )
    {
        aImageSize = rItem.maTabImage.GetSizePixel();
        if( !rItem.maFormatText.isEmpty() )
            aImageSize.AdjustWidth(GetTextHeight()/4 );
    }

    if( !rItem.maFormatText.isEmpty() )
    {
        // show focus around text
        aRect.SetLeft( aRect.Left()+aImageSize.Width()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-1-1 );
        aRect.SetTop( aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-1-1 );
        aRect.SetRight( aRect.Left()+nTextWidth+2 );
        aRect.SetBottom( aRect.Top()+nTextHeight+2 );
    }
    else
    {
        // show focus around image
        long nXPos = aRect.Left()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-1;
        long nYPos = aRect.Top();
        if( aImageSize.Height() < aRect.GetHeight() )
            nYPos += (aRect.GetHeight() - aImageSize.Height())/2;

        aRect.SetLeft( nXPos - 2 );
        aRect.SetTop( nYPos - 2 );
        aRect.SetRight( aRect.Left() + aImageSize.Width() + 4 );
        aRect.SetBottom( aRect.Top() + aImageSize.Height() + 4 );
    }
    ShowFocus( aRect );
}

void TabControl::ImplDrawItem(vcl::RenderContext& rRenderContext, ImplTabItem const * pItem, const tools::Rectangle& rCurRect,
                              bool bFirstInGroup, bool bLastInGroup )
{
    if (pItem->maRect.IsEmpty())
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    tools::Rectangle aRect = pItem->maRect;
    long nLeftBottom = aRect.Bottom();
    long nRightBottom = aRect.Bottom();
    bool bLeftBorder = true;
    bool bRightBorder = true;
    sal_uInt16 nOff;
    bool bNativeOK = false;

    sal_uInt16 nOff2 = 0;
    sal_uInt16 nOff3 = 0;

    if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
        nOff = 1;
    else
        nOff = 0;

    // if this is the active Page, we have to draw a little more
    if (pItem->mnId == mnCurPageId)
    {
        nOff2 = 2;
        if (!ImplGetSVData()->maNWFData.mbNoActiveTabTextRaise)
            nOff3 = 1;
    }
    else
    {
        Point aLeftTestPos = aRect.BottomLeft();
        Point aRightTestPos = aRect.BottomRight();
        if (aLeftTestPos.Y() == rCurRect.Bottom())
        {
            aLeftTestPos.AdjustX( -2 );
            if (rCurRect.IsInside(aLeftTestPos))
                bLeftBorder = false;
            aRightTestPos.AdjustX(2 );
            if (rCurRect.IsInside(aRightTestPos))
                bRightBorder = false;
        }
        else
        {
            if (rCurRect.IsInside(aLeftTestPos))
                nLeftBottom -= 2;
            if (rCurRect.IsInside(aRightTestPos))
                nRightBottom -= 2;
        }
    }

    ControlState nState = ControlState::NONE;

    if (pItem->mnId == mnCurPageId)
    {
        nState |= ControlState::SELECTED;
        // only the selected item can be focused
        if (HasFocus())
            nState |= ControlState::FOCUSED;
    }
    if (IsEnabled())
        nState |= ControlState::ENABLED;
    if (IsMouseOver() && pItem->maRect.IsInside(GetPointerPosPixel()))
    {
        nState |= ControlState::ROLLOVER;
        for (auto const& item : mpTabCtrlData->maItemList)
        {
            if( (&item != pItem) && (item.maRect.IsInside(GetPointerPosPixel())))
            {
                nState &= ~ControlState::ROLLOVER; // avoid multiple highlighted tabs
                break;
            }
        }
    }

    bNativeOK = rRenderContext.IsNativeControlSupported(ControlType::TabItem, ControlPart::Entire);
    if ( bNativeOK )
    {
        TabitemValue tiValue(tools::Rectangle(pItem->maRect.Left() + TAB_TABOFFSET_X,
                                       pItem->maRect.Top() + TAB_TABOFFSET_Y,
                                       pItem->maRect.Right() - TAB_TABOFFSET_X,
                                       pItem->maRect.Bottom() - TAB_TABOFFSET_Y));
        if (pItem->maRect.Left() < 5)
            tiValue.mnAlignment |= TabitemFlags::LeftAligned;
        if (pItem->maRect.Right() > mnLastWidth - 5)
            tiValue.mnAlignment |= TabitemFlags::RightAligned;
        if (bFirstInGroup)
            tiValue.mnAlignment |= TabitemFlags::FirstInGroup;
        if (bLastInGroup)
            tiValue.mnAlignment |= TabitemFlags::LastInGroup;

        tools::Rectangle aCtrlRegion( pItem->maRect );
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::TabItem, ControlPart::Entire,
                                                     aCtrlRegion, nState, tiValue, OUString() );
    }

    if (!bNativeOK)
    {
        if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
        {
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            rRenderContext.DrawPixel(Point(aRect.Left() + 1 - nOff2, aRect.Top() + 1 - nOff2)); // diagonally indented top-left pixel
            if (bLeftBorder)
            {
                rRenderContext.DrawLine(Point(aRect.Left() - nOff2, aRect.Top() + 2 - nOff2),
                                        Point(aRect.Left() - nOff2, nLeftBottom - 1));
            }
            rRenderContext.DrawLine(Point(aRect.Left() + 2 - nOff2, aRect.Top() - nOff2),   // top line starting 2px from left border
                                    Point(aRect.Right() + nOff2 - 3, aRect.Top() - nOff2)); // ending 3px from right border

            if (bRightBorder)
            {
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawLine(Point(aRect.Right() + nOff2 - 2, aRect.Top() + 1 - nOff2),
                                        Point(aRect.Right() + nOff2 - 2, nRightBottom - 1));

                rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
                rRenderContext.DrawLine(Point(aRect.Right() + nOff2 - 1, aRect.Top() + 3 - nOff2),
                                        Point(aRect.Right() + nOff2 - 1, nRightBottom - 1));
            }
        }
        else
        {
            rRenderContext.SetLineColor(COL_BLACK);
            rRenderContext.DrawPixel(Point(aRect.Left() + 1 - nOff2, aRect.Top() + 1 - nOff2));
            rRenderContext.DrawPixel(Point(aRect.Right() + nOff2 - 2, aRect.Top() + 1 - nOff2));
            if (bLeftBorder)
            {
                rRenderContext.DrawLine(Point(aRect.Left() - nOff2, aRect.Top() + 2 - nOff2),
                                        Point(aRect.Left() - nOff2, nLeftBottom - 1));
            }
            rRenderContext.DrawLine(Point(aRect.Left() + 2 - nOff2, aRect.Top() - nOff2),
                                    Point(aRect.Right() - 3, aRect.Top() - nOff2));
            if (bRightBorder)
            {
                rRenderContext.DrawLine(Point(aRect.Right() + nOff2 - 1, aRect.Top() + 2 - nOff2),
                                        Point(aRect.Right() + nOff2 - 1, nRightBottom - 1));
            }
        }
    }

    // set font accordingly, current item is painted bold
    // we set the font attributes always before drawing to be re-entrant (DrawNativeControl may trigger additional paints)
    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);

    Size aTabSize = aRect.GetSize();
    Size aImageSize(0, 0);
    long nTextHeight = rRenderContext.GetTextHeight();
    long nTextWidth = rRenderContext.GetCtrlTextWidth(pItem->maFormatText);
    if (!!pItem->maTabImage)
    {
        aImageSize = pItem->maTabImage.GetSizePixel();
        if (!pItem->maFormatText.isEmpty())
            aImageSize.AdjustWidth(GetTextHeight() / 4 );
    }
    long nXPos = aRect.Left() + ((aTabSize.Width() - nTextWidth - aImageSize.Width()) / 2) - nOff - nOff3;
    long nYPos = aRect.Top() + ((aTabSize.Height() - nTextHeight) / 2) - nOff3;
    if (!pItem->maFormatText.isEmpty())
    {
        DrawTextFlags nStyle = DrawTextFlags::Mnemonic;
        if (!pItem->mbEnabled)
            nStyle |= DrawTextFlags::Disable;

        Color aColor(rStyleSettings.GetTabTextColor());
        if (nState & ControlState::SELECTED)
            aColor = rStyleSettings.GetTabHighlightTextColor();
        else if (nState & ControlState::ROLLOVER)
            aColor = rStyleSettings.GetTabRolloverTextColor();

        Color aOldColor(rRenderContext.GetTextColor());
        rRenderContext.SetTextColor(aColor);

        const tools::Rectangle aOutRect(nXPos + aImageSize.Width(), nYPos,
                                 nXPos + aImageSize.Width() + nTextWidth, nYPos + nTextHeight);
        DrawControlText(rRenderContext, aOutRect, pItem->maFormatText, nStyle,
                        nullptr, nullptr);

        rRenderContext.SetTextColor(aOldColor);
    }

    if (!!pItem->maTabImage)
    {
        Point aImgTL( nXPos, aRect.Top() );
        if (aImageSize.Height() < aRect.GetHeight())
            aImgTL.AdjustY((aRect.GetHeight() - aImageSize.Height()) / 2 );
        rRenderContext.DrawImage(aImgTL, pItem->maTabImage, pItem->mbEnabled ? DrawImageFlags::NONE : DrawImageFlags::Disable );
    }
}

bool TabControl::ImplHandleKeyEvent( const KeyEvent& rKeyEvent )
{
    bool bRet = false;

    if ( GetPageCount() > 1 )
    {
        vcl::KeyCode aKeyCode = rKeyEvent.GetKeyCode();
        sal_uInt16 nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.IsMod1() )
        {
            if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                {
                    ImplActivateTabPage( false );
                    bRet = true;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    ImplActivateTabPage( true );
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

IMPL_LINK_NOARG(TabControl, ImplListBoxSelectHdl, ListBox&, void)
{
    SelectTabPage( GetPageId( mpTabCtrlData->mpListBox->GetSelectedEntryPos() ) );
}

IMPL_LINK( TabControl, ImplWindowEventListener, VclWindowEvent&, rEvent, void )
{
    if ( rEvent.GetId() == VclEventId::WindowKeyInput )
    {
        // Do not handle events from TabControl or its children, which is done in Notify(), where the events can be consumed.
        if ( !IsWindowOrChild( rEvent.GetWindow() ) )
        {
            KeyEvent* pKeyEvent = static_cast< KeyEvent* >(rEvent.GetData());
            ImplHandleKeyEvent( *pKeyEvent );
        }
    }
}

void TabControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( mpTabCtrlData->mpListBox.get() == nullptr )
    {
        if( rMEvt.IsLeft() )
        {
            sal_uInt16 nPageId = GetPageId( rMEvt.GetPosPixel() );
            ImplTabItem* pItem = ImplGetItem( nPageId );
            if( pItem && pItem->mbEnabled )
                SelectTabPage( nPageId );
        }
    }
}

void TabControl::KeyInput( const KeyEvent& rKEvt )
{
    if( mpTabCtrlData->mpListBox )
        mpTabCtrlData->mpListBox->KeyInput( rKEvt );
    else if ( GetPageCount() > 1 )
    {
        vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
        sal_uInt16  nKeyCode = aKeyCode.GetCode();

        if ( (nKeyCode == KEY_LEFT) || (nKeyCode == KEY_RIGHT) )
        {
            bool bNext = (nKeyCode == KEY_RIGHT);
            ImplActivateTabPage( bNext );
        }
    }

    Control::KeyInput( rKEvt );
}

void TabControl::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!(GetStyle() & WB_NOBORDER))
        ImplPaint(rRenderContext, rRect);
}

void TabControl::ImplPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    HideFocus();

    // reformat if needed
    tools::Rectangle aRect = ImplGetTabRect(TAB_PAGERECT);

    // find current item
    ImplTabItem* pCurItem = nullptr;
    for (auto & item : mpTabCtrlData->maItemList)
    {
        if (item.mnId == mnCurPageId)
        {
            pCurItem = &item;
            break;
        }
    }

    // Draw the TabPage border
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    tools::Rectangle aCurRect;
    aRect.AdjustLeft( -(TAB_OFFSET) );
    aRect.AdjustTop( -(TAB_OFFSET) );
    aRect.AdjustRight(TAB_OFFSET );
    aRect.AdjustBottom(TAB_OFFSET );

    // if we have an invisible tabpage or no tabpage at all the tabpage rect should be
    // increased to avoid round corners that might be drawn by a theme
    // in this case we're only interested in the top border of the tabpage because the tabitems are used
    // standalone (eg impress)
    bool bNoTabPage = false;
    TabPage* pCurPage = pCurItem ? pCurItem->mpTabPage.get() : nullptr;
    if (!pCurPage || !pCurPage->IsVisible())
    {
        bNoTabPage = true;
        aRect.AdjustLeft( -10 );
        aRect.AdjustRight(10 );
    }

    if (rRenderContext.IsNativeControlSupported(ControlType::TabPane, ControlPart::Entire))
    {
        const ImplControlValue aControlValue;

        ControlState nState = ControlState::ENABLED;
        if (!IsEnabled())
            nState &= ~ControlState::ENABLED;
        if (HasFocus())
            nState |= ControlState::FOCUSED;

        vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
        aClipRgn.Intersect(aRect);
        if (!rRect.IsEmpty())
            aClipRgn.Intersect(rRect);

        if (!aClipRgn.IsEmpty())
        {
            rRenderContext.DrawNativeControl(ControlType::TabPane, ControlPart::Entire,
                                             aRect, nState, aControlValue, OUString());
        }

        if (rRenderContext.IsNativeControlSupported(ControlType::TabHeader, ControlPart::Entire))
        {
            tools::Rectangle aHeaderRect(aRect.Left(), 0, aRect.Right(), aRect.Top());

            aClipRgn = rRenderContext.GetActiveClipRegion();
            aClipRgn.Intersect(aHeaderRect);
            if (!rRect.IsEmpty())
                aClipRgn.Intersect(rRect);

            if (!aClipRgn.IsEmpty())
            {
                rRenderContext.DrawNativeControl(ControlType::TabHeader, ControlPart::Entire,
                                                 aHeaderRect, nState, aControlValue, OUString());
            }
        }
    }
    else
    {
        long nTopOff = 1;
        if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        else
            rRenderContext.SetLineColor(COL_BLACK);
        if (pCurItem && !pCurItem->maRect.IsEmpty())
        {
            aCurRect = pCurItem->maRect;
            rRenderContext.DrawLine(aRect.TopLeft(), Point(aCurRect.Left() - 2, aRect.Top()));
            if (aCurRect.Right() + 1 < aRect.Right())
            {
                rRenderContext.DrawLine(Point(aCurRect.Right(), aRect.Top()), aRect.TopRight());
            }
            else
            {
                nTopOff = 0;
            }
        }
        else
            rRenderContext.DrawLine(aRect.TopLeft(), aRect.TopRight());

        rRenderContext.DrawLine(aRect.TopLeft(), aRect.BottomLeft());

        if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
        {
            // if we have not tab page the bottom line of the tab page
            // directly touches the tab items, so choose a color that fits seamlessly
            if (bNoTabPage)
                rRenderContext.SetLineColor(rStyleSettings.GetDialogColor());
            else
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(Point(1, aRect.Bottom() - 1), Point(aRect.Right() - 1, aRect.Bottom() - 1));
            rRenderContext.DrawLine(Point(aRect.Right() - 1, aRect.Top() + nTopOff), Point(aRect.Right() - 1, aRect.Bottom() - 1));
            if (bNoTabPage)
                rRenderContext.SetLineColor(rStyleSettings.GetDialogColor());
            else
                rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
            rRenderContext.DrawLine(Point(0, aRect.Bottom()), Point(aRect.Right(), aRect.Bottom()));
            rRenderContext.DrawLine(Point(aRect.Right(), aRect.Top() + nTopOff), Point(aRect.Right(), aRect.Bottom()));
        }
        else
        {
            rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
            rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
        }
    }

    if (!mpTabCtrlData->maItemList.empty() && mpTabCtrlData->mpListBox == nullptr)
    {
        // Some native toolkits (GTK+) draw tabs right-to-left, with an
        // overlap between adjacent tabs
        bool bDrawTabsRTL = rRenderContext.IsNativeControlSupported(ControlType::TabItem, ControlPart::TabsDrawRtl);
        ImplTabItem* pFirstTab = nullptr;
        ImplTabItem* pLastTab = nullptr;
        size_t idx;

        // Even though there is a tab overlap with GTK+, the first tab is not
        // overlapped on the left side. Other toolkits ignore this option.
        if (bDrawTabsRTL)
        {
            pFirstTab = mpTabCtrlData->maItemList.data();
            pLastTab = pFirstTab + mpTabCtrlData->maItemList.size() - 1;
            idx = mpTabCtrlData->maItemList.size() - 1;
        }
        else
        {
            pLastTab = mpTabCtrlData->maItemList.data();
            pFirstTab = pLastTab + mpTabCtrlData->maItemList.size() - 1;
            idx = 0;
        }

        while (idx < mpTabCtrlData->maItemList.size())
        {
            ImplTabItem* pItem = &mpTabCtrlData->maItemList[idx];

            if (pItem != pCurItem)
            {
                vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
                aClipRgn.Intersect(pItem->maRect);
                if (!rRect.IsEmpty())
                    aClipRgn.Intersect(rRect);
                if (!aClipRgn.IsEmpty())
                {
                    ImplDrawItem(rRenderContext, pItem, aCurRect,
                                 pItem == pFirstTab, pItem == pLastTab);
                }
            }

            if (bDrawTabsRTL)
                idx--;
            else
                idx++;
        }

        if (pCurItem)
        {
            vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
            aClipRgn.Intersect(pCurItem->maRect);
            if (!rRect.IsEmpty())
                aClipRgn.Intersect(rRect);
            if (!aClipRgn.IsEmpty())
            {
                ImplDrawItem(rRenderContext, pCurItem, aCurRect,
                             pCurItem == pFirstTab, pCurItem == pLastTab);
            }
        }
    }

    if (HasFocus())
        ImplShowFocus();

    mbSmallInvalidate = true;
}

void TabControl::setAllocation(const Size &rAllocation)
{
    ImplFreeLayoutData();

    if ( !IsReallyShown() )
        return;

    if( mpTabCtrlData->mpListBox )
    {
        // get the listbox' preferred size
        Size aTabCtrlSize( GetSizePixel() );
        long nPrefWidth = mpTabCtrlData->mpListBox->get_preferred_size().Width();
        if( nPrefWidth > aTabCtrlSize.Width() )
            nPrefWidth = aTabCtrlSize.Width();
        Size aNewSize( nPrefWidth, LogicToPixel( Size( 12, 12 ), MapMode( MapUnit::MapAppFont ) ).Height() );
        Point aNewPos( (aTabCtrlSize.Width() - nPrefWidth) / 2, 0 );
        mpTabCtrlData->mpListBox->SetPosSizePixel( aNewPos, aNewSize );
    }

    mbFormat = true;

    // resize/position active TabPage
    bool bTabPage = ImplPosCurTabPage();

    // check what needs to be invalidated
    Size aNewSize = rAllocation;
    long nNewWidth = aNewSize.Width();
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        if ( !item.mbFullVisible ||
             (item.maRect.Right()-2 >= nNewWidth) )
        {
            mbSmallInvalidate = false;
            break;
        }
    }

    if ( mbSmallInvalidate )
    {
        tools::Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        aRect.AdjustLeft( -(TAB_OFFSET+TAB_BORDER_LEFT) );
        aRect.AdjustTop( -(TAB_OFFSET+TAB_BORDER_TOP) );
        aRect.AdjustRight(TAB_OFFSET+TAB_BORDER_RIGHT );
        aRect.AdjustBottom(TAB_OFFSET+TAB_BORDER_BOTTOM );
        if ( bTabPage )
            Invalidate( aRect, InvalidateFlags::NoChildren );
        else
            Invalidate( aRect );

    }
    else
    {
        if ( bTabPage )
            Invalidate( InvalidateFlags::NoChildren );
        else
            Invalidate();
    }

    mbLayoutDirty = false;
}

void TabControl::SetPosSizePixel(const Point& rNewPos, const Size& rNewSize)
{
    Window::SetPosSizePixel(rNewPos, rNewSize);
    //if size changed, TabControl::Resize got called already
    if (mbLayoutDirty)
        setAllocation(rNewSize);
}

void TabControl::SetSizePixel(const Size& rNewSize)
{
    Window::SetSizePixel(rNewSize);
    //if size changed, TabControl::Resize got called already
    if (mbLayoutDirty)
        setAllocation(rNewSize);
}

void TabControl::SetPosPixel(const Point& rPos)
{
    Window::SetPosPixel(rPos);
    if (mbLayoutDirty)
        setAllocation(GetOutputSizePixel());
}

void TabControl::Resize()
{
    setAllocation(Control::GetOutputSizePixel());
}

void TabControl::GetFocus()
{
    if( ! mpTabCtrlData->mpListBox )
    {
        ImplShowFocus();
        SetInputContext( InputContext( GetFont() ) );
    }
    else
    {
        if( mpTabCtrlData->mpListBox->IsReallyVisible() )
            mpTabCtrlData->mpListBox->GrabFocus();
    }
    Control::GetFocus();
}

void TabControl::LoseFocus()
{
    if( mpTabCtrlData && ! mpTabCtrlData->mpListBox )
        HideFocus();
    Control::LoseFocus();
}

void TabControl::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = rHEvt.KeyboardActivated() ? mnCurPageId : GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
        {
            OUString aStr = GetHelpText( nItemId );
            if ( !aStr.isEmpty() )
            {
                tools::Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );
                Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HelpEventMode::EXTENDED )
        {
            OUString aHelpId( OStringToOUString( GetHelpId( nItemId ), RTL_TEXTENCODING_UTF8 ) );
            if ( !aHelpId.isEmpty() )
            {
                // call Help if existing
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( aHelpId, this );
                return;
            }
        }

        // for Quick or Ballon Help, we show the text, if it is cut
        if ( rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON) )
        {
            ImplTabItem* pItem = ImplGetItem( nItemId );
            const OUString& rStr = pItem->maText;
            if ( rStr != pItem->maFormatText )
            {
                tools::Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );
                if ( !rStr.isEmpty() )
                {
                    if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, rStr );
                    else
                        Help::ShowQuickHelp( this, aItemRect, rStr );
                    return;
                }
            }
        }

        if ( rHEvt.GetMode() & HelpEventMode::QUICK )
        {
            ImplTabItem* pItem = ImplGetItem( nItemId );
            const OUString& rHelpText = pItem->maHelpText;
            // show tooltip if not text but image is set and helptext is available
            if ( !rHelpText.isEmpty() && pItem->maText.isEmpty() && !!pItem->maTabImage )
            {
                tools::Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );
                Help::ShowQuickHelp( this, aItemRect, rHelpText );
                return;
            }
        }
    }

    Control::RequestHelp( rHEvt );
}

void TabControl::Command( const CommandEvent& rCEvt )
{
    if( (mpTabCtrlData->mpListBox == nullptr) && (rCEvt.GetCommand() == CommandEventId::ContextMenu) && (GetPageCount() > 1) )
    {
        Point   aMenuPos;
        bool    bMenu;
        if ( rCEvt.IsMouseEvent() )
        {
            aMenuPos = rCEvt.GetMousePosPixel();
            bMenu = GetPageId( aMenuPos ) != 0;
        }
        else
        {
            aMenuPos = ImplGetTabRect( GetPagePos( mnCurPageId ) ).Center();
            bMenu = true;
        }

        if ( bMenu )
        {
            ScopedVclPtrInstance<PopupMenu> aMenu;
            for (auto const& item : mpTabCtrlData->maItemList)
            {
                aMenu->InsertItem( item.mnId, item.maText, MenuItemBits::CHECKABLE | MenuItemBits::RADIOCHECK );
                if ( item.mnId == mnCurPageId )
                    aMenu->CheckItem( item.mnId );
                aMenu->SetHelpId( item.mnId, item.maHelpId );
            }

            sal_uInt16 nId = aMenu->Execute( this, aMenuPos );
            if ( nId && (nId != mnCurPageId) )
                SelectTabPage( nId );
            return;
        }
    }

    Control::Command( rCEvt );
}

void TabControl::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
    {
        ImplPosCurTabPage();
        if( mpTabCtrlData->mpListBox )
            Resize();
    }
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == StateChangedType::Zoom)  ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

void TabControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

tools::Rectangle* TabControl::ImplFindPartRect( const Point& rPt )
{
    ImplTabItem* pFoundItem = nullptr;
    int nFound = 0;
    for (auto & item : mpTabCtrlData->maItemList)
    {
        if ( item.maRect.IsInside( rPt ) )
        {
            // assure that only one tab is highlighted at a time
            nFound++;
            pFoundItem = &item;
        }
    }
    // assure that only one tab is highlighted at a time
    return nFound == 1 ? &pFoundItem->maRect : nullptr;
}

bool TabControl::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::TabItem, ControlPart::Entire) )
            {
                tools::Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                tools::Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
                    vcl::Region aClipRgn;
                    if( pLastRect )
                    {
                        // allow for slightly bigger tabitems
                        // as used by gtk
                        // TODO: query for the correct sizes
                        tools::Rectangle aRect(*pLastRect);
                        aRect.AdjustLeft( -2 );
                        aRect.AdjustRight(2 );
                        aRect.AdjustTop( -3 );
                        aClipRgn.Union( aRect );
                    }
                    if( pRect )
                    {
                        // allow for slightly bigger tabitems
                        // as used by gtk
                        // TODO: query for the correct sizes
                        tools::Rectangle aRect(*pRect);
                        aRect.AdjustLeft( -2 );
                        aRect.AdjustRight(2 );
                        aRect.AdjustTop( -3 );
                        aClipRgn.Union( aRect );
                    }
                    if( !aClipRgn.IsEmpty() )
                        Invalidate( aClipRgn );
                }
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

bool TabControl::EventNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        bRet = ImplHandleKeyEvent( *rNEvt.GetKeyEvent() );

    return bRet || Control::EventNotify( rNEvt );
}

void TabControl::ActivatePage()
{
    maActivateHdl.Call( this );
}

bool TabControl::DeactivatePage()
{
    return !maDeactivateHdl.IsSet() || maDeactivateHdl.Call( this );
}

void TabControl::SetTabPageSizePixel( const Size& rSize )
{
    ImplFreeLayoutData();

    Size aNewSize( rSize );
    aNewSize.AdjustWidth(TAB_OFFSET*2 );
    tools::Rectangle aRect = ImplGetTabRect( TAB_PAGERECT,
                                      aNewSize.Width(), aNewSize.Height() );
    aNewSize.AdjustHeight(aRect.Top()+TAB_OFFSET );
    Window::SetOutputSizePixel( aNewSize );
}

Size TabControl::GetTabPageSizePixel() const
{
    tools::Rectangle aRect = const_cast<TabControl*>(this)->ImplGetTabRect( TAB_PAGERECT );
    return aRect.GetSize();
}

void TabControl::InsertPage( sal_uInt16 nPageId, const OUString& rText,
                             sal_uInt16 nPos )
{
    SAL_WARN_IF( !nPageId, "vcl", "TabControl::InsertPage(): PageId == 0" );
    SAL_WARN_IF( GetPagePos( nPageId ) != TAB_PAGE_NOTFOUND, "vcl",
                "TabControl::InsertPage(): PageId already exists" );

    // insert new page item
    ImplTabItem* pItem = nullptr;
    if( nPos == TAB_APPEND || size_t(nPos) >= mpTabCtrlData->maItemList.size() )
    {
        mpTabCtrlData->maItemList.emplace_back( );
        pItem = &mpTabCtrlData->maItemList.back();
        if( mpTabCtrlData->mpListBox )
            mpTabCtrlData->mpListBox->InsertEntry( rText );
    }
    else
    {
        std::vector< ImplTabItem >::iterator new_it =
            mpTabCtrlData->maItemList.insert( mpTabCtrlData->maItemList.begin() + nPos, ImplTabItem() );
        pItem = &(*new_it);
        if( mpTabCtrlData->mpListBox )
            mpTabCtrlData->mpListBox->InsertEntry( rText, nPos);
    }
    if( mpTabCtrlData->mpListBox )
    {
        if( ! mnCurPageId )
            mpTabCtrlData->mpListBox->SelectEntryPos( 0 );
        mpTabCtrlData->mpListBox->SetDropDownLineCount( mpTabCtrlData->mpListBox->GetEntryCount() );
    }

    // set current page id
    if ( !mnCurPageId )
        mnCurPageId = nPageId;

    // init new page item
    pItem->mnId             = nPageId;
    pItem->mpTabPage        = nullptr;
    pItem->maText           = rText;
    pItem->mbFullVisible    = false;

    mbFormat = true;
    if ( IsUpdateMode() )
        Invalidate();

    ImplFreeLayoutData();
    if( mpTabCtrlData->mpListBox ) // reposition/resize listbox
        Resize();

    CallEventListeners( VclEventId::TabpageInserted, reinterpret_cast<void*>(nPageId) );
}

void TabControl::RemovePage( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    // does the item exist ?
    if ( nPos != TAB_PAGE_NOTFOUND )
    {
        //remove page item
        std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin() + nPos;
        bool bIsCurrentPage = (it->mnId == mnCurPageId);
        mpTabCtrlData->maItemList.erase( it );
        if( mpTabCtrlData->mpListBox )
        {
            mpTabCtrlData->mpListBox->RemoveEntry( nPos );
            mpTabCtrlData->mpListBox->SetDropDownLineCount( mpTabCtrlData->mpListBox->GetEntryCount() );
        }

        // If current page is removed, then first page gets the current page
        if ( bIsCurrentPage  )
        {
            mnCurPageId = 0;

            if( ! mpTabCtrlData->maItemList.empty() )
            {
                // don't do this by simply setting mnCurPageId to pFirstItem->mnId
                // this leaves a lot of stuff (such trivia as _showing_ the new current page) undone
                // instead, call SetCurPageId
                // without this, the next (outside) call to SetCurPageId with the id of the first page
                // will result in doing nothing (as we assume that nothing changed, then), and the page
                // will never be shown.
                // 86875 - 05/11/2001 - frank.schoenheit@germany.sun.com

                SetCurPageId( mpTabCtrlData->maItemList[0].mnId );
            }
        }

        mbFormat = true;
        if ( IsUpdateMode() )
            Invalidate();

        ImplFreeLayoutData();

        CallEventListeners( VclEventId::TabpageRemoved, reinterpret_cast<void*>(nPageId) );
    }
}

void TabControl::Clear()
{
    // clear item list
    mpTabCtrlData->maItemList.clear();
    mnCurPageId = 0;
    if( mpTabCtrlData->mpListBox )
        mpTabCtrlData->mpListBox->Clear();

    ImplFreeLayoutData();

    mbFormat = true;
    if ( IsUpdateMode() )
        Invalidate();

    CallEventListeners( VclEventId::TabpageRemovedAll );
}

void TabControl::EnablePage( sal_uInt16 i_nPageId, bool i_bEnable )
{
    ImplTabItem* pItem = ImplGetItem( i_nPageId );

    if ( pItem && pItem->mbEnabled != i_bEnable )
    {
        pItem->mbEnabled = i_bEnable;
        mbFormat = true;
        if( mpTabCtrlData->mpListBox )
            mpTabCtrlData->mpListBox->SetEntryFlags( GetPagePos( i_nPageId ),
                                                     i_bEnable ? ListBoxEntryFlags::NONE : (ListBoxEntryFlags::DisableSelection | ListBoxEntryFlags::DrawDisabled) );
        if( pItem->mnId == mnCurPageId )
        {
             // SetCurPageId will change to an enabled page
            SetCurPageId( mnCurPageId );
        }
        else if ( IsUpdateMode() )
            Invalidate();
    }
}

sal_uInt16 TabControl::GetPageCount() const
{
    return static_cast<sal_uInt16>(mpTabCtrlData->maItemList.size());
}

sal_uInt16 TabControl::GetPageId( sal_uInt16 nPos ) const
{
    if( size_t(nPos) < mpTabCtrlData->maItemList.size() )
        return mpTabCtrlData->maItemList[ nPos ].mnId;
    return 0;
}

sal_uInt16 TabControl::GetPagePos( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = 0;
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        if ( item.mnId == nPageId )
            return nPos;
        ++nPos;
    }

    return TAB_PAGE_NOTFOUND;
}

sal_uInt16 TabControl::GetPageId( const Point& rPos ) const
{
    for( size_t i = 0; i < mpTabCtrlData->maItemList.size(); ++i )
    {
        if ( const_cast<TabControl*>(this)->ImplGetTabRect( static_cast<sal_uInt16>(i) ).IsInside( rPos ) )
            return mpTabCtrlData->maItemList[ i ].mnId;
    }

    return 0;
}

sal_uInt16 TabControl::GetPageId( const TabPage& rPage ) const
{
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        if ( item.mpTabPage == &rPage )
            return item.mnId;
    }

    return 0;
}

sal_uInt16 TabControl::GetPageId( const OString& rName ) const
{
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        if ( item.maTabName == rName )
            return item.mnId;
    }

    return 0;
}

void TabControl::SetCurPageId( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    while( nPos != TAB_PAGE_NOTFOUND &&
           ! mpTabCtrlData->maItemList[nPos].mbEnabled )
    {
        nPos++;
        if( size_t(nPos) >= mpTabCtrlData->maItemList.size() )
            nPos = 0;
        if( mpTabCtrlData->maItemList[nPos].mnId == nPageId )
            break;
    }

    if( nPos != TAB_PAGE_NOTFOUND )
    {
        nPageId = mpTabCtrlData->maItemList[nPos].mnId;
        if ( nPageId == mnCurPageId )
        {
            if ( mnActPageId )
                mnActPageId = nPageId;
            return;
        }

        if ( mnActPageId )
            mnActPageId = nPageId;
        else
        {
            mbFormat = true;
            sal_uInt16 nOldId = mnCurPageId;
            mnCurPageId = nPageId;
            ImplChangeTabPage( nPageId, nOldId );
        }
    }
}

sal_uInt16 TabControl::GetCurPageId() const
{
    if ( mnActPageId )
        return mnActPageId;
    else
        return mnCurPageId;
}

void TabControl::SelectTabPage( sal_uInt16 nPageId )
{
    if ( nPageId && (nPageId != mnCurPageId) )
    {
        ImplFreeLayoutData();

        CallEventListeners( VclEventId::TabpageDeactivate, reinterpret_cast<void*>(mnCurPageId) );
        if ( DeactivatePage() )
        {
            mnActPageId = nPageId;
            ActivatePage();
            // Page could have been switched by the Activate handler
            nPageId = mnActPageId;
            mnActPageId = 0;
            SetCurPageId( nPageId );
            if( mpTabCtrlData->mpListBox )
                mpTabCtrlData->mpListBox->SelectEntryPos( GetPagePos( nPageId ) );
            CallEventListeners( VclEventId::TabpageActivate, reinterpret_cast<void*>(nPageId) );
        }
    }
}

void TabControl::SetTabPage( sal_uInt16 nPageId, TabPage* pTabPage )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && (pItem->mpTabPage.get() != pTabPage) )
    {
        if ( pTabPage )
        {
            if ( IsDefaultSize() )
                SetTabPageSizePixel( pTabPage->GetSizePixel() );

            // only set here, so that Resize does not reposition TabPage
            pItem->mpTabPage = pTabPage;
            queue_resize();
            if ( pItem->mnId == mnCurPageId )
                ImplChangeTabPage( pItem->mnId, 0 );
        }
        else
        {
            pItem->mpTabPage = nullptr;
            queue_resize();
        }
    }
}

TabPage* TabControl::GetTabPage( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->mpTabPage;
    else
        return nullptr;
}

void TabControl::SetPageText( sal_uInt16 nPageId, const OUString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && pItem->maText != rText )
    {
        pItem->maText = rText;
        mbFormat = true;
        if( mpTabCtrlData->mpListBox )
        {
            sal_uInt16 nPos = GetPagePos( nPageId );
            mpTabCtrlData->mpListBox->RemoveEntry( nPos );
            mpTabCtrlData->mpListBox->InsertEntry( rText, nPos );
        }
        if ( IsUpdateMode() )
            Invalidate();
        ImplFreeLayoutData();
        CallEventListeners( VclEventId::TabpagePageTextChanged, reinterpret_cast<void*>(nPageId) );
    }
}

OUString const & TabControl::GetPageText( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    assert( pItem );

    return pItem->maText;
}

void TabControl::SetHelpText( sal_uInt16 nPageId, const OUString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    assert( pItem );

    pItem->maHelpText = rText;
}

const OUString& TabControl::GetHelpText( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    assert( pItem );

    if ( pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty() )
    {
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
            pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
    }
    return pItem->maHelpText;
}

void TabControl::SetHelpId( sal_uInt16 nPageId, const OString& rId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        pItem->maHelpId = rId;
}

OString TabControl::GetHelpId( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if (pItem)
        return pItem->maHelpId;

    return OString();
}

void TabControl::SetPageName( sal_uInt16 nPageId, const OString& rName ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        pItem->maTabName = rName;
}

OString TabControl::GetPageName( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if (pItem)
        return pItem->maTabName;

    return OString();
}

void TabControl::SetPageImage( sal_uInt16 i_nPageId, const Image& i_rImage )
{
    ImplTabItem* pItem = ImplGetItem( i_nPageId );

    if ( pItem )
    {
        pItem->maTabImage = i_rImage;
        mbFormat = true;
        if ( IsUpdateMode() )
            Invalidate();
    }
}

tools::Rectangle TabControl::GetCharacterBounds( sal_uInt16 nPageId, long nIndex ) const
{
    tools::Rectangle aRet;

    if( !HasLayoutData() || mpTabCtrlData->maLayoutPageIdToLine.empty() )
        FillLayoutData();

    if( HasLayoutData() )
    {
        std::unordered_map< int, int >::const_iterator it = mpTabCtrlData->maLayoutPageIdToLine.find( static_cast<int>(nPageId) );
        if( it != mpTabCtrlData->maLayoutPageIdToLine.end() )
        {
            Pair aPair = mpControlData->mpLayoutData->GetLineStartEnd( it->second );
            if( (aPair.B() - aPair.A()) >= nIndex )
                aRet = mpControlData->mpLayoutData->GetCharacterBounds( aPair.A() + nIndex );
        }
    }

    return aRet;
}

long TabControl::GetIndexForPoint( const Point& rPoint, sal_uInt16& rPageId ) const
{
    long nRet = -1;

    if( !HasLayoutData() || mpTabCtrlData->maLayoutPageIdToLine.empty() )
        FillLayoutData();

    if( HasLayoutData() )
    {
        int nIndex = mpControlData->mpLayoutData->GetIndexForPoint( rPoint );
        if( nIndex != -1 )
        {
            // what line (->pageid) is this index in ?
            int nLines = mpControlData->mpLayoutData->GetLineCount();
            int nLine = -1;
            while( ++nLine < nLines )
            {
                Pair aPair = mpControlData->mpLayoutData->GetLineStartEnd( nLine );
                if( aPair.A() <= nIndex && aPair.B() >= nIndex )
                {
                    nRet = nIndex - aPair.A();
                    rPageId = static_cast<sal_uInt16>(mpTabCtrlData->maLayoutLineToPageId[ nLine ]);
                    break;
                }
            }
        }
    }

    return nRet;
}

void TabControl::FillLayoutData() const
{
    mpTabCtrlData->maLayoutLineToPageId.clear();
    mpTabCtrlData->maLayoutPageIdToLine.clear();
    const_cast<TabControl*>(this)->Invalidate();
}

tools::Rectangle TabControl::GetTabBounds( sal_uInt16 nPageId ) const
{
    tools::Rectangle aRet;

    ImplTabItem* pItem = ImplGetItem( nPageId );
    if(pItem)
        aRet = pItem->maRect;

    return aRet;
}

void TabControl::SetItemsOffset( const Point& rOffs )
{
    if( mpTabCtrlData )
        mpTabCtrlData->maItemsOffset = rOffs;
}

Point TabControl::GetItemsOffset() const
{
    if( mpTabCtrlData )
        return mpTabCtrlData->maItemsOffset;
    else
        return Point();
}

Size TabControl::calculateRequisition() const
{
    Size aOptimalPageSize(0, 0);

    sal_uInt16 nOrigPageId = GetCurPageId();
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        const TabPage *pPage = item.mpTabPage;
        //it's a real nuisance if the page is not inserted yet :-(
        //We need to force all tabs to exist to get overall optimal size for dialog
        if (!pPage)
        {
            TabControl *pThis = const_cast<TabControl*>(this);
            pThis->SetCurPageId(item.mnId);
            pThis->ActivatePage();
            pPage = item.mpTabPage;
        }

        if (!pPage)
            continue;

        Size aPageSize(VclContainer::getLayoutRequisition(*pPage));

        if (aPageSize.Width() > aOptimalPageSize.Width())
            aOptimalPageSize.setWidth( aPageSize.Width() );
        if (aPageSize.Height() > aOptimalPageSize.Height())
            aOptimalPageSize.setHeight( aPageSize.Height() );
    }

    //fdo#61940 If we were forced to activate pages in order to on-demand
    //create them to get their optimal size, then switch back to the original
    //page and re-activate it
    if (nOrigPageId != GetCurPageId())
    {
        TabControl *pThis = const_cast<TabControl*>(this);
        pThis->SetCurPageId(nOrigPageId);
        pThis->ActivatePage();
    }

    long nTabLabelsBottom = 0, nTabLabelsRight = 0;
    for (sal_uInt16 nPos(0), sizeList(static_cast <sal_uInt16> (mpTabCtrlData->maItemList.size()));
            nPos < sizeList; ++nPos)
    {
        TabControl* pThis = const_cast<TabControl*>(this);

        tools::Rectangle aTabRect = pThis->ImplGetTabRect(nPos, aOptimalPageSize.Width(), LONG_MAX);
        if (aTabRect.Bottom() > nTabLabelsBottom)
            nTabLabelsBottom = aTabRect.Bottom();
        if (aTabRect.Right() > nTabLabelsRight)
            nTabLabelsRight = aTabRect.Right();
    }

    Size aOptimalSize(aOptimalPageSize);
    aOptimalSize.AdjustHeight(nTabLabelsBottom );
    aOptimalSize.setWidth( std::max(nTabLabelsRight, aOptimalSize.Width()) );

    aOptimalSize.AdjustWidth(TAB_OFFSET * 2 );
    aOptimalSize.AdjustHeight(TAB_OFFSET * 2 );

    return aOptimalSize;
}

Size TabControl::GetOptimalSize() const
{
    return calculateRequisition();
}

void TabControl::queue_resize(StateChangedType eReason)
{
    mbLayoutDirty = true;
    Window::queue_resize(eReason);
}

std::vector<sal_uInt16> TabControl::GetPageIDs() const
{
    std::vector<sal_uInt16> aIDs;
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        aIDs.push_back(item.mnId);
    }

    return aIDs;
}

FactoryFunction TabControl::GetUITestFactory() const
{
    return TabControlUIObject::create;
}

boost::property_tree::ptree TabControl::DumpAsPropertyTree()
{
    boost::property_tree::ptree aTree = Control::DumpAsPropertyTree();

    boost::property_tree::ptree aTabs;
    for(auto id : GetPageIDs())
    {
        boost::property_tree::ptree aTab;
        aTab.put("text", GetPageText(id));
        aTab.put("id", id);
        aTab.put("name", GetPageName(id));
        aTabs.push_back(std::make_pair("", aTab));
    }

    aTree.add_child("tabs", aTabs);
    aTree.put("selected", GetCurPageId());

    return aTree;
}

sal_uInt16 NotebookbarTabControlBase::m_nHeaderHeight = 0;

IMPL_LINK_NOARG(NotebookbarTabControlBase, OpenMenu, Button*, void)
{
    m_aIconClickHdl.Call(static_cast<NotebookBar*>(GetParent()->GetParent()));
}

NotebookbarTabControlBase::NotebookbarTabControlBase(vcl::Window* pParent)
    : TabControl(pParent, WB_STDTABCONTROL)
    , bLastContextWasSupported(true)
    , eLastContext(vcl::EnumContext::Context::Any)
{
    m_pOpenMenu = VclPtr<PushButton>::Create( this , WB_CENTER | WB_VCENTER );
    m_pOpenMenu->SetSizePixel(Size(HAMBURGER_DIM, HAMBURGER_DIM));
    m_pOpenMenu->SetClickHdl(LINK(this, NotebookbarTabControlBase, OpenMenu));
    m_pOpenMenu->SetModeImage(Image(StockImage::Yes, SV_RESID_BITMAP_NOTEBOOKBAR));
    m_pOpenMenu->Show();
}

NotebookbarTabControlBase::~NotebookbarTabControlBase()
{
    disposeOnce();
}

void NotebookbarTabControlBase::SetContext( vcl::EnumContext::Context eContext )
{
    if (eLastContext != eContext)
    {
        bool bHandled = false;

        for (int nChild = 0; nChild < GetPageCount(); ++nChild)
        {
            sal_uInt16 nPageId = TabControl::GetPageId(nChild);
            TabPage* pPage = GetTabPage(nPageId);

            if (pPage)
            {
                if (pPage->HasContext(eContext) || pPage->HasContext(vcl::EnumContext::Context::Any))
                    EnablePage(nPageId);
                else
                    EnablePage(nPageId, false);

                if (!bHandled && bLastContextWasSupported
                    && pPage->HasContext(vcl::EnumContext::Context::Default))
                {
                    SetCurPageId(nPageId);
                }

                if (pPage->HasContext(eContext) && eContext != vcl::EnumContext::Context::Any)
                {
                    SetCurPageId(nPageId);
                    bHandled = true;
                    bLastContextWasSupported = true;
                }
            }
        }

        if (!bHandled)
            bLastContextWasSupported = false;
        eLastContext = eContext;
    }
}

void NotebookbarTabControlBase::dispose()
{
    m_pShortcuts.disposeAndClear();
    m_pOpenMenu.disposeAndClear();
    TabControl::dispose();
}

void NotebookbarTabControlBase::SetToolBox( ToolBox* pToolBox )
{
    m_pShortcuts.set( pToolBox );
}

void NotebookbarTabControlBase::SetIconClickHdl( Link<NotebookBar*, void> aHdl )
{
    m_aIconClickHdl = aHdl;
}

sal_uInt16 NotebookbarTabControlBase::GetPageId( const Point& rPos ) const
{
    for( size_t i = 0; i < mpTabCtrlData->maItemList.size(); ++i )
    {
        if ( const_cast<NotebookbarTabControlBase*>(this)->ImplGetTabRect( static_cast<sal_uInt16>(i) ).IsInside( rPos ) )
            if ( mpTabCtrlData->maItemList[ i ].mbEnabled )
                return mpTabCtrlData->maItemList[ i ].mnId;
    }

    return 0;
}

void NotebookbarTabControlBase::SelectTabPage( sal_uInt16 nPageId )
{
    TabControl::SelectTabPage( nPageId );
    Resize();
}

void NotebookbarTabControlBase::SetCurPageId( sal_uInt16 nPageId )
{
    TabControl::SetCurPageId( nPageId );
    Resize();
    if ( nPageId == GetPageCount() )
        ImplActivateTabPage( true );
}

void NotebookbarTabControlBase::ImplActivateTabPage( bool bNext )
{
    sal_uInt16 nCurPos = GetPagePos( GetCurPageId() );

    if ( bNext && nCurPos + 1 < GetPageCount() )
    {
        sal_uInt16 nOldPos = nCurPos;
        nCurPos++;

        ImplTabItem* pItem = &mpTabCtrlData->maItemList[nCurPos];
        while ( !pItem->mbEnabled && nCurPos + 1 < GetPageCount())
        {
            nCurPos++;
            pItem = &mpTabCtrlData->maItemList[nCurPos];
        }

        if ( !pItem->mbEnabled )
            nCurPos = nOldPos;
    }
    else if ( !bNext && nCurPos )
    {
        nCurPos--;
        ImplTabItem* pItem = &mpTabCtrlData->maItemList[nCurPos];
        while ( nCurPos && !pItem->mbEnabled )
        {
            nCurPos--;
            pItem = &mpTabCtrlData->maItemList[nCurPos];
        }
    }

    SelectTabPage( TabControl::GetPageId( nCurPos ) );
}

sal_uInt16 NotebookbarTabControlBase::GetHeaderHeight()
{
    return m_nHeaderHeight;
}

bool NotebookbarTabControlBase::ImplPlaceTabs( long nWidth )
{
    if ( nWidth <= 0 )
        return false;
    if ( mpTabCtrlData->maItemList.empty() )
        return false;
    if (!m_pOpenMenu || m_pOpenMenu->isDisposed())
        return false;

    long nMaxWidth = nWidth - HAMBURGER_DIM;
    long nShortcutsWidth = m_pShortcuts != nullptr ? m_pShortcuts->GetSizePixel().getWidth() + 1 : 0;
    long nFullWidth = nShortcutsWidth;

    const long nOffsetX = 2 + GetItemsOffset().X() + nShortcutsWidth;
    const long nOffsetY = 2 + GetItemsOffset().Y();

    //fdo#66435 throw Knuth/Tex minimum raggedness algorithm at the problem
    //of ugly bare tabs on lines of their own

    //collect widths
    std::vector<sal_Int32> aWidths;
    for (auto & item : mpTabCtrlData->maItemList)
    {
        if( item.mbEnabled )
        {
            long aSize = ImplGetItemSize( &item, nMaxWidth ).getWidth();
            if( !item.maText.isEmpty() && aSize < 100)
            {
                nFullWidth += 100;
                aSize = 100;
            }
            else
                nFullWidth += aSize;

            aWidths.push_back(aSize);
        }
        else
            aWidths.push_back(0);
    }

    nMaxWidth -= GetItemsOffset().X();

    long nX = nOffsetX;
    long nY = nOffsetY;

    long nLineWidthAry[100];
    nLineWidthAry[0] = 0;

    for (auto & item : mpTabCtrlData->maItemList)
    {
        Size aSize = ImplGetItemSize( &item, nMaxWidth );

        if ( !item.mbEnabled )
        {
            continue;
        }

        // set minimum tab size
        if( nFullWidth < nMaxWidth && !item.maText.isEmpty() && aSize.getWidth() < 100)
            aSize.setWidth( 100 );

        if( !item.maText.isEmpty() && aSize.getHeight() < 28 )
            aSize.setHeight( 28 );

        tools::Rectangle aNewRect( Point( nX, nY ), aSize );
        if ( mbSmallInvalidate && (item.maRect != aNewRect) )
            mbSmallInvalidate = false;

        item.maRect = aNewRect;
        item.mnLine = 0;
        item.mbFullVisible = true;

        nLineWidthAry[0] += aSize.Width();
        nX += aSize.Width();
    }

    // only one line
    if(ImplGetSVData()->maNWFData.mbCenteredTabs)
    {
        int nRightSpace = nMaxWidth;//space left on the right by the tabs
        for (auto const& item : mpTabCtrlData->maItemList)
        {
            nRightSpace -= item.maRect.Right()-item.maRect.Left();
        }
        for (auto & item : mpTabCtrlData->maItemList)
        {
            item.maRect.AdjustLeft(nRightSpace / 2 );
            item.maRect.AdjustRight(nRightSpace / 2 );
        }
    }

    // position the shortcutbox
    if (m_pShortcuts)
        m_pShortcuts->SetPosPixel(Point(0, 0));

    // position the menu
    m_pOpenMenu->SetPosPixel(Point(nWidth - HAMBURGER_DIM, 0));

    return true;
}

void NotebookbarTabControlBase::ImplPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    HideFocus();

    // reformat if needed
    tools::Rectangle aRect = ImplGetTabRect(TAB_PAGERECT);

    // find current item
    ImplTabItem* pCurItem = nullptr;
    for (auto & item : mpTabCtrlData->maItemList)
    {
        if (item.mnId == mnCurPageId)
        {
            pCurItem = &item;
            break;
        }
    }

    // Draw the TabPage border
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    tools::Rectangle aCurRect;
    aRect.AdjustLeft( -(TAB_OFFSET) );
    aRect.AdjustTop( -(TAB_OFFSET) );
    aRect.AdjustRight(TAB_OFFSET );
    aRect.AdjustBottom(TAB_OFFSET );

    // if we have an invisible tabpage or no tabpage at all the tabpage rect should be
    // increased to avoid round corners that might be drawn by a theme
    // in this case we're only interested in the top border of the tabpage because the tabitems are used
    // standalone (eg impress)
    bool bNoTabPage = false;
    TabPage* pCurPage = pCurItem ? pCurItem->mpTabPage.get() : nullptr;
    if (!pCurPage || !pCurPage->IsVisible())
    {
        bNoTabPage = true;
        aRect.AdjustLeft( -10 );
        aRect.AdjustRight(10 );
    }

    if (rRenderContext.IsNativeControlSupported(ControlType::TabPane, ControlPart::Entire))
    {
        const ImplControlValue aControlValue;

        ControlState nState = ControlState::ENABLED;
        if (!IsEnabled())
            nState &= ~ControlState::ENABLED;
        if (HasFocus())
            nState |= ControlState::FOCUSED;

        vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
        aClipRgn.Intersect(aRect);
        if (!rRect.IsEmpty())
            aClipRgn.Intersect(rRect);

        if (!aClipRgn.IsEmpty())
        {
            rRenderContext.DrawNativeControl(ControlType::TabPane, ControlPart::Entire,
                                             aRect, nState, aControlValue, OUString());
        }

        if (rRenderContext.IsNativeControlSupported(ControlType::TabHeader, ControlPart::Entire))
        {
            tools::Rectangle aHeaderRect(aRect.Left(), 0, aRect.Right(), aRect.Top());

            aClipRgn = rRenderContext.GetActiveClipRegion();
            aClipRgn.Intersect(aHeaderRect);
            if (!rRect.IsEmpty())
                aClipRgn.Intersect(rRect);

            if (!aClipRgn.IsEmpty())
            {
                rRenderContext.DrawNativeControl(ControlType::TabHeader, ControlPart::Entire,
                                                 aHeaderRect, nState, aControlValue, OUString());
            }
        }
    }
    else
    {
        long nTopOff = 1;
        if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        else
            rRenderContext.SetLineColor(COL_BLACK);
        if (pCurItem && !pCurItem->maRect.IsEmpty())
        {
            aCurRect = pCurItem->maRect;
            rRenderContext.DrawLine(aRect.TopLeft(), Point(aCurRect.Left() - 2, aRect.Top()));
            if (aCurRect.Right() + 1 < aRect.Right())
            {
                rRenderContext.DrawLine(Point(aCurRect.Right(), aRect.Top()), aRect.TopRight());
            }
            else
            {
                nTopOff = 0;
            }
        }
        else
            rRenderContext.DrawLine(aRect.TopLeft(), aRect.TopRight());

        rRenderContext.DrawLine(aRect.TopLeft(), aRect.BottomLeft());

        if (!(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
        {
            // if we have not tab page the bottom line of the tab page
            // directly touches the tab items, so choose a color that fits seamlessly
            if (bNoTabPage)
                rRenderContext.SetLineColor(rStyleSettings.GetDialogColor());
            else
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(Point(1, aRect.Bottom() - 1), Point(aRect.Right() - 1, aRect.Bottom() - 1));
            rRenderContext.DrawLine(Point(aRect.Right() - 1, aRect.Top() + nTopOff), Point(aRect.Right() - 1, aRect.Bottom() - 1));
            if (bNoTabPage)
                rRenderContext.SetLineColor(rStyleSettings.GetDialogColor());
            else
                rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
            rRenderContext.DrawLine(Point(0, aRect.Bottom()), Point(aRect.Right(), aRect.Bottom()));
            rRenderContext.DrawLine(Point(aRect.Right(), aRect.Top() + nTopOff), Point(aRect.Right(), aRect.Bottom()));
        }
        else
        {
            rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
            rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
        }
    }

    if (!mpTabCtrlData->maItemList.empty() && mpTabCtrlData->mpListBox == nullptr)
    {
        // Some native toolkits (GTK+) draw tabs right-to-left, with an
        // overlap between adjacent tabs
        bool bDrawTabsRTL = rRenderContext.IsNativeControlSupported(ControlType::TabItem, ControlPart::TabsDrawRtl);
        ImplTabItem* pFirstTab = nullptr;
        ImplTabItem* pLastTab = nullptr;
        size_t idx;

        // Event though there is a tab overlap with GTK+, the first tab is not
        // overlapped on the left side. Other toolkits ignore this option.
        if (bDrawTabsRTL)
        {
            pFirstTab = mpTabCtrlData->maItemList.data();
            pLastTab = pFirstTab + mpTabCtrlData->maItemList.size() - 1;
            idx = mpTabCtrlData->maItemList.size() - 1;
        }
        else
        {
            pLastTab = mpTabCtrlData->maItemList.data();
            pFirstTab = pLastTab + mpTabCtrlData->maItemList.size() - 1;
            idx = 0;
        }

        while (idx < mpTabCtrlData->maItemList.size())
        {
            ImplTabItem* pItem = &mpTabCtrlData->maItemList[idx];

            if ((pItem != pCurItem) && (pItem->mbEnabled))
            {
                vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
                aClipRgn.Intersect(pItem->maRect);
                if (!rRect.IsEmpty())
                    aClipRgn.Intersect(rRect);
                if (!aClipRgn.IsEmpty())
                {
                    ImplDrawItem(rRenderContext, pItem, aCurRect,
                                 pItem == pFirstTab, pItem == pLastTab);
                }
            }

            if (bDrawTabsRTL)
                idx--;
            else
                idx++;
        }

        if (pCurItem)
        {
            vcl::Region aClipRgn(rRenderContext.GetActiveClipRegion());
            aClipRgn.Intersect(pCurItem->maRect);
            if (!rRect.IsEmpty())
                aClipRgn.Intersect(rRect);
            if (!aClipRgn.IsEmpty())
            {
                ImplDrawItem(rRenderContext, pCurItem, aCurRect,
                             pCurItem == pFirstTab, pCurItem == pLastTab);
            }
        }
    }

    if (HasFocus())
        ImplShowFocus();

    mbSmallInvalidate = true;

    Control::Paint(rRenderContext, rRect);
}

Size NotebookbarTabControlBase::calculateRequisition() const
{
    Size aOptimalPageSize(0, 0);

    sal_uInt16 nOrigPageId = GetCurPageId();
    for (auto const& item : mpTabCtrlData->maItemList)
    {
        const TabPage *pPage = item.mpTabPage;
        //it's a real nuisance if the page is not inserted yet :-(
        //We need to force all tabs to exist to get overall optimal size for dialog
        if (!pPage)
        {
            NotebookbarTabControlBase *pThis = const_cast<NotebookbarTabControlBase*>(this);
            pThis->SetCurPageId(item.mnId);
            pThis->ActivatePage();
            pPage = item.mpTabPage;
        }

        if (!pPage)
            continue;

        Size aPageSize(VclContainer::getLayoutRequisition(*pPage));

        if (aPageSize.Width() > aOptimalPageSize.Width())
            aOptimalPageSize.setWidth( aPageSize.Width() );
        if (aPageSize.Height() > aOptimalPageSize.Height())
            aOptimalPageSize.setHeight( aPageSize.Height() );
    }

    //fdo#61940 If we were forced to activate pages in order to on-demand
    //create them to get their optimal size, then switch back to the original
    //page and re-activate it
    if (nOrigPageId != GetCurPageId())
    {
        NotebookbarTabControlBase *pThis = const_cast<NotebookbarTabControlBase*>(this);
        pThis->SetCurPageId(nOrigPageId);
        pThis->ActivatePage();
    }

    long nTabLabelsBottom = 0, nTabLabelsRight = 0;
    for (sal_uInt16 nPos(0), sizeList(static_cast <sal_uInt16> (mpTabCtrlData->maItemList.size()));
            nPos < sizeList; ++nPos)
    {
        NotebookbarTabControlBase* pThis = const_cast<NotebookbarTabControlBase*>(this);

        tools::Rectangle aTabRect = pThis->ImplGetTabRect(nPos, aOptimalPageSize.Width(), LONG_MAX);
        if (aTabRect.Bottom() > nTabLabelsBottom)
        {
            nTabLabelsBottom = aTabRect.Bottom();
            m_nHeaderHeight = aTabRect.Bottom();
        }
        if (aTabRect.Right() > nTabLabelsRight)
            nTabLabelsRight = aTabRect.Right();
    }

    Size aOptimalSize(aOptimalPageSize);
    aOptimalSize.AdjustHeight(nTabLabelsBottom );
    aOptimalSize.setWidth( std::max(nTabLabelsRight, aOptimalSize.Width()) );

    aOptimalSize.AdjustWidth(TAB_OFFSET * 2 );
    aOptimalSize.AdjustHeight(TAB_OFFSET * 2 );

    return aOptimalSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
