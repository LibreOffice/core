/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
#include "vcl/lstbox.hxx"

#include "controldata.hxx"
#include "svdata.hxx"
#include "window.h"

#include <boost/unordered_map.hpp>
#include <vector>

// =======================================================================

struct ImplTabItem
{
    sal_uInt16              mnId;
    sal_uInt16              mnTabPageResId;
    TabPage*            mpTabPage;
    String              maText;
    String              maFormatText;
    String              maHelpText;
    rtl::OString        maHelpId;
    Rectangle           maRect;
    sal_uInt16              mnLine;
    bool                mbFullVisible;
    bool                mbEnabled;
    Image               maTabImage;

    ImplTabItem()
    : mnId( 0 ), mnTabPageResId( 0 ), mpTabPage( NULL ),
      mnLine( 0 ), mbFullVisible( sal_False ), mbEnabled( true )
    {}
};

// -----------------------------------------------------------------------

struct ImplTabCtrlData
{
    boost::unordered_map< int, int >        maLayoutPageIdToLine;
    boost::unordered_map< int, int >        maLayoutLineToPageId;
    std::vector< Rectangle >        maTabRectangles;
    Point                           maItemsOffset;       // offset of the tabitems
    std::vector< ImplTabItem >      maItemList;
    ListBox*                        mpListBox;
    Size                            maMinSize;
};

// -----------------------------------------------------------------------

#define TAB_OFFSET          3
#define TAB_TABOFFSET_X     3
#define TAB_TABOFFSET_Y     3
#define TAB_EXTRASPACE_X    6
#define TAB_BORDER_LEFT     1
#define TAB_BORDER_TOP      1
#define TAB_BORDER_RIGHT    2
#define TAB_BORDER_BOTTOM   2

// Fuer die Ermittlung von den Tab-Positionen
#define TAB_PAGERECT        0xFFFF

// =======================================================================

void TabControl::ImplInit( Window* pParent, WinBits nStyle )
{
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
    mbFormat                    = sal_True;
    mbRestoreHelpId             = sal_False;
    mbRestoreUnqId              = sal_False;
    mbSmallInvalidate           = sal_False;
    mpTabCtrlData               = new ImplTabCtrlData;
    mpTabCtrlData->mpListBox    = NULL;


    ImplInitSettings( sal_True, sal_True, sal_True );

    if( (nStyle & WB_DROPDOWN) )
    {
        mpTabCtrlData->mpListBox = new ListBox( this, WB_DROPDOWN );
        mpTabCtrlData->mpListBox->SetPosSizePixel( Point( 0, 0 ), Size( 200, 20 ) );
        mpTabCtrlData->mpListBox->SetSelectHdl( LINK( this, TabControl, ImplListBoxSelectHdl ) );
        mpTabCtrlData->mpListBox->Show();
    }

    // if the tabcontrol is drawn (ie filled) by a native widget, make sure all contols will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(CTRL_TAB_PANE, PART_ENTIRE_CONTROL) )
        EnableChildTransparentMode( sal_True );

    if ( pParent->IsDialog() )
        pParent->AddChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );
}

// -----------------------------------------------------------------

const Font& TabControl::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetAppFont();
}

// -----------------------------------------------------------------
const Color& TabControl::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetButtonTextColor();
}

// -----------------------------------------------------------------------

void TabControl::ImplInitSettings( sal_Bool bFont,
                                   sal_Bool bForeground, sal_Bool bBackground )
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
            // set transparent mode for NWF tabcontrols to have
            // the background always cleared properly
            EnableChildTransparentMode( sal_True );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( sal_True );
            SetBackground();
            ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( sal_False );
            SetParentClipMode( 0 );
            SetPaintTransparent( sal_False );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

// -----------------------------------------------------------------------

void TabControl::ImplFreeLayoutData()
{
    if( HasLayoutData() )
    {
        ImplClearLayoutData();
        mpTabCtrlData->maLayoutPageIdToLine.clear();
        mpTabCtrlData->maLayoutLineToPageId.clear();
    }
}

// -----------------------------------------------------------------------

