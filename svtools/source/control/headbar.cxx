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

#include <headbar.hxx>
#include <tools/debug.hxx>

#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/salnativewidgets.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

// =======================================================================

class ImplHeadItem
{
public:
    sal_uInt16              mnId;
    HeaderBarItemBits   mnBits;
    long                mnSize;
    OString        maHelpId;
    Image               maImage;
    XubString           maOutText;
    XubString           maText;
    XubString           maHelpText;
};

// =======================================================================

#define HEAD_ARROWSIZE1             4
#define HEAD_ARROWSIZE2             7

#define HEADERBAR_TEXTOFF           2
#define HEADERBAR_ARROWOFF          5
#define HEADERBAR_SPLITOFF          3

#define HEADERBAR_DRAGOUTOFF        15

#define HEAD_HITTEST_ITEM           ((sal_uInt16)0x0001)
#define HEAD_HITTEST_DIVIDER        ((sal_uInt16)0x0002)

// =======================================================================

void HeaderBar::ImplInit( WinBits nWinStyle )
{
    mpItemList      = new ImplHeadItemList;
    mnBorderOff1    = 0;
    mnBorderOff2    = 0;
    mnOffset        = 0;
    mnDX            = 0;
    mnDY            = 0;
    mnDragSize      = 0;
    mnStartPos      = 0;
    mnDragPos       = 0;
    mnMouseOff      = 0;
    mnCurItemId     = 0;
    mnItemDragPos   = HEADERBAR_ITEM_NOTFOUND;
    mbDrag          = sal_False;
    mbItemDrag      = sal_False;
    mbOutDrag       = sal_False;
    mbItemMode      = sal_False;

    // StyleBits auswerten
    if ( nWinStyle & WB_DRAG )
        mbDragable = sal_True;
    else
        mbDragable = sal_False;
    if ( nWinStyle & WB_BUTTONSTYLE )
        mbButtonStyle = sal_True;
    else
        mbButtonStyle = sal_False;
    if ( nWinStyle & WB_BORDER )
    {
        mnBorderOff1 = 1;
        mnBorderOff2 = 1;
    }
    else
    {
        if ( nWinStyle & WB_BOTTOMBORDER )
            mnBorderOff2 = 1;
    }

    ImplInitSettings( sal_True, sal_True, sal_True );
}

// -----------------------------------------------------------------------

HeaderBar::HeaderBar( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle & WB_3DLOOK )
{
    ImplInit( nWinStyle );
    SetSizePixel( CalcWindowSizePixel() );
}

// -----------------------------------------------------------------------

HeaderBar::HeaderBar( Window* pParent, const ResId& rResId ) :
    Window( pParent, rResId )
{
    ImplInit( rResId.GetWinBits() );
}

// -----------------------------------------------------------------------

HeaderBar::~HeaderBar()
{
    // Alle Items loeschen
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        delete (*mpItemList)[ i ];
    }
    mpItemList->clear();
    delete mpItemList;
}

// -----------------------------------------------------------------------

void HeaderBar::ImplInitSettings( sal_Bool bFont,
                                  sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetToolFont();
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
}

// -----------------------------------------------------------------------

long HeaderBar::ImplGetItemPos( sal_uInt16 nPos ) const
{
    long nX = -mnOffset;
    for ( size_t i = 0; i < nPos; i++ )
        nX += (*mpItemList)[ i ]->mnSize;
    return nX;
}

// -----------------------------------------------------------------------

Rectangle HeaderBar::ImplGetItemRect( sal_uInt16 nPos ) const
{
    Rectangle aRect( ImplGetItemPos( nPos ), 0, 0, mnDY-1 );
    aRect.Right() = aRect.Left() + (*mpItemList)[ nPos ]->mnSize - 1;
    // Gegen Ueberlauf auf einigen Systemen testen
    if ( aRect.Right() > 16000 )
        aRect.Right() = 16000;
    return aRect;
}

// -----------------------------------------------------------------------

