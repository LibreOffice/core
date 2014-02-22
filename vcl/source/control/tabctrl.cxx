/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "tools/debug.hxx"
#include "tools/rc.h"

#include "vcl/svapp.hxx"
#include "vcl/help.hxx"
#include "vcl/event.hxx"
#include "vcl/menu.hxx"
#include "vcl/button.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/controllayout.hxx"
#include "vcl/layout.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/settings.hxx"

#include "controldata.hxx"
#include "svdata.hxx"
#include "window.h"

#include <boost/unordered_map.hpp>
#include <vector>



struct ImplTabItem
{
    sal_uInt16          mnId;
    TabPage*            mpTabPage;
    OUString            maText;
    OUString            maFormatText;
    OUString            maHelpText;
    OString             maHelpId;
    OString             maTabName;
    Rectangle           maRect;
    sal_uInt16          mnLine;
    bool                mbFullVisible;
    bool                mbEnabled;
    Image               maTabImage;

    ImplTabItem()
    : mnId( 0 ), mpTabPage( NULL ),
      mnLine( 0 ), mbFullVisible( false ), mbEnabled( true )
    {}
};



struct ImplTabCtrlData
{
    boost::unordered_map< int, int >        maLayoutPageIdToLine;
    boost::unordered_map< int, int >        maLayoutLineToPageId;
    std::vector< Rectangle >        maTabRectangles;
    Point                           maItemsOffset;       
    std::vector< ImplTabItem >      maItemList;
    ListBox*                        mpListBox;
};



#define TAB_OFFSET          3
#define TAB_TABOFFSET_X     3
#define TAB_TABOFFSET_Y     3
#define TAB_EXTRASPACE_X    6
#define TAB_BORDER_LEFT     1
#define TAB_BORDER_TOP      1
#define TAB_BORDER_RIGHT    2
#define TAB_BORDER_BOTTOM   2


#define TAB_PAGERECT        0xFFFF



void TabControl::ImplInit( Window* pParent, WinBits nStyle )
{
    mbLayoutDirty = true;

    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Control::ImplInit( pParent, nStyle, NULL );

    mnLastWidth                 = 0;
    mnLastHeight                = 0;
    mnBtnSize                   = 0;
    mnMaxPageWidth              = 0;
    mnActPageId                 = 0;
    mnCurPageId                 = 0;
    mbFormat                    = true;
    mbRestoreHelpId             = false;
    mbRestoreUnqId              = false;
    mbSmallInvalidate           = false;
    mpTabCtrlData               = new ImplTabCtrlData;
    mpTabCtrlData->mpListBox    = NULL;


    ImplInitSettings( true, true, true );

    if( (nStyle & WB_DROPDOWN) )
    {
        mpTabCtrlData->mpListBox = new ListBox( this, WB_DROPDOWN );
        mpTabCtrlData->mpListBox->SetPosSizePixel( Point( 0, 0 ), Size( 200, 20 ) );
        mpTabCtrlData->mpListBox->SetSelectHdl( LINK( this, TabControl, ImplListBoxSelectHdl ) );
        mpTabCtrlData->mpListBox->Show();
    }

    
    
    if( IsNativeControlSupported(CTRL_TAB_PANE, PART_ENTIRE_CONTROL) )
        EnableChildTransparentMode( true );

    if ( pParent->IsDialog() )
        pParent->AddChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );
}



const Font& TabControl::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetAppFont();
}


const Color& TabControl::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetButtonTextColor();
}



void TabControl::ImplInitSettings( bool bFont,
                                   bool bForeground, bool bBackground )
{
    Control::ImplInitSettings( bFont, bForeground );

    if ( bBackground )
    {
        Window* pParent = GetParent();
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled()
            || IsNativeControlSupported(CTRL_TAB_PANE, PART_ENTIRE_CONTROL)
            || IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL) ) )

        {
            
            
            EnableChildTransparentMode( true );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( true );
            SetBackground();
            ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode( 0 );
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



TabControl::TabControl( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_TABCONTROL )
{
    ImplInit( pParent, nStyle );
    OSL_TRACE("*** TABCONTROL no notabs? %s", ( GetStyle() & WB_NOBORDER ) ? "true" : "false" );
}



TabControl::TabControl( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_TABCONTROL )
{
    rResId.SetRT( RSC_TABCONTROL );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}



void TabControl::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uLong nObjMask = ReadLongRes();

    if ( nObjMask & RSC_TABCONTROL_ITEMLIST )
    {
        sal_uLong nEle = ReadLongRes();

        
        for( sal_uLong i = 0; i < nEle; i++ )
        {
            InsertPage( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }
}



TabControl::~TabControl()
{
    if ( GetParent()->IsDialog() )
        GetParent()->RemoveChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );

    ImplFreeLayoutData();

    
    if ( mpTabCtrlData )
    {
        if( mpTabCtrlData->mpListBox )
            delete mpTabCtrlData->mpListBox;
        delete mpTabCtrlData;
    }
}



ImplTabItem* TabControl::ImplGetItem( sal_uInt16 nId ) const
{
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if( it->mnId == nId )
            return &(*it);
    }

    return NULL;
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
            aImageSize.Width() += GetTextHeight()/4;
    }
    aSize.Width() += aImageSize.Width();
    if( aImageSize.Height() > aSize.Height() )
        aSize.Height() = aImageSize.Height();

    aSize.Width()  += TAB_TABOFFSET_X*2;
    aSize.Height() += TAB_TABOFFSET_Y*2;

    Rectangle aCtrlRegion( Point( 0, 0 ), aSize );
    Rectangle aBoundingRgn, aContentRgn;
    const ImplControlValue aControlValue;
    if(GetNativeControlRegion( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion,
                                           CTRL_STATE_ENABLED, aControlValue, OUString(),
                                           aBoundingRgn, aContentRgn ) )
    {
        return aContentRgn.GetSize();
    }

    
    
    if ( pItem->maFormatText.getLength() < TAB_EXTRASPACE_X )
        aSize.Width() += TAB_EXTRASPACE_X-pItem->maFormatText.getLength();

    
    if ( aSize.Width()+4 >= nMaxWidth )
    {
        OUString aAppendStr("...");
        pItem->maFormatText += aAppendStr;
        do
        {
            pItem->maFormatText = pItem->maFormatText.replaceAt( pItem->maFormatText.getLength()-aAppendStr.getLength()-1, 1, "" );
            aSize.Width() = GetCtrlTextWidth( pItem->maFormatText );
            aSize.Width() += aImageSize.Width();
            aSize.Width() += TAB_TABOFFSET_X*2;
        }
        while ( (aSize.Width()+4 >= nMaxWidth) && (pItem->maFormatText.getLength() > aAppendStr.getLength()) );
        if ( aSize.Width()+4 >= nMaxWidth )
        {
            pItem->maFormatText = ".";
            aSize.Width() = 1;
        }
    }

    if( pItem->maFormatText.isEmpty() )
    {
        if( aSize.Height() < aImageSize.Height()+4 ) 
            aSize.Height() = aImageSize.Height()+4;
    }

    return aSize;
}