TabControl::TabControl( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_TABCONTROL )
{
    ImplInit( pParent, nStyle );
    OSL_TRACE("*** TABCONTROL no notabs? %s", ( GetStyle() & WB_NOBORDER ) ? "true" : "false" );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void TabControl::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uLong nObjMask = ReadLongRes();

    if ( nObjMask & RSC_TABCONTROL_ITEMLIST )
    {
        sal_uLong nEle = ReadLongRes();

        // Item hinzufuegen
        for( sal_uLong i = 0; i < nEle; i++ )
        {
            InsertPage( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }
}

// -----------------------------------------------------------------------

TabControl::~TabControl()
{
    if ( GetParent()->IsDialog() )
        GetParent()->RemoveChildEventListener( LINK( this, TabControl, ImplWindowEventListener ) );

    ImplFreeLayoutData();

    // TabCtrl-Daten loeschen
    if ( mpTabCtrlData )
    {
        if( mpTabCtrlData->mpListBox )
            delete mpTabCtrlData->mpListBox;
        delete mpTabCtrlData;
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

Size TabControl::ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth )
{
    pItem->maFormatText = pItem->maText;
    Size aSize( GetCtrlTextWidth( pItem->maFormatText ), GetTextHeight() );
    Size aImageSize( 0, 0 );
    if( !!pItem->maTabImage )
    {
        aImageSize = pItem->maTabImage.GetSizePixel();
        if( pItem->maFormatText.Len() )
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
                                           CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                           aBoundingRgn, aContentRgn ) )
    {
        return aContentRgn.GetSize();
    }

    // For languages with short names (e.g. Chinese), because the space is
    // normally only one pixel per char
    if ( pItem->maFormatText.Len() < TAB_EXTRASPACE_X )
        aSize.Width() += TAB_EXTRASPACE_X-pItem->maFormatText.Len();

    // Evt. den Text kuerzen
    if ( aSize.Width()+4 >= nMaxWidth )
    {
        rtl::OUString aAppendStr("...");
        pItem->maFormatText += aAppendStr;
        do
        {
            pItem->maFormatText.Erase( pItem->maFormatText.Len()-aAppendStr.getLength()-1, 1 );
            aSize.Width() = GetCtrlTextWidth( pItem->maFormatText );
            aSize.Width() += aImageSize.Width();
            aSize.Width() += TAB_TABOFFSET_X*2;
        }
        while ( (aSize.Width()+4 >= nMaxWidth) && (pItem->maFormatText.Len() > aAppendStr.getLength()) );
        if ( aSize.Width()+4 >= nMaxWidth )
        {
            pItem->maFormatText.Assign( '.' );
            aSize.Width() = 1;
        }
    }

    if( pItem->maFormatText.Len() == 0 )
    {
        if( aSize.Height() < aImageSize.Height()+4 ) //leave space for focus rect
            aSize.Height() = aImageSize.Height()+4;
    }

    return aSize;
}

// -----------------------------------------------------------------------

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
        aFont.SetTransparent( sal_True );
        SetFont( aFont );

        Size            aSize;
        const long      nOffsetX = 2 + GetItemsOffset().X();
        const long      nOffsetY = 2 + GetItemsOffset().Y();
        long            nX = nOffsetX;
        long            nY = nOffsetY;
        long            nMaxWidth = nWidth;
        sal_uInt16          nPos = 0;

        if ( (mnMaxPageWidth > 0) && (mnMaxPageWidth < nMaxWidth) )
            nMaxWidth = mnMaxPageWidth;
        nMaxWidth -= GetItemsOffset().X();

        sal_uInt16          nLines = 0;
        sal_uInt16          nCurLine = 0;
        long            nLineWidthAry[100];
        sal_uInt16          nLinePosAry[101];

        nLineWidthAry[0] = 0;
        nLinePosAry[0] = 0;
        for( std::vector<ImplTabItem>::iterator it = mpTabCtrlData->maItemList.begin();
             it != mpTabCtrlData->maItemList.end(); ++it )
        {
            aSize = ImplGetItemSize( &(*it), nMaxWidth );

            if ( ((nX+aSize.Width()) > nWidth - 2) && (nWidth > 2+nOffsetX) )
            {
                if ( nLines == 99 )
                    break;

                nX  = nOffsetX;
                nY += aSize.Height();
                nLines++;
                nLineWidthAry[nLines] = 0;
                nLinePosAry[nLines] = nPos;
            }

            Rectangle aNewRect( Point( nX, nY ), aSize );
            if ( mbSmallInvalidate && (it->maRect != aNewRect) )
                mbSmallInvalidate = sal_False;
            it->maRect = aNewRect;
            it->mnLine = nLines;
            it->mbFullVisible = sal_True;

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
                        // FIXME: this is a bad case of tabctrl way too small
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
        {//only one line
            if(ImplGetSVData()->maNWFData.mbCenteredTabs)
            {
                int nRightSpace=nMaxWidth;//space left on the right by the tabs
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
        mbFormat        = sal_False;
    }

    return size_t(nItemPos) < mpTabCtrlData->maItemList.size() ? mpTabCtrlData->maItemList[nItemPos].maRect : Rectangle();
}

// -----------------------------------------------------------------------

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
            pCtrlParent->SetHelpId( rtl::OString() );
        if ( mbRestoreUnqId )
            pCtrlParent->SetUniqueId( rtl::OString() );
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

        // activate page here so the conbtrols can be switched
        // also set the help id of the parent window to that of the tab page
        if ( GetHelpId().isEmpty() )
        {
            mbRestoreHelpId = sal_True;
            pCtrlParent->SetHelpId( pPage->GetHelpId() );
        }
        if ( pCtrlParent->GetUniqueId().isEmpty() )
        {
            mbRestoreUnqId = sal_True;
            pCtrlParent->SetUniqueId( pPage->GetUniqueId() );
        }

        pPage->ActivatePage();

        if ( pOldPage && pOldPage->HasChildPathFocus() )
        {
            sal_uInt16  n = 0;
            Window* pFirstChild = pPage->ImplGetDlgWindow( n, DLGWINDOW_FIRST );
            if ( pFirstChild )
                pFirstChild->ImplControlFocus( GETFOCUS_INIT );
            else
                GrabFocus();
        }

        pPage->Show();
    }

    if ( pOldPage )
        pOldPage->Hide();

    // Invalidate the same region that will be send to NWF
    // to always allow for bitmap caching
    // see Window::DrawNativeControl()
    if( IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL ) )
    {
        aRect.Left()   -= TAB_OFFSET;
        aRect.Top()    -= TAB_OFFSET;
        aRect.Right()  += TAB_OFFSET;
        aRect.Bottom() += TAB_OFFSET;
    }

    Invalidate( aRect );
}