sal_uInt16 HeaderBar::ImplHitTest( const Point& rPos,
                               long& nMouseOff, sal_uInt16& nPos ) const
{
    ImplHeadItem*   pItem;
    size_t          nCount = (sal_uInt16)mpItemList->size();
    sal_Bool            bLastFixed = sal_True;
    long            nX = -mnOffset;

    for ( size_t i = 0; i < nCount; i++ )
    {
        pItem = (*mpItemList)[ i ];

        if ( rPos.X() < (nX+pItem->mnSize) )
        {
            sal_uInt16 nMode;

            if ( !bLastFixed && (rPos.X() < (nX+HEADERBAR_SPLITOFF)) )
            {
                nMode = HEAD_HITTEST_DIVIDER;
                nPos = i-1;
                nMouseOff = rPos.X()-nX+1;
            }
            else
            {
                nPos = i;

                if ( !(pItem->mnBits & HIB_FIXED) && (rPos.X() >= (nX+pItem->mnSize-HEADERBAR_SPLITOFF)) )
                {
                    nMode = HEAD_HITTEST_DIVIDER;
                    nMouseOff = rPos.X()-(nX+pItem->mnSize);
                }
                else
                {
                    nMode = HEAD_HITTEST_ITEM;
                    nMouseOff = rPos.X()-nX;
                }
            }

            return nMode;
        }

        if ( pItem->mnBits & HIB_FIXED )
            bLastFixed = sal_True;
        else
            bLastFixed = sal_False;

        nX += pItem->mnSize;
    }

    if ( !bLastFixed )
    {
        pItem = (*mpItemList)[ nCount-1 ];
        if ( (pItem->mnSize < 4)  && (rPos.X() < (nX+HEADERBAR_SPLITOFF)) )
        {
            nPos = nCount-1;
            nMouseOff = rPos.X()-nX+1;
            return HEAD_HITTEST_DIVIDER;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void HeaderBar::ImplInvertDrag( sal_uInt16 nStartPos, sal_uInt16 nEndPos )
{
    Rectangle aRect1 = ImplGetItemRect( nStartPos );
    Rectangle aRect2 = ImplGetItemRect( nEndPos );
    Point     aStartPos = aRect1.Center();
    Point     aEndPos = aStartPos;
    Rectangle aStartRect( aStartPos.X()-2, aStartPos.Y()-2,
                          aStartPos.X()+2, aStartPos.Y()+2 );

    if ( nEndPos > nStartPos )
    {
        aStartPos.X() += 3;
        aEndPos.X() = aRect2.Right()-6;
    }
    else
    {
        aStartPos.X() -= 3;
        aEndPos.X() = aRect2.Left()+6;
    }

    SetRasterOp( ROP_INVERT );
    DrawRect( aStartRect );
    DrawLine( aStartPos, aEndPos );
    if ( nEndPos > nStartPos )
    {
        DrawLine( Point( aEndPos.X()+1, aEndPos.Y()-3 ),
                  Point( aEndPos.X()+1, aEndPos.Y()+3 ) );
        DrawLine( Point( aEndPos.X()+2, aEndPos.Y()-2 ),
                  Point( aEndPos.X()+2, aEndPos.Y()+2 ) );
        DrawLine( Point( aEndPos.X()+3, aEndPos.Y()-1 ),
                  Point( aEndPos.X()+3, aEndPos.Y()+1 ) );
        DrawPixel( Point( aEndPos.X()+4, aEndPos.Y() ) );
    }
    else
    {
        DrawLine( Point( aEndPos.X()-1, aEndPos.Y()-3 ),
                  Point( aEndPos.X()-1, aEndPos.Y()+3 ) );
        DrawLine( Point( aEndPos.X()-2, aEndPos.Y()-2 ),
                  Point( aEndPos.X()-2, aEndPos.Y()+2 ) );
        DrawLine( Point( aEndPos.X()-3, aEndPos.Y()-1 ),
                  Point( aEndPos.X()-3, aEndPos.Y()+1 ) );
        DrawPixel( Point( aEndPos.X()-4, aEndPos.Y() ) );
    }
    SetRasterOp( ROP_OVERPAINT );
}

// -----------------------------------------------------------------------

void HeaderBar::ImplDrawItem( OutputDevice* pDev,
                              sal_uInt16 nPos, sal_Bool bHigh, sal_Bool bDrag,
                              const Rectangle& rItemRect,
                              const Rectangle* pRect,
                              sal_uLong )
{
    Window *const pWin = (pDev->GetOutDevType()==OUTDEV_WINDOW) ? (Window*) pDev : NULL;
    ImplControlValue aControlValue(0);
    Rectangle aCtrlRegion;
    ControlState nState(0);

    Rectangle aRect = rItemRect;

    // Wenn kein Platz, dann brauchen wir auch nichts ausgeben
    if ( aRect.GetWidth() <= 1 )
        return;

    // Feststellen, ob Rectangle ueberhaupt sichtbar
    if ( pRect )
    {
        if ( aRect.Right() < pRect->Left() )
            return;
        else if ( aRect.Left() > pRect->Right() )
            return;
    }
    else
    {
        if ( aRect.Right() < 0 )
            return;
        else if ( aRect.Left() > mnDX )
            return;
    }

    ImplHeadItem*           pItem  = (*mpItemList)[ nPos ];
    HeaderBarItemBits       nBits = pItem->mnBits;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();

    if( pWin && pWin->IsNativeControlSupported(CTRL_WINDOW_BACKGROUND, PART_ENTIRE_CONTROL) )
    {
        aCtrlRegion=aRect;
        pWin->DrawNativeControl( CTRL_WINDOW_BACKGROUND, PART_ENTIRE_CONTROL,
                                 aCtrlRegion, nState, aControlValue,
                                 OUString() );

    }
    else
    {
        // Border muss nicht gemalt werden
        aRect.Top()     += mnBorderOff1;
        aRect.Bottom()  -= mnBorderOff2;

        // Hintergrund loeschen
        if ( !pRect || bDrag )
        {
            if ( bDrag )
            {
                pDev->SetLineColor();
                pDev->SetFillColor( rStyleSettings.GetCheckedColor() );
                pDev->DrawRect( aRect );
            }
            else
                pDev->DrawWallpaper( aRect, GetBackground() );
        }
    }

    Color aSelectionTextColor( COL_TRANSPARENT );

    if( pWin && pWin->IsNativeControlSupported(CTRL_LISTHEADER, PART_BUTTON) )
    {
        aCtrlRegion=aRect;
        aControlValue.setTristateVal(BUTTONVALUE_ON);
        nState|=CTRL_STATE_ENABLED;
        if(bHigh)
            nState|=CTRL_STATE_PRESSED;
        pWin->DrawNativeControl( CTRL_LISTHEADER, PART_BUTTON,
                                 aCtrlRegion, nState, aControlValue,
                                 OUString() );
    }
    else
    {
        // Trennlinie malen
        pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
        pDev->DrawLine( Point( aRect.Right(), aRect.Top() ),
                        Point( aRect.Right(), aRect.Bottom() ) );

        // ButtonStyle malen
        // avoid 3D borders
        if( bHigh )
            DrawSelectionBackground( aRect, 1, sal_True, sal_False, sal_False, &aSelectionTextColor );
        else if ( !mbButtonStyle || (nBits & HIB_FLAT) )
            DrawSelectionBackground( aRect, 0, sal_True, sal_False, sal_False, &aSelectionTextColor );
    }

    // Wenn kein Platz, dann brauchen wir auch nichts ausgeben
    if ( aRect.GetWidth() < 1 )
        return;

    // Positionen und Groessen berechnen und Inhalt ausgeben
    pItem->maOutText = pItem->maText;
    Size aImageSize = pItem->maImage.GetSizePixel();
    Size aTxtSize( pDev->GetTextWidth( pItem->maOutText ), 0  );
    if ( pItem->maOutText.Len() )
        aTxtSize.Height() = pDev->GetTextHeight();
    long nArrowWidth = 0;
    if ( nBits & (HIB_UPARROW | HIB_DOWNARROW) )
        nArrowWidth = HEAD_ARROWSIZE2+HEADERBAR_ARROWOFF;

    // Wenn kein Platz fuer Image, dann nicht ausgeben
    long nTestHeight = aImageSize.Height();
    if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
        nTestHeight += aTxtSize.Height();
    if ( (aImageSize.Width() > aRect.GetWidth()) || (nTestHeight > aRect.GetHeight()) )
    {
        aImageSize.Width() = 0;
        aImageSize.Height() = 0;
    }

    // Text auf entsprechende Laenge kuerzen
    sal_Bool bLeftText = sal_False;
    long nMaxTxtWidth = aRect.GetWidth()-(HEADERBAR_TEXTOFF*2)-nArrowWidth;
    if ( nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE) )
        nMaxTxtWidth -= aImageSize.Width();
    long nTxtWidth = aTxtSize.Width();
    if ( nTxtWidth > nMaxTxtWidth )
    {
        bLeftText = sal_True;
        // 3 == Len of "..."
        pItem->maOutText.AppendAscii( "..." );
        do
        {
            pItem->maOutText.Erase( pItem->maOutText.Len()-3-1, 1 );
            nTxtWidth = pDev->GetTextWidth( pItem->maOutText );
        }
        while ( (nTxtWidth > nMaxTxtWidth) && (pItem->maOutText.Len() > 3) );
        if ( pItem->maOutText.Len() == 3 )
        {
            nTxtWidth = 0;
            pItem->maOutText.Erase();
        }
    }

    // Text/Imageposition berechnen
    long nTxtPos;
    if ( !bLeftText && (nBits & HIB_RIGHT) )
    {
        nTxtPos = aRect.Right()-nTxtWidth-HEADERBAR_TEXTOFF;
        if ( nBits & HIB_RIGHTIMAGE )
            nTxtPos -= aImageSize.Width();
    }
    else if ( !bLeftText && (nBits & HIB_CENTER) )
    {
        long nTempWidth = nTxtWidth;
        if ( nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE) )
            nTempWidth += aImageSize.Width();
        nTxtPos = aRect.Left()+(aRect.GetWidth()-nTempWidth)/2;
        if ( nBits & HIB_LEFTIMAGE )
            nTxtPos += aImageSize.Width();
        if ( nArrowWidth )
        {
            if ( nTxtPos+nTxtWidth+nArrowWidth >= aRect.Right() )
            {
                nTxtPos = aRect.Left()+HEADERBAR_TEXTOFF;
                if ( nBits & HIB_LEFTIMAGE )
                    nTxtPos += aImageSize.Width();
            }
        }
    }
    else
    {
        nTxtPos = aRect.Left()+HEADERBAR_TEXTOFF;
        if ( nBits & HIB_LEFTIMAGE )
            nTxtPos += aImageSize.Width();
        if ( nBits & HIB_RIGHT )
            nTxtPos += nArrowWidth;
    }

    // TextPosition berechnen
    long nTxtPosY = 0;
    if ( pItem->maOutText.Len() || (nArrowWidth && aTxtSize.Height()) )
    {
        if ( nBits & HIB_TOP )
        {
            nTxtPosY = aRect.Top();
            if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
                nTxtPosY += aImageSize.Height();
        }
        else if ( nBits & HIB_BOTTOM )
            nTxtPosY = aRect.Bottom()-aTxtSize.Height();
        else
        {
            long nTempHeight = aTxtSize.Height();
            if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
                nTempHeight += aImageSize.Height();
            nTxtPosY = aRect.Top()+((aRect.GetHeight()-nTempHeight)/2);
            if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
                nTxtPosY += aImageSize.Height();
        }
    }

    // Text ausgebeben
    if ( pItem->maOutText.Len() )
    {
        if( aSelectionTextColor != Color( COL_TRANSPARENT ) )
        {
            pDev->Push( PUSH_TEXTCOLOR );
            pDev->SetTextColor( aSelectionTextColor );
        }
        if ( IsEnabled() )
            pDev->DrawText( Point( nTxtPos, nTxtPosY ), pItem->maOutText );
        else
            pDev->DrawCtrlText( Point( nTxtPos, nTxtPosY ), pItem->maOutText, 0, STRING_LEN, TEXT_DRAW_DISABLE );
        if( aSelectionTextColor != Color( COL_TRANSPARENT ) )
            pDev->Pop();
    }

    // Wenn Image vorhanden, Position berechnen und ausgeben
    long nImagePosY = 0;
    if ( aImageSize.Width() && aImageSize.Height() )
    {
        long nImagePos = nTxtPos;
        if ( nBits & HIB_LEFTIMAGE )
        {
            nImagePos -= aImageSize.Width();
            if ( nBits & HIB_RIGHT )
                nImagePos -= nArrowWidth;
        }
        else if ( nBits & HIB_RIGHTIMAGE )
        {
            nImagePos += nTxtWidth;
            if ( !(nBits & HIB_RIGHT) )
                nImagePos += nArrowWidth;
        }
        else
        {
            if ( nBits & HIB_RIGHT )
                nImagePos = aRect.Right()-aImageSize.Width();
            else if ( nBits & HIB_CENTER )
                nImagePos = aRect.Left()+(aRect.GetWidth()-aImageSize.Width())/2;
            else
                nImagePos = aRect.Left()+HEADERBAR_TEXTOFF;
        }

        if ( nBits & HIB_TOP )
            nImagePosY = aRect.Top();
        else if ( nBits & HIB_BOTTOM )
        {
            nImagePosY = aRect.Bottom()-aImageSize.Height();
            if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
                nImagePosY -= aTxtSize.Height();
        }
        else
        {
            long nTempHeight = aImageSize.Height();
            if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) )
                nTempHeight += aTxtSize.Height();
            nImagePosY = aRect.Top()+((aRect.GetHeight()-nTempHeight)/2);
        }
        if ( nImagePos+aImageSize.Width() <= aRect.Right() )
        {
            sal_uInt16 nStyle = 0;
            if ( !IsEnabled() )
                nStyle |= IMAGE_DRAW_DISABLE;
            pDev->DrawImage( Point( nImagePos, nImagePosY ), pItem->maImage, nStyle );
        }
    }

    if ( nBits & (HIB_UPARROW | HIB_DOWNARROW) )
    {
        long nArrowX = nTxtPos;
        if ( nBits & HIB_RIGHT )
            nArrowX -= nArrowWidth;
        else
            nArrowX += nTxtWidth+HEADERBAR_ARROWOFF;
        if ( !(nBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) && !pItem->maText.Len() )
        {
            if ( nBits & HIB_RIGHT )
                nArrowX -= aImageSize.Width();
            else
                nArrowX += aImageSize.Width();
        }

        // Feststellen, ob Platz genug ist, das Item zu malen
        sal_Bool bDraw = sal_True;
        if ( nArrowX < aRect.Left()+HEADERBAR_TEXTOFF )
            bDraw = sal_False;
        else if ( nArrowX+HEAD_ARROWSIZE2 > aRect.Right() )
            bDraw = sal_False;

        if ( bDraw )
        {
            if( pWin && pWin->IsNativeControlSupported(CTRL_LISTHEADER, PART_ARROW) )
            {
                aCtrlRegion=Rectangle(Point(nArrowX,aRect.Top()),Size(nArrowWidth,aRect.GetHeight()));
                // control value passes 1 if arrow points down, 0 otherwise
                aControlValue.setNumericVal((nBits&HIB_DOWNARROW)?1:0);
                nState|=CTRL_STATE_ENABLED;
                if(bHigh)
                    nState|=CTRL_STATE_PRESSED;
                pWin->DrawNativeControl( CTRL_LISTHEADER, PART_ARROW,
                                         aCtrlRegion, nState, aControlValue,
                                         OUString() );
            }
            else
            {
                long nArrowY;
                if ( aTxtSize.Height() )
                    nArrowY = nTxtPosY+(aTxtSize.Height()/2);
                else if ( aImageSize.Width() && aImageSize.Height() )
                    nArrowY = nImagePosY+(aImageSize.Height()/2);
                else
                {
                    if ( nBits & HIB_TOP )
                        nArrowY = aRect.Top()+1;
                    else if ( nBits & HIB_BOTTOM )
                        nArrowY = aRect.Bottom()-HEAD_ARROWSIZE2-1;
                    else
                        nArrowY = aRect.Top()+((aRect.GetHeight()-HEAD_ARROWSIZE2)/2);
                }
                nArrowY -= HEAD_ARROWSIZE1-1;
                if ( nBits & HIB_DOWNARROW )
                {
                    pDev->SetLineColor( rStyleSettings.GetLightColor() );
                    pDev->DrawLine( Point( nArrowX, nArrowY ),
                                    Point( nArrowX+HEAD_ARROWSIZE2, nArrowY ) );
                    pDev->DrawLine( Point( nArrowX, nArrowY ),
                                    Point( nArrowX+HEAD_ARROWSIZE1, nArrowY+HEAD_ARROWSIZE2 ) );
                    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                    pDev->DrawLine( Point( nArrowX+HEAD_ARROWSIZE1, nArrowY+HEAD_ARROWSIZE2 ),
                                    Point( nArrowX+HEAD_ARROWSIZE2, nArrowY ) );
                }
                else
                {
                    pDev->SetLineColor( rStyleSettings.GetLightColor() );
                    pDev->DrawLine( Point( nArrowX, nArrowY+HEAD_ARROWSIZE2 ),
                                    Point( nArrowX+HEAD_ARROWSIZE1, nArrowY ) );
                    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                    pDev->DrawLine( Point( nArrowX, nArrowY+HEAD_ARROWSIZE2 ),
                                    Point( nArrowX+HEAD_ARROWSIZE2, nArrowY+HEAD_ARROWSIZE2 ) );
                    pDev->DrawLine( Point( nArrowX+HEAD_ARROWSIZE2, nArrowY+HEAD_ARROWSIZE2 ),
                                    Point( nArrowX+HEAD_ARROWSIZE1, nArrowY ) );
                }
            }
        }
    }

    // Gegebenenfalls auch UserDraw aufrufen
    if ( nBits & HIB_USERDRAW )
    {
        Region aRegion( aRect );
        if ( pRect )
            aRegion.Intersect( *pRect );
        pDev->SetClipRegion( aRegion );
        UserDrawEvent aODEvt( pDev, aRect, pItem->mnId );
        UserDraw( aODEvt );
        pDev->SetClipRegion();
    }
}

