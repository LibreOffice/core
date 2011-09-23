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

namespace
{
    class FramePredicate
    {
        const SwFrm* m_pToMatch;

        public:
            FramePredicate( const SwFrm* pFrm ) : m_pToMatch( pFrm ) { };

            virtual bool operator()( SwFrameControlPtr pToCheck )
                { return m_pToMatch == pToCheck->GetFrame(); };
    };
}

SwFrameControlsManager::SwFrameControlsManager( SwEditWin* pEditWin ) :
    m_pEditWin( pEditWin )
{
}

SwFrameControlsManager::~SwFrameControlsManager()
{
    map< FrameControlType, vector< SwFrameControlPtr > >::iterator pIt = m_aControls.begin();

    while ( pIt != m_aControls.end() )
    {
        pIt->second.clear( );
        ++pIt;
    }
    m_aControls.clear();
}

std::vector< SwFrameControlPtr > SwFrameControlsManager::GetControls( FrameControlType eType )
{
    return m_aControls[eType];
}

void SwFrameControlsManager::AddControl( FrameControlType eType, SwFrameControlPtr pControl )
{
    m_aControls[eType].push_back( pControl );
}

void SwFrameControlsManager::RemoveControls( const SwFrm* pFrm )
{
    map< FrameControlType, vector< SwFrameControlPtr > >::iterator pIt = m_aControls.begin();

    while ( pIt != m_aControls.end() )
    {
        vector< SwFrameControlPtr > aVect = pIt->second;
        aVect.erase( remove_if( aVect.begin(),
                                aVect.end(),
                                FramePredicate( pFrm ) ), aVect.end() );
        ++pIt;
    }
}


void SwFrameControlsManager::HideControls( FrameControlType eType )
{
    vector< SwFrameControlPtr >::iterator pIt = m_aControls[eType].begin();
    while ( pIt != m_aControls[eType].end() )
    {
        ( *pIt )->ShowAll( false );
        pIt++;
    }
}

void SwFrameControlsManager::SetReadonlyControls( bool bReadonly )
{
    map< FrameControlType, vector< SwFrameControlPtr > >::iterator pIt = m_aControls.begin();

    while ( pIt != m_aControls.end() )
    {
        vector< SwFrameControlPtr >::iterator pVectIt = pIt->second.begin();
        while ( pVectIt != pIt->second.end() )
        {
            ( *pVectIt )->SetReadonly( bReadonly );
            ++pVectIt;
        }
        ++pIt;
    }
}

void SwFrameControlsManager::SetHeaderFooterControl( const SwPageFrm* pPageFrm, bool bHeader, Point aOffset )
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    vector< SwFrameControlPtr > aControls = m_aControls[HeaderFooter];

    vector< SwFrameControlPtr >::iterator pIt = aControls.begin();
    while ( pIt != aControls.end() && !pControl.get() )
    {
        SwHeaderFooterWin* pToTest = dynamic_cast< SwHeaderFooterWin* >( pIt->get() );
        if ( pToTest->GetPageFrame( ) == pPageFrm &&
             pToTest->IsHeader( ) == bHeader )
            pControl = *pIt;
        pIt++;
    }

    if ( !pControl.get() )
    {
        SwFrameControlPtr pNewControl( new SwHeaderFooterWin( m_pEditWin, pPageFrm, bHeader ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        AddControl( HeaderFooter, pNewControl );
        pControl.swap( pNewControl );
    }

    Rectangle aPageRect = m_pEditWin->LogicToPixel( pPageFrm->Frm().SVRect() );

    SwHeaderFooterWin* pHFWin = dynamic_cast< SwHeaderFooterWin* >( pControl.get() );
    pHFWin->SetOffset( aOffset, aPageRect.Left(), aPageRect.Right() );

    if ( !pHFWin->IsVisible() )
        pControl->ShowAll( true );
}

void SwFrameControlsManager::SetPageBreakControl( const SwPageFrm* pPageFrm )
{
    // Check if we already have the control
    SwFrameControlPtr pControl;

    vector< SwFrameControlPtr > aControls = m_aControls[PageBreak];

    vector< SwFrameControlPtr >::iterator pIt = aControls.begin();
    while ( pIt != aControls.end() && !pControl.get() )
    {
        SwPageBreakWin* pToTest = dynamic_cast< SwPageBreakWin* >( pIt->get() );
        if ( pToTest->GetPageFrame( ) == pPageFrm )
            pControl = *pIt;
        pIt++;
    }

    if ( !pControl.get() )
    {
        SwFrameControlPtr pNewControl( new SwPageBreakWin( m_pEditWin, pPageFrm ) );
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        pNewControl->SetReadonly( pViewOpt->IsReadonly() );
        AddControl( PageBreak, pNewControl );
        pControl.swap( pNewControl );
    }

    SwPageBreakWin* pWin = dynamic_cast< SwPageBreakWin* >( pControl.get() );
    pWin->UpdatePosition();
    if ( !pWin->IsVisible() )
        pControl->ShowAll( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