// -----------------------------------------------------------------------

sal_Bool TabControl::ImplPosCurTabPage()
{
    // Aktuelle TabPage resizen/positionieren
    ImplTabItem* pItem = ImplGetItem( GetCurPageId() );
    if ( pItem && pItem->mpTabPage )
    {
        if (  ( GetStyle() & WB_NOBORDER ) )
        {
            Rectangle aRectNoTab( (const Point&)Point( 0, 0 ), GetSizePixel() );
            pItem->mpTabPage->SetPosSizePixel( aRectNoTab.TopLeft(), aRectNoTab.GetSize() );
            return sal_True;
        }
        Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        pItem->mpTabPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void TabControl::ImplActivateTabPage( sal_Bool bNext )
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

// -----------------------------------------------------------------------

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
        if( rItem.maFormatText.Len() )
            aImageSize.Width() += GetTextHeight()/4;
    }

    if( rItem.maFormatText.Len() )
    {
        // show focus around text
        aRect.Left()   = aRect.Left()+aImageSize.Width()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-1-1;
        aRect.Top()    = aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-1-1;
        aRect.Right()  = aRect.Left()+nTextWidth+2;
        aRect.Bottom() = aRect.Top()+nTextHeight+2;
    }
    else
    {
        // show focus around image
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

// -----------------------------------------------------------------------

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
    sal_Bool                    bLeftBorder = sal_True;
    sal_Bool                    bRightBorder = sal_True;
    sal_uInt16                  nOff;
    sal_Bool                    bNativeOK = sal_False;

    sal_uInt16 nOff2 = 0;
    sal_uInt16 nOff3 = 0;

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nOff = 1;
    else
        nOff = 0;

    // Wenn wir die aktuelle Page sind, muessen wir etwas mehr zeichnen
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
                bLeftBorder = sal_False;
            aRightTestPos.X() += 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                bRightBorder = sal_False;
        }
        else
        {
            if ( rCurRect.IsInside( aLeftTestPos ) )
                nLeftBottom -= 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                nRightBottom -= 2;
        }
    }

    if( !bLayout && (bNativeOK = IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL)) == sal_True )
    {
        Rectangle           aCtrlRegion( pItem->maRect );
        ControlState        nState = 0;

        if( pItem->mnId == mnCurPageId )
        {
            nState |= CTRL_STATE_SELECTED;
            // only the selected item can be focussed
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
                    nState &= ~CTRL_STATE_ROLLOVER; // avoid multiple highlighted tabs
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
                    tiValue, rtl::OUString() );
    }

    if( ! bLayout && !bNativeOK )
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawPixel( Point( aRect.Left()+1-nOff2, aRect.Top()+1-nOff2 ) );    // diagonally indented top-left pixel
            if ( bLeftBorder )
            {
                DrawLine( Point( aRect.Left()-nOff2, aRect.Top()+2-nOff2 ),
                          Point( aRect.Left()-nOff2, nLeftBottom-1 ) );
            }
            DrawLine( Point( aRect.Left()+2-nOff2, aRect.Top()-nOff2 ),         // top line starting 2px from left border
                      Point( aRect.Right()+nOff2-3, aRect.Top()-nOff2 ) );      // ending 3px from right border

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
        mpControlData->mpLayoutData->m_aLineIndices.push_back( mpControlData->mpLayoutData->m_aDisplayText.Len() );
        mpTabCtrlData->maLayoutPageIdToLine[ (int)pItem->mnId ] = nLine;
        mpTabCtrlData->maLayoutLineToPageId[ nLine ] = (int)pItem->mnId;
        mpTabCtrlData->maTabRectangles.push_back( aRect );
    }

    // set font accordingly, current item is painted bold
    // we set the font attributes always before drawing to be re-entrant (DrawNativeControl may trigger additional paints)
    Font aFont( GetFont() );
    aFont.SetTransparent( sal_True );
    SetFont( aFont );

    Size aTabSize = aRect.GetSize();
    Size aImageSize( 0, 0 );
    long nTextHeight = GetTextHeight();
    long nTextWidth = GetCtrlTextWidth( pItem->maFormatText );
    if( !! pItem->maTabImage )
    {
        aImageSize = pItem->maTabImage.GetSizePixel();
        if( pItem->maFormatText.Len() )
            aImageSize.Width() += GetTextHeight()/4;
    }
    long nXPos = aRect.Left()+((aTabSize.Width()-nTextWidth-aImageSize.Width())/2)-nOff-nOff3;
    long nYPos = aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-nOff3;
    if( pItem->maFormatText.Len() )
    {
        sal_uInt16 nStyle = TEXT_DRAW_MNEMONIC;
        if( ! pItem->mbEnabled )
            nStyle |= TEXT_DRAW_DISABLE;
        DrawCtrlText( Point( nXPos + aImageSize.Width(), nYPos ),
                      pItem->maFormatText,
                      0, STRING_LEN, nStyle,
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

// -----------------------------------------------------------------------

long TabControl::ImplHandleKeyEvent( const KeyEvent& rKeyEvent )
{
    long nRet = 0;

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
                    ImplActivateTabPage( sal_False );
                    nRet = 1;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    ImplActivateTabPage( sal_True );
                    nRet = 1;
                }
            }
        }
    }

    return nRet;
}


