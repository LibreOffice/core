/*************************************************************************
 *
 *  $RCSfile: valueset.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2000-11-16 19:18:50 $
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

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifdef VCL
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#include <vcl/svapp.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _HELP_HXX
#include <vcl/help.hxx>
#endif

#define _SV_VALUESET_CXX
#define private public
#include <valueset.hxx>

// =======================================================================

#define ITEM_OFFSET                 4
#define ITEM_OFFSET_DOUBLE          6
#define NAME_LINE_OFF_X             2
#define NAME_LINE_OFF_Y             2
#define NAME_LINE_HEIGHT            2
#define NAME_OFFSET                 2
#define SCRBAR_OFFSET               1
#define VALUESET_ITEM_NONEITEM      0xFFFE

#define VALUESET_SCROLL_OFFSET      4

enum ValueSetItemType { VALUESETITEM_NONE, VALUESETITEM_IMAGE,
                        VALUESETITEM_COLOR, VALUESETITEM_USERDRAW,
                        VALUESETITEM_SPACE };

struct ValueSetItem
{
    USHORT              mnId;
    USHORT              mnBits;
    ValueSetItemType    meType;
    Image               maImage;
    Color               maColor;
    XubString           maText;
    void*               mpData;
    Rectangle           maRect;

                        ValueSetItem();
                        ~ValueSetItem();
};

DECLARE_LIST( ValueItemList, ValueSetItem* );

// =======================================================================

ValueSetItem::ValueSetItem()
{
    mnBits      = 0;
    mpData      = NULL;
}

// -----------------------------------------------------------------------

ValueSetItem::~ValueSetItem()
{
}

// =======================================================================

void ValueSet::ImplInit( WinBits nWinStyle )
{
    Size aWinSize       = GetSizePixel();
    mpItemList          = new ValueItemList;
    mpNoneItem          = NULL;
    mpScrBar            = NULL;
    mnTextOffset        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnOldItemId         = 0;
    mnSelItemId         = 0;
    mnHighItemId        = 0;
    mnDropPos           = VALUESET_ITEM_NOTFOUND;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = 0;
    mbFormat            = TRUE;
    mbHighlight         = FALSE;
    mbSelection         = FALSE;
    mbNoSelection       = TRUE;
    mbDrawSelection     = TRUE;
    mbBlackSel          = FALSE;
    mbDoubleSel         = FALSE;
    mbScroll            = FALSE;
    mbDropPos           = FALSE;
    mbFullMode          = TRUE;

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit( nWinStyle );
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit( rResId.aWinBits );
}

// -----------------------------------------------------------------------

ValueSet::~ValueSet()
{
    if ( mpScrBar )
        delete mpScrBar;

    if ( mpNoneItem )
        delete mpNoneItem;

    ValueSetItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }
    delete mpItemList;
}

// -----------------------------------------------------------------------

void ValueSet::ImplInitSettings( BOOL bFont,
                                 BOOL bForeground, BOOL bBackground )
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
        else if ( GetStyle() & WB_FLATVALUESET )
            aColor = rStyleSettings.GetWindowColor();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplInitScrollBar()
{
    if ( GetStyle() & WB_VSCROLL )
    {
        if ( !mpScrBar )
        {
            mpScrBar = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
            mpScrBar->SetScrollHdl( LINK( this, ValueSet, ImplScrollHdl ) );
        }
        else
        {
            // Wegen Einstellungsaenderungen passen wir hier die Breite an
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->SetPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplFormatItem( ValueSetItem* pItem )
{
    if ( pItem->meType == VALUESETITEM_SPACE )
        return;

    Rectangle aRect = pItem->maRect;
    WinBits nStyle = GetStyle();
    if ( nStyle & WB_ITEMBORDER )
    {
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        if ( nStyle & WB_FLATVALUESET )
        {
            if ( nStyle  & WB_DOUBLEBORDER )
            {
                aRect.Left()    += 2;
                aRect.Top()     += 2;
                aRect.Right()   -= 2;
                aRect.Bottom()  -= 2;
            }
            else
            {
                aRect.Left()++;
                aRect.Top()++;
                aRect.Right()--;
                aRect.Bottom()--;
            }
        }
        else
        {
            DecorationView aView( &maVirDev );
            aRect = aView.DrawFrame( aRect, mnFrameStyle );
        }
    }

    if ( pItem == mpNoneItem )
        pItem->maText = GetText();

    if ( (aRect.GetHeight() > 0) && (aRect.GetWidth() > 0) )
    {
        if ( pItem == mpNoneItem )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            maVirDev.SetFont( GetFont() );
            maVirDev.SetTextColor( rStyleSettings.GetWindowTextColor() );
            maVirDev.SetTextFillColor();
            maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
            maVirDev.DrawRect( aRect );
            Point   aTxtPos( aRect.Left()+2, aRect.Top() );
            long    nTxtWidth = GetTextWidth( pItem->maText );
            if ( nStyle & WB_RADIOSEL )
            {
                aTxtPos.X() += 4;
                aTxtPos.Y() += 4;
            }
            if ( (aTxtPos.X()+nTxtWidth) > aRect.Right() )
            {
                maVirDev.SetClipRegion( Region( aRect ) );
                maVirDev.DrawText( aTxtPos, pItem->maText );
                maVirDev.SetClipRegion();
            }
            else
                maVirDev.DrawText( aTxtPos, pItem->maText );
        }
        else if ( pItem->meType == VALUESETITEM_COLOR )
        {
            maVirDev.SetFillColor( pItem->maColor );
            maVirDev.DrawRect( aRect );
        }
        else
        {
            if ( IsColor() )
                maVirDev.SetFillColor( maColor );
            else
                maVirDev.SetFillColor( GetSettings().GetStyleSettings().GetWindowColor() );
            maVirDev.DrawRect( aRect );

            if ( pItem->meType == VALUESETITEM_USERDRAW )
            {
                UserDrawEvent aUDEvt( &maVirDev, aRect, pItem->mnId );
                UserDraw( aUDEvt );
            }
            else
            {
                Size    aImageSize = pItem->maImage.GetSizePixel();
                Size    aRectSize = aRect.GetSize();
                Point   aPos( aRect.Left(), aRect.Top() );
                aPos.X() += (aRectSize.Width()-aImageSize.Width())/2;
                aPos.Y() += (aRectSize.Height()-aImageSize.Height())/2;
                if ( (aImageSize.Width()  > aRectSize.Width()) ||
                     (aImageSize.Height() > aRectSize.Height()) )
                {
                    maVirDev.SetClipRegion( Region( aRect ) );
                    maVirDev.DrawImage( aPos, pItem->maImage );
                    maVirDev.SetClipRegion();
                }
                else
                    maVirDev.DrawImage( aPos, pItem->maImage );
            }
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::Format()
{
    Size        aWinSize = GetOutputSizePixel();
    ULONG       nItemCount = mpItemList->Count();
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        nOff;
    long        nSpace;
    long        nNoneHeight;
    long        nNoneSpace;
    ScrollBar*  pDelScrBar = NULL;

    // Scrolling beruecksichtigen
    if ( nStyle & WB_VSCROLL )
        ImplInitScrollBar();
    else
    {
        if ( mpScrBar )
        {
            // ScrollBar erst spaeter zerstoeren, damit keine rekursiven
            // Aufrufe entstehen koennen
            pDelScrBar = mpScrBar;
            mpScrBar = NULL;
        }
    }

    // Item-Offset berechnen
    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            nOff = ITEM_OFFSET_DOUBLE;
        else
            nOff = ITEM_OFFSET;
    }
    else
        nOff = 0;
    nSpace = mnSpacing;

    // Groesse beruecksichtigen, wenn NameField vorhanden
    if ( nStyle & WB_NAMEFIELD )
    {
        mnTextOffset = aWinSize.Height()-nTxtHeight-NAME_OFFSET;
        aWinSize.Height() -= nTxtHeight+NAME_OFFSET;

        if ( !(nStyle & WB_FLATVALUESET) )
        {
            mnTextOffset -= NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
            aWinSize.Height() -= NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        }
    }
    else
        mnTextOffset = 0;

    // Offset und Groesse beruecksichtigen, wenn NoneField vorhanden
    if ( nStyle & WB_NONEFIELD )
    {
        nNoneHeight = nTxtHeight+nOff;
        nNoneSpace = nSpace;
        if ( nStyle & WB_RADIOSEL )
            nNoneHeight += 8;
    }
    else
    {
        nNoneHeight = 0;
        nNoneSpace = 0;

        if ( mpNoneItem )
        {
            delete mpNoneItem;
            mpNoneItem = NULL;
        }
    }

    // Breite vom ScrollBar berechnen
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;

    // Spaltenanzahl berechnen
    if ( !mnUserCols )
    {
        if ( mnUserItemWidth )
        {
            mnCols = (USHORT)((aWinSize.Width()-nScrBarWidth+nSpace) / (mnUserItemWidth+nSpace));
            if ( !mnCols )
                mnCols = 1;
        }
        else
            mnCols = 1;
    }
    else
        mnCols = mnUserCols;

    // Zeilenanzahl berechnen
    mbScroll = FALSE;
    mnLines = (long)mpItemList->Count() / mnCols;
    if ( mpItemList->Count() % mnCols )
        mnLines++;
    else if ( !mnLines )
        mnLines = 1;

    long nCalcHeight = aWinSize.Height()-nNoneHeight;
    if ( mnUserVisLines )
        mnVisLines = mnUserVisLines;
    else if ( mnUserItemHeight )
    {
        mnVisLines = (nCalcHeight-nNoneSpace+nSpace) / (mnUserItemHeight+nSpace);
        if ( !mnVisLines )
            mnVisLines = 1;
    }
    else
        mnVisLines = mnLines;
    if ( mnLines > mnVisLines )
        mbScroll = TRUE;
    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else
    {
        if ( mnFirstLine > (USHORT)(mnLines-mnVisLines) )
            mnFirstLine = (USHORT)(mnLines-mnVisLines);
    }

    // Itemgroessen berechnen
    long nColSpace  = (mnCols-1)*nSpace;
    long nLineSpace = ((mnVisLines-1)*nSpace)+nNoneSpace;
    long nItemWidth;
    long nItemHeight;
    if ( mnUserItemWidth && !mnUserCols )
    {
        nItemWidth = mnUserItemWidth;
        if ( nItemWidth > aWinSize.Width()-nScrBarWidth-nColSpace )
            nItemWidth = aWinSize.Width()-nScrBarWidth-nColSpace;
    }
    else
        nItemWidth = (aWinSize.Width()-nScrBarWidth-nColSpace) / mnCols;
    if ( mnUserItemHeight && !mnUserVisLines )
    {
        nItemHeight = mnUserItemHeight;
        if ( nItemHeight > nCalcHeight-nNoneSpace )
            nItemHeight = nCalcHeight-nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        nItemHeight = nCalcHeight / mnVisLines;
    }

    // Init VirDev
    maVirDev.SetSettings( GetSettings() );
    maVirDev.SetBackground( GetBackground() );
    maVirDev.SetOutputSizePixel( aWinSize, TRUE );

    // Bei zu kleinen Items machen wir nichts
    long nMinHeight = 2;
    if ( nStyle & WB_ITEMBORDER )
        nMinHeight = 4;
    if ( (nItemWidth <= 0) || (nItemHeight <= nMinHeight) || !nItemCount )
    {
        if ( nStyle & WB_NONEFIELD )
        {
            if ( mpNoneItem )
            {
                mpNoneItem->maRect.SetEmpty();
                mpNoneItem->maText = GetText();
            }
        }

        for ( ULONG i = 0; i < nItemCount; i++ )
        {
            ValueSetItem* pItem = mpItemList->GetObject( i );
            pItem->maRect.SetEmpty();
        }

        if ( mpScrBar )
            mpScrBar->Hide();
    }
    else
    {
        // Frame-Style ermitteln
        if ( nStyle & WB_DOUBLEBORDER )
            mnFrameStyle = FRAME_DRAW_DOUBLEIN;
        else
            mnFrameStyle = FRAME_DRAW_IN;

        // Selektionsfarben und -breiten ermitteln
        // Gegebenenfalls die Farben anpassen, damit man die Selektion besser
        // erkennen kann
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aHighColor( rStyleSettings.GetHighlightColor() );
        if ( ((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
              (aHighColor.GetBlue() > 0x80)) ||
             ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
              (aHighColor.GetBlue() == 0x80)) )
            mbBlackSel = TRUE;
        else
            mbBlackSel = FALSE;

        // Wenn die Items groesser sind, dann die Selektion doppelt so breit
        // zeichnen
        if ( (nStyle & WB_DOUBLEBORDER) &&
             ((nItemWidth >= 25) && (nItemHeight >= 20)) )
            mbDoubleSel = TRUE;
        else
            mbDoubleSel = FALSE;

        // Calculate offsets
        long nStartX;
        long nStartY;
        if ( mbFullMode )
        {
            long nAllItemWidth = (nItemWidth*mnCols)+nColSpace;
            long nAllItemHeight = (nItemHeight*mnVisLines)+nNoneHeight+nLineSpace;
            nStartX = (aWinSize.Width()-nScrBarWidth-nAllItemWidth)/2;
            nStartY = (aWinSize.Height()-nAllItemHeight)/2;
        }
        else
        {
            nStartX = 0;
            nStartY = 0;
        }

        // Items berechnen und zeichnen
        maVirDev.SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // NoSelection-Field erzeugen und anzeigen
        if ( nStyle & WB_NONEFIELD )
        {
            if ( !mpNoneItem )
                mpNoneItem = new ValueSetItem;

            mpNoneItem->mnId            = 0;
            mpNoneItem->meType          = VALUESETITEM_NONE;
            mpNoneItem->maRect.Left()   = x;
            mpNoneItem->maRect.Top()    = y;
            mpNoneItem->maRect.Right()  = mpNoneItem->maRect.Left()+aWinSize.Width()-x-1;
            mpNoneItem->maRect.Bottom() = y+nNoneHeight-1;

            ImplFormatItem( mpNoneItem );

            y += nNoneHeight+nNoneSpace;
        }

        // draw items
        ULONG nFirstItem = mnFirstLine * mnCols;
        ULONG nLastItem = nFirstItem + (mnVisLines * mnCols);
        if ( !mbFullMode )
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if ( y+(mnVisLines*(nItemHeight+nSpace)) < aWinSize.Height() )
                nLastItem += mnCols;
        }
        for ( ULONG i = 0; i < nItemCount; i++ )
        {
            ValueSetItem* pItem = mpItemList->GetObject( i );

            if ( (i >= nFirstItem) && (i < nLastItem) )
            {
                pItem->maRect.Left()    = x;
                pItem->maRect.Top()     = y;
                pItem->maRect.Right()   = pItem->maRect.Left()+nItemWidth-1;
                pItem->maRect.Bottom()  = pItem->maRect.Top()+nItemHeight-1;

                ImplFormatItem( pItem );

                if ( !((i+1) % mnCols) )
                {
                    x = nStartX;
                    y += nItemHeight+nSpace;
                }
                else
                    x += nItemWidth+nSpace;
            }
            else
                pItem->maRect.SetEmpty();
        }

        // ScrollBar anordnen, Werte setzen und anzeigen
        if ( mpScrBar )
        {
            Point   aPos( aWinSize.Width()-nScrBarWidth+SCRBAR_OFFSET, 0 );
            Size    aSize( nScrBarWidth-SCRBAR_OFFSET, aWinSize.Height() );
            // If a none field is visible, then we center the scrollbar
            if ( nStyle & WB_NONEFIELD )
            {
                aPos.Y() = nStartY+nNoneHeight+1;
                aSize.Height() = ((nItemHeight+nSpace)*mnVisLines)-2-nSpace;
            }
            mpScrBar->SetPosSizePixel( aPos, aSize );
            mpScrBar->SetRangeMax( mnLines );
            mpScrBar->SetVisibleSize( mnVisLines );
            mpScrBar->SetThumbPos( (long)mnFirstLine );
            long nPageSize = mnVisLines;
            if ( nPageSize < 1 )
                nPageSize = 1;
            mpScrBar->SetPageSize( nPageSize );
            mpScrBar->Show();
        }
    }

    // Jetzt haben wir formatiert und warten auf das naechste
    mbFormat = FALSE;

    // ScrollBar loeschen
    if ( pDelScrBar )
        delete pDelScrBar;
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawItemText( const XubString& rText )
{
    if ( !(GetStyle() & WB_NAMEFIELD) )
        return;

    Size    aWinSize = GetOutputSizePixel();
    long    nTxtWidth = GetTextWidth( rText );
    long    nTxtOffset = mnTextOffset;

    // Rechteck loeschen und Text ausgeben
    if ( GetStyle() & WB_FLATVALUESET )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor();
        SetFillColor( rStyleSettings.GetFaceColor() );
        DrawRect( Rectangle( Point( 0, nTxtOffset ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
        SetTextColor( rStyleSettings.GetButtonTextColor() );
    }
    else
    {
        nTxtOffset += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
        Erase( Rectangle( Point( 0, nTxtOffset ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
    }
    DrawText( Point( (aWinSize.Width()-nTxtWidth) / 2, nTxtOffset+(NAME_OFFSET/2) ), rText );
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawSelect()
{
    if ( !IsReallyVisible() )
        return;

    BOOL bFocus = HasFocus();
    BOOL bDrawSel;

    if ( (mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight) )
        bDrawSel = FALSE;
    else
        bDrawSel = TRUE;

    if ( !bFocus &&
         ((mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight)) )
    {
        XubString aEmptyStr;
        ImplDrawItemText( aEmptyStr );
        return;
    }

    USHORT nItemId = mnSelItemId;

    if ( mbHighlight )
        nItemId = mnHighItemId;

    ValueSetItem* pItem;
    if ( nItemId )
        pItem = mpItemList->GetObject( GetItemPos( nItemId ) );
    else
    {
        if ( mpNoneItem )
            pItem = mpNoneItem;
        else
        {
            pItem = ImplGetFirstItem();
            if ( !bFocus || !pItem )
                return;
        }
    }

    if ( pItem->maRect.IsEmpty() )
        return;

    // Selection malen
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle               aRect = pItem->maRect;
    Control::SetFillColor();

    // Selectionsausgabe festlegen
    WinBits nStyle = GetStyle();
    if ( nStyle & WB_RADIOSEL )
    {
        aRect.Left()    += 3;
        aRect.Top()     += 3;
        aRect.Right()   -= 3;
        aRect.Bottom()  -= 3;
        if ( nStyle & WB_DOUBLEBORDER )
        {
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
        }

        if ( bFocus )
            ShowFocus( aRect );

        if ( bDrawSel )
        {
            SetLineColor( rStyleSettings.GetHighlightColor() );
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            DrawRect( aRect );
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            DrawRect( aRect );
        }
    }
    else
    {
        if ( bDrawSel )
        {
            if ( mbBlackSel )
                SetLineColor( Color( COL_BLACK ) );
            else
                SetLineColor( rStyleSettings.GetHighlightColor() );
            DrawRect( aRect );
        }
        if ( mbDoubleSel )
        {
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            if ( bDrawSel )
                DrawRect( aRect );
        }
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        Rectangle aRect2 = aRect;
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        if ( bDrawSel )
            DrawRect( aRect );
        if ( mbDoubleSel )
        {
            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;
            if ( bDrawSel )
                DrawRect( aRect );
        }

        if ( bDrawSel )
        {
            if ( mbBlackSel )
                SetLineColor( Color( COL_WHITE ) );
            else
                SetLineColor( rStyleSettings.GetHighlightTextColor() );
        }
        else
            SetLineColor( Color( COL_LIGHTGRAY ) );
        DrawRect( aRect2 );

        if ( bFocus )
            ShowFocus( aRect2 );
    }

    ImplDrawItemText( pItem->maText );
}

// -----------------------------------------------------------------------

void ValueSet::ImplHideSelect( USHORT nItemId )
{
    Rectangle aRect;

    if ( nItemId )
        aRect = mpItemList->GetObject( GetItemPos( nItemId ) )->maRect;
    else
    {
        if ( mpNoneItem )
            aRect = mpNoneItem->maRect;
    }

    if ( !aRect.IsEmpty() )
    {
        HideFocus();
        Point aPos  = aRect.TopLeft();
        Size  aSize = aRect.GetSize();
        DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplHighlightItem( USHORT nItemId, BOOL bIsSelection )
{
    if ( mnHighItemId != nItemId )
    {
        // Alten merken, um vorherige Selektion zu entfernen
        USHORT nOldItem = mnHighItemId;
        mnHighItemId = nItemId;

        // Wenn keiner selektiert ist, dann Selektion nicht malen
        if ( !bIsSelection && mbNoSelection )
            mbDrawSelection = FALSE;

        // Neu ausgeben und alte Selection wegnehmen
        ImplHideSelect( nOldItem );
        ImplDrawSelect();
        mbDrawSelection = TRUE;
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawDropPos( BOOL bShow )
{
    if ( (mnDropPos != VALUESET_ITEM_NOTFOUND) && mpItemList->Count() )
    {
        USHORT  nItemPos = mnDropPos;
        USHORT  nItemId1;
        USHORT  nItemId2 = 0;
        BOOL    bRight;
        if ( nItemPos >= mpItemList->Count() )
        {
            nItemPos = (USHORT)(mpItemList->Count()-1);
            bRight = TRUE;
        }
        else
            bRight = FALSE;

        nItemId1 = GetItemId( nItemPos );
        if ( (nItemId1 != mnSelItemId) && (nItemId1 != mnHighItemId) )
            nItemId1 = 0;
        Rectangle aRect2 = mpItemList->GetObject( nItemPos )->maRect;
        Rectangle aRect1;
        if ( bRight )
        {
            aRect1 = aRect2;
            aRect2.SetEmpty();
        }
        else if ( nItemPos > 0 )
        {
            aRect1 = mpItemList->GetObject( nItemPos-1 )->maRect;
            nItemId2 = GetItemId( nItemPos-1 );
            if ( (nItemId2 != mnSelItemId) && (nItemId2 != mnHighItemId) )
                nItemId2 = 0;
        }

        // Items ueberhaupt sichtbar (nur Erstes/Letztes)
        if ( !aRect1.IsEmpty() || !aRect2.IsEmpty() )
        {
            if ( nItemId1 )
                ImplHideSelect( nItemId1 );
            if ( nItemId2 )
                ImplHideSelect( nItemId2 );

            if ( bShow )
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                long    nX;
                long    nY;
                SetLineColor( rStyleSettings.GetButtonTextColor() );
                if ( !aRect1.IsEmpty() )
                {
                    Point aPos = aRect1.RightCenter();
                    nX = aPos.X()-2;
                    nY = aPos.Y();
                    for ( USHORT i = 0; i < 4; i++ )
                        DrawLine( Point( nX-i, nY-i ), Point( nX-i, nY+i ) );
                }
                if ( !aRect2.IsEmpty() )
                {
                    Point aPos = aRect2.LeftCenter();
                    nX = aPos.X()+2;
                    nY = aPos.Y();
                    for ( USHORT i = 0; i < 4; i++ )
                        DrawLine( Point( nX+i, nY-i ), Point( nX+i, nY+i ) );
                }
            }
            else
            {
                if ( !aRect1.IsEmpty() )
                {
                    Point aPos  = aRect1.TopLeft();
                    Size  aSize = aRect1.GetSize();
                    DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
                }
                if ( !aRect2.IsEmpty() )
                {
                    Point aPos  = aRect2.TopLeft();
                    Size  aSize = aRect2.GetSize();
                    DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
                }
            }

            if ( nItemId1 || nItemId2 )
                ImplDrawSelect();
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplDraw()
{
    if ( mbFormat )
        Format();

    HideFocus();

    Point   aDefPos;
    Size    aSize = maVirDev.GetOutputSizePixel();

    if ( mpScrBar && mpScrBar->IsVisible() )
    {
        Point   aScrPos = mpScrBar->GetPosPixel();
        Size    aScrSize = mpScrBar->GetSizePixel();
        Point   aTempPos( 0, aScrPos.Y() );
        Size    aTempSize( aSize.Width(), aScrPos.Y() );

        DrawOutDev( aDefPos, aTempSize, aDefPos, aTempSize, maVirDev );
        aTempSize.Width()   = aScrPos.X()-1;
        aTempSize.Height()  = aScrSize.Height();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
        aTempPos.Y()        = aScrPos.Y()+aScrSize.Height();
        aTempSize.Width()   = aSize.Width();
        aTempSize.Height()  = aSize.Height()-aTempPos.Y();
        DrawOutDev( aTempPos, aTempSize, aTempPos, aTempSize, maVirDev );
    }
    else
        DrawOutDev( aDefPos, aSize, aDefPos, aSize, maVirDev );

    // Trennlinie zum Namefield zeichnen
    if ( GetStyle() & WB_NAMEFIELD )
    {
        if ( !(GetStyle() & WB_FLATVALUESET) )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            Size aWinSize = GetOutputSizePixel();
            Point aPos1( NAME_LINE_OFF_X, mnTextOffset+NAME_LINE_OFF_Y );
            Point aPos2( aWinSize.Width()-(NAME_LINE_OFF_X*2), mnTextOffset+NAME_LINE_OFF_Y );
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( aPos1, aPos2 );
                aPos1.Y()++;
                aPos2.Y()++;
                SetLineColor( rStyleSettings.GetLightColor() );
            }
            else
                SetLineColor( rStyleSettings.GetWindowTextColor() );
            DrawLine( aPos1, aPos2 );
        }
    }

    ImplDrawSelect();
}

// -----------------------------------------------------------------------

BOOL ValueSet::ImplScroll( const Point& rPos )
{
    Size aOutSize = GetOutputSizePixel();
    long nScrBarWidth;

    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width();
    else
        nScrBarWidth = 0;

    if ( !mbScroll || (rPos.X() < 0) || (rPos.X() > aOutSize.Width()-nScrBarWidth) )
        return FALSE;

    long             nScrollOffset;
    USHORT           nOldLine = mnFirstLine;
    const Rectangle& rTopRect = mpItemList->GetObject( mnFirstLine*mnCols )->maRect;
    if ( rTopRect.GetHeight() <= 16 )
        nScrollOffset = VALUESET_SCROLL_OFFSET/2;
    else
        nScrollOffset = VALUESET_SCROLL_OFFSET;
    if ( (mnFirstLine > 0) && (rPos.Y() >= 0) )
    {
        long nTopPos = rTopRect.Top();
        if ( (rPos.Y() >= nTopPos) && (rPos.Y() <= nTopPos+nScrollOffset) )
            mnFirstLine--;
    }
    if ( (mnFirstLine == nOldLine) &&
         (mnFirstLine < (USHORT)(mnLines-mnVisLines)) && (rPos.Y() < aOutSize.Height()) )
    {
        long nBottomPos = mpItemList->GetObject( (mnFirstLine+mnVisLines-1)*mnCols )->maRect.Bottom();
        if ( (rPos.Y() >= nBottomPos-nScrollOffset) && (rPos.Y() <= nBottomPos) )
            mnFirstLine++;
    }

    if ( mnFirstLine != nOldLine )
    {
        mbFormat = TRUE;
        ImplDraw();
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

USHORT ValueSet::ImplGetItem( const Point& rPos, BOOL bMove ) const
{
    if ( mpNoneItem )
    {
        if ( mpNoneItem->maRect.IsInside( rPos ) )
            return VALUESET_ITEM_NONEITEM;
    }

    Point     aDefPos;
    Rectangle aWinRect( aDefPos, maVirDev.GetOutputSizePixel() );

    ULONG nItemCount = mpItemList->Count();
    for ( ULONG i = 0; i < nItemCount; i++ )
    {
        ValueSetItem* pItem = mpItemList->GetObject( i );
        if ( pItem->maRect.IsInside( rPos ) )
        {
            if ( aWinRect.IsInside( rPos ) )
                return (USHORT)i;
            else
                return VALUESET_ITEM_NOTFOUND;
        }
    }

    // Wenn Spacing gesetzt ist, wird der vorher selektierte
    // Eintrag zurueckgegeben, wenn die Maus noch nicht das Fenster
    // verlassen hat
    if ( bMove && mnSpacing && mnHighItemId )
    {
        if ( aWinRect.IsInside( rPos ) )
            return GetItemPos( mnHighItemId );
    }

    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetItem( USHORT nPos )
{
    if ( nPos == VALUESET_ITEM_NONEITEM )
        return mpNoneItem;
    else
        return mpItemList->GetObject( nPos );
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetFirstItem()
{
    USHORT nItemCount = (USHORT)mpItemList->Count();
    USHORT i = 0;

    while ( i < nItemCount )
    {
        ValueSetItem* pItem = mpItemList->GetObject( i );
        if ( pItem->meType != VALUESETITEM_SPACE )
            return pItem;
        i++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

IMPL_LINK( ValueSet,ImplScrollHdl, ScrollBar*, pScrollBar )
{
    USHORT nNewFirstLine = (USHORT)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = TRUE;
        ImplDraw();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ValueSet,ImplTimerHdl, Timer*, pTimer )
{
    ImplTracking( GetPointerPosPixel(), TRUE );
    return 0;
}

// -----------------------------------------------------------------------

void ValueSet::ImplTracking( const Point& rPos, BOOL bRepeat )
{
    if ( bRepeat || mbSelection )
    {
        if ( ImplScroll( rPos ) )
        {
            if ( mbSelection )
            {
                maTimer.SetTimeoutHdl( LINK( this, ValueSet, ImplTimerHdl ) );
                maTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
                maTimer.Start();
            }
        }
    }

    ValueSetItem* pItem = ImplGetItem( ImplGetItem( rPos ) );
    if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
        ImplHighlightItem( pItem->mnId );
    else
        ImplHighlightItem( mnSelItemId, FALSE );
}

// -----------------------------------------------------------------------

void ValueSet::ImplEndTracking( const Point& rPos, BOOL bCancel )
{
    ValueSetItem* pItem;

    // Bei Abbruch, den alten Status wieder herstellen
    if ( bCancel )
        pItem = NULL;
    else
        pItem = ImplGetItem( ImplGetItem( rPos ) );

    if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
    {
        SelectItem( pItem->mnId );
        if ( !mbSelection && !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        mbHighlight = FALSE;
        mbSelection = FALSE;
        Select();
    }
    else
    {
        ImplHighlightItem( mnSelItemId, FALSE );
        mbHighlight = FALSE;
        mbSelection = FALSE;
    }
}

// -----------------------------------------------------------------------

void ValueSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        ValueSetItem* pItem = ImplGetItem( ImplGetItem( rMEvt.GetPosPixel() ) );
        if ( mbSelection )
        {
            mbHighlight = TRUE;
            if ( pItem && (pItem->meType != VALUESETITEM_SPACE) )
            {
                mnOldItemId  = mnSelItemId;
                mnHighItemId = mnSelItemId;
                ImplHighlightItem( pItem->mnId );
            }

            return;
        }
        else
        {
            if ( pItem && (pItem->meType != VALUESETITEM_SPACE) && !rMEvt.IsMod2() )
            {
                if ( (pItem->mnBits & VIB_NODOUBLECLICK) || (rMEvt.GetClicks() == 1) )
                {
                    mnOldItemId  = mnSelItemId;
                    mbHighlight  = TRUE;
                    mnHighItemId = mnSelItemId;
                    ImplHighlightItem( pItem->mnId );
                    StartTracking( STARTTRACK_SCROLLREPEAT );
                }
                else if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();

                return;
            }
        }
    }

    Control::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    // Wegen SelectionMode
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( rMEvt.GetPosPixel(), FALSE );
    else
        Control::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::MouseMove( const MouseEvent& rMEvt )
{
    // Wegen SelectionMode
    if ( mbSelection )
        ImplTracking( rMEvt.GetPosPixel(), FALSE );
    Control::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

// -----------------------------------------------------------------------

void ValueSet::KeyInput( const KeyEvent& rKEvt )
{
    USHORT nLastItem = (USHORT)mpItemList->Count();
    USHORT nItemPos = VALUESET_ITEM_NOTFOUND;
    USHORT nCurPos;
    USHORT nCalcPos;

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKEvt );
        return;
    }
    else
        nLastItem--;

    if ( mnSelItemId )
        nCurPos = GetItemPos( mnSelItemId );
    else
        nCurPos = VALUESET_ITEM_NONEITEM;
    nCalcPos = nCurPos;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            if ( mpNoneItem )
                nItemPos = VALUESET_ITEM_NONEITEM;
            else
            {
                nItemPos = 0;
                while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE )
                    nItemPos++;
            }
            break;

        case KEY_END:
            nItemPos = nLastItem;
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE )
            {
                if ( nItemPos == 0 )
                    nItemPos = VALUESET_ITEM_NONEITEM;
                else
                    nItemPos--;
            }
            break;

        case KEY_LEFT:
            do
            {
                if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                    nItemPos = nLastItem;
                else if ( !nCalcPos )
                {
                    if ( mpNoneItem )
                        nItemPos = VALUESET_ITEM_NONEITEM;
                    else
                        nItemPos = nLastItem;
                }
                else
                    nItemPos = nCalcPos-1;
                nCalcPos = nItemPos;
            }
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            break;

        case KEY_RIGHT:
            do
            {
                if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                    nItemPos = 0;
                else if ( nCalcPos == nLastItem )
                {
                    if ( mpNoneItem )
                        nItemPos = VALUESET_ITEM_NONEITEM;
                    else
                        nItemPos = 0;
                }
                else
                    nItemPos = nCalcPos+1;
                nCalcPos = nItemPos;
            }
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            break;

        case KEY_UP:
            do
            {
                if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                {
                    if ( nLastItem+1 <= mnCols )
                        nItemPos = mnCurCol;
                    else
                    {
                        nItemPos = ((((nLastItem+1)/mnCols)-1)*mnCols)+mnCurCol;
                        if ( nItemPos+mnCols <= nLastItem )
                            nItemPos += mnCols;
                    }
                }
                else if ( nCalcPos >= mnCols )
                    nItemPos = nCalcPos-mnCols;
                else
                {
                    if ( mpNoneItem )
                    {
                        mnCurCol  = nCalcPos%mnCols;
                        nItemPos = VALUESET_ITEM_NONEITEM;
                    }
                    else
                    {
                        if ( nLastItem+1 <= mnCols )
                            nItemPos = nCalcPos;
                        else
                        {
                            nItemPos = ((((nLastItem+1)/mnCols)-1)*mnCols)+nCalcPos;
                            if ( nItemPos+mnCols <= nLastItem )
                                nItemPos += mnCols;
                        }
                    }
                }
                nCalcPos = nItemPos;
            }
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            break;

        case KEY_DOWN:
            do
            {
                if ( nCalcPos == VALUESET_ITEM_NONEITEM )
                    nItemPos = mnCurCol;
                else if ( nCalcPos+mnCols <= nLastItem )
                    nItemPos = nCalcPos+mnCols;
                else
                {
                    if ( mpNoneItem )
                    {
                        mnCurCol  = nCalcPos%mnCols;
                        nItemPos = VALUESET_ITEM_NONEITEM;
                    }
                    else
                        nItemPos = nCalcPos%mnCols;
                }
                nCalcPos = nItemPos;
            }
            while ( ImplGetItem( nItemPos )->meType == VALUESETITEM_SPACE );
            break;

        default:
            Control::KeyInput( rKEvt );
            break;
    }

    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
        USHORT nItemId;
        if ( nItemPos != VALUESET_ITEM_NONEITEM )
            nItemId = GetItemId( nItemPos );
        else
            nItemId = 0;
        if ( nItemId != mnSelItemId )
        {
            SelectItem( nItemId );
            Select();
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::Command( const CommandEvent& rCEvt )
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

// -----------------------------------------------------------------------

void ValueSet::Paint( const Rectangle& rRect )
{
    if ( GetStyle() & WB_FLATVALUESET )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor();
        SetFillColor( rStyleSettings.GetFaceColor() );
        long nOffY = maVirDev.GetOutputSizePixel().Height();
        Size aWinSize = GetOutputSizePixel();
        DrawRect( Rectangle( Point( 0, nOffY ), Point( aWinSize.Width(), aWinSize.Height() ) ) );
    }

    ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::GetFocus()
{
    ImplDrawSelect();
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void ValueSet::LoseFocus()
{
    if ( mbNoSelection && mnSelItemId )
        ImplHideSelect( mnSelItemId );
    else
        HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void ValueSet::Resize()
{
    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    Control::Resize();
}

// -----------------------------------------------------------------------

void ValueSet::RequestHelp( const HelpEvent& rHEvt )
{
    if ( (rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON)) == HELPMODE_QUICK )
    {
        Point aPos = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        USHORT nItemPos = ImplGetItem( aPos );
        if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            ValueSetItem* pItem = ImplGetItem( nItemPos );
            Rectangle aItemRect = pItem->maRect;
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            Help::ShowQuickHelp( this, aItemRect, GetItemText( pItem->mnId ) );
            return;
        }
    }

    Control::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void ValueSet::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( mbFormat )
            Format();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
        if ( mpNoneItem && !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            ImplFormatItem( mpNoneItem );
            Invalidate( mpNoneItem->maRect );
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
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
    else if ( nType == STATE_CHANGE_STYLE )
    {
        mbFormat = TRUE;
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        mbFormat = TRUE;
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void ValueSet::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ValueSet::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( USHORT nItemId, const Image& rImage, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( USHORT nItemId, const Color& rColor, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( USHORT nItemId, const Image& rImage,
                           const XubString& rText, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( USHORT nItemId, const Color& rColor,
                           const XubString& rText, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( USHORT nItemId, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::InsertSpace( USHORT nItemId, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ValueSet::InsertSpace(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertSpace(): ItemId already exists" );

    ValueSetItem* pItem = new ValueSetItem;
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_SPACE;
    mpItemList->Insert( pItem, (ULONG)nPos );

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::RemoveItem( USHORT nItemId )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    delete mpItemList->Remove( nPos );

    // Variablen zuruecksetzen
    if ( (mnHighItemId == nItemId) || (mnSelItemId == nItemId) )
    {
        mnCurCol        = 0;
        mnOldItemId     = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = TRUE;
    }

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::CopyItems( const ValueSet& rValueSet )
{
    ValueSetItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    pItem = rValueSet.mpItemList->First();
    while ( pItem )
    {
        mpItemList->Insert( new ValueSetItem( *pItem ) );
        pItem = rValueSet.mpItemList->Next();
    }

    // Variablen zuruecksetzen
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnOldItemId     = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = TRUE;

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::Clear()
{
    ValueSetItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }
    mpItemList->Clear();

    // Variablen zuruecksetzen
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnOldItemId     = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = TRUE;

    mbFormat = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

USHORT ValueSet::GetItemCount() const
{
    return (USHORT)mpItemList->Count();
}

// -----------------------------------------------------------------------

USHORT ValueSet::GetItemPos( USHORT nItemId ) const
{
    ValueSetItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nItemId )
            return (USHORT)mpItemList->GetCurPos();
        pItem = mpItemList->Next();
    }

    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ValueSet::GetItemId( USHORT nPos ) const
{
    ValueSetItem* pItem = mpItemList->GetObject( nPos );

    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT ValueSet::GetItemId( const Point& rPos ) const
{
    USHORT nItemPos = ImplGetItem( rPos );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

// -----------------------------------------------------------------------

Rectangle ValueSet::GetItemRect( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

void ValueSet::SetColCount( USHORT nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetLineCount( USHORT nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetItemWidth( long nNewItemWidth )
{
    if ( mnUserItemWidth != nNewItemWidth )
    {
        mnUserItemWidth = nNewItemWidth;
        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetItemHeight( long nNewItemHeight )
{
    if ( mnUserItemHeight != nNewItemHeight )
    {
        mnUserItemHeight = nNewItemHeight;
        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetFirstLine( USHORT nNewLine )
{
    if ( mnFirstLine != nNewLine )
    {
        mnFirstLine = nNewLine;
        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SelectItem( USHORT nItemId )
{
    USHORT nPos;

    if ( nItemId )
    {
        nPos = GetItemPos( nItemId );
        if ( nPos == VALUESET_ITEM_NOTFOUND )
            return;
        if ( mpItemList->GetObject( nPos )->meType == VALUESETITEM_SPACE )
            return;
    }

    if ( (mnSelItemId != nItemId) || mbNoSelection )
    {
        USHORT nOldItem = mnSelItemId;
        mnSelItemId = nItemId;
        mbNoSelection = FALSE;

        BOOL bNewOut;
        BOOL bNewLine;
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
            bNewOut = TRUE;
        else
            bNewOut = FALSE;
        bNewLine = FALSE;

        // Gegebenenfalls in den sichtbaren Bereich scrollen
        if ( mbScroll && nItemId )
        {
            USHORT nNewLine = (USHORT)(nPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
                bNewLine = TRUE;
            }
            else if ( nNewLine > (USHORT)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (USHORT)(nNewLine-mnVisLines+1);
                bNewLine = TRUE;
            }
        }

        if ( bNewOut )
        {
            if ( bNewLine )
            {
                // Falls sich der sichtbare Bereich geaendert hat,
                // alles neu ausgeben
                mbFormat = TRUE;
                ImplDraw();
            }
            else
            {
                // alte Selection wegnehmen und neue ausgeben
                ImplHideSelect( nOldItem );
                ImplDrawSelect();
            }
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetNoSelection()
{
    mbNoSelection   = TRUE;
    mbHighlight     = FALSE;
    mbSelection     = FALSE;

    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemBits( USHORT nItemId, USHORT nItemBits )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        mpItemList->GetObject( nPos )->mnBits = nItemBits;
}

// -----------------------------------------------------------------------

USHORT ValueSet::GetItemBits( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ValueSet::SetItemImage( USHORT nItemId, const Image& rImage )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mpItemList->GetObject( nPos );
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        ImplFormatItem( pItem );
        Invalidate( pItem->maRect );
    }
    else
        mbFormat = TRUE;
}

// -----------------------------------------------------------------------

Image ValueSet::GetItemImage( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemColor( USHORT nItemId, const Color& rColor )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mpItemList->GetObject( nPos );
    pItem->meType  = VALUESETITEM_COLOR;
    pItem->maColor = rColor;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        ImplFormatItem( pItem );
        Invalidate( pItem->maRect );
    }
    else
        mbFormat = TRUE;
}

// -----------------------------------------------------------------------

Color ValueSet::GetItemColor( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maColor;
    else
        return Color();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemData( USHORT nItemId, void* pData )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mpItemList->GetObject( nPos );
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            ImplFormatItem( pItem );
            Invalidate( pItem->maRect );
        }
        else
            mbFormat = TRUE;
    }
}

// -----------------------------------------------------------------------

void* ValueSet::GetItemData( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mpData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ValueSet::SetItemText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mpItemList->GetObject( nPos );
    pItem->maText = rText;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        USHORT nTempId = mnSelItemId;

        if ( mbHighlight )
            nTempId = mnHighItemId;

        if ( nTempId == nItemId )
            ImplDrawItemText( pItem->maText );
    }
}

// -----------------------------------------------------------------------

XubString ValueSet::GetItemText( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maText;
    else
        return XubString();
}

// -----------------------------------------------------------------------

void ValueSet::SetColor( const Color& rColor )
{
    maColor     = rColor;
    mbFormat    = TRUE;
    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetExtraSpacing( USHORT nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;

        mbFormat = TRUE;
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::StartSelection()
{
    mnOldItemId     = mnSelItemId;
    mbHighlight     = TRUE;
    mbSelection     = TRUE;
    mnHighItemId    = mnSelItemId;
}

// -----------------------------------------------------------------------

void ValueSet::EndSelection()
{
    if ( mbHighlight )
    {
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );

        ImplHighlightItem( mnSelItemId );
        mbHighlight = FALSE;
    }
    mbSelection = FALSE;
}

// -----------------------------------------------------------------------

BOOL ValueSet::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return FALSE;

    // Gegebenenfalls eine vorhandene Aktion abbrechen
    EndSelection();

    // Testen, ob angeklickte Seite selektiert ist. Falls dies nicht
    // der Fall ist, setzen wir ihn als aktuellen Eintrag. Falls Drag and
    // Drop auch mal ueber Tastatur ausgeloest werden kann, testen wir
    // dies nur bei einer Mausaktion.
    USHORT nSelId;
    if ( rCEvt.IsMouseEvent() )
        nSelId = GetItemId( rCEvt.GetMousePosPixel() );
    else
        nSelId = mnSelItemId;

    // Falls kein Eintrag angeklickt wurde, starten wir kein Dragging
    if ( !nSelId )
        return FALSE;

    // Testen, ob Seite selektiertiert ist. Falls nicht, als aktuelle
    // Seite setzen und Select rufen.
    if ( nSelId != mnSelItemId )
    {
        SelectItem( nSelId );
        Update();
        Select();
    }

#ifdef MAC
    Region aRegion( GetItemRect( nSelId ) );
#else
    Region aRegion;
#endif

    // Region zuweisen
    rRegion = aRegion;

    return TRUE;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcWindowSizePixel( const Size& rItemSize, USHORT nDesireCols,
                                    USHORT nDesireLines )
{
    long nCalcCols = (long)nDesireCols;
    long nCalcLines = (long)nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = (long)mnUserCols;
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
                nCalcLines = (long)mpItemList->Count() / nCalcCols;
                if ( mpItemList->Count() % nCalcCols )
                    nCalcLines++;
                else if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width()*nCalcCols, rItemSize.Height()*nCalcLines );
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        nSpace;
    long        n;

    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            n = ITEM_OFFSET_DOUBLE;
        else
            n = ITEM_OFFSET;

        aSize.Width()  += n*nCalcCols;
        aSize.Height() += n*nCalcLines;
    }
    else
        n = 0;

    if ( mnSpacing )
    {
        nSpace = mnSpacing;
        aSize.Width()  += mnSpacing*(nCalcCols-1);
        aSize.Height() += mnSpacing*(nCalcLines-1);
    }
    else
        nSpace = 0;

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.Height() += nTxtHeight + NAME_OFFSET;
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.Height() += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.Height() += nTxtHeight + n + nSpace;
        if ( nStyle & WB_RADIOSEL )
            aSize.Height() += 8;
    }

    // Evt. ScrollBar-Breite aufaddieren
    aSize.Width() += GetScrollWidth();

    return aSize;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcItemSizePixel( const Size& rItemSize, BOOL bOut ) const
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

        if ( bOut )
        {
            aSize.Width()  += n;
            aSize.Height() += n;
        }
        else
        {
            aSize.Width()  -= n;
            aSize.Height() -= n;
        }
    }

    return aSize;
}

// -----------------------------------------------------------------------

long ValueSet::GetScrollWidth() const
{
    if ( GetStyle() & WB_VSCROLL )
    {
        ((ValueSet*)this)->ImplInitScrollBar();
        return mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT ValueSet::ShowDropPos( const Point& rPos )
{
    mbDropPos = TRUE;

    // Gegebenenfalls scrollen
    ImplScroll( rPos );

    // DropPosition ermitteln
    USHORT nPos = ImplGetItem( rPos, TRUE );
    if ( nPos == VALUESET_ITEM_NONEITEM )
        nPos = 0;
    else if ( nPos == VALUESET_ITEM_NOTFOUND )
    {
        Size aOutSize = GetOutputSizePixel();
        if ( GetStyle() & WB_NAMEFIELD )
            aOutSize.Height() = mnTextOffset;
        if ( (rPos.X() >= 0) && (rPos.X() < aOutSize.Width()) &&
             (rPos.Y() >= 0) && (rPos.Y() < aOutSize.Height()) )
            nPos = (USHORT)mpItemList->Count();
    }
    else
    {
        // Im letzten viertel, dann wird ein Item spaeter eingefuegt
        Rectangle aRect = mpItemList->GetObject( nPos )->maRect;
        if ( rPos.X() > aRect.Left()+aRect.GetWidth()-(aRect.GetWidth()/4) )
            nPos++;
    }

    if ( nPos != mnDropPos )
    {
        ImplDrawDropPos( FALSE );
        mnDropPos = nPos;
        ImplDrawDropPos( TRUE );
    }

    return mnDropPos;
}

// -----------------------------------------------------------------------

void ValueSet::HideDropPos()
{
    if ( mbDropPos )
    {
        ImplDrawDropPos( FALSE );
        mbDropPos = FALSE;
    }
}
