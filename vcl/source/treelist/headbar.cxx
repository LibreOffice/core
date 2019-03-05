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

#include <vcl/headbar.hxx>
#include <tools/debug.hxx>

#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

class ImplHeadItem
{
public:
    sal_uInt16          mnId;
    HeaderBarItemBits   mnBits;
    long                mnSize;
    OString const       maHelpId;
    Image const         maImage;
    OUString            maOutText;
    OUString            maText;
    OUString            maHelpText;
};

#define HEAD_ARROWSIZE1             4
#define HEAD_ARROWSIZE2             7

#define HEADERBAR_TEXTOFF           2
#define HEADERBAR_ARROWOFF          5
#define HEADERBAR_SPLITOFF          3

#define HEADERBAR_DRAGOUTOFF        15

#define HEAD_HITTEST_ITEM           (sal_uInt16(0x0001))
#define HEAD_HITTEST_DIVIDER        (sal_uInt16(0x0002))

void HeaderBar::ImplInit( WinBits nWinStyle )
{
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
    mbDrag          = false;
    mbItemDrag      = false;
    mbOutDrag       = false;
    mbItemMode      = false;

    // evaluate StyleBits
    if ( nWinStyle & WB_DRAG )
        mbDragable = true;
    else
        mbDragable = false;
    if ( nWinStyle & WB_BUTTONSTYLE )
        mbButtonStyle = true;
    else
        mbButtonStyle = false;
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

    ImplInitSettings( true, true, true );
}

HeaderBar::HeaderBar(vcl::Window* pParent, WinBits nWinStyle)
    : Window(pParent, nWinStyle & WB_3DLOOK)
{
    SetType(WindowType::HEADERBAR);
    ImplInit(nWinStyle);
    SetSizePixel( CalcWindowSizePixel() );
}

Size HeaderBar::GetOptimalSize() const
{
    return CalcWindowSizePixel();
}

HeaderBar::~HeaderBar() = default;

void HeaderBar::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    ApplyControlFont(rRenderContext, rStyleSettings.GetToolFont());

    ApplyControlForeground(rRenderContext, rStyleSettings.GetButtonTextColor());
    SetTextFillColor();

    ApplyControlBackground(rRenderContext, rStyleSettings.GetFaceColor());
}

void HeaderBar::ImplInitSettings(bool bFont, bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
        ApplyControlFont(*this, rStyleSettings.GetToolFont());

    if (bForeground || bFont)
    {
        ApplyControlForeground(*this, rStyleSettings.GetButtonTextColor());
        SetTextFillColor();
    }

    if (bBackground)
        ApplyControlBackground(*this, rStyleSettings.GetFaceColor());
}

long HeaderBar::ImplGetItemPos( sal_uInt16 nPos ) const
{
    long nX = -mnOffset;
    for ( size_t i = 0; i < nPos; i++ )
        nX += mvItemList[ i ]->mnSize;
    return nX;
}

tools::Rectangle HeaderBar::ImplGetItemRect( sal_uInt16 nPos ) const
{
    tools::Rectangle aRect( ImplGetItemPos( nPos ), 0, 0, mnDY-1 );
    aRect.SetRight( aRect.Left() + mvItemList[ nPos ]->mnSize - 1 );
    // check for overflow on various systems
    if ( aRect.Right() > 16000 )
        aRect.SetRight( 16000 );
    return aRect;
}

sal_uInt16 HeaderBar::ImplHitTest( const Point& rPos,
                               long& nMouseOff, sal_uInt16& nPos ) const
{
    size_t          nCount = static_cast<sal_uInt16>(mvItemList.size());
    bool            bLastFixed = true;
    long            nX = -mnOffset;

    for ( size_t i = 0; i < nCount; i++ )
    {
        auto& pItem = mvItemList[ i ];

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

                if ( !(pItem->mnBits & HeaderBarItemBits::FIXED) && (rPos.X() >= (nX+pItem->mnSize-HEADERBAR_SPLITOFF)) )
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

        bLastFixed = static_cast<bool>(pItem->mnBits & HeaderBarItemBits::FIXED);

        nX += pItem->mnSize;
    }

    if ( !bLastFixed )
    {
        auto& pItem = mvItemList[ nCount-1 ];
        if ( (pItem->mnSize < 4)  && (rPos.X() < (nX+HEADERBAR_SPLITOFF)) )
        {
            nPos = nCount-1;
            nMouseOff = rPos.X()-nX+1;
            return HEAD_HITTEST_DIVIDER;
        }
    }

    return 0;
}

