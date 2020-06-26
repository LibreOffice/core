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

#include <vcl/accel.hxx>
#include <vcl/event.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>

void TabPage::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Window::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();

    // if the tabpage is drawn (ie filled) by a native widget, make sure all controls will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL) )
        EnableChildTransparentMode();

    InitFromStyle(nStyle);
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
    , IContext()
    , mpHScrollBar(VclPtr<ScrollBar>::Create(this, WB_HSCROLL | WB_DRAG))
    , mpVScrollBar(VclPtr<ScrollBar>::Create(this, WB_VSCROLL | WB_DRAG))
    , mpScrollBox(VclPtr<ScrollBarBox>::Create(this, WB_SIZEABLE))
{
    ImplInit( pParent, nStyle | WB_AUTOHSCROLL | WB_AUTOVSCROLL);
    mpVScrollBar->SetScrollHdl(LINK(this, TabPage, ScrollHdl));
    mpHScrollBar->SetScrollHdl(LINK(this, TabPage, ScrollHdl));
}

TabPage::TabPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription)
    : Window(WindowType::TABPAGE)
    , IContext()
{
    ImplInit(pParent, 0);
    m_pUIBuilder.reset( new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID) );
    set_hexpand(true);
    set_vexpand(true);
    set_expand(true);
}

TabPage::~TabPage()
{
    disposeOnce();
    mpHScrollBar.disposeAndClear();
    mpVScrollBar.disposeAndClear();
    mpScrollBox.disposeAndClear();
}

void TabPage::dispose()
{
    disposeBuilder();
    vcl::Window::dispose();
}

void TabPage::InitFromStyle(WinBits nWinStyle)
{
    ImpUpdateSrollBarVis(nWinStyle);
}

void TabPage::ImpUpdateSrollBarVis(WinBits nWinStyle)
{
    const bool bHaveVScroll = mpVScrollBar->IsVisible();
    const bool bHaveHScroll = mpHScrollBar->IsVisible();
    const bool bHaveScrollBox = mpScrollBox->IsVisible();

    bool bNeedVScroll = (nWinStyle & WB_VSCROLL) == WB_VSCROLL;
    const bool bNeedHScroll = (nWinStyle & WB_HSCROLL) == WB_HSCROLL;

    const bool bAutoVScroll = (nWinStyle & WB_AUTOVSCROLL) == WB_AUTOVSCROLL;

    const bool bNeedScrollBox = bNeedVScroll && bNeedHScroll;

    bool bScrollbarsChanged = false;
    if (bHaveVScroll != bNeedVScroll)
    {
        mpVScrollBar->Show(bNeedVScroll);
        bScrollbarsChanged = true;
    }

    if (bHaveHScroll != bNeedHScroll)
    {
        mpHScrollBar->Show(bNeedHScroll);
        bScrollbarsChanged = true;
    }

    if (bHaveScrollBox != bNeedScrollBox)
    {
        mpScrollBox->Show(bNeedScrollBox);
    }

    if (bScrollbarsChanged)
    {
        ImpInitScrollBars();
        Resize();
    }
}

void TabPage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
    {
        if (GetSettings().GetStyleSettings().GetAutoMnemonic())
            Accelerator::GenerateAutoMnemonicsOnHierarchy(this);
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
    if( IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL) )
    {
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
}

void TabPage::Draw( OutputDevice* pDev, const Point& rPos, DrawFlags )
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

void TabPage::ImpSetScrollBarRanges()
{
    // set range of scroll bar to size of this window plus max extents of children
    Size sz = GetSizePixel();

    Window* win = GetWindow(GetWindowType::FirstChild);
    while (win)
    {
        long testX = win->GetPosPixel().X() + win->GetSizePixel().getWidth();
        long testY = win->GetPosPixel().Y() + win->GetSizePixel().getHeight();
        if (testX > sz.getWidth())
            sz.setWidth(testX);
        if (testY > sz.getHeight())
            sz.setWidth(testY);
        win = win->GetWindow(GetWindowType::Next);
    }

    mpVScrollBar->SetRange(Range(0, sz.getHeight() - 1));
    mpHScrollBar->SetRange(Range(0, sz.getWidth() - 1));
}

void TabPage::ImpInitScrollBars()
{
    ImpSetScrollBarRanges();

    Size aOutSz = this->GetSizePixel();

    mpHScrollBar->SetVisibleSize(aOutSz.Width());
    mpHScrollBar->SetPageSize(aOutSz.Width() * 8 / 10);
    mpHScrollBar->SetLineSize(aOutSz.Width() * 8 / 100); // 10 "lines" per "page"?
    ImpSetHScrollBarThumbPos();

    mpVScrollBar->SetVisibleSize(aOutSz.Height());
    mpVScrollBar->SetPageSize(aOutSz.Height() * 8 / 10);
    mpVScrollBar->SetLineSize(aOutSz.Height() * 8 / 100); // 10 "lines" per "page"?

    // set vertical thumb pos based on reference point?
    mpVScrollBar->SetThumbPos(0);
}

