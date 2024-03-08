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

#include <strings.hrc>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <salframe.hxx>
#include <window.h>

#include <vcl/textrectinfo.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/syswin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/help.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/ptrstyle.hxx>

using namespace ::com::sun::star::uno;

// useful caption height for title bar buttons
#define MIN_CAPTION_HEIGHT 18

namespace vcl {

void Window::ImplCalcSymbolRect( tools::Rectangle& rRect )
{
    // Add border, not shown in the non-default representation,
    // as we want to use it for small buttons
    rRect.AdjustLeft( -1 );
    rRect.AdjustTop( -1 );
    rRect.AdjustRight( 1 );
    rRect.AdjustBottom( 1 );

    // we leave 5% room between the symbol and the button border
    tools::Long nExtraWidth = ((rRect.GetWidth()*50)+500)/1000;
    tools::Long nExtraHeight = ((rRect.GetHeight()*50)+500)/1000;
    rRect.AdjustLeft(nExtraWidth );
    rRect.AdjustRight( -nExtraWidth );
    rRect.AdjustTop(nExtraHeight );
    rRect.AdjustBottom( -nExtraHeight );
}

} /* namespace vcl */

static void ImplDrawBrdWinSymbol( vcl::RenderContext* pDev,
                                  const tools::Rectangle& rRect, SymbolType eSymbol )
{
    // we leave 5% room between the symbol and the button border
    DecorationView  aDecoView( pDev );
    tools::Rectangle       aTempRect = rRect;
    vcl::Window::ImplCalcSymbolRect( aTempRect );
    aDecoView.DrawSymbol( aTempRect, eSymbol,
                          pDev->GetSettings().GetStyleSettings().GetButtonTextColor() );
}

static void ImplDrawBrdWinSymbolButton( vcl::RenderContext* pDev,
                                        const tools::Rectangle& rRect,
                                        SymbolType eSymbol, DrawButtonFlags nState )
{
    bool bMouseOver(nState & DrawButtonFlags::Highlight);
    nState &= ~DrawButtonFlags::Highlight;

    tools::Rectangle aTempRect;
    vcl::Window *pWin = pDev->GetOwnerWindow();
    if( pWin )
    {
        if( bMouseOver )
        {
            // provide a bright background for selection effect
            pDev->SetFillColor( pDev->GetSettings().GetStyleSettings().GetWindowColor() );
            pDev->SetLineColor();
            pDev->DrawRect( rRect );
            pWin->DrawSelectionBackground( rRect, 2, bool(nState & DrawButtonFlags::Pressed),
                                            true );
        }
        aTempRect = rRect;
        aTempRect.AdjustLeft(3 );
        aTempRect.AdjustRight( -4 );
        aTempRect.AdjustTop(3 );
        aTempRect.AdjustBottom( -4 );
    }
    else
    {
        DecorationView aDecoView( pDev );
        aTempRect = aDecoView.DrawButton( rRect, nState|DrawButtonFlags::Flat );
    }
    ImplDrawBrdWinSymbol( pDev, aTempRect, eSymbol );
}


ImplBorderWindowView::~ImplBorderWindowView()
{
}

bool ImplBorderWindowView::MouseMove( const MouseEvent& )
{
    return false;
}

bool ImplBorderWindowView::MouseButtonDown( const MouseEvent& )
{
    return false;
}

bool ImplBorderWindowView::Tracking( const TrackingEvent& )
{
    return false;
}

OUString ImplBorderWindowView::RequestHelp( const Point&, tools::Rectangle& )
{
    return OUString();
}

tools::Rectangle ImplBorderWindowView::GetMenuRect() const
{
    return tools::Rectangle();
}

void ImplBorderWindowView::ImplInitTitle(ImplBorderFrameData* pData)
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) ||
          (pData->mnTitleType == BorderWindowTitleType::NONE) )
    {
        pData->mnTitleType   = BorderWindowTitleType::NONE;
        pData->mnTitleHeight = 0;
    }
    else
    {
        const StyleSettings& rStyleSettings = pData->mpOutDev->GetSettings().GetStyleSettings();
        if (pData->mnTitleType == BorderWindowTitleType::Tearoff)
            pData->mnTitleHeight = ToolBox::ImplGetDragWidth(*pData->mpBorderWindow, false) + 2;
        else
        {
            if (pData->mnTitleType == BorderWindowTitleType::Small)
            {
                pBorderWindow->SetPointFont(*pBorderWindow->GetOutDev(), rStyleSettings.GetFloatTitleFont() );
                pData->mnTitleHeight = rStyleSettings.GetFloatTitleHeight();
            }
            else // pData->mnTitleType == BorderWindowTitleType::Normal
            {
                // FIXME RenderContext
                pBorderWindow->SetPointFont(*pBorderWindow->GetOutDev(), rStyleSettings.GetTitleFont());
                pData->mnTitleHeight = rStyleSettings.GetTitleHeight();
            }
            tools::Long nTextHeight = pBorderWindow->GetTextHeight();
            if (nTextHeight > pData->mnTitleHeight)
                pData->mnTitleHeight = nTextHeight;
        }
    }
}

BorderWindowHitTest ImplBorderWindowView::ImplHitTest( ImplBorderFrameData const * pData, const Point& rPos )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( pData->maTitleRect.Contains( rPos ) )
    {
        if ( pData->maCloseRect.Contains( rPos ) )
            return BorderWindowHitTest::Close;
        else if ( pData->maMenuRect.Contains( rPos ) )
            return BorderWindowHitTest::Menu;
        else if ( pData->maDockRect.Contains( rPos ) )
            return BorderWindowHitTest::Dock;
        else if ( pData->maHideRect.Contains( rPos ) )
            return BorderWindowHitTest::Hide;
        else if ( pData->maHelpRect.Contains( rPos ) )
            return BorderWindowHitTest::Help;
        else
            return BorderWindowHitTest::Title;
    }

    if (pBorderWindow->GetStyle() & WB_SIZEABLE)
    {
        tools::Long nSizeWidth = pData->mnNoTitleTop+pData->mnTitleHeight;
        if ( nSizeWidth < 16 )
            nSizeWidth = 16;

        // no corner resize for floating toolbars, which would lead to jumps while formatting
        // setting nSizeWidth = 0 will only return pure left,top,right,bottom
        if( pBorderWindow->GetStyle() & (WB_OWNERDRAWDECORATION | WB_POPUP) )
            nSizeWidth = 0;

        if ( rPos.X() < pData->mnLeftBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BorderWindowHitTest::TopLeft;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BorderWindowHitTest::BottomLeft;
            else
                return BorderWindowHitTest::Left;
        }
        else if ( rPos.X() >= pData->mnWidth-pData->mnRightBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BorderWindowHitTest::TopRight;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BorderWindowHitTest::BottomRight;
            else
                return BorderWindowHitTest::Right;
        }
        else if ( rPos.Y() < pData->mnNoTitleTop )
        {
            if ( rPos.X() < nSizeWidth )
                return BorderWindowHitTest::TopLeft;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BorderWindowHitTest::TopRight;
            else
                return BorderWindowHitTest::Top;
        }
        else if ( rPos.Y() >= pData->mnHeight-pData->mnBottomBorder )
        {
            if ( rPos.X() < nSizeWidth )
                return BorderWindowHitTest::BottomLeft;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BorderWindowHitTest::BottomRight;
            else
                return BorderWindowHitTest::Bottom;
        }
    }

    return BorderWindowHitTest::NONE;
}