namespace MinimumRaggednessWrap
{
    std::deque<size_t> GetEndOfLineIndexes(const std::vector<sal_Int32> rWidthsOf, sal_Int32 nLineWidth)
    {
        ++nLineWidth;

        size_t nWidthsCount = rWidthsOf.size();
        std::vector<sal_Int32> aCosts(nWidthsCount * nWidthsCount);

        
        
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

        
        if (aFunction[nWidthsCount - 1] == SAL_MAX_INT32)
            return aSolution;

        
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

Rectangle TabControl::ImplGetTabRect( sal_uInt16 nItemPos, long nWidth, long nHeight )
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
        ? Rectangle( Point( TAB_OFFSET, TAB_OFFSET ), Size( nW, nH ) )
        : Rectangle();
    }

    if ( nItemPos == TAB_PAGERECT )
    {
        sal_uInt16 nLastPos;
        if ( mnCurPageId )
            nLastPos = GetPagePos( mnCurPageId );
        else
            nLastPos = 0;

        Rectangle aRect = ImplGetTabRect( nLastPos, nWidth, nHeight );
        long nW = nWidth-TAB_OFFSET*2;
        long nH = nHeight-aRect.Bottom()-TAB_OFFSET*2;
        aRect = (nW > 0 && nH > 0)
        ? Rectangle( Point( TAB_OFFSET, aRect.Bottom()+TAB_OFFSET ), Size( nW, nH ) )
        : Rectangle();
        return aRect;
    }

    nWidth -= 1;

    if ( (nWidth <= 0) || (nHeight <= 0) )
        return Rectangle();

    if ( mbFormat || (mnLastWidth != nWidth) || (mnLastHeight != nHeight) )
    {
        Font aFont( GetFont() );
        aFont.SetTransparent( true );
        SetFont( aFont );

        Size            aSize;
        const long      nOffsetX = 2 + GetItemsOffset().X();
        const long      nOffsetY = 2 + GetItemsOffset().Y();
        long            nX = nOffsetX;
        long            nY = nOffsetY;
        long            nMaxWidth = nWidth;
        sal_uInt16          nPos = 0;

        
        

        
        std::vector<sal_Int32> aWidths;
        for( std::vector<ImplTabItem>::iterator it = mpTabCtrlData->maItemList.begin();
             it != mpTabCtrlData->maItemList.end(); ++it )
        {
            aWidths.push_back(ImplGetItemSize( &(*it), nMaxWidth ).Width());
        }

        
        std::deque<size_t> aBreakIndexes(MinimumRaggednessWrap::GetEndOfLineIndexes(aWidths, nMaxWidth - nOffsetX - 2));

        if ( (mnMaxPageWidth > 0) && (mnMaxPageWidth < nMaxWidth) )
            nMaxWidth = mnMaxPageWidth;
        nMaxWidth -= GetItemsOffset().X();

        sal_uInt16          nLines = 0;
        sal_uInt16          nCurLine = 0;
        long            nLineWidthAry[100];
        sal_uInt16          nLinePosAry[101];

        nLineWidthAry[0] = 0;
        nLinePosAry[0] = 0;
        size_t nIndex = 0;
        for( std::vector<ImplTabItem>::iterator it = mpTabCtrlData->maItemList.begin();
             it != mpTabCtrlData->maItemList.end(); ++it, ++nIndex )
        {
            aSize = ImplGetItemSize( &(*it), nMaxWidth );

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

            Rectangle aNewRect( Point( nX, nY ), aSize );
            if ( mbSmallInvalidate && (it->maRect != aNewRect) )
                mbSmallInvalidate = false;
            it->maRect = aNewRect;
            it->mnLine = nLines;
            it->mbFullVisible = true;

            nLineWidthAry[nLines] += aSize.Width();
            nX += aSize.Width();

            if ( it->mnId == mnCurPageId )
                nCurLine = nLines;

            nPos++;
        }

        if ( nLines && !mpTabCtrlData->maItemList.empty() )
        {
            long    nDX = 0;
            long    nModDX = 0;
            long    nIDX = 0;
            sal_uInt16  i;
            sal_uInt16  n;
            long    nLineHeightAry[100];
            long    nIH = mpTabCtrlData->maItemList[0].maRect.Bottom()-2;

            i = 0;
            while ( i < nLines+1 )
            {
                if ( i <= nCurLine )
                    nLineHeightAry[i] = nIH*(nLines-(nCurLine-i)) + GetItemsOffset().Y();
                else
                    nLineHeightAry[i] = nIH*(i-nCurLine-1) + GetItemsOffset().Y();
                i++;
            }

            i = 0;
            n = 0;
            nLinePosAry[nLines+1] = (sal_uInt16)mpTabCtrlData->maItemList.size();
            for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                 it != mpTabCtrlData->maItemList.end(); ++it )
            {
                if ( i == nLinePosAry[n] )
                {
                    if ( n == nLines+1 )
                        break;

                    nIDX = 0;
                    if( nLinePosAry[n+1]-i > 0 )
                    {
                        nDX = (nWidth-nOffsetX-nLineWidthAry[n]) / (nLinePosAry[n+1]-i);
                        nModDX = (nWidth-nOffsetX-nLineWidthAry[n]) % (nLinePosAry[n+1]-i);
                    }
                    else
                    {
                        
                        nDX = 0;
                        nModDX = 0;
                    }
                    n++;
                }

                it->maRect.Left()   += nIDX;
                it->maRect.Right()  += nIDX+nDX;
                it->maRect.Top()     = nLineHeightAry[n-1];
                it->maRect.Bottom()  = nLineHeightAry[n-1]+nIH;
                nIDX += nDX;

                if ( nModDX )
                {
                    nIDX++;
                    it->maRect.Right()++;
                    nModDX--;
                }

                i++;
            }
        }
        else
        {
            if(ImplGetSVData()->maNWFData.mbCenteredTabs)
            {
                int nRightSpace=nMaxWidth;
                for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                     it != mpTabCtrlData->maItemList.end(); ++it )
                {
                    nRightSpace-=it->maRect.Right()-it->maRect.Left();
                }
                for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                     it != mpTabCtrlData->maItemList.end(); ++it )
                {
                    it->maRect.Left()+=(int) (nRightSpace/2);
                    it->maRect.Right()+=(int) (nRightSpace/2);
                }
            }
        }

        mnLastWidth     = nWidth;
        mnLastHeight    = nHeight;
        mbFormat        = false;
    }

    return size_t(nItemPos) < mpTabCtrlData->maItemList.size() ? mpTabCtrlData->maItemList[nItemPos].maRect : Rectangle();
}



