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
#include <svl/lstner.hxx>
#include <vcl/scrbar.hxx>

TabPageContainer::TabPageContainer(TabPage* pParent)
    : Window(pParent, WB_CLIPCHILDREN)
    , mxParent(pParent)
{
    Color aBackgroundColor = GetSettings().GetStyleSettings().GetWorkspaceColor();
    SetBackground(aBackgroundColor);
    pParent->SetBackground(aBackgroundColor);
}

TabPageContainer::~TabPageContainer()
{
    disposeOnce();
}

void TabPageContainer::dispose()
{
    mxParent.clear();
    Window::dispose();
}

Size TabPageContainer::CalcSizeChildren()
{
    long minX = 0, maxX = 0, minY = 0, maxY = 0;
    vcl::Window* win = GetWindow(GetWindowType::FirstChild);
    while (win)
    {
        Point winPos(win->GetPosPixel());
        Size winSize(win->GetOutputSizePixel());
        minX = std::min(minX, winPos.getX());
        minY = std::min(minY, winPos.getY());
        maxX = std::max(maxX, winPos.getX()  + winSize.Width());
        maxY = std::max(maxY, winPos.getY() + winSize.Height());
        win = win->GetWindow(GetWindowType::Next);
    }

    // test
    maxX = 4000;
    maxY = 3000;

    return Size(maxX - minX, maxY - minY);
}

void TabPageContainer::Scroll(long nHorzScroll, long nVertScroll, ScrollFlags nFlags)
{
    mStartPos.AdjustX(nHorzScroll);
    mStartPos.AdjustY(nVertScroll);
    Window::Scroll(nHorzScroll, nVertScroll, nFlags);
}

class ImpVclTabPage : public SfxListener
{
private:
    VclPtr<TabPage>            pVclTabPage;

    VclPtr<TabPageContainer>   mpTabPageContainer;
    VclPtr<ScrollBar>          mpHScrollBar;
    VclPtr<ScrollBar>          mpVScrollBar;
    VclPtr<ScrollBarBox>       mpScrollBox;

protected:
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    void                ImpUpdateSrollBarVis( WinBits nWinStyle );
    void                ImpInitScrollBars();
    void                ImpSetScrollBarRanges();
    DECL_LINK(    ScrollHdl, ScrollBar*, void );

public:
                ImpVclTabPage( TabPage* pVclTabPage, WinBits nWinStyle );
                virtual ~ImpVclTabPage() override;

    void        Resize();
    void        GetFocus();

    bool        HandleCommand( const CommandEvent& rCEvt );

    void        Enable( bool bEnable );

    Size        CalcMinimumSize() const;

    void        InitFromStyle( WinBits nWinStyle );

    TabPageContainer* GetTabPageContainer() { return mpTabPageContainer; }
    ScrollBar&  GetHScrollBar() { return *mpHScrollBar; }
    ScrollBar&  GetVScrollBar() { return *mpVScrollBar; }
};

ImpVclTabPage::ImpVclTabPage( TabPage* pTabPage, WinBits nWinStyle )
    : pVclTabPage(pTabPage)
    , mpTabPageContainer(VclPtr<TabPageContainer>::Create(pVclTabPage))
    , mpHScrollBar(VclPtr<ScrollBar>::Create(pVclTabPage, WB_HSCROLL|WB_DRAG))
    , mpVScrollBar(VclPtr<ScrollBar>::Create(pVclTabPage, WB_VSCROLL|WB_DRAG))
    , mpScrollBox(VclPtr<ScrollBarBox>::Create(pVclTabPage, WB_SIZEABLE))
{
    mpVScrollBar->SetScrollHdl( LINK( this, ImpVclTabPage, ScrollHdl ) );
    mpHScrollBar->SetScrollHdl( LINK( this, ImpVclTabPage, ScrollHdl ) );
    InitFromStyle( nWinStyle );
}