// -----------------------------------------------------------------------

void HeaderBar::ImplDrawItem( sal_uInt16 nPos, sal_Bool bHigh, sal_Bool bDrag,
                              const Rectangle* pRect )
{
    Rectangle aRect = ImplGetItemRect( nPos );
    ImplDrawItem( this, nPos, bHigh, bDrag, aRect, pRect, 0 );
}

// -----------------------------------------------------------------------

void HeaderBar::ImplUpdate( sal_uInt16 nPos, sal_Bool bEnd, sal_Bool bDirect )
{
    if ( IsVisible() && IsUpdateMode() )
    {
        if ( !bDirect )
        {
            Rectangle   aRect;
            size_t      nItemCount = mpItemList->size();
            if ( nPos < nItemCount )
                aRect = ImplGetItemRect( nPos );
            else
            {
                aRect.Bottom() = mnDY-1;
                if ( nItemCount )
                    aRect.Left() = ImplGetItemRect( nItemCount-1 ).Right();
            }
            if ( bEnd )
                aRect.Right() = mnDX-1;
            aRect.Top()     += mnBorderOff1;
            aRect.Bottom()  -= mnBorderOff2;
            Invalidate( aRect );
        }
        else
        {
            for ( size_t i = nPos; i < mpItemList->size(); i++ )
                ImplDrawItem( i );
            if ( bEnd )
            {
                Rectangle aRect = ImplGetItemRect( (sal_uInt16)mpItemList->size() );
                aRect.Left()  = aRect.Right();
                aRect.Right() = mnDX-1;
                if ( aRect.Left() < aRect.Right() )
                {
                    aRect.Top()     += mnBorderOff1;
                    aRect.Bottom()  -= mnBorderOff2;
                    Erase( aRect );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void HeaderBar::ImplStartDrag( const Point& rMousePos, sal_Bool bCommand )
{
    sal_uInt16  nPos;
    sal_uInt16  nHitTest = ImplHitTest( rMousePos, mnMouseOff, nPos );
    if ( nHitTest )
    {
        mbDrag = sal_False;
        ImplHeadItem* pItem = (*mpItemList)[ nPos ];
        if ( nHitTest & HEAD_HITTEST_DIVIDER )
            mbDrag = sal_True;
        else
        {
            if ( ((pItem->mnBits & HIB_CLICKABLE) && !(pItem->mnBits & HIB_FLAT)) ||
                 (mbDragable && !(pItem->mnBits & HIB_FIXEDPOS)) )
            {
                mbItemMode = sal_True;
                mbDrag = sal_True;
                if ( bCommand )
                {
                    if ( mbDragable )
                        mbItemDrag = sal_True;
                    else
                    {
                        mbItemMode = sal_False;
                        mbDrag = sal_False;
                    }
                }
            }
            else
            {
                if ( !bCommand )
                {
                    mnCurItemId = pItem->mnId;
                    Select();
                    mnCurItemId = 0;
                }
            }
        }

        if ( mbDrag )
        {
            mbOutDrag = sal_False;
            mnCurItemId = pItem->mnId;
            mnItemDragPos = nPos;
            StartTracking();
            mnStartPos = rMousePos.X()-mnMouseOff;
            mnDragPos = mnStartPos;
            StartDrag();
            if ( mbItemMode )
                ImplDrawItem( nPos, sal_True, mbItemDrag );
            else
            {
                Rectangle aSizeRect( mnDragPos, 0, mnDragPos, mnDragSize+mnDY );
                ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
            }
        }
        else
            mnMouseOff = 0;
    }
}

// -----------------------------------------------------------------------

void HeaderBar::ImplDrag( const Point& rMousePos )
{
    sal_Bool    bNewOutDrag;
    sal_uInt16  nPos = GetItemPos( mnCurItemId );

    mnDragPos = rMousePos.X()-mnMouseOff;
    if ( mbItemMode )
    {
        Rectangle aItemRect = ImplGetItemRect( nPos );
        if ( aItemRect.IsInside( rMousePos ) )
            bNewOutDrag = sal_False;
        else
            bNewOutDrag = sal_True;

        // Evt. ItemDrag anschalten
        if ( bNewOutDrag && mbDragable && !mbItemDrag &&
             !((*mpItemList)[ nPos ]->mnBits & HIB_FIXEDPOS) )
        {
            if ( (rMousePos.Y() >= aItemRect.Top()) && (rMousePos.Y() <= aItemRect.Bottom()) )
            {
                mbItemDrag = sal_True;
                ImplDrawItem( nPos, sal_True, mbItemDrag );
            }
        }

        sal_uInt16 nOldItemDragPos = mnItemDragPos;
        if ( mbItemDrag )
        {
            if ( (rMousePos.Y() < -HEADERBAR_DRAGOUTOFF) || (rMousePos.Y() > mnDY+HEADERBAR_DRAGOUTOFF) )
                bNewOutDrag = sal_True;
            else
                bNewOutDrag = sal_False;

            if ( bNewOutDrag )
                mnItemDragPos = HEADERBAR_ITEM_NOTFOUND;
            else
            {
                sal_uInt16 nTempId = GetItemId( Point( rMousePos.X(), 2 ) );
                if ( nTempId )
                    mnItemDragPos = GetItemPos( nTempId );
                else
                {
                    if ( rMousePos.X() <= 0 )
                        mnItemDragPos = 0;
                    else
                        mnItemDragPos = GetItemCount()-1;
                }

                // Nicht verschiebbare Items aussparen
                if ( mnItemDragPos < nPos )
                {
                    while ( ((*mpItemList)[ mnItemDragPos ]->mnBits & HIB_FIXEDPOS) &&
                            (mnItemDragPos < nPos) )
                        mnItemDragPos++;
                }
                else if ( mnItemDragPos > nPos )
                {
                    while ( ((*mpItemList)[ mnItemDragPos ]->mnBits & HIB_FIXEDPOS) &&
                            (mnItemDragPos > nPos) )
                        mnItemDragPos--;
                }
            }

            if ( (mnItemDragPos != nOldItemDragPos) &&
                 (nOldItemDragPos != nPos) &&
                 (nOldItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
            {
                ImplInvertDrag( nPos, nOldItemDragPos );
                ImplDrawItem( nOldItemDragPos );
            }
        }

        if ( bNewOutDrag != mbOutDrag )
            ImplDrawItem( nPos, !bNewOutDrag, mbItemDrag );

        if ( mbItemDrag  )
        {
            if ( (mnItemDragPos != nOldItemDragPos) &&
                 (mnItemDragPos != nPos) &&
                 (mnItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
            {
                ImplDrawItem( mnItemDragPos, sal_False, sal_True );
                ImplInvertDrag( nPos, mnItemDragPos );
            }
        }

        mbOutDrag = bNewOutDrag;
    }
    else
    {
        Rectangle aItemRect = ImplGetItemRect( nPos );
        if ( mnDragPos < aItemRect.Left() )
            mnDragPos = aItemRect.Left();
        if ( (mnDragPos < 0) || (mnDragPos > mnDX-1) )
            HideTracking();
        else
        {
            Rectangle aSizeRect( mnDragPos, 0, mnDragPos, mnDragSize+mnDY );
            ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
        }
    }

    Drag();
}

// -----------------------------------------------------------------------

void HeaderBar::ImplEndDrag( sal_Bool bCancel )
{
    HideTracking();

    if ( bCancel || mbOutDrag )
    {
        if ( mbItemMode && (!mbOutDrag || mbItemDrag) )
        {
            sal_uInt16 nPos = GetItemPos( mnCurItemId );
            ImplDrawItem( nPos );
        }

        mnCurItemId = 0;
    }
    else
    {
        sal_uInt16 nPos = GetItemPos( mnCurItemId );
        if ( mbItemMode )
        {
            if ( mbItemDrag )
            {
                Pointer aPointer( POINTER_ARROW );
                SetPointer( aPointer );
                if ( (mnItemDragPos != nPos) &&
                     (mnItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
                {
                    ImplInvertDrag( nPos, mnItemDragPos );
                    MoveItem( mnCurItemId, mnItemDragPos );
                }
                else
                    ImplDrawItem( nPos );
            }
            else
            {
                Select();
                ImplUpdate( nPos );
            }
        }
        else
        {
            long nDelta = mnDragPos - mnStartPos;
            if ( nDelta )
            {
                ImplHeadItem* pItem = (*mpItemList)[ nPos ];
                pItem->mnSize += nDelta;
                ImplUpdate( nPos, sal_True );
            }
        }
    }

    mbDrag          = sal_False;
    EndDrag();
    mnCurItemId     = 0;
    mnItemDragPos   = HEADERBAR_ITEM_NOTFOUND;
    mbOutDrag       = sal_False;
    mbItemMode      = sal_False;
    mbItemDrag      = sal_False;
}

// -----------------------------------------------------------------------

void HeaderBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 2 )
        {
            long    nTemp;
            sal_uInt16  nPos;
            sal_uInt16  nHitTest = ImplHitTest( rMEvt.GetPosPixel(), nTemp, nPos );
            if ( nHitTest )
            {
                ImplHeadItem* pItem = (*mpItemList)[ nPos ];
                if ( nHitTest & HEAD_HITTEST_DIVIDER )
                    mbItemMode = sal_False;
                else
                    mbItemMode = sal_True;
                mnCurItemId = pItem->mnId;
                DoubleClick();
                mbItemMode = sal_False;
                mnCurItemId = 0;
            }
        }
        else
            ImplStartDrag( rMEvt.GetPosPixel(), sal_False );
    }
}

// -----------------------------------------------------------------------

void HeaderBar::MouseMove( const MouseEvent& rMEvt )
{
    long            nTemp1;
    sal_uInt16          nTemp2;
    PointerStyle    eStyle = POINTER_ARROW;
    sal_uInt16          nHitTest = ImplHitTest( rMEvt.GetPosPixel(), nTemp1, nTemp2 );

    if ( nHitTest & HEAD_HITTEST_DIVIDER )
        eStyle = POINTER_HSIZEBAR;
    Pointer aPtr( eStyle );
    SetPointer( aPtr );
}

// -----------------------------------------------------------------------

void HeaderBar::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndDrag( rTEvt.IsTrackingCanceled() );
    else
        ImplDrag( aMousePos );
}

// -----------------------------------------------------------------------

void HeaderBar::Paint( const Rectangle& rRect )
{
    if ( mnBorderOff1 || mnBorderOff2 )
    {
        SetLineColor( GetSettings().GetStyleSettings().GetDarkShadowColor() );
        if ( mnBorderOff1 )
            DrawLine( Point( 0, 0 ), Point( mnDX-1, 0 ) );
        if ( mnBorderOff2 )
            DrawLine( Point( 0, mnDY-1 ), Point( mnDX-1, mnDY-1 ) );
        // #i40393# draw left and right border, if WB_BORDER was set in ImplInit()
        if ( mnBorderOff1 && mnBorderOff2 )
        {
            DrawLine( Point( 0, 0 ), Point( 0, mnDY-1 ) );
            DrawLine( Point( mnDX-1, 0 ), Point( mnDX-1, mnDY-1 ) );
        }
    }

    sal_uInt16 nCurItemPos;
    if ( mbDrag )
        nCurItemPos = GetItemPos( mnCurItemId );
    else
        nCurItemPos = HEADERBAR_ITEM_NOTFOUND;
    sal_uInt16 nItemCount = (sal_uInt16)mpItemList->size();
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
        ImplDrawItem( i, (i == nCurItemPos) ? sal_True : sal_False, sal_False, &rRect );
}

// -----------------------------------------------------------------------

void HeaderBar::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                      sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );
    Font        aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    if ( !(nFlags & WINDOW_DRAW_NOBACKGROUND) )
    {
        pDev->DrawWallpaper( aRect, GetBackground() );
        if ( mnBorderOff1 || mnBorderOff2 )
        {
            pDev->SetLineColor( GetSettings().GetStyleSettings().GetDarkShadowColor() );
            if ( mnBorderOff1 )
                pDev->DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
            if ( mnBorderOff2 )
                pDev->DrawLine( Point( aRect.Left(), aRect.Bottom() ), Point( aRect.Right(), aRect.Bottom() ) );
            // #i40393# draw left and right border, if WB_BORDER was set in ImplInit()
            if ( mnBorderOff1 && mnBorderOff2 )
            {
                pDev->DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() ) );
                pDev->DrawLine( Point( aRect.Right(), aRect.Top() ), Point( aRect.Right(), aRect.Bottom() ) );
            }
        }
    }

    Rectangle aItemRect( aRect );
    size_t nItemCount = mpItemList->size();
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        aItemRect.Left() = aRect.Left()+ImplGetItemPos( i );
        aItemRect.Right() = aItemRect.Left() + (*mpItemList)[ i ]->mnSize - 1;
        // Gegen Ueberlauf auf einigen Systemen testen
        if ( aItemRect.Right() > 16000 )
            aItemRect.Right() = 16000;
        Region aRegion( aRect );
        pDev->SetClipRegion( aRegion );
        ImplDrawItem( pDev, i, sal_False, sal_False, aItemRect, &aRect, nFlags );
        pDev->SetClipRegion();
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void HeaderBar::Resize()
{
    Size aSize = GetOutputSizePixel();
    if ( IsVisible() && (mnDY != aSize.Height()) )
        Invalidate();
    mnDX = aSize.Width();
    mnDY = aSize.Height();
}

// -----------------------------------------------------------------------

void HeaderBar::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.IsMouseEvent() && (rCEvt.GetCommand() == COMMAND_STARTDRAG) && !mbDrag )
    {
        ImplStartDrag( rCEvt.GetMousePosPixel(), sal_True );
        return;
    }

    Window::Command( rCEvt );
}

// -----------------------------------------------------------------------

void HeaderBar::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();

            XubString aStr = GetHelpText( nItemId );
            if ( !aStr.Len() || !(rHEvt.GetMode() & HELPMODE_BALLOON) )
            {
                ImplHeadItem* pItem = (*mpItemList)[ GetItemPos( nItemId ) ];
                // Wir zeigen die Quick-Hilfe nur an, wenn Text nicht
                // vollstaendig sichtbar, ansonsten zeigen wir den Hilfetext
                // an, wenn das Item keinen Text besitzt
                if ( pItem->maOutText != pItem->maText )
                    aStr = pItem->maText;
                else if ( pItem->maText.Len() )
                    aStr.Erase();
            }

            if ( aStr.Len() )
            {
                if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                    Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                else
                    Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            OUString aHelpId( OStringToOUString( GetHelpId( nItemId ), RTL_TEXTENCODING_UTF8 ) );
            if ( !aHelpId.isEmpty() )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( aHelpId, this );
                return;
            }
        }
    }

    Window::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void HeaderBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_ENABLE )
        Invalidate();
    else if ( (nType == STATE_CHANGE_ZOOM) ||
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

void HeaderBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

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

void HeaderBar::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void HeaderBar::StartDrag()
{
    maStartDragHdl.Call( this );
}

// -----------------------------------------------------------------------

void HeaderBar::Drag()
{
    maDragHdl.Call( this );
}

// -----------------------------------------------------------------------

void HeaderBar::EndDrag()
{
    maEndDragHdl.Call( this );
}

// -----------------------------------------------------------------------

void HeaderBar::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void HeaderBar::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void HeaderBar::InsertItem( sal_uInt16 nItemId, const XubString& rText,
                            long nSize, HeaderBarItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "HeaderBar::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == HEADERBAR_ITEM_NOTFOUND,
                "HeaderBar::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    ImplHeadItem* pItem = new ImplHeadItem;
    pItem->mnId         = nItemId;
    pItem->mnBits       = nBits;
    pItem->mnSize       = nSize;
    pItem->maText       = rText;
    if ( nPos < mpItemList->size() ) {
        ImplHeadItemList::iterator it = mpItemList->begin();
        ::std::advance( it, nPos );
        mpItemList->insert( it, pItem );
    } else {
        mpItemList->push_back( pItem );
    }

    // Ausgabe updaten
    ImplUpdate( nPos, sal_True );
}

// -----------------------------------------------------------------------

void HeaderBar::RemoveItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        if ( nPos < mpItemList->size() ) {
            ImplHeadItemList::iterator it = mpItemList->begin();
            ::std::advance( it, nPos );
            delete *it;
            mpItemList->erase( it );
        }
    }
}

