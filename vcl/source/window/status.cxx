/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: status.cxx,v $
 * $Revision: 1.28 $
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
#include <tools/list.hxx>
#include <tools/debug.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/svdata.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/status.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.h>

// =======================================================================

#define STATUSBAR_OFFSET_X      STATUSBAR_OFFSET
#define STATUSBAR_OFFSET_Y      2
#define STATUSBAR_OFFSET_TEXTY  3

#define STATUSBAR_PRGS_OFFSET   3
#define STATUSBAR_PRGS_COUNT    100
#define STATUSBAR_PRGS_MIN      5

// -----------------------------------------------------------------------

class StatusBar::ImplData
{
public:
    ImplData();
    ~ImplData();

    VirtualDevice*      mpVirDev;
    BOOL                mbTopBorder:1;
};

StatusBar::ImplData::ImplData()
{
    mpVirDev = NULL;
    mbTopBorder = FALSE;
}

StatusBar::ImplData::~ImplData()
{
}

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
    XubString           maQuickHelpText;
    ULONG               mnHelpId;
    void*               mpUserData;
    BOOL                mbVisible;
    XubString           maAccessibleName;
    XubString           maCommand;
};

DECLARE_LIST( ImplStatusItemList, ImplStatusItem* )

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
    long delta = (rTextSize.Height()/4) + 1;
    if( delta + rTextSize.Width() > rRectSize.Width() )
        delta = 0;

    if ( nStyle & SIB_LEFT )
        nX = delta;
    else if ( nStyle & SIB_RIGHT )
        nX = rRectSize.Width()-rTextSize.Width()-delta;
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
    mpImplData = new ImplData;

    // Default ist RightAlign
    if ( !(nStyle & (WB_LEFT | WB_RIGHT)) )
        nStyle |= WB_RIGHT;

    Window::ImplInit( pParent, nStyle & ~WB_BORDER, NULL );

    // WinBits merken
    mpItemList      = new ImplStatusItemList;
    mpImplData->mpVirDev        = new VirtualDevice( *this );
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

    SetOutputSizePixel( CalcWindowSizePixel() );
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
    delete mpImplData->mpVirDev;

    delete mpImplData;
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

        mpImplData->mpVirDev->SetFont( GetFont() );
        mpImplData->mpVirDev->SetTextColor( GetTextColor() );
        mpImplData->mpVirDev->SetTextAlign( GetTextAlign() );
        mpImplData->mpVirDev->SetTextFillColor();
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
        mpImplData->mpVirDev->SetBackground( GetBackground() );

        // NWF background
        if( ! IsControlBackground() &&
              IsNativeControlSupported( CTRL_WINDOW_BACKGROUND, PART_BACKGROUND_WINDOW ) )
        {
            ImplGetWindowImpl()->mnNativeBackground = PART_BACKGROUND_WINDOW;
            EnableChildTransparentMode( TRUE );
        }
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
            if( IsTopBorder() )
                aRect.Bottom()+=2;
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
            mpImplData->mpVirDev->SetOutputSizePixel( aVirDevSize );
            Rectangle aTempRect = aTextRect;
            aTempRect.SetPos( Point( 0, 0 ) );
            mpImplData->mpVirDev->DrawText( aTempRect, aStr, TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
            DrawOutDev( aTextRect.TopLeft(), aVirDevSize, Point(), aVirDevSize, *mpImplData->mpVirDev );
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
        mpImplData->mpVirDev->SetOutputSizePixel( aTextRectSize );
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
            mpImplData->mpVirDev->DrawText( aTextPos, pItem->maText );
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
            UserDrawEvent aODEvt( mpImplData->mpVirDev, Rectangle( Point(), aTextRectSize ), pItem->mnId );
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
        DrawOutDev( aTextRect.TopLeft(), aTextRectSize, Point(), aTextRectSize, *mpImplData->mpVirDev );
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

    if ( !ImplIsRecordLayout() )
        ImplCallEventListeners( VCLEVENT_STATUSBAR_DRAWITEM, (void*) sal_IntPtr(pItem->mnId) );
}