void ImpVclTabPage::ImpUpdateSrollBarVis( WinBits nWinStyle )
{
    const bool bHaveVScroll = mpVScrollBar->IsVisible();
    const bool bHaveHScroll = mpHScrollBar->IsVisible();
    const bool bHaveScrollBox = mpScrollBox->IsVisible();

    bool bNeedVScroll = ( nWinStyle & WB_VSCROLL ) == WB_VSCROLL;
    bool bNeedHScroll = ( nWinStyle & WB_HSCROLL ) == WB_HSCROLL;

    const bool bAutoVScroll = ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL;
    const bool bAutoHScroll = (nWinStyle & WB_AUTOHSCROLL) == WB_AUTOHSCROLL;

    if (!bNeedVScroll && bAutoVScroll && mpTabPageContainer)
    {
        Size aSz(mpTabPageContainer->CalcSizeChildren());

        if (aSz.Height() > mpTabPageContainer->GetOutputSizePixel().Height())
            bNeedVScroll = true;
    }

    if (!bNeedHScroll && bAutoHScroll && mpTabPageContainer)
    {
        Size aSz(mpTabPageContainer->CalcSizeChildren());

        if (aSz.Height() > mpTabPageContainer->GetOutputSizePixel().Height())
            bNeedHScroll = true;
    }

    const bool bNeedScrollBox = bNeedVScroll && bNeedHScroll;

    bool bScrollbarsChanged = false;
    if ( bHaveVScroll != bNeedVScroll )
    {
        mpVScrollBar->Show(bNeedVScroll);
        bScrollbarsChanged = true;
    }

    if ( bHaveHScroll != bNeedHScroll )
    {
        mpHScrollBar->Show(bNeedHScroll);
        bScrollbarsChanged = true;
    }

    if ( bHaveScrollBox != bNeedScrollBox )
    {
        mpScrollBox->Show(bNeedScrollBox);
    }

    if ( bScrollbarsChanged )
    {
        ImpInitScrollBars();
        Resize();
    }
}

void ImpVclTabPage::InitFromStyle( WinBits nWinStyle )
{
    ImpUpdateSrollBarVis( nWinStyle );
}

ImpVclTabPage::~ImpVclTabPage()
{
    //EndListening( *mpTabPageContainer->GetTextEngine() );
    mpTabPageContainer.disposeAndClear();
    mpHScrollBar.disposeAndClear();
    mpVScrollBar.disposeAndClear();
    mpScrollBox.disposeAndClear();
    pVclTabPage.disposeAndClear();
}

void ImpVclTabPage::ImpSetScrollBarRanges()
{
    Size aMinSize (mpTabPageContainer->GetOutputSizePixel());
    Size aContainerSize(mpTabPageContainer->CalcSizeChildren());
    Size aRange(std::max(aMinSize.getWidth(), aContainerSize.getWidth()),
        std::max(aMinSize.getHeight(), aContainerSize.getHeight()));
    mpVScrollBar->SetRange( Range( 0, aRange.Height()-1 ) );
    mpHScrollBar->SetRange( Range( 0, aRange.Width()-1 ) );
}

void ImpVclTabPage::ImpInitScrollBars()
{
    ImpSetScrollBarRanges();

    Size aVisibleSize(mpTabPageContainer->GetOutputSizePixel());

    mpHScrollBar->SetVisibleSize(aVisibleSize.Width());
    mpHScrollBar->SetPageSize(aVisibleSize.Width() * 8 / 10);
    mpHScrollBar->SetLineSize(aVisibleSize.Width() * 2 / 10);
    mpHScrollBar->SetThumbPos(0);

    mpVScrollBar->SetVisibleSize(aVisibleSize.Height());
    mpVScrollBar->SetPageSize(aVisibleSize.Height() * 8 / 10);
    mpVScrollBar->SetLineSize(aVisibleSize.Height() * 2 / 10);
    mpVScrollBar->SetThumbPos(0);
}

IMPL_LINK( ImpVclTabPage, ScrollHdl, ScrollBar*, pCurScrollBar, void )
{
    long nDiffX = 0, nDiffY = 0;

    if ( pCurScrollBar == mpVScrollBar )
        nDiffY = mpTabPageContainer->GetStartPos().Y() - pCurScrollBar->GetThumbPos();
    else if ( pCurScrollBar == mpHScrollBar )
        nDiffX = mpTabPageContainer->GetStartPos().X() - pCurScrollBar->GetThumbPos();

    mpTabPageContainer->Scroll( nDiffX, nDiffY );
}

void ImpVclTabPage::Resize()
{
    WinBits nWinStyle( pVclTabPage->GetStyle() );
    if (((nWinStyle & WB_AUTOHSCROLL) == WB_AUTOHSCROLL)
        || ((nWinStyle & WB_AUTOVSCROLL) == WB_AUTOVSCROLL))
    {
        ImpUpdateSrollBarVis(nWinStyle);
    }

    Size aSizeBrutto = pVclTabPage->GetOutputSizePixel();
    Size aSizeNetto = aSizeBrutto;
    long nSBWidth = pVclTabPage->GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = pVclTabPage->CalcZoom( nSBWidth );

    if (mpHScrollBar->IsVisible())
        aSizeNetto.AdjustHeight( -(nSBWidth+1) );
    if (mpVScrollBar->IsVisible())
        aSizeNetto.AdjustWidth( -(nSBWidth+1) );

    if (mpHScrollBar->IsVisible())
        mpHScrollBar->setPosSizePixel( 0, aSizeNetto.Height(), aSizeNetto.Width(), nSBWidth );

    if (mpVScrollBar->IsVisible())
        mpVScrollBar->setPosSizePixel(aSizeNetto.Width(), 0, nSBWidth, aSizeNetto.Height());

    if (mpScrollBox->IsVisible())
        mpScrollBox->setPosSizePixel(aSizeNetto.Width(), aSizeNetto.Height(), nSBWidth, nSBWidth);

    Size aNewSizeContainer( aSizeNetto );
    if (aNewSizeContainer.Width() < 0)
        aNewSizeContainer.setWidth(0);
    if (aNewSizeContainer.Height() < 0)
        aNewSizeContainer.setHeight(0);

    Size aOldSizeContainer( mpTabPageContainer->GetSizePixel() );
    mpTabPageContainer->SetPosSizePixel( Point(0,0), aNewSizeContainer );
    if (aOldSizeContainer == aNewSizeContainer)
        return;

    ImpInitScrollBars();
}