void HeaderBar::ImplInvertDrag( sal_uInt16 nStartPos, sal_uInt16 nEndPos )
{
    tools::Rectangle aRect1 = ImplGetItemRect( nStartPos );
    tools::Rectangle aRect2 = ImplGetItemRect( nEndPos );
    Point     aStartPos = aRect1.Center();
    Point     aEndPos = aStartPos;
    tools::Rectangle aStartRect( aStartPos.X()-2, aStartPos.Y()-2,
                          aStartPos.X()+2, aStartPos.Y()+2 );

    if ( nEndPos > nStartPos )
    {
        aStartPos.AdjustX(3 );
        aEndPos.setX( aRect2.Right()-6 );
    }
    else
    {
        aStartPos.AdjustX( -3 );
        aEndPos.setX( aRect2.Left()+6 );
    }

    SetRasterOp( RasterOp::Invert );
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
    SetRasterOp( RasterOp::OverPaint );
}

void HeaderBar::ImplDrawItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, bool bHigh,
                             const tools::Rectangle& rItemRect, const tools::Rectangle* pRect )
{
    ImplControlValue aControlValue(0);
    tools::Rectangle aCtrlRegion;
    ControlState nState(ControlState::NONE);

    tools::Rectangle aRect = rItemRect;

    // do not display if there is no space
    if (aRect.GetWidth() <= 1)
        return;

    // check of rectangle is visible
    if (pRect)
    {
        if (aRect.Right() < pRect->Left())
            return;
        else if (aRect.Left() > pRect->Right())
            return;
    }
    else
    {
        if (aRect.Right() < 0)
            return;
        else if (aRect.Left() > mnDX)
            return;
    }

    auto& pItem  = mvItemList[nPos];
    HeaderBarItemBits nBits = pItem->mnBits;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (rRenderContext.IsNativeControlSupported(ControlType::WindowBackground, ControlPart::Entire))
    {
        aCtrlRegion = aRect;
        rRenderContext.DrawNativeControl(ControlType::WindowBackground, ControlPart::Entire,
                                         aCtrlRegion, nState, aControlValue, OUString());

    }
    else
    {
        // do not draw border
        aRect.AdjustTop(mnBorderOff1 );
        aRect.AdjustBottom( -mnBorderOff2 );

        // delete background
        if ( !pRect )
        {
            rRenderContext.DrawWallpaper(aRect, rRenderContext.GetBackground());
        }
    }

    Color aSelectionTextColor(COL_TRANSPARENT);

    if (rRenderContext.IsNativeControlSupported(ControlType::ListHeader, ControlPart::Button))
    {
        aCtrlRegion = aRect;
        aControlValue.setTristateVal(ButtonValue::On);
        nState |= ControlState::ENABLED;
        if (bHigh)
            nState |= ControlState::PRESSED;
        rRenderContext.DrawNativeControl(ControlType::ListHeader, ControlPart::Button,
                                         aCtrlRegion, nState, aControlValue, OUString());
    }
    else
    {
        // draw separation line
        rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
        rRenderContext.DrawLine(Point(aRect.Right(), aRect.Top()), Point(aRect.Right(), aRect.Bottom()));

        // draw ButtonStyle
        // avoid 3D borders
        if (bHigh)
            vcl::RenderTools::DrawSelectionBackground(rRenderContext, *this, aRect, 1, true, false, false, &aSelectionTextColor);
        else if (!mbButtonStyle || (nBits & HeaderBarItemBits::FLAT))
            vcl::RenderTools::DrawSelectionBackground(rRenderContext, *this, aRect, 0, true, false, false, &aSelectionTextColor);
    }

    // do not draw if there is no space
    if (aRect.GetWidth() < 1)
        return;

    // calculate size and position and draw content
    pItem->maOutText = pItem->maText;
    Size aImageSize = pItem->maImage.GetSizePixel();
    Size aTxtSize(rRenderContext.GetTextWidth(pItem->maOutText), 0);
    if (!pItem->maOutText.isEmpty())
        aTxtSize.setHeight( rRenderContext.GetTextHeight() );
    long nArrowWidth = 0;
    if (nBits & (HeaderBarItemBits::UPARROW | HeaderBarItemBits::DOWNARROW))
        nArrowWidth = HEAD_ARROWSIZE2 + HEADERBAR_ARROWOFF;

    // do not draw if there is not enough space for the image
    long nTestHeight = aImageSize.Height();
    if (!(nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE)))
        nTestHeight += aTxtSize.Height();
    if ((aImageSize.Width() > aRect.GetWidth()) || (nTestHeight > aRect.GetHeight()))
    {
        aImageSize.setWidth( 0 );
        aImageSize.setHeight( 0 );
    }

    // cut text to correct length
    bool bLeftText = false;
    long nMaxTxtWidth = aRect.GetWidth() - (HEADERBAR_TEXTOFF * 2) - nArrowWidth;
    if (nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE))
        nMaxTxtWidth -= aImageSize.Width();
    long nTxtWidth = aTxtSize.Width();
    if (nTxtWidth > nMaxTxtWidth)
    {
        bLeftText = true;
        OUStringBuffer aBuf(pItem->maOutText);
        aBuf.append("...");
        do
        {
            aBuf.remove(aBuf.getLength() - 3 - 1, 1);
            nTxtWidth = rRenderContext.GetTextWidth(aBuf.toString());
        }
        while ((nTxtWidth > nMaxTxtWidth) && (aBuf.getLength() > 3));
        pItem->maOutText = aBuf.makeStringAndClear();
        if (pItem->maOutText.getLength() == 3)
        {
            nTxtWidth = 0;
            pItem->maOutText.clear();
        }
    }

    // calculate text/imageposition
    long nTxtPos;
    if (!bLeftText && (nBits & HeaderBarItemBits::RIGHT))
    {
        nTxtPos = aRect.Right() - nTxtWidth - HEADERBAR_TEXTOFF;
        if (nBits & HeaderBarItemBits::RIGHTIMAGE)
            nTxtPos -= aImageSize.Width();
    }
    else if (!bLeftText && (nBits & HeaderBarItemBits::CENTER))
    {
        long nTempWidth = nTxtWidth;
        if (nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE))
            nTempWidth += aImageSize.Width();
        nTxtPos = aRect.Left() + (aRect.GetWidth() - nTempWidth) / 2;
        if (nBits & HeaderBarItemBits::LEFTIMAGE)
            nTxtPos += aImageSize.Width();
        if (nArrowWidth)
        {
            if (nTxtPos + nTxtWidth + nArrowWidth >= aRect.Right())
            {
                nTxtPos = aRect.Left() + HEADERBAR_TEXTOFF;
                if (nBits & HeaderBarItemBits::LEFTIMAGE)
                    nTxtPos += aImageSize.Width();
            }
        }
    }
    else
    {
        nTxtPos = aRect.Left() + HEADERBAR_TEXTOFF;
        if (nBits & HeaderBarItemBits::LEFTIMAGE)
            nTxtPos += aImageSize.Width();
        if (nBits & HeaderBarItemBits::RIGHT)
            nTxtPos += nArrowWidth;
    }

    // calculate text/imageposition
    long nTxtPosY = 0;
    if (!pItem->maOutText.isEmpty() || (nArrowWidth && aTxtSize.Height()))
    {
        long nTempHeight = aTxtSize.Height();
        nTempHeight += aImageSize.Height();
        nTxtPosY = aRect.Top()+((aRect.GetHeight()-nTempHeight)/2);
        if (!(nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE)))
            nTxtPosY += aImageSize.Height();
    }

    // display text
    if (!pItem->maOutText.isEmpty())
    {
        if (aSelectionTextColor != COL_TRANSPARENT)
        {
            rRenderContext.Push(PushFlags::TEXTCOLOR);
            rRenderContext.SetTextColor(aSelectionTextColor);
        }
        if (IsEnabled())
            rRenderContext.DrawText(Point(nTxtPos, nTxtPosY), pItem->maOutText);
        else
            rRenderContext.DrawCtrlText(Point(nTxtPos, nTxtPosY), pItem->maOutText, 0, pItem->maOutText.getLength(), DrawTextFlags::Disable);
        if (aSelectionTextColor != COL_TRANSPARENT)
            rRenderContext.Pop();
    }

    // calculate the position and draw image if it is available
    long nImagePosY = 0;
    if (aImageSize.Width() && aImageSize.Height())
    {
        long nImagePos = nTxtPos;
        if (nBits & HeaderBarItemBits::LEFTIMAGE)
        {
            nImagePos -= aImageSize.Width();
            if (nBits & HeaderBarItemBits::RIGHT)
                nImagePos -= nArrowWidth;
        }
        else if (nBits & HeaderBarItemBits::RIGHTIMAGE)
        {
            nImagePos += nTxtWidth;
            if (!(nBits & HeaderBarItemBits::RIGHT))
                nImagePos += nArrowWidth;
        }
        else
        {
            if (nBits & HeaderBarItemBits::RIGHT )
                nImagePos = aRect.Right()-aImageSize.Width();
            else if (nBits & HeaderBarItemBits::CENTER)
                nImagePos = aRect.Left() + (aRect.GetWidth() - aImageSize.Width()) / 2;
            else
                nImagePos = aRect.Left() + HEADERBAR_TEXTOFF;
        }

        long nTempHeight = aImageSize.Height();
        if (!(nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE)))
            nTempHeight += aTxtSize.Height();
        nImagePosY = aRect.Top() + ((aRect.GetHeight() - nTempHeight) / 2);

        if (nImagePos + aImageSize.Width() <= aRect.Right())
        {
            DrawImageFlags nStyle = DrawImageFlags::NONE;
            if (!IsEnabled())
                nStyle |= DrawImageFlags::Disable;
            rRenderContext.DrawImage(Point(nImagePos, nImagePosY), pItem->maImage, nStyle);
        }
    }

    if (!(nBits & (HeaderBarItemBits::UPARROW | HeaderBarItemBits::DOWNARROW)))
        return;

    long nArrowX = nTxtPos;
    if (nBits & HeaderBarItemBits::RIGHT)
        nArrowX -= nArrowWidth;
    else
        nArrowX += nTxtWidth + HEADERBAR_ARROWOFF;
    if (!(nBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE)) && pItem->maText.isEmpty())
    {
        if (nBits & HeaderBarItemBits::RIGHT)
            nArrowX -= aImageSize.Width();
        else
            nArrowX += aImageSize.Width();
    }

    // is there enough space to draw the item?
    bool bDraw = true;
    if (nArrowX < aRect.Left() + HEADERBAR_TEXTOFF)
        bDraw = false;
    else if (nArrowX + HEAD_ARROWSIZE2 > aRect.Right())
        bDraw = false;

    if (!bDraw)
        return;

    if (rRenderContext.IsNativeControlSupported(ControlType::ListHeader, ControlPart::Arrow))
    {
        aCtrlRegion = tools::Rectangle(Point(nArrowX, aRect.Top()), Size(nArrowWidth, aRect.GetHeight()));
        // control value passes 1 if arrow points down, 0 otherwise
        aControlValue.setNumericVal((nBits & HeaderBarItemBits::DOWNARROW) ? 1 : 0);
        nState |= ControlState::ENABLED;
        if (bHigh)
            nState |= ControlState::PRESSED;
        rRenderContext.DrawNativeControl(ControlType::ListHeader, ControlPart::Arrow, aCtrlRegion,
                                         nState, aControlValue, OUString());
    }
    else
    {
        long nArrowY;
        if (aTxtSize.Height())
            nArrowY = nTxtPosY + (aTxtSize.Height() / 2);
        else if (aImageSize.Width() && aImageSize.Height())
            nArrowY = nImagePosY + (aImageSize.Height() / 2);
        else
            nArrowY = aRect.Top() + ((aRect.GetHeight() - HEAD_ARROWSIZE2) / 2);
        nArrowY -= HEAD_ARROWSIZE1 - 1;
        if (nBits & HeaderBarItemBits::DOWNARROW)
        {
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            rRenderContext.DrawLine(Point(nArrowX, nArrowY),
                                    Point(nArrowX + HEAD_ARROWSIZE2, nArrowY));
            rRenderContext.DrawLine(Point(nArrowX, nArrowY),
                                    Point(nArrowX + HEAD_ARROWSIZE1, nArrowY + HEAD_ARROWSIZE2));
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(Point(nArrowX + HEAD_ARROWSIZE1, nArrowY + HEAD_ARROWSIZE2),
                                    Point(nArrowX + HEAD_ARROWSIZE2, nArrowY));
        }
        else
        {
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            rRenderContext.DrawLine(Point(nArrowX, nArrowY + HEAD_ARROWSIZE2),
                                    Point(nArrowX + HEAD_ARROWSIZE1, nArrowY));
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(Point(nArrowX, nArrowY + HEAD_ARROWSIZE2),
                                    Point(nArrowX + HEAD_ARROWSIZE2, nArrowY + HEAD_ARROWSIZE2));
            rRenderContext.DrawLine(Point(nArrowX + HEAD_ARROWSIZE2, nArrowY + HEAD_ARROWSIZE2),
                                    Point(nArrowX + HEAD_ARROWSIZE1, nArrowY));
        }
    }
}