// -----------------------------------------------------------------------

void HeaderBar::MoveItem( sal_uInt16 nItemId, sal_uInt16 nNewPos )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        if ( nPos != nNewPos )
        {
            ImplHeadItemList::iterator it = mpItemList->begin();
            ::std::advance( it, nPos );
            ImplHeadItem* pItem = *it;
            mpItemList->erase( it );
            if ( nNewPos < nPos )
                nPos = nNewPos;
            it = mpItemList->begin();
            ::std::advance( it, nPos );
            mpItemList->insert( it, pItem );
        }
    }
}

// -----------------------------------------------------------------------

void HeaderBar::Clear()
{
    // Alle Items loeschen
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        delete (*mpItemList)[ i ];
    }
    mpItemList->clear();

    ImplUpdate( 0, sal_True );
}

// -----------------------------------------------------------------------

void HeaderBar::SetOffset( long nNewOffset )
{
    // Bereich verschieben
    Rectangle aRect( 0, mnBorderOff1, mnDX-1, mnDY-mnBorderOff1-mnBorderOff2-1 );
    long nDelta = mnOffset-nNewOffset;
    mnOffset = nNewOffset;
    Scroll( nDelta, 0, aRect );
}

// -----------------------------------------------------------------------

sal_uInt16 HeaderBar::GetItemCount() const
{
    return (sal_uInt16)mpItemList->size();
}