// -----------------------------------------------------------------------

IMPL_LINK_NOARG(TabControl, ImplListBoxSelectHdl)
{
    SelectTabPage( GetPageId( mpTabCtrlData->mpListBox->GetSelectEntryPos() ) );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( TabControl, ImplWindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( pEvent && pEvent->ISA( VclWindowEvent ) && (pEvent->GetId() == VCLEVENT_WINDOW_KEYINPUT) )
    {
        VclWindowEvent* pWindowEvent = static_cast< VclWindowEvent* >(pEvent);
        // Do not handle events from TabControl or it's children, which is done in Notify(), where the events can be consumed.
        if ( !IsWindowOrChild( pWindowEvent->GetWindow() ) )
        {
            KeyEvent* pKeyEvent = static_cast< KeyEvent* >(pWindowEvent->GetData());
            ImplHandleKeyEvent( *pKeyEvent );
        }
    }
    return 0;
}


// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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
            sal_Bool bNext = (nKeyCode == KEY_RIGHT);
            ImplActivateTabPage( bNext );
        }
    }

    Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void TabControl::Paint( const Rectangle& rRect )
{
    if (  !( GetStyle() & WB_NOBORDER ) )
        ImplPaint( rRect, false );
}

// -----------------------------------------------------------------------

void TabControl::ImplPaint( const Rectangle& rRect, bool bLayout )
{
    if( ! bLayout )
        HideFocus();

    // Hier wird gegebenenfalls auch neu formatiert
    Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );

    // find current item
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

    // Draw the TabPage border
    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aCurRect;
    aRect.Left()   -= TAB_OFFSET;
    aRect.Top()    -= TAB_OFFSET;
    aRect.Right()  += TAB_OFFSET;
    aRect.Bottom() += TAB_OFFSET;

    // if we have an invisible tabpage or no tabpage at all the tabpage rect should be
    // increased to avoid round corners that might be drawn by a theme
    // in this case we're only interested in the top border of the tabpage because the tabitems are used
    // standalone (eg impress)
    sal_Bool bNoTabPage = sal_False;
    TabPage*        pCurPage = (pCurItem) ? pCurItem->mpTabPage : NULL;
    if( !pCurPage || !pCurPage->IsVisible() )
    {
        bNoTabPage = sal_True;
        aRect.Left()-=10;
        aRect.Right()+=10;
    }

    sal_Bool bNativeOK = sal_False;
    if( ! bLayout && (bNativeOK = IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL) ) == sal_True )
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
                aControlValue, rtl::OUString() );
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
                // if we have not tab page the bottom line of the tab page
                // directly touches the tab items, so choose a color that fits seamlessly
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
        // Some native toolkits (GTK+) draw tabs right-to-left, with an
        // overlap between adjacent tabs
        bool            bDrawTabsRTL = IsNativeControlSupported( CTRL_TAB_ITEM, PART_TABS_DRAW_RTL );
        ImplTabItem *   pFirstTab = NULL;
        ImplTabItem *   pLastTab = NULL;
        size_t idx;

        // Event though there is a tab overlap with GTK+, the first tab is not
        // overlapped on the left side.  Other tookits ignore this option.
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
                    ImplDrawItem( pItem, aCurRect, bLayout, (pItem==pFirstTab), (pItem==pLastTab), sal_False );
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
                ImplDrawItem( pCurItem, aCurRect, bLayout, (pCurItem==pFirstTab), (pCurItem==pLastTab), sal_True );
        }
    }

    if ( !bLayout && HasFocus() )
        ImplShowFocus();

    if( ! bLayout )
        mbSmallInvalidate = sal_True;
}