void HeaderBar::ImplDrawItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos,
                             bool bHigh, const tools::Rectangle* pRect )
{
    tools::Rectangle aRect = ImplGetItemRect(nPos);
    ImplDrawItem(rRenderContext, nPos, bHigh, aRect, pRect );
}

void HeaderBar::ImplUpdate(sal_uInt16 nPos, bool bEnd)
{
    if (!(IsVisible() && IsUpdateMode()))
        return;

    tools::Rectangle aRect;
    size_t nItemCount = mvItemList.size();
    if (nPos < nItemCount)
        aRect = ImplGetItemRect(nPos);
    else
    {
        aRect.SetBottom( mnDY - 1 );
        if (nItemCount)
            aRect.SetLeft( ImplGetItemRect(nItemCount - 1).Right() );
    }
    if (bEnd)
        aRect.SetRight( mnDX - 1 );
    aRect.AdjustTop(mnBorderOff1 );
    aRect.AdjustBottom( -mnBorderOff2 );
    Invalidate(aRect);
}

void HeaderBar::ImplStartDrag( const Point& rMousePos, bool bCommand )
{
    sal_uInt16  nPos;
    sal_uInt16  nHitTest = ImplHitTest( rMousePos, mnMouseOff, nPos );
    if ( !nHitTest )
        return;

    mbDrag = false;
    auto& pItem = mvItemList[ nPos ];
    if ( nHitTest & HEAD_HITTEST_DIVIDER )
        mbDrag = true;
    else
    {
        if ( ((pItem->mnBits & HeaderBarItemBits::CLICKABLE) && !(pItem->mnBits & HeaderBarItemBits::FLAT)) ||
             (mbDragable && !(pItem->mnBits & HeaderBarItemBits::FIXEDPOS)) )
        {
            mbItemMode = true;
            mbDrag = true;
            if ( bCommand )
            {
                if ( mbDragable )
                    mbItemDrag = true;
                else
                {
                    mbItemMode = false;
                    mbDrag = false;
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
        mbOutDrag = false;
        mnCurItemId = pItem->mnId;
        mnItemDragPos = nPos;
        StartTracking();
        mnStartPos = rMousePos.X()-mnMouseOff;
        mnDragPos = mnStartPos;
        maStartDragHdl.Call( this );
        if (mbItemMode)
            Invalidate();
        else
        {
            tools::Rectangle aSizeRect( mnDragPos, 0, mnDragPos, mnDragSize+mnDY );
            ShowTracking( aSizeRect, ShowTrackFlags::Split );
        }
    }
    else
        mnMouseOff = 0;
}

void HeaderBar::ImplDrag( const Point& rMousePos )
{
    sal_uInt16  nPos = GetItemPos( mnCurItemId );

    mnDragPos = rMousePos.X()-mnMouseOff;
    if ( mbItemMode )
    {
        bool bNewOutDrag;

        tools::Rectangle aItemRect = ImplGetItemRect( nPos );
        bNewOutDrag = !aItemRect.IsInside( rMousePos );

        //  if needed switch on ItemDrag
        if ( bNewOutDrag && mbDragable && !mbItemDrag &&
             !(mvItemList[ nPos ]->mnBits & HeaderBarItemBits::FIXEDPOS) )
        {
            if ( (rMousePos.Y() >= aItemRect.Top()) && (rMousePos.Y() <= aItemRect.Bottom()) )
            {
                mbItemDrag = true;
                Invalidate();
            }
        }

        sal_uInt16 nOldItemDragPos = mnItemDragPos;
        if ( mbItemDrag )
        {
            bNewOutDrag = (rMousePos.Y() < -HEADERBAR_DRAGOUTOFF) || (rMousePos.Y() > mnDY+HEADERBAR_DRAGOUTOFF);

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

                // do not use non-movable items
                if ( mnItemDragPos < nPos )
                {
                    while ( (mvItemList[ mnItemDragPos ]->mnBits & HeaderBarItemBits::FIXEDPOS) &&
                            (mnItemDragPos < nPos) )
                        mnItemDragPos++;
                }
                else if ( mnItemDragPos > nPos )
                {
                    while ( (mvItemList[ mnItemDragPos ]->mnBits & HeaderBarItemBits::FIXEDPOS) &&
                            (mnItemDragPos > nPos) )
                        mnItemDragPos--;
                }
            }

            if ( (mnItemDragPos != nOldItemDragPos) &&
                 (nOldItemDragPos != nPos) &&
                 (nOldItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
            {
                ImplInvertDrag( nPos, nOldItemDragPos );
                Invalidate();
            }
        }

        if ( bNewOutDrag != mbOutDrag )
            Invalidate();

        if ( mbItemDrag  )
        {
            if ( (mnItemDragPos != nOldItemDragPos) &&
                 (mnItemDragPos != nPos) &&
                 (mnItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
            {
                Invalidate();
                ImplInvertDrag( nPos, mnItemDragPos );
            }
        }

        mbOutDrag = bNewOutDrag;
    }
    else
    {
        tools::Rectangle aItemRect = ImplGetItemRect( nPos );
        if ( mnDragPos < aItemRect.Left() )
            mnDragPos = aItemRect.Left();
        if ( (mnDragPos < 0) || (mnDragPos > mnDX-1) )
            HideTracking();
        else
        {
            tools::Rectangle aSizeRect( mnDragPos, 0, mnDragPos, mnDragSize+mnDY );
            ShowTracking( aSizeRect, ShowTrackFlags::Split );
        }
    }

    maDragHdl.Call( this );
}

void HeaderBar::ImplEndDrag( bool bCancel )
{
    HideTracking();

    if ( bCancel || mbOutDrag )
    {
        if ( mbItemMode && (!mbOutDrag || mbItemDrag) )
        {
            Invalidate();
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
                SetPointer( PointerStyle::Arrow );
                if ( (mnItemDragPos != nPos) &&
                     (mnItemDragPos != HEADERBAR_ITEM_NOTFOUND) )
                {
                    ImplInvertDrag( nPos, mnItemDragPos );
                    MoveItem( mnCurItemId, mnItemDragPos );
                }
                else
                    Invalidate();
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
                auto& pItem = mvItemList[ nPos ];
                pItem->mnSize += nDelta;
                ImplUpdate( nPos, true );
            }
        }
    }

    mbDrag          = false;
    EndDrag();
    mnCurItemId     = 0;
    mnItemDragPos   = HEADERBAR_ITEM_NOTFOUND;
    mbOutDrag       = false;
    mbItemMode      = false;
    mbItemDrag      = false;
}

void HeaderBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
        return;

    if ( rMEvt.GetClicks() == 2 )
    {
        long    nTemp;
        sal_uInt16  nPos;
        sal_uInt16  nHitTest = ImplHitTest( rMEvt.GetPosPixel(), nTemp, nPos );
        if ( nHitTest )
        {
            auto& pItem = mvItemList[ nPos ];
            if ( nHitTest & HEAD_HITTEST_DIVIDER )
                mbItemMode = false;
            else
                mbItemMode = true;
            mnCurItemId = pItem->mnId;
            DoubleClick();
            mbItemMode = false;
            mnCurItemId = 0;
        }
    }
    else
        ImplStartDrag( rMEvt.GetPosPixel(), false );
}

void HeaderBar::MouseMove( const MouseEvent& rMEvt )
{
    long            nTemp1;
    sal_uInt16          nTemp2;
    PointerStyle    eStyle = PointerStyle::Arrow;
    sal_uInt16          nHitTest = ImplHitTest( rMEvt.GetPosPixel(), nTemp1, nTemp2 );

    if ( nHitTest & HEAD_HITTEST_DIVIDER )
        eStyle = PointerStyle::HSizeBar;
    SetPointer( eStyle );
}

void HeaderBar::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndDrag( rTEvt.IsTrackingCanceled() );
    else
        ImplDrag( aMousePos );
}

void HeaderBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (mnBorderOff1 || mnBorderOff2)
    {
        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetDarkShadowColor());
        if (mnBorderOff1)
            rRenderContext.DrawLine(Point(0, 0), Point(mnDX - 1, 0));
        if (mnBorderOff2)
            rRenderContext.DrawLine(Point(0, mnDY - 1), Point(mnDX - 1, mnDY - 1));
        // #i40393# draw left and right border, if WB_BORDER was set in ImplInit()
        if (mnBorderOff1 && mnBorderOff2)
        {
            rRenderContext.DrawLine(Point(0, 0), Point(0, mnDY - 1));
            rRenderContext.DrawLine(Point(mnDX - 1, 0), Point(mnDX - 1, mnDY - 1));
        }
    }

    sal_uInt16 nCurItemPos;
    if (mbDrag)
        nCurItemPos = GetItemPos(mnCurItemId);
    else
        nCurItemPos = HEADERBAR_ITEM_NOTFOUND;
    sal_uInt16 nItemCount = static_cast<sal_uInt16>(mvItemList.size());
    for (sal_uInt16 i = 0; i < nItemCount; i++)
        ImplDrawItem(rRenderContext, i, (i == nCurItemPos), &rRect);
}

void HeaderBar::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                      DrawFlags nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    tools::Rectangle   aRect( aPos, aSize );
    vcl::Font   aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & DrawFlags::Mono )
        pDev->SetTextColor( COL_BLACK );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    // draw background
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

    tools::Rectangle aItemRect( aRect );
    size_t nItemCount = mvItemList.size();
    for ( size_t i = 0; i < nItemCount; i++ )
    {
        aItemRect.SetLeft( aRect.Left()+ImplGetItemPos( i ) );
        aItemRect.SetRight( aItemRect.Left() + mvItemList[ i ]->mnSize - 1 );
        // check for overflow on some systems
        if ( aItemRect.Right() > 16000 )
            aItemRect.SetRight( 16000 );
        vcl::Region aRegion( aRect );
        pDev->SetClipRegion( aRegion );
        ImplDrawItem(*pDev, i, false, aItemRect, &aRect );
        pDev->SetClipRegion();
    }

    pDev->Pop();
}

