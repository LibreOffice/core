/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tabctrl.cxx,v $
 * $Revision: 1.36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/svdata.hxx>
#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#include <vcl/help.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/controllayout.hxx>

#include <vcl/window.h>

#include <hash_map>
#include <vector>

// =======================================================================

struct ImplTabCtrlData
{
    PushButton*                     mpLeftBtn;
    PushButton*                     mpRightBtn;
    std::hash_map< int, int >       maLayoutPageIdToLine;
    std::hash_map< int, int >       maLayoutLineToPageId;
    std::vector< Rectangle >        maTabRectangles;
    Point                           maItemsOffset;       // offset of the tabitems
};

// -----------------------------------------------------------------------

struct ImplTabItem
{
    USHORT              mnId;
    USHORT              mnTabPageResId;
    TabPage*            mpTabPage;
    String              maText;
    String              maFormatText;
    String              maHelpText;
    ULONG               mnHelpId;
    Rectangle           maRect;
    USHORT              mnLine;
    BOOL                mbFullVisible;
};

DECLARE_LIST( ImplTabItemList, ImplTabItem* )

// -----------------------------------------------------------------------

#if 0
// not used
#define TABCOLORCOUNT   10

static ColorData aImplTabColorAry[TABCOLORCOUNT] =
{
    RGB_COLORDATA(  80, 216, 248 ),
    RGB_COLORDATA( 128, 216, 168 ),
    RGB_COLORDATA( 128, 144, 248 ),
    RGB_COLORDATA( 208, 180, 168 ),
    RGB_COLORDATA( 248, 252, 168 ),
    RGB_COLORDATA( 168, 144, 168 ),
    RGB_COLORDATA( 248, 144,  80 ),
    RGB_COLORDATA( 248, 216,  80 ),
    RGB_COLORDATA( 248, 180, 168 ),
    RGB_COLORDATA( 248, 216, 168 )
};
#endif

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

    // no single line tabs since NWF
    nStyle &= ~WB_SINGLELINE;

    Control::ImplInit( pParent, nStyle, NULL );

    mpItemList                  = new ImplTabItemList( 8, 8 );
    mnLastWidth                 = 0;
    mnLastHeight                = 0;
    mnBtnSize                   = 0;
    mnMaxPageWidth              = 0;
    mnActPageId                 = 0;
    mnCurPageId                 = 0;
    mnFirstPagePos              = 0;
    mnLastFirstPagePos          = 0;
    mbFormat                    = TRUE;
    mbRestoreHelpId             = FALSE;
    mbRestoreUnqId              = FALSE;
    mbSingleLine                = FALSE;
    mbScroll                    = FALSE;
    mbColored                   = FALSE;
    mbSmallInvalidate           = FALSE;
    mbExtraSpace                = FALSE;
    mpTabCtrlData               = new ImplTabCtrlData;
    mpTabCtrlData->mpLeftBtn    = NULL;
    mpTabCtrlData->mpRightBtn   = NULL;


    ImplInitSettings( TRUE, TRUE, TRUE );

    // if the tabcontrol is drawn (ie filled) by a native widget, make sure all contols will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(CTRL_TAB_PANE, PART_ENTIRE_CONTROL) )
        EnableChildTransparentMode( TRUE );
}

// -----------------------------------------------------------------------

void TabControl::ImplInitSettings( BOOL bFont,
                                   BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
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
        Window* pParent = GetParent();
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled()
            || IsNativeControlSupported(CTRL_TAB_PANE, PART_ENTIRE_CONTROL)
            || IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL) ) )

        {
            // set transparent mode for NWF tabcontrols to have
            // the background always cleared properly
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
            ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( FALSE );
            SetParentClipMode( 0 );
            SetPaintTransparent( FALSE );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }

    ImplScrollBtnsColor();
}

// -----------------------------------------------------------------------

void TabControl::ImplFreeLayoutData()
{
    if( mpLayoutData )
    {
        delete mpLayoutData, mpLayoutData = NULL;
        mpTabCtrlData->maLayoutPageIdToLine.clear();
        mpTabCtrlData->maLayoutLineToPageId.clear();
    }
}

// -----------------------------------------------------------------------

TabControl::TabControl( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_TABCONTROL )
{
    ImplInit( pParent, nStyle );
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

    ULONG nObjMask = ReadLongRes();

    if ( nObjMask & RSC_TABCONTROL_ITEMLIST )
    {
        ULONG nEle = ReadLongRes();

        // Item hinzufuegen
        for( ULONG i = 0; i < nEle; i++ )
        {
            InsertPage( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }
}

// -----------------------------------------------------------------------

TabControl::~TabControl()
{
    ImplFreeLayoutData();

    // Alle Items loeschen
    ImplTabItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Itemlist loeschen
    delete mpItemList;

    // TabCtrl-Daten loeschen
    if ( mpTabCtrlData )
    {
        if ( mpTabCtrlData->mpLeftBtn )
            delete mpTabCtrlData->mpLeftBtn;
        if ( mpTabCtrlData->mpRightBtn )
            delete mpTabCtrlData->mpRightBtn;
        delete mpTabCtrlData;
    }
}

// -----------------------------------------------------------------------

ImplTabItem* TabControl::ImplGetItem( USHORT nId ) const
{
    ImplTabItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nId )
            return pItem;

        pItem = mpItemList->Next();
    }

    return NULL;
}