// -----------------------------------------------------------------------

void TabControl::Resize()
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
        Size aNewSize( nPrefWidth, LogicToPixel( Size( 12, 12 ), MapMode( MAP_APPFONT ) ).Height() );
        Point aNewPos( (aTabCtrlSize.Width() - nPrefWidth) / 2, 0 );
        mpTabCtrlData->mpListBox->SetPosSizePixel( aNewPos, aNewSize );
    }

    mbFormat = sal_True;

    // Aktuelle TabPage resizen/positionieren
    sal_Bool bTabPage = ImplPosCurTabPage();
    // Feststellen, was invalidiert werden muss
    Size aNewSize = Control::GetOutputSizePixel();
    long nNewWidth = aNewSize.Width();
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( !it->mbFullVisible ||
             (it->maRect.Right()-2 >= nNewWidth) )
        {
            mbSmallInvalidate = sal_False;
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
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void TabControl::LoseFocus()
{
    if( ! mpTabCtrlData->mpListBox )
        HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void TabControl::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = rHEvt.KeyboardActivated() ? mnCurPageId : GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        {
            XubString aStr = GetHelpText( nItemId );
            if ( aStr.Len() )
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
            rtl::OUString aHelpId( rtl::OStringToOUString( GetHelpId( nItemId ), RTL_TEXTENCODING_UTF8 ) );
            if ( !aHelpId.isEmpty() )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( aHelpId, this );
                return;
            }
        }

        // Bei Quick- oder Balloon-Help zeigen wir den Text an,
        // wenn dieser abgeschnitten ist
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            ImplTabItem* pItem = ImplGetItem( nItemId );
            const XubString& rStr = pItem->maText;
            if ( rStr != pItem->maFormatText )
            {
                Rectangle aItemRect = ImplGetTabRect( GetPagePos( nItemId ) );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                if ( rStr.Len() )
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
            const XubString& rHelpText = pItem->maHelpText;
            // show tooltip if not text but image is set and helptext is available
            if ( rHelpText.Len() > 0 && pItem->maText.Len() == 0 && !!pItem->maTabImage )
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

// -----------------------------------------------------------------------

void TabControl::Command( const CommandEvent& rCEvt )
{
    if( (mpTabCtrlData->mpListBox == NULL) && (rCEvt.GetCommand() == COMMAND_CONTEXTMENU) && (GetPageCount() > 1) )
    {
        Point   aMenuPos;
        sal_Bool    bMenu;
        if ( rCEvt.IsMouseEvent() )
        {
            aMenuPos = rCEvt.GetMousePosPixel();
            bMenu = GetPageId( aMenuPos ) != 0;
        }
        else
        {
            aMenuPos = ImplGetTabRect( GetPagePos( mnCurPageId ) ).Center();
            bMenu = sal_True;
        }

        if ( bMenu )
        {
            PopupMenu aMenu;
            for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
                 it != mpTabCtrlData->maItemList.end(); ++it )
            {
                aMenu.InsertItem( it->mnId, it->maText, MIB_CHECKABLE | MIB_RADIOCHECK );
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

// -----------------------------------------------------------------------

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
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void TabControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Rectangle* TabControl::ImplFindPartRect( const Point& rPt )
{
    ImplTabItem* pFoundItem = NULL;
    int nFound = 0;
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if ( it->maRect.IsInside( rPt ) )
        {
            // assure that only one tab is highlighted at a time
            nFound++;
            pFoundItem = &(*it);
        }
    }
    // assure that only one tab is highlighted at a time
    return nFound == 1 ? &pFoundItem->maRect : NULL;
}

long TabControl::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
                    Region aClipRgn;
                    if( pLastRect )
                    {
                        // allow for slightly bigger tabitems
                        // as used by gtk
                        // TODO: query for the correct sizes
                        Rectangle aRect(*pLastRect);
                        aRect.nLeft-=2;
                        aRect.nRight+=2;
                        aRect.nTop-=3;
                        aClipRgn.Union( aRect );
                    }
                    if( pRect )
                    {
                        // allow for slightly bigger tabitems
                        // as used by gtk
                        // TODO: query for the correct sizes
                        Rectangle aRect(*pRect);
                        aRect.nLeft-=2;
                        aRect.nRight+=2;
                        aRect.nTop-=3;
                        aClipRgn.Union( aRect );
                    }
                    if( !aClipRgn.IsEmpty() )
                        Invalidate( aClipRgn );
                }
            }
        }
    }

    return nDone ? nDone : Control::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