void HeaderBar::Resize()
{
    Size aSize = GetOutputSizePixel();
    if ( IsVisible() && (mnDY != aSize.Height()) )
        Invalidate();
    mnDX = aSize.Width();
    mnDY = aSize.Height();
}

void HeaderBar::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.IsMouseEvent() && (rCEvt.GetCommand() == CommandEventId::StartDrag) && !mbDrag )
    {
        ImplStartDrag( rCEvt.GetMousePosPixel(), true );
        return;
    }

    Window::Command( rCEvt );
}

void HeaderBar::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON) )
        {
            tools::Rectangle aItemRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.SetLeft( aPt.X() );
            aItemRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.SetRight( aPt.X() );
            aItemRect.SetBottom( aPt.Y() );

            OUString aStr = GetHelpText( nItemId );
            if ( aStr.isEmpty() || !(rHEvt.GetMode() & HelpEventMode::BALLOON) )
            {
                auto& pItem = mvItemList[ GetItemPos( nItemId ) ];
                // Quick-help is only displayed if the text is not fully visible.
                // Otherwise we display Helptext only if the items do not contain text
                if ( pItem->maOutText != pItem->maText )
                    aStr = pItem->maText;
                else if (!pItem->maText.isEmpty())
                    aStr.clear();
            }

            if (!aStr.isEmpty())
            {
                if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                    Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                else
                    Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
        }
    }

    Window::RequestHelp( rHEvt );
}

void HeaderBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::Enable )
        Invalidate();
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void HeaderBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

void HeaderBar::EndDrag()
{
    maEndDragHdl.Call( this );
}

void HeaderBar::Select()
{
    maSelectHdl.Call( this );
}

void HeaderBar::DoubleClick()
{
}

void HeaderBar::InsertItem( sal_uInt16 nItemId, const OUString& rText,
                            long nSize, HeaderBarItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "HeaderBar::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == HEADERBAR_ITEM_NOTFOUND,
                "HeaderBar::InsertItem(): ItemId already exists" );

    // create item and insert in the list
    std::unique_ptr<ImplHeadItem> pItem(new ImplHeadItem);
    pItem->mnId         = nItemId;
    pItem->mnBits       = nBits;
    pItem->mnSize       = nSize;
    pItem->maText       = rText;
    if ( nPos < mvItemList.size() ) {
        auto it = mvItemList.begin();
        it += nPos;
        mvItemList.insert( it, std::move(pItem) );
    } else {
        mvItemList.push_back( std::move(pItem) );
    }

    // update display
    ImplUpdate( nPos, true );
}

void HeaderBar::RemoveItem( sal_uInt16 nItemId )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        if ( nPos < mvItemList.size() ) {
            auto it = mvItemList.begin();
            it += nPos;
            mvItemList.erase( it );
        }
    }
}

