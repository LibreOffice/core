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
#include <pagefrm.hxx>
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

SwFrameControlsManager::SwFrameControlsManager( const SwFrameControlsManager& rCopy ) :
    m_pEditWin( rCopy.m_pEditWin ),
    m_aControls( rCopy.m_aControls )
{
}

const SwFrameControlsManager& SwFrameControlsManager::operator=( const SwFrameControlsManager& rCopy )
{
    m_pEditWin = rCopy.m_pEditWin;
    m_aControls = rCopy.m_aControls;
    return *this;
}

SwFrameControlPtr SwFrameControlsManager::GetControl( FrameControlType eType, const SwFrm* pFrm )
{
    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator aIt = rControls.find(pFrm);

    if (aIt != rControls.end())
        return aIt->second;

    return SwFrameControlPtr();
}

void SwFrameControlsManager::RemoveControls( const SwFrm* pFrm )
{
    map< FrameControlType, SwFrameControlPtrMap >::iterator pIt = m_aControls.begin();

    while ( pIt != m_aControls.end() )
    {
        SwFrameControlPtrMap& rMap = pIt->second;
        rMap.erase(pFrm);
        ++pIt;
    }
}

void SwFrameControlsManager::RemoveControlsByType( FrameControlType eType, const SwFrm* pFrm )
{
    SwFrameControlPtrMap& rMap = m_aControls[eType];
    rMap.erase(pFrm);
}

void SwFrameControlsManager::HideControls( FrameControlType eType )
{
    SwFrameControlPtrMap::iterator pIt = m_aControls[eType].begin();
    while ( pIt != m_aControls[eType].end() )
    {
        pIt->second->ShowAll( false );
        ++pIt;
    }
}

void SwFrameControlsManager::SetReadonlyControls( bool bReadonly )
{
    map< FrameControlType, SwFrameControlPtrMap >::iterator pIt = m_aControls.begin();

    while ( pIt != m_aControls.end() )
    {
        SwFrameControlPtrMap::iterator aCtrlIt = pIt->second.begin();
        while ( aCtrlIt != pIt->second.end() )
        {
            aCtrlIt->second->SetReadonly( bReadonly );
            ++aCtrlIt;
        }
        ++pIt;
    }
}

void SwFrameControlsManager::SetHeaderFooterControl( const SwPageFrm* pPageFrm, FrameControlType eType, Point aOffset )
{
    OSL_ASSERT( eType == Header || eType == Footer );

    // Check if we already have the control
    SwFrameControlPtr pControl;
    const bool bHeader = ( eType == Header );

    SwFrameControlPtrMap& rControls = m_aControls[eType];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrm);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrm, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl( new SwHeaderFooterWin( m_pEditWin, pPageFrm, bHeader ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        rControls.insert(lb, make_pair(pPageFrm, pNewControl));
        pControl.swap( pNewControl );
    }

    Rectangle aPageRect = m_pEditWin->LogicToPixel( pPageFrm->Frm().SVRect() );

    SwHeaderFooterWin* pHFWin = dynamic_cast< SwHeaderFooterWin* >( pControl.get() );
    assert(pHFWin->IsHeader() == bHeader);
    pHFWin->SetOffset( aOffset, aPageRect.Left(), aPageRect.Right() );

    if ( !pHFWin->IsVisible() )
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetPageBreakControl( const SwPageFrm* pPageFrm )
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    SwFrameControlPtrMap& rControls = m_aControls[PageBreak];

    SwFrameControlPtrMap::iterator lb = rControls.lower_bound(pPageFrm);
    if (lb != rControls.end() && !(rControls.key_comp()(pPageFrm, lb->first)))
        pControl = lb->second;
    else
    {
        SwFrameControlPtr pNewControl( new SwPageBreakWin( m_pEditWin, pPageFrm ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );

        rControls.insert(lb, make_pair(pPageFrm, pNewControl));

        pControl.swap( pNewControl );
    }

    SwPageBreakWin* pWin = dynamic_cast< SwPageBreakWin* >( pControl.get() );
    pWin->UpdatePosition();
    if ( !pWin->IsVisible() )
        pControl->ShowAll( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
