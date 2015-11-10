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

#include <svids.hrc>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/syswin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/help.hxx>
#include <vcl/edit.hxx>
#include <vcl/metric.hxx>
#include <vcl/settings.hxx>

#include "notebookbarwindow.hxx"

using namespace ::com::sun::star::uno;

// useful caption height for title bar buttons
#define MIN_CAPTION_HEIGHT 18

static void ImplGetPinImage( DrawButtonFlags nStyle, bool bPinIn, Image& rImage )
{
    // load ImageList if not available yet
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpPinImgList )
    {
        ResMgr* pResMgr = ImplGetResMgr();
        pSVData->maCtrlData.mpPinImgList = new ImageList();
        if( pResMgr )
        {
            Color aMaskColor( 0x00, 0x00, 0xFF );
            pSVData->maCtrlData.mpPinImgList->InsertFromHorizontalBitmap
                ( ResId( SV_RESID_BITMAP_PIN, *pResMgr ), 4,
                  &aMaskColor );
        }
    }

    // get and return Image
    sal_uInt16 nId;
    if ( nStyle & DrawButtonFlags::Pressed )
    {
        if ( bPinIn )
            nId = 4;
        else
            nId = 3;
    }
    else
    {
        if ( bPinIn )
            nId = 2;
        else
            nId = 1;
    }
    rImage = pSVData->maCtrlData.mpPinImgList->GetImage( nId );
}

namespace vcl {

void Window::ImplCalcSymbolRect( Rectangle& rRect )
{
    // Add border, not shown in the non-default representation,
    // as we want to use it for small buttons
    rRect.Left()--;
    rRect.Top()--;
    rRect.Right()++;
    rRect.Bottom()++;

    // we leave 5% room between the symbol and the button border
    long nExtraWidth = ((rRect.GetWidth()*50)+500)/1000;
    long nExtraHeight = ((rRect.GetHeight()*50)+500)/1000;
    rRect.Left()    += nExtraWidth;
    rRect.Right()   -= nExtraWidth;
    rRect.Top()     += nExtraHeight;
    rRect.Bottom()  -= nExtraHeight;
}

} /* namespace vcl */

static void ImplDrawBrdWinSymbol( vcl::RenderContext* pDev,
                                  const Rectangle& rRect, SymbolType eSymbol )
{
    // we leave 5% room between the symbol and the button border
    DecorationView  aDecoView( pDev );
    Rectangle       aTempRect = rRect;
    vcl::Window::ImplCalcSymbolRect( aTempRect );
    aDecoView.DrawSymbol( aTempRect, eSymbol,
                          pDev->GetSettings().GetStyleSettings().GetButtonTextColor() );
}

static void ImplDrawBrdWinSymbolButton( vcl::RenderContext* pDev,
                                        const Rectangle& rRect,
                                        SymbolType eSymbol, DrawButtonFlags nState )
{
    bool bMouseOver(nState & DrawButtonFlags::Highlight);
    nState &= ~DrawButtonFlags::Highlight;

    Rectangle aTempRect;
    vcl::Window *pWin = dynamic_cast< vcl::Window* >(pDev);
    if( pWin )
    {
        if( bMouseOver )
        {
            // provide a bright background for selection effect
            pDev->SetFillColor( pDev->GetSettings().GetStyleSettings().GetWindowColor() );
            pDev->SetLineColor();
            pDev->DrawRect( rRect );
            pWin->DrawSelectionBackground( rRect, 2, bool(nState & DrawButtonFlags::Pressed),
                                            true, false );
        }
        aTempRect = rRect;
        aTempRect.Left()+=3;
        aTempRect.Right()-=4;
        aTempRect.Top()+=3;
        aTempRect.Bottom()-=4;
    }
    else
    {
        DecorationView aDecoView( pDev );
        aTempRect = aDecoView.DrawButton( rRect, nState|DrawButtonFlags::Flat );
    }
    ImplDrawBrdWinSymbol( pDev, aTempRect, eSymbol );
}

// - ImplBorderWindowView -

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

OUString ImplBorderWindowView::RequestHelp( const Point&, Rectangle& )
{
    return OUString();
}

Rectangle ImplBorderWindowView::GetMenuRect() const
{
    return Rectangle();
}

void ImplBorderWindowView::ImplInitTitle(ImplBorderFrameData* pData)
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) ||
          (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
    {
        pData->mnTitleType   = BORDERWINDOW_TITLE_NONE;
        pData->mnTitleHeight = 0;
    }
    else
    {
        const StyleSettings& rStyleSettings = pData->mpOutDev->GetSettings().GetStyleSettings();
        if (pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF)
            pData->mnTitleHeight = rStyleSettings.GetTearOffTitleHeight();
        else
        {
            if (pData->mnTitleType == BORDERWINDOW_TITLE_SMALL)
            {
                pBorderWindow->SetPointFont(*pBorderWindow, rStyleSettings.GetFloatTitleFont() );
                pData->mnTitleHeight = rStyleSettings.GetFloatTitleHeight();
            }
            else // pData->mnTitleType == BORDERWINDOW_TITLE_NORMAL
            {
                // FIXME RenderContext
                pBorderWindow->SetPointFont(*pBorderWindow, rStyleSettings.GetTitleFont());
                pData->mnTitleHeight = rStyleSettings.GetTitleHeight();
            }
            long nTextHeight = pBorderWindow->GetTextHeight();
            if (nTextHeight > pData->mnTitleHeight)
                pData->mnTitleHeight = nTextHeight;
        }
    }
}

