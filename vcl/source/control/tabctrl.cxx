/*************************************************************************
 *
 *  $RCSfile: tabctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_TABCTRL_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_APP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <tabpage.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <tabctrl.hxx>
#endif

#pragma hdrstop

// =======================================================================

struct ImplTabCtrlData
{
    PushButton*         mpLeftBtn;
    PushButton*         mpRightBtn;
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

DECLARE_LIST( ImplTabItemList, ImplTabItem* );

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

#define TAB_OFFSET          3
#define TAB_TABOFFSET_X     3
#define TAB_TABOFFSET_Y     3
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

    mpItemList          = new ImplTabItemList( 8, 8 );
    mpTabCtrlData       = NULL;
    mnLastWidth         = 0;
    mnLastHeight        = 0;
    mnBtnSize           = 0;
    mnMaxPageWidth      = 0;
    mnActPageId         = 0;
    mnCurPageId         = 0;
    mnFirstPagePos      = 0;
    mnLastFirstPagePos  = 0;
    mbFormat            = TRUE;
    mbRestoreHelpId     = FALSE;
    mbRestoreUnqId      = FALSE;
    mbSingleLine        = FALSE;
    mbScroll            = FALSE;
    mbColored           = FALSE;

    if ( GetSettings().GetStyleSettings().GetTabControlStyle() & STYLE_TABCONTROL_SINGLELINE )
        mbSingleLine = TRUE;

    if ( mbSingleLine )
    {
        mpTabCtrlData               = new ImplTabCtrlData;
        mpTabCtrlData->mpLeftBtn    = NULL;
        mpTabCtrlData->mpRightBtn   = NULL;
    }

    ImplInitSettings( TRUE, TRUE, TRUE );
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
        if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
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

    // Sollen TabReiter farbig dargestellt werden
    mbColored =  (rStyleSettings.GetTabControlStyle() & STYLE_TABCONTROL_COLOR) != 0;
    ImplScrollBtnsColor();
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

    USHORT nObjMask = ReadShortRes();

    if ( nObjMask & RSC_TABCONTROL_ITEMLIST )
    {
        USHORT nEle = ReadShortRes();

        // Item hinzufuegen
        for( USHORT i = 0; i < nEle; i++ )
        {
            InsertPage( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }
}

// -----------------------------------------------------------------------

TabControl::~TabControl()
{
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
        if ( mbColored )
        {
            Color aScrollBtnColor( COL_LIGHTBLUE );
            mpTabCtrlData->mpLeftBtn->SetControlForeground( aScrollBtnColor );
            mpTabCtrlData->mpRightBtn->SetControlForeground( aScrollBtnColor );
        }
        else
        {
            mpTabCtrlData->mpLeftBtn->SetControlForeground();
            mpTabCtrlData->mpRightBtn->SetControlForeground();
        }
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

Size TabControl::ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth ) const
{
    pItem->maFormatText = pItem->maText;
    Size aSize( GetCtrlTextWidth( pItem->maFormatText ), GetTextHeight() );
    aSize.Width()  += TAB_TABOFFSET_X*2;
    aSize.Height() += TAB_TABOFFSET_Y*2;

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

Rectangle TabControl::ImplGetTabRect( USHORT nPos, long nWidth, long nHeight )
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

    if ( nPos == TAB_PAGERECT )
    {
        USHORT nLastPos;
        if ( mbSingleLine )
            nLastPos = mnFirstPagePos;
        else
        {
            if ( mnCurPageId )
                nLastPos = GetPagePos( mnCurPageId );
            else
                nLastPos = 0;
        }
        Rectangle aRect = ImplGetTabRect( nLastPos, nWidth, nHeight );
        aRect = Rectangle( Point( TAB_OFFSET, aRect.Bottom()+TAB_OFFSET ),
                           Size( nWidth-TAB_OFFSET*2,
                                 nHeight-aRect.Bottom()-TAB_OFFSET*2 ) );
        return aRect;
    }

    nWidth -= 1;

    if ( (nWidth <= 0) || (nHeight <= 0) )
        return Rectangle();

    Font aFont = GetFont();
    FontWeight eWeight = aFont.GetWeight();
    if ( eWeight != WEIGHT_BOLD )
        aFont.SetWeight( WEIGHT_BOLD );
    if ( !aFont.IsTransparent() )
        aFont.SetTransparent( TRUE );

    if ( mbFormat || (mnLastWidth != nWidth) || (mnLastHeight != nHeight) )
    {
        if ( eWeight != WEIGHT_BOLD )
            SetFont( aFont );

        ImplTabItem*    pItem;
        Size            aSize;
        long            nX = 2;
        long            nY = 2;
        long            nMaxWidth = nWidth;
        USHORT          nPos = 0;

        if ( (mnMaxPageWidth > 0) && (mnMaxPageWidth < nMaxWidth) )
            nMaxWidth = mnMaxPageWidth;

        mbScroll = FALSE;
        if ( mbSingleLine )
        {
            // Zuerst ermitteln wir, ob wir scrollen muessen
            pItem = mpItemList->First();
            while ( pItem )
            {
                aSize = ImplGetItemSize( pItem, nMaxWidth );
                pItem->maRect = Rectangle( Point( nX, nY ), aSize );
                pItem->mnLine = 1;
                pItem->mbFullVisible = TRUE;
                nX += aSize.Width();

                if ( (nX > nWidth-2) && (nWidth > 4) )
                    mbScroll = TRUE;

                pItem = mpItemList->Next();
            }

            // Wenn wir Scrollen muessen, dann muessen die Reiter
            // entsprechend angeordnet werden
            if ( mbScroll )
            {
                // Zuerst ermitteln wir den letzten TabReiter, bei dem
                // die restlichen noch sichtbar bleiben und passen
                // gegebenenfalls den ersten sichtbaren Writer an
                mnBtnSize = GetTextHeight()+(TAB_TABOFFSET_Y*2);
                long    nMaxWidth = nWidth-(mnBtnSize*2);
                long    nTempWidth = 0;
                USHORT  nPageCount = GetPageCount();
                mnLastFirstPagePos = nPageCount;
                pItem = mpItemList->Last();
                while ( pItem )
                {
                    nTempWidth += pItem->maRect.GetSize().Width();
                    if ( nTempWidth > nMaxWidth )
                        break;

                    mnLastFirstPagePos--;
                    pItem = mpItemList->Prev();
                }
                if ( mnLastFirstPagePos > nPageCount-1 )
                    mnLastFirstPagePos = nPageCount-1;
                if ( mnFirstPagePos > mnLastFirstPagePos )
                   mnFirstPagePos = mnLastFirstPagePos;

                // Jetzt die TabReiter anordnen und die Reiter ausblenden,
                // die nicht zu sehen sind
                nPos = 0;
                nX = 2;
                pItem = mpItemList->First();
                while ( pItem )
                {
                    if ( (nPos < mnFirstPagePos) ||
                         ((nX > nWidth-2) && (nWidth > 4)) )
                    {
                        pItem->mbFullVisible = FALSE;
                        pItem->maRect.SetEmpty();
                    }
                    else
                    {
                        aSize = pItem->maRect.GetSize();
                        pItem->maRect = Rectangle( Point( nX, nY ), aSize );
                        nX += aSize.Width();
                    }

                    if ( nX > nMaxWidth )
                        pItem->mbFullVisible = FALSE;

                    pItem = mpItemList->Next();
                    nPos++;
                }
            }
            else
                mnFirstPagePos = 0;
        }
        else
        {
            USHORT          nLines = 0;
            USHORT          nCurLine = 0;
            long            nLineWidthAry[100];
            USHORT          nLinePosAry[100];

            nLineWidthAry[0] = 0;
            nLinePosAry[0] = 0;
            pItem = mpItemList->First();
            while ( pItem )
            {
                aSize = ImplGetItemSize( pItem, nMaxWidth );

                if ( ((nX+aSize.Width()) > nWidth-2) && (nWidth > 4) )
                {
                    if ( nLines == 99 )
                        break;

                    nX  = 2;
                    nY += aSize.Height();
                    nLines++;
                    nLineWidthAry[nLines] = 0;
                    nLinePosAry[nLines] = nPos;
                }

                pItem->maRect = Rectangle( Point( nX, nY ), aSize );
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
                long    nDX;
                long    nModDX;
                long    nIDX;
                USHORT  i;
                USHORT  n;
                long    nLineHeightAry[100];
                long    nIH = mpItemList->GetObject( 0 )->maRect.Bottom()-2;

                i = 0;
                while ( i < nLines+1 )
                {
                    if ( i <= nCurLine )
                        nLineHeightAry[i] = nIH*(nLines-(nCurLine-i));
                    else
                        nLineHeightAry[i] = nIH*(i-nCurLine-1);
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
                        nDX = (nWidth-2-nLineWidthAry[n]) / (nLinePosAry[n+1]-i);
                        nModDX = (nWidth-2-nLineWidthAry[n]) % (nLinePosAry[n+1]-i);
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
        }

        mnLastWidth     = nWidth;
        mnLastHeight    = nHeight;
        mbFormat        = FALSE;

        ImplPosScrollBtns();
    }

    return mpItemList->GetObject( nPos )->maRect;
}

// -----------------------------------------------------------------------

void TabControl::ImplChangeTabPage( USHORT nId, USHORT nOldId )
{
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

        if( Application::GetAccessHdlCount() && pPage->IsReallyVisible() )
            Application::AccessNotify( AccessNotification( ACCESS_EVENT_DLGCONTROLS, GetParent() ) );
    }

    if ( pOldPage )
        pOldPage->Hide();

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

void TabControl::ImplSetFirstPagePos( USHORT nPagePos )
{
    if ( !mbSingleLine )
        return;

    if ( mbFormat )
        mnFirstPagePos = nPagePos;
    else
    {
        if ( nPagePos > mnLastFirstPagePos )
            nPagePos = mnLastFirstPagePos;

        if ( nPagePos != mnFirstPagePos )
        {
            // Neu auszugebene Rechteck berechnen
            Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );
            aRect.Bottom()  = aRect.Top();
            aRect.Left()    = 0;
            aRect.Top()     = 0;
            aRect.Right()   = Control::GetOutputSizePixel().Width();

            mbFormat = TRUE;
            mnFirstPagePos = nPagePos;
            Invalidate( aRect, INVALIDATE_NOCHILDREN );
        }
    }
}

// -----------------------------------------------------------------------

void TabControl::ImplShowFocus()
{
    if ( !GetPageCount() )
        return;

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
}

// -----------------------------------------------------------------------

void TabControl::ImplDrawItem( ImplTabItem* pItem, const Rectangle& rCurRect )
{
    if ( pItem->maRect.IsEmpty() )
        return;

    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aRect = pItem->maRect;
    long                    nLeftBottom = aRect.Bottom();
    long                    nRightBottom = aRect.Bottom();
    BOOL                    bLeftBorder = TRUE;
    BOOL                    bRightBorder = TRUE;
    USHORT                  nOff;

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

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        if ( mbColored )
        {
            USHORT  nPos = (USHORT)mpItemList->GetPos( pItem );
            Color   aOldFillColor = GetFillColor();
            SetLineColor();
            SetFillColor( aImplTabColorAry[nPos%TABCOLORCOUNT] );
            Rectangle aColorRect;
            aColorRect.Left()   = aRect.Left()-nOff2+1;
            aColorRect.Top()    = aRect.Top()-nOff2+1;
            aColorRect.Right()  = aRect.Right()+nOff2-3;
            aColorRect.Bottom() = nLeftBottom;
            if ( pItem->mnId != mnCurPageId )
                aColorRect.Bottom()--;
            DrawRect( aColorRect );
            SetFillColor( aOldFillColor );
        }

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

    Size aTabSize = aRect.GetSize();
    long nTextHeight = GetTextHeight();
    long nTextWidth = GetCtrlTextWidth( pItem->maFormatText );
    DrawCtrlText( Point( aRect.Left()+((aTabSize.Width()-nTextWidth)/2)-nOff-nOff3,
                         aRect.Top()+((aTabSize.Height()-nTextHeight)/2)-nOff3 ),
                  pItem->maFormatText );
}

// -----------------------------------------------------------------------

IMPL_LINK( TabControl, ImplScrollBtnHdl, PushButton*, pBtn )
{
    if ( pBtn == mpTabCtrlData->mpRightBtn )
        ImplSetFirstPagePos( mnFirstPagePos+1 );
    else
        ImplSetFirstPagePos( mnFirstPagePos-1 );
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
    HideFocus();

    // Hier wird gegebenenfalls auch neu formatiert
    Rectangle aRect = ImplGetTabRect( TAB_PAGERECT );

    // Fonts entsprechend setzen
    Font aFont( GetFont() );
    Font aLightFont = aFont;
    aFont.SetTransparent( TRUE );
    aFont.SetWeight( WEIGHT_BOLD );
    aLightFont.SetTransparent( TRUE );
    aLightFont.SetWeight( WEIGHT_LIGHT );

    // Aktuelles Item ermitteln
    ImplTabItem* pPrevCurItem = NULL;
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

    // Border um TabPage zeichnen
    const StyleSettings&    rStyleSettings  = GetSettings().GetStyleSettings();
    Rectangle               aCurRect;
    long                    nTopOff = 1;
    aRect.Left()   -= TAB_OFFSET;
    aRect.Top()    -= TAB_OFFSET;
    aRect.Right()  += TAB_OFFSET;
    aRect.Bottom() += TAB_OFFSET;
    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        SetLineColor( rStyleSettings.GetLightColor() );
    else
        SetLineColor( Color( COL_BLACK ) );
    if ( pCurItem && !pCurItem->maRect.IsEmpty() )
    {
        aCurRect = pCurItem->maRect;
        DrawLine( aRect.TopLeft(), Point( aCurRect.Left()-2, aRect.Top() ) );
        if ( aCurRect.Right()+1 < aRect.Right() )
            DrawLine( Point( aCurRect.Right(), aRect.Top() ), aRect.TopRight() );
        else
            nTopOff = 0;
    }
    else
        DrawLine( aRect.TopLeft(), aRect.TopRight() );
    DrawLine( aRect.TopLeft(), aRect.BottomLeft() );

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        SetLineColor( rStyleSettings.GetShadowColor() );
        DrawLine( Point( 1, aRect.Bottom()-1 ),
                  Point( aRect.Right()-1, aRect.Bottom()-1 ) );
        DrawLine( Point( aRect.Right()-1, aRect.Top()+nTopOff ),
                  Point( aRect.Right()-1, aRect.Bottom()-1 ) );
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

    // Alle Items bis auf das aktuelle Zeichnen (nicht fett)
    SetFont( aLightFont );
    pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem != pCurItem )
            ImplDrawItem( pItem, aCurRect );
        pItem = mpItemList->Next();
    }

    // aktuelles Item zeichnen wir fett
    SetFont( aFont );
    if ( pCurItem )
        ImplDrawItem( pCurItem, aCurRect );

    if ( HasFocus() )
        ImplShowFocus();
}

// -----------------------------------------------------------------------

void TabControl::Resize()
{
    if ( !IsReallyShown() )
        return;

    mbFormat = TRUE;

    // Aktuelle TabPage resizen/positionieren
    BOOL bTabPage = ImplPosCurTabPage();
    // Feststellen, was invalidiert werden muss
    Size aNewSize = Control::GetOutputSizePixel();
    long nWidthTest = aNewSize.Width();
    BOOL bSmallInvalidate = TRUE;
    BOOL bWidthChanged = (nWidthTest != mnLastWidth);
    if ( mbScroll )
        bSmallInvalidate = FALSE;
    else if ( bWidthChanged )
    {
        if ( mnLastWidth < nWidthTest )
            nWidthTest = mnLastWidth;

        ImplTabItem* pItem;
        pItem = mpItemList->First();
        while ( pItem )
        {
            if ( !pItem->mbFullVisible ||
                 (pItem->maRect.Right()-2 >= nWidthTest) )
            {
                bSmallInvalidate = FALSE;
                break;
            }

            pItem = mpItemList->Next();
        }
    }

    if ( bSmallInvalidate )
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
    USHORT nItemId = GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

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
                    pHelp->Start( nHelpId );
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

    USHORT nObjMask = ReadShortRes();
    USHORT nItemId  = 1;

    // ID
    if ( nObjMask & RSC_TABCONTROLITEM_ID )
        nItemId = ReadShortRes();

    // Text
    XubString aTmpStr;
    if( nObjMask & RSC_TABCONTROLITEM_TEXT )
        aTmpStr = ReadStringRes();
    InsertPage( nItemId, aTmpStr, nPos );

    // PageResID
    if ( nObjMask & RSC_TABCONTROLITEM_PAGERESID )
    {
        ImplTabItem* pItem = mpItemList->GetObject( GetPagePos( nItemId ) );
        pItem->mnTabPageResId = ReadShortRes();
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
        if ( pItem->mnId == mnCurPageId )
            mnCurPageId = 0;
        delete pItem;

        mbFormat = TRUE;
        if ( IsUpdateMode() )
            Invalidate();
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

    mbFormat = TRUE;
    if ( IsUpdateMode() )
        Invalidate();
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
            if ( pItem->maRect.IsEmpty() || !pItem->mbFullVisible )
                SetFirstPageId( nPageId );
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

void TabControl::SetFirstPageId( USHORT nPageId )
{
    USHORT nPos = GetPagePos( nPageId );
    if ( (nPos != TAB_PAGE_NOTFOUND) && (nPos != mnFirstPagePos) )
        ImplSetFirstPagePos( nPos );
}

// -----------------------------------------------------------------------

void TabControl::SelectTabPage( USHORT nPageId )
{
    if ( nPageId && (nPageId != mnCurPageId) )
    {
        if ( DeactivatePage() )
        {
            mnActPageId = nPageId;
            ActivatePage();
            // Page koennte im Activate-Handler umgeschaltet wurden sein
            nPageId = mnActPageId;
            mnActPageId = 0;
            SetCurPageId( nPageId );
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

    if ( pItem )
    {
        pItem->maText = rText;
        mbFormat = TRUE;
        if ( IsUpdateMode() )
            Invalidate();
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
                pItem->maHelpText = pHelp->GetHelpText( pItem->mnHelpId );
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