void TabControl::ImplChangeTabPage( sal_uInt16 nId, sal_uInt16 nOldId )
{
    ImplFreeLayoutData();

    ImplTabItem*    pOldItem = ImplGetItem( nOldId );
    ImplTabItem*    pItem = ImplGetItem( nId );
    TabPage*        pOldPage = (pOldItem) ? pOldItem->mpTabPage : NULL;
    TabPage*        pPage = (pItem) ? pItem->mpTabPage : NULL;
    Window*         pCtrlParent = GetParent();

    if ( IsReallyVisible() && IsUpdateMode() )
    {
        sal_uInt16 nPos = GetPagePos( nId );
        Rectangle aRect = ImplGetTabRect( nPos );

        if ( !pOldItem || (pOldItem->mnLine != pItem->mnLine) )
        {
            aRect.Left() = 0;
            aRect.Top() = 0;
            aRect.Right() = Control::GetOutputSizePixel().Width();
        }
        else
        {
            aRect.Left()    -= 3;
            aRect.Top()     -= 2;
            aRect.Right()   += 3;
            Invalidate( aRect );
            nPos = GetPagePos( nOldId );
            aRect = ImplGetTabRect( nPos );
            aRect.Left()    -= 3;
            aRect.Top()     -= 2;
            aRect.Right()   += 3;
        }
        Invalidate( aRect );
    }

    if ( pOldPage == pPage )
        return;

    Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );

    if ( pOldPage )
    {
        if ( mbRestoreHelpId )
            pCtrlParent->SetHelpId( OString() );
        if ( mbRestoreUnqId )
            pCtrlParent->SetUniqueId( OString() );
        pOldPage->DeactivatePage();
    }

    if ( pPage )
    {
        if (  ( GetStyle() & WB_NOBORDER ) )
        {
            Rectangle aRectNoTab( (const Point&)Point( 0, 0 ), GetSizePixel() );
            pPage->SetPosSizePixel( aRectNoTab.TopLeft(), aRectNoTab.GetSize() );
        }
        else
            pPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );

        
        
        if ( GetHelpId().isEmpty() )
        {
            mbRestoreHelpId = true;
            pCtrlParent->SetHelpId( pPage->GetHelpId() );
        }
        if ( pCtrlParent->GetUniqueId().isEmpty() )
        {
            mbRestoreUnqId = true;
            pCtrlParent->SetUniqueId( pPage->GetUniqueId() );
        }

        pPage->ActivatePage();
        pPage->Show();

        if ( pOldPage && pOldPage->HasChildPathFocus() )
        {
            sal_uInt16  n = 0;
            Window* pFirstChild = pPage->ImplGetDlgWindow( n, DLGWINDOW_FIRST );
            if ( pFirstChild )
                pFirstChild->ImplControlFocus( GETFOCUS_INIT );
            else
                GrabFocus();
        }
    }

    if ( pOldPage )
        pOldPage->Hide();

    
    
    
    if( IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL ) )
    {
        aRect.Left()   -= TAB_OFFSET;
        aRect.Top()    -= TAB_OFFSET;
        aRect.Right()  += TAB_OFFSET;
        aRect.Bottom() += TAB_OFFSET;
    }

    Invalidate( aRect );
}



bool TabControl::ImplPosCurTabPage()
{
    
    ImplTabItem* pItem = ImplGetItem( GetCurPageId() );
    if ( pItem && pItem->mpTabPage )
    {
        if (  ( GetStyle() & WB_NOBORDER ) )
        {
            Rectangle aRectNoTab( (const Point&)Point( 0, 0 ), GetSizePixel() );
            pItem->mpTabPage->SetPosSizePixel( aRectNoTab.TopLeft(), aRectNoTab.GetSize() );
            return true;
        }
        Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
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
    Rectangle                aRect       = ImplGetTabRect( nCurPos );
    const ImplTabItem&       rItem       = mpTabCtrlData->maItemList[ nCurPos ];
    Size                     aTabSize    = aRect.GetSize();
    Size aImageSize( 0, 0 );
    long                     nTextHeight = GetTextHeight();
    long                     nTextWidth  = GetCtrlTextWidth( rItem.maFormatText );
    sal_uInt16                   nOff;

    if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_MONO) )
        nOff = 1;
    else
        nOff = 0;

    if( !! rItem.maTabImage )
    {
        aImageSize = rItem.maTabImage.GetSizePixel();
        if( !rItem.maFormatText.isEmpty() )
            aImageSize.Width() += GetTextHeight()/4;
    }

    if( !rItem.maFormatText.isEmpty() )
    {
        
        aRect.Left()   = aRect.Left()+aImageSize.Width()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-1-1;
        aRect.Top()    = aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-1-1;
        aRect.Right()  = aRect.Left()+nTextWidth+2;
        aRect.Bottom() = aRect.Top()+nTextHeight+2;
    }
    else
    {
        
        long nXPos = aRect.Left()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-1;
        long nYPos = aRect.Top();
        if( aImageSize.Height() < aRect.GetHeight() )
            nYPos += (aRect.GetHeight() - aImageSize.Height())/2;

        aRect.Left() = nXPos - 2;
        aRect.Top() = nYPos - 2;
        aRect.Right() = aRect.Left() + aImageSize.Width() + 4;
        aRect.Bottom() = aRect.Top() + aImageSize.Height() + 4;
    }
    ShowFocus( aRect );
}