// -----------------------------------------------------------------------

void TabControl::ImplScrollBtnsColor()
{
    if ( mpTabCtrlData && mpTabCtrlData->mpLeftBtn )
    {
        mpTabCtrlData->mpLeftBtn->SetControlForeground();
        mpTabCtrlData->mpRightBtn->SetControlForeground();
    }
}

// -----------------------------------------------------------------------

void TabControl::ImplSetScrollBtnsState()
{
    if ( mbScroll )
    {
        mpTabCtrlData->mpLeftBtn->Enable( mnFirstPagePos != 0 );
        mpTabCtrlData->mpRightBtn->Enable( mnFirstPagePos < mnLastFirstPagePos );
    }
}

// -----------------------------------------------------------------------

void TabControl::ImplPosScrollBtns()
{
    if ( mbScroll )
    {
        if ( !mpTabCtrlData->mpLeftBtn )
        {
            mpTabCtrlData->mpLeftBtn = new PushButton( this, WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS | WB_REPEAT );
            mpTabCtrlData->mpLeftBtn->SetSymbol( SYMBOL_PREV );
            mpTabCtrlData->mpLeftBtn->SetClickHdl( LINK( this, TabControl, ImplScrollBtnHdl ) );
        }
        if ( !mpTabCtrlData->mpRightBtn )
        {
            mpTabCtrlData->mpRightBtn = new PushButton( this, WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS | WB_REPEAT );
            mpTabCtrlData->mpRightBtn->SetSymbol( SYMBOL_NEXT );
            mpTabCtrlData->mpRightBtn->SetClickHdl( LINK( this, TabControl, ImplScrollBtnHdl ) );
        }

        Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        aRect.Left()   -= TAB_OFFSET;
        aRect.Top()    -= TAB_OFFSET;
        aRect.Right()  += TAB_OFFSET;
        aRect.Bottom() += TAB_OFFSET;
        long nX = aRect.Right()-mnBtnSize+1;
        long nY = aRect.Top()-mnBtnSize;
        mpTabCtrlData->mpRightBtn->SetPosSizePixel( nX, nY, mnBtnSize, mnBtnSize );
        nX -= mnBtnSize;
        mpTabCtrlData->mpLeftBtn->SetPosSizePixel( nX, nY, mnBtnSize, mnBtnSize );
        ImplScrollBtnsColor();
        ImplSetScrollBtnsState();
        mpTabCtrlData->mpLeftBtn->Show();
        mpTabCtrlData->mpRightBtn->Show();
    }
    else
    {
        if ( mpTabCtrlData )
        {
            if ( mpTabCtrlData->mpLeftBtn )
                mpTabCtrlData->mpLeftBtn->Hide();
            if ( mpTabCtrlData->mpRightBtn )
                mpTabCtrlData->mpRightBtn->Hide();
        }
    }
}

// -----------------------------------------------------------------------

Size TabControl::ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth )
{
    pItem->maFormatText = pItem->maText;
    Size aSize( GetCtrlTextWidth( pItem->maFormatText ), GetTextHeight() );

    Region aCtrlRegion(  Rectangle( (const Point&)Point( 0, 0 ), aSize ) );
    Region aBoundingRgn, aContentRgn;
    const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );
    if(GetNativeControlRegion( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion,
                                           CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                           aBoundingRgn, aContentRgn ) )
    {
            Rectangle aCont(aContentRgn.GetBoundRect());
            return aCont.GetSize();
    }

    aSize.Width()  += TAB_TABOFFSET_X*2;
    aSize.Height() += TAB_TABOFFSET_Y*2;
    // For systems without synthetic bold support
    if ( mbExtraSpace )
        aSize.Width() += TAB_EXTRASPACE_X;
    // For languages with short names (e.g. Chinese), because the space is
    // normally only one pixel per char
    else if ( pItem->maFormatText.Len() < TAB_EXTRASPACE_X )
        aSize.Width() += TAB_EXTRASPACE_X-pItem->maFormatText.Len();

    // Evt. den Text kuerzen
    if ( aSize.Width()+4 >= nMaxWidth )
    {
        XubString aAppendStr( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
        pItem->maFormatText += aAppendStr;
        do
        {
            pItem->maFormatText.Erase( pItem->maFormatText.Len()-aAppendStr.Len()-1, 1 );
            aSize.Width() = GetCtrlTextWidth( pItem->maFormatText );
            aSize.Width() += TAB_TABOFFSET_X*2;
        }
        while ( (aSize.Width()+4 >= nMaxWidth) && (pItem->maFormatText.Len() > aAppendStr.Len()) );
        if ( aSize.Width()+4 >= nMaxWidth )
        {
            pItem->maFormatText.Assign( '.' );
            aSize.Width() = 1;
        }
    }

    return aSize;
}

// -----------------------------------------------------------------------