void TabPage::ImpSetHScrollBarThumbPos()
{
    //long nX = mpTextWindow->GetTextView()->GetStartDocPos().X();
    //if (!mpTextWindow->GetTextEngine()->IsRightToLeft())
    //    mpHScrollBar->SetThumbPos(nX);
    //else
    //    mpHScrollBar->SetThumbPos(mnTextWidth - mpHScrollBar->GetVisibleSize() - nX);

    // set horizontal thumb pos based on reference point (visible top left corner)?
    mpHScrollBar->SetThumbPos(0);
}

// for multiline edit, this scrolls the text view by the difference of current text view and scrollbar position
// for tab pages, something like StartDocPos should be introduced, position of left top visible corner
// 'Scroll' then means changing positions of child controls?
IMPL_LINK(TabPage, ScrollHdl, ScrollBar*, pCurScrollBar, void)
{
    pCurScrollBar;
    //long nDiffX = 0, nDiffY = 0;

    //if (pCurScrollBar == mpVScrollBar)
    //    nDiffY = mpTextWindow->GetTextView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
    //else if (pCurScrollBar == mpHScrollBar)
    //    nDiffX = mpTextWindow->GetTextView()->GetStartDocPos().X() - pCurScrollBar->GetThumbPos();

    //mpTextWindow->GetTextView()->Scroll(nDiffX, nDiffY);
    //// mpTextWindow->GetTextView()->ShowCursor( false, true );
}

Size TabPage::CalcMinimumSize() const
{
    Size aSz;
    // reference size is max(position+size) of all child controls?
    //Size aSz(mpTextWindow->GetTextEngine()->CalcTextWidth(),
    //         mpTextWindow->GetTextEngine()->GetTextHeight());

    if (mpHScrollBar->IsVisible())
        aSz.AdjustHeight(mpHScrollBar->GetSizePixel().Height());
    if (mpVScrollBar->IsVisible())
        aSz.AdjustWidth(mpVScrollBar->GetSizePixel().Width());

    return aSz;
}

void TabPage::Resize()
{
    WinBits nWinStyle(GetStyle());
    if ((nWinStyle & WB_AUTOVSCROLL) == WB_AUTOVSCROLL)
        ImpUpdateSrollBarVis(nWinStyle);

    Size aSz = GetOutputSizePixel();
    Size aEditSize = aSz;
    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom(nSBWidth);

    if (mpHScrollBar->IsVisible())
        aSz.AdjustHeight(-(nSBWidth + 1));
    if (mpVScrollBar->IsVisible())
        aSz.AdjustWidth(-(nSBWidth + 1));

    if (mpHScrollBar->IsVisible())
        mpHScrollBar->setPosSizePixel(0, aEditSize.Height() - nSBWidth, aSz.Width(), nSBWidth);

    Point aTextWindowPos;
    if (mpVScrollBar->IsVisible())
    {
        if (AllSettings::GetLayoutRTL())
        {
            mpVScrollBar->setPosSizePixel(0, 0, nSBWidth, aSz.Height());
            aTextWindowPos.AdjustX(nSBWidth);
        }
        else
            mpVScrollBar->setPosSizePixel(aEditSize.Width() - nSBWidth, 0, nSBWidth,
                                            aSz.Height());
    }

    if (mpScrollBox->IsVisible())
        mpScrollBox->setPosSizePixel(aSz.Width(), aSz.Height(), nSBWidth, nSBWidth);

    Size aTextWindowSize(aSz);
    if (aTextWindowSize.Width() < 0)
        aTextWindowSize.setWidth(0);
    if (aTextWindowSize.Height() < 0)
        aTextWindowSize.setHeight(0);

    Size aOldTextWindowSize(GetSizePixel());
    SetPosSizePixel(aTextWindowPos, aTextWindowSize);
    if (aOldTextWindowSize == aTextWindowSize)
        return;

    ImpInitScrollBars();
}

void TabPage::Enable(bool bEnable)
{
    if (mpHScrollBar->IsVisible())
        mpHScrollBar->Enable(bEnable);
    if (mpVScrollBar->IsVisible())
        mpVScrollBar->Enable(bEnable);
}

bool TabPage::HandleCommand(const CommandEvent& rCEvt)
{
    bool bDone = false;
    CommandEventId nCommand = rCEvt.GetCommand();
    if (nCommand == CommandEventId::Wheel || nCommand == CommandEventId::StartAutoScroll
        || nCommand == CommandEventId::AutoScroll || nCommand == CommandEventId::Gesture)
    {
        ScrollBar* pHScrollBar = mpHScrollBar->IsVisible() ? mpHScrollBar.get() : nullptr;
        ScrollBar* pVScrollBar = mpVScrollBar->IsVisible() ? mpVScrollBar.get() : nullptr;
        HandleScrollCommand(rCEvt, pHScrollBar, pVScrollBar);
        bDone = true;
    }
    return bDone;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