void HeaderBar::MoveItem( sal_uInt16 nItemId, sal_uInt16 nNewPos )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos == HEADERBAR_ITEM_NOTFOUND )
        return;

    if ( nPos == nNewPos )
        return;

    auto it = mvItemList.begin();
    it += nPos;
    std::unique_ptr<ImplHeadItem> pItem = std::move(*it);
    mvItemList.erase( it );
    if ( nNewPos < nPos )
        nPos = nNewPos;
    it = mvItemList.begin();
    it += nNewPos;
    mvItemList.insert( it, std::move(pItem) );
    ImplUpdate( nPos, true);
}

void HeaderBar::Clear()
{
    // delete all items
    mvItemList.clear();

    ImplUpdate( 0, true );
}

void HeaderBar::SetOffset( long nNewOffset )
{
    // move area
    tools::Rectangle aRect( 0, mnBorderOff1, mnDX-1, mnDY-mnBorderOff1-mnBorderOff2-1 );
    long nDelta = mnOffset-nNewOffset;
    mnOffset = nNewOffset;
    Scroll( nDelta, 0, aRect );
}

sal_uInt16 HeaderBar::GetItemCount() const
{
    return static_cast<sal_uInt16>(mvItemList.size());
}

sal_uInt16 HeaderBar::GetItemPos( sal_uInt16 nItemId ) const
{
    for ( size_t i = 0, n = mvItemList.size(); i < n; ++i ) {
        auto& pItem = mvItemList[ i ];
        if ( pItem->mnId == nItemId )
            return static_cast<sal_uInt16>(i);
    }
    return HEADERBAR_ITEM_NOTFOUND;
}