Rectangle TabControl::ImplGetTabRect( USHORT nItemPos, long nWidth, long nHeight )
{
    Size aWinSize = Control::GetOutputSizePixel();
    if ( nWidth == -1 )
        nWidth = aWinSize.Width();
    if ( nHeight == -1 )
        nHeight = aWinSize.Height();

    if ( !mpItemList->Count() )
    {
        return Rectangle( Point( TAB_OFFSET, TAB_OFFSET ),
                          Size( nWidth-TAB_OFFSET*2, nHeight-TAB_OFFSET*2 ) );
    }

    if ( nItemPos == TAB_PAGERECT )
    {
        USHORT nLastPos;
        if ( mnCurPageId )
            nLastPos = GetPagePos( mnCurPageId );
        else
            nLastPos = 0;

        Rectangle aRect = ImplGetTabRect( nLastPos, nWidth, nHeight );
        aRect = Rectangle( Point( TAB_OFFSET, aRect.Bottom()+TAB_OFFSET ),
                           Size( nWidth-TAB_OFFSET*2,
                                 nHeight-aRect.Bottom()-TAB_OFFSET*2 ) );
        return aRect;
    }

    nWidth -= 1;

    if ( (nWidth <= 0) || (nHeight <= 0) )
        return Rectangle();

    if ( mbFormat || (mnLastWidth != nWidth) || (mnLastHeight != nHeight) )
    {
        Font aFont( GetFont() );
        Font aLightFont = aFont;
        aFont.SetTransparent( TRUE );
        aFont.SetWeight( (!ImplGetSVData()->maNWFData.mbNoBoldTabFocus) ? WEIGHT_BOLD : WEIGHT_LIGHT );
        aLightFont.SetTransparent( TRUE );
        aLightFont.SetWeight( WEIGHT_LIGHT );

        // If Bold and none Bold strings have the same width, we
        // add in the calcultion extra space, so that the tabs
        // looks better. The could be the case on systems without
        // an bold UI font and without synthetic bold support
        XubString aTestStr( RTL_CONSTASCII_USTRINGPARAM( "Abc." ) );
        SetFont( aLightFont );
        long nTextWidth1 = GetTextWidth( aTestStr );
        SetFont( aFont );
        long nTextWidth2 = GetTextWidth( aTestStr );
        mbExtraSpace = (nTextWidth1 == nTextWidth2);

        ImplTabItem*    pItem;
        Size            aSize;
        const long      nOffsetX = 2 + GetItemsOffset().X();
        const long      nOffsetY = 2 + GetItemsOffset().Y();
        long            nX = nOffsetX;
        long            nY = nOffsetY;
        long            nMaxWidth = nWidth;
        USHORT          nPos = 0;

        if ( (mnMaxPageWidth > 0) && (mnMaxPageWidth < nMaxWidth) )
            nMaxWidth = mnMaxPageWidth;
        nMaxWidth -= GetItemsOffset().X();

        mbScroll = FALSE;
        if( 1 )
        {
            USHORT          nLines = 0;
            USHORT          nCurLine = 0;
            long            nLineWidthAry[100];
            USHORT          nLinePosAry[101];

            nLineWidthAry[0] = 0;
            nLinePosAry[0] = 0;
            pItem = mpItemList->First();
            while ( pItem )
            {
                aSize = ImplGetItemSize( pItem, nMaxWidth );

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
                if ( mbSmallInvalidate && (pItem->maRect != aNewRect) )
                    mbSmallInvalidate = FALSE;
                pItem->maRect = aNewRect;
                pItem->mnLine = nLines;
                pItem->mbFullVisible = TRUE;

                nLineWidthAry[nLines] += aSize.Width();
                nX += aSize.Width();

                if ( pItem->mnId == mnCurPageId )
                    nCurLine = nLines;

                pItem = mpItemList->Next();
                nPos++;
            }

            if ( nLines )
            {
                long    nDX = 0;
                long    nModDX = 0;
                long    nIDX = 0;
                USHORT  i;
                USHORT  n;
                long    nLineHeightAry[100];
                long    nIH = mpItemList->GetObject( 0 )->maRect.Bottom()-2;

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
                nLinePosAry[nLines+1] = (USHORT)mpItemList->Count();
                pItem = mpItemList->First();
                while ( pItem )
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

                    pItem->maRect.Left()   += nIDX;
                    pItem->maRect.Right()  += nIDX+nDX;
                    pItem->maRect.Top()     = nLineHeightAry[n-1];
                    pItem->maRect.Bottom()  = nLineHeightAry[n-1]+nIH;
                    nIDX += nDX;

                    if ( nModDX )
                    {
                        nIDX++;
                        pItem->maRect.Right()++;
                        nModDX--;
                    }

                    pItem = mpItemList->Next();
                    i++;
                }
            }
            else {//only one line
                if(ImplGetSVData()->maNWFData.mbCenteredTabs) {
                    pItem = mpItemList->First();
                    int nRightSpace=nMaxWidth;//space left on the right by the tabs
                    while ( pItem )
                    {
                        nRightSpace-=pItem->maRect.Right()-pItem->maRect.Left();
                        pItem = mpItemList->Next();
                    }
                    pItem = mpItemList->First();
                    while ( pItem )
                    {
                        pItem->maRect.Left()+=(int) (nRightSpace/2);
                        pItem->maRect.Right()+=(int) (nRightSpace/2);
                        pItem = mpItemList->Next();
                    }
                }
            }
        }

        mnLastWidth     = nWidth;
        mnLastHeight    = nHeight;
        mbFormat        = FALSE;

        ImplPosScrollBtns();
    }

    return mpItemList->GetObject( nItemPos )->maRect;
}