void ImpVclTabPage::GetFocus()
{
    mpTabPageContainer->GrabFocus();
}

void ImpVclTabPage::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    rHint;

    //const TextHint* pTextHint = dynamic_cast<const TextHint*>(&rHint);
    //if ( !pTextHint )
    //    return;

    //switch (pTextHint->GetId())
    //{
    //    case SfxHintId::TextViewScrolled:
    //        if (mpHScrollBar->IsVisible())
    //            ImpSetHScrollBarThumbPos();
    //        if (mpVScrollBar->IsVisible())
    //            mpVScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().Y() );
    //        break;

    //    case SfxHintId::TextHeightChanged:
    //        if ( mpTextWindow->GetTextView()->GetStartDocPos().Y() )
    //        {
    //            long nOutHeight = mpTextWindow->GetOutputSizePixel().Height();
    //            long nTextHeight = mpTextWindow->GetTextEngine()->GetTextHeight();
    //            if ( nTextHeight < nOutHeight )
    //                mpTextWindow->GetTextView()->Scroll( 0, mpTextWindow->GetTextView()->GetStartDocPos().Y() );
    //        }
    //        ImpSetScrollBarRanges();
    //        break;

    //    case SfxHintId::TextFormatted:
    //        if (mpHScrollBar->IsVisible())
    //        {
    //            const long nWidth = mpTextWindow->GetTextEngine()->CalcTextWidth();
    //            if ( nWidth != mnTextWidth )
    //            {
    //                mnTextWidth = nWidth;
    //                mpHScrollBar->SetRange( Range( 0, mnTextWidth-1 ) );
    //                ImpSetHScrollBarThumbPos();
    //            }
    //        }
    //        break;

    //    case SfxHintId::TextModified:
    //        ImpUpdateSrollBarVis(pVclMultiLineEdit->GetStyle());
    //        pVclMultiLineEdit->Modify();
    //        break;

    //    case SfxHintId::TextViewSelectionChanged:
    //        pVclMultiLineEdit->SelectionChanged();
    //        break;

    //    case SfxHintId::TextViewCaretChanged:
    //        pVclMultiLineEdit->CaretChanged();
    //        break;

    //    default: break;
    //}
}


Size ImpVclTabPage::CalcMinimumSize() const
{
    Size aSz(0, 0); // or query mpTabPageContainer for minimum size?

    if (mpHScrollBar->IsVisible())
        aSz.AdjustHeight(mpHScrollBar->GetSizePixel().Height() );
    if (mpVScrollBar->IsVisible())
        aSz.AdjustWidth(mpVScrollBar->GetSizePixel().Width() );

    return aSz;
}

void ImpVclTabPage::Enable( bool bEnable )
{
    mpTabPageContainer->Enable( bEnable );
    if (mpHScrollBar->IsVisible())
        mpHScrollBar->Enable( bEnable );
    if (mpVScrollBar->IsVisible())
        mpVScrollBar->Enable( bEnable );
}

bool ImpVclTabPage::HandleCommand( const CommandEvent& rCEvt )
{
    bool bDone = false;
    CommandEventId nCommand = rCEvt.GetCommand();
    if (nCommand == CommandEventId::Wheel ||
        nCommand == CommandEventId::StartAutoScroll ||
        nCommand == CommandEventId::AutoScroll ||
        nCommand == CommandEventId::Gesture)
    {
        ScrollBar* pHScrollBar = mpHScrollBar->IsVisible() ? mpHScrollBar.get() : nullptr;
        ScrollBar* pVScrollBar = mpVScrollBar->IsVisible() ? mpVScrollBar.get() : nullptr;
        mpTabPageContainer->HandleScrollCommand(rCEvt, pHScrollBar, pVScrollBar);
        bDone = true;
    }
    return bDone;
}

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
}

