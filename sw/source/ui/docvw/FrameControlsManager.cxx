/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