// -----------------------------------------------------------------------

void TabControl::ImplChangeTabPage( USHORT nId, USHORT nOldId )
{
    ImplFreeLayoutData();

    ImplTabItem*    pOldItem = ImplGetItem( nOldId );
    ImplTabItem*    pItem = ImplGetItem( nId );
    TabPage*        pOldPage = (pOldItem) ? pOldItem->mpTabPage : NULL;
    TabPage*        pPage = (pItem) ? pItem->mpTabPage : NULL;
    Window*         pCtrlParent = GetParent();

    if ( IsReallyVisible() && IsUpdateMode() )
    {
        USHORT nPos = GetPagePos( nId );
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
            pCtrlParent->SetHelpId( 0 );
        if ( mbRestoreUnqId )
            pCtrlParent->SetUniqueId( 0 );
        pOldPage->DeactivatePage();
    }

    if ( pPage )
    {
        pPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );

        // Hier Page aktivieren, damit die Controls entsprechend umgeschaltet
        // werden koennen und HilfeId gegebenenfalls beim Parent umsetzen
        if ( !GetHelpId() )
        {
            mbRestoreHelpId = TRUE;
            pCtrlParent->SetHelpId( pPage->GetHelpId() );
        }
        if ( !pCtrlParent->GetUniqueId() )
        {
            mbRestoreUnqId = TRUE;
            pCtrlParent->SetUniqueId( pPage->GetUniqueId() );
        }

        pPage->ActivatePage();

        if ( pOldPage && pOldPage->HasChildPathFocus() )
        {
            USHORT  n = 0;
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

BOOL TabControl::ImplPosCurTabPage()
{
    // Aktuelle TabPage resizen/positionieren
    ImplTabItem* pItem = ImplGetItem( GetCurPageId() );
    if ( pItem && pItem->mpTabPage )
    {
        Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
        pItem->mpTabPage->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void TabControl::ImplActivateTabPage( BOOL bNext )
{
    USHORT nCurPos = GetPagePos( GetCurPageId() );

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

void TabControl::ImplSetFirstPagePos( USHORT )
{
    return; // was only required for single line
}

// -----------------------------------------------------------------------

void TabControl::ImplShowFocus()
{
    if ( !GetPageCount() )
        return;

    // make sure the focussed item rect is computed using a bold font
    // the font may have changed meanwhile due to mouse over

    Font aOldFont( GetFont() );
    Font aFont( aOldFont );
    aFont.SetWeight( (!ImplGetSVData()->maNWFData.mbNoBoldTabFocus) ? WEIGHT_BOLD : WEIGHT_LIGHT );
    SetFont( aFont );

    USHORT          nCurPos     = GetPagePos( mnCurPageId );
    Rectangle       aRect       = ImplGetTabRect( nCurPos );
    ImplTabItem*    pItem       = mpItemList->GetObject( nCurPos );
    Size            aTabSize    = aRect.GetSize();
    long            nTextHeight = GetTextHeight();
    long            nTextWidth  = GetCtrlTextWidth( pItem->maFormatText );
    USHORT          nOff;

    if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_MONO) )
        nOff = 1;
    else
        nOff = 0;

    aRect.Left()   = aRect.Left()+((aTabSize.Width()-nTextWidth)/2)-nOff-1-1;
    aRect.Top()    = aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-1-1;
    aRect.Right()  = aRect.Left()+nTextWidth+2;
    aRect.Bottom() = aRect.Top()+nTextHeight+2;
    ShowFocus( aRect );

    SetFont( aOldFont );
}

// -----------------------------------------------------------------------

void TabControl::ImplDrawItem( ImplTabItem* pItem, const Rectangle& rCurRect, bool bLayout, bool bFirstInGroup, bool bLastInGroup, bool bIsCurrentItem )
{
    if ( pItem->maRect.IsEmpty() )
        return;

    if( bLayout )
    {
        if( ! mpLayoutData )
        {
            mpLayoutData = new vcl::ControlLayoutData();
            mpTabCtrlData->maLayoutLineToPageId.clear();
            mpTabCtrlData->maLayoutPageIdToLine.clear();
            mpTabCtrlData->maTabRectangles.clear();
        }
    }

    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aRect = pItem->maRect;
    long                    nLeftBottom = aRect.Bottom();
    long                    nRightBottom = aRect.Bottom();
    BOOL                    bLeftBorder = TRUE;
    BOOL                    bRightBorder = TRUE;
    USHORT                  nOff;
    BOOL                    bNativeOK = FALSE;

    USHORT nOff2 = 0;
    USHORT nOff3 = 0;

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nOff = 1;
    else
        nOff = 0;

    // Wenn wir die aktuelle Page sind, muessen wir etwas mehr zeichnen
    if ( pItem->mnId == mnCurPageId )
    {
        nOff2 = 2;
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
                bLeftBorder = FALSE;
            aRightTestPos.X() += 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                bRightBorder = FALSE;
        }
        else
        {
            if ( rCurRect.IsInside( aLeftTestPos ) )
                nLeftBottom -= 2;
            if ( rCurRect.IsInside( aRightTestPos ) )
                nRightBottom -= 2;
        }
    }

    if( !bLayout && (bNativeOK = IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL)) == TRUE )
    {
        ImplControlValue        aControlValue;
        Region              aCtrlRegion( pItem->maRect );
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
            ImplTabItem* pI;
            int idx=0;
            while( (pI = mpItemList->GetObject(idx++)) != NULL )
                if( (pI != pItem) && (pI->maRect.IsInside( GetPointerPosPixel() ) ) )
                {
                    nState &= ~CTRL_STATE_ROLLOVER; // avoid multiple highlighted tabs
                    break;
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
        aControlValue.setOptionalVal( (void *)(&tiValue) );

        bNativeOK = DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                    aControlValue, rtl::OUString() );
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
        int nLine = mpLayoutData->m_aLineIndices.size();
        mpLayoutData->m_aLineIndices.push_back( mpLayoutData->m_aDisplayText.Len() );
        mpTabCtrlData->maLayoutPageIdToLine[ (int)pItem->mnId ] = nLine;
        mpTabCtrlData->maLayoutLineToPageId[ nLine ] = (int)pItem->mnId;
        mpTabCtrlData->maTabRectangles.push_back( aRect );
    }

    // set font accordingly, current item is painted bold
    // we set the font attributes always before drawing to be re-entrant (DrawNativeControl may trigger additional paints)
    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    aFont.SetWeight( ((bIsCurrentItem) && (!ImplGetSVData()->maNWFData.mbNoBoldTabFocus)) ? WEIGHT_BOLD : WEIGHT_LIGHT );
    SetFont( aFont );

    Size aTabSize = aRect.GetSize();
    long nTextHeight = GetTextHeight();
    long nTextWidth = GetCtrlTextWidth( pItem->maFormatText );
    DrawCtrlText( Point( aRect.Left()+((aTabSize.Width()-nTextWidth)/2)-nOff-nOff3,
                         aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-nOff3 ),
                  pItem->maFormatText,
                  0, STRING_LEN, TEXT_DRAW_MNEMONIC,
                  bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL,
                  bLayout ? &mpLayoutData->m_aDisplayText : NULL
                  );
}

// -----------------------------------------------------------------------

IMPL_LINK( TabControl, ImplScrollBtnHdl, PushButton*, EMPTYARG )
{
    ImplSetScrollBtnsState();
    return 0;
}

// -----------------------------------------------------------------------

void TabControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
        SelectTabPage( GetPageId( rMEvt.GetPosPixel() ) );
}

