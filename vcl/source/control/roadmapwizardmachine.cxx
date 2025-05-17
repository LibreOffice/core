/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <wizdlg.hxx>

#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <vcl/roadmapwizardmachine.hxx>

#include <vector>

namespace vcl
{
    RoadmapWizardMachine::RoadmapWizardMachine(weld::Window* pParent)
        : WizardMachine(pParent, WizardButtonFlags::NEXT | WizardButtonFlags::PREVIOUS | WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::HELP)
        , m_pImpl( new RoadmapWizardImpl )
    {
        m_xAssistant->connect_jump_page(LINK(this, RoadmapWizardMachine, OnRoadmapItemSelected));
    }

    RoadmapWizardMachine::~RoadmapWizardMachine()
    {
    }

    void RoadmapWizardMachine::SetRoadmapHelpId(const OUString& rId)
    {
        m_xAssistant->set_page_side_help_id(rId);
    }

    void RoadmapWizardMachine::declarePath( PathId _nPathId, const WizardPath& _lWizardStates)
    {
        m_pImpl->aPaths.emplace( _nPathId, _lWizardStates );

        if ( m_pImpl->aPaths.size() == 1 )
            // the very first path -> activate it
            activatePath( _nPathId );
        else
            implUpdateRoadmap( );
    }

    void RoadmapWizardMachine::activatePath( PathId _nPathId, bool _bDecideForIt )
    {
        if ( ( _nPathId == m_pImpl->nActivePath ) && ( _bDecideForIt == m_pImpl->bActivePathIsDefinite ) )
            // nothing to do
            return;

        // does the given path exist?
        Paths::const_iterator aNewPathPos = m_pImpl->aPaths.find( _nPathId );
        DBG_ASSERT( aNewPathPos != m_pImpl->aPaths.end(), "RoadmapWizard::activate: there is no such path!" );
        if ( aNewPathPos == m_pImpl->aPaths.end() )
            return;

        // determine the index of the current state in the current path
        sal_Int32 nCurrentStatePathIndex = -1;
        if ( m_pImpl->nActivePath != PathId::INVALID )
            nCurrentStatePathIndex = m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath );

        DBG_ASSERT( static_cast<sal_Int32>(aNewPathPos->second.size()) > nCurrentStatePathIndex,
                   "RoadmapWizard::activate: you cannot activate a path which has less states than we've already advanced!" );
        // If this asserts, this for instance means that we are already in state number, say, 5
        // of our current path, and the caller tries to activate a path which has less than 5
        // states
        if ( static_cast<sal_Int32>(aNewPathPos->second.size()) <= nCurrentStatePathIndex )
            return;

        // assert that the current and the new path are equal, up to nCurrentStatePathIndex
        Paths::const_iterator aActivePathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
        if ( aActivePathPos != m_pImpl->aPaths.end() )
        {
            if ( RoadmapWizardImpl::getFirstDifferentIndex( aActivePathPos->second, aNewPathPos->second ) <= nCurrentStatePathIndex )
            {
                OSL_FAIL( "RoadmapWizard::activate: you cannot activate a path which conflicts with the current one *before* the current state!" );
                return;
            }
        }

        m_pImpl->nActivePath = _nPathId;
        m_pImpl->bActivePathIsDefinite = _bDecideForIt;