void TabControl::ImplDrawItem( ImplTabItem* pItem, const Rectangle& rCurRect, bool bLayout, bool bFirstInGroup, bool bLastInGroup, bool /* bIsCurrentItem */ )
{
    if ( pItem->maRect.IsEmpty() )
        return;

    if( bLayout )
    {
        if( !HasLayoutData() )
        {
            mpControlData->mpLayoutData = new vcl::ControlLayoutData();
            mpTabCtrlData->maLayoutLineToPageId.clear();
            mpTabCtrlData->maLayoutPageIdToLine.clear();
            mpTabCtrlData->maTabRectangles.clear();
        }
    }

    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aRect = pItem->maRect;
    long                    nLeftBottom = aRect.Bottom();
    long                    nRightBottom = aRect.Bottom();
    bool                    bLeftBorder = true;
    bool                    bRightBorder = true;
    sal_uInt16                  nOff;
    bool                    bNativeOK = false;

    sal_uInt16 nOff2 = 0;
    sal_uInt16 nOff3 = 0;

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nOff = 1;
    else
        nOff = 0;

    
    if ( pItem->mnId == mnCurPageId )
    {
        nOff2 = 2;
        if( ! ImplGetSVData()->maNWFData.mbNoActiveTabTextRaise )
            nOff3 = 1;
    }
    else
    {
        Point aLeftTestPos = aRect.BottomLeft();
        Point aRightTestPos = aRect.BottomRight();
        if ( aLeftTestPos.Y() == rCurRect.Bottom() )
        {
            aLeftTestPos.X() -= 2;
            if ( rCurRect.IsInside( aLeftTestPos ) )
                bLeftBorder = false;
            aRightTestPos.X() += 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                bRightBorder = false;
        }
        else
        {
            if ( rCurRect.IsInside( aLeftTestPos ) )
                nLeftBottom -= 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                nRightBottom -= 2;
        }
    }

    if( !bLayout && (bNativeOK = IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL)) )
    {
        Rectangle           aCtrlRegion( pItem->maRect );
        ControlState        nState = 0;

        if( pItem->mnId == mnCurPageId )
        {
            nState |= CTRL_STATE_SELECTED;
            
            if ( HasFocus() )
                nState |= CTRL_STATE_FOCUSED;
        }
        if ( IsEnabled() )
            nState |= CTRL_STATE_ENABLED;
        if( IsMouseOver() && pItem->maRect.IsInside( GetPointerPosPixel() ) )
        {
            nState |= CTRL_STATE_ROLLOVER;
            for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                 it != mpTabCtrlData->maItemList.end(); ++it )
            {
                if( (&(*it) != pItem) && (it->maRect.IsInside( GetPointerPosPixel() ) ) )
                {
                    nState &= ~CTRL_STATE_ROLLOVER; 
                    break;
                }
            }
        }

        TabitemValue tiValue;
        if(pItem->maRect.Left() < 5)
            tiValue.mnAlignment |= TABITEM_LEFTALIGNED;
        if(pItem->maRect.Right() > mnLastWidth - 5)
            tiValue.mnAlignment |= TABITEM_RIGHTALIGNED;
        if ( bFirstInGroup )
            tiValue.mnAlignment |= TABITEM_FIRST_IN_GROUP;
        if ( bLastInGroup )
            tiValue.mnAlignment |= TABITEM_LAST_IN_GROUP;

        bNativeOK = DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                    tiValue, OUString() );
    }

    if( ! bLayout && !bNativeOK )
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawPixel( Point( aRect.Left()+1-nOff2, aRect.Top()+1-nOff2 ) );    
            if ( bLeftBorder )
            {
                DrawLine( Point( aRect.Left()-nOff2, aRect.Top()+2-nOff2 ),
                          Point( aRect.Left()-nOff2, nLeftBottom-1 ) );
            }
            DrawLine( Point( aRect.Left()+2-nOff2, aRect.Top()-nOff2 ),         
                      Point( aRect.Right()+nOff2-3, aRect.Top()-nOff2 ) );      

            if ( bRightBorder )
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( Point( aRect.Right()+nOff2-2, aRect.Top()+1-nOff2 ),
                          Point( aRect.Right()+nOff2-2, nRightBottom-1 ) );

                SetLineColor( rStyleSettings.GetDarkShadowColor() );
                DrawLine( Point( aRect.Right()+nOff2-1, aRect.Top()+3-nOff2 ),
                          Point( aRect.Right()+nOff2-1, nRightBottom-1 ) );
            }
        }
        else
        {
            SetLineColor( Color( COL_BLACK ) );
            DrawPixel( Point( aRect.Left()+1-nOff2, aRect.Top()+1-nOff2 ) );
            DrawPixel( Point( aRect.Right()+nOff2-2, aRect.Top()+1-nOff2 ) );
            if ( bLeftBorder )
            {
                DrawLine( Point( aRect.Left()-nOff2, aRect.Top()+2-nOff2 ),
                          Point( aRect.Left()-nOff2, nLeftBottom-1 ) );
            }
            DrawLine( Point( aRect.Left()+2-nOff2, aRect.Top()-nOff2 ),
                      Point( aRect.Right()-3, aRect.Top()-nOff2 ) );
            if ( bRightBorder )
            {
            DrawLine( Point( aRect.Right()+nOff2-1, aRect.Top()+2-nOff2 ),
                      Point( aRect.Right()+nOff2-1, nRightBottom-1 ) );
            }
        }
    }

    if( bLayout )
    {
        int nLine = mpControlData->mpLayoutData->m_aLineIndices.size();
        mpControlData->mpLayoutData->m_aLineIndices.push_back( mpControlData->mpLayoutData->m_aDisplayText.getLength() );
        mpTabCtrlData->maLayoutPageIdToLine[ (int)pItem->mnId ] = nLine;
        mpTabCtrlData->maLayoutLineToPageId[ nLine ] = (int)pItem->mnId;
        mpTabCtrlData->maTabRectangles.push_back( aRect );
    }

    
    
    Font aFont( GetFont() );
    aFont.SetTransparent( true );
    SetFont( aFont );

    Size aTabSize = aRect.GetSize();
    Size aImageSize( 0, 0 );
    long nTextHeight = GetTextHeight();
    long nTextWidth = GetCtrlTextWidth( pItem->maFormatText );
    if( !! pItem->maTabImage )
    {
        aImageSize = pItem->maTabImage.GetSizePixel();
        if( !pItem->maFormatText.isEmpty() )
            aImageSize.Width() += GetTextHeight()/4;
    }
    long nXPos = aRect.Left()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-nOff3;
    long nYPos = aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-nOff3;
    if( !pItem->maFormatText.isEmpty() )
    {
        sal_uInt16 nStyle = TEXT_DRAW_MNEMONIC;
        if( ! pItem->mbEnabled )
            nStyle |= TEXT_DRAW_DISABLE;
        DrawCtrlText( Point( nXPos + aImageSize.Width(), nYPos ),
                      pItem->maFormatText, 0,
                      pItem->maFormatText.getLength(), nStyle,
                      bLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : NULL,
                      bLayout ? &mpControlData->mpLayoutData->m_aDisplayText : NULL
                      );
    }

    if( !! pItem->maTabImage )
    {
        Point aImgTL( nXPos, aRect.Top() );
        if( aImageSize.Height() < aRect.GetHeight() )
            aImgTL.Y() += (aRect.GetHeight() - aImageSize.Height())/2;
        DrawImage( aImgTL, pItem->maTabImage, pItem->mbEnabled ? 0 : IMAGE_DRAW_DISABLE );
    }
}