// -----------------------------------------------------------------------

void TabControl::KeyInput( const KeyEvent& rKEvt )
{
    if ( GetPageCount() > 1 )
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();
        USHORT  nKeyCode = aKeyCode.GetCode();

        if ( (nKeyCode == KEY_LEFT) || (nKeyCode == KEY_RIGHT) )
        {
            BOOL bNext = (nKeyCode == KEY_RIGHT);
            ImplActivateTabPage( bNext );
        }
    }

    Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void TabControl::Paint( const Rectangle& rRect )
{
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
    ImplTabItem* pItem = mpItemList->First();
    pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == mnCurPageId )
        {
            pCurItem = pItem;
            break;
        }

        pItem = mpItemList->Next();
    }

    // Draw the TabPage border
    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aCurRect;
    long                    nTopOff = 1;
    aRect.Left()   -= TAB_OFFSET;
    aRect.Top()    -= TAB_OFFSET;
    aRect.Right()  += TAB_OFFSET;
    aRect.Bottom() += TAB_OFFSET;

    // if we have an invisible tabpage or no tabpage at all the tabpage rect should be
    // increased to avoid round corners that might be drawn by a theme
    // in this case we're only interested in the top border of the tabpage because the tabitems are used
    // standalone (eg impress)
    BOOL bNoTabPage = FALSE;
    TabPage*        pCurPage = (pCurItem) ? pCurItem->mpTabPage : NULL;
    if( !pCurPage || !pCurPage->IsVisible() )
    {
        bNoTabPage = TRUE;
        aRect.Left()-=10;
        aRect.Right()+=10;
    }

    BOOL bNativeOK = FALSE;
    if( (bNativeOK = IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL) ) == TRUE )
    {
        const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

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

        Region aCtrlRegion( aRect );
        Rectangle aClipRect( aClipRgn.GetBoundRect() );
        if( !aClipRgn.IsEmpty() ) //&& aClipRect.getHeight() && aClipRect.getWidth() )
            bNativeOK = DrawNativeControl( CTRL_TAB_PANE, part, aCtrlRegion, nState,
                aControlValue, rtl::OUString() );
    }
    else
    {
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

    // Some native toolkits (GTK+) draw tabs right-to-left, with an
    // overlap between adjacent tabs
    bool            bDrawTabsRTL = IsNativeControlSupported( CTRL_TAB_ITEM, PART_TABS_DRAW_RTL );
    ImplTabItem *   pFirstTab = NULL;
    ImplTabItem *   pLastTab = NULL;
    unsigned idx;

    // Event though there is a tab overlap with GTK+, the first tab is not
    // overlapped on the left side.  Other tookits ignore this option.
    if ( bDrawTabsRTL )
    {
        pFirstTab = mpItemList->First();
        pLastTab = mpItemList->Last();
        idx = mpItemList->Count()-1;
    }
    else
    {
        pLastTab = mpItemList->Last();
        pFirstTab = mpItemList->First();
        idx = 0;
    }

    while ( (pItem = mpItemList->GetObject(idx)) != NULL )
    {
        if ( pItem != pCurItem )
        {
            Region aClipRgn( GetActiveClipRegion() );
            aClipRgn.Intersect( pItem->maRect );
            if( !rRect.IsEmpty() )
                aClipRgn.Intersect( rRect );
            if( bLayout || !aClipRgn.IsEmpty() )
                ImplDrawItem( pItem, aCurRect, bLayout, (pItem==pFirstTab), (pItem==pLastTab), FALSE );
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
            ImplDrawItem( pCurItem, aCurRect, bLayout, (pCurItem==pFirstTab), (pCurItem==pLastTab), TRUE );
    }

    if ( !bLayout && HasFocus() )
        ImplShowFocus();

    if( ! bLayout )
        mbSmallInvalidate = TRUE;
}

// -----------------------------------------------------------------------

void TabControl::Resize()
{
    ImplFreeLayoutData();

    if ( !IsReallyShown() )
        return;

    mbFormat = TRUE;

    // Aktuelle TabPage resizen/positionieren
    BOOL bTabPage = ImplPosCurTabPage();
    // Feststellen, was invalidiert werden muss
    Size aNewSize = Control::GetOutputSizePixel();
    long nNewWidth = aNewSize.Width();
    if ( mbScroll )
        mbSmallInvalidate = FALSE;
    else
    {
        ImplTabItem* pItem;
        pItem = mpItemList->First();
        while ( pItem )
        {
            if ( !pItem->mbFullVisible ||
                 (pItem->maRect.Right()-2 >= nNewWidth) )
            {
                mbSmallInvalidate = FALSE;
                break;
            }

            pItem = mpItemList->Next();
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
    ImplShowFocus();
    SetInputContext( InputContext( GetFont() ) );
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void TabControl::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void TabControl::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nItemId = rHEvt.KeyboardActivated() ? mnCurPageId : GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

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
            ULONG nHelpId = GetHelpId( nItemId );
            if ( nHelpId )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( nHelpId, this );
                return;
            }
        }

        // Bei Quick- oder Ballloon-Help zeigen wir den Text an,
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
    }

    Control::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void TabControl::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_CONTEXTMENU) && (GetPageCount() > 1) )
    {
        Point   aMenuPos;
        BOOL    bMenu;
        if ( rCEvt.IsMouseEvent() )
        {
            aMenuPos = rCEvt.GetMousePosPixel();
            bMenu = GetPageId( aMenuPos ) != 0;
        }
        else
        {
            aMenuPos = ImplGetTabRect( GetPagePos( mnCurPageId ) ).Center();
            bMenu = TRUE;
        }

        if ( bMenu )
        {
            PopupMenu aMenu;
            ImplTabItem* pItem = mpItemList->First();
            while ( pItem )
            {
                aMenu.InsertItem( pItem->mnId, pItem->maText, MIB_CHECKABLE | MIB_RADIOCHECK );
                if ( pItem->mnId == mnCurPageId )
                    aMenu.CheckItem( pItem->mnId );
                aMenu.SetHelpId( pItem->mnId, pItem->mnHelpId );
                pItem = mpItemList->Next();
            }

            USHORT nId = aMenu.Execute( this, aMenuPos );
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
        ImplPosCurTabPage();
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == STATE_CHANGE_ZOOM)  ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
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
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Rectangle* TabControl::ImplFindPartRect( const Point& rPt )
{
    ImplTabItem* pItem = mpItemList->First();
    ImplTabItem* pFoundItem = NULL;
    int nFound = 0;
    while ( pItem )
    {
        if ( pItem->maRect.IsInside( rPt ) )
        {
            // assure that only one tab is highlighted at a time
            nFound++;
            pFoundItem = pItem;
        }
        pItem = mpItemList->Next();
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
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && (GetPageCount() > 1) )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode         aKeyCode = pKEvt->GetKeyCode();
        USHORT          nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.IsMod1() )
        {
            if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                {
                    ImplActivateTabPage( FALSE );
                    return TRUE;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    ImplActivateTabPage( TRUE );
                    return TRUE;
                }
            }
        }
    }

    return Control::Notify( rNEvt );
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
        return TRUE;
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