        implUpdateRoadmap( );
    }

    void RoadmapWizardMachine::implUpdateRoadmap( )
    {

        DBG_ASSERT( m_pImpl->aPaths.find( m_pImpl->nActivePath ) != m_pImpl->aPaths.end(),
                   "RoadmapWizard::implUpdateRoadmap: there is no such path!" );
        const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );

        sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );
        if (nCurrentStatePathIndex < 0)
            return;
        assert(nCurrentStatePathIndex >= 0 && o3tl::make_unsigned(nCurrentStatePathIndex) < rActivePath.size());

        // determine up to which index (in the new path) we have to display the items
        RoadmapTypes::ItemIndex nUpperStepBoundary = static_cast<RoadmapTypes::ItemIndex>(rActivePath.size());
        if ( !m_pImpl->bActivePathIsDefinite )
        {
            for (auto const& path : m_pImpl->aPaths)
            {
                if ( path.first == m_pImpl->nActivePath )
                    // it's the path we are just activating -> no need to check anything
                    continue;
                // the index from which on both paths differ
                sal_Int32 nDivergenceIndex = RoadmapWizardImpl::getFirstDifferentIndex( rActivePath, path.second );
                if ( nDivergenceIndex <= nCurrentStatePathIndex )
                    // they differ in an index which we have already left behind us
                    // -> this is no conflict anymore
                    continue;

                // the path conflicts with our new path -> don't activate the
                // *complete* new path, but only up to the step which is unambiguous
                nUpperStepBoundary = nDivergenceIndex;
            }
        }

        // can we advance from the current page?
        bool bCurrentPageCanAdvance = true;
        BuilderPage* pCurrentPage = GetPage( getCurrentState() );
        if ( pCurrentPage )
        {
            const IWizardPageController* pController = getPageController( GetPage( getCurrentState() ) );
            OSL_ENSURE( pController != nullptr, "RoadmapWizard::implUpdateRoadmap: no controller for the current page!" );
            bCurrentPageCanAdvance = !pController || pController->canAdvance();
        }

        // now, we have to remove all items after nCurrentStatePathIndex, and insert the items from the active
        // path, up to (excluding) nUpperStepBoundary
        RoadmapTypes::ItemIndex nRoadmapItems = m_xAssistant->get_n_pages();
        RoadmapTypes::ItemIndex nLoopUntil = ::std::max( nUpperStepBoundary, nRoadmapItems );
        for ( RoadmapTypes::ItemIndex nItemIndex = nCurrentStatePathIndex; nItemIndex < nLoopUntil; ++nItemIndex )
        {
            bool bExistentItem = ( nItemIndex < nRoadmapItems );
            bool bNeedItem = ( nItemIndex < nUpperStepBoundary );

            bool bInsertItem = false;
            if ( bExistentItem )
            {
                if ( !bNeedItem )
                {
                    int nPages = nRoadmapItems;
                    for (int i = nPages - 1; i >= nItemIndex; --i)
                    {
                        m_xAssistant->set_page_title(m_xAssistant->get_page_ident(i), u""_ustr);
                        --nRoadmapItems;
                    }
                    break;
                }
                else
                {
                    // there is an item with this index in the roadmap - does it match what is requested by
                    // the respective state in the active path?
                    RoadmapTypes::ItemId nPresentItemId = m_xAssistant->get_page_ident(nItemIndex).toInt32();
                    WizardTypes::WizardState nRequiredState = rActivePath[ nItemIndex ];
                    if ( nPresentItemId != nRequiredState )
                    {
                        m_xAssistant->set_page_title(OUString::number(nPresentItemId), u""_ustr);
                        bInsertItem = true;
                    }
                }
            }
            else
            {
                DBG_ASSERT( bNeedItem, "RoadmapWizard::implUpdateRoadmap: ehm - none needed, none present - why did the loop not terminate?" );
                bInsertItem = bNeedItem;
            }

            WizardTypes::WizardState nState( rActivePath[ nItemIndex ] );

            if ( bInsertItem )
            {
                GetOrCreatePage(nState);
            }

            OUString sIdent(getPageIdentForState(nState));
            m_xAssistant->set_page_index(sIdent, nItemIndex);
            m_xAssistant->set_page_title(sIdent, getStateDisplayName(nState));

            // if the item is *after* the current state, but the current page does not
            // allow advancing, the disable the state. This relieves derived classes
            // from disabling all future states just because the current state does not
            // (yet) allow advancing.
            const bool bUnconditionedDisable = !bCurrentPageCanAdvance && ( nItemIndex > nCurrentStatePathIndex );
            const bool bEnable = !bUnconditionedDisable && ( m_pImpl->aDisabledStates.find( nState ) == m_pImpl->aDisabledStates.end() );
            m_xAssistant->set_page_sensitive(sIdent, bEnable);
        }
    }

    WizardTypes::WizardState RoadmapWizardMachine::determineNextState( WizardTypes::WizardState _nCurrentState ) const
    {
        sal_Int32 nCurrentStatePathIndex = -1;

        Paths::const_iterator aActivePathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
        if ( aActivePathPos != m_pImpl->aPaths.end() )
            nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( _nCurrentState, aActivePathPos->second );

        DBG_ASSERT( nCurrentStatePathIndex != -1, "RoadmapWizard::determineNextState: ehm - how can we travel if there is no (valid) active path?" );
        if (nCurrentStatePathIndex < 0)
            return WZS_INVALID_STATE;
        assert(nCurrentStatePathIndex >= 0 && o3tl::make_unsigned(nCurrentStatePathIndex) < aActivePathPos->second.size());

        sal_Int32 nNextStateIndex = nCurrentStatePathIndex + 1;

        while   (   ( nNextStateIndex < static_cast<sal_Int32>(aActivePathPos->second.size()) )
               &&  ( m_pImpl->aDisabledStates.find( aActivePathPos->second[ nNextStateIndex ] ) != m_pImpl->aDisabledStates.end() )
               )
        {
            ++nNextStateIndex;
        }

        if ( nNextStateIndex >= static_cast<sal_Int32>(aActivePathPos->second.size()) )
            // there is no next state in the current path (at least none which is enabled)
            return WZS_INVALID_STATE;

        return aActivePathPos->second[ nNextStateIndex ];
    }

    bool RoadmapWizardMachine::canAdvance() const
    {
        if ( !m_pImpl->bActivePathIsDefinite )
        {
            // check how many paths are still allowed
            const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );

            // if current path has only the base item, it is not possible to proceed without activating another path
            if(rActivePath.size()<=1)
                return false;

            sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );

            size_t nPossiblePaths(0);
            for (auto const& path : m_pImpl->aPaths)
            {
                // the index from which on both paths differ
                sal_Int32 nDivergenceIndex = RoadmapWizardImpl::getFirstDifferentIndex( rActivePath, path.second );

                if ( nDivergenceIndex > nCurrentStatePathIndex )
                    // this path is still a possible path
                    nPossiblePaths += 1;
            }

            // if we have more than one path which is still possible, then we assume
            // to always have a next state. Though there might be scenarios where this
            // is not true, but this is too sophisticated (means not really needed) right now.
            if ( nPossiblePaths > 1 )
                return true;
        }

        const WizardPath& rPath = m_pImpl->aPaths[ m_pImpl->nActivePath ];
        return *rPath.rbegin() != getCurrentState();
    }

    void RoadmapWizardMachine::updateTravelUI()
    {
        WizardMachine::updateTravelUI();

        // disable the "Previous" button if all states in our history are disabled
        std::vector< WizardTypes::WizardState > aHistory;
        getStateHistory( aHistory );
        bool bHaveEnabledState = false;
        for (auto const& state : aHistory)
        {
            if ( isStateEnabled(state) )
            {
                bHaveEnabledState = true;
                break;
            }
        }

        enableButtons( WizardButtonFlags::PREVIOUS, bHaveEnabledState );

        implUpdateRoadmap();
    }

    void RoadmapWizardMachine::enterState( WizardTypes::WizardState _nState )
    {
        WizardMachine::enterState( _nState );

        // synchronize the roadmap
        implUpdateRoadmap();
    }

    OUString RoadmapWizardMachine::getStateDisplayName( WizardTypes::WizardState _nState ) const
    {
        OUString sDisplayName;

        StateDescriptions::const_iterator pos = m_pImpl->aStateDescriptors.find( _nState );
        OSL_ENSURE( pos != m_pImpl->aStateDescriptors.end(),
                   "RoadmapWizard::getStateDisplayName: no default implementation available for this state!" );
        if ( pos != m_pImpl->aStateDescriptors.end() )
            sDisplayName = pos->second.first;

        return sDisplayName;
    }

    void RoadmapWizardMachine::enableState( WizardTypes::WizardState _nState, bool _bEnable )
    {
        // remember this (in case the state appears in the roadmap later on)
        if ( _bEnable )
            m_pImpl->aDisabledStates.erase( _nState );
        else
        {
            m_pImpl->aDisabledStates.insert( _nState );
            removePageFromHistory( _nState );
        }

        // if the state is currently in the roadmap, reflect it's new status
        m_xAssistant->set_page_sensitive(getPageIdentForState(_nState), _bEnable);
    }

    bool RoadmapWizardMachine::knowsState( WizardTypes::WizardState i_nState ) const
    {
        for (auto const& path : m_pImpl->aPaths)
        {
            for (auto const& state : path.second)
            {
                if ( state == i_nState )
                    return true;
            }
        }
        return false;
    }

    bool RoadmapWizardMachine::isStateEnabled( WizardTypes::WizardState _nState ) const
    {
        return m_pImpl->aDisabledStates.find( _nState ) == m_pImpl->aDisabledStates.end();
    }

    IMPL_LINK(RoadmapWizardMachine, OnRoadmapItemSelected, const OUString&, rCurItemId, bool)
    {
        WizardTypes::WizardState nSelectedState = getStateFromPageIdent(rCurItemId);

        if (nSelectedState == getCurrentState())
            // nothing to do
            return false;

        if ( isTravelingSuspended() )
            return false;

        WizardTravelSuspension aTravelGuard( *this );

        sal_Int32 nCurrentIndex = m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath );
        sal_Int32 nNewIndex     = m_pImpl->getStateIndexInPath( nSelectedState, m_pImpl->nActivePath );

        DBG_ASSERT( ( nCurrentIndex != -1 ) && ( nNewIndex != -1 ),
                   "RoadmapWizard::OnRoadmapItemSelected: something's wrong here!" );
        if ( ( nCurrentIndex == -1 ) || ( nNewIndex == -1 ) )
        {
            return false;
        }

        bool bResult = true;
        if ( nNewIndex > nCurrentIndex )
        {
            bResult = skipUntil(nSelectedState);
            WizardTypes::WizardState nTemp = nSelectedState;
            while( nTemp )
            {
                if( m_pImpl->aDisabledStates.find( --nTemp ) != m_pImpl->aDisabledStates.end() )
                    removePageFromHistory( nTemp );
            }
        }
        else
            bResult = skipBackwardUntil(nSelectedState);

        return bResult;
    }

}   // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