bool TabControl::ImplHandleKeyEvent( const KeyEvent& rKeyEvent )
{
    bool nRet = false;

    if ( GetPageCount() > 1 )
    {
        KeyCode         aKeyCode = rKeyEvent.GetKeyCode();
        sal_uInt16          nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.IsMod1() )
        {
            if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                {
                    ImplActivateTabPage( false );
                    nRet = true;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    ImplActivateTabPage( true );
                    nRet = true;
                }
            }
        }
    }

    return nRet;
}




IMPL_LINK_NOARG(TabControl, ImplListBoxSelectHdl)
{
    SelectTabPage( GetPageId( mpTabCtrlData->mpListBox->GetSelectEntryPos() ) );
    return 0;
}



IMPL_LINK( TabControl, ImplWindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( pEvent && pEvent->ISA( VclWindowEvent ) && (pEvent->GetId() == VCLEVENT_WINDOW_KEYINPUT) )
    {
        VclWindowEvent* pWindowEvent = static_cast< VclWindowEvent* >(pEvent);
        
        if ( !IsWindowOrChild( pWindowEvent->GetWindow() ) )
        {
            KeyEvent* pKeyEvent = static_cast< KeyEvent* >(pWindowEvent->GetData());
            ImplHandleKeyEvent( *pKeyEvent );
        }
    }
    return 0;
}




void TabControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( mpTabCtrlData->mpListBox == NULL )
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
        KeyCode aKeyCode = rKEvt.GetKeyCode();
        sal_uInt16  nKeyCode = aKeyCode.GetCode();

        if ( (nKeyCode == KEY_LEFT) || (nKeyCode == KEY_RIGHT) )
        {
            bool bNext = (nKeyCode == KEY_RIGHT);
            ImplActivateTabPage( bNext );
        }
    }

    Control::KeyInput( rKEvt );
}



void TabControl::Paint( const Rectangle& rRect )
{
    if (  !( GetStyle() & WB_NOBORDER ) )
        ImplPaint( rRect, false );
}



void TabControl::ImplPaint( const Rectangle& rRect, bool bLayout )
{
    if( ! bLayout )
        HideFocus();

    
    Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );

    
    ImplTabItem* pCurItem = NULL;
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->mnId == mnCurPageId )
        {
            pCurItem = &(*it);
            break;
        }
    }

    
    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aCurRect;
    aRect.Left()   -= TAB_OFFSET;
    aRect.Top()    -= TAB_OFFSET;
    aRect.Right()  += TAB_OFFSET;
    aRect.Bottom() += TAB_OFFSET;

    
    
    
    
    bool bNoTabPage = false;
    TabPage* pCurPage = pCurItem ? pCurItem->mpTabPage : NULL;
    if( !pCurPage || !pCurPage->IsVisible() )
    {
        bNoTabPage = true;
        aRect.Left()-=10;
        aRect.Right()+=10;
    }

    bool bNativeOK = false;
    if( ! bLayout && (bNativeOK = IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL) ) )
    {
        const ImplControlValue aControlValue;

        ControlState nState = CTRL_STATE_ENABLED;
        int part = PART_ENTIRE_CONTROL;
        if ( !IsEnabled() )
            nState &= ~CTRL_STATE_ENABLED;
        if ( HasFocus() )
            nState |= CTRL_STATE_FOCUSED;

        Region aClipRgn( GetActiveClipRegion() );
        aClipRgn.Intersect( aRect );
        if( !rRect.IsEmpty() )
            aClipRgn.Intersect( rRect );

        if( !aClipRgn.IsEmpty() )
            bNativeOK = DrawNativeControl( CTRL_TAB_PANE, part, aRect, nState,
                aControlValue, OUString() );
    }
    else
    {
        long nTopOff = 1;
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            SetLineColor( rStyleSettings.GetLightColor() );
        else
            SetLineColor( Color( COL_BLACK ) );
        if ( pCurItem && !pCurItem->maRect.IsEmpty() )
        {
            aCurRect = pCurItem->maRect;
            if( ! bLayout )
                DrawLine( aRect.TopLeft(), Point( aCurRect.Left()-2, aRect.Top() ) );
            if ( aCurRect.Right()+1 < aRect.Right() )
            {
                if( ! bLayout )
                    DrawLine( Point( aCurRect.Right(), aRect.Top() ), aRect.TopRight() );
            }
            else
                nTopOff = 0;
        }
        else
            if( ! bLayout )
                DrawLine( aRect.TopLeft(), aRect.TopRight() );

        if( ! bLayout )
        {
            DrawLine( aRect.TopLeft(), aRect.BottomLeft() );

            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                
                
                if( bNoTabPage )
                    SetLineColor( rStyleSettings.GetDialogColor() );
                else
                    SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( Point( 1, aRect.Bottom()-1 ),
                        Point( aRect.Right()-1, aRect.Bottom()-1 ) );
                DrawLine( Point( aRect.Right()-1, aRect.Top()+nTopOff ),
                        Point( aRect.Right()-1, aRect.Bottom()-1 ) );
                if( bNoTabPage )
                    SetLineColor( rStyleSettings.GetDialogColor() );
                else
                    SetLineColor( rStyleSettings.GetDarkShadowColor() );
                DrawLine( Point( 0, aRect.Bottom() ),
                        Point( aRect.Right(), aRect.Bottom() ) );
                DrawLine( Point( aRect.Right(), aRect.Top()+nTopOff ),
                        Point( aRect.Right(), aRect.Bottom() ) );
            }
            else
            {
                DrawLine( aRect.TopRight(), aRect.BottomRight() );
                DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
            }
        }
    }

    if ( !mpTabCtrlData->maItemList.empty() && mpTabCtrlData->mpListBox == NULL )
    {
        
        
        bool            bDrawTabsRTL = IsNativeControlSupported( CTRL_TAB_ITEM, PART_TABS_DRAW_RTL );
        ImplTabItem *   pFirstTab = NULL;
        ImplTabItem *   pLastTab = NULL;
        size_t idx;

        
        
        if ( bDrawTabsRTL )
        {
            pFirstTab = &mpTabCtrlData->maItemList.front();
            pLastTab = &mpTabCtrlData->maItemList.back();
            idx = mpTabCtrlData->maItemList.size()-1;
        }
        else
        {
            pLastTab = &mpTabCtrlData->maItemList.back();
            pFirstTab = &mpTabCtrlData->maItemList.front();
            idx = 0;
        }

        while ( idx < mpTabCtrlData->maItemList.size() )
        {
            ImplTabItem* pItem = &mpTabCtrlData->maItemList[idx];
            if ( pItem != pCurItem )
            {
                Region aClipRgn( GetActiveClipRegion() );
                aClipRgn.Intersect( pItem->maRect );
                if( !rRect.IsEmpty() )
                    aClipRgn.Intersect( rRect );
                if( bLayout || !aClipRgn.IsEmpty() )
                    ImplDrawItem( pItem, aCurRect, bLayout, (pItem==pFirstTab), (pItem==pLastTab), false );
            }

            if ( bDrawTabsRTL )
                idx--;
            else
                idx++;
        }

        if ( pCurItem )
        {
            Region aClipRgn( GetActiveClipRegion() );
            aClipRgn.Intersect( pCurItem->maRect );
            if( !rRect.IsEmpty() )
                aClipRgn.Intersect( rRect );
            if( bLayout || !aClipRgn.IsEmpty() )
                ImplDrawItem( pCurItem, aCurRect, bLayout, (pCurItem==pFirstTab), (pCurItem==pLastTab), true );
        }
    }

    if ( !bLayout && HasFocus() )
        ImplShowFocus();

    if( ! bLayout )
        mbSmallInvalidate = true;
}