void TabControl::InsertPage( const ResId& rResId, USHORT nPos )
{
    GetRes( rResId.SetRT( RSC_TABCONTROLITEM ) );

    ULONG nObjMask = ReadLongRes();
    USHORT nItemId  = 1;

    // ID
    if ( nObjMask & RSC_TABCONTROLITEM_ID )
        nItemId = sal::static_int_cast<USHORT>(ReadLongRes());

    // Text
    XubString aTmpStr;
    if( nObjMask & RSC_TABCONTROLITEM_TEXT )
        aTmpStr = ReadStringRes();
    InsertPage( nItemId, aTmpStr, nPos );

    // PageResID
    if ( nObjMask & RSC_TABCONTROLITEM_PAGERESID )
    {
        ImplTabItem* pItem = mpItemList->GetObject( GetPagePos( nItemId ) );
        pItem->mnTabPageResId = sal::static_int_cast<USHORT>(ReadLongRes());
    }
}

// -----------------------------------------------------------------------

void TabControl::InsertPage( USHORT nPageId, const XubString& rText,
                             USHORT nPos )
{
    DBG_ASSERT( nPageId, "TabControl::InsertPage(): PageId == 0" );
    DBG_ASSERT( GetPagePos( nPageId ) == TAB_PAGE_NOTFOUND,
                "TabControl::InsertPage(): PageId already exists" );

    // CurPageId gegebenenfalls setzen
    if ( !mnCurPageId )
        mnCurPageId = nPageId;

    // PageItem anlegen
    ImplTabItem* pItem      = new ImplTabItem;
    pItem->mnId             = nPageId;
    pItem->mpTabPage        = NULL;
    pItem->mnTabPageResId   = 0;
    pItem->mnHelpId         = 0;
    pItem->maText           = rText;
    pItem->mbFullVisible    = FALSE;

    // In die StarView-Liste eintragen
    mpItemList->Insert( pItem, nPos );

    mbFormat = TRUE;
    if ( IsUpdateMode() )
        Invalidate();

    ImplFreeLayoutData();

    ImplCallEventListeners( VCLEVENT_TABPAGE_INSERTED, (void*) (ULONG)nPageId );
}