void ImplBorderWindowView::ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    DrawButtonFlags oldCloseState = pData->mnCloseState;
    DrawButtonFlags oldMenuState = pData->mnMenuState;
    pData->mnCloseState &= ~DrawButtonFlags::Highlight;
    pData->mnMenuState &= ~DrawButtonFlags::Highlight;

    Point aMousePos = rMEvt.GetPosPixel();
    BorderWindowHitTest nHitTest = ImplHitTest( pData, aMousePos );
    PointerStyle ePtrStyle = PointerStyle::Arrow;
    if ( nHitTest & BorderWindowHitTest::Left )
        ePtrStyle = PointerStyle::WindowWSize;
    else if ( nHitTest & BorderWindowHitTest::Right )
        ePtrStyle = PointerStyle::WindowESize;
    else if ( nHitTest & BorderWindowHitTest::Top )
        ePtrStyle = PointerStyle::WindowNSize;
    else if ( nHitTest & BorderWindowHitTest::Bottom )
        ePtrStyle = PointerStyle::WindowSSize;
    else if ( nHitTest & BorderWindowHitTest::TopLeft )
        ePtrStyle = PointerStyle::WindowNWSize;
    else if ( nHitTest & BorderWindowHitTest::BottomRight )
        ePtrStyle = PointerStyle::WindowSESize;
    else if ( nHitTest & BorderWindowHitTest::TopRight )
        ePtrStyle = PointerStyle::WindowNESize;
    else if ( nHitTest & BorderWindowHitTest::BottomLeft )
        ePtrStyle = PointerStyle::WindowSWSize;
    else if ( nHitTest & BorderWindowHitTest::Close )
        pData->mnCloseState |= DrawButtonFlags::Highlight;
    else if ( nHitTest & BorderWindowHitTest::Menu )
        pData->mnMenuState |= DrawButtonFlags::Highlight;
    else if ( nHitTest & BorderWindowHitTest::Title &&
              pData->mnTitleType == BorderWindowTitleType::Tearoff && !rMEvt.IsLeaveWindow() )
        ePtrStyle = PointerStyle::Move;
    pData->mpBorderWindow->SetPointer( ePtrStyle );

    if( pData->mnCloseState != oldCloseState )
        pData->mpBorderWindow->Invalidate( pData->maCloseRect );
    if( pData->mnMenuState != oldMenuState )
        pData->mpBorderWindow->Invalidate( pData->maMenuRect );
}

OUString ImplBorderWindowView::ImplRequestHelp( ImplBorderFrameData const * pData,
                                              const Point& rPos,
                                              tools::Rectangle& rHelpRect )
{
    TranslateId pHelpId;
    OUString aHelpStr;
    BorderWindowHitTest nHitTest = ImplHitTest( pData, rPos );
    if ( nHitTest != BorderWindowHitTest::NONE )
    {
        if ( nHitTest & BorderWindowHitTest::Close )
        {
            pHelpId     = SV_HELPTEXT_CLOSE;
            rHelpRect   = pData->maCloseRect;
        }
        else if ( nHitTest & BorderWindowHitTest::Dock )
        {
            pHelpId     = SV_HELPTEXT_MAXIMIZE;
            rHelpRect   = pData->maDockRect;
        }
        else if ( nHitTest & BorderWindowHitTest::Hide )
        {
            pHelpId     = SV_HELPTEXT_MINIMIZE;
            rHelpRect   = pData->maHideRect;
        }
        else if ( nHitTest & BorderWindowHitTest::Help )
        {
            pHelpId     = SV_HELPTEXT_HELP;
            rHelpRect   = pData->maHelpRect;
        }
        else if ( nHitTest & BorderWindowHitTest::Title )
        {
            if( !pData->maTitleRect.IsEmpty() )
            {
                // tooltip only if title truncated
                if( pData->mbTitleClipped )
                {
                    rHelpRect   = pData->maTitleRect;
                    // no help id, use window title as help string
                    aHelpStr    = pData->mpBorderWindow->GetText();
                }
            }
        }
    }

    if (pHelpId)
        aHelpStr = VclResId(pHelpId);

    return aHelpStr;
}

tools::Long ImplBorderWindowView::ImplCalcTitleWidth( const ImplBorderFrameData* pData )
{
    // title is not visible therefore no width
    if ( !pData->mnTitleHeight )
        return 0;

    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    tools::Long nTitleWidth = pBorderWindow->GetTextWidth( pBorderWindow->GetText() )+6;
    nTitleWidth += pData->maCloseRect.GetWidth();
    nTitleWidth += pData->maDockRect.GetWidth();
    nTitleWidth += pData->maMenuRect.GetWidth();
    nTitleWidth += pData->maHideRect.GetWidth();
    nTitleWidth += pData->maHelpRect.GetWidth();
    nTitleWidth += pData->mnLeftBorder+pData->mnRightBorder;
    return nTitleWidth;
}


ImplNoBorderWindowView::ImplNoBorderWindowView()
{
}

void ImplNoBorderWindowView::Init( OutputDevice*, tools::Long, tools::Long )
{
}

void ImplNoBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                        sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = 0;
    rTopBorder      = 0;
    rRightBorder    = 0;
    rBottomBorder   = 0;
}

tools::Long ImplNoBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplNoBorderWindowView::DrawWindow(vcl::RenderContext&, const Point*)
{
}

ImplSmallBorderWindowView::ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow )
    : mpBorderWindow(pBorderWindow)
    , mpOutDev(nullptr)
    , mnWidth(0)
    , mnHeight(0)
    , mnLeftBorder(0)
    , mnTopBorder(0)
    , mnRightBorder(0)
    , mnBottomBorder(0)
    , mbNWFBorder(false)
{
}

void ImplSmallBorderWindowView::Init( OutputDevice* pDev, tools::Long nWidth, tools::Long nHeight )
{
    mpOutDev    = pDev;
    mnWidth     = nWidth;
    mnHeight    = nHeight;
    mbNWFBorder = false;

    vcl::Window *pWin = mpOutDev->GetOwnerWindow();
    vcl::Window *pCtrl = nullptr;
    vcl::Window *pSubEdit = nullptr;
    if (pWin)
        pCtrl = mpBorderWindow->GetWindow(GetWindowType::Client);

    tools::Long nOrigLeftBorder = mnLeftBorder;
    tools::Long nOrigTopBorder = mnTopBorder;
    tools::Long nOrigRightBorder = mnRightBorder;
    tools::Long nOrigBottomBorder = mnBottomBorder;

    WindowBorderStyle nBorderStyle = mpBorderWindow->GetBorderStyle();
    if ( nBorderStyle & WindowBorderStyle::NOBORDER )
    {
        mnLeftBorder    = 0;
        mnTopBorder     = 0;
        mnRightBorder   = 0;
        mnBottomBorder  = 0;
    }
    else
    {
        // FIXME: this is currently only on macOS, check with other
        // platforms
        if( ImplGetSVData()->maNWFData.mbNoFocusRects && !( nBorderStyle & WindowBorderStyle::NWF ) )
        {
            // for native widget drawing we must find out what
            // control this border belongs to
            ControlType aCtrlType = ControlType::Generic;
            ControlPart aCtrlPart = ControlPart::Entire;
            if (pCtrl)
            {
                switch( pCtrl->GetType() )
                {
                    case WindowType::LISTBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = ControlType::Listbox;
                            mbNWFBorder = true;
                            pSubEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
                        }
                        break;
                    case WindowType::LISTBOXWINDOW:
                        aCtrlType = ControlType::Listbox;
                        aCtrlPart = ControlPart::ListboxWindow;
                        mbNWFBorder = true;
                        break;
                    case WindowType::COMBOBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = ControlType::Combobox;
                            mbNWFBorder = true;
                            pSubEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
                        }
                        break;
                    case WindowType::MULTILINEEDIT:
                        aCtrlType = ControlType::MultilineEditbox;
                        mbNWFBorder = true;
                        break;
                    case WindowType::EDIT:
                    case WindowType::PATTERNFIELD:
                    case WindowType::METRICFIELD:
                    case WindowType::CURRENCYFIELD:
                    case WindowType::DATEFIELD:
                    case WindowType::TIMEFIELD:
                    case WindowType::SPINFIELD:
                    case WindowType::FORMATTEDFIELD:
                        mbNWFBorder = true;
                        if (pCtrl->GetStyle() & WB_SPIN)
                            aCtrlType = ControlType::Spinbox;
                        else
                            aCtrlType = ControlType::Editbox;
                        pSubEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
                        break;
                    default:
                        break;
                }
            }
            if( mbNWFBorder )
            {
                ImplControlValue aControlValue;
                Size aMinSize( mnWidth, mnHeight );
                if( aMinSize.Width() < 10 ) aMinSize.setWidth( 10 );
                if( aMinSize.Height() < 10 ) aMinSize.setHeight( 10 );
                tools::Rectangle aCtrlRegion( Point(), aMinSize );
                tools::Rectangle aBounds, aContent;
                if( pWin->GetNativeControlRegion( aCtrlType, aCtrlPart, aCtrlRegion,
                                                  ControlState::ENABLED, aControlValue,
                                                  aBounds, aContent ) )
                {
                    aBounds.AdjustLeft(mnLeftBorder);
                    aBounds.AdjustRight(-mnRightBorder);
                    aBounds.AdjustTop(mnTopBorder);
                    aBounds.AdjustBottom(-mnBottomBorder);
                    aContent.AdjustLeft(mnLeftBorder);
                    aContent.AdjustRight(-mnRightBorder);
                    aContent.AdjustTop(mnTopBorder);
                    aContent.AdjustBottom(-mnBottomBorder);
                    mnLeftBorder    = aContent.Left() - aBounds.Left();
                    mnRightBorder   = aBounds.Right() - aContent.Right();
                    mnTopBorder     = aContent.Top() - aBounds.Top();
                    mnBottomBorder  = aBounds.Bottom() - aContent.Bottom();
                    if( mnWidth && mnHeight )
                    {

                        mpBorderWindow->SetPaintTransparent( true );
                        mpBorderWindow->SetBackground();
                        if (!pCtrl->IsControlBackground())
                        {
                            pCtrl->SetPaintTransparent(true);
                            if (pSubEdit)
                                pSubEdit->SetPaintTransparent(true);
                        }

                        vcl::Window* pCompoundParent = nullptr;
                        if( pWin->GetParent() && pWin->GetParent()->IsCompoundControl() )
                            pCompoundParent = pWin->GetParent();

                        if( pCompoundParent )
                            pCompoundParent->SetPaintTransparent( true );

                        if( mnWidth < aBounds.GetWidth() || mnHeight < aBounds.GetHeight() )
                        {
                            if( ! pCompoundParent ) // compound controls have to fix themselves
                            {
                                Point aPos( mpBorderWindow->GetPosPixel() );
                                if( mnWidth < aBounds.GetWidth() )
                                    aPos.AdjustX( -((aBounds.GetWidth() - mnWidth) / 2) );
                                if( mnHeight < aBounds.GetHeight() )
                                    aPos.AdjustY( -((aBounds.GetHeight() - mnHeight) / 2) );
                                mpBorderWindow->SetPosSizePixel( aPos, aBounds.GetSize() );
                            }
                        }
                    }
                }
                else
                    mbNWFBorder = false;
            }
        }

        if( ! mbNWFBorder )
        {
            DrawFrameStyle nStyle = DrawFrameStyle::NONE;
            DrawFrameFlags nFlags = DrawFrameFlags::NoDraw;
            // move border outside if border was converted or if the BorderWindow is a frame window,
            if ( mpBorderWindow->mbSmallOutBorder )
                nStyle = DrawFrameStyle::DoubleOut;
            else if ( nBorderStyle & WindowBorderStyle::NWF )
                nStyle = DrawFrameStyle::NWF;
            else
                nStyle = DrawFrameStyle::DoubleIn;
            if ( nBorderStyle & WindowBorderStyle::MONO )
                nFlags |= DrawFrameFlags::Mono;

            DecorationView  aDecoView( mpOutDev );
            tools::Rectangle       aRect( 0, 0, 10, 10 );
            tools::Rectangle       aCalcRect = aDecoView.DrawFrame( aRect, nStyle, nFlags );
            mnLeftBorder    = aCalcRect.Left();
            mnTopBorder     = aCalcRect.Top();
            mnRightBorder   = aRect.Right()-aCalcRect.Right();
            mnBottomBorder  = aRect.Bottom()-aCalcRect.Bottom();
        }
    }

    if (pCtrl)
    {
        //fdo#57090 If the borders have changed, then trigger a queue_resize on
        //the bordered window, which will resync its borders at that point
        if (nOrigLeftBorder != mnLeftBorder ||
            nOrigTopBorder != mnTopBorder ||
            nOrigRightBorder != mnRightBorder ||
            nOrigBottomBorder != mnBottomBorder)
        {
            pCtrl->queue_resize();
        }
    }
}

void ImplSmallBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                           sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mnLeftBorder;
    rTopBorder      = mnTopBorder;
    rRightBorder    = mnRightBorder;
    rBottomBorder   = mnBottomBorder;
}

tools::Long ImplSmallBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplSmallBorderWindowView::DrawWindow(vcl::RenderContext& rRenderContext, const Point*)
{
    WindowBorderStyle nBorderStyle = mpBorderWindow->GetBorderStyle();
    if (nBorderStyle & WindowBorderStyle::NOBORDER)
        return;

    bool bNativeOK = false;
    // for native widget drawing we must find out what
    // control this border belongs to
    vcl::Window* pCtrl = mpBorderWindow->GetWindow(GetWindowType::Client);

    ControlType aCtrlType = ControlType::Generic;
    ControlPart aCtrlPart = ControlPart::Entire;
    if (pCtrl)
    {
        switch (pCtrl->GetType())
        {
            case WindowType::MULTILINEEDIT:
                aCtrlType = ControlType::MultilineEditbox;
                break;
            case WindowType::EDIT:
            case WindowType::PATTERNFIELD:
            case WindowType::METRICFIELD:
            case WindowType::CURRENCYFIELD:
            case WindowType::DATEFIELD:
            case WindowType::TIMEFIELD:
            case WindowType::SPINFIELD:
            case WindowType::FORMATTEDFIELD:
                if (pCtrl->GetStyle() & WB_SPIN)
                    aCtrlType = ControlType::Spinbox;
                else
                    aCtrlType = ControlType::Editbox;
                break;

            case WindowType::LISTBOX:
            case WindowType::MULTILISTBOX:
            case WindowType::TREELISTBOX:
                aCtrlType = ControlType::Listbox;
                if (pCtrl->GetStyle() & WB_DROPDOWN)
                    aCtrlPart = ControlPart::Entire;
                else
                    aCtrlPart = ControlPart::ListboxWindow;
                break;

            case WindowType::LISTBOXWINDOW:
                aCtrlType = ControlType::Listbox;
                aCtrlPart = ControlPart::ListboxWindow;
                break;

            case WindowType::COMBOBOX:
            case WindowType::PATTERNBOX:
            case WindowType::NUMERICBOX:
            case WindowType::METRICBOX:
            case WindowType::CURRENCYBOX:
            case WindowType::DATEBOX:
            case WindowType::TIMEBOX:
            case WindowType::LONGCURRENCYBOX:
                if (pCtrl->GetStyle() & WB_DROPDOWN)
                {
                    aCtrlType = ControlType::Combobox;
                    aCtrlPart = ControlPart::Entire;
                }
                else
                {
                    aCtrlType = ControlType::Listbox;
                    aCtrlPart = ControlPart::ListboxWindow;
                }
                break;

            default:
                break;
        }
    }

    if (aCtrlType != ControlType::Generic && pCtrl->IsNativeControlSupported(aCtrlType, aCtrlPart))
    {
        ImplControlValue aControlValue;
        ControlState nState = ControlState::ENABLED;

        if (!mpBorderWindow->IsEnabled())
            nState &= ~ControlState::ENABLED;
        if (mpBorderWindow->HasFocus() || pCtrl->HasFocus() || pCtrl->HasChildPathFocus())
            nState |= ControlState::FOCUSED;

        bool bMouseOver = pCtrl->IsMouseOver();
        if (!bMouseOver)
        {
            vcl::Window *pCtrlChild = pCtrl->GetWindow(GetWindowType::FirstChild);
            while(pCtrlChild)
            {
                bMouseOver = pCtrlChild->IsMouseOver();
                if (bMouseOver)
                    break;
                pCtrlChild = pCtrlChild->GetWindow(GetWindowType::Next);
            }
        }

        if (bMouseOver)
            nState |= ControlState::ROLLOVER;

        Point aPoint;
        tools::Rectangle aCtrlRegion(aPoint, Size(mnWidth, mnHeight));

        tools::Rectangle aBoundingRgn(aPoint, Size(mnWidth, mnHeight));
        tools::Rectangle aContentRgn(aCtrlRegion);
        if (!ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
            rRenderContext.GetNativeControlRegion(aCtrlType, aCtrlPart, aCtrlRegion,
                                         nState, aControlValue,
                                         aBoundingRgn, aContentRgn))
        {
            aCtrlRegion=aContentRgn;
        }

        Color aBackgroundColor = COL_AUTO;
        if (pCtrl->IsControlBackground())
            aBackgroundColor = pCtrl->GetBackgroundColor();
        bNativeOK = rRenderContext.DrawNativeControl(aCtrlType, aCtrlPart, aCtrlRegion, nState, aControlValue, OUString(), aBackgroundColor);

        // if the native theme draws the spinbuttons in one call, make sure the proper settings
        // are passed, this might force a redraw though... (TODO: improve)
        if ((aCtrlType == ControlType::Spinbox) && !pCtrl->IsNativeControlSupported(ControlType::Spinbox, ControlPart::ButtonUp))
        {
            Edit* pEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
            if (pEdit && !pEdit->SupportsDoubleBuffering())
                pCtrl->Paint(*pCtrl->GetOutDev(), tools::Rectangle());  // make sure the buttons are also drawn as they might overwrite the border
        }
    }

    if (bNativeOK)
        return;

    DrawFrameStyle nStyle = DrawFrameStyle::NONE;
    DrawFrameFlags nFlags = DrawFrameFlags::NONE;
    // move border outside if border was converted or if the border window is a frame window,
    if (mpBorderWindow->mbSmallOutBorder)
        nStyle = DrawFrameStyle::DoubleOut;
    else if (nBorderStyle & WindowBorderStyle::NWF)
        nStyle = DrawFrameStyle::NWF;
    else
        nStyle = DrawFrameStyle::DoubleIn;
    if (nBorderStyle & WindowBorderStyle::MONO)
        nFlags |= DrawFrameFlags::Mono;
    if (nBorderStyle & WindowBorderStyle::MENU)
        nFlags |= DrawFrameFlags::Menu;
    // tell DrawFrame that we're drawing a window border of a frame window to avoid round corners
    if (mpBorderWindow == mpBorderWindow->ImplGetFrameWindow())
        nFlags |= DrawFrameFlags::WindowBorder;

    DecorationView aDecoView(&rRenderContext);
    tools::Rectangle aInRect(Point(), Size(mnWidth, mnHeight));
    aDecoView.DrawFrame(aInRect, nStyle, nFlags);
}