sal_uInt16 ImplBorderWindowView::ImplHitTest( ImplBorderFrameData* pData, const Point& rPos )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( pData->maTitleRect.IsInside( rPos ) )
    {
        if ( pData->maCloseRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_CLOSE;
        else if ( pData->maRollRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_ROLL;
        else if ( pData->maMenuRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_MENU;
        else if ( pData->maDockRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_DOCK;
        else if ( pData->maHideRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_HIDE;
        else if ( pData->maHelpRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_HELP;
        else if ( pData->maPinRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_PIN;
        else
            return BORDERWINDOW_HITTEST_TITLE;
    }

    if ( (pBorderWindow->GetStyle() & WB_SIZEABLE) &&
         !pBorderWindow->mbRollUp )
    {
        long nSizeWidth = pData->mnNoTitleTop+pData->mnTitleHeight;
        if ( nSizeWidth < 16 )
            nSizeWidth = 16;

        // no corner resize for floating toolbars, which would lead to jumps while formatting
        // setting nSizeWidth = 0 will only return pure left,top,right,bottom
        if( pBorderWindow->GetStyle() & (WB_OWNERDRAWDECORATION | WB_POPUP) )
            nSizeWidth = 0;

        if ( rPos.X() < pData->mnLeftBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPLEFT;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMLEFT;
            else
                return BORDERWINDOW_HITTEST_LEFT;
        }
        else if ( rPos.X() >= pData->mnWidth-pData->mnRightBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPRIGHT;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMRIGHT;
            else
                return BORDERWINDOW_HITTEST_RIGHT;
        }
        else if ( rPos.Y() < pData->mnNoTitleTop )
        {
            if ( rPos.X() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPLEFT;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPRIGHT;
            else
                return BORDERWINDOW_HITTEST_TOP;
        }
        else if ( rPos.Y() >= pData->mnHeight-pData->mnBottomBorder )
        {
            if ( rPos.X() < nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMLEFT;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMRIGHT;
            else
                return BORDERWINDOW_HITTEST_BOTTOM;
        }
    }

    return 0;
}

bool ImplBorderWindowView::ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    DrawButtonFlags oldCloseState = pData->mnCloseState;
    DrawButtonFlags oldMenuState = pData->mnMenuState;
    pData->mnCloseState &= ~DrawButtonFlags::Highlight;
    pData->mnMenuState &= ~DrawButtonFlags::Highlight;

    Point aMousePos = rMEvt.GetPosPixel();
    sal_uInt16 nHitTest = ImplHitTest( pData, aMousePos );
    PointerStyle ePtrStyle = PointerStyle::Arrow;
    if ( nHitTest & BORDERWINDOW_HITTEST_LEFT )
        ePtrStyle = PointerStyle::WindowWSize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_RIGHT )
        ePtrStyle = PointerStyle::WindowESize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOP )
        ePtrStyle = PointerStyle::WindowNSize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOM )
        ePtrStyle = PointerStyle::WindowSSize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOPLEFT )
        ePtrStyle = PointerStyle::WindowNWSize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOMRIGHT )
        ePtrStyle = PointerStyle::WindowSESize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOPRIGHT )
        ePtrStyle = PointerStyle::WindowNESize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOMLEFT )
        ePtrStyle = PointerStyle::WindowSWSize;
    else if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        pData->mnCloseState |= DrawButtonFlags::Highlight;
    else if ( nHitTest & BORDERWINDOW_HITTEST_MENU )
        pData->mnMenuState |= DrawButtonFlags::Highlight;
    pData->mpBorderWindow->SetPointer( Pointer( ePtrStyle ) );

    if( pData->mnCloseState != oldCloseState )
        pData->mpBorderWindow->Invalidate( pData->maCloseRect );
    if( pData->mnMenuState != oldMenuState )
        pData->mpBorderWindow->Invalidate( pData->maMenuRect );

    return true;
}

OUString ImplBorderWindowView::ImplRequestHelp( ImplBorderFrameData* pData,
                                              const Point& rPos,
                                              Rectangle& rHelpRect )
{
    sal_uInt16 nHelpId = 0;
    OUString aHelpStr;
    sal_uInt16 nHitTest = ImplHitTest( pData, rPos );
    if ( nHitTest )
    {
        if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            nHelpId     = SV_HELPTEXT_CLOSE;
            rHelpRect   = pData->maCloseRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( pData->mpBorderWindow->mbRollUp )
                nHelpId = SV_HELPTEXT_ROLLDOWN;
            else
                nHelpId = SV_HELPTEXT_ROLLUP;
            rHelpRect   = pData->maRollRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_DOCK )
        {
            nHelpId     = SV_HELPTEXT_MAXIMIZE;
            rHelpRect   = pData->maDockRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            nHelpId     = SV_HELPTEXT_MINIMIZE;
            rHelpRect   = pData->maHideRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            nHelpId     = SV_HELPTEXT_HELP;
            rHelpRect   = pData->maHelpRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            nHelpId     = SV_HELPTEXT_ALWAYSVISIBLE;
            rHelpRect   = pData->maPinRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_TITLE )
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

    if( nHelpId && ImplGetResMgr() )
        aHelpStr = ResId(nHelpId, *ImplGetResMgr()).toString();

    return aHelpStr;
}

long ImplBorderWindowView::ImplCalcTitleWidth( const ImplBorderFrameData* pData )
{
    // title is not visible therefore no width
    if ( !pData->mnTitleHeight )
        return 0;

    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    long nTitleWidth = pBorderWindow->GetTextWidth( pBorderWindow->GetText() )+6;
    nTitleWidth += pData->maPinRect.GetWidth();
    nTitleWidth += pData->maCloseRect.GetWidth();
    nTitleWidth += pData->maRollRect.GetWidth();
    nTitleWidth += pData->maDockRect.GetWidth();
    nTitleWidth += pData->maMenuRect.GetWidth();
    nTitleWidth += pData->maHideRect.GetWidth();
    nTitleWidth += pData->maHelpRect.GetWidth();
    nTitleWidth += pData->mnLeftBorder+pData->mnRightBorder;
    return nTitleWidth;
}

// - ImplNoBorderWindowView -

ImplNoBorderWindowView::ImplNoBorderWindowView( ImplBorderWindow* )
{
}

void ImplNoBorderWindowView::Init( OutputDevice*, long, long )
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

long ImplNoBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplNoBorderWindowView::DrawWindow(vcl::RenderContext&, sal_uInt16, const Point*)
{
}

// - ImplSmallBorderWindowView -
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

void ImplSmallBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    mpOutDev    = pDev;
    mnWidth     = nWidth;
    mnHeight    = nHeight;
    mbNWFBorder = false;

    vcl::Window *pWin = nullptr, *pCtrl = nullptr;
    if (mpOutDev->GetOutDevType() == OUTDEV_WINDOW)
        pWin = static_cast<vcl::Window*>(mpOutDev.get());

    if (pWin)
        pCtrl = mpBorderWindow->GetWindow(GetWindowType::Client);

    long nOrigLeftBorder = mnLeftBorder;
    long nOrigTopBorder = mnTopBorder;
    long nOrigRightBorder = mnRightBorder;
    long nOrigBottomBorder = mnBottomBorder;

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
        // FIXME: this is currently only on OS X, check with other
        // platforms
        if( ImplGetSVData()->maNWFData.mbNoFocusRects && !( nBorderStyle & WindowBorderStyle::NWF ) )
        {
            // for native widget drawing we must find out what
            // control this border belongs to
            ControlType aCtrlType = 0;
            if (pCtrl)
            {
                switch( pCtrl->GetType() )
                {
                    case WINDOW_LISTBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = CTRL_LISTBOX;
                            mbNWFBorder = true;
                        }
                        break;
                    case WINDOW_COMBOBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = CTRL_COMBOBOX;
                            mbNWFBorder = true;
                        }
                        break;
                    case WINDOW_MULTILINEEDIT:
                        aCtrlType = CTRL_MULTILINE_EDITBOX;
                        mbNWFBorder = true;
                        break;
                    case WINDOW_EDIT:
                    case WINDOW_PATTERNFIELD:
                    case WINDOW_METRICFIELD:
                    case WINDOW_CURRENCYFIELD:
                    case WINDOW_DATEFIELD:
                    case WINDOW_TIMEFIELD:
                    case WINDOW_LONGCURRENCYFIELD:
                    case WINDOW_NUMERICFIELD:
                    case WINDOW_SPINFIELD:
                    case WINDOW_CALCINPUTLINE:
                        mbNWFBorder = true;
                        if (pCtrl->GetStyle() & WB_SPIN)
                            aCtrlType = CTRL_SPINBOX;
                        else
                            aCtrlType = CTRL_EDITBOX;
                        break;
                    default:
                        break;
                }
            }
            if( mbNWFBorder )
            {
                ImplControlValue aControlValue;
                Rectangle aCtrlRegion( (const Point&)Point(), Size( mnWidth < 10 ? 10 : mnWidth, mnHeight < 10 ? 10 : mnHeight ) );
                Rectangle aBounds( aCtrlRegion );
                Rectangle aContent( aCtrlRegion );
                if( pWin->GetNativeControlRegion( aCtrlType, PART_ENTIRE_CONTROL, aCtrlRegion,
                                                  ControlState::ENABLED, aControlValue, OUString(),
                                                  aBounds, aContent ) )
                {
                    mnLeftBorder    = aContent.Left() - aBounds.Left();
                    mnRightBorder   = aBounds.Right() - aContent.Right();
                    mnTopBorder     = aContent.Top() - aBounds.Top();
                    mnBottomBorder  = aBounds.Bottom() - aContent.Bottom();
                    if( mnWidth && mnHeight )
                    {

                        mpBorderWindow->SetPaintTransparent( true );
                        mpBorderWindow->SetBackground();
                        pCtrl->SetPaintTransparent( true );

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
                                    aPos.X() -= (aBounds.GetWidth() - mnWidth) / 2;
                                if( mnHeight < aBounds.GetHeight() )
                                    aPos.Y() -= (aBounds.GetHeight() - mnHeight) / 2;
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
            Rectangle       aRect( 0, 0, 10, 10 );
            Rectangle       aCalcRect = aDecoView.DrawFrame( aRect, nStyle, nFlags );
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

long ImplSmallBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplSmallBorderWindowView::DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point*)
{
    WindowBorderStyle nBorderStyle = mpBorderWindow->GetBorderStyle();
    if (nBorderStyle & WindowBorderStyle::NOBORDER)
        return;

    bool bNativeOK = false;
    // for native widget drawing we must find out what
    // control this border belongs to
    vcl::Window* pCtrl = mpBorderWindow->GetWindow(GetWindowType::Client);

    ControlType aCtrlType = 0;
    ControlPart aCtrlPart = PART_ENTIRE_CONTROL;
    if (pCtrl)
    {
        switch (pCtrl->GetType())
        {
            case WINDOW_MULTILINEEDIT:
                aCtrlType = CTRL_MULTILINE_EDITBOX;
                break;
            case WINDOW_EDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_SPINFIELD:
            case WINDOW_CALCINPUTLINE:
                if (pCtrl->GetStyle() & WB_SPIN)
                    aCtrlType = CTRL_SPINBOX;
                else
                    aCtrlType = CTRL_EDITBOX;
                break;

            case WINDOW_LISTBOX:
            case WINDOW_MULTILISTBOX:
            case WINDOW_TREELISTBOX:
                aCtrlType = CTRL_LISTBOX;
                if (pCtrl->GetStyle() & WB_DROPDOWN)
                    aCtrlPart = PART_ENTIRE_CONTROL;
                else
                    aCtrlPart = PART_WINDOW;
                break;

            case WINDOW_LISTBOXWINDOW:
                aCtrlType = CTRL_LISTBOX;
                aCtrlPart = PART_WINDOW;
                break;

            case WINDOW_COMBOBOX:
            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_LONGCURRENCYBOX:
                if (pCtrl->GetStyle() & WB_DROPDOWN)
                {
                    aCtrlType = CTRL_COMBOBOX;
                    aCtrlPart = PART_ENTIRE_CONTROL;
                }
                else
                {
                    aCtrlType = CTRL_LISTBOX;
                    aCtrlPart = PART_WINDOW;
                }
                break;

            default:
                break;
        }
    }

    if (aCtrlType && pCtrl->IsNativeControlSupported(aCtrlType, aCtrlPart))
    {
        ImplControlValue aControlValue;
        ControlState nState = ControlState::ENABLED;

        if (!mpBorderWindow->IsEnabled())
            nState &= ~ControlState::ENABLED;
        if (mpBorderWindow->HasFocus())
            nState |= ControlState::FOCUSED;
        else if(mbNWFBorder)
        {
            // FIXME: this is currently only on OS X, see if other platforms can profit

            // FIXME: for OS X focus rings all controls need to support GetNativeControlRegion
            // for the dropdown style
            if (pCtrl->HasFocus() || pCtrl->HasChildPathFocus())
                nState |= ControlState::FOCUSED;
        }

        bool bMouseOver = false;
        vcl::Window *pCtrlChild = pCtrl->GetWindow(GetWindowType::FirstChild);
        while(pCtrlChild && !(bMouseOver = pCtrlChild->IsMouseOver()))
        {
            pCtrlChild = pCtrlChild->GetWindow(GetWindowType::Next);
        }

        if (bMouseOver)
            nState |= ControlState::ROLLOVER;

        Point aPoint;
        Rectangle aCtrlRegion(aPoint, Size(mnWidth, mnHeight));

        Rectangle aBoundingRgn(aPoint, Size(mnWidth, mnHeight));
        Rectangle aContentRgn(aCtrlRegion);
        if (!ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
            rRenderContext.GetNativeControlRegion(aCtrlType, aCtrlPart, aCtrlRegion,
                                         nState, aControlValue, OUString(),
                                         aBoundingRgn, aContentRgn))
        {
            aCtrlRegion=aContentRgn;
        }

        bNativeOK = rRenderContext.DrawNativeControl(aCtrlType, aCtrlPart, aCtrlRegion, nState, aControlValue, OUString());

        // if the native theme draws the spinbuttons in one call, make sure the proper settings
        // are passed, this might force a redraw though.... (TODO: improve)
        if ((aCtrlType == CTRL_SPINBOX) && !pCtrl->IsNativeControlSupported(CTRL_SPINBOX, PART_BUTTON_UP))
        {
            Edit* pEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
            if (pEdit && !pEdit->SupportsDoubleBuffering())
                pCtrl->Paint(*pCtrl, Rectangle());  // make sure the buttons are also drawn as they might overwrite the border
        }
    }

    if (bNativeOK)
        return;

    if (nDrawFlags & BORDERWINDOW_DRAW_FRAME)
    {
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
        Point aTmpPoint;
        Rectangle aInRect(aTmpPoint, Size(mnWidth, mnHeight));
        aDecoView.DrawFrame(aInRect, nStyle, nFlags);
    }
}

// - ImplStdBorderWindowView -

ImplStdBorderWindowView::ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = false;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = DrawButtonFlags::NONE;
    maFrameData.mnCloseState    = DrawButtonFlags::NONE;
    maFrameData.mnRollState     = DrawButtonFlags::NONE;
    maFrameData.mnDockState     = DrawButtonFlags::NONE;
    maFrameData.mnMenuState     = DrawButtonFlags::NONE;
    maFrameData.mnHideState     = DrawButtonFlags::NONE;
    maFrameData.mnHelpState     = DrawButtonFlags::NONE;
    maFrameData.mbTitleClipped  = false;

    mpATitleVirDev              = nullptr;
    mpDTitleVirDev              = nullptr;
}

ImplStdBorderWindowView::~ImplStdBorderWindowView()
{
    mpATitleVirDev.disposeAndClear();
    mpDTitleVirDev.disposeAndClear();
}

bool ImplStdBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

bool ImplStdBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    ImplBorderWindow* pBorderWindow = maFrameData.mpBorderWindow;

    if ( rMEvt.IsLeft() || rMEvt.IsRight() )
    {
        maFrameData.maMouseOff = rMEvt.GetPosPixel();
        maFrameData.mnHitTest = ImplHitTest( &maFrameData, maFrameData.maMouseOff );
        if ( maFrameData.mnHitTest )
        {
            DragFullOptions nDragFullTest = DragFullOptions::NONE;
            bool bTracking = true;
            bool bHitTest = true;

            if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_CLOSE )
            {
                maFrameData.mnCloseState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_ROLL )
            {
                maFrameData.mnRollState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_DOCK )
            {
                maFrameData.mnDockState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_MENU )
            {
                maFrameData.mnMenuState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // call handler already on mouse down
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                    pClientWindow->TitleButtonClick( TitleButton::Menu );
                }
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_HIDE )
            {
                maFrameData.mnHideState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_HELP )
            {
                maFrameData.mnHelpState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_PIN )
            {
                maFrameData.mnPinState |= DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();
            }
            else
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    if ( bTracking )
                    {
                        Point   aPos         = pBorderWindow->GetPosPixel();
                        Size    aSize        = pBorderWindow->GetOutputSizePixel();
                        maFrameData.mnTrackX      = aPos.X();
                        maFrameData.mnTrackY      = aPos.Y();
                        maFrameData.mnTrackWidth  = aSize.Width();
                        maFrameData.mnTrackHeight = aSize.Height();

                        if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_TITLE )
                            nDragFullTest = DragFullOptions::WindowMove;
                        else
                            nDragFullTest = DragFullOptions::WindowSize;
                    }
                }
                else
                {
                    bTracking = false;

                    if ( (maFrameData.mnHitTest & BORDERWINDOW_DRAW_TITLE) &&
                         ((rMEvt.GetClicks() % 2) == 0) )
                    {
                        maFrameData.mnHitTest = 0;
                        bHitTest = false;

                        if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                        {
                            SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                            if ( true /*pBorderWindow->mbDockBtn*/ )   // always perform docking on double click, no button required
                                pClientWindow->TitleButtonClick( TitleButton::Docking );
                            else if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
                            {
                                if ( pClientWindow->IsRollUp() )
                                    pClientWindow->RollDown();
                                else
                                    pClientWindow->RollUp();
                                pClientWindow->Roll();
                            }
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
                maFrameData.mnHitTest = 0;
        }
    }

    return true;
}