void TabControl::setAllocation(const Size &rAllocation)
{
    ImplFreeLayoutData();

    if ( !IsReallyShown() )
        return;

    if( mpTabCtrlData->mpListBox )
    {
        
        Size aTabCtrlSize( GetSizePixel() );
        long nPrefWidth = mpTabCtrlData->mpListBox->get_preferred_size().Width();
        if( nPrefWidth > aTabCtrlSize.Width() )
            nPrefWidth = aTabCtrlSize.Width();
        Size aNewSize( nPrefWidth, LogicToPixel( Size( 12, 12 ), MapMode( MAP_APPFONT ) ).Height() );
        Point aNewPos( (aTabCtrlSize.Width() - nPrefWidth) / 2, 0 );
        mpTabCtrlData->mpListBox->SetPosSizePixel( aNewPos, aNewSize );
    }

    mbFormat = true;

    
    bool bTabPage = ImplPosCurTabPage();

    
    Size aNewSize = rAllocation;
    long nNewWidth = aNewSize.Width();
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( !it->mbFullVisible ||
             (it->maRect.Right()-2 >= nNewWidth) )
        {
            mbSmallInvalidate = false;
            break;
        }
    }

    if ( mbSmallInvalidate )
    {
        Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        aRect.Left()   -= TAB_OFFSET+TAB_BORDER_LEFT;
        aRect.Top()    -= TAB_OFFSET+TAB_BORDER_TOP;
        aRect.Right()  += TAB_OFFSET+TAB_BORDER_RIGHT;
        aRect.Bottom() += TAB_OFFSET+TAB_BORDER_BOTTOM;
        if ( bTabPage )
            Invalidate( aRect, INVALIDATE_NOCHILDREN );
        else
            Invalidate( aRect );

    }
    else
    {
        if ( bTabPage )
            Invalidate( INVALIDATE_NOCHILDREN );
        else
            Invalidate();
    }

    mbLayoutDirty = false;
}

void TabControl::SetPosSizePixel(const Point& rNewPos, const Size& rNewSize)
{
    Window::SetPosSizePixel(rNewPos, rNewSize);
    
    if (mbLayoutDirty)
        setAllocation(rNewSize);
}

void TabControl::SetSizePixel(const Size& rNewSize)
{
    Window::SetSizePixel(rNewSize);
    
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
    if( ! mpTabCtrlData->mpListBox )
        HideFocus();
    Control::LoseFocus();
}



void TabControl::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = rHEvt.KeyboardActivated() ? mnCurPageId : GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        {
            OUString aStr = GetHelpText( nItemId );
            if ( !aStr.isEmpty() )
            {
                Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            OUString aHelpId( OStringToOUString( GetHelpId( nItemId ), RTL_TEXTENCODING_UTF8 ) );
            if ( !aHelpId.isEmpty() )
            {
                
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( aHelpId, this );
                return;
            }
        }

        
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            ImplTabItem* pItem = ImplGetItem( nItemId );
            const OUString& rStr = pItem->maText;
            if ( rStr != pItem->maFormatText )
            {
                Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                if ( !rStr.isEmpty() )
                {
                    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, rStr );
                    else
                        Help::ShowQuickHelp( this, aItemRect, rStr );
                    return;
                }
            }
        }

        if ( rHEvt.GetMode() & HELPMODE_QUICK )
        {
            ImplTabItem* pItem = ImplGetItem( nItemId );
            const OUString& rHelpText = pItem->maHelpText;
            
            if ( !rHelpText.isEmpty() && pItem->maText.isEmpty() && !!pItem->maTabImage )
            {
                Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                Help::ShowQuickHelp( this, aItemRect, rHelpText );
                return;
            }
        }
    }

    Control::RequestHelp( rHEvt );
}



