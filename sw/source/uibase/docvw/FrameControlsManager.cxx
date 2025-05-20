/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <cntfrm.hxx>
#include <FrameControlsManager.hxx>
#include <HeaderFooterWin.hxx>
#include <PageBreakWin.hxx>
#include <UnfloatTableButton.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>
#include <viewopt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <OutlineContentVisibilityWin.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <contentcontrolaliasbutton.hxx>

SwFrameControlsManager::SwFrameControlsManager( SwEditWin* pEditWin ) :
    m_pEditWin( pEditWin )
{
}

SwFrameControlsManager::~SwFrameControlsManager()
{
}

void SwFrameControlsManager::dispose()
{
    m_aControls.clear();
}

SwFrameControlPtr SwFrameControlsManager::GetControl( FrameControlType eType, const SwFrame* pFrame )
{
    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator aIt = rControls.find(pFrame);

    if (aIt != rControls.end())
        return aIt->second;

    return SwFrameControlPtr();
}

void SwFrameControlsManager::RemoveControls( const SwFrame* pFrame )
{
    for ( auto& rEntry : m_aControls )
    {
        SwFrameControlPtrMap& rMap = rEntry.second;
        rMap.erase(pFrame);
    }
}

void SwFrameControlsManager::RemoveControlsByType( FrameControlType eType, const SwFrame* pFrame )
{
    SwFrameControlPtrMap& rMap = m_aControls[eType];
    rMap.erase(pFrame);
}

void SwFrameControlsManager::HideControls( FrameControlType eType )
{
    for ( const auto& rCtrl : m_aControls[eType] )
        rCtrl.second->ShowAll( false );
}

void SwFrameControlsManager::SetReadonlyControls( bool bReadonly )
{
    for ( auto& rEntry : m_aControls )
        for ( auto& rCtrl : rEntry.second )
            rCtrl.second->SetReadonly( bReadonly );
}

void SwFrameControlsManager::SetHeaderFooterControl( const SwPageFrame* pPageFrame, FrameControlType eType, Point aOffset )
{
    assert( eType == FrameControlType::Header || eType == FrameControlType::Footer );

    // Check if we already have the control
    SwFrameControlPtr pControl;
    const bool bHeader = ( eType == FrameControlType::Header );

    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl =
                std::make_shared<SwFrameControl>( VclPtr<SwHeaderFooterDashedLine>::Create(
                                        m_pEditWin, pPageFrame, bHeader ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        rControls.insert(lb, make_pair(pPageFrame, pNewControl));
        pControl.swap( pNewControl );
    }

    tools::Rectangle aPageRect = m_pEditWin->LogicToPixel( pPageFrame->getFrameArea().SVRect() );

    SwHeaderFooterDashedLine* pWin = dynamic_cast<SwHeaderFooterDashedLine*>(pControl->GetWindow());
    assert( pWin != nullptr) ;
    assert( pWin->IsHeader() == bHeader );
    pWin->SetOffset( aOffset, aPageRect.Left(), aPageRect.Right() );

    if (!pWin->IsVisible())
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetPageBreakControl( const SwPageFrame* pPageFrame )
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::PageBreak];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl = std::make_shared<SwFrameControl>(
                VclPtr<SwBreakDashedLine>::Create( m_pEditWin, pPageFrame ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pPageFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    SwBreakDashedLine* pWin = static_cast<SwBreakDashedLine*>(pControl->GetWindow());
    assert (pWin != nullptr);
    pWin->UpdatePosition();
    if (!pWin->IsVisible())
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetUnfloatTableButton( const SwFlyFrame* pFlyFrame, bool bShow, Point aTopRightPixel )
{
    if(pFlyFrame == nullptr)
        return;

    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::FloatingTable];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pFlyFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pFlyFrame, lb->first)))
        pControl = lb->second;
    else if (!bShow) // Do not create the control when it's not shown
        return;
    else
    {
        SwFrameControlPtr pNewControl = std::make_shared<SwFrameControl>(
                VclPtr<UnfloatTableButton>::Create( m_pEditWin, pFlyFrame ).get() );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pFlyFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    UnfloatTableButton* pButton = dynamic_cast<UnfloatTableButton*>(pControl->GetWindow());
    assert(pButton != nullptr);
    pButton->SetOffset(aTopRightPixel);
    pControl->ShowAll( bShow );
}