bool ImplStdBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    ImplBorderWindow* pBorderWindow = maFrameData.mpBorderWindow;

    if ( rTEvt.IsTrackingEnded() )
    {
        sal_uInt16 nHitTest = maFrameData.mnHitTest;
        maFrameData.mnHitTest = 0;

        if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
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
                    vcl::Window *pWin = pBorderWindow->ImplGetClientWindow()->ImplGetWindow();
                    SystemWindow  *pSysWin  = dynamic_cast<SystemWindow* >(pWin);
                    DockingWindow *pDockWin = dynamic_cast<DockingWindow*>(pWin);
                    if ( pSysWin )
                        pSysWin->Close();
                    else if ( pDockWin )
                        pDockWin->Close();
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( maFrameData.mnRollState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnRollState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        if ( pClientWindow->IsRollUp() )
                            pClientWindow->RollDown();
                        else
                            pClientWindow->RollUp();
                        pClientWindow->Roll();
                    }
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_DOCK )
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
        else if ( nHitTest & BORDERWINDOW_HITTEST_MENU )
        {
            if ( maFrameData.mnMenuState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnMenuState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // handler already called on mouse down
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HIDE )
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
        else if ( nHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            if ( maFrameData.mnHelpState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnHelpState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            if ( maFrameData.mnPinState & DrawButtonFlags::Pressed )
            {
                maFrameData.mnPinState &= ~DrawButtonFlags::Pressed;
                pBorderWindow->InvalidateBorder();

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->SetPin( !pClientWindow->IsPinned() );
                        pClientWindow->Pin();
                    }
                }
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

        if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            if ( maFrameData.maCloseRect.IsInside( aMousePos ) )
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
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( maFrameData.maRollRect.IsInside( aMousePos ) )
            {
                if ( !(maFrameData.mnRollState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnRollState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnRollState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnRollState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_DOCK )
        {
            if ( maFrameData.maDockRect.IsInside( aMousePos ) )
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
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_MENU )
        {
            if ( maFrameData.maMenuRect.IsInside( aMousePos ) )
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
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            if ( maFrameData.maHideRect.IsInside( aMousePos ) )
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
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            if ( maFrameData.maHelpRect.IsInside( aMousePos ) )
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
        else if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            if ( maFrameData.maPinRect.IsInside( aMousePos ) )
            {
                if ( !(maFrameData.mnPinState & DrawButtonFlags::Pressed) )
                {
                    maFrameData.mnPinState |= DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
            else
            {
                if ( maFrameData.mnPinState & DrawButtonFlags::Pressed )
                {
                    maFrameData.mnPinState &= ~DrawButtonFlags::Pressed;
                    pBorderWindow->InvalidateBorder();
                }
            }
        }
        else
        {
            aMousePos.X()    -= maFrameData.maMouseOff.X();
            aMousePos.Y()    -= maFrameData.maMouseOff.Y();

            if ( maFrameData.mnHitTest & BORDERWINDOW_HITTEST_TITLE )
            {
                maFrameData.mpBorderWindow->SetPointer( Pointer( PointerStyle::Move ) );

                Point aPos = pBorderWindow->GetPosPixel();
                aPos.X() += aMousePos.X();
                aPos.Y() += aMousePos.Y();
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
                    pBorderWindow->ShowTracking( Rectangle( pBorderWindow->ScreenToOutputPixel( aPos ), pBorderWindow->GetOutputSizePixel() ), SHOWTRACK_BIG );
                }
            }
            else
            {
                Point       aOldPos         = pBorderWindow->GetPosPixel();
                Size        aSize           = pBorderWindow->GetSizePixel();
                Rectangle   aNewRect( aOldPos, aSize );
                long        nOldWidth       = aSize.Width();
                long        nOldHeight      = aSize.Height();
                long        nBorderWidth    = maFrameData.mnLeftBorder+maFrameData.mnRightBorder;
                long        nBorderHeight   = maFrameData.mnTopBorder+maFrameData.mnBottomBorder;
                long        nMinWidth       = pBorderWindow->mnMinWidth+nBorderWidth;
                long        nMinHeight      = pBorderWindow->mnMinHeight+nBorderHeight;
                long        nMinWidth2      = nBorderWidth;
                long        nMaxWidth       = pBorderWindow->mnMaxWidth+nBorderWidth;
                long        nMaxHeight      = pBorderWindow->mnMaxHeight+nBorderHeight;

                if ( maFrameData.mnTitleHeight )
                {
                    nMinWidth2 += 4;

                    if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
                        nMinWidth2 += maFrameData.maCloseRect.GetWidth();
                }
                if ( nMinWidth2 > nMinWidth )
                    nMinWidth = nMinWidth2;
                if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_LEFT | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_BOTTOMLEFT) )
                {
                    aNewRect.Left() += aMousePos.X();
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.Left() = aNewRect.Right()-nMinWidth+1;
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.Left() = aNewRect.Right()-nMaxWidth+1;
                }
                else if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_RIGHT | BORDERWINDOW_HITTEST_TOPRIGHT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Right() += aMousePos.X();
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.Right() = aNewRect.Left()+nMinWidth+1;
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.Right() = aNewRect.Left()+nMaxWidth+1;
                }
                if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_TOP | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_TOPRIGHT) )
                {
                    aNewRect.Top() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Top() = aNewRect.Bottom()-nMinHeight+1;
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.Top() = aNewRect.Bottom()-nMaxHeight+1;
                }
                else if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_BOTTOM | BORDERWINDOW_HITTEST_BOTTOMLEFT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Bottom() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Bottom() = aNewRect.Top()+nMinHeight+1;
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.Bottom() = aNewRect.Top()+nMaxHeight+1;
                }

                // call Resizing-Handler for SystemWindows
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    // adjust size for Resizing-call
                    aSize = aNewRect.GetSize();
                    aSize.Width()   -= nBorderWidth;
                    aSize.Height()  -= nBorderHeight;
                    static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow())->Resizing( aSize );
                    aSize.Width()   += nBorderWidth;
                    aSize.Height()  += nBorderHeight;
                    if ( aSize.Width() < nMinWidth )
                        aSize.Width() = nMinWidth;
                    if ( aSize.Height() < nMinHeight )
                        aSize.Height() = nMinHeight;
                    if ( aSize.Width() > nMaxWidth )
                        aSize.Width() = nMaxWidth;
                    if ( aSize.Height() > nMaxHeight )
                        aSize.Height() = nMaxHeight;
                    if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_LEFT | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_BOTTOMLEFT) )
                        aNewRect.Left() = aNewRect.Right()-aSize.Width()+1;
                    else
                        aNewRect.Right() = aNewRect.Left()+aSize.Width()-1;
                    if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_TOP | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_TOPRIGHT) )
                        aNewRect.Top() = aNewRect.Bottom()-aSize.Height()+1;
                    else
                        aNewRect.Bottom() = aNewRect.Top()+aSize.Height()-1;
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
                    if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_RIGHT | BORDERWINDOW_HITTEST_TOPRIGHT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                        maFrameData.maMouseOff.X() += aNewRect.GetWidth()-nOldWidth;
                    if ( maFrameData.mnHitTest & (BORDERWINDOW_HITTEST_BOTTOM | BORDERWINDOW_HITTEST_BOTTOMLEFT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                        maFrameData.maMouseOff.Y() += aNewRect.GetHeight()-nOldHeight;
                }
                else
                {
                    maFrameData.mnTrackX        = aNewRect.Left();
                    maFrameData.mnTrackY        = aNewRect.Top();
                    maFrameData.mnTrackWidth    = aNewRect.GetWidth();
                    maFrameData.mnTrackHeight   = aNewRect.GetHeight();
                    pBorderWindow->ShowTracking( Rectangle( pBorderWindow->ScreenToOutputPixel( aNewRect.TopLeft() ), aNewRect.GetSize() ), SHOWTRACK_BIG );
                }
            }
        }
    }

    return true;
}