sal_uInt16 HeaderBar::GetItemId( sal_uInt16 nPos ) const
{
    ImplHeadItem* pItem = (nPos < mvItemList.size() ) ? mvItemList[ nPos ].get() : nullptr;
    if ( pItem )
        return pItem->mnId;
    else
        return 0;
}

sal_uInt16 HeaderBar::GetItemId( const Point& rPos ) const
{
    for ( size_t i = 0, n = mvItemList.size(); i < n; ++i ) {
        if ( ImplGetItemRect( i ).IsInside( rPos ) ) {
            return GetItemId( i );
        }
    }
    return 0;
}

tools::Rectangle HeaderBar::GetItemRect( sal_uInt16 nItemId ) const
{
    tools::Rectangle aRect;
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        aRect = ImplGetItemRect( nPos );
    return aRect;
}

void HeaderBar::SetItemSize( sal_uInt16 nItemId, long nNewSize )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        auto& pItem = mvItemList[ nPos ];
        if ( pItem->mnSize != nNewSize )
        {
            pItem->mnSize = nNewSize;
            ImplUpdate( nPos, true );
        }
    }
}

long HeaderBar::GetItemSize( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mnSize;
    else
        return 0;
}

void HeaderBar::SetItemBits( sal_uInt16 nItemId, HeaderBarItemBits nNewBits )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        auto& pItem = mvItemList[ nPos ];
        if ( pItem->mnBits != nNewBits )
        {
            pItem->mnBits = nNewBits;
            ImplUpdate( nPos );
        }
    }
}