void TabControl::Command( const CommandEvent& rCEvt )
{
    if( (mpTabCtrlData->mpListBox == NULL) && (rCEvt.GetCommand() == COMMAND_CONTEXTMENU) && (GetPageCount() > 1) )
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
            PopupMenu aMenu;
            for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                 it != mpTabCtrlData->maItemList.end(); ++it )
            {
                aMenu.InsertItem( it->mnId, it->maText, MIB_CHECKABLE );
                if ( it->mnId == mnCurPageId )
                    aMenu.CheckItem( it->mnId );
                aMenu.SetHelpId( it->mnId, it->maHelpId );
            }

            sal_uInt16 nId = aMenu.Execute( this, aMenuPos );
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

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplPosCurTabPage();
        if( mpTabCtrlData->mpListBox )
            Resize();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == STATE_CHANGE_ZOOM)  ||
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
}



void TabControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}



Rectangle* TabControl::ImplFindPartRect( const Point& rPt )
{
    ImplTabItem* pFoundItem = NULL;
    int nFound = 0;
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->maRect.IsInside( rPt ) )
        {
            
            nFound++;
            pFoundItem = &(*it);
        }
    }
    
    return nFound == 1 ? &pFoundItem->maRect : NULL;
}

bool TabControl::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            
            if( IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
                    Region aClipRgn;
                    if( pLastRect )
                    {
                        
                        
                        
                        Rectangle aRect(*pLastRect);
                        aRect.Left()-=2;
                        aRect.Right()+=2;
                        aRect.Top()-=3;
                        aClipRgn.Union( aRect );
                    }
                    if( pRect )
                    {
                        
                        
                        
                        Rectangle aRect(*pRect);
                        aRect.Left()-=2;
                        aRect.Right()+=2;
                        aRect.Top()-=3;
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



bool TabControl::Notify( NotifyEvent& rNEvt )
{
    bool nRet = false;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
        nRet = ImplHandleKeyEvent( *rNEvt.GetKeyEvent() );

    return nRet || Control::Notify( rNEvt );
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
    aNewSize.Width() += TAB_OFFSET*2;
    Rectangle aRect = ImplGetTabRect( TAB_PAGERECT,
                                      aNewSize.Width(), aNewSize.Height() );
    aNewSize.Height() += aRect.Top()+TAB_OFFSET;
    Window::SetOutputSizePixel( aNewSize );
}



Size TabControl::GetTabPageSizePixel() const
{
    Rectangle aRect = ((TabControl*)this)->ImplGetTabRect( TAB_PAGERECT );
    return aRect.GetSize();
}



void TabControl::InsertPage( const ResId& rResId, sal_uInt16 nPos )
{
    GetRes( rResId.SetRT( RSC_TABCONTROLITEM ) );

    sal_uLong nObjMask = ReadLongRes();
    sal_uInt16 nItemId  = 1;

    
    if ( nObjMask & RSC_TABCONTROLITEM_ID )
        nItemId = sal::static_int_cast<sal_uInt16>(ReadLongRes());

    
    OUString aTmpStr;
    if( nObjMask & RSC_TABCONTROLITEM_TEXT )
        aTmpStr = ReadStringRes();
    InsertPage( nItemId, aTmpStr, nPos );

    
    if ( nObjMask & RSC_TABCONTROLITEM_PAGERESID )
    {
        
        ReadLongRes();
    }
}



void TabControl::InsertPage( sal_uInt16 nPageId, const OUString& rText,
                             sal_uInt16 nPos )
{
    DBG_ASSERT( nPageId, "TabControl::InsertPage(): PageId == 0" );
    DBG_ASSERT( GetPagePos( nPageId ) == TAB_PAGE_NOTFOUND,
                "TabControl::InsertPage(): PageId already exists" );

    
    ImplTabItem* pItem = NULL;
    if( nPos == TAB_APPEND || size_t(nPos) >= mpTabCtrlData->maItemList.size() )
    {
        mpTabCtrlData->maItemList.push_back( ImplTabItem() );
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

    
    if ( !mnCurPageId )
        mnCurPageId = nPageId;

    
    pItem->mnId             = nPageId;
    pItem->mpTabPage        = NULL;
    pItem->maText           = rText;
    pItem->mbFullVisible    = false;

    mbFormat = true;
    if ( IsUpdateMode() )
        Invalidate();

    ImplFreeLayoutData();
    if( mpTabCtrlData->mpListBox ) 
        Resize();

    ImplCallEventListeners( VCLEVENT_TABPAGE_INSERTED, (void*) (sal_uLong)nPageId );
}



void TabControl::RemovePage( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    
    if ( nPos != TAB_PAGE_NOTFOUND )
    {
        
        std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin() + nPos;
        bool bIsCurrentPage = (it->mnId == mnCurPageId);
        mpTabCtrlData->maItemList.erase( it );
        if( mpTabCtrlData->mpListBox )
        {
            mpTabCtrlData->mpListBox->RemoveEntry( nPos );
            mpTabCtrlData->mpListBox->SetDropDownLineCount( mpTabCtrlData->mpListBox->GetEntryCount() );
        }

        
        if ( bIsCurrentPage  )
        {
            mnCurPageId = 0;

            if( ! mpTabCtrlData->maItemList.empty() )
            {
                
                
                
                
                
                
                

                SetCurPageId( mpTabCtrlData->maItemList[0].mnId );
            }
        }

        mbFormat = true;
        if ( IsUpdateMode() )
            Invalidate();

        ImplFreeLayoutData();

        ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVED, (void*) (sal_uLong) nPageId );
    }
}



void TabControl::Clear()
{
    
    mpTabCtrlData->maItemList.clear();
    mnCurPageId = 0;
    if( mpTabCtrlData->mpListBox )
        mpTabCtrlData->mpListBox->Clear();

    ImplFreeLayoutData();

    mbFormat = true;
    if ( IsUpdateMode() )
        Invalidate();

    ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVEDALL );
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
                                                     i_bEnable ? 0 : (LISTBOX_ENTRY_FLAG_DISABLE_SELECTION | LISTBOX_ENTRY_FLAG_DRAW_DISABLED) );
        if( pItem->mnId == mnCurPageId )
        {
             
            SetCurPageId( mnCurPageId );
        }
        else if ( IsUpdateMode() )
            Invalidate();
    }
}



sal_uInt16 TabControl::GetPageCount() const
{
    return (sal_uInt16)mpTabCtrlData->maItemList.size();
}