// -----------------------------------------------------------------------

void DrawProgress( Window* pWindow, const Point& rPos,
                   long nOffset, long nPrgsWidth, long nPrgsHeight,
                   USHORT nPercent1, USHORT nPercent2, USHORT nPercentCount,
                   const Rectangle& rFramePosSize
                   )
{
    if( pWindow->IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        bool bNeedErase = ImplGetSVData()->maNWFData.mbProgressNeedsErase;

        long nFullWidth = (nPrgsWidth + nOffset) * (10000 / nPercentCount);
        long nPerc = (nPercent2 > 10000) ? 10000 : nPercent2;
        ImplControlValue aValue( nFullWidth * (long)nPerc / 10000 );
        Rectangle aDrawRect( rPos, Size( nFullWidth, nPrgsHeight ) );
        Region aControlRegion( aDrawRect );
        if( bNeedErase )
        {
            Window* pEraseWindow = pWindow;
            while( pEraseWindow->IsPaintTransparent()                         &&
                   ! pEraseWindow->ImplGetWindowImpl()->mbFrame )
            {
                pEraseWindow = pEraseWindow->ImplGetWindowImpl()->mpParent;
            }
            if( pEraseWindow == pWindow )
                // restore background of pWindow
                pEraseWindow->Erase( rFramePosSize );
            else
            {
                // restore transparent background
                Point aTL( pWindow->OutputToAbsoluteScreenPixel( rFramePosSize.TopLeft() ) );
                aTL = pEraseWindow->AbsoluteScreenToOutputPixel( aTL );
                Rectangle aRect( aTL, rFramePosSize.GetSize() );
                pEraseWindow->Invalidate( aRect, INVALIDATE_NOCHILDREN     |
                                                 INVALIDATE_NOCLIPCHILDREN |
                                                 INVALIDATE_TRANSPARENT );
                pEraseWindow->Update();
            }
            pWindow->Push( PUSH_CLIPREGION );
            pWindow->IntersectClipRegion( rFramePosSize );
        }
        BOOL bNativeOK = pWindow->DrawNativeControl( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                                     CTRL_STATE_ENABLED, aValue, rtl::OUString() );
        if( bNeedErase )
            pWindow->Pop();
        if( bNativeOK )
        {
            pWindow->Flush();
            return;
        }
    }

    // Werte vorberechnen
    USHORT nPerc1 = nPercent1 / nPercentCount;
    USHORT nPerc2 = nPercent2 / nPercentCount;

    if ( nPerc1 > nPerc2 )
    {
        // Support progress that can also decrease

        // Rechteck berechnen
        long        nDX = nPrgsWidth + nOffset;
        long        nLeft = rPos.X()+((nPerc1-1)*nDX);
        Rectangle   aRect( nLeft, rPos.Y(), nLeft+nPrgsWidth, rPos.Y()+nPrgsHeight );

        do
        {
            pWindow->Erase( aRect );
            aRect.Left()  -= nDX;
            aRect.Right() -= nDX;
            nPerc1--;
        }
        while ( nPerc1 > nPerc2 );

        pWindow->Flush();
    }
    else if ( nPerc1 < nPerc2 )
    {
        // Percent-Rechtecke malen
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

// -----------------------------------------------------------------------

void StatusBar::ImplDrawProgress( BOOL bPaint,
                                  USHORT nPercent1, USHORT nPercent2 )
{
    bool bNative = IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL );
    // bPaint: draw text also, else only update progress
    if ( bPaint )
    {
        DrawText( maPrgsTxtPos, maPrgsTxt );
        if( ! bNative )
        {
            DecorationView aDecoView( this );
            aDecoView.DrawFrame( maPrgsFrameRect, FRAME_DRAW_IN );
        }
    }

    Point aPos( maPrgsFrameRect.Left()+STATUSBAR_PRGS_OFFSET,
                maPrgsFrameRect.Top()+STATUSBAR_PRGS_OFFSET );
    long nPrgsHeight = mnPrgsSize;
    if( bNative )
    {
        aPos = maPrgsFrameRect.TopLeft();
        nPrgsHeight += STATUSBAR_PRGS_OFFSET;
    }
    DrawProgress( this, aPos, mnPrgsSize/2, mnPrgsSize, nPrgsHeight,
                  nPercent1*100, nPercent2*100, mnPercentCount, maPrgsFrameRect );
}

// -----------------------------------------------------------------------

void StatusBar::ImplCalcProgressRect()
{
    // calculate text size
    Size aPrgsTxtSize( GetTextWidth( maPrgsTxt ), GetTextHeight() );
    maPrgsTxtPos.X()    = STATUSBAR_OFFSET_X+1;

    // calculate progress frame
    maPrgsFrameRect.Left()      = maPrgsTxtPos.X()+aPrgsTxtSize.Width()+STATUSBAR_OFFSET;
    maPrgsFrameRect.Top()       = mnItemY;
    maPrgsFrameRect.Bottom()    = mnCalcHeight - STATUSBAR_OFFSET_Y;
    if( IsTopBorder() )
        maPrgsFrameRect.Bottom()+=2;

    // calculate size of progress rects
    mnPrgsSize = maPrgsFrameRect.Bottom()-maPrgsFrameRect.Top()-(STATUSBAR_PRGS_OFFSET*2);
    USHORT nMaxPercent = STATUSBAR_PRGS_COUNT;

    long nMaxWidth = mnDX-STATUSBAR_OFFSET-1;

    // make smaller if there are too many rects
    while ( maPrgsFrameRect.Left()+ImplCalcProgessWidth( nMaxPercent, mnPrgsSize ) > nMaxWidth )
    {
        nMaxPercent--;
        if ( nMaxPercent <= STATUSBAR_PRGS_MIN )
            break;
    }
    maPrgsFrameRect.Right() = maPrgsFrameRect.Left() + ImplCalcProgessWidth( nMaxPercent, mnPrgsSize );

    // save the divisor for later
    mnPercentCount = 10000 / nMaxPercent;
    BOOL bNativeOK = FALSE;
    if( IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        Region aControlRegion( Rectangle( (const Point&)Point(), maPrgsFrameRect.GetSize() ) );
        Region aNativeControlRegion, aNativeContentRegion;
        if( (bNativeOK = GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                                 CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                                 aNativeControlRegion, aNativeContentRegion ) ) != FALSE )
        {
            long nProgressHeight = aNativeControlRegion.GetBoundRect().GetHeight();
            maPrgsTxtPos.Y() = mnItemY + (nProgressHeight - GetTextHeight())/2;
        }
    }
    if( ! bNativeOK )
        maPrgsTxtPos.Y()    = mnTextY;


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

    // draw borders
    if( IsTopBorder() )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor( rStyleSettings.GetShadowColor() );
        DrawLine( Point( 0, 0 ), Point( mnDX-1, 0 ) );
        SetLineColor( rStyleSettings.GetLightColor() );
        DrawLine( Point( 0, 1 ), Point( mnDX-1, 1 ) );
    }

    if ( IsBottomBorder() )
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
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width();
    mnDY = aSize.Height();
    mnCalcHeight = mnDY;
    // subtract border
    if( IsTopBorder() )
        mnCalcHeight -= 2;
    if ( IsBottomBorder() )
        mnCalcHeight -= 2;

    mnItemY = STATUSBAR_OFFSET_Y;
    if( IsTopBorder() )
        mnItemY += 2;
    mnTextY = (mnCalcHeight-GetTextHeight())/2;
    if( IsTopBorder() )
        mnTextY += 2;

    // Formatierung neu ausloesen
    mbFormat = TRUE;

    if ( mbProgressMode )
        ImplCalcProgressRect();

    Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help in status bar
    if( rHEvt.KeyboardActivated() )
        return;

    USHORT nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( nItemId )
    {
        Rectangle aItemRect = GetItemRect( nItemId );
        Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();

        if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        {
            XubString aStr = GetHelpText( nItemId );
            Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
            return;
        }
        else if ( rHEvt.GetMode() & HELPMODE_QUICK )
        {
            XubString   aStr = GetQuickHelpText( nItemId );
            // Show quickhelp if available
            if( aStr.Len() )
            {
                Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
            aStr = GetItemText( nItemId );
            // show a quick help if item text doesn't fit
            if ( GetTextWidth( aStr ) > aItemRect.GetWidth() )
            {
                Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            String aCommand = GetItemCommand( nItemId );
            ULONG nHelpId = GetHelpId( nItemId );

            if ( aCommand.Len() || nHelpId )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                {
                    if ( aCommand.Len() )
                        pHelp->Start( aCommand, this );
                    else if ( nHelpId )
                        pHelp->Start( nHelpId, this );
                }
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
        ImplStatusItem* pItem = mpItemList->First();
        long nFudge = GetTextHeight() / 4;
        while ( pItem )
        {
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( nWidth > pItem->mnWidth + STATUSBAR_OFFSET )
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;
            pItem = mpItemList->Next();
        }
        Size aSize = GetSizePixel();
        // do not disturb current width, since
        // CalcWindowSizePixel calculates a minimum width
        aSize.Height() = CalcWindowSizePixel().Height();
        SetSizePixel( aSize );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void StatusBar::Click()
{
    ImplCallEventListeners( VCLEVENT_STATUSBAR_CLICK );
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void StatusBar::DoubleClick()
{
    ImplCallEventListeners( VCLEVENT_STATUSBAR_DOUBLECLICK );
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
    long nFudge = GetTextHeight()/4;
    ImplStatusItem* pItem   = new ImplStatusItem;
    pItem->mnId             = nItemId;
    pItem->mnBits           = nBits;
    pItem->mnWidth          = (long)nWidth+nFudge+STATUSBAR_OFFSET;
    pItem->mnOffset         = nOffset;
    pItem->mnHelpId         = 0;
    pItem->mpUserData       = 0;
    pItem->mbVisible        = TRUE;

    // Item in die Liste einfuegen
    mpItemList->Insert( pItem, nPos );

    mbFormat = TRUE;
    if ( ImplIsItemUpdate() )
        Invalidate();

    ImplCallEventListeners( VCLEVENT_STATUSBAR_ITEMADDED, (void*) sal_IntPtr(nItemId) );
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

        ImplCallEventListeners( VCLEVENT_STATUSBAR_ITEMREMOVED, (void*) sal_IntPtr(nItemId) );
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

            ImplCallEventListeners( VCLEVENT_STATUSBAR_SHOWITEM, (void*) sal_IntPtr(nItemId) );
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

            ImplCallEventListeners( VCLEVENT_STATUSBAR_HIDEITEM, (void*) sal_IntPtr(nItemId) );
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

        ImplCallEventListeners( VCLEVENT_STATUSBAR_SHOWALLITEMS );
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

        ImplCallEventListeners( VCLEVENT_STATUSBAR_HIDEALLITEMS );
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

    ImplCallEventListeners( VCLEVENT_STATUSBAR_ALLITEMSREMOVED );
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

            // adjust item width - see also DataChanged()
            long nFudge = GetTextHeight()/4;
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( nWidth > pItem->mnWidth + STATUSBAR_OFFSET )
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;

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

void StatusBar::SetItemCommand( USHORT nItemId, const XubString& rCommand )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );

        if ( pItem->maCommand != rCommand )
            pItem->maCommand = rCommand;
    }
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetItemCommand( USHORT nItemId )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maCommand;
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
        if ( !pItem->maHelpText.Len() && ( pItem->mnHelpId || pItem->maCommand.Len() ))
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if ( pItem->maCommand.Len() )
                    pItem->maHelpText = pHelp->GetHelpText( pItem->maCommand, this );
                if ( !pItem->maHelpText.Len() && pItem->mnHelpId )
                    pItem->maHelpText = pHelp->GetHelpText( pItem->mnHelpId, this );
            }
        }

        return pItem->maHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetQuickHelpText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        mpItemList->GetObject( nPos )->maQuickHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetQuickHelpText( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );
        return pItem->maQuickHelpText;
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

void StatusBar::ImplCalcBorder( )
{
    mnCalcHeight = mnDY;
    // subtract border
    if( IsTopBorder() )
    {
        mnCalcHeight -= 2;
        mnTextY += 2;
        mnItemY += 2;
    }
    if ( IsBottomBorder() )
        mnCalcHeight -= 2;
    mbFormat = TRUE;
    Invalidate();
}

void StatusBar::SetBottomBorder( BOOL bBottomBorder )
{
    if ( mbBottomBorder != bBottomBorder )
    {
        mbBottomBorder = bBottomBorder;
        ImplCalcBorder();
    }
}

void StatusBar::SetTopBorder( BOOL bTopBorder )
{
    if ( mpImplData->mbTopBorder != bTopBorder )
    {
        mpImplData->mbTopBorder = bTopBorder;
        ImplCalcBorder();
    }
}

BOOL StatusBar::IsTopBorder() const
{
    return mpImplData->mbTopBorder;
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

    if ( mbProgressMode
    &&   IsReallyVisible()
    &&   (!mnPercent || (mnPercent != nNewPercent)) )
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

void StatusBar::ResetProgressMode()
{
    if ( mbProgressMode )
    {
        mnPercent = 0;
        maPrgsTxt.Erase();
        if ( IsReallyVisible() )
        {
            Invalidate();
            Update();
            Flush();
        }
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
    else if ( mbProgressMode )
    {
        maPrgsTxt = rText;
        if ( IsReallyVisible() )
        {
            Invalidate();
            Update();
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

    long nMinHeight = GetTextHeight();
    // FIXME: IsNativeControlSupported and GetNativeControlRegion should be const ?
    StatusBar* pThis = const_cast<StatusBar*>( this );
    if( pThis->IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        Region aControlRegion( Rectangle( (const Point&)Point(), Size( nCalcWidth, nMinHeight ) ) );
        Region aNativeControlRegion, aNativeContentRegion;
        if( pThis->GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                           CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                           aNativeControlRegion, aNativeContentRegion ) )
        {
            long nProgressHeight = aNativeControlRegion.GetBoundRect().GetHeight();
            if( nProgressHeight > nMinHeight )
                nMinHeight = nProgressHeight;
        }
    }

    nCalcHeight = nMinHeight+(STATUSBAR_OFFSET_TEXTY*2);
    // add border
    if( IsTopBorder() )
        nCalcHeight += 2;
    if ( IsBottomBorder() )
        nCalcHeight += 2;

    return Size( nCalcWidth, nCalcHeight );
}


// -----------------------------------------------------------------------

void StatusBar::SetAccessibleName( USHORT nItemId, const XubString& rName )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
        ImplStatusItem* pItem = mpItemList->GetObject( nPos );

        if ( pItem->maAccessibleName != rName )
        {
            pItem->maAccessibleName = rName;
            ImplCallEventListeners( VCLEVENT_STATUSBAR_NAMECHANGED, (void*) sal_IntPtr(pItem->mnId) );
        }
    }
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetAccessibleName( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != STATUSBAR_ITEM_NOTFOUND )
        return mpItemList->GetObject( nPos )->maAccessibleName;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------