OUString ImplStdBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

Rectangle ImplStdBorderWindowView::GetMenuRect() const
{
    return maFrameData.maMenuRect;
}

void ImplStdBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    ImplBorderFrameData*    pData = &maFrameData;
    ImplBorderWindow*       pBorderWindow = maFrameData.mpBorderWindow;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    Rectangle               aRect( 0, 0, 10, 10 );
    Rectangle               aCalcRect = aDecoView.DrawFrame( aRect, DrawFrameStyle::DoubleOut, DrawFrameFlags::NoDraw );

    pData->mpOutDev         = pDev;
    pData->mnWidth          = nWidth;
    pData->mnHeight         = nHeight;

    pData->mnTitleType      = pBorderWindow->mnTitleType;
    pData->mbFloatWindow    = pBorderWindow->mbFloatWindow;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) || (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
        pData->mnBorderSize = 0;
    else if ( pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF )
        pData->mnBorderSize = 0;
    else
        pData->mnBorderSize = rStyleSettings.GetBorderSize();
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
        if( pData->mnTitleHeight < MIN_CAPTION_HEIGHT )
            pData->mnTitleHeight = MIN_CAPTION_HEIGHT;

        // set a proper background for drawing
        // highlighted buttons in the title
        pBorderWindow->SetBackground( rStyleSettings.GetFaceColor() );

        pData->maTitleRect.Left()    = pData->mnLeftBorder;
        pData->maTitleRect.Right()   = nWidth-pData->mnRightBorder-1;
        pData->maTitleRect.Top()     = pData->mnTopBorder;
        pData->maTitleRect.Bottom()  = pData->maTitleRect.Top()+pData->mnTitleHeight-1;

        if ( pData->mnTitleType & (BORDERWINDOW_TITLE_NORMAL | BORDERWINDOW_TITLE_SMALL) )
        {
            long nLeft          = pData->maTitleRect.Left() + 1;
            long nRight         = pData->maTitleRect.Right() - 3;
            long const nItemTop = pData->maTitleRect.Top() + 2;
            long const nItemBottom = pData->maTitleRect.Bottom() - 2;

            auto addOnLeft = [&nLeft, nItemTop, nItemBottom](
                Rectangle & rect, long width, long gap)
            {
                rect.Top() = nItemTop;
                rect.Bottom() = nItemBottom;
                rect.Left() = nLeft;
                rect.Right() = rect.Left() + width;
                nLeft += rect.GetWidth() + gap;
            };
            auto addSquareOnRight = [&nRight, nItemTop, nItemBottom](
                Rectangle & rect, long gap)
            {
                rect.Top() = nItemTop;
                rect.Bottom() = nItemBottom;
                rect.Right() = nRight;
                rect.Left() = rect.Right() - rect.GetHeight() + 1;
                nRight -= rect.GetWidth() + gap;
            };

            if ( pBorderWindow->GetStyle() & WB_PINABLE )
            {
                Image aImage;
                ImplGetPinImage( DrawButtonFlags::NONE, false, aImage );
                addOnLeft(pData->maPinRect, aImage.GetSizePixel().Width(), 3);
            }

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

            if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
            {
                addSquareOnRight(pData->maRollRect, 0);
            }
        }
        else
        {
            pData->maPinRect.SetEmpty();
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
            pData->maMenuRect.SetEmpty();
            pData->maHideRect.SetEmpty();
            pData->maRollRect.SetEmpty();
            pData->maHelpRect.SetEmpty();
        }

        pData->mnTopBorder  += pData->mnTitleHeight;
    }
    else
    {
        pData->maTitleRect.SetEmpty();
        pData->maPinRect.SetEmpty();
        pData->maCloseRect.SetEmpty();
        pData->maDockRect.SetEmpty();
        pData->maMenuRect.SetEmpty();
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
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

long ImplStdBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

void ImplStdBorderWindowView::DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point* pOffset)
{
    ImplBorderFrameData* pData = &maFrameData;
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    Point aTmpPoint = pOffset ? Point(*pOffset) : Point();
    Rectangle aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    DecorationView aDecoView(&rRenderContext);
    Color aFaceColor(rStyleSettings.GetFaceColor());
    Color aFrameColor(aFaceColor);

    aFrameColor.DecreaseContrast(sal_uInt8(0.5 * 255));

    // Draw Frame
    if (nDrawFlags & BORDERWINDOW_DRAW_FRAME)
    {
        vcl::Region oldClipRgn(rRenderContext.GetClipRegion());

        // for popups, don't draw part of the frame
        if (pData->mnTitleType == BORDERWINDOW_TITLE_POPUP)
        {
            FloatingWindow* pWin = dynamic_cast<FloatingWindow*>(pData->mpBorderWindow->GetWindow(GetWindowType::Client));
            if (pWin)
            {
                vcl::Region aClipRgn(aInRect);
                Rectangle aItemClipRect(pWin->ImplGetItemEdgeClipRect());
                if (!aItemClipRect.IsEmpty())
                {
                    aItemClipRect.SetPos(pData->mpBorderWindow->AbsoluteScreenToOutputPixel(aItemClipRect.TopLeft()));
                    aClipRgn.Exclude(aItemClipRect);
                    rRenderContext.SetClipRegion(aClipRgn);
                }
            }
        }

        // single line frame
        rRenderContext.SetLineColor(aFrameColor);
        rRenderContext.SetFillColor();
        rRenderContext.DrawRect(aInRect);
        ++aInRect.Left();
        --aInRect.Right();
        ++aInRect.Top();
        --aInRect.Bottom();

        // restore
        if (pData->mnTitleType == BORDERWINDOW_TITLE_POPUP)
            rRenderContext.SetClipRegion(oldClipRgn);
    }
    else
        aInRect = aDecoView.DrawFrame(aInRect, DrawFrameStyle::DoubleOut, DrawFrameFlags::NoDraw);

    // Draw Border
    rRenderContext.SetLineColor();
    long nBorderSize = pData->mnBorderSize;
    if ((nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize)
    {
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        rRenderContext.DrawRect(Rectangle(Point(aInRect.Left(), aInRect.Top()),
                                 Size(aInRect.GetWidth(), nBorderSize)));
        rRenderContext.DrawRect(Rectangle(Point(aInRect.Left(), aInRect.Top() + nBorderSize),
                                 Size(nBorderSize, aInRect.GetHeight() - nBorderSize)));
        rRenderContext.DrawRect(Rectangle(Point(aInRect.Left(), aInRect.Bottom() - nBorderSize + 1),
                                 Size(aInRect.GetWidth(), nBorderSize)));
        rRenderContext.DrawRect(Rectangle(Point(aInRect.Right()-nBorderSize + 1, aInRect.Top() + nBorderSize),
                                 Size(nBorderSize, aInRect.GetHeight() - nBorderSize)));
    }

    // Draw Title
    if ((nDrawFlags & BORDERWINDOW_DRAW_TITLE) && !pData->maTitleRect.IsEmpty())
    {
        aInRect = pData->maTitleRect;

        // use no gradient anymore, just a static titlecolor
        if (pData->mnTitleType != BORDERWINDOW_TITLE_POPUP)
            rRenderContext.SetFillColor(aFrameColor);
        else
            rRenderContext.SetFillColor(aFaceColor);

        rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
        Rectangle aTitleRect(pData->maTitleRect);
        if(pOffset)
            aTitleRect.Move(pOffset->X(), pOffset->Y());
        rRenderContext.DrawRect(aTitleRect);

        if (pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF)
        {
            aInRect.Left()  += 2;
            aInRect.Right() -= 2;

            if (!pData->maPinRect.IsEmpty())
                aInRect.Left() = pData->maPinRect.Right() + 2;

            if (!pData->maHelpRect.IsEmpty())
                aInRect.Right() = pData->maHelpRect.Left() - 2;
            else if (!pData->maRollRect.IsEmpty())
                aInRect.Right() = pData->maRollRect.Left() - 2;
            else if (!pData->maHideRect.IsEmpty())
                aInRect.Right() = pData->maHideRect.Left() - 2;
            else if (!pData->maDockRect.IsEmpty())
                aInRect.Right() = pData->maDockRect.Left() - 2;
            else if (!pData->maMenuRect.IsEmpty())
                aInRect.Right() = pData->maMenuRect.Left() - 2;
            else if (!pData->maCloseRect.IsEmpty())
                aInRect.Right() = pData->maCloseRect.Left() - 2;

            if (pOffset)
                aInRect.Move(pOffset->X(), pOffset->Y());

            DrawTextFlags nTextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis | DrawTextFlags::Clip;

            // must show tooltip ?
            TextRectInfo aInfo;
            rRenderContext.GetTextRect(aInRect, pBorderWindow->GetText(), nTextStyle, &aInfo);
            pData->mbTitleClipped = aInfo.IsEllipses();

            rRenderContext.DrawText(aInRect, pBorderWindow->GetText(), nTextStyle);
        }
    }

    if (((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maCloseRect.IsEmpty())
    {
        Rectangle aSymbolRect(pData->maCloseRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::CLOSE, pData->mnCloseState);
    }
    if (((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maDockRect.IsEmpty())
    {
        Rectangle aSymbolRect(pData->maDockRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::DOCK, pData->mnDockState);
    }
    if (((nDrawFlags & BORDERWINDOW_DRAW_MENU) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maMenuRect.IsEmpty())
    {
        Rectangle aSymbolRect(pData->maMenuRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::MENU, pData->mnMenuState);
    }
    if (((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maHideRect.IsEmpty())
    {
        Rectangle aSymbolRect(pData->maHideRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::HIDE, pData->mnHideState);
    }
    if (((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maRollRect.IsEmpty())
    {
        SymbolType eType;
        if (pBorderWindow->mbRollUp)
            eType = SymbolType::ROLLDOWN;
        else
            eType = SymbolType::ROLLUP;
        Rectangle aSymbolRect(pData->maRollRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, eType, pData->mnRollState);
    }

    if (((nDrawFlags & BORDERWINDOW_DRAW_HELP) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maHelpRect.IsEmpty())
    {
        Rectangle aSymbolRect(pData->maHelpRect);
        if (pOffset)
            aSymbolRect.Move(pOffset->X(), pOffset->Y());
        ImplDrawBrdWinSymbolButton(&rRenderContext, aSymbolRect, SymbolType::HELP, pData->mnHelpState);
    }
    if (((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE))
       && !pData->maPinRect.IsEmpty())
    {
        Image aImage;
        ImplGetPinImage(pData->mnPinState, pBorderWindow->mbPinned, aImage);
        Size  aImageSize = aImage.GetSizePixel();
        long  nRectHeight = pData->maPinRect.GetHeight();
        Point aPos(pData->maPinRect.TopLeft());
        if (pOffset)
            aPos.Move(pOffset->X(), pOffset->Y());
        if (nRectHeight < aImageSize.Height())
        {
            rRenderContext.DrawImage(aPos, Size( aImageSize.Width(), nRectHeight ), aImage);
        }
        else
        {
            aPos.Y() += (nRectHeight-aImageSize.Height()) / 2;
            rRenderContext.DrawImage(aPos, aImage);
        }
    }
}

void ImplBorderWindow::ImplInit( vcl::Window* pParent,
                                 WinBits nStyle, sal_uInt16 nTypeStyle,
                                 const ::com::sun::star::uno::Any& )
{
    ImplInit( pParent, nStyle, nTypeStyle, nullptr );
}

void ImplBorderWindow::ImplInit( vcl::Window* pParent,
                                 WinBits nStyle, sal_uInt16 nTypeStyle,
                                 SystemParentData* pSystemParentData
                                 )
{
    // remove all unwanted WindowBits
    WinBits nOrgStyle = nStyle;
    WinBits nTestStyle = (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_PINABLE | WB_CLOSEABLE | WB_STANDALONE | WB_DIALOGCONTROL | WB_NODIALOGCONTROL | WB_SYSTEMFLOATWIN | WB_INTROWIN | WB_DEFAULTWIN | WB_TOOLTIPWIN | WB_NOSHADOW | WB_OWNERDRAWDECORATION | WB_SYSTEMCHILDWINDOW  | WB_NEEDSFOCUS | WB_POPUP);
    if ( nTypeStyle & BORDERWINDOW_STYLE_APP )
        nTestStyle |= WB_APP;
    nStyle &= nTestStyle;

    mpWindowImpl->mbBorderWin       = true;
    mbSmallOutBorder    = false;
    if ( nTypeStyle & BORDERWINDOW_STYLE_FRAME )
    {
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder               = false;
        }
        else if( (nStyle & (WB_OWNERDRAWDECORATION | WB_POPUP)) )
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder   = (nOrgStyle & WB_NOBORDER) == 0;
        }
        else
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder   = false;
            // closeable windows may have a border as well, eg. system floating windows without caption
            if ( (nOrgStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE/* | WB_CLOSEABLE*/)) == WB_BORDER )
                mbSmallOutBorder = true;
        }
    }
    else if ( nTypeStyle & BORDERWINDOW_STYLE_OVERLAP )
    {
        mpWindowImpl->mbOverlapWin  = true;
        mbFrameBorder   = true;
    }
    else
        mbFrameBorder   = false;

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
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
    mnRollHeight    = 0;
    mnOrgMenuHeight = 0;
    mbPinned        = false;
    mbRollUp        = false;
    mbMenuHide      = false;
    mbDockBtn       = false;
    mbMenuBtn       = false;
    mbHideBtn       = false;
    mbDisplayActive = IsActive();

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
        mnTitleType = BORDERWINDOW_TITLE_SMALL;
    else
        mnTitleType = BORDERWINDOW_TITLE_NORMAL;
    mnBorderStyle   = WindowBorderStyle::NORMAL;
    InitView();
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent,
                                    SystemParentData* pSystemParentData,
                                    WinBits nStyle, sal_uInt16 nTypeStyle
                                    ) : Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, pSystemParentData );
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent, WinBits nStyle ,
                                    sal_uInt16 nTypeStyle ) :
    Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, ::com::sun::star::uno::Any() );
}

ImplBorderWindow::~ImplBorderWindow()
{
    disposeOnce();
}

void ImplBorderWindow::dispose()
{
    delete mpBorderView;
    mpBorderView = nullptr;
    mpMenuBarWindow.clear();
    mpNotebookBarWindow.disposeAndClear();
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

void ImplBorderWindow::Paint( vcl::RenderContext& rRenderContext, const Rectangle& )
{
    if (mpBorderView)
        mpBorderView->DrawWindow(rRenderContext, BORDERWINDOW_DRAW_ALL);
}

void ImplBorderWindow::Draw( const Rectangle&, OutputDevice* pOutDev, const Point& rPos )
{
    if (mpBorderView)
        mpBorderView->DrawWindow(*pOutDev, BORDERWINDOW_DRAW_ALL, &rPos);
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
    if ( GetActivateMode() != ActivateModeFlags::NONE && !ImplGetSVData()->maWinData.mbNoDeactivate )
        SetDisplayActive( false );
    Window::Deactivate();
}

void ImplBorderWindow::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help for border window
    if ( rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK) && !rHEvt.KeyboardActivated() )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        Rectangle   aHelpRect;
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

    if ( !mbRollUp )
    {
        vcl::Window* pClientWindow = ImplGetClientWindow();

        sal_Int32 nLeftBorder;
        sal_Int32 nTopBorder;
        sal_Int32 nRightBorder;
        sal_Int32 nBottomBorder;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );

        if (mpMenuBarWindow)
        {
            long nMenuHeight = mpMenuBarWindow->GetSizePixel().Height();
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
                    aSize.Width()-nLeftBorder-nRightBorder, nMenuHeight);

            // shift the notebookbar down accordingly
            nTopBorder += nMenuHeight;
        }

        if (mpNotebookBarWindow)
        {
            long nNotebookBarHeight = mpNotebookBarWindow->GetSizePixel().Height();
            mpNotebookBarWindow->setPosSizePixel(
                    nLeftBorder, nTopBorder,
                    aSize.Width() - nLeftBorder - nRightBorder, nNotebookBarHeight);
        }

        GetBorder( pClientWindow->mpWindowImpl->mnLeftBorder, pClientWindow->mpWindowImpl->mnTopBorder,
                   pClientWindow->mpWindowImpl->mnRightBorder, pClientWindow->mpWindowImpl->mnBottomBorder );
        pClientWindow->ImplPosSizeWindow( pClientWindow->mpWindowImpl->mnLeftBorder,
                                          pClientWindow->mpWindowImpl->mnTopBorder,
                                          aSize.Width()-pClientWindow->mpWindowImpl->mnLeftBorder-pClientWindow->mpWindowImpl->mnRightBorder,
                                          aSize.Height()-pClientWindow->mpWindowImpl->mnTopBorder-pClientWindow->mpWindowImpl->mnBottomBorder,
                                          PosSizeFlags::X | PosSizeFlags::Y |
                                          PosSizeFlags::Width | PosSizeFlags::Height );
    }

    // UpdateView
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();

    Window::Resize();
}