void SwFrameControlsManager::SetContentControlAliasButton(SwContentControl* pContentControl,
                                                          Point aTopLeftPixel)
{
    SwFrameControlPtr pControl;
    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::ContentControl];
    // We don't really have a key, the SwPaM's mark decides what is the single content control in
    // this view that can have an alias button.
    SwFrameControlPtrMap::iterator it = rControls.find(nullptr);
    if (it != rControls.end())
        pControl = it->second;
    else
    {
        pControl = std::make_shared<SwFrameControl>(
            VclPtr<SwContentControlAliasButton>::Create(m_pEditWin, pContentControl).get());
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pControl->SetReadonly(pViewOpt->IsReadonly());
        rControls[nullptr] = pControl;
    }

    auto pButton = dynamic_cast<SwContentControlAliasButton*>(pControl->GetWindow());
    assert(pButton);
    pButton->SetOffset(aTopLeftPixel);
    pButton->SetContentControl(pContentControl);
    pControl->ShowAll(true);
}

SwFrameMenuButtonBase::SwFrameMenuButtonBase(SwEditWin* pEditWin, const SwFrame* pFrame,
                                             const OUString& rUIXMLDescription, const OUString& rID)
    : InterimItemWindow(pEditWin, rUIXMLDescription, rID)
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
{
}

void SwFrameControlsManager::SetOutlineContentVisibilityButton(const SwContentFrame* pContentFrame)
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FrameControlType::Outline];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pContentFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pContentFrame, lb->first)))
    {
        pControl = lb->second;
    }
    else
    {
        SwFrameControlPtr pNewControl =
                std::make_shared<SwFrameControl>(VclPtr<SwOutlineContentVisibilityWin>::Create(
                                        m_pEditWin, pContentFrame).get());
        rControls.insert(lb, make_pair(pContentFrame, pNewControl));
        pControl.swap(pNewControl);
    }

    SwOutlineContentVisibilityWin* pWin = dynamic_cast<SwOutlineContentVisibilityWin *>(pControl->GetWindow());
    assert(pWin != nullptr);
    pWin->Set();

    if (pWin->GetSymbol() == ButtonSymbol::SHOW)
        pWin->Show(); // show the SHOW button immediately
    else if (!pWin->IsVisible() && pWin->GetSymbol() == ButtonSymbol::HIDE)
        pWin->ShowAll(true);
}

const SwPageFrame* SwFrameMenuButtonBase::GetPageFrame(const SwFrame* pFrame)
{
    if (pFrame->IsPageFrame())
        return static_cast<const SwPageFrame*>(pFrame);

    if (pFrame->IsFlyFrame())
        return static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame()->FindPageFrame();

    return pFrame->FindPageFrame();
}

const SwPageFrame* SwFrameMenuButtonBase::GetPageFrame() const
{
    return SwFrameMenuButtonBase::GetPageFrame(m_pFrame);
}

void SwFrameMenuButtonBase::dispose()
{
    m_pEditWin.reset();
    m_pFrame = nullptr;
    m_xVirDev.disposeAndClear();
    InterimItemWindow::dispose();
}

void SwFrameMenuButtonBase::SetVirDevFont(OutputDevice& rVirDev)
{
    // Get the font and configure it
    vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetToolFont();
    weld::SetPointFont(rVirDev, aFont);
}

void SwFrameMenuButtonBase::SetVirDevFont()
{
    SetVirDevFont(*m_xVirDev);
}

SwFrameControl::SwFrameControl( const VclPtr<vcl::Window> &pWindow )
{
    assert(static_cast<bool>(pWindow));
    mxWindow.reset( pWindow );
    mpIFace = dynamic_cast<ISwFrameControl *>( pWindow.get() );
}

SwFrameControl::~SwFrameControl()
{
    mpIFace = nullptr;
    mxWindow.disposeAndClear();
}

ISwFrameControl::~ISwFrameControl()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