// -----------------------------------------------------------------------

sal_uInt16 HeaderBar::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        ImplHeadItem* pItem = (*mpItemList)[ i ];
        if ( pItem->mnId == nItemId )
            return (sal_uInt16)i;
    }
    return HEADERBAR_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 HeaderBar::GetItemId( sal_uInt16 nPos ) const
{
    ImplHeadItem* pItem = (nPos < mpItemList->size() ) ? (*mpItemList)[ nPos ] : NULL;
    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 HeaderBar::GetItemId( const Point& rPos ) const
{
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        if ( ImplGetItemRect( i ).IsInside( rPos ) ) {
            return GetItemId( i );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

Rectangle HeaderBar::GetItemRect( sal_uInt16 nItemId ) const
{
    Rectangle aRect;
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        aRect = ImplGetItemRect( nPos );
    return aRect;
}

// -----------------------------------------------------------------------

void HeaderBar::SetItemSize( sal_uInt16 nItemId, long nNewSize )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        ImplHeadItem* pItem = (*mpItemList)[ nPos ];
        if ( pItem->mnSize != nNewSize )
        {
            pItem->mnSize = nNewSize;
            ImplUpdate( nPos, sal_True );
        }
    }
}

// -----------------------------------------------------------------------

long HeaderBar::GetItemSize( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mnSize;
    else
        return 0;
}

// -----------------------------------------------------------------------

void HeaderBar::SetItemBits( sal_uInt16 nItemId, HeaderBarItemBits nNewBits )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        ImplHeadItem* pItem = (*mpItemList)[ nPos ];
        if ( pItem->mnBits != nNewBits )
        {
            pItem->mnBits = nNewBits;
            ImplUpdate( nPos );
        }
    }
}