void ImplBorderWindow::StateChanged( StateChangedType nType )
{
    if ( (nType == StateChangedType::Text) ||
         (nType == StateChangedType::Image) ||
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
        mpBorderView = new ImplSmallBorderWindowView( this );
    else if ( mpWindowImpl->mbFrame )
    {
        if( mbFrameBorder )
            mpBorderView = new ImplStdBorderWindowView( this );
        else
            mpBorderView = new ImplNoBorderWindowView( this );
    }
    else if ( !mbFrameBorder )
        mpBorderView = new ImplSmallBorderWindowView( this );
    else
        mpBorderView = new ImplStdBorderWindowView( this );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
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
        delete mpBorderView;
        InitView();
    }
    else
    {
        Size aSize = aOutputSize;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        aSize.Width()  += nLeftBorder+nRightBorder;
        aSize.Height() += nTopBorder+nBottomBorder;
        mpBorderView->Init( this, aSize.Width(), aSize.Height() );
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
        aOutputSize.Width()     += nLeftBorder+nRightBorder;
        aOutputSize.Height()    += nTopBorder+nBottomBorder;
        if ( aOutputSize == GetSizePixel() )
            InvalidateBorder();
        else
            SetSizePixel( aOutputSize );
    }
}

void ImplBorderWindow::InvalidateBorder()
{
    if ( IsReallyVisible() )
    {
        // invalidate only if we have a border
        sal_Int32 nLeftBorder;
        sal_Int32 nTopBorder;
        sal_Int32 nRightBorder;
        sal_Int32 nBottomBorder;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        if ( nLeftBorder || nTopBorder || nRightBorder || nBottomBorder )
        {
            Rectangle   aWinRect( Point( 0, 0 ), GetOutputSizePixel() );
            vcl::Region      aRegion( aWinRect );
            aWinRect.Left()   += nLeftBorder;
            aWinRect.Top()    += nTopBorder;
            aWinRect.Right()  -= nRightBorder;
            aWinRect.Bottom() -= nBottomBorder;
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

void ImplBorderWindow::SetTitleType( sal_uInt16 nTitleType, const Size& rSize )
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

void ImplBorderWindow::SetPin( bool bPin )
{
    mbPinned = bPin;
    InvalidateBorder();
}

void ImplBorderWindow::SetRollUp( bool bRollUp, const Size& rSize )
{
    mbRollUp = bRollUp;
    mnRollHeight = rSize.Height();
    UpdateView( false, rSize );
}

void ImplBorderWindow::SetCloseButton()
{
    SetStyle( GetStyle() | WB_CLOSEABLE );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetDockButton( bool bDockButton )
{
    mbDockBtn = bDockButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetHideButton( bool bHideButton )
{
    mbHideBtn = bHideButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetMenuButton( bool bMenuButton )
{
    mbMenuBtn = bMenuButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
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

void ImplBorderWindow::SetNotebookBarWindow(const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame>& rFrame)
{
    mpNotebookBarWindow = VclPtr<NotebookBarWindow>::Create(this, "NotebookBar", rUIXMLDescription, rFrame);
    Resize();
    mpNotebookBarWindow->Show();
}

void ImplBorderWindow::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                  sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    mpBorderView->GetBorder(rLeftBorder, rTopBorder, rRightBorder, rBottomBorder);

    if (mpMenuBarWindow && !mbMenuHide)
        rTopBorder += mpMenuBarWindow->GetSizePixel().Height();

    if (mpNotebookBarWindow)
        rTopBorder += mpNotebookBarWindow->GetSizePixel().Height();
}

long ImplBorderWindow::CalcTitleWidth() const
{
    return mpBorderView->CalcTitleWidth();
}

Rectangle ImplBorderWindow::GetMenuRect() const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