long TabControl::Notify( NotifyEvent& rNEvt )
{
    long nRet = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
        nRet = ImplHandleKeyEvent( *rNEvt.GetKeyEvent() );

    return nRet ? nRet : Control::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void TabControl::ActivatePage()
{
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

long TabControl::DeactivatePage()
{
    if ( maDeactivateHdl.IsSet() )
        return maDeactivateHdl.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

Size TabControl::GetTabPageSizePixel() const
{
    Rectangle aRect = ((TabControl*)this)->ImplGetTabRect( TAB_PAGERECT );
    return aRect.GetSize();
}

// -----------------------------------------------------------------------

void TabControl::InsertPage( const ResId& rResId, sal_uInt16 nPos )
{
    GetRes( rResId.SetRT( RSC_TABCONTROLITEM ) );

    sal_uLong nObjMask = ReadLongRes();
    sal_uInt16 nItemId  = 1;

    // ID
    if ( nObjMask & RSC_TABCONTROLITEM_ID )
        nItemId = sal::static_int_cast<sal_uInt16>(ReadLongRes());

    // Text
    XubString aTmpStr;
    if( nObjMask & RSC_TABCONTROLITEM_TEXT )
        aTmpStr = ReadStringRes();
    InsertPage( nItemId, aTmpStr, nPos );

    // PageResID
    if ( nObjMask & RSC_TABCONTROLITEM_PAGERESID )
    {
        ImplTabItem& rItem = mpTabCtrlData->maItemList[ GetPagePos( nItemId ) ];
        rItem.mnTabPageResId = sal::static_int_cast<sal_uInt16>(ReadLongRes());
    }
}

// -----------------------------------------------------------------------

void TabControl::InsertPage( sal_uInt16 nPageId, const XubString& rText,
                             sal_uInt16 nPos )
{
    DBG_ASSERT( nPageId, "TabControl::InsertPage(): PageId == 0" );
    DBG_ASSERT( GetPagePos( nPageId ) == TAB_PAGE_NOTFOUND,
                "TabControl::InsertPage(): PageId already exists" );

    // insert new page item
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

    // set current page id
    if ( !mnCurPageId )
        mnCurPageId = nPageId;

    // init new page item
    pItem->mnId             = nPageId;
    pItem->mpTabPage        = NULL;
    pItem->mnTabPageResId   = 0;
    pItem->maText           = rText;
    pItem->mbFullVisible    = sal_False;

    mbFormat = sal_True;
    if ( IsUpdateMode() )
        Invalidate();

    ImplFreeLayoutData();
    if( mpTabCtrlData->mpListBox ) // reposition/resize listbox
        Resize();

    ImplCallEventListeners( VCLEVENT_TABPAGE_INSERTED, (void*) (sal_uLong)nPageId );
}

// -----------------------------------------------------------------------

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

        // If current page is removed, than first page gets the current page
        if ( bIsCurrentPage  )
        {
            mnCurPageId = 0;

            if( ! mpTabCtrlData->maItemList.empty() )
            {
                // don't do this by simply setting mnCurPageId to pFirstItem->mnId
                // this leaves a lot of stuff (such trivias as _showing_ the new current page) undone
                // instead, call SetCurPageId
                // without this, the next (outside) call to SetCurPageId with the id of the first page
                // will result in doing nothing (as we assume that nothing changed, then), and the page
                // will never be shown.
                // 86875 - 05/11/2001 - frank.schoenheit@germany.sun.com

                SetCurPageId( mpTabCtrlData->maItemList[0].mnId );
            }
        }

        mbFormat = sal_True;
        if ( IsUpdateMode() )
            Invalidate();

        ImplFreeLayoutData();

        ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVED, (void*) (sal_uLong) nPageId );
    }
}