void TabPage::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

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

    if (IsPaintTransparent())
    {
        pImpVclTabPage->GetTabPageContainer()->SetPaintTransparent(true);
        pImpVclTabPage->GetTabPageContainer()->SetBackground();
        pImpVclTabPage->GetTabPageContainer()->SetControlBackground();
        SetBackground();
        SetControlBackground();
    }
    else
    {
        if (IsControlBackground())
            pImpVclTabPage->GetTabPageContainer()->SetBackground(GetControlBackground());
        else
            pImpVclTabPage->GetTabPageContainer()->SetBackground(rStyleSettings.GetFieldColor());
        // also adjust for VclTabPage as the Container might hide Scrollbars
        SetBackground(pImpVclTabPage->GetTabPageContainer()->GetBackground());
    }
}

TabPage::TabPage( vcl::Window* pParent, WinBits nStyle ) :
    Window( pParent, nStyle )
    , IContext()
{
    SetType(WindowType::TABPAGE);

    pImpVclTabPage.reset(new ImpVclTabPage(this, nStyle));
    ImplInitSettings();

    SetCompoundControl(true);
    SetStyle(ImplInitStyle(nStyle));
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
}

void TabPage::dispose()
{
    pImpVclTabPage.reset();
    disposeBuilder();
    vcl::Window::dispose();
}

WinBits TabPage::ImplInitStyle(WinBits nStyle)
{
    if (!(nStyle & WB_NOTABSTOP))
        nStyle |= WB_TABSTOP;

    if (!(nStyle & WB_NOGROUP))
        nStyle |= WB_GROUP;

    if (!(nStyle & WB_IGNORETAB))
        nStyle |= WB_NODIALOGCONTROL;

    return nStyle;
}

void TabPage::Resize()
{
    pImpVclTabPage->Resize();
}

void TabPage::GetFocus()
{
    if (!pImpVclTabPage) // might be called from within the dtor, when pImpVclMEdit == NULL is a valid state
        return;

    pImpVclTabPage->GetFocus();
}

Size TabPage::CalcMinimumSize() const
{
    Size aSz = pImpVclTabPage->CalcMinimumSize();

    sal_Int32 nLeft, nTop, nRight, nBottom;
    static_cast<vcl::Window*>(const_cast<TabPage*>(this))->GetBorder(nLeft, nTop, nRight, nBottom);
    aSz.AdjustWidth(nLeft + nRight);
    aSz.AdjustHeight(nTop + nBottom);

    return aSz;
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

bool TabPage::EventNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        bDone = pImpVclTabPage->HandleCommand(*rNEvt.GetCommandEvent());
    }
    return bDone || Window::EventNotify(rNEvt);
}

bool TabPage::PreNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;

    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent& rKEvent = *rNEvt.GetKeyEvent();
        if (!rKEvent.GetKeyCode().IsShift() && (rKEvent.GetKeyCode().GetGroup() == KEYGROUP_CURSOR))
        {
            bDone = true;
            switch (rKEvent.GetKeyCode().GetCode())
            {
                case KEY_UP:
                {
                    if (pImpVclTabPage->GetVScrollBar().IsVisible())
                        pImpVclTabPage->GetVScrollBar().DoScrollAction(ScrollType::LineUp);
                }
                break;
                case KEY_DOWN:
                {
                    if (pImpVclTabPage->GetVScrollBar().IsVisible())
                        pImpVclTabPage->GetVScrollBar().DoScrollAction(ScrollType::LineDown);
                }
                break;
                case KEY_PAGEUP:
                {
                    if (pImpVclTabPage->GetVScrollBar().IsVisible())
                        pImpVclTabPage->GetVScrollBar().DoScrollAction(ScrollType::PageUp);
                }
                break;
                case KEY_PAGEDOWN:
                {
                    if (pImpVclTabPage->GetVScrollBar().IsVisible())
                        pImpVclTabPage->GetVScrollBar().DoScrollAction(ScrollType::PageDown);
                }
                break;
                case KEY_LEFT:
                {
                    if (pImpVclTabPage->GetHScrollBar().IsVisible())
                        pImpVclTabPage->GetHScrollBar().DoScrollAction(ScrollType::LineUp);
                }
                break;
                case KEY_RIGHT:
                {
                    if (pImpVclTabPage->GetHScrollBar().IsVisible())
                        pImpVclTabPage->GetHScrollBar().DoScrollAction(ScrollType::LineDown);
                }
                break;
                default:
                {
                    bDone = false;
                }
            }
        }
    }

    return bDone || Window::PreNotify(rNEvt);
}

ScrollBar& TabPage::GetVScrollBar() const
{
    return pImpVclTabPage->GetVScrollBar();
}

TabPageContainer* TabPage::GetTabPageContainer()
{
    return pImpVclTabPage->GetTabPageContainer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