// -----------------------------------------------------------------------

void TabControl::RemovePage( USHORT nPageId )
{
    USHORT nPos = GetPagePos( nPageId );

    // Existiert Item
    if ( nPos != TAB_PAGE_NOTFOUND )
    {
        // Item-Daten loeschen und Windows-Item entfernen
        ImplTabItem* pItem = mpItemList->Remove( nPos );
        // If current page is removed, than first page gets the current page
        if ( pItem->mnId == mnCurPageId )
        {
            mnCurPageId = 0;

            // don't do this by simply setting mnCurPageId to pFirstItem->mnId
            // this leaves a lot of stuff (such trivias as _showing_ the new current page) undone
            // instead, call SetCurPageId
            // without this, the next (outside) call to SetCurPageId with the id of the first page
            // will result in doing nothing (as we assume that nothing changed, then), and the page
            // will never be shown.
            // 86875 - 05/11/2001 - frank.schoenheit@germany.sun.com

            ImplTabItem* pFirstItem = mpItemList->GetObject( 0 );
            if ( pFirstItem )
                SetCurPageId( pFirstItem->mnId );

        }
        delete pItem;

        mbFormat = TRUE;
        if ( IsUpdateMode() )
            Invalidate();

        ImplFreeLayoutData();

        ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVED, (void*) (ULONG) nPageId );
    }
}

// -----------------------------------------------------------------------

void TabControl::Clear()
{
    // Alle Items loeschen
    ImplTabItem* pItem = mpItemList->First();
    while ( pItem )
    {
        // Item-Daten loeschen
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Items aus der Liste loeschen
    mpItemList->Clear();
    mnCurPageId = 0;

    ImplFreeLayoutData();

    mbFormat = TRUE;
    if ( IsUpdateMode() )
        Invalidate();

    ImplCallEventListeners( VCLEVENT_TABPAGE_REMOVEDALL );
}

// -----------------------------------------------------------------------

USHORT TabControl::GetPageCount() const
{
    return (USHORT)mpItemList->Count();
}

// -----------------------------------------------------------------------

USHORT TabControl::GetPageId( USHORT nPos ) const
{
    ImplTabItem* pItem = mpItemList->GetObject( nPos );
    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT TabControl::GetPagePos( USHORT nPageId ) const
{
    ImplTabItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nPageId )
            return (USHORT)mpItemList->GetCurPos();

        pItem = mpItemList->Next();
    }

    return TAB_PAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT TabControl::GetPageId( const Point& rPos ) const
{
    USHORT i = 0;
    while ( i < mpItemList->Count() )
    {
        if ( ((TabControl*)this)->ImplGetTabRect( i ).IsInside( rPos ) )
            return mpItemList->GetObject( i )->mnId;
        i++;
    }

    return 0;
}

// -----------------------------------------------------------------------

void TabControl::SetCurPageId( USHORT nPageId )
{
    if ( nPageId == mnCurPageId )
    {
        if ( mnActPageId )
            mnActPageId = nPageId;
        return;
    }

    ImplTabItem* pItem = ImplGetItem( nPageId );
    if ( pItem )
    {
        if ( mnActPageId )
            mnActPageId = nPageId;
        else
        {
            mbFormat = TRUE;
            USHORT nOldId = mnCurPageId;
            mnCurPageId = nPageId;
            ImplChangeTabPage( nPageId, nOldId );
        }
    }
}

// -----------------------------------------------------------------------

USHORT TabControl::GetCurPageId() const
{
    if ( mnActPageId )
        return mnActPageId;
    else
        return mnCurPageId;
}

// -----------------------------------------------------------------------

void TabControl::SetFirstPageId( USHORT )
{
    return; // was only required for single line
}

// -----------------------------------------------------------------------

void TabControl::SelectTabPage( USHORT nPageId )
{
    if ( nPageId && (nPageId != mnCurPageId) )
    {
        ImplFreeLayoutData();

        ImplCallEventListeners( VCLEVENT_TABPAGE_DEACTIVATE, (void*) (ULONG) mnCurPageId );
        if ( DeactivatePage() )
        {
            mnActPageId = nPageId;
            ActivatePage();
            // Page koennte im Activate-Handler umgeschaltet wurden sein
            nPageId = mnActPageId;
            mnActPageId = 0;
            SetCurPageId( nPageId );
            ImplCallEventListeners( VCLEVENT_TABPAGE_ACTIVATE, (void*) (ULONG) nPageId );
        }
    }
}

// -----------------------------------------------------------------------

void TabControl::SetTabPage( USHORT nPageId, TabPage* pTabPage )
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
            if ( pItem->mnId == mnCurPageId )
                ImplChangeTabPage( pItem->mnId, 0 );
        }
        else
            pItem->mpTabPage = NULL;
    }
}

