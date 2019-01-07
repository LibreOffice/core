/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <FrameControlsManager.hxx>
#include <HeaderFooterWin.hxx>
#include <PageBreakWin.hxx>
#include <UnfloatTableButton.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>
#include <viewopt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

using namespace std;

SwFrameControlsManager::SwFrameControlsManager( SwEditWin* pEditWin ) :
    m_pEditWin( pEditWin ),
    m_aControls( )
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
    assert( eType == Header || eType == Footer );

    // Check if we already have the control
    SwFrameControlPtr pControl;
    const bool bHeader = ( eType == Header );

    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl(
                new SwFrameControl( VclPtr<SwHeaderFooterWin>::Create(
                                        m_pEditWin, pPageFrame, bHeader ).get() ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        rControls.insert(lb, make_pair(pPageFrame, pNewControl));
        pControl.swap( pNewControl );
    }

    tools::Rectangle aPageRect = m_pEditWin->LogicToPixel( pPageFrame->getFrameArea().SVRect() );

    SwHeaderFooterWin* pWin = dynamic_cast<SwHeaderFooterWin *>(pControl->GetWindow());
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

    SwFrameControlPtrMap& rControls = m_aControls[PageBreak];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrame, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl( new SwFrameControl(
                VclPtr<SwPageBreakWin>::Create( m_pEditWin, pPageFrame ).get() ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pPageFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    SwPageBreakWin* pWin = dynamic_cast<SwPageBreakWin *>(pControl->GetWindow());
    assert (pWin != nullptr);
    pWin->UpdatePosition();
    if (!pWin->IsVisible())
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetUnfloatTableButton( const SwFlyFrame* pFlyFrame, bool bShow, Point aBottomRightPixel )
{
    if(pFlyFrame == nullptr)
        return;

    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[FloatingTable];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pFlyFrame);
    if (lb != rControls.end() && !(rControls.key_comp()(pFlyFrame, lb->first)))
        pControl = lb->second;
    else if (!bShow) // Do not create the control when it's not shown
        return;
    else
    {
        SwFrameControlPtr pNewControl( new SwFrameControl(
                VclPtr<UnfloatTableButton>::Create( m_pEditWin, pFlyFrame ).get() ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pFlyFrame, pNewControl));

        pControl.swap( pNewControl );
    }

    UnfloatTableButton* pButton = dynamic_cast<UnfloatTableButton*>(pControl->GetWindow());
    assert(pButton != nullptr);
    pButton->SetOffset(aBottomRightPixel);
    pControl->ShowAll( bShow );
}

SwFrameMenuButtonBase::SwFrameMenuButtonBase( SwEditWin* pEditWin, const SwFrame* pFrame ) :
    MenuButton( pEditWin, WB_DIALOGCONTROL ),
    m_pEditWin( pEditWin ),
    m_pFrame( pFrame )
{
}

const SwPageFrame* SwFrameMenuButtonBase::GetPageFrame()
{
    if (m_pFrame->IsPageFrame())
        return static_cast<const SwPageFrame*>( m_pFrame );

    if (m_pFrame->IsFlyFrame())
        return static_cast<const SwFlyFrame*>(m_pFrame)->GetAnchorFrame()->FindPageFrame();

    return m_pFrame->FindPageFrame();
}

void SwFrameMenuButtonBase::dispose()
{
    m_pEditWin.clear();
    m_pFrame = nullptr;
    MenuButton::dispose();
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
