/*************************************************************************
 *
 *  $RCSfile: status.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_STATUS_CXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_STATUS_HXX
#include <status.hxx>
#endif

#pragma hdrstop

// =======================================================================

#define STATUSBAR_OFFSET_X      STATUSBAR_OFFSET
#define STATUSBAR_OFFSET_Y      2
#define STATUSBAR_OFFSET_TEXTY  3

#define STATUSBAR_PRGS_OFFSET   3
#define STATUSBAR_PRGS_COUNT    100
#define STATUSBAR_PRGS_MIN      5

// -----------------------------------------------------------------------

struct ImplStatusItem
{
    USHORT              mnId;
    StatusBarItemBits   mnBits;
    long                mnWidth;
    long                mnOffset;
    long                mnExtraWidth;
    long                mnX;
    XubString           maText;
    XubString           maHelpText;
    ULONG               mnHelpId;
    void*               mpUserData;
    BOOL                mbVisible;
};

DECLARE_LIST( ImplStatusItemList, ImplStatusItem* );

// =======================================================================

inline long ImplCalcProgessWidth( USHORT nMax, long nSize )
{
    return ((nMax*(nSize+(nSize/2)))-(nSize/2)+(STATUSBAR_PRGS_OFFSET*2));
}

// -----------------------------------------------------------------------

static Point ImplGetItemTextPos( const Size& rRectSize, const Size& rTextSize,
                                 USHORT nStyle )
{
    long nX;
    long nY;

    if ( nStyle & SIB_LEFT )
        nX = 0;
    else if ( nStyle & SIB_RIGHT )
        nX = rRectSize.Width()-rTextSize.Width();
    else // SIB_CENTER
        nX = (rRectSize.Width()-rTextSize.Width())/2;
    nY = (rRectSize.Height()-rTextSize.Height())/2 + 1;
    return Point( nX, nY );
}

// -----------------------------------------------------------------------

BOOL StatusBar::ImplIsItemUpdate()
{
    if ( !mbProgressMode && mbVisibleItems && IsReallyVisible() && IsUpdateMode() )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void StatusBar::ImplInit( Window* pParent, WinBits nStyle )
{
    // Default ist RightAlign
    if ( !(nStyle & (WB_LEFT | WB_RIGHT)) )
        nStyle |= WB_RIGHT;

    Window::ImplInit( pParent, nStyle & ~WB_BORDER, NULL );

    // WinBits merken
    mpItemList      = new ImplStatusItemList;
    mpVirDev        = new VirtualDevice( *this );
    mnCurItemId     = 0;
    mbFormat        = TRUE;
    mbVisibleItems  = TRUE;
    mbProgressMode  = FALSE;
    mbInUserDraw    = FALSE;
    mbBottomBorder  = FALSE;
    mnDX            = 0;
    mnDY            = 0;
    mnCalcHeight    = 0;
    mnItemY         = STATUSBAR_OFFSET_Y;
    mnTextY         = STATUSBAR_OFFSET_TEXTY;

    ImplInitSettings( TRUE, TRUE, TRUE );
    SetLineColor();

    SetSizePixel( CalcWindowSizePixel() );
}

// -----------------------------------------------------------------------

StatusBar::StatusBar( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_STATUSBAR )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

StatusBar::StatusBar( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_STATUSBAR )
{
    rResId.SetRT( RSC_STATUSBAR );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

StatusBar::~StatusBar()
{
    // Alle Items loeschen
    ImplStatusItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    delete mpItemList;

    // VirtualDevice loeschen
    delete mpVirDev;
}

// -----------------------------------------------------------------------

void StatusBar::ImplInitSettings( BOOL bFont,
                                  BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetToolFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else if ( GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetButtonTextColor();
        else
            aColor = rStyleSettings.GetWindowTextColor();
        SetTextColor( aColor );
        SetTextFillColor();

        mpVirDev->SetFont( GetFont() );
        mpVirDev->SetTextColor( GetTextColor() );
        mpVirDev->SetTextAlign( GetTextAlign() );
        mpVirDev->SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
        mpVirDev->SetBackground( GetBackground() );
    }
}

// -----------------------------------------------------------------------

void StatusBar::ImplFormat()
{
    ImplStatusItem* pItem;
    long            nExtraWidth;
    long            nExtraWidth2;
    long            nX;
    USHORT          nAutoSizeItems = 0;

    // Breiten zusammenrechnen
    mnItemsWidth = STATUSBAR_OFFSET_X;
    long nOffset = 0;
    pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbVisible )
        {
            if ( pItem->mnBits & SIB_AUTOSIZE )
                nAutoSizeItems++;

            mnItemsWidth += pItem->mnWidth + nOffset;
            nOffset = pItem->mnOffset;
        }

        pItem = mpItemList->Next();
    }

    if ( GetStyle() & WB_RIGHT )
    {
        // Bei rechtsbuendiger Ausrichtung wird kein AutoSize ausgewertet,
        // da wir links den Text anzeigen, der mit SetText gesetzt wird
        nX              = mnDX - mnItemsWidth;
        nExtraWidth     = 0;
        nExtraWidth2    = 0;
    }
    else
    {
        mnItemsWidth += STATUSBAR_OFFSET_X;

        // Bei linksbuendiger Ausrichtung muessen wir gegebenenfalls noch
        // AutoSize auswerten
        if ( nAutoSizeItems && (mnDX > (mnItemsWidth - STATUSBAR_OFFSET)) )
        {
            nExtraWidth  = (mnDX - mnItemsWidth - 1) / nAutoSizeItems;
            nExtraWidth2 = (mnDX - mnItemsWidth - 1) % nAutoSizeItems;
        }
        else
        {
            nExtraWidth  = 0;
            nExtraWidth2 = 0;
        }
        nX = STATUSBAR_OFFSET_X;
    }

    pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbVisible )
        {
            if ( pItem->mnBits & SIB_AUTOSIZE )
            {
                pItem->mnExtraWidth = nExtraWidth;
                if ( nExtraWidth2 )
                {
                    pItem->mnExtraWidth++;
                    nExtraWidth2--;
                }
            }
            else
                pItem->mnExtraWidth = 0;

            pItem->mnX = nX;
            nX += pItem->mnWidth + pItem->mnExtraWidth + pItem->mnOffset;
        }

        pItem = mpItemList->Next();
    }

    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

Rectangle StatusBar::ImplGetItemRectPos( USHORT nPos ) const
{
    Rectangle       aRect;
    ImplStatusItem* pItem;

    pItem = mpItemList->GetObject( nPos );

    if ( pItem )
    {
        if ( pItem->mbVisible )
        {
            aRect.Left()   = pItem->mnX;
            aRect.Right()  = aRect.Left() + pItem->mnWidth + pItem->mnExtraWidth;
            aRect.Top()    = mnItemY;
            aRect.Bottom() = mnCalcHeight - STATUSBAR_OFFSET_Y;
        }
    }

    return aRect;
}

// -----------------------------------------------------------------------

void StatusBar::ImplDrawText( BOOL bOffScreen, long nOldTextWidth )
{
    // Das ueberschreiben der Item-Box verhindern
    Rectangle aTextRect;
    aTextRect.Left() = STATUSBAR_OFFSET_X+1;
    aTextRect.Top()  = mnTextY;
    if ( mbVisibleItems && (GetStyle() & WB_RIGHT) )
        aTextRect.Right() = mnDX - mnItemsWidth - 1;
    else
        aTextRect.Right() = mnDX - 1;
    if ( aTextRect.Right() > aTextRect.Left() )
    {
        // Position ermitteln
        XubString aStr = GetText();
        USHORT nPos = aStr.Search( _LF );
        if ( nPos != STRING_NOTFOUND )
            aStr.Erase( nPos );

        aTextRect.Bottom() = aTextRect.Top()+GetTextHeight()+1;

        if ( bOffScreen )
        {
            long nMaxWidth = Max( nOldTextWidth, GetTextWidth( aStr ) );
            Size aVirDevSize( nMaxWidth, aTextRect.GetHeight() );
            mpVirDev->SetOutputSizePixel( aVirDevSize );
            Rectangle aTempRect = aTextRect;
            aTempRect.SetPos( Point( 0, 0 ) );
            mpVirDev->DrawText( aTempRect, aStr, TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
            DrawOutDev( aTextRect.TopLeft(), aVirDevSize, Point(), aVirDevSize, *mpVirDev );
        }
        else
            DrawText( aTextRect, aStr, TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
    }
}

// -----------------------------------------------------------------------

void StatusBar::ImplDrawItem( BOOL bOffScreen, USHORT nPos, BOOL bDrawText, BOOL bDrawFrame )
{
    Rectangle aRect = ImplGetItemRectPos( nPos );

    if ( aRect.IsEmpty() )
        return;

    // Ausgabebereich berechnen
    ImplStatusItem*     pItem = mpItemList->GetObject( nPos );
    Rectangle           aTextRect( aRect.Left()+1, aRect.Top()+1,
                                   aRect.Right()-1, aRect.Bottom()-1 );
    Size                aTextRectSize( aTextRect.GetSize() );

    if ( bOffScreen )
        mpVirDev->SetOutputSizePixel( aTextRectSize );
    else
    {
        Region aRegion( aTextRect );
        SetClipRegion( aRegion );
    }

    // Text ausgeben
    if ( bDrawText )
    {
        Size    aTextSize( GetTextWidth( pItem->maText ), GetTextHeight() );
        Point   aTextPos = ImplGetItemTextPos( aTextRectSize, aTextSize, pItem->mnBits );
        if ( bOffScreen )
            mpVirDev->DrawText( aTextPos, pItem->maText );
        else
        {
            aTextPos.X() += aTextRect.Left();
            aTextPos.Y() += aTextRect.Top();
            DrawText( aTextPos, pItem->maText );
        }
    }

    // Gegebenenfalls auch DrawItem aufrufen
    if ( pItem->mnBits & SIB_USERDRAW )
    {
        if ( bOffScreen )
        {
            mbInUserDraw = TRUE;
            UserDrawEvent aODEvt( mpVirDev, Rectangle( Point(), aTextRectSize ), pItem->mnId );
            UserDraw( aODEvt );
            mbInUserDraw = FALSE;
        }
        else
        {
            UserDrawEvent aODEvt( this, aTextRect, pItem->mnId );
            UserDraw( aODEvt );
        }
    }

    if ( bOffScreen )
        DrawOutDev( aTextRect.TopLeft(), aTextRectSize, Point(), aTextRectSize, *mpVirDev );
    else
        SetClipRegion();

    // Frame ausgeben
    if ( bDrawFrame && !(pItem->mnBits & SIB_FLAT) )
    {
        USHORT nStyle;

        if ( pItem->mnBits & SIB_IN )
            nStyle = FRAME_DRAW_IN;
        else
            nStyle = FRAME_DRAW_OUT;

        DecorationView aDecoView( this );
        aDecoView.DrawFrame( aRect, nStyle );
    }
}

// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize( "", off )
#endif

void DrawProgress( Window* pWindow, const Point& rPos,
                   long nOffset, long nPrgsWidth, long nPrgsHeight,
                   USHORT nPercent1, USHORT nPercent2, USHORT nPercentCount )
{
    // Werte vorberechnen
    USHORT nPerc1 = nPercent1 / nPercentCount;
    USHORT nPerc2 = nPercent2 / nPercentCount;

    // Percent-Rechtecke malen
    if ( nPerc1 < nPerc2 )
    {
        // Wenn Percent2 ueber 100%, Werte anpassen
        if ( nPercent2 > 10000 )
        {
            nPerc2 = 10000 / nPercentCount;
            if ( nPerc1 >= nPerc2 )
                nPerc1 = nPerc2-1;
        }

        // Rechteck berechnen
        long        nDX = nPrgsWidth + nOffset;
        long        nLeft = rPos.X()+(nPerc1*nDX);
        Rectangle   aRect( nLeft, rPos.Y(), nLeft+nPrgsWidth, rPos.Y()+nPrgsHeight );

        do
        {
            pWindow->DrawRect( aRect );
            aRect.Left()  += nDX;
            aRect.Right() += nDX;
            nPerc1++;
        }
        while ( nPerc1 < nPerc2 );

        // Bei mehr als 100%, lassen wir das Rechteck blinken
        if ( nPercent2 > 10000 )
        {
            // an/aus-Status festlegen
            if ( ((nPercent2 / nPercentCount) & 0x01) == (nPercentCount & 0x01) )
            {
                aRect.Left()  -= nDX;
                aRect.Right() -= nDX;
                pWindow->Erase( aRect );
            }
        }

        pWindow->Flush();
    }
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif

// -----------------------------------------------------------------------

void StatusBar::ImplDrawProgress( BOOL bPaint,
                                  USHORT nPercent1, USHORT nPercent2 )
{
    // Wenn Paint, dann muss auch Text und Frame gemalt werden
    if ( bPaint )
    {
        DrawText( maPrgsTxtPos, maPrgsTxt );
        DecorationView aDecoView( this );
        aDecoView.DrawFrame( maPrgsFrameRect, FRAME_DRAW_IN );
    }

    Point aPos( maPrgsFrameRect.Left()+STATUSBAR_PRGS_OFFSET,
                maPrgsFrameRect.Top()+STATUSBAR_PRGS_OFFSET );
    DrawProgress( this, aPos, mnPrgsSize/2, mnPrgsSize, mnPrgsSize,
                  nPercent1*100, nPercent2*100, mnPercentCount );
}

// -----------------------------------------------------------------------

void StatusBar::ImplCalcProgressRect()
{
    // Groessen berechnen
    Size aPrgsTxtSize( GetTextWidth( maPrgsTxt ), GetTextHeight() );
    maPrgsTxtPos.X()    = STATUSBAR_OFFSET_X+1;
    maPrgsTxtPos.Y()    = mnTextY;

    // Progress-Frame berechnen
    maPrgsFrameRect.Left()      = maPrgsTxtPos.X()+aPrgsTxtSize.Width()+STATUSBAR_OFFSET;
    maPrgsFrameRect.Top()       = mnItemY;
    maPrgsFrameRect.Bottom()    = mnCalcHeight - STATUSBAR_OFFSET_Y;

    // Dabei die Breite des Fensters berechnen
    mnPrgsSize = maPrgsFrameRect.Bottom()-maPrgsFrameRect.Top()-(STATUSBAR_PRGS_OFFSET*2);
    USHORT nMaxPercent = STATUSBAR_PRGS_COUNT;

    long nMaxWidth = mnDX-STATUSBAR_OFFSET-1;

    // Wenn es zu viele Percent-Rects sind, verkuerzen wir
    while ( maPrgsFrameRect.Left()+ImplCalcProgessWidth( nMaxPercent, mnPrgsSize ) > nMaxWidth )
    {
        nMaxPercent--;
        if ( nMaxPercent <= STATUSBAR_PRGS_MIN )
            break;
    }
    maPrgsFrameRect.Right() = maPrgsFrameRect.Left() + ImplCalcProgessWidth( nMaxPercent, mnPrgsSize );
    // Fuer die weitere Berechnung brauchen wir den Teiler
    mnPercentCount = 10000 / nMaxPercent;
}

// -----------------------------------------------------------------------

void StatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    // Nur bei linker Maustaste ToolBox ausloesen
    if ( rMEvt.IsLeft() )
    {
        if ( mbVisibleItems )
        {
            Point  aMousePos = rMEvt.GetPosPixel();
            USHORT i = 0;

            // Item suchen, das geklickt wurde
            ImplStatusItem* pItem = mpItemList->First();
            while ( pItem )
            {
                // Ist es dieses Item
                if ( ImplGetItemRectPos( i ).IsInside( aMousePos ) )
                {
                    mnCurItemId = pItem->mnId;
                    if ( rMEvt.GetClicks() == 2 )
                        DoubleClick();
                    else
                        Click();
                    mnCurItemId = 0;

                    // Item wurde gefunden
                    return;
                }

                i++;
                pItem = mpItemList->Next();
            }
        }

        // Kein Item, dann nur Click oder DoubleClick
        if ( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else
            Click();
    }
}

// -----------------------------------------------------------------------

void StatusBar::Paint( const Rectangle& )
{
    if ( mbFormat )
        ImplFormat();

    USHORT nItemCount = (USHORT)mpItemList->Count();

    if ( mbProgressMode )
        ImplDrawProgress( TRUE, 0, mnPercent );
    else
    {
        // Text zeichen
        if ( !mbVisibleItems || (GetStyle() & WB_RIGHT) )
            ImplDrawText( FALSE, 0 );

        // Items zeichnen
        if ( mbVisibleItems )
        {
            // Items zeichnen
            for ( USHORT i = 0; i < nItemCount; i++ )
                ImplDrawItem( FALSE, i, TRUE, TRUE );
        }
    }

    if ( mbBottomBorder )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor( rStyleSettings.GetShadowColor() );
        DrawLine( Point( 0, mnDY-2 ), Point( mnDX-1, mnDY-2 ) );
        SetLineColor( rStyleSettings.GetLightColor() );
        DrawLine( Point( 0, mnDY-1 ), Point( mnDX-1, mnDY-1 ) );
    }
}

// -----------------------------------------------------------------------

void StatusBar::Move()
{
    Window::Move();
}

// -----------------------------------------------------------------------

void StatusBar::Resize()
{
    // Breite und Hoehe abfragen und merken
    long nOldDY = mnDX;
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width();
    mnDY = aSize.Height();
    mnCalcHeight = mnDY;
    if ( mbBottomBorder )
        mnCalcHeight -= 2;

    // Evt. neue Textposition berechnen
    if ( nOldDY && (nOldDY < mnDY) )
    {
        mnTextY = 0;
        mnTextY += (mnCalcHeight-GetTextHeight()-mnTextY)/2;
    }

    // Formatierung neu ausloesen
    mbFormat = TRUE;

    if ( mbProgressMode )
        ImplCalcProgressRect();

    Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        {
            Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            XubString aStr = GetHelpText( nItemId );
            Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
            return;
        }
        else if ( rHEvt.GetMode() & HELPMODE_QUICK )
        {
            Rectangle   aItemRect = GetItemRect( nItemId );
            XubString   aStr = GetItemText( nItemId );
            // Wir zeigen die Quick-Hilfe nur an, wenn Text nicht
            // vollstaendig sichtbar
            if ( GetTextWidth( aStr ) > aItemRect.GetWidth() )
            {
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                Help::ShowQuickHelp( this, aItemRect, aStr );
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
    }

    Window::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void StatusBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplFormat();
    else if ( nType == STATE_CHANGE_UPDATEMODE )
        Invalidate();
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        mbFormat = TRUE;
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

void StatusBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
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

void StatusBar::Click()
{
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void StatusBar::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void StatusBar::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void StatusBar::InsertItem( USHORT nItemId, ULONG nWidth,
                            StatusBarItemBits nBits,
                            long nOffset, USHORT nPos )
{
    DBG_ASSERT( nItemId, "StatusBar::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == STATUSBAR_ITEM_NOTFOUND,
                "StatusBar::InsertItem(): ItemId already exists" );

    // IN und CENTER sind Default
    if ( !(nBits & (SIB_IN | SIB_OUT | SIB_FLAT)) )
        nBits |= SIB_IN;
    if ( !(nBits & (SIB_LEFT | SIB_RIGHT | SIB_CENTER)) )
        nBits |= SIB_CENTER;

    // Item anlegen
    ImplStatusItem* pItem   = new ImplStatusItem;
    pItem->mnId             = nItemId;
    pItem->mnBits           = nBits;
    pItem->mnWidth          = (long)nWidth+2;
    pItem->mnOffset         = nOffset;
    pItem->mnHelpId         = 0;
    pItem->mpUserData       = 0;
    pItem->mbVisible        = TRUE;

    // Item in die Liste einfuegen
    mpItemList->Insert( pItem, nPos );

    mbFormat = TRUE;
    if ( ImplIsItemUpdate() )
        Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::RemoveItem( USHORT nItemId )
{
    USHORT nPos = GetItemPos( nItemId );
    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->Remove( nPos );
        delete pItem;

        mbFormat = TRUE;
        if ( ImplIsItemUpdate() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void StatusBar::ShowItem( USHORT nItemId )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );
        if ( !pItem->mbVisible )
        {
            pItem->mbVisible = TRUE;

            mbFormat = TRUE;
            if ( ImplIsItemUpdate() )
                Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

void StatusBar::HideItem( USHORT nItemId )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );
        if ( pItem->mbVisible )
        {
            pItem->mbVisible = FALSE;

            mbFormat = TRUE;
            if ( ImplIsItemUpdate() )
                Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

BOOL StatusBar::IsItemVisible( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mbVisible;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void StatusBar::ShowItems()
{
    if ( !mbVisibleItems )
    {
        mbVisibleItems = TRUE;
        if ( !mbProgressMode )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void StatusBar::HideItems()
{
    if ( mbVisibleItems )
    {
        mbVisibleItems = FALSE;
        if ( !mbProgressMode )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void StatusBar::CopyItems( const StatusBar& rStatusBar )
{
    // Alle Items entfernen
    ImplStatusItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Items aus der Liste loeschen
    mpItemList->Clear();

    // Items kopieren
    ULONG i = 0;
    pItem = rStatusBar.mpItemList->GetObject( i );
    while ( pItem )
    {
        mpItemList->Insert( new ImplStatusItem( *pItem ), LIST_APPEND );
        i++;
        pItem = rStatusBar.mpItemList->GetObject( i );
    }

    mbFormat = TRUE;
    if ( ImplIsItemUpdate() )
        Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::Clear()
{
    // Alle Item loeschen
    ImplStatusItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Items aus der Liste loeschen
    mpItemList->Clear();

    mbFormat = TRUE;
    if ( ImplIsItemUpdate() )
        Invalidate();
}

// -----------------------------------------------------------------------

USHORT StatusBar::GetItemCount() const
{
    return (USHORT)mpItemList->Count();
}

// -----------------------------------------------------------------------

USHORT StatusBar::GetItemId( USHORT nPos ) const
{
    ImplStatusItem* pItem = mpItemList->GetObject( nPos );
    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT StatusBar::GetItemPos( USHORT nItemId ) const
{
    ImplStatusItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nItemId )
            return (USHORT)mpItemList->GetCurPos();

        pItem = mpItemList->Next();
    }

    return STATUSBAR_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT StatusBar::GetItemId( const Point& rPos ) const
{
    if ( AreItemsVisible() && !mbFormat )
    {
        USHORT nItemCount = GetItemCount();
        USHORT nPos;
        for ( nPos = 0; nPos < nItemCount; nPos++ )
        {
            // Rechteck holen
            Rectangle aRect = ImplGetItemRectPos( nPos );
            if ( aRect.IsInside( rPos ) )
                return mpItemList->GetObject( nPos )->mnId;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

Rectangle StatusBar::GetItemRect( USHORT nItemId ) const
{
    Rectangle aRect;

    if ( AreItemsVisible() && !mbFormat )
    {
        USHORT nPos = GetItemPos( nItemId );
        if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        {
            // Rechteck holen und Rahmen abziehen
            aRect = ImplGetItemRectPos( nPos );
            aRect.Left()++;
            aRect.Right()--;
            return aRect;
        }
    }

    return aRect;
}

// -----------------------------------------------------------------------

Point StatusBar::GetItemTextPos( USHORT nItemId ) const
{
    if ( !mbFormat )
    {
        USHORT nPos = GetItemPos( nItemId );
        if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        {
            // Rechteck holen
            ImplStatusItem* pItem = mpItemList->GetObject( nPos );
            Rectangle aRect = ImplGetItemRectPos( nPos );
            Rectangle aTextRect( aRect.Left()+1, aRect.Top()+1,
                                 aRect.Right()-1, aRect.Bottom()-1 );
            Point aPos = ImplGetItemTextPos( aTextRect.GetSize(),
                                             Size( GetTextWidth( pItem->maText ), GetTextHeight() ),
                                             pItem->mnBits );
            if ( !mbInUserDraw )
            {
                aPos.X() += aTextRect.Left();
                aPos.Y() += aTextRect.Top();
            }
            return aPos;
        }
    }

    return Point();
}

// -----------------------------------------------------------------------

ULONG StatusBar::GetItemWidth( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mnWidth;
    else
        return 0;
}

// -----------------------------------------------------------------------

StatusBarItemBits StatusBar::GetItemBits( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

long StatusBar::GetItemOffset( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mnOffset;
    else
        return 0;
}

// -----------------------------------------------------------------------

void StatusBar::SetItemText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );

        if ( pItem->maText != rText )
        {
            pItem->maText = rText;

            // Item neu Zeichen, wenn StatusBar sichtbar und
            // UpdateMode gesetzt ist
            if ( pItem->mbVisible && !mbFormat && ImplIsItemUpdate() )
            {
                Update();
                ImplDrawItem( TRUE, nPos, TRUE, FALSE );
                Flush();
            }
        }
    }
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetItemText( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetItemData( USHORT nItemId, void* pNewData )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );
        pItem->mpUserData = pNewData;

        // Wenn es ein User-Item ist, DrawItem-Aufrufen
        if ( (pItem->mnBits & SIB_USERDRAW) && pItem->mbVisible &&
             !mbFormat && ImplIsItemUpdate() )
        {
            Update();
            ImplDrawItem( TRUE, nPos, FALSE, FALSE );
            Flush();
        }
    }
}

// -----------------------------------------------------------------------

void* StatusBar::GetItemData( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mpUserData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void StatusBar::SetHelpText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        mpItemList->GetObject( nPos )->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetHelpText( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );
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

void StatusBar::SetHelpId( USHORT nItemId, ULONG nHelpId )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        mpItemList->GetObject( nPos )->mnHelpId = nHelpId;
}

// -----------------------------------------------------------------------

ULONG StatusBar::GetHelpId( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->mnHelpId;
    else
        return 0;
}

// -----------------------------------------------------------------------

void StatusBar::SetBottomBorder( BOOL bBottomBorder )
{
    if ( mbBottomBorder != bBottomBorder )
    {
        mbBottomBorder = bBottomBorder;
        mnCalcHeight = mnDY;
        if ( mbBottomBorder )
            mnCalcHeight -= 2;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void StatusBar::StartProgressMode( const XubString& rText )
{
    DBG_ASSERT( !mbProgressMode, "StatusBar::StartProgressMode(): progress mode is active" );

    mbProgressMode  = TRUE;
    mnPercent       = 0;
    maPrgsTxt       = rText;

    // Groessen berechnen
    ImplCalcProgressRect();

    // Paint ausloesen (dort wird der Text und der Frame gemalt)
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Color aPrgsColor = rStyleSettings.GetHighlightColor();
    if ( aPrgsColor == rStyleSettings.GetFaceColor() )
        aPrgsColor = rStyleSettings.GetDarkShadowColor();
    SetLineColor();
    SetFillColor( aPrgsColor );
    if ( IsReallyVisible() )
    {
        Invalidate();
        Update();
        Flush();
    }
}

// -----------------------------------------------------------------------

void StatusBar::SetProgressValue( USHORT nNewPercent )
{
    DBG_ASSERT( mbProgressMode, "StatusBar::SetProgressValue(): no progrss mode" );
    DBG_ASSERTWARNING( nNewPercent <= 100, "StatusBar::SetProgressValue(): nPercent > 100" );

    if ( mbProgressMode && IsReallyVisible() )
    {
        Update();
        SetLineColor();
        ImplDrawProgress( FALSE, mnPercent, nNewPercent );
        Flush();
    }
    mnPercent = nNewPercent;
}

// -----------------------------------------------------------------------

void StatusBar::EndProgressMode()
{
    DBG_ASSERT( mbProgressMode, "StatusBar::EndProgressMode(): no progress mode" );

    mbProgressMode = FALSE;
    maPrgsTxt.Erase();

    // Paint neu ausloesen um StatusBar wieder herzustellen
    SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
    if ( IsReallyVisible() )
    {
        Invalidate();
        Update();
        Flush();
    }
}

// -----------------------------------------------------------------------

void StatusBar::SetText( const XubString& rText )
{
    if ( (!mbVisibleItems || (GetStyle() & WB_RIGHT)) && !mbProgressMode &&
         IsReallyVisible() && IsUpdateMode() )
    {
        if ( mbFormat  )
        {
            Invalidate();
            Window::SetText( rText );
        }
        else
        {
            Update();
            long nOldTextWidth = GetTextWidth( GetText() );
            Window::SetText( rText );
            ImplDrawText( TRUE, nOldTextWidth );
            Flush();
        }
    }
    else
        Window::SetText( rText );
}

// -----------------------------------------------------------------------

Size StatusBar::CalcWindowSizePixel() const
{
    ULONG   i = 0;
    ULONG   nCount = mpItemList->Count();
    long    nOffset = 0;
    long    nCalcWidth = (STATUSBAR_OFFSET_X*2);
    long    nCalcHeight;

    while ( i < nCount )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( i );
        nCalcWidth += pItem->mnWidth + nOffset;
        nOffset = pItem->mnOffset;
        i++;
    }

    nCalcHeight = GetTextHeight()+(STATUSBAR_OFFSET_TEXTY*2);
    if ( mbBottomBorder )
        nCalcHeight += 2;

    return Size( nCalcWidth, nCalcHeight );
}