ImplStdBorderWindowView::ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = false;
    maFrameData.mnHitTest       = BorderWindowHitTest::NONE;
    maFrameData.mnCloseState    = DrawButtonFlags::NONE;
    maFrameData.mnDockState     = DrawButtonFlags::NONE;
    maFrameData.mnMenuState     = DrawButtonFlags::NONE;
    maFrameData.mnHideState     = DrawButtonFlags::NONE;
    maFrameData.mnHelpState     = DrawButtonFlags::NONE;
    maFrameData.mbTitleClipped  = false;
}

ImplStdBorderWindowView::~ImplStdBorderWindowView()
{
}

bool ImplStdBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    ImplMouseMove( &maFrameData, rMEvt );
    return true;
}

bool ImplStdBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    ImplBorderWindow* pBorderWindow = maFrameData.mpBorderWindow;

    if ( rMEvt.IsLeft() || rMEvt.IsRight() )
    {
        maFrameData.maMouseOff = rMEvt.GetPosPixel();
        maFrameData.mnHitTest = ImplHitTest( &maFrameData, maFrameData.maMouseOff );
        if ( maFrameData.mnHitTest != BorderWindowHitTest::NONE )
        {
            DragFullOptions nDragFullTest = DragFullOptions::NONE;
            bool bTracking = true;
            bool bHitTest = true;

            if ( maFrameData.mnHitTest & BorderWindowHitTest::Close )
            {
                maFrameData.mnCloseState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BorderWindowHitTest::Dock )
            {
                maFrameData.mnDockState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BorderWindowHitTest::Menu )
            {
                maFrameData.mnMenuState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // call handler already on mouse down
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                    pClientWindow->TitleButtonClick( TitleButton::Menu );
                }

                maFrameData.mnMenuState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                bTracking = false;
            }
            else if ( maFrameData.mnHitTest & BorderWindowHitTest::Hide )
            {
                maFrameData.mnHideState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BorderWindowHitTest::Help )
            {
                maFrameData.mnHelpState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    Point aPos  = pBorderWindow->GetPosPixel();
                    Size  aSize = pBorderWindow->GetOutputSizePixel();
                    maFrameData.mnTrackX      = aPos.X();
                    maFrameData.mnTrackY      = aPos.Y();
                    maFrameData.mnTrackWidth  = aSize.Width();
                    maFrameData.mnTrackHeight = aSize.Height();

                    if (maFrameData.mnHitTest & BorderWindowHitTest::Title)
                        nDragFullTest = DragFullOptions::WindowMove;
                    else
                        nDragFullTest = DragFullOptions::WindowSize;
                }
                else
                {
                    bTracking = false;

                    if ( (maFrameData.mnHitTest & BorderWindowHitTest::Title) &&
                         ((rMEvt.GetClicks() % 2) == 0) )
                    {
                        maFrameData.mnHitTest = BorderWindowHitTest::NONE;
                        bHitTest = false;

                        if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                        {
                            SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                            // always perform docking on double click, no button required
                            pClientWindow->TitleButtonClick( TitleButton::Docking );
                        }
                    }
                }
            }

            if ( bTracking )
            {
                maFrameData.mbDragFull = false;
                if ( nDragFullTest != DragFullOptions::NONE )
                    maFrameData.mbDragFull = true;   // always fulldrag for proper docking, ignore system settings
                pBorderWindow->StartTracking();
            }
            else if ( bHitTest )
                maFrameData.mnHitTest = BorderWindowHitTest::NONE;
        }
    }

    return true;
}