HeaderBarItemBits HeaderBar::GetItemBits( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->mnBits;
    else
        return HeaderBarItemBits::NONE;
}

void HeaderBar::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        mvItemList[ nPos ]->maText = rText;
        ImplUpdate( nPos );
    }
}

OUString HeaderBar::GetItemText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
        return mvItemList[ nPos ]->maText;
    return OUString();
}

OUString HeaderBar::GetHelpText( sal_uInt16 nItemId ) const
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos != HEADERBAR_ITEM_NOTFOUND )
    {
        auto& pItem = mvItemList[ nPos ];
        if ( pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty() )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }

        return pItem->maHelpText;
    }

    return OUString();
}

Size HeaderBar::CalcWindowSizePixel() const
{
    long nMaxImageSize = 0;
    Size aSize( 0, GetTextHeight() );

    for (auto& pItem : mvItemList)
    {
        // take image size into account
        long nImageHeight = pItem->maImage.GetSizePixel().Height();
        if ( !(pItem->mnBits & (HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::RIGHTIMAGE)) && !pItem->maText.isEmpty() )
            nImageHeight += aSize.Height();
        if ( nImageHeight > nMaxImageSize )
            nMaxImageSize = nImageHeight;

        // add width
        aSize.AdjustWidth(pItem->mnSize );
    }

    if ( nMaxImageSize > aSize.Height() )
        aSize.setHeight( nMaxImageSize );

    // add border
    if ( mbButtonStyle )
        aSize.AdjustHeight(4 );
    else
        aSize.AdjustHeight(2 );
    aSize.AdjustHeight(mnBorderOff1+mnBorderOff2 );

    return aSize;
}

css::uno::Reference< css::accessibility::XAccessible > HeaderBar::CreateAccessible()
{
    if ( !mxAccessible.is() )
    {
        maCreateAccessibleHdl.Call( this );

        if ( !mxAccessible.is() )
            mxAccessible = Window::CreateAccessible();
    }

    return mxAccessible;
}

void HeaderBar::SetAccessible( const css::uno::Reference< css::accessibility::XAccessible >& _xAccessible )
{
    mxAccessible = _xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
