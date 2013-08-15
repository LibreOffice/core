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

#include <tools/debug.hxx>
#include <vcl/builder.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>
#include "valueimp.hxx"

#include <svtools/valueset.hxx>

namespace
{

enum {
    ITEM_OFFSET = 4,
    ITEM_OFFSET_DOUBLE = 6,
    NAME_LINE_OFF_X = 2,
    NAME_LINE_OFF_Y = 2,
    NAME_LINE_HEIGHT = 2,
    NAME_OFFSET = 2,
    SCRBAR_OFFSET = 1,
    SCROLL_OFFSET = 4
};

}

// ------------
// - ValueSet -
// ------------

void ValueSet::ImplInit()
{
    mpNoneItem          = NULL;
    mpScrBar            = NULL;
    mnItemWidth         = 0;
    mnItemHeight        = 0;
    mnTextOffset        = 0;
    mnVisLines          = 0;
    mnLines             = 0;
    mnUserItemWidth     = 0;
    mnUserItemHeight    = 0;
    mnFirstLine         = 0;
    mnSelItemId         = 0;
    mnHighItemId        = 0;
    mnCols              = 0;
    mnCurCol            = 0;
    mnUserCols          = 0;
    mnUserVisLines      = 0;
    mnSpacing           = 0;
    mnFrameStyle        = 0;
    mbFormat            = true;
    mbHighlight         = false;
    mbSelection         = false;
    mbNoSelection       = true;
    mbDrawSelection     = true;
    mbBlackSel          = false;
    mbDoubleSel         = false;
    mbScroll            = false;
    mbFullMode          = true;
    mbEdgeBlending      = false;
    mbHasVisibleItems   = false;

    // #106446#, #106601# force mirroring of virtual device
    maVirDev.EnableRTL( GetParent()->IsRTLEnabled() );

    ImplInitSettings( true, true, true );
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, WinBits nWinStyle, bool bDisableTransientChildren ) :
    Control( pParent, nWinStyle ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeValueSet(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinBits = WB_TABSTOP;

    VclBuilder::stringmap::iterator aFind = rMap.find(OString("border"));
    if (aFind != rMap.end())
    {
        if (toBool(aFind->second))
            nWinBits |= WB_BORDER;
        rMap.erase(aFind);
    }

    return new ValueSet(pParent, nWinBits);
}

// -----------------------------------------------------------------------

ValueSet::ValueSet( Window* pParent, const ResId& rResId, bool bDisableTransientChildren ) :
    Control( pParent, rResId ),
    maVirDev( *this ),
    maColor( COL_TRANSPARENT )
{
    ImplInit();
    mbIsTransientChildrenDisabled = bDisableTransientChildren;
}

// -----------------------------------------------------------------------

ValueSet::~ValueSet()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
          xComponent (GetAccessible(sal_False), ::com::sun::star::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose ();

    delete mpScrBar;
    delete mpNoneItem;

    ImplDeleteItems();
}

// -----------------------------------------------------------------------

void ValueSet::ImplDeleteItems()
{
    const size_t n = mItemList.size();

    for ( size_t i = 0; i < n; ++i )
    {
        ValueSetItem *const pItem = mItemList[i];
        if ( pItem->mbVisible && ImplHasAccessibleListeners() )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;

            aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
        }

        delete pItem;
    }

    mItemList.clear();
}

// -----------------------------------------------------------------------

void ValueSet::ImplInitSettings( bool bFont, bool bForeground, bool bBackground )
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
        else if ( GetStyle() & WB_MENUSTYLEVALUESET )
            aColor = rStyleSettings.GetMenuColor();
        else if ( IsEnabled() && (GetStyle() & WB_FLATVALUESET) )
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
            // adapt the width because of the changed settings
            long nScrBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            mpScrBar->setPosSizePixel( 0, 0, nScrBarWidth, 0, WINDOW_POSSIZE_WIDTH );
        }
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplFormatItem( ValueSetItem* pItem, Rectangle aRect )
{
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
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        if ( pItem == mpNoneItem )
        {
            maVirDev.SetFont( GetFont() );
            maVirDev.SetTextColor( ( nStyle & WB_MENUSTYLEVALUESET ) ? rStyleSettings.GetMenuTextColor() : rStyleSettings.GetWindowTextColor() );
            maVirDev.SetTextFillColor();
            maVirDev.SetFillColor( ( nStyle & WB_MENUSTYLEVALUESET ) ? rStyleSettings.GetMenuColor() : rStyleSettings.GetWindowColor() );
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
            else if ( nStyle & WB_MENUSTYLEVALUESET )
                maVirDev.SetFillColor( rStyleSettings.GetMenuColor() );
            else if ( IsEnabled() )
                maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
            else
                maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
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

                sal_uInt16  nImageStyle  = 0;
                if( !IsEnabled() )
                    nImageStyle  |= IMAGE_DRAW_DISABLE;

                if ( (aImageSize.Width()  > aRectSize.Width()) ||
                     (aImageSize.Height() > aRectSize.Height()) )
                {
                    maVirDev.SetClipRegion( Region( aRect ) );
                    maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle);
                    maVirDev.SetClipRegion();
                }
                else
                    maVirDev.DrawImage( aPos, pItem->maImage, nImageStyle );
            }
        }

        const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

        if(nEdgeBlendingPercent)
        {
            const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
            const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
            const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
            const BitmapEx aBlendFrame(createBlendFrame(aRect.GetSize(), nAlpha, rTopLeft, rBottomRight));

            if(!aBlendFrame.IsEmpty())
            {
                maVirDev.DrawBitmapEx(aRect.TopLeft(), aBlendFrame);
            }
        }
    }
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ValueSet::CreateAccessible()
{
    return new ValueSetAcc( this, mbIsTransientChildrenDisabled );
}