// -----------------------------------------------------------------------

TabPage* TabControl::GetTabPage( USHORT nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->mpTabPage;
    else
        return NULL;
}

// -----------------------------------------------------------------------

USHORT TabControl::GetTabPageResId( USHORT nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->mnTabPageResId;
    else
        return 0;
}

// -----------------------------------------------------------------------

void TabControl::SetPageText( USHORT nPageId, const XubString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem && pItem->maText != rText )
    {
        pItem->maText = rText;
        mbFormat = TRUE;
        if ( IsUpdateMode() )
            Invalidate();
        ImplFreeLayoutData();
        ImplCallEventListeners( VCLEVENT_TABPAGE_PAGETEXTCHANGED, (void*) (ULONG) nPageId );
    }
}

// -----------------------------------------------------------------------

XubString TabControl::GetPageText( USHORT nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->maText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void TabControl::SetHelpText( USHORT nPageId, const XubString& rText )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        pItem->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& TabControl::GetHelpText( USHORT nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
    {
        if ( !pItem->maHelpText.Len() && pItem->mnHelpId )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( pItem->mnHelpId, this );
        }

        return pItem->maHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void TabControl::SetHelpId( USHORT nPageId, ULONG nHelpId )
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        pItem->mnHelpId = nHelpId;
}

// -----------------------------------------------------------------------

ULONG TabControl::GetHelpId( USHORT nPageId ) const
{
    ImplTabItem* pItem = ImplGetItem( nPageId );

    if ( pItem )
        return pItem->mnHelpId;
    else
        return 0;
}

// -----------------------------------------------------------------------

Rectangle TabControl::GetCharacterBounds( USHORT nPageId, long nIndex ) const
{
    Rectangle aRet;

    if( ! mpLayoutData || ! mpTabCtrlData->maLayoutPageIdToLine.size() )
        FillLayoutData();

    if( mpLayoutData )
    {
        std::hash_map< int, int >::const_iterator it = mpTabCtrlData->maLayoutPageIdToLine.find( (int)nPageId );
        if( it != mpTabCtrlData->maLayoutPageIdToLine.end() )
        {
            Pair aPair = mpLayoutData->GetLineStartEnd( it->second );
            if( (aPair.B() - aPair.A()) >= nIndex )
                aRet = mpLayoutData->GetCharacterBounds( aPair.A() + nIndex );
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

long TabControl::GetIndexForPoint( const Point& rPoint, USHORT& rPageId ) const
{
    long nRet = -1;

    if( ! mpLayoutData || ! mpTabCtrlData->maLayoutPageIdToLine.size() )
        FillLayoutData();

    if( mpLayoutData )
    {
        int nIndex = mpLayoutData->GetIndexForPoint( rPoint );
        if( nIndex != -1 )
        {
            // what line (->pageid) is this index in ?
            int nLines = mpLayoutData->GetLineCount();
            int nLine = -1;
            while( ++nLine < nLines )
            {
                Pair aPair = mpLayoutData->GetLineStartEnd( nLine );
                if( aPair.A() <= nIndex && aPair.B() >= nIndex )
                {
                    nRet = nIndex - aPair.A();
                    rPageId = (USHORT)mpTabCtrlData->maLayoutLineToPageId[ nLine ];
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

Rectangle TabControl::GetTabPageBounds( USHORT nPage ) const
{
    Rectangle aRet;

    if( ! mpLayoutData || ! mpTabCtrlData->maLayoutPageIdToLine.size() )
        FillLayoutData();

    if( mpLayoutData )
    {
        std::hash_map< int, int >::const_iterator it = mpTabCtrlData->maLayoutPageIdToLine.find( (int)nPage );
        if( it != mpTabCtrlData->maLayoutPageIdToLine.end() )
        {
            if( it->second >= 0 && it->second < static_cast<int>(mpTabCtrlData->maTabRectangles.size()) )
            {
                aRet = mpTabCtrlData->maTabRectangles[ it->second ];
                aRet.Union( const_cast<TabControl*>(this)->ImplGetTabRect( TAB_PAGERECT ) );
            }
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

Rectangle TabControl::GetTabBounds( USHORT nPageId ) const
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