// -----------------------------------------------------------------------

void TabControl::Clear()
{
    // clear item list
    mpTabCtrlData->maItemList.clear();
    mnCurPageId = 0;
    if( mpTabCtrlData->mpListBox )
        mpTabCtrlData->mpListBox->Clear();

    ImplFreeLayoutData();

    mbFormat = sal_True;
    if ( IsUpdateMode() )
        Invalidate();

    ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVEDALL );
}

// -----------------------------------------------------------------------

void TabControl::EnablePage( sal_uInt16 i_nPageId, bool i_bEnable )
{
    ImplTabItem* pItem = ImplGetItem( i_nPageId );

    if ( pItem && pItem->mbEnabled != i_bEnable )
    {
        pItem->mbEnabled = i_bEnable;
        mbFormat = sal_True;
        if( mpTabCtrlData->mpListBox )
            mpTabCtrlData->mpListBox->SetEntryFlags( GetPagePos( i_nPageId ),
                                                     i_bEnable ? 0 : (LISTBOX_ENTRY_FLAG_DISABLE_SELECTION | LISTBOX_ENTRY_FLAG_DRAW_DISABLED) );
        if( pItem->mnId == mnCurPageId )
        {
             // SetCurPageId will change to an enabled page
            SetCurPageId( mnCurPageId );
        }
        else if ( IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 TabControl::GetPageCount() const
{
    return (sal_uInt16)mpTabCtrlData->maItemList.size();
}

// -----------------------------------------------------------------------

sal_uInt16 TabControl::GetPageId( sal_uInt16 nPos ) const
{
    if( size_t(nPos) < mpTabCtrlData->maItemList.size() )
        return mpTabCtrlData->maItemList[ nPos ].mnId;
    return 0;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

sal_uInt16 TabControl::GetPageId( const Point& rPos ) const
{
    for( size_t i = 0; i < mpTabCtrlData->maItemList.size(); ++i )
    {
        if ( ((TabControl*)this)->ImplGetTabRect( static_cast<sal_uInt16>(i) ).IsInside( rPos ) )
            return mpTabCtrlData->maItemList[ i ].mnId;
    }

    return 0;
}

// -----------------------------------------------------------------------

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
            mbFormat = sal_True;
            sal_uInt16 nOldId = mnCurPageId;
            mnCurPageId = nPageId;
            ImplChangeTabPage( nPageId, nOldId );
        }
    }
}

// -----------------------------------------------------------------------

sal_uInt16 TabControl::GetCurPageId() const
{
    if ( mnActPageId )
        return mnActPageId;
    else
        return mnCurPageId;
}

// -----------------------------------------------------------------------

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
            // Page koennte im Activate-Handler umgeschaltet wurden sein
            nPageId = mnActPageId;
            mnActPageId = 0;
            SetCurPageId( nPageId );
            if( mpTabCtrlData->mpListBox )
                mpTabCtrlData->mpListBox->SelectEntryPos( GetPagePos( nPageId ) );
            ImplCallEventListeners( VCLEVENT_TABPAGE_ACTIVATE, (void*) (sal_uLong) nPageId );
        }
    }
}

// -----------------------------------------------------------------------

