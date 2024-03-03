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

#include <vcl/event.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolkit/scrbar.hxx>
#include <svdata.hxx>

void TabPage::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Window::ImplInit( pParent, nStyle, nullptr );

    bool bHasHoriBar = false;
    bool bHasVertBar = false;

    Link<ScrollBar*,void> aLink( LINK( this, TabPage, ScrollBarHdl ) );

    if ( nStyle & ( WB_AUTOHSCROLL | WB_AUTOVSCROLL ) )
    {
        if ( nStyle & WB_AUTOHSCROLL )
        {
            bHasHoriBar = true;
            m_pHScroll.set(VclPtr<ScrollBar>::Create(this, (WB_HSCROLL | WB_DRAG)));
            m_pHScroll->Show();
            m_pHScroll->SetScrollHdl(aLink);
        }
        if ( nStyle &  WB_AUTOVSCROLL )
        {
            bHasVertBar = true;
            m_pVScroll.set(VclPtr<ScrollBar>::Create(this, (WB_VSCROLL | WB_DRAG)));
            m_pVScroll->Show();
            m_pVScroll->SetScrollHdl(aLink);
        }
    }

    if ( bHasHoriBar || bHasVertBar )
    {
        SetStyle( GetStyle() | WB_CLIPCHILDREN );
    }

    mnScrWidth = Application::GetSettings().GetStyleSettings().GetScrollBarSize();

    ImplInitSettings();

    // if the tabpage is drawn (ie filled) by a native widget, make sure all controls will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL) )
        EnableChildTransparentMode();
}

void TabPage::ImplInitSettings()
{
    vcl::Window* pParent = GetParent();
    if (pParent && pParent->IsChildTransparentModeEnabled() && !IsControlBackground())
    {
        EnableChildTransparentMode();
        SetParentClipMode( ParentClipMode::NoClip );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode();
        SetPaintTransparent( false );

        if (IsControlBackground() || !pParent)
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

TabPage::TabPage( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::TABPAGE )
{
    ImplInit( pParent, nStyle );
}

TabPage::~TabPage()
{
    disposeOnce();
}

void TabPage::dispose()
{
    m_pVScroll.disposeAndClear();
    m_pHScroll.disposeAndClear();
    vcl::Window::dispose();
}

void TabPage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
    {
        if (GetSettings().GetStyleSettings().GetAutoMnemonic())
            GenerateAutoMnemonicsOnHierarchy(this);
        // FIXME: no layouting, workaround some clipping issues
        ImplAdjustNWFSizes();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void TabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void TabPage::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    // draw native tabpage only inside tabcontrols, standalone tabpages look ugly (due to bad dialog design)
    if( !(IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL)) )
        return;

    const ImplControlValue aControlValue;

    ControlState nState = ControlState::ENABLED;
    if ( !IsEnabled() )
        nState &= ~ControlState::ENABLED;
    if ( HasFocus() )
        nState |= ControlState::FOCUSED;
    // pass the whole window region to NWF as the tab body might be a gradient or bitmap
    // that has to be scaled properly, clipping makes sure that we do not paint too much
    tools::Rectangle aCtrlRegion( Point(), GetOutputSizePixel() );
    rRenderContext.DrawNativeControl( ControlType::TabBody, ControlPart::Entire, aCtrlRegion, nState,
            aControlValue, OUString() );
}

void TabPage::Draw( OutputDevice* pDev, const Point& rPos, SystemTextColorFlags )
{
    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = GetSizePixel();

    Wallpaper aWallpaper = GetBackground();
    if ( !aWallpaper.IsBitmap() )
        ImplInitSettings();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetLineColor();

    if ( aWallpaper.IsBitmap() )
        pDev->DrawBitmapEx( aPos, aSize, aWallpaper.GetBitmap() );
    else
    {
        if( aWallpaper.GetColor() == COL_AUTO )
            pDev->SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
        else
            pDev->SetFillColor( aWallpaper.GetColor() );
        pDev->DrawRect( tools::Rectangle( aPos, aSize ) );
    }

    pDev->Pop();
}

Size TabPage::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));
    return getLegacyBestSizeForChildren(*this);
}