// -----------------------------------------------------------------------

void ValueSet::Format()
{
    Size        aWinSize = GetOutputSizePixel();
    size_t      nItemCount = mItemList.size();
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
    long        nOff;
    long        nNoneHeight;
    long        nNoneSpace;
    ScrollBar*  pDelScrBar = NULL;

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

    // calculate item offset
    if ( nStyle & WB_ITEMBORDER )
    {
        if ( nStyle & WB_DOUBLEBORDER )
            nOff = ITEM_OFFSET_DOUBLE;
        else
            nOff = ITEM_OFFSET;
    }
    else
        nOff = 0;

    // consider size, if NameField does exist
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

    // consider offset and size, if NoneField does exist
    if ( nStyle & WB_NONEFIELD )
    {
        nNoneHeight = nTxtHeight+nOff;
        nNoneSpace = mnSpacing;
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

    // calculate ScrollBar width
    long nScrBarWidth = 0;
    if ( mpScrBar )
        nScrBarWidth = mpScrBar->GetSizePixel().Width()+SCRBAR_OFFSET;

    // calculate number of columns
    if ( !mnUserCols )
    {
        if ( mnUserItemWidth )
        {
            mnCols = (sal_uInt16)((aWinSize.Width()-nScrBarWidth+mnSpacing) / (mnUserItemWidth+mnSpacing));
            if ( !mnCols )
                mnCols = 1;
        }
        else
            mnCols = 1;
    }
    else
        mnCols = mnUserCols;

    // calculate number of rows
    mbScroll = false;
    // Floor( (M+N-1)/N )==Ceiling( M/N )
    mnLines = (static_cast<long>(nItemCount)+mnCols-1) / mnCols;
    if ( !mnLines )
        mnLines = 1;

    long nCalcHeight = aWinSize.Height()-nNoneHeight;
    if ( mnUserVisLines )
        mnVisLines = mnUserVisLines;
    else if ( mnUserItemHeight )
    {
        mnVisLines = (nCalcHeight-nNoneSpace+mnSpacing) / (mnUserItemHeight+mnSpacing);
        if ( !mnVisLines )
            mnVisLines = 1;
    }
    else
        mnVisLines = mnLines;
    if ( mnLines > mnVisLines )
        mbScroll = true;
    if ( mnLines <= mnVisLines )
        mnFirstLine = 0;
    else
    {
        if ( mnFirstLine > (sal_uInt16)(mnLines-mnVisLines) )
            mnFirstLine = (sal_uInt16)(mnLines-mnVisLines);
    }

    // calculate item size
    const long nColSpace  = (mnCols-1)*mnSpacing;
    const long nLineSpace = ((mnVisLines-1)*mnSpacing)+nNoneSpace;
    if ( mnUserItemWidth && !mnUserCols )
    {
        mnItemWidth = mnUserItemWidth;
        if ( mnItemWidth > aWinSize.Width()-nScrBarWidth-nColSpace )
            mnItemWidth = aWinSize.Width()-nScrBarWidth-nColSpace;
    }
    else
        mnItemWidth = (aWinSize.Width()-nScrBarWidth-nColSpace) / mnCols;
    if ( mnUserItemHeight && !mnUserVisLines )
    {
        mnItemHeight = mnUserItemHeight;
        if ( mnItemHeight > nCalcHeight-nNoneSpace )
            mnItemHeight = nCalcHeight-nNoneSpace;
    }
    else
    {
        nCalcHeight -= nLineSpace;
        mnItemHeight = nCalcHeight / mnVisLines;
    }

    // Init VirDev
    maVirDev.SetSettings( GetSettings() );
    maVirDev.SetBackground( GetBackground() );
    maVirDev.SetOutputSizePixel( aWinSize, sal_True );

    // nothing is changed in case of too small items
    if ( (mnItemWidth <= 0) ||
         (mnItemHeight <= (( nStyle & WB_ITEMBORDER ) ? 4 : 2)) ||
         !nItemCount )
    {
        mbHasVisibleItems = false;

        if ( nStyle & WB_NONEFIELD )
        {
            if ( mpNoneItem )
            {
                mpNoneItem->mbVisible = false;
                mpNoneItem->maText = GetText();
            }
        }

        for ( size_t i = 0; i < nItemCount; i++ )
        {
            mItemList[i]->mbVisible = false;
        }

        if ( mpScrBar )
            mpScrBar->Hide();
    }
    else
    {
        mbHasVisibleItems = true;

        // determine Frame-Style
        if ( nStyle & WB_DOUBLEBORDER )
            mnFrameStyle = FRAME_DRAW_DOUBLEIN;
        else
            mnFrameStyle = FRAME_DRAW_IN;

        // determine selected color and width
        // if necessary change the colors, to make the selection
        // better detectable
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aHighColor( rStyleSettings.GetHighlightColor() );
        if ( ((aHighColor.GetRed() > 0x80) || (aHighColor.GetGreen() > 0x80) ||
              (aHighColor.GetBlue() > 0x80)) ||
             ((aHighColor.GetRed() == 0x80) && (aHighColor.GetGreen() == 0x80) &&
              (aHighColor.GetBlue() == 0x80)) )
            mbBlackSel = true;
        else
            mbBlackSel = false;

        // draw the selection with double width if the items are bigger
        if ( (nStyle & WB_DOUBLEBORDER) &&
             ((mnItemWidth >= 25) && (mnItemHeight >= 20)) )
            mbDoubleSel = true;
        else
            mbDoubleSel = false;

        // calculate offsets
        long nStartX;
        long nStartY;
        if ( mbFullMode )
        {
            long nAllItemWidth = (mnItemWidth*mnCols)+nColSpace;
            long nAllItemHeight = (mnItemHeight*mnVisLines)+nNoneHeight+nLineSpace;
            nStartX = (aWinSize.Width()-nScrBarWidth-nAllItemWidth)/2;
            nStartY = (aWinSize.Height()-nAllItemHeight)/2;
        }
        else
        {
            nStartX = 0;
            nStartY = 0;
        }

        // calculate and draw items
        maVirDev.SetLineColor();
        long x = nStartX;
        long y = nStartY;

        // create NoSelection field and show it
        if ( nStyle & WB_NONEFIELD )
        {
            if ( !mpNoneItem )
                mpNoneItem = new ValueSetItem( *this );

            mpNoneItem->mnId            = 0;
            mpNoneItem->meType          = VALUESETITEM_NONE;
            mpNoneItem->mbVisible       = true;
            maNoneItemRect.Left()       = x;
            maNoneItemRect.Top()        = y;
            maNoneItemRect.Right()      = maNoneItemRect.Left()+aWinSize.Width()-x-1;
            maNoneItemRect.Bottom()     = y+nNoneHeight-1;

            ImplFormatItem( mpNoneItem, maNoneItemRect );

            y += nNoneHeight+nNoneSpace;
        }

        // draw items
        sal_uLong nFirstItem = mnFirstLine * mnCols;
        sal_uLong nLastItem = nFirstItem + (mnVisLines * mnCols);

        maItemListRect.Left() = x;
        maItemListRect.Top() = y;
        maItemListRect.Right() = x + mnCols*(mnItemWidth+mnSpacing) - mnSpacing - 1;
        maItemListRect.Bottom() = y + mnVisLines*(mnItemHeight+mnSpacing) - mnSpacing - 1;

        if ( !mbFullMode )
        {
            // If want also draw parts of items in the last line,
            // then we add one more line if parts of these line are
            // visible
            if ( y+(mnVisLines*(mnItemHeight+mnSpacing)) < aWinSize.Height() )
                nLastItem += mnCols;
            maItemListRect.Bottom() = aWinSize.Height() - y;
        }
        for ( size_t i = 0; i < nItemCount; i++ )
        {
            ValueSetItem *const pItem = mItemList[i];

            if ( (i >= nFirstItem) && (i < nLastItem) )
            {
                if( !pItem->mbVisible && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aNewAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->mbVisible = true;
                ImplFormatItem( pItem, Rectangle( Point(x,y), Size(mnItemWidth, mnItemHeight) ) );

                if ( !((i+1) % mnCols) )
                {
                    x = nStartX;
                    y += mnItemHeight+mnSpacing;
                }
                else
                    x += mnItemWidth+mnSpacing;
            }
            else
            {
                if( pItem->mbVisible && ImplHasAccessibleListeners() )
                {
                    ::com::sun::star::uno::Any aOldAny, aNewAny;

                    aOldAny <<= pItem->GetAccessible( mbIsTransientChildrenDisabled );
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::CHILD, aOldAny, aNewAny );
                }

                pItem->mbVisible = false;
            }
        }

        // arrange ScrollBar, set values and show it
        if ( mpScrBar )
        {
            Point   aPos( aWinSize.Width()-nScrBarWidth+SCRBAR_OFFSET, 0 );
            Size    aSize( nScrBarWidth-SCRBAR_OFFSET, aWinSize.Height() );
            // If a none field is visible, then we center the scrollbar
            if ( nStyle & WB_NONEFIELD )
            {
                aPos.Y() = nStartY+nNoneHeight+1;
                aSize.Height() = ((mnItemHeight+mnSpacing)*mnVisLines)-2-mnSpacing;
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

    // waiting for the next since the formatting is finished
    mbFormat = false;

    // delete ScrollBar
    delete pDelScrBar;
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawItemText(const OUString& rText)
{
    if ( !(GetStyle() & WB_NAMEFIELD) )
        return;

    Size    aWinSize = GetOutputSizePixel();
    long    nTxtWidth = GetTextWidth(rText);
    long    nTxtOffset = mnTextOffset;

    // delete rectangle and show text
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

    const bool bFocus = HasFocus();
    const bool bDrawSel = !( (mbNoSelection && !mbHighlight) || (!mbDrawSelection && mbHighlight) );

    if ( !bFocus && !bDrawSel )
    {
        ImplDrawItemText(OUString());
        return;
    }

    ImplDrawSelect( mnSelItemId, bFocus, bDrawSel );
    if (mbHighlight)
    {
        ImplDrawSelect( mnHighItemId, bFocus, bDrawSel );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplDrawSelect( sal_uInt16 nItemId, const bool bFocus, const bool bDrawSel )
{
    ValueSetItem* pItem;
    Rectangle aRect;
    if ( nItemId )
    {
        const size_t nPos = GetItemPos( nItemId );
        pItem = mItemList[ nPos ];
        aRect = ImplGetItemRect( nPos );
    }
    else if ( mpNoneItem )
    {
        pItem = mpNoneItem;
        aRect = maNoneItemRect;
    }
    else if ( bFocus && (pItem = ImplGetFirstItem()) )
    {
        aRect = ImplGetItemRect( 0 );
    }
    else
    {
        return;
    }

    if ( pItem->mbVisible )
    {
        // draw selection
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        Control::SetFillColor();

        Color aDoubleColor( rStyleSettings.GetHighlightColor() );
        Color aSingleColor( rStyleSettings.GetHighlightTextColor() );
        if( !mbDoubleSel )
        {
            /*
            *  #99777# contrast enhancement for thin mode
            */
            const Wallpaper& rWall = GetDisplayBackground();
            if( ! rWall.IsBitmap() && ! rWall.IsGradient() )
            {
                const Color& rBack = rWall.GetColor();
                if( rBack.IsDark() && ! aDoubleColor.IsBright() )
                {
                    aDoubleColor = Color( COL_WHITE );
                    aSingleColor = Color( COL_BLACK );
                }
                else if( rBack.IsBright() && ! aDoubleColor.IsDark() )
                {
                    aDoubleColor = Color( COL_BLACK );
                    aSingleColor = Color( COL_WHITE );
                }
            }
        }

        // specify selection output
        WinBits nStyle = GetStyle();
        if ( nStyle & WB_MENUSTYLEVALUESET )
        {
            if ( bFocus )
                ShowFocus( aRect );

            if ( bDrawSel )
            {
                SetLineColor( mbBlackSel ? Color( COL_BLACK ) : aDoubleColor );
                DrawRect( aRect );
            }
        }
        else if ( nStyle & WB_RADIOSEL )
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

            aRect.Left()++;
            aRect.Top()++;
            aRect.Right()--;
            aRect.Bottom()--;

            if ( bDrawSel )
            {
                SetLineColor( aDoubleColor );
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
                SetLineColor( mbBlackSel ? Color( COL_BLACK ) : aDoubleColor );
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
                SetLineColor( mbBlackSel ? Color( COL_WHITE ) : aSingleColor );
            }
            else
            {
                SetLineColor( Color( COL_LIGHTGRAY ) );
            }
            DrawRect( aRect2 );

            if ( bFocus )
                ShowFocus( aRect2 );
        }

        ImplDrawItemText(pItem->maText);
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplHideSelect( sal_uInt16 nItemId )
{
    Rectangle aRect;

    const size_t nItemPos = GetItemPos( nItemId );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
        if ( !mItemList[nItemPos]->mbVisible )
        {
            return;
        }
        aRect = ImplGetItemRect(nItemPos);
    }
    else
    {
        if ( !mpNoneItem )
        {
            return;
        }
        aRect = maNoneItemRect;
    }

    HideFocus();
    const Point aPos  = aRect.TopLeft();
    const Size  aSize = aRect.GetSize();
    DrawOutDev( aPos, aSize, aPos, aSize, maVirDev );
}

// -----------------------------------------------------------------------

void ValueSet::ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection )
{
    if ( mnHighItemId != nItemId )
    {
        // remember the old item to delete the previous selection
        sal_uInt16 nOldItem = mnHighItemId;
        mnHighItemId = nItemId;

        // don't draw the selection if nothing is selected
        if ( !bIsSelection && mbNoSelection )
            mbDrawSelection = false;

        // remove the old selection and draw the new one
        ImplHideSelect( nOldItem );
        ImplDrawSelect();
        mbDrawSelection = true;
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

    // draw parting line to the Namefield
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

bool ValueSet::ImplScroll( const Point& rPos )
{
    if ( !mbScroll || !maItemListRect.IsInside(rPos) )
        return false;

    const long nScrollOffset = (mnItemHeight <= 16) ? SCROLL_OFFSET/2 : SCROLL_OFFSET;
    bool bScroll = false;

    if ( rPos.Y() <= maItemListRect.Top()+nScrollOffset )
    {
        if ( mnFirstLine > 0 )
        {
            --mnFirstLine;
            bScroll = true;
        }
    }
    else if ( rPos.Y() >= maItemListRect.Bottom()-nScrollOffset )
    {
        if ( mnFirstLine < static_cast<sal_uInt16>(mnLines-mnVisLines) )
        {
            ++mnFirstLine;
            bScroll = true;
        }
    }

    if ( !bScroll )
        return false;

    mbFormat = true;
    ImplDraw();
    return true;
}

// -----------------------------------------------------------------------

size_t ValueSet::ImplGetItem( const Point& rPos, bool bMove ) const
{
    if ( !mbHasVisibleItems )
    {
        return VALUESET_ITEM_NOTFOUND;
    }

    if ( mpNoneItem && maNoneItemRect.IsInside( rPos ) )
    {
        return VALUESET_ITEM_NONEITEM;
    }

    if ( maItemListRect.IsInside( rPos ) )
    {
        const int xc = rPos.X()-maItemListRect.Left();
        const int yc = rPos.Y()-maItemListRect.Top();
        // The point is inside the area of item list,
        // let's find the containing item.
        const int col = xc/(mnItemWidth+mnSpacing);
        const int x = xc%(mnItemWidth+mnSpacing);
        const int row = yc/(mnItemHeight+mnSpacing);
        const int y = yc%(mnItemHeight+mnSpacing);

        if (x<mnItemWidth && y<mnItemHeight)
        {
            // the point is inside item rect and not inside spacing
            const size_t item = (mnFirstLine+row)*mnCols+col;
            if (item < mItemList.size())
            {
                return item;
            }
        }

        // return the previously selected item if spacing is set and
        // the mouse hasn't left the window yet
        if ( bMove && mnSpacing && mnHighItemId )
        {
            return GetItemPos( mnHighItemId );
        }
    }

    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetItem( size_t nPos )
{
    if ( nPos == VALUESET_ITEM_NONEITEM )
        return mpNoneItem;
    else
        return ( nPos < mItemList.size() ) ? mItemList[nPos] : NULL;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetFirstItem()
{
    return mItemList.size() ? mItemList[0] : NULL;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::ImplGetVisibleItemCount() const
{
    sal_uInt16 nRet = 0;
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        if ( mItemList[n]->mbVisible )
            ++nRet;
    }

    return nRet;
}

// -----------------------------------------------------------------------

ValueSetItem* ValueSet::ImplGetVisibleItem( sal_uInt16 nVisiblePos )
{
    const size_t nItemCount = mItemList.size();

    for ( size_t n = 0; n < nItemCount; ++n )
    {
        ValueSetItem *const pItem = mItemList[n];

        if ( pItem->mbVisible && !nVisiblePos-- )
            return pItem;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void ValueSet::ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );

    if( pAcc )
        pAcc->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

// -----------------------------------------------------------------------

bool ValueSet::ImplHasAccessibleListeners()
{
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    return( pAcc && pAcc->HasAccessibleListeners() );
}

// -----------------------------------------------------------------------

IMPL_LINK( ValueSet,ImplScrollHdl, ScrollBar*, pScrollBar )
{
    sal_uInt16 nNewFirstLine = (sal_uInt16)pScrollBar->GetThumbPos();
    if ( nNewFirstLine != mnFirstLine )
    {
        mnFirstLine = nNewFirstLine;
        mbFormat = true;
        ImplDraw();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ValueSet, ImplTimerHdl)
{
    ImplTracking( GetPointerPosPixel(), true );
    return 0;
}

// -----------------------------------------------------------------------

void ValueSet::ImplTracking( const Point& rPos, bool bRepeat )
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
    if ( pItem )
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
            mbHighlight = true;

        ImplHighlightItem( pItem->mnId );
    }
    else
    {
        if( GetStyle() & WB_MENUSTYLEVALUESET )
            mbHighlight = true;

        ImplHighlightItem( mnSelItemId, false );
    }
}

// -----------------------------------------------------------------------

void ValueSet::ImplEndTracking( const Point& rPos, bool bCancel )
{
    ValueSetItem* pItem;

    // restore the old status in case of termination
    if ( bCancel )
        pItem = NULL;
    else
        pItem = ImplGetItem( ImplGetItem( rPos ) );

    if ( pItem )
    {
        SelectItem( pItem->mnId );
        if ( !mbSelection && !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        mbHighlight = false;
        mbSelection = false;
        Select();
    }
    else
    {
        ImplHighlightItem( mnSelItemId, false );
        mbHighlight = false;
        mbSelection = false;
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
            mbHighlight = true;
            if ( pItem )
            {
                mnHighItemId = mnSelItemId;
                ImplHighlightItem( pItem->mnId );
            }

            return;
        }
        else
        {
            if ( pItem && !rMEvt.IsMod2() )
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    mbHighlight  = true;
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
    // because of SelectionMode
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( rMEvt.GetPosPixel(), false );
    else
        Control::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void ValueSet::MouseMove( const MouseEvent& rMEvt )
{
    // because of SelectionMode
    if ( mbSelection || (GetStyle() & WB_MENUSTYLEVALUESET) )
        ImplTracking( rMEvt.GetPosPixel(), false );
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
    size_t nLastItem = mItemList.size();

    if ( !nLastItem || !ImplGetFirstItem() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    if ( mbFormat )
        Format();

    --nLastItem;
    const size_t nCurPos = mnSelItemId ? GetItemPos( mnSelItemId )
                                       : mpNoneItem ? VALUESET_ITEM_NONEITEM : 0;
    size_t nItemPos = VALUESET_ITEM_NOTFOUND;
    size_t nVStep = mnCols;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            nItemPos = mpNoneItem ? VALUESET_ITEM_NONEITEM : 0;
            break;

        case KEY_END:
            nItemPos = nLastItem;
            break;

        case KEY_LEFT:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos)
                {
                    nItemPos = nCurPos-1;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
            }
            break;

        case KEY_RIGHT:
            if (nCurPos < nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = 0;
                }
                else
                {
                    nItemPos = nCurPos+1;
                }
            }
            break;

        case KEY_PAGEUP:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_UP:
            if (nCurPos != VALUESET_ITEM_NONEITEM)
            {
                if (nCurPos == nLastItem)
                {
                    const size_t nCol = nLastItem % mnCols;
                    if (nCol < mnCurCol)
                    {
                        // Move to previous row/page, keeping the old column
                        nVStep -= mnCurCol - nCol;
                    }
                }
                if (nCurPos >= nVStep)
                {
                    // Go up of a whole page
                    nItemPos = nCurPos-nVStep;
                }
                else if (mpNoneItem)
                {
                    nItemPos = VALUESET_ITEM_NONEITEM;
                }
                else if (nCurPos > mnCols)
                {
                    // Go to same column in first row
                    nItemPos = nCurPos % mnCols;
                }
            }
            break;

        case KEY_PAGEDOWN:
            if (rKEvt.GetKeyCode().IsShift() || rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                Control::KeyInput( rKEvt );
                return;
            }
            nVStep *= mnVisLines;
            // intentional fall-through
        case KEY_DOWN:
            if (nCurPos != nLastItem)
            {
                if (nCurPos == VALUESET_ITEM_NONEITEM)
                {
                    nItemPos = nVStep-mnCols+mnCurCol;
                }
                else
                {
                    nItemPos = nCurPos+nVStep;
                }
                if (nItemPos > nLastItem)
                {
                    nItemPos = nLastItem;
                }
            }
            break;

        case KEY_RETURN:
            if (GetStyle() & WB_NO_DIRECTSELECT)
            {
                Select();
                break;
            }
            // intentional fall-through
        default:
            Control::KeyInput( rKEvt );
            return;
    }

    // This point is reached only if key travelling was used,
    // in which case selection mode should be switched off
    EndSelection();

    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
    {
        if ( nItemPos!=VALUESET_ITEM_NONEITEM && nItemPos<nLastItem )
        {
            // update current column only in case of a new position
            // which is also not a "specially" handled one.
            mnCurCol = nItemPos % mnCols;
        }
        const sal_uInt16 nItemId = (nItemPos != VALUESET_ITEM_NONEITEM) ? GetItemId( nItemPos ) : 0;
        if ( nItemId != mnSelItemId )
        {
            SelectItem( nItemId );
            if (!(GetStyle() & WB_NO_DIRECTSELECT))
            {
                // select only if WB_NO_DIRECTSELECT is not set
                Select();
            }
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

void ValueSet::Paint( const Rectangle& )
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
    OSL_TRACE ("value set getting focus");
    ImplDrawSelect();
    Control::GetFocus();

    // Tell the accessible object that we got the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->GetFocus();
}

// -----------------------------------------------------------------------

void ValueSet::LoseFocus()
{
    OSL_TRACE ("value set losing focus");
    if ( mbNoSelection && mnSelItemId )
        ImplHideSelect( mnSelItemId );
    else
        HideFocus();
    Control::LoseFocus();

    // Tell the accessible object that we lost the focus.
    ValueSetAcc* pAcc = ValueSetAcc::getImplementation( GetAccessible( sal_False ) );
    if( pAcc )
        pAcc->LoseFocus();
}

// -----------------------------------------------------------------------

void ValueSet::Resize()
{
    mbFormat = true;
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
        size_t nItemPos = ImplGetItem( aPos );
        if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            Rectangle aItemRect = ImplGetItemRect( nItemPos );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            Help::ShowQuickHelp( this, aItemRect, GetItemText( ImplGetItem( nItemPos )->mnId ) );
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
            ImplFormatItem( mpNoneItem, maNoneItemRect );
            Invalidate( maNoneItemRect );
        }
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
        mbFormat = true;
        ImplInitSettings( false, false, true );
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
        mbFormat = true;
        ImplInitSettings( true, true, true );
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

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    ImplInsertItem( pItem, nPos );
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    ImplInsertItem( pItem, nPos );
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                           const OUString& rText, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_IMAGE;
    pItem->maImage  = rImage;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, const Color& rColor,
                           const OUString& rText, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_COLOR;
    pItem->maColor  = rColor;
    pItem->maText   = rText;
    ImplInsertItem( pItem, nPos );
}

// -----------------------------------------------------------------------

void ValueSet::InsertItem( sal_uInt16 nItemId, size_t nPos )
{
    ValueSetItem* pItem = new ValueSetItem( *this );
    pItem->mnId     = nItemId;
    pItem->meType   = VALUESETITEM_USERDRAW;
    ImplInsertItem( pItem, nPos );
}

// -----------------------------------------------------------------------

void ValueSet::ImplInsertItem( ValueSetItem *const pItem, const size_t nPos )
{
    DBG_ASSERT( pItem->mnId, "ValueSet::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( pItem->mnId ) == VALUESET_ITEM_NOTFOUND,
                "ValueSet::InsertItem(): ItemId already exists" );

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        mItemList.insert( it, pItem );
    } else {
        mItemList.push_back( pItem );
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

Rectangle ValueSet::ImplGetItemRect( size_t nPos ) const
{
    const size_t nVisibleBegin = static_cast<size_t>(mnFirstLine)*mnCols;
    const size_t nVisibleEnd = nVisibleBegin + static_cast<size_t>(mnVisLines)*mnCols;

    // Check if the item is inside the range of the displayed ones,
    // taking into account that last row could be incomplete
    if ( nPos<nVisibleBegin || nPos>=nVisibleEnd || nPos>=mItemList.size() )
        return Rectangle();

    nPos -= nVisibleBegin;

    const size_t row = nPos/mnCols;
    const size_t col = nPos%mnCols;
    const long x = maItemListRect.Left()+col*(mnItemWidth+mnSpacing);
    const long y = maItemListRect.Top()+row*(mnItemHeight+mnSpacing);

    return Rectangle( Point(x, y), Size(mnItemWidth, mnItemHeight) );
}

// -----------------------------------------------------------------------

void ValueSet::RemoveItem( sal_uInt16 nItemId )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    if ( nPos < mItemList.size() ) {
        ValueItemList::iterator it = mItemList.begin();
        ::std::advance( it, nPos );
        delete *it;
        mItemList.erase( it );
    }

    // reset variables
    if ( (mnHighItemId == nItemId) || (mnSelItemId == nItemId) )
    {
        mnCurCol        = 0;
        mnHighItemId    = 0;
        mnSelItemId     = 0;
        mbNoSelection   = true;
    }

    queue_resize();

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ValueSet::Clear()
{
    ImplDeleteItems();

    // reset variables
    mnFirstLine     = 0;
    mnCurCol        = 0;
    mnHighItemId    = 0;
    mnSelItemId     = 0;
    mbNoSelection   = true;

    mbFormat = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
}

// -----------------------------------------------------------------------

size_t ValueSet::GetItemCount() const
{
    return mItemList.size();
}

// -----------------------------------------------------------------------

size_t ValueSet::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mItemList.size(); i < n; ++i ) {
        if ( mItemList[i]->mnId == nItemId ) {
            return i;
        }
    }
    return VALUESET_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::GetItemId( size_t nPos ) const
{
    return ( nPos < mItemList.size() ) ? mItemList[nPos]->mnId : 0 ;
}

// -----------------------------------------------------------------------

sal_uInt16 ValueSet::GetItemId( const Point& rPos ) const
{
    size_t nItemPos = ImplGetItem( rPos );
    if ( nItemPos != VALUESET_ITEM_NOTFOUND )
        return GetItemId( nItemPos );

    return 0;
}

// -----------------------------------------------------------------------

Rectangle ValueSet::GetItemRect( sal_uInt16 nItemId ) const
{
    const size_t nPos = GetItemPos( nItemId );

    if ( nPos!=VALUESET_ITEM_NOTFOUND && mItemList[nPos]->mbVisible )
        return ImplGetItemRect( nPos );

    return Rectangle();
}

// -----------------------------------------------------------------------

void ValueSet::EnableFullItemMode( bool bFullMode )
{
    mbFullMode = bFullMode;
}

// -----------------------------------------------------------------------

void ValueSet::SetColCount( sal_uInt16 nNewCols )
{
    if ( mnUserCols != nNewCols )
    {
        mnUserCols = nNewCols;
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetLineCount( sal_uInt16 nNewLines )
{
    if ( mnUserVisLines != nNewLines )
    {
        mnUserVisLines = nNewLines;
        mbFormat = true;
        queue_resize();
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
        mbFormat = true;
        queue_resize();
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
        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::SelectItem( sal_uInt16 nItemId )
{
    size_t nItemPos = 0;

    if ( nItemId )
    {
        nItemPos = GetItemPos( nItemId );
        if ( nItemPos == VALUESET_ITEM_NOTFOUND )
            return;
    }

    if ( (mnSelItemId != nItemId) || mbNoSelection )
    {
        sal_uInt16 nOldItem = mnSelItemId ? mnSelItemId : 1;
        mnSelItemId = nItemId;
        mbNoSelection = false;

        bool bNewOut = !mbFormat && IsReallyVisible() && IsUpdateMode();
        bool bNewLine = false;

        // if necessary scroll to the visible area
        if ( mbScroll && nItemId )
        {
            sal_uInt16 nNewLine = (sal_uInt16)(nItemPos / mnCols);
            if ( nNewLine < mnFirstLine )
            {
                mnFirstLine = nNewLine;
                bNewLine = true;
            }
            else if ( nNewLine > (sal_uInt16)(mnFirstLine+mnVisLines-1) )
            {
                mnFirstLine = (sal_uInt16)(nNewLine-mnVisLines+1);
                bNewLine = true;
            }
        }

        if ( bNewOut )
        {
            if ( bNewLine )
            {
                // redraw everything if the visible area has changed
                mbFormat = true;
                ImplDraw();
            }
            else
            {
                // remove old selection and draw the new one
                ImplHideSelect( nOldItem );
                ImplDrawSelect();
            }
        }

        if( ImplHasAccessibleListeners() )
        {
            // focus event (deselect)
            if( nOldItem )
            {
                const size_t nPos = GetItemPos( nItemId );

                if( nPos != VALUESET_ITEM_NOTFOUND )
                {
                    ValueItemAcc* pItemAcc = ValueItemAcc::getImplementation(
                        mItemList[nPos]->GetAccessible( mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;
                        if( !mbIsTransientChildrenDisabled )
                        {
                            aOldAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                                static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                            ImplFireAccessibleEvent (::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                        }
                    }
                }
            }

            // focus event (select)
            const size_t nPos = GetItemPos( mnSelItemId );

            ValueSetItem* pItem;
            if( nPos != VALUESET_ITEM_NOTFOUND )
                pItem = mItemList[nPos];
            else
                pItem = mpNoneItem;

            ValueItemAcc* pItemAcc = NULL;
            if (pItem != NULL)
                pItemAcc = ValueItemAcc::getImplementation( pItem->GetAccessible( mbIsTransientChildrenDisabled ) );

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
        maHighlightHdl.Call(this);
    }
}

// -----------------------------------------------------------------------

void ValueSet::SetNoSelection()
{
    mbNoSelection   = true;
    mbHighlight     = false;
    mbSelection     = false;

    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos];
    pItem->meType  = VALUESETITEM_IMAGE;
    pItem->maImage = rImage;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const Rectangle aRect = ImplGetItemRect(nPos);
        ImplFormatItem( pItem, aRect );
        Invalidate( aRect );
    }
    else
        mbFormat = true;
}

// -----------------------------------------------------------------------

Image ValueSet::GetItemImage( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemColor( sal_uInt16 nItemId, const Color& rColor )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos];
    pItem->meType  = VALUESETITEM_COLOR;
    pItem->maColor = rColor;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        const Rectangle aRect = ImplGetItemRect(nPos);
        ImplFormatItem( pItem, aRect );
        Invalidate( aRect );
    }
    else
        mbFormat = true;
}

// -----------------------------------------------------------------------

Color ValueSet::GetItemColor( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maColor;
    else
        return Color();
}

// -----------------------------------------------------------------------

void ValueSet::SetItemData( sal_uInt16 nItemId, void* pData )
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;

    ValueSetItem* pItem = mItemList[nPos];
    pItem->mpData = pData;

    if ( pItem->meType == VALUESETITEM_USERDRAW )
    {
        if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
        {
            const Rectangle aRect = ImplGetItemRect(nPos);
            ImplFormatItem( pItem, aRect );
            Invalidate( aRect );
        }
        else
            mbFormat = true;
    }
}

// -----------------------------------------------------------------------

void* ValueSet::GetItemData( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->mpData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ValueSet::SetItemText(sal_uInt16 nItemId, const OUString& rText)
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos == VALUESET_ITEM_NOTFOUND )
        return;


    ValueSetItem* pItem = mItemList[nPos];

    // Remember old and new name for accessibility event.
    ::com::sun::star::uno::Any aOldName, aNewName;
    OUString sString (pItem->maText);
    aOldName <<= sString;
    sString = rText;
    aNewName <<= sString;

    pItem->maText = rText;

    if ( !mbFormat && IsReallyVisible() && IsUpdateMode() )
    {
        sal_uInt16 nTempId = mnSelItemId;

        if ( mbHighlight )
            nTempId = mnHighItemId;

        if ( nTempId == nItemId )
            ImplDrawItemText(pItem->maText);
    }

    if (ImplHasAccessibleListeners())
    {
        ::com::sun::star::uno::Reference<
              ::com::sun::star::accessibility::XAccessible> xAccessible (
                  pItem->GetAccessible( mbIsTransientChildrenDisabled ) );
        static_cast<ValueItemAcc*>(xAccessible.get())->FireAccessibleEvent (
            ::com::sun::star::accessibility::AccessibleEventId::NAME_CHANGED,
            aOldName, aNewName);
    }
}

// -----------------------------------------------------------------------

OUString ValueSet::GetItemText( sal_uInt16 nItemId ) const
{
    size_t nPos = GetItemPos( nItemId );

    if ( nPos != VALUESET_ITEM_NOTFOUND )
        return mItemList[nPos]->maText;

    return OUString();
}

// -----------------------------------------------------------------------

void ValueSet::SetColor( const Color& rColor )
{
    maColor     = rColor;
    mbFormat    = true;
    if ( IsReallyVisible() && IsUpdateMode() )
        ImplDraw();
}

// -----------------------------------------------------------------------

void ValueSet::SetExtraSpacing( sal_uInt16 nNewSpacing )
{
    if ( GetStyle() & WB_ITEMBORDER )
    {
        mnSpacing = nNewSpacing;

        mbFormat = true;
        queue_resize();
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ValueSet::StartSelection()
{
    mbHighlight     = true;
    mbSelection     = true;
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
        mbHighlight = false;
    }
    mbSelection = false;
}

// -----------------------------------------------------------------------

bool ValueSet::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return false;

    // if necessary abort an existing action
    EndSelection();

    // Check out if the clicked on page is selected. If this is not the
    // case set it as the current item. We only check mouse actions since
    // drag-and-drop can also be triggered by the keyboard
    sal_uInt16 nSelId;
    if ( rCEvt.IsMouseEvent() )
        nSelId = GetItemId( rCEvt.GetMousePosPixel() );
    else
        nSelId = mnSelItemId;

    // don't activate dragging if no item was clicked on
    if ( !nSelId )
        return false;

    // Check out if the page was selected. If not set as current page and
    // call select.
    if ( nSelId != mnSelItemId )
    {
        SelectItem( nSelId );
        Update();
        Select();
    }

    Region aRegion;

    // assign region
    rRegion = aRegion;

    return true;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcWindowSizePixel( const Size& rItemSize, sal_uInt16 nDesireCols,
                                    sal_uInt16 nDesireLines ) const
{
    size_t nCalcCols = nDesireCols;
    size_t nCalcLines = nDesireLines;

    if ( !nCalcCols )
    {
        if ( mnUserCols )
            nCalcCols = mnUserCols;
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
                // Floor( (M+N-1)/N )==Ceiling( M/N )
                nCalcLines = (mItemList.size()+nCalcCols-1) / nCalcCols;
                if ( !nCalcLines )
                    nCalcLines = 1;
            }
        }
    }

    Size        aSize( rItemSize.Width()*nCalcCols, rItemSize.Height()*nCalcLines );
    WinBits     nStyle = GetStyle();
    long        nTxtHeight = GetTextHeight();
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
        aSize.Width()  += mnSpacing*(nCalcCols-1);
        aSize.Height() += mnSpacing*(nCalcLines-1);
    }

    if ( nStyle & WB_NAMEFIELD )
    {
        aSize.Height() += nTxtHeight + NAME_OFFSET;
        if ( !(nStyle & WB_FLATVALUESET) )
            aSize.Height() += NAME_LINE_HEIGHT+NAME_LINE_OFF_Y;
    }

    if ( nStyle & WB_NONEFIELD )
    {
        aSize.Height() += nTxtHeight + n + mnSpacing;
        if ( nStyle & WB_RADIOSEL )
            aSize.Height() += 8;
    }

    // sum possible ScrollBar width
    aSize.Width() += GetScrollWidth();

    return aSize;
}

// -----------------------------------------------------------------------

Size ValueSet::CalcItemSizePixel( const Size& rItemSize, bool bOut ) const
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

void ValueSet::SetHighlightHdl( const Link& rLink )
{
    maHighlightHdl = rLink;
}

Size ValueSet::GetOptimalSize() const
{
    Size aLargestItemSize;

    for (size_t i = 0, n = mItemList.size(); i < n; ++i)
    {
        const ValueSetItem* pItem = mItemList[i];
        if (!pItem->mbVisible)
            continue;

        if (pItem->meType != VALUESETITEM_IMAGE)
        {
            //handle determining an optimal size for this case
            continue;
        }

        Size aImageSize = pItem->maImage.GetSizePixel();
        aLargestItemSize.Width() = std::max(aLargestItemSize.Width(), aImageSize.Width());
        aLargestItemSize.Height() = std::max(aLargestItemSize.Height(), aImageSize.Height());
    }

    return CalcWindowSizePixel(aLargestItemSize);
}

// -----------------------------------------------------------------------

void ValueSet::SetEdgeBlending(bool bNew)
{
    if(mbEdgeBlending != bNew)
    {
        mbEdgeBlending = bNew;
        mbFormat = true;

        if(IsReallyVisible() && IsUpdateMode())
        {
            Invalidate();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
