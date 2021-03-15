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


#include <vcl/toolkit/roadmap.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include <strings.hrc>
#include <svdata.hxx>
#include <wizdlg.hxx>

#include <vector>
#include <map>
#include <set>

#include "wizimpldata.hxx"
#include <uiobject-internal.hxx>

namespace vcl
{
    using namespace RoadmapWizardTypes;

    namespace
    {
        typedef ::std::set< WizardTypes::WizardState > StateSet;

        typedef ::std::map<
                    PathId,
                    WizardPath
                > Paths;

        typedef ::std::map<
                    WizardTypes::WizardState,
                    ::std::pair<
                        OUString,
                        RoadmapPageFactory
                    >
                > StateDescriptions;
    }

    struct RoadmapWizardImpl
    {
        ScopedVclPtr<ORoadmap> pRoadmap;
        Paths               aPaths;
        PathId              nActivePath;
        StateDescriptions   aStateDescriptors;
        StateSet            aDisabledStates;
        bool                bActivePathIsDefinite;

        RoadmapWizardImpl()
            :pRoadmap( nullptr )
            ,nActivePath( -1 )
            ,bActivePathIsDefinite( false )
        {
        }

        /// returns the index of the current state in given path, or -1
        static sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath );
        /// returns the index of the current state in the path with the given id, or -1
        sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId );
        /// returns the index of the first state in which the two given paths differ
        static sal_Int32 getFirstDifferentIndex( const WizardPath& _rLHS, const WizardPath& _rRHS );
    };


    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath )
    {
        sal_Int32 nStateIndexInPath = 0;
        bool bFound = false;
        for (auto const& path : _rPath)
        {
            if (path == _nState)
            {
                bFound = true;
                break;
            }
            ++nStateIndexInPath;
        }
        if (!bFound)
            nStateIndexInPath = -1;
        return nStateIndexInPath;
    }


    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId )
    {
        sal_Int32 nStateIndexInPath = -1;
        Paths::const_iterator aPathPos = aPaths.find( _nPathId );
        if ( aPathPos != aPaths.end( ) )
            nStateIndexInPath = getStateIndexInPath( _nState, aPathPos->second );
        return nStateIndexInPath;
    }


    sal_Int32 RoadmapWizardImpl::getFirstDifferentIndex( const WizardPath& _rLHS, const WizardPath& _rRHS )
    {
        sal_Int32 nMinLength = ::std::min( _rLHS.size(), _rRHS.size() );
        for ( sal_Int32 nCheck = 0; nCheck < nMinLength; ++nCheck )
        {
            if ( _rLHS[ nCheck ] != _rRHS[ nCheck ] )
                return nCheck;
        }
        return nMinLength;
    }

    //= RoadmapWizard
    RoadmapWizard::RoadmapWizard(vcl::Window* pParent, WinBits nStyle, InitFlag eFlag)
        : Dialog(pParent, nStyle, eFlag)
        , m_pFinish(nullptr)
        , m_pCancel(nullptr)
        , m_pNextPage(nullptr)
        , m_pPrevPage(nullptr)
        , m_pHelp(nullptr)
        , m_xWizardImpl(new WizardMachineImplData)
        , m_xRoadmapImpl(new RoadmapWizardImpl)
    {
        mpFirstPage     = nullptr;
        mpFirstBtn      = nullptr;
        mpCurTabPage    = nullptr;
        mpPrevBtn       = nullptr;
        mpNextBtn       = nullptr;
        mpViewWindow    = nullptr;
        mnCurLevel      = 0;
        mbEmptyViewMargin =  false;
        mnLeftAlignCount = 0;

        maWizardLayoutIdle.SetPriority(TaskPriority::RESIZE);
        maWizardLayoutIdle.SetInvokeHandler( LINK( this, RoadmapWizard, ImplHandleWizardLayoutTimerHdl ) );

        implConstruct(WizardButtonFlags::NEXT | WizardButtonFlags::PREVIOUS | WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::HELP);

        SetLeftAlignedButtonCount( 1 );
        mbEmptyViewMargin = true;

        m_xRoadmapImpl->pRoadmap.disposeAndReset( VclPtr<ORoadmap>::Create( this, WB_TABSTOP ) );
        m_xRoadmapImpl->pRoadmap->SetText( VclResId( STR_WIZDLG_ROADMAP_TITLE ) );
        m_xRoadmapImpl->pRoadmap->SetPosPixel( Point( 0, 0 ) );
        m_xRoadmapImpl->pRoadmap->SetItemSelectHdl( LINK( this, RoadmapWizard, OnRoadmapItemSelected ) );

        Size aRoadmapSize = LogicToPixel(Size(85, 0), MapMode(MapUnit::MapAppFont));
        aRoadmapSize.setHeight( GetSizePixel().Height() );
        m_xRoadmapImpl->pRoadmap->SetSizePixel( aRoadmapSize );

        mpViewWindow = m_xRoadmapImpl->pRoadmap;
        m_xRoadmapImpl->pRoadmap->Show();
    }

    RoadmapWizardMachine::RoadmapWizardMachine(weld::Window* pParent)
        : WizardMachine(pParent, WizardButtonFlags::NEXT | WizardButtonFlags::PREVIOUS | WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::HELP)
        , m_pImpl( new RoadmapWizardImpl )
    {
        m_xAssistant->connect_jump_page(LINK(this, RoadmapWizardMachine, OnRoadmapItemSelected));
    }

    void RoadmapWizard::ShowRoadmap(bool bShow)
    {
        m_xRoadmapImpl->pRoadmap->Show(bShow);
        CalcAndSetSize();
    }

    RoadmapWizard::~RoadmapWizard()
    {
        disposeOnce();
    }

    RoadmapWizardMachine::~RoadmapWizardMachine()
    {
    }

    void RoadmapWizard::dispose()
    {
        m_xRoadmapImpl.reset();

        m_pFinish.disposeAndClear();
        m_pCancel.disposeAndClear();
        m_pNextPage.disposeAndClear();
        m_pPrevPage.disposeAndClear();
        m_pHelp.disposeAndClear();

        if (m_xWizardImpl)
        {
            for (WizardTypes::WizardState i = 0; i < m_xWizardImpl->nFirstUnknownPage; ++i)
            {
                TabPage *pPage = GetPage(i);
                if (pPage)
                    pPage->disposeOnce();
            }
            m_xWizardImpl.reset();
        }

        maWizardLayoutIdle.Stop();

        // Remove all buttons
        while ( mpFirstBtn )
            RemoveButton( mpFirstBtn->mpButton );

        // Remove all pages
        while ( mpFirstPage )
            RemovePage( mpFirstPage->mpPage );

        mpCurTabPage.clear();
        mpPrevBtn.clear();
        mpNextBtn.clear();
        mpViewWindow.clear();
        Dialog::dispose();
    }

    void RoadmapWizard::SetRoadmapHelpId( const OString& _rId )
    {
        m_xRoadmapImpl->pRoadmap->SetHelpId( _rId );
    }

    void RoadmapWizardMachine::SetRoadmapHelpId(const OString& rId)
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
        if ( m_pImpl->nActivePath != -1 )
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

    void RoadmapWizard::implUpdateRoadmap( )
    {
        DBG_ASSERT( m_xRoadmapImpl->aPaths.find( m_xRoadmapImpl->nActivePath ) != m_xRoadmapImpl->aPaths.end(),
            "RoadmapWizard::implUpdateRoadmap: there is no such path!" );
        const WizardPath& rActivePath( m_xRoadmapImpl->aPaths[ m_xRoadmapImpl->nActivePath ] );

        sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );
        if (nCurrentStatePathIndex < 0)
            return;

        // determine up to which index (in the new path) we have to display the items
        RoadmapTypes::ItemIndex nUpperStepBoundary = static_cast<RoadmapTypes::ItemIndex>(rActivePath.size());
        bool bIncompletePath = false;
        if ( !m_xRoadmapImpl->bActivePathIsDefinite )
        {
            for (auto const& path : m_xRoadmapImpl->aPaths)
            {
                if ( path.first == m_xRoadmapImpl->nActivePath )
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
                bIncompletePath = true;
            }
        }

        // can we advance from the current page?
        bool bCurrentPageCanAdvance = true;
        TabPage* pCurrentPage = GetPage( getCurrentState() );
        if ( pCurrentPage )
        {
            const IWizardPageController* pController = getPageController( GetPage( getCurrentState() ) );
            OSL_ENSURE( pController != nullptr, "RoadmapWizard::implUpdateRoadmap: no controller for the current page!" );
            bCurrentPageCanAdvance = !pController || pController->canAdvance();
        }

        // now, we have to remove all items after nCurrentStatePathIndex, and insert the items from the active
        // path, up to (excluding) nUpperStepBoundary
        RoadmapTypes::ItemIndex nLoopUntil = ::std::max( nUpperStepBoundary, m_xRoadmapImpl->pRoadmap->GetItemCount() );
        for ( RoadmapTypes::ItemIndex nItemIndex = nCurrentStatePathIndex; nItemIndex < nLoopUntil; ++nItemIndex )
        {
            bool bExistentItem = ( nItemIndex < m_xRoadmapImpl->pRoadmap->GetItemCount() );
            bool bNeedItem = ( nItemIndex < nUpperStepBoundary );

            bool bInsertItem = false;
            if ( bExistentItem )
            {
                if ( !bNeedItem )
                {
                    while ( nItemIndex < m_xRoadmapImpl->pRoadmap->GetItemCount() )
                        m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
                    break;
                }
                else
                {
                    // there is an item with this index in the roadmap - does it match what is requested by
                    // the respective state in the active path?
                    RoadmapTypes::ItemId nPresentItemId = m_xRoadmapImpl->pRoadmap->GetItemID( nItemIndex );
                    WizardTypes::WizardState nRequiredState = rActivePath[ nItemIndex ];
                    if ( nPresentItemId != nRequiredState )
                    {
                        m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
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
                m_xRoadmapImpl->pRoadmap->InsertRoadmapItem(
                    nItemIndex,
                    getStateDisplayName( nState ),
                    nState,
                    true
                );
            }

            // if the item is *after* the current state, but the current page does not
            // allow advancing, the disable the state. This relieves derived classes
            // from disabling all future states just because the current state does not
            // (yet) allow advancing.
            const bool bUnconditionedDisable = !bCurrentPageCanAdvance && ( nItemIndex > nCurrentStatePathIndex );
            const bool bEnable = !bUnconditionedDisable && ( m_xRoadmapImpl->aDisabledStates.find( nState ) == m_xRoadmapImpl->aDisabledStates.end() );

            m_xRoadmapImpl->pRoadmap->EnableRoadmapItem( m_xRoadmapImpl->pRoadmap->GetItemID( nItemIndex ), bEnable );
        }

        m_xRoadmapImpl->pRoadmap->SetRoadmapComplete( !bIncompletePath );
    }

    void RoadmapWizardMachine::implUpdateRoadmap( )
    {

        DBG_ASSERT( m_pImpl->aPaths.find( m_pImpl->nActivePath ) != m_pImpl->aPaths.end(),
            "RoadmapWizard::implUpdateRoadmap: there is no such path!" );
        const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );

        sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );
        if (nCurrentStatePathIndex < 0)
            return;

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
                        m_xAssistant->set_page_title(m_xAssistant->get_page_ident(i), "");
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
                        m_xAssistant->set_page_title(OString::number(nPresentItemId), "");
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

            OString sIdent(OString::number(nState));
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

    WizardTypes::WizardState RoadmapWizard::determineNextState( WizardTypes::WizardState _nCurrentState ) const
    {
        sal_Int32 nCurrentStatePathIndex = -1;

        Paths::const_iterator aActivePathPos = m_xRoadmapImpl->aPaths.find( m_xRoadmapImpl->nActivePath );
        if ( aActivePathPos != m_xRoadmapImpl->aPaths.end() )
            nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( _nCurrentState, aActivePathPos->second );

        DBG_ASSERT( nCurrentStatePathIndex != -1, "RoadmapWizard::determineNextState: ehm - how can we travel if there is no (valid) active path?" );
        if ( nCurrentStatePathIndex == -1 )
            return WZS_INVALID_STATE;

        sal_Int32 nNextStateIndex = nCurrentStatePathIndex + 1;

        while   (   ( nNextStateIndex < static_cast<sal_Int32>(aActivePathPos->second.size()) )
                &&  ( m_xRoadmapImpl->aDisabledStates.find( aActivePathPos->second[ nNextStateIndex ] ) != m_xRoadmapImpl->aDisabledStates.end() )
                )
        {
            ++nNextStateIndex;
        }

        if ( nNextStateIndex >= static_cast<sal_Int32>(aActivePathPos->second.size()) )
            // there is no next state in the current path (at least none which is enabled)
            return WZS_INVALID_STATE;

        return aActivePathPos->second[ nNextStateIndex ];
    }

    WizardTypes::WizardState RoadmapWizardMachine::determineNextState( WizardTypes::WizardState _nCurrentState ) const
    {
        sal_Int32 nCurrentStatePathIndex = -1;

        Paths::const_iterator aActivePathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
        if ( aActivePathPos != m_pImpl->aPaths.end() )
            nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( _nCurrentState, aActivePathPos->second );

        DBG_ASSERT( nCurrentStatePathIndex != -1, "RoadmapWizard::determineNextState: ehm - how can we travel if there is no (valid) active path?" );
        if ( nCurrentStatePathIndex == -1 )
            return WZS_INVALID_STATE;

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

    bool RoadmapWizard::canAdvance() const
    {
        if ( !m_xRoadmapImpl->bActivePathIsDefinite )
        {
            // check how many paths are still allowed
            const WizardPath& rActivePath( m_xRoadmapImpl->aPaths[ m_xRoadmapImpl->nActivePath ] );
            sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );

            size_t nPossiblePaths(0);
            for (auto const& path : m_xRoadmapImpl->aPaths)
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

        const WizardPath& rPath = m_xRoadmapImpl->aPaths[ m_xRoadmapImpl->nActivePath ];
        return *rPath.rbegin() != getCurrentState();
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

    IMPL_LINK_NOARG(RoadmapWizard, OnRoadmapItemSelected, LinkParamNone*, void)
    {
        RoadmapTypes::ItemId nCurItemId = m_xRoadmapImpl->pRoadmap->GetCurrentRoadmapItemID();
        if ( nCurItemId == getCurrentState() )
            // nothing to do
            return;

        if ( isTravelingSuspended() )
            return;

        RoadmapWizardTravelSuspension aTravelGuard( *this );

        sal_Int32 nCurrentIndex = m_xRoadmapImpl->getStateIndexInPath( getCurrentState(), m_xRoadmapImpl->nActivePath );
        sal_Int32 nNewIndex     = m_xRoadmapImpl->getStateIndexInPath( nCurItemId, m_xRoadmapImpl->nActivePath );

        DBG_ASSERT( ( nCurrentIndex != -1 ) && ( nNewIndex != -1 ),
            "RoadmapWizard::OnRoadmapItemSelected: something's wrong here!" );
        if ( ( nCurrentIndex == -1 ) || ( nNewIndex == -1 ) )
        {
            return;
        }

        bool bResult = true;
        if ( nNewIndex > nCurrentIndex )
        {
            bResult = skipUntil( static_cast<WizardTypes::WizardState>(nCurItemId) );
            WizardTypes::WizardState nTemp = static_cast<WizardTypes::WizardState>(nCurItemId);
            while( nTemp )
            {
                if( m_xRoadmapImpl->aDisabledStates.find( --nTemp ) != m_xRoadmapImpl->aDisabledStates.end() )
                    removePageFromHistory( nTemp );
            }
        }
        else
            bResult = skipBackwardUntil( static_cast<WizardTypes::WizardState>(nCurItemId) );

        if ( !bResult )
            m_xRoadmapImpl->pRoadmap->SelectRoadmapItemByID( getCurrentState() );
    }

    IMPL_LINK(RoadmapWizardMachine, OnRoadmapItemSelected, const OString&, rCurItemId, bool)
    {
        int nCurItemId = rCurItemId.toInt32();

        if ( nCurItemId == getCurrentState() )
            // nothing to do
            return false;

        if ( isTravelingSuspended() )
            return false;

        WizardTravelSuspension aTravelGuard( *this );

        sal_Int32 nCurrentIndex = m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath );
        sal_Int32 nNewIndex     = m_pImpl->getStateIndexInPath( nCurItemId, m_pImpl->nActivePath );

        DBG_ASSERT( ( nCurrentIndex != -1 ) && ( nNewIndex != -1 ),
            "RoadmapWizard::OnRoadmapItemSelected: something's wrong here!" );
        if ( ( nCurrentIndex == -1 ) || ( nNewIndex == -1 ) )
        {
            return false;
        }

        bool bResult = true;
        if ( nNewIndex > nCurrentIndex )
        {
            bResult = skipUntil( static_cast<WizardTypes::WizardState>(nCurItemId) );
            WizardTypes::WizardState nTemp = static_cast<WizardTypes::WizardState>(nCurItemId);
            while( nTemp )
            {
                if( m_pImpl->aDisabledStates.find( --nTemp ) != m_pImpl->aDisabledStates.end() )
                    removePageFromHistory( nTemp );
            }
        }
        else
            bResult = skipBackwardUntil( static_cast<WizardTypes::WizardState>(nCurItemId) );

        return bResult;
    }

    void RoadmapWizard::enterState(WizardTypes::WizardState nState)
    {
        // tell the page
        IWizardPageController* pController = getPageController( GetPage( nState ) );
        if (pController)
        {
            pController->initializePage();

            if ( isAutomaticNextButtonStateEnabled() )
                enableButtons( WizardButtonFlags::NEXT, canAdvance() );

            enableButtons( WizardButtonFlags::PREVIOUS, !m_xWizardImpl->aStateHistory.empty() );

            // set the new title - it depends on the current page (i.e. state)
            implUpdateTitle();
        }

        // synchronize the roadmap
        implUpdateRoadmap( );
        m_xRoadmapImpl->pRoadmap->SelectRoadmapItemByID( getCurrentState() );
    }

    void RoadmapWizardMachine::enterState( WizardTypes::WizardState _nState )
    {
        WizardMachine::enterState( _nState );

        // synchronize the roadmap
        implUpdateRoadmap();
    }

    OUString RoadmapWizard::getStateDisplayName( WizardTypes::WizardState _nState ) const
    {
        OUString sDisplayName;

        StateDescriptions::const_iterator pos = m_xRoadmapImpl->aStateDescriptors.find( _nState );
        OSL_ENSURE( pos != m_xRoadmapImpl->aStateDescriptors.end(),
            "RoadmapWizard::getStateDisplayName: no default implementation available for this state!" );
        if ( pos != m_xRoadmapImpl->aStateDescriptors.end() )
            sDisplayName = pos->second.first;

        return sDisplayName;
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

    VclPtr<TabPage> RoadmapWizard::createPage( WizardTypes::WizardState _nState )
    {
        VclPtr<TabPage> pPage;

        StateDescriptions::const_iterator pos = m_xRoadmapImpl->aStateDescriptors.find( _nState );
        OSL_ENSURE( pos != m_xRoadmapImpl->aStateDescriptors.end(),
            "RoadmapWizard::createPage: no default implementation available for this state!" );
        if ( pos != m_xRoadmapImpl->aStateDescriptors.end() )
        {
            RoadmapPageFactory pFactory = pos->second.second;
            pPage = (*pFactory)( *this );
        }

        return pPage;
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
        m_xAssistant->set_page_sensitive(OString::number(_nState), _bEnable);
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

    void RoadmapWizard::InsertRoadmapItem(int nItemIndex, const OUString& rText, int nItemId, bool bEnable)
    {
        m_xRoadmapImpl->pRoadmap->InsertRoadmapItem(nItemIndex, rText, nItemId, bEnable);
    }

    void RoadmapWizard::SelectRoadmapItemByID(int nItemId, bool bGrabFocus)
    {
        m_xRoadmapImpl->pRoadmap->SelectRoadmapItemByID(nItemId, bGrabFocus);
    }

    void RoadmapWizard::DeleteRoadmapItems()
    {
        while (m_xRoadmapImpl->pRoadmap->GetItemCount())
            m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem(0);
    }

    void RoadmapWizard::SetItemSelectHdl( const Link<LinkParamNone*,void>& _rHdl )
    {
        m_xRoadmapImpl->pRoadmap->SetItemSelectHdl(_rHdl);
    }

    int RoadmapWizard::GetCurrentRoadmapItemID() const
    {
        return m_xRoadmapImpl->pRoadmap->GetCurrentRoadmapItemID();
    }

    FactoryFunction RoadmapWizard::GetUITestFactory() const
    {
        return RoadmapWizardUIObject::create;
    }

}   // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