sal_uInt16 TabControl::GetPageId( sal_uInt16 nPos ) const
{
    if( size_t(nPos) < mpTabCtrlData->maItemList.size() )
        return mpTabCtrlData->maItemList[ nPos ].mnId;
    return 0;
}



sal_uInt16 TabControl::GetPagePos( sal_uInt16 nPageId ) const
{
    for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->mnId == nPageId )
            return (sal_uInt16)(it - mpTabCtrlData->maItemList.begin());
    }

    return TAB_PAGE_NOTFOUND;
}



sal_uInt16 TabControl::GetPageId( const Point& rPos ) const
{
    for( size_t i = 0; i < mpTabCtrlData->maItemList.size(); ++i )
    {
        if ( ((TabControl*)this)->ImplGetTabRect( static_cast<sal_uInt16>(i) ).IsInside( rPos ) )
            return mpTabCtrlData->maItemList[ i ].mnId;
    }

    return 0;
}

sal_uInt16 TabControl::GetPageId( const TabPage& rPage ) const
{
    for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->mpTabPage == &rPage )
            return it->mnId;
    }

    return 0;
}

sal_uInt16 TabControl::GetPageId( const OString& rName ) const
{
    for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->maTabName == rName )
            return it->mnId;
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

        ImplCallEventListeners( VCLEVENT_TABPAGE_DEACTIVATE, (void*) (sal_uLong) mnCurPageId );
        if ( DeactivatePage() )
        {
            mnActPageId = nPageId;
            ActivatePage();
            
            nPageId = mnActPageId;
            mnActPageId = 0;
            SetCurPageId( nPageId );
            if( mpTabCtrlData->mpListBox )
                mpTabCtrlData->mpListBox->SelectEntryPos( GetPagePos( nPageId ) );
            ImplCallEventListeners( VCLEVENT_TABPAGE_ACTIVATE, (void*) (sal_uLong) nPageId );
        }
    }
}



void TabControl::SetTabPage( sal_uInt16 nPageId, TabPage* pTabPage )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && (pItem->mpTabPage != pTabPage) )
    {
        if ( pTabPage )
        {
            DBG_ASSERT( !pTabPage->IsVisible() || isLayoutEnabled(pTabPage),
                "TabControl::SetTabPage() - Non-Layout Enabled Page is visible" );

            if ( IsDefaultSize() )
                SetTabPageSizePixel( pTabPage->GetSizePixel() );

            
            pItem->mpTabPage = pTabPage;
            queue_resize();
            if ( pItem->mnId == mnCurPageId )
                ImplChangeTabPage( pItem->mnId, 0 );
        }
        else
        {
            pItem->mpTabPage = NULL;
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
        return NULL;
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
        ImplCallEventListeners( VCLEVENT_TABPAGE_PAGETEXTCHANGED, (void*) (sal_uLong) nPageId );
    }
}



OUString TabControl::GetPageText( sal_uInt16 nPageId ) const
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



Rectangle TabControl::GetCharacterBounds( sal_uInt16 nPageId, long nIndex ) const
{
    Rectangle aRet;

    if( !HasLayoutData() || ! mpTabCtrlData->maLayoutPageIdToLine.size() )
        FillLayoutData();

    if( HasLayoutData() )
    {
        boost::unordered_map< int, int >::const_iterator it = mpTabCtrlData->maLayoutPageIdToLine.find( (int)nPageId );
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

    if( !HasLayoutData() || ! mpTabCtrlData->maLayoutPageIdToLine.size() )
        FillLayoutData();

    if( HasLayoutData() )
    {
        int nIndex = mpControlData->mpLayoutData->GetIndexForPoint( rPoint );
        if( nIndex != -1 )
        {
            
            int nLines = mpControlData->mpLayoutData->GetLineCount();
            int nLine = -1;
            while( ++nLine < nLines )
            {
                Pair aPair = mpControlData->mpLayoutData->GetLineStartEnd( nLine );
                if( aPair.A() <= nIndex && aPair.B() >= nIndex )
                {
                    nRet = nIndex - aPair.A();
                    rPageId = (sal_uInt16)mpTabCtrlData->maLayoutLineToPageId[ nLine ];
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
    const_cast<TabControl*>(this)->ImplPaint( Rectangle(), true );
}



Rectangle TabControl::GetTabBounds( sal_uInt16 nPageId ) const
{
    Rectangle aRet;

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
    for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        const TabPage *pPage = it->mpTabPage;
        
        
        if (!pPage)
        {
            TabControl *pThis = const_cast<TabControl*>(this);
            pThis->SetCurPageId(it->mnId);
            pThis->ActivatePage();
            pPage = it->mpTabPage;
        }

        if (!pPage)
            continue;

        Size aPageSize(VclContainer::getLayoutRequisition(*pPage));

        if (aPageSize.Width() > aOptimalPageSize.Width())
            aOptimalPageSize.Width() = aPageSize.Width();
        if (aPageSize.Height() > aOptimalPageSize.Height())
            aOptimalPageSize.Height() = aPageSize.Height();
    }

    
    
    
    if (nOrigPageId != GetCurPageId())
    {
        TabControl *pThis = const_cast<TabControl*>(this);
        pThis->SetCurPageId(nOrigPageId);
        pThis->ActivatePage();
    }

    long nTabLabelsBottom = 0, nTabLabelsRight = 0;
    for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        TabControl* pThis = const_cast<TabControl*>(this);

        sal_uInt16 nPos = it - mpTabCtrlData->maItemList.begin();
        Rectangle aTabRect = pThis->ImplGetTabRect(nPos, aOptimalPageSize.Width(), LONG_MAX);
        if (aTabRect.Bottom() > nTabLabelsBottom)
            nTabLabelsBottom = aTabRect.Bottom();
        if (aTabRect.Right() > nTabLabelsRight)
            nTabLabelsRight = aTabRect.Right();
    }

    Size aOptimalSize(aOptimalPageSize);
    aOptimalSize.Height() += nTabLabelsBottom;
    aOptimalSize.Width() = std::max(nTabLabelsRight, aOptimalSize.Width());

    aOptimalSize.Width() += TAB_OFFSET * 2;
    aOptimalSize.Height() += TAB_OFFSET * 2;

    return aOptimalSize;
}

Size TabControl::GetOptimalSize() const
{
    return calculateRequisition();
}

void TabControl::queue_resize()
{
    markLayoutDirty();
    Window::queue_resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