void TabPage::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    Window::SetPosSizePixel(rAllocPos, rAllocation);
    if (isLayoutEnabled(this) && rAllocation.Width() && rAllocation.Height())
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), rAllocation);
}

void TabPage::SetSizePixel(const Size& rAllocation)
{
    Window::SetSizePixel(rAllocation);
    if (isLayoutEnabled(this) && rAllocation.Width() && rAllocation.Height())
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), rAllocation);
}

void TabPage::SetPosPixel(const Point& rAllocPos)
{
    Window::SetPosPixel(rAllocPos);
    Size aAllocation(GetOutputSizePixel());
    if (isLayoutEnabled(this) && aAllocation.Width() && aAllocation.Height())
    {
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), aAllocation);
    }
}

void TabPage::lcl_Scroll( tools::Long nX, tools::Long nY )
{
    tools::Long nXScroll = mnScrollPos.X() - nX;
    tools::Long nYScroll = mnScrollPos.Y() - nY;
    mnScrollPos = Point( nX, nY );

    tools::Rectangle aScrollableArea( 0, 0, maScrollArea.Width(), maScrollArea.Height() );
    Scroll(nXScroll, nYScroll, aScrollableArea );
    // Manually scroll all children ( except the scrollbars )
    for ( int index = 0; index < GetChildCount(); ++index )
    {
        vcl::Window* pChild = GetChild( index );
        if ( pChild && pChild != m_pVScroll.get() && pChild != m_pHScroll.get() )
        {
            Point aPos = pChild->GetPosPixel();
            aPos += Point( nXScroll, nYScroll );
            pChild->SetPosPixel( aPos );
        }
    }
}

IMPL_LINK( TabPage, ScrollBarHdl, ScrollBar*, pSB, void )
{
    sal_uInt16 nPos = static_cast<sal_uInt16>(pSB->GetThumbPos());
    if( pSB == m_pVScroll.get() )
        lcl_Scroll(mnScrollPos.X(), nPos );
    else if( pSB == m_pHScroll.get() )
        lcl_Scroll(nPos, mnScrollPos.Y() );
}

void TabPage::SetScrollTop( tools::Long nTop )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() , mnScrollPos.Y() - nTop );
    if( m_pHScroll )
        m_pHScroll->SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}
void TabPage::SetScrollLeft( tools::Long nLeft )
{
    Point aOld = mnScrollPos;
    lcl_Scroll( mnScrollPos.X() - nLeft , mnScrollPos.Y() );
    if( m_pVScroll )
        m_pVScroll->SetThumbPos( 0 );
    // new pos is 0,0
    mnScrollPos = aOld;
}

void TabPage::SetScrollWidth( tools::Long nWidth )
{
    maScrollArea.setWidth( nWidth );
    ResetScrollBars();
}

void TabPage::SetScrollHeight( tools::Long nHeight )
{
    maScrollArea.setHeight( nHeight );
    ResetScrollBars();
}

void TabPage::Resize()
{
    ResetScrollBars();
}

void TabPage::ResetScrollBars()
{
    Size aOutSz = GetOutputSizePixel();

    Point aVPos( aOutSz.Width() - mnScrWidth, 0 );
    Point aHPos( 0, aOutSz.Height() - mnScrWidth );

    if( m_pVScroll )
    {
        m_pVScroll->SetPosSizePixel( aVPos, Size( mnScrWidth,  GetSizePixel().Height() - mnScrWidth ) );
        m_pVScroll->SetRangeMax( maScrollArea.Height() + mnScrWidth );
        m_pVScroll->SetVisibleSize( GetSizePixel().Height() );
    }

    if( m_pHScroll )
    {
        m_pHScroll->SetPosSizePixel( aHPos, Size(  GetSizePixel().Width() - mnScrWidth, mnScrWidth ) );
        m_pHScroll->SetRangeMax( maScrollArea.Width() + mnScrWidth  );
        m_pHScroll->SetVisibleSize( GetSizePixel().Width() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