void TabControl::SetTabPage( sal_uInt16 nPageId, TabPage* pTabPage )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && (pItem->mpTabPage != pTabPage) )
    {
        if ( pTabPage )
        {
            DBG_ASSERT( !pTabPage->IsVisible(), "TabControl::SetTabPage() - Page is visible" );

            if ( IsDefaultSize() )
                SetTabPageSizePixel( pTabPage->GetSizePixel() );

            // Erst hier setzen, damit Resize nicht TabPage umpositioniert
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

// -----------------------------------------------------------------------

TabPage* TabControl::GetTabPage( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->mpTabPage;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void TabControl::SetPageText( sal_uInt16 nPageId, const XubString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && pItem->maText != rText )
    {
        pItem->maText = rText;
        mbFormat = sal_True;
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

// -----------------------------------------------------------------------

XubString TabControl::GetPageText( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->maText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void TabControl::SetHelpText( sal_uInt16 nPageId, const XubString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        pItem->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& TabControl::GetHelpText( sal_uInt16 nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
    {
        if ( !pItem->maHelpText.Len() && !pItem->maHelpId.isEmpty() )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( rtl::OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }

        return pItem->maHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

rtl::OString TabControl::GetHelpId( sal_uInt16 nPageId ) const
{
    rtl::OString aRet;
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        aRet = pItem->maHelpId;

    return aRet;
}

// -----------------------------------------------------------------------

void TabControl::SetPageImage( sal_uInt16 i_nPageId, const Image& i_rImage )
{
    ImplTabItem* pItem = ImplGetItem( i_nPageId );

    if ( pItem )
    {
        pItem->maTabImage = i_rImage;
        mbFormat = sal_True;
        if ( IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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
            // what line (->pageid) is this index in ?
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

// -----------------------------------------------------------------------

void TabControl::FillLayoutData() const
{
    mpTabCtrlData->maLayoutLineToPageId.clear();
    mpTabCtrlData->maLayoutPageIdToLine.clear();
    const_cast<TabControl*>(this)->ImplPaint( Rectangle(), true );
}

// -----------------------------------------------------------------------

Rectangle TabControl::GetTabBounds( sal_uInt16 nPageId ) const
{
    Rectangle aRet;

    ImplTabItem* pItem = ImplGetItem( nPageId );
    if(pItem)
        aRet = pItem->maRect;

    return aRet;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

Size TabControl::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return mpTabCtrlData ? mpTabCtrlData->maMinSize : Size();

    default:
    {
        Size aOptimalPageSize(0, 0);
        long nTabLabelsBottom = 0;
        long nTotalTabLabelWidths = 0;

        for( std::vector< ImplTabItem >::const_iterator it = mpTabCtrlData->maItemList.begin();
             it != mpTabCtrlData->maItemList.end(); ++it )
        {
            Size aPageSize;
            const TabPage *pPage = it->mpTabPage;
            if (pPage)
                aPageSize = pPage->GetOptimalSize(eType);
            else
                fprintf(stderr, "nuisance, page not inserted yet :-(\n");

            if (aPageSize.Width() > aOptimalPageSize.Width())
                aOptimalPageSize.Width() = aPageSize.Width();
            if (aPageSize.Height() > aOptimalPageSize.Height())
                aOptimalPageSize.Height() = aPageSize.Height();

            sal_uInt16 nPos = it - mpTabCtrlData->maItemList.begin();
            TabControl* pThis = const_cast<TabControl*>(this);
            Rectangle aTabRect = pThis->ImplGetTabRect(nPos, aPageSize.Width(), aPageSize.Height());
            if (aTabRect.Bottom() > nTabLabelsBottom)
                nTabLabelsBottom = aTabRect.Bottom();
            nTotalTabLabelWidths += aTabRect.GetWidth();
        }

        Size aOptimalSize(aOptimalPageSize);
        aOptimalSize.Height() += nTabLabelsBottom;

        if (nTotalTabLabelWidths > aOptimalSize.Width())
            aOptimalSize.Width() = nTotalTabLabelWidths;

        aOptimalSize.Width() += TAB_OFFSET * 2;
        aOptimalSize.Height() += TAB_OFFSET * 2;

        return aOptimalSize;
    }
    }
}

// -----------------------------------------------------------------------

void TabControl::SetMinimumSizePixel( const Size& i_rSize )
{
    if( mpTabCtrlData )
        mpTabCtrlData->maMinSize = i_rSize;
}

void TabControl::ReassignPageId(sal_uInt16 nOldId, sal_uInt16 nNewId)
{
    for( std::vector< ImplTabItem >::iterator it = mpTabCtrlData->maItemList.begin();
         it != mpTabCtrlData->maItemList.end(); ++it )
    {
        if( it->mnId == nOldId )
            it->mnId = nNewId;
    }

    if (mnActPageId == nOldId)
        mnActPageId = nNewId;

    if (mnCurPageId == nOldId)
        mnCurPageId = nOldId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