bool ImplStdBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    ImplBorderWindow* pBorderWindow = maFrameData.mpBorderWindow;

    if ( rTEvt.IsTrackingEnded() )
    {
        BorderWindowHitTest nHitTest = maFrameData.mnHitTest;
        maFrameData.mnHitTest = BorderWindowHitTest::NONE;

        if ( nHitTest & BorderWindowHitTest::Close )
        {
            if ( maFrameData.mnCloseState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnCloseState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    // dispatch to correct window type (why is Close() not virtual ??? )
                    // TODO: make Close() virtual
                    VclPtr<vcl::Window> pWin = pBorderWindow->ImplGetClientWindow()->ImplGetWindow();
                    SystemWindow  *pSysWin  = dynamic_cast<SystemWindow* >(pWin.get());
                    DockingWindow *pDockWin = dynamic_cast<DockingWindow*>(pWin.get());
                    if ( pSysWin )
                        pSysWin->Close();
                    else if ( pDockWin )
                        pDockWin->Close();
                }
            }
        }
        else if ( nHitTest & BorderWindowHitTest::Dock )
        {
            if ( maFrameData.mnDockState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnDockState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->TitleButtonClick( TitleButton::Docking );
                    }
                }
            }
        }
        else if ( nHitTest & BorderWindowHitTest::Menu )
        {
            if ( maFrameData.mnMenuState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnMenuState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // handler already called on mouse down
            }
        }
        else if ( nHitTest & BorderWindowHitTest::Hide )
        {
            if ( maFrameData.mnHideState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnHideState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->TitleButtonClick( TitleButton::Hide );
                    }
                }
            }
        }
        else if ( nHitTest & BorderWindowHitTest::Help )
        {
            if ( maFrameData.mnHelpState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnHelpState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
        }
        else
        {
            if ( maFrameData.mbDragFull )
            {
                // restore old state when aborting
                if ( rTEvt.IsTrackingCanceled() )
                    pBorderWindow->SetPosSizePixel( Point( maFrameData.mnTrackX, maFrameData.mnTrackY ), Size( maFrameData.mnTrackWidth, maFrameData.mnTrackHeight ) );
            }
            else
            {
                pBorderWindow->HideTracking();
                if ( !rTEvt.IsTrackingCanceled() )
                    pBorderWindow->SetPosSizePixel( Point( maFrameData.mnTrackX, maFrameData.mnTrackY ), Size( maFrameData.mnTrackWidth, maFrameData.mnTrackHeight ) );
            }

            if ( !rTEvt.IsTrackingCanceled() )
            {
                if ( pBorderWindow->ImplGetClientWindow()->ImplIsFloatingWindow() )
                {
                    if ( static_cast<FloatingWindow*>(pBorderWindow->ImplGetClientWindow())->IsInPopupMode() )
                        static_cast<FloatingWindow*>(pBorderWindow->ImplGetClientWindow())->EndPopupMode( FloatWinPopupEndFlags::TearOff );
                }
            }
        }
    }
    else if ( !rTEvt.GetMouseEvent().IsSynthetic() )
    {
        Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        if ( maFrameData.mnHitTest & BorderWindowHitTest::Close )
        {
            if ( maFrameData.maCloseRect.Contains( aMousePos ) )
            {
                if ( !(maFrameData.mnCloseState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnCloseState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnCloseState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnCloseState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else if ( maFrameData.mnHitTest & BorderWindowHitTest::Dock )
        {
            if ( maFrameData.maDockRect.Contains( aMousePos ) )
            {
                if ( !(maFrameData.mnDockState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnDockState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnDockState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnDockState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else if ( maFrameData.mnHitTest & BorderWindowHitTest::Menu )
        {
            if ( maFrameData.maMenuRect.Contains( aMousePos ) )
            {
                if ( !(maFrameData.mnMenuState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnMenuState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnMenuState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnMenuState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else if ( maFrameData.mnHitTest & BorderWindowHitTest::Hide )
        {
            if ( maFrameData.maHideRect.Contains( aMousePos ) )
            {
                if ( !(maFrameData.mnHideState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnHideState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnHideState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnHideState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else if ( maFrameData.mnHitTest & BorderWindowHitTest::Help )
        {
            if ( maFrameData.maHelpRect.Contains( aMousePos ) )
            {
                if ( !(maFrameData.mnHelpState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnHelpState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnHelpState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnHelpState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else
        {
            aMousePos.AdjustX( -(maFrameData.maMouseOff.X()) );
            aMousePos.AdjustY( -(maFrameData.maMouseOff.Y()) );

            if ( maFrameData.mnHitTest & BorderWindowHitTest::Title )
            {
                maFrameData.mpBorderWindow->SetPointer( PointerStyle::Move );

                Point aPos = pBorderWindow->GetPosPixel();
                aPos.AdjustX(aMousePos.X() );
                aPos.AdjustY(aMousePos.Y() );
                if ( maFrameData.mbDragFull )
                {
                    pBorderWindow->SetPosPixel( aPos );
                    pBorderWindow->ImplUpdateAll();
                    pBorderWindow->ImplGetFrameWindow()->ImplUpdateAll();
                }
                else
                {
                    maFrameData.mnTrackX = aPos.X();
                    maFrameData.mnTrackY = aPos.Y();
                    pBorderWindow->ShowTracking( tools::Rectangle( pBorderWindow->ScreenToOutputPixel( aPos ), pBorderWindow->GetOutputSizePixel() ), ShowTrackFlags::Big );
                }
            }
            else
            {
                Point       aOldPos         = pBorderWindow->GetPosPixel();
                Size        aSize           = pBorderWindow->GetSizePixel();
                tools::Rectangle   aNewRect( aOldPos, aSize );
                tools::Long        nOldWidth       = aSize.Width();
                tools::Long        nOldHeight      = aSize.Height();
                tools::Long        nBorderWidth    = maFrameData.mnLeftBorder+maFrameData.mnRightBorder;
                tools::Long        nBorderHeight   = maFrameData.mnTopBorder+maFrameData.mnBottomBorder;
                tools::Long        nMinWidth       = pBorderWindow->mnMinWidth+nBorderWidth;
                tools::Long        nMinHeight      = pBorderWindow->mnMinHeight+nBorderHeight;
                tools::Long        nMinWidth2      = nBorderWidth;
                tools::Long        nMaxWidth       = pBorderWindow->mnMaxWidth+nBorderWidth;
                tools::Long        nMaxHeight      = pBorderWindow->mnMaxHeight+nBorderHeight;

                if ( maFrameData.mnTitleHeight )
                {
                    nMinWidth2 += 4;

                    if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
                        nMinWidth2 += maFrameData.maCloseRect.GetWidth();
                }
                if ( nMinWidth2 > nMinWidth )
                    nMinWidth = nMinWidth2;
                if ( maFrameData.mnHitTest & (BorderWindowHitTest::Left | BorderWindowHitTest::TopLeft | BorderWindowHitTest::BottomLeft) )
                {
                    aNewRect.AdjustLeft(aMousePos.X() );
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.SetLeft( aNewRect.Right()-nMinWidth+1 );
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.SetLeft( aNewRect.Right()-nMaxWidth+1 );
                }
                else if ( maFrameData.mnHitTest & (BorderWindowHitTest::Right | BorderWindowHitTest::TopRight | BorderWindowHitTest::BottomRight) )
                {
                    aNewRect.AdjustRight(aMousePos.X() );
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.SetRight( aNewRect.Left()+nMinWidth+1 );
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.SetRight( aNewRect.Left()+nMaxWidth+1 );
                }
                if ( maFrameData.mnHitTest & (BorderWindowHitTest::Top | BorderWindowHitTest::TopLeft | BorderWindowHitTest::TopRight) )
                {
                    aNewRect.AdjustTop(aMousePos.Y() );
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.SetTop( aNewRect.Bottom()-nMinHeight+1 );
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.SetTop( aNewRect.Bottom()-nMaxHeight+1 );
                }
                else if ( maFrameData.mnHitTest & (BorderWindowHitTest::Bottom | BorderWindowHitTest::BottomLeft | BorderWindowHitTest::BottomRight) )
                {
                    aNewRect.AdjustBottom(aMousePos.Y() );
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.SetBottom( aNewRect.Top()+nMinHeight+1 );
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.SetBottom( aNewRect.Top()+nMaxHeight+1 );
                }

                // call Resizing-Handler for SystemWindows
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    // adjust size for Resizing-call
                    aSize = aNewRect.GetSize();
                    aSize.AdjustWidth( -nBorderWidth );
                    aSize.AdjustHeight( -nBorderHeight );
                    static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow())->Resizing( aSize );
                    aSize.AdjustWidth(nBorderWidth );
                    aSize.AdjustHeight(nBorderHeight );
                    if ( aSize.Width() < nMinWidth )
                        aSize.setWidth( nMinWidth );
                    if ( aSize.Height() < nMinHeight )
                        aSize.setHeight( nMinHeight );
                    if ( aSize.Width() > nMaxWidth )
                        aSize.setWidth( nMaxWidth );
                    if ( aSize.Height() > nMaxHeight )
                        aSize.setHeight( nMaxHeight );
                    if ( maFrameData.mnHitTest & (BorderWindowHitTest::Left | BorderWindowHitTest::TopLeft | BorderWindowHitTest::BottomLeft) )
                        aNewRect.SetLeft( aNewRect.Right()-aSize.Width()+1 );
                    else
                        aNewRect.SetRight( aNewRect.Left()+aSize.Width()-1 );
                    if ( maFrameData.mnHitTest & (BorderWindowHitTest::Top | BorderWindowHitTest::TopLeft | BorderWindowHitTest::TopRight) )
                        aNewRect.SetTop( aNewRect.Bottom()-aSize.Height()+1 );
                    else
                        aNewRect.SetBottom( aNewRect.Top()+aSize.Height()-1 );
                }

                if ( maFrameData.mbDragFull )
                {
                    // no move (only resize) if position did not change
                    if( aOldPos != aNewRect.TopLeft() )
                        pBorderWindow->setPosSizePixel( aNewRect.Left(), aNewRect.Top(),
                                                    aNewRect.GetWidth(), aNewRect.GetHeight() );
                    else
                        pBorderWindow->setPosSizePixel( aNewRect.Left(), aNewRect.Top(),
                                                    aNewRect.GetWidth(), aNewRect.GetHeight(), PosSizeFlags::Size );

                    pBorderWindow->ImplUpdateAll();
                    pBorderWindow->ImplGetFrameWindow()->ImplUpdateAll();
                    if ( maFrameData.mnHitTest & (BorderWindowHitTest::Right | BorderWindowHitTest::TopRight | BorderWindowHitTest::BottomRight) )
                        maFrameData.maMouseOff.AdjustX(aNewRect.GetWidth()-nOldWidth );
                    if ( maFrameData.mnHitTest & (BorderWindowHitTest::Bottom | BorderWindowHitTest::BottomLeft | BorderWindowHitTest::BottomRight) )
                        maFrameData.maMouseOff.AdjustY(aNewRect.GetHeight()-nOldHeight );
                }
                else
                {
                    maFrameData.mnTrackX        = aNewRect.Left();
                    maFrameData.mnTrackY        = aNewRect.Top();
                    maFrameData.mnTrackWidth    = aNewRect.GetWidth();
                    maFrameData.mnTrackHeight   = aNewRect.GetHeight();
                    pBorderWindow->ShowTracking( tools::Rectangle( pBorderWindow->ScreenToOutputPixel( aNewRect.TopLeft() ), aNewRect.GetSize() ), ShowTrackFlags::Big );
                }
            }
        }
    }

    return true;
}

OUString ImplStdBorderWindowView::RequestHelp( const Point& rPos, tools::Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

tools::Rectangle ImplStdBorderWindowView::GetMenuRect() const
{
    return maFrameData.maMenuRect;
}

void ImplStdBorderWindowView::Init( OutputDevice* pDev, tools::Long nWidth, tools::Long nHeight )
{
    ImplBorderFrameData*    pData = &maFrameData;
    ImplBorderWindow*       pBorderWindow = maFrameData.mpBorderWindow;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    tools::Rectangle               aRect( 0, 0, 10, 10 );
    tools::Rectangle               aCalcRect = aDecoView.DrawFrame( aRect, DrawFrameStyle::DoubleOut, DrawFrameFlags::NoDraw );

    pData->mpOutDev         = pDev;
    pData->mnWidth          = nWidth;
    pData->mnHeight         = nHeight;

    pData->mnTitleType      = pBorderWindow->mnTitleType;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) || (pData->mnTitleType == BorderWindowTitleType::NONE) )
        pData->mnBorderSize = 0;
    else if ( pData->mnTitleType == BorderWindowTitleType::Tearoff )
        pData->mnBorderSize = 0;
    else
        pData->mnBorderSize = StyleSettings::GetBorderSize();
    pData->mnLeftBorder     = aCalcRect.Left();
    pData->mnTopBorder      = aCalcRect.Top();
    pData->mnRightBorder    = aRect.Right()-aCalcRect.Right();
    pData->mnBottomBorder   = aRect.Bottom()-aCalcRect.Bottom();
    pData->mnLeftBorder    += pData->mnBorderSize;
    pData->mnTopBorder     += pData->mnBorderSize;
    pData->mnRightBorder   += pData->mnBorderSize;
    pData->mnBottomBorder  += pData->mnBorderSize;
    pData->mnNoTitleTop     = pData->mnTopBorder;

    ImplInitTitle(&maFrameData);
    if (pData->mnTitleHeight)
    {
        // to improve symbol display force a minimum title height
        if (pData->mnTitleType != BorderWindowTitleType::Tearoff &&
            pData->mnTitleHeight < MIN_CAPTION_HEIGHT)
            pData->mnTitleHeight = MIN_CAPTION_HEIGHT;

        // set a proper background for drawing
        // highlighted buttons in the title
        pBorderWindow->SetBackground( rStyleSettings.GetFaceColor() );

        pData->maTitleRect.SetLeft( pData->mnLeftBorder );
        pData->maTitleRect.SetRight( nWidth-pData->mnRightBorder-1 );
        pData->maTitleRect.SetTop( pData->mnTopBorder );
        pData->maTitleRect.SetBottom( pData->maTitleRect.Top()+pData->mnTitleHeight-1 );

        if ( pData->mnTitleType & (BorderWindowTitleType::Normal | BorderWindowTitleType::Small) )
        {
            tools::Long nRight         = pData->maTitleRect.Right() - 3;
            tools::Long const nItemTop = pData->maTitleRect.Top() + 2;
            tools::Long const nItemBottom = pData->maTitleRect.Bottom() - 2;

            auto addSquareOnRight = [&nRight, nItemTop, nItemBottom](
                tools::Rectangle & rect, tools::Long gap)
            {
                rect.SetTop( nItemTop );
                rect.SetBottom( nItemBottom );
                rect.SetRight( nRight );
                rect.SetLeft( rect.Right() - rect.GetHeight() + 1 );
                nRight -= rect.GetWidth() + gap;
            };

            if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
            {
                addSquareOnRight(pData->maCloseRect, 3);
            }

            if ( pBorderWindow->mbMenuBtn )
            {
                addSquareOnRight(pData->maMenuRect, 0);
            }

            if ( pBorderWindow->mbDockBtn )
            {
                addSquareOnRight(pData->maDockRect, 0);
            }

            if ( pBorderWindow->mbHideBtn )
            {
                addSquareOnRight(pData->maHideRect, 0);
            }
        }
        else
        {
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
            pData->maMenuRect.SetEmpty();
            pData->maHideRect.SetEmpty();
            pData->maHelpRect.SetEmpty();
        }

        pData->mnTopBorder  += pData->mnTitleHeight;
    }
    else
    {
        pData->maTitleRect.SetEmpty();
        pData->maCloseRect.SetEmpty();
        pData->maDockRect.SetEmpty();
        pData->maMenuRect.SetEmpty();
        pData->maHideRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

void ImplStdBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

tools::Long ImplStdBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

void ImplStdBorderWindowView::DrawWindow(vcl::RenderContext& rRenderContext, const Point* pOffset)
{
    ImplBorderFrameData* pData = &maFrameData;
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    Point aTmpPoint = pOffset ? *pOffset : Point();
    tools::Rectangle aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    Color aFaceColor(rStyleSettings.GetFaceColor());
    Color aFrameColor(aFaceColor);

    aFrameColor.DecreaseContrast(sal_uInt8(0.5 * 255));

    // Draw Frame
    vcl::Region oldClipRgn(rRenderContext.GetClipRegion());

    // for popups, don't draw part of the frame
    const bool bShowJunctionToLauncher = !(pData->mnTitleType & (BorderWindowTitleType::Normal | BorderWindowTitleType::Small));
    if (bShowJunctionToLauncher && !ImplGetSVData()->maNWFData.mbNoFrameJunctionForPopups)
    {
        FloatingWindow* pWin = dynamic_cast<FloatingWindow*>(pData->mpBorderWindow->GetWindow(GetWindowType::Client));
        if (pWin)
        {
            vcl::Region aClipRgn(aInRect);
            AbsoluteScreenPixelRectangle aItemClipRect(pWin->ImplGetItemEdgeClipRect());
            if (!aItemClipRect.IsEmpty())
            {
                tools::Rectangle aTmp(pData->mpBorderWindow->AbsoluteScreenToOutputPixel(aItemClipRect.TopLeft()), aItemClipRect.GetSize());
                aClipRgn.Exclude(aTmp);
                rRenderContext.SetClipRegion(aClipRgn);
            }
        }
    }

    // single line frame
    rRenderContext.SetLineColor(aFrameColor);
    rRenderContext.SetFillColor();
    rRenderContext.DrawRect(aInRect);
    aInRect.AdjustLeft( 1 );
    aInRect.AdjustRight( -1 );
    aInRect.AdjustTop( 1 );
    aInRect.AdjustBottom( -1 );

    // restore
    if (!(pData->mnTitleType & (BorderWindowTitleType::Normal | BorderWindowTitleType::Small)))
        rRenderContext.SetClipRegion(oldClipRgn);

    // Draw Border
    rRenderContext.SetLineColor();
    tools::Long nBorderSize = pData->mnBorderSize;
    if (nBorderSize)
    {
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        rRenderContext.DrawRect(tools::Rectangle(Point(aInRect.Left(), aInRect.Top()),
                                 Size(aInRect.GetWidth(), nBorderSize)));
        rRenderContext.DrawRect(tools::Rectangle(Point(aInRect.Left(), aInRect.Top() + nBorderSize),
                                 Size(nBorderSize, aInRect.GetHeight() - nBorderSize)));
        rRenderContext.DrawRect(tools::Rectangle(Point(aInRect.Left(), aInRect.Bottom() - nBorderSize + 1),
                                 Size(aInRect.GetWidth(), nBorderSize)));
        rRenderContext.DrawRect(tools::Rectangle(Point(aInRect.Right()-nBorderSize + 1, aInRect.Top() + nBorderSize),
                                 Size(nBorderSize, aInRect.GetHeight() - nBorderSize)));
    }

    // Draw Title
    if (!pData->maTitleRect.IsEmpty())
    {
        aInRect = pData->maTitleRect;

        // use no gradient anymore, just a static titlecolor
        if (pData->mnTitleType == BorderWindowTitleType::Tearoff)
            rRenderContext.SetFillColor(rStyleSettings.GetFaceGradientColor());
        else if (pData->mnTitleType == BorderWindowTitleType::Popup)
            rRenderContext.SetFillColor(aFaceColor);
        else
            rRenderContext.SetFillColor(aFrameColor);

        rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
        tools::Rectangle aTitleRect(pData->maTitleRect);
        if(pOffset)
            aTitleRect.Move(pOffset->X(), pOffset->Y());
        rRenderContext.DrawRect(aTitleRect);

        if (pData->mnTitleType != BorderWindowTitleType::Tearoff)
        {
            aInRect.AdjustLeft(2 );
            aInRect.AdjustRight( -2 );

            if (!pData->maHelpRect.IsEmpty())
                aInRect.SetRight( pData->maHelpRect.Left() - 2 );
            else if (!pData->maHideRect.IsEmpty())
                aInRect.SetRight( pData->maHideRect.Left() - 2 );
            else if (!pData->maDockRect.IsEmpty())
                aInRect.SetRight( pData->maDockRect.Left() - 2 );
            else if (!pData->maMenuRect.IsEmpty())
                aInRect.SetRight( pData->maMenuRect.Left() - 2 );
            else if (!pData->maCloseRect.IsEmpty())
                aInRect.SetRight( pData->maCloseRect.Left() - 2 );

            if (pOffset)
                aInRect.Move(pOffset->X(), pOffset->Y());

            DrawTextFlags nTextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis | DrawTextFlags::Clip;

            // must show tooltip ?
            TextRectInfo aInfo;
            rRenderContext.GetTextRect(aInRect, pBorderWindow->GetText(), nTextStyle, &aInfo);
            pData->mbTitleClipped = aInfo.IsEllipses();

            rRenderContext.DrawText(aInRect, pBorderWindow->GetText(), nTextStyle);
        }
        else
        {
            ToolBox::ImplDrawGrip(rRenderContext, aTitleRect, ToolBox::ImplGetDragWidth(rRenderContext, false),
                                  WindowAlign::Left, false);
        }
    }

    if (!pData->maCloseRect.IsEmpty())
    {
        tools::Rectangle aSymbolRect(pData->maCloseRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::CLOSE, pData->mnCloseState);
    }
    if (!pData->maDockRect.IsEmpty())
    {
        tools::Rectangle aSymbolRect(pData->maDockRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::DOCK, pData->mnDockState);
    }
    if (!pData->maMenuRect.IsEmpty())
    {
        tools::Rectangle aSymbolRect(pData->maMenuRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::MENU, pData->mnMenuState);
    }
    if (!pData->maHideRect.IsEmpty())
    {
        tools::Rectangle aSymbolRect(pData->maHideRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::HIDE, pData->mnHideState);
    }

    if (!pData->maHelpRect.IsEmpty())
    {
        tools::Rectangle aSymbolRect(pData->maHelpRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::HELP, pData->mnHelpState);
    }
}

void ImplBorderWindow::ImplInit( vcl::Window* pParent,
                                 WinBits nStyle, BorderWindowStyle nTypeStyle,
                                 SystemParentData* pSystemParentData
                                 )
{
    // remove all unwanted WindowBits
    WinBits nOrgStyle = nStyle;
    WinBits nTestStyle = (WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_DIALOGCONTROL | WB_NODIALOGCONTROL | WB_SYSTEMFLOATWIN | WB_INTROWIN | WB_DEFAULTWIN | WB_TOOLTIPWIN | WB_NOSHADOW | WB_OWNERDRAWDECORATION | WB_SYSTEMCHILDWINDOW  | WB_POPUP);
    if ( nTypeStyle & BorderWindowStyle::App )
        nTestStyle |= WB_APP;
    nStyle &= nTestStyle;

    mpWindowImpl->mbBorderWin       = true;
    mbSmallOutBorder    = false;
    if ( nTypeStyle & BorderWindowStyle::Frame )
    {
        mpWindowImpl->mbOverlapWin = true;
        mpWindowImpl->mbFrame = true;

        if( nStyle & WB_SYSTEMCHILDWINDOW )
        {
            mbFrameBorder               = false;
        }
        else if( nStyle & (WB_OWNERDRAWDECORATION | WB_POPUP) )
        {
            mbFrameBorder   = (nOrgStyle & WB_NOBORDER) == 0;
        }
        else
        {
            mbFrameBorder   = false;
            // closeable windows may have a border as well, eg. system floating windows without caption
            if ( (nOrgStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE/* | WB_CLOSEABLE*/)) == WB_BORDER )
                mbSmallOutBorder = true;
        }
    }
    else if ( nTypeStyle & BorderWindowStyle::Overlap )
    {
        mpWindowImpl->mbOverlapWin  = true;
        mbFrameBorder   = true;
    }
    else
        mbFrameBorder   = false;

    if ( nTypeStyle & BorderWindowStyle::Float )
        mbFloatWindow = true;
    else
        mbFloatWindow = false;

    Window::ImplInit( pParent, nStyle, pSystemParentData );
    SetBackground();
    SetTextFillColor();

    mpMenuBarWindow = nullptr;
    mnMinWidth      = 0;
    mnMinHeight     = 0;
    mnMaxWidth      = SHRT_MAX;
    mnMaxHeight     = SHRT_MAX;
    mnOrgMenuHeight = 0;
    mbMenuHide      = false;
    mbDockBtn       = false;
    mbMenuBtn       = false;
    mbHideBtn       = false;
    mbDisplayActive = IsActive();

    if ( nTypeStyle & BorderWindowStyle::Float )
        mnTitleType = BorderWindowTitleType::Small;
    else
        mnTitleType = BorderWindowTitleType::Normal;
    mnBorderStyle   = WindowBorderStyle::NORMAL;
    InitView();
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent,
                                    SystemParentData* pSystemParentData,
                                    WinBits nStyle, BorderWindowStyle nTypeStyle
                                    ) : Window( WindowType::BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, pSystemParentData );
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent, WinBits nStyle ,
                                    BorderWindowStyle nTypeStyle ) :
    Window( WindowType::BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, nullptr );
}

ImplBorderWindow::~ImplBorderWindow()
{
    disposeOnce();
}

void ImplBorderWindow::dispose()
{
    mpBorderView.reset();
    mpMenuBarWindow.clear();
    mpNotebookBar.disposeAndClear();
    vcl::Window::dispose();
}

void ImplBorderWindow::MouseMove( const MouseEvent& rMEvt )
{
    if (mpBorderView)
        mpBorderView->MouseMove( rMEvt );
}

void ImplBorderWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (mpBorderView)
        mpBorderView->MouseButtonDown( rMEvt );
}

void ImplBorderWindow::Tracking( const TrackingEvent& rTEvt )
{
    if (mpBorderView)
        mpBorderView->Tracking( rTEvt );
}

void ImplBorderWindow::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    if (mpBorderView)
        mpBorderView->DrawWindow(rRenderContext);
}

void ImplBorderWindow::Draw( OutputDevice* pOutDev, const Point& rPos )
{
    if (mpBorderView)
        mpBorderView->DrawWindow(*pOutDev, &rPos);
}

void ImplBorderWindow::Activate()
{
    SetDisplayActive( true );
    Window::Activate();
}

void ImplBorderWindow::Deactivate()
{
    // remove active windows from the ruler, also ignore the Deactivate
    // if a menu becomes active
    if (GetActivateMode() != ActivateModeFlags::NONE && !ImplGetSVData()->mpWinData->mbNoDeactivate)
        SetDisplayActive( false );
    Window::Deactivate();
}

void ImplBorderWindow::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help for border window
    if ( rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK) && !rHEvt.KeyboardActivated() )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        tools::Rectangle   aHelpRect;
        OUString    aHelpStr( mpBorderView->RequestHelp( aMousePosPixel, aHelpRect ) );

        // retrieve rectangle
        if ( !aHelpStr.isEmpty() )
        {
            aHelpRect.SetPos( OutputToScreenPixel( aHelpRect.TopLeft() ) );
            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                Help::ShowBalloon( this, aHelpRect.Center(), aHelpRect, aHelpStr );
            else
                Help::ShowQuickHelp( this, aHelpRect, aHelpStr );
            return;
        }
    }

    Window::RequestHelp( rHEvt );
}

void ImplBorderWindow::Resize()
{
    Size aSize = GetOutputSizePixel();

    vcl::Window* pClientWindow = ImplGetClientWindow();

    sal_Int32 nLeftBorder;
    sal_Int32 nTopBorder;
    sal_Int32 nRightBorder;
    sal_Int32 nBottomBorder;
    mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );

    if (mpMenuBarWindow)
    {
        tools::Long nMenuHeight = mpMenuBarWindow->GetSizePixel().Height();
        if ( mbMenuHide )
        {
            if ( nMenuHeight )
                mnOrgMenuHeight = nMenuHeight;
            nMenuHeight = 0;
        }
        else
        {
            if ( !nMenuHeight )
                nMenuHeight = mnOrgMenuHeight;
        }
        mpMenuBarWindow->setPosSizePixel(
                nLeftBorder, nTopBorder,
                aSize.Width()-nLeftBorder-nRightBorder,
                nMenuHeight);

        // shift the notebookbar down accordingly
        nTopBorder += nMenuHeight;
    }

    if (mpNotebookBar)
    {
        tools::Long nNotebookBarHeight = mpNotebookBar->GetSizePixel().Height();

        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        const BitmapEx& aPersona = rStyleSettings.GetPersonaHeader();
        // since size of notebookbar changes, to make common persona for menubar
        // and notebookbar persona should be set again with changed coordinates
        if (!aPersona.IsEmpty())
        {
            Wallpaper aWallpaper(aPersona);
            aWallpaper.SetStyle(WallpaperStyle::TopRight);
            aWallpaper.SetRect(tools::Rectangle(Point(0, -nTopBorder),
                   Size(aSize.Width() - nLeftBorder - nRightBorder,
                        nNotebookBarHeight + nTopBorder)));
            mpNotebookBar->SetBackground(aWallpaper);
        }

        mpNotebookBar->setPosSizePixel(
                nLeftBorder, nTopBorder,
                aSize.Width() - nLeftBorder - nRightBorder,
                nNotebookBarHeight);
    }

    GetBorder( pClientWindow->mpWindowImpl->mnLeftBorder, pClientWindow->mpWindowImpl->mnTopBorder,
               pClientWindow->mpWindowImpl->mnRightBorder, pClientWindow->mpWindowImpl->mnBottomBorder );
    pClientWindow->ImplPosSizeWindow( pClientWindow->mpWindowImpl->mnLeftBorder,
                                      pClientWindow->mpWindowImpl->mnTopBorder,
                                      aSize.Width()-pClientWindow->mpWindowImpl->mnLeftBorder-pClientWindow->mpWindowImpl->mnRightBorder,
                                      aSize.Height()-pClientWindow->mpWindowImpl->mnTopBorder-pClientWindow->mpWindowImpl->mnBottomBorder,
                                      PosSizeFlags::X | PosSizeFlags::Y |
                                      PosSizeFlags::Width | PosSizeFlags::Height );

    // UpdateView
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    InvalidateBorder();

    Window::Resize();
}

void ImplBorderWindow::StateChanged( StateChangedType nType )
{
    if ( (nType == StateChangedType::Text) ||
         (nType == StateChangedType::Data) )
    {
        if (IsReallyVisible() && mbFrameBorder)
            InvalidateBorder();
    }

    Window::StateChanged( nType );
}

void ImplBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        if ( !mpWindowImpl->mbFrame || (GetStyle() & (WB_OWNERDRAWDECORATION | WB_POPUP)) )
            UpdateView( true, ImplGetWindow()->GetOutputSizePixel() );
    }

    Window::DataChanged( rDCEvt );
}

void ImplBorderWindow::InitView()
{
    if ( mbSmallOutBorder )
        mpBorderView.reset(new ImplSmallBorderWindowView( this ));
    else if ( mpWindowImpl->mbFrame )
    {
        if( mbFrameBorder )
            mpBorderView.reset(new ImplStdBorderWindowView( this ));
        else
            mpBorderView.reset(new ImplNoBorderWindowView);
    }
    else if ( !mbFrameBorder )
        mpBorderView.reset(new ImplSmallBorderWindowView( this ));
    else
        mpBorderView.reset(new ImplStdBorderWindowView( this ));
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
}

void ImplBorderWindow::UpdateView( bool bNewView, const Size& rNewOutSize )
{
    sal_Int32 nLeftBorder;
    sal_Int32 nTopBorder;
    sal_Int32 nRightBorder;
    sal_Int32 nBottomBorder;
    Size aOldSize = GetSizePixel();
    Size aOutputSize = rNewOutSize;

    if ( bNewView )
    {
        mpBorderView.reset();
        InitView();
    }
    else
    {
        Size aSize = aOutputSize;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        aSize.AdjustWidth(nLeftBorder+nRightBorder );
        aSize.AdjustHeight(nTopBorder+nBottomBorder );
        mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    }

    vcl::Window* pClientWindow = ImplGetClientWindow();
    if ( pClientWindow )
    {
        GetBorder( pClientWindow->mpWindowImpl->mnLeftBorder, pClientWindow->mpWindowImpl->mnTopBorder,
                   pClientWindow->mpWindowImpl->mnRightBorder, pClientWindow->mpWindowImpl->mnBottomBorder );
    }
    GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
    if ( aOldSize.Width() || aOldSize.Height() )
    {
        aOutputSize.AdjustWidth(nLeftBorder+nRightBorder );
        aOutputSize.AdjustHeight(nTopBorder+nBottomBorder );
        if ( aOutputSize == GetSizePixel() )
            InvalidateBorder();
        else
            SetSizePixel( aOutputSize );
    }
}

void ImplBorderWindow::InvalidateBorder()
{
    if ( !IsReallyVisible() )
        return;

    // invalidate only if we have a border
    sal_Int32 nLeftBorder;
    sal_Int32 nTopBorder;
    sal_Int32 nRightBorder;
    sal_Int32 nBottomBorder;
    mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
    if ( !(nLeftBorder || nTopBorder || nRightBorder || nBottomBorder) )
        return;

    tools::Rectangle   aWinRect( Point( 0, 0 ), GetOutputSizePixel() );
    vcl::Region      aRegion( aWinRect );
    aWinRect.AdjustLeft(nLeftBorder );
    aWinRect.AdjustTop(nTopBorder );
    aWinRect.AdjustRight( -nRightBorder );
    aWinRect.AdjustBottom( -nBottomBorder );
    // no output area anymore, now invalidate all
    if ( (aWinRect.Right() < aWinRect.Left()) ||
         (aWinRect.Bottom() < aWinRect.Top()) )
        Invalidate( InvalidateFlags::NoChildren );
    else
    {
        aRegion.Exclude( aWinRect );
        Invalidate( aRegion, InvalidateFlags::NoChildren );
    }
}

void ImplBorderWindow::SetDisplayActive( bool bActive )
{
    if ( mbDisplayActive != bActive )
    {
        mbDisplayActive = bActive;
        if ( mbFrameBorder )
            InvalidateBorder();
    }
}

void ImplBorderWindow::SetTitleType( BorderWindowTitleType nTitleType, const Size& rSize )
{
    mnTitleType = nTitleType;
    UpdateView( false, rSize );
}

void ImplBorderWindow::SetBorderStyle( WindowBorderStyle nStyle )
{
    if ( !mbFrameBorder && (mnBorderStyle != nStyle) )
    {
        mnBorderStyle = nStyle;
        UpdateView( false, ImplGetWindow()->GetOutputSizePixel() );
    }
}

void ImplBorderWindow::SetCloseButton()
{
    SetStyle( GetStyle() | WB_CLOSEABLE );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetDockButton( bool bDockButton )
{
    mbDockBtn = bDockButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetHideButton( bool bHideButton )
{
    mbHideBtn = bHideButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetMenuButton( bool bMenuButton )
{
    mbMenuBtn = bMenuButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( GetOutDev(), aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::UpdateMenuHeight()
{
    Resize();
}

void ImplBorderWindow::SetMenuBarWindow( vcl::Window* pWindow )
{
    mpMenuBarWindow = pWindow;
    UpdateMenuHeight();
    if ( pWindow )
        pWindow->Show();
}

void ImplBorderWindow::SetMenuBarMode( bool bHide )
{
    mbMenuHide = bHide;
    UpdateMenuHeight();
}

void ImplBorderWindow::SetNotebookBar(const OUString& rUIXMLDescription,
                                      const css::uno::Reference<css::frame::XFrame>& rFrame,
                                      const NotebookBarAddonsItem& aNotebookBarAddonsItem)
{
    if (mpNotebookBar)
        mpNotebookBar.disposeAndClear();
    mpNotebookBar = VclPtr<NotebookBar>::Create(this, "NotebookBar", rUIXMLDescription, rFrame,
                                                aNotebookBarAddonsItem);
    Resize();
}

void ImplBorderWindow::CloseNotebookBar()
{
    if (mpNotebookBar)
        mpNotebookBar.disposeAndClear();
    mpNotebookBar = nullptr;
    Resize();
}

void ImplBorderWindow::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                  sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    mpBorderView->GetBorder(rLeftBorder, rTopBorder, rRightBorder, rBottomBorder);

    if (mpMenuBarWindow && !mbMenuHide)
        rTopBorder += mpMenuBarWindow->GetSizePixel().Height();

    if (mpNotebookBar && mpNotebookBar->IsVisible())
        rTopBorder += mpNotebookBar->GetSizePixel().Height();
}

tools::Long ImplBorderWindow::CalcTitleWidth() const
{
    return mpBorderView->CalcTitleWidth();
}

tools::Rectangle ImplBorderWindow::GetMenuRect() const
{
    return mpBorderView->GetMenuRect();
}

Size ImplBorderWindow::GetOptimalSize() const
{
    const vcl::Window* pClientWindow = ImplGetClientWindow();
    if (pClientWindow)
        return pClientWindow->GetOptimalSize();
    return Size(mnMinWidth, mnMinHeight);
}

void ImplBorderWindow::queue_resize(StateChangedType eReason)
{
    //if we are floating, then we don't want to inform our parent that it needs
    //to calculate a new layout allocation. Because while we are a child
    //of our parent we are not embedded into the parent so it doesn't care
    //about us.
    if (mbFloatWindow)
        return;
    vcl::Window::queue_resize(eReason);
}

void ImplBorderWindow::FlashWindow() const
{
    // We are showing top level window without focus received. Let's flash it
    // Use OS features to bring user attention to this window: find topmost one and FlashWindow
    vcl::Window* pMyParent = mpWindowImpl->mpParent;
    while (pMyParent && pMyParent->mpWindowImpl && pMyParent->mpWindowImpl->mpParent)
        pMyParent = pMyParent->mpWindowImpl->mpParent;

    if (pMyParent && pMyParent->mpWindowImpl)
        pMyParent->mpWindowImpl->mpFrame->FlashWindow();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