// -----------------------------------------------------------------------

HeaderBarItemBits HeaderBar::GetItemBits( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

void HeaderBar::SetItemText( sal_uInt16 nItemId, const XubString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        (*mpItemList)[ nPos ]->maText = rText;
        ImplUpdate( nPos );
    }
}

// -----------------------------------------------------------------------

XubString HeaderBar::GetItemText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->maText;
    else
        return String();
}

// -----------------------------------------------------------------------

XubString HeaderBar::GetHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        ImplHeadItem* pItem = (*mpItemList)[ nPos ];
        if ( !pItem->maHelpText.Len() && !pItem->maHelpId.isEmpty() )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }

        return pItem->maHelpText;
    }
    else
        return XubString();
}

// -----------------------------------------------------------------------

OString HeaderBar::GetHelpId( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    OString aRet;
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return (*mpItemList)[ nPos ]->maHelpId;
    return aRet;
}

// -----------------------------------------------------------------------

Size HeaderBar::CalcWindowSizePixel() const
{
    long nMaxImageSize = 0;
    Size aSize( 0, GetTextHeight() );

    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
    {
        ImplHeadItem* pItem = (*mpItemList)[ i ];
        // Image-Groessen beruecksichtigen
        long nImageHeight = pItem->maImage.GetSizePixel().Height();
        if ( !(pItem->mnBits & (HIB_LEFTIMAGE | HIB_RIGHTIMAGE)) && pItem->maText.Len() )
            nImageHeight += aSize.Height();
        if ( nImageHeight > nMaxImageSize )
            nMaxImageSize = nImageHeight;

        // Breite aufaddieren
        aSize.Width() += pItem->mnSize;
    }

    if ( nMaxImageSize > aSize.Height() )
        aSize.Height() = nMaxImageSize;

    // Border aufaddieren
    if ( mbButtonStyle )
        aSize.Height() += 4;
    else
        aSize.Height() += 2;
    aSize.Height() += mnBorderOff1+mnBorderOff2;

    return aSize;
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > HeaderBar::CreateAccessible()
{
    if ( !mxAccessible.is() )
    {
        if ( maCreateAccessibleHdl.IsSet() )
            maCreateAccessibleHdl.Call( this );

        if ( !mxAccessible.is() )
            mxAccessible = Window::CreateAccessible();
    }

    return mxAccessible;
}

void HeaderBar::SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > _xAccessible )
{
    mxAccessible = _xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
