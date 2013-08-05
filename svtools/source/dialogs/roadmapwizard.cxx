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


#include <roadmapwizard.hxx>
#include <svtools.hrc>
#include <svtresid.hxx>
#include <roadmap.hxx>
#include <tools/debug.hxx>

#include <stdarg.h>

#include <vector>
#include <map>
#include <set>

//........................................................................
namespace svt
{
//........................................................................

    namespace
    {
        typedef ::std::set< WizardTypes::WizardState >          StateSet;

        typedef ::std::map<
                    RoadmapWizardTypes::PathId,
                    RoadmapWizardTypes::WizardPath
                >                                               Paths;

        typedef ::std::map<
                    WizardTypes::WizardState,
                    ::std::pair<
                        String,
                        RoadmapWizardTypes::RoadmapPageFactory
                    >
                >                                               StateDescriptions;
    }

    struct RoadmapWizardImpl : public RoadmapWizardTypes
    {
        ORoadmap*           pRoadmap;
        Paths               aPaths;
        PathId              nActivePath;
        StateDescriptions   aStateDescriptors;
        StateSet            aDisabledStates;
        bool                bActivePathIsDefinite;
           FixedLine*           pFixedLine;

        RoadmapWizardImpl()
            :pRoadmap( NULL )
            ,nActivePath( -1 )
            ,bActivePathIsDefinite( false )
            ,pFixedLine(NULL)
        {
        }

        ~RoadmapWizardImpl()
        {
            delete pRoadmap;
            delete pFixedLine;
        }

        /// returns the index of the current state in given path, or -1
        sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath );
        /// returns the index of the current state in the path with the given id, or -1
        sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId );
        /// returns the index of the first state in which the two given paths differ
        sal_Int32 getFirstDifferentIndex( const WizardPath& _rLHS, const WizardPath& _rRHS );
    };

    //--------------------------------------------------------------------
    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath )
    {
        sal_Int32 nStateIndexInPath = 0;
        WizardPath::const_iterator aPathLoop = _rPath.begin();
        for ( ; aPathLoop != _rPath.end(); ++aPathLoop, ++nStateIndexInPath )
            if ( *aPathLoop == _nState )
                break;
        if ( aPathLoop == _rPath.end() )
            nStateIndexInPath = -1;
        return nStateIndexInPath;
    }

    //--------------------------------------------------------------------
    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId )
    {
        sal_Int32 nStateIndexInPath = -1;
        Paths::const_iterator aPathPos = aPaths.find( _nPathId );
        if ( aPathPos != aPaths.end( ) )
            nStateIndexInPath = getStateIndexInPath( _nState, aPathPos->second );
        return nStateIndexInPath;
    }

    //--------------------------------------------------------------------
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

    //====================================================================
    //= RoadmapWizard
    //====================================================================
    DBG_NAME( RoadmapWizard )
    //--------------------------------------------------------------------
#ifdef DBG_UTIL
    const char* CheckInvariants( const void* pVoid )
    {
        return static_cast< const RoadmapWizard* >( pVoid )->checkInvariants();
    }

    //--------------------------------------------------------------------
    const sal_Char* RoadmapWizard::checkInvariants() const
    {
        // all paths have to start with the same state
        WizardState nSharedFirstState = WZS_INVALID_STATE;
        for ( Paths::const_iterator aPath = m_pImpl->aPaths.begin();
              aPath != m_pImpl->aPaths.end();
              ++aPath
            )
        {
            if ( aPath->second.empty() )
                return "RoadmapWizard::checkInvariants: paths should not be empty!";

            if ( nSharedFirstState == WZS_INVALID_STATE )
                // first path
                nSharedFirstState = aPath->second[ 0 ];
            else
                if ( nSharedFirstState != aPath->second[ 0 ] )
                    return "RoadmapWizard::checkInvariants: alls paths must start with the same state!";
        }

        if ( !m_pImpl->aPaths.empty() )
        {
            Paths::const_iterator aCurrentPathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
            if ( aCurrentPathPos == m_pImpl->aPaths.end() )
                return "RoadmapWizard::checkInvariants: invalid active path!";

            if ( -1 == m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath ) )
                return "RoadmapWizard::checkInvariants: the current state is not part of the current path!";
        }

        return NULL;
    }
#endif

    //--------------------------------------------------------------------
    RoadmapWizard::RoadmapWizard( Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags )
        :OWizardMachine( _pParent, _rRes, _nButtonFlags )
        ,m_pImpl( new RoadmapWizardImpl )
    {
        DBG_CTOR( RoadmapWizard, CheckInvariants );
        impl_construct();
    }

    //--------------------------------------------------------------------
    RoadmapWizard::RoadmapWizard( Window* _pParent, const WinBits i_nStyle, sal_uInt32 _nButtonFlags )
        :OWizardMachine( _pParent, i_nStyle, _nButtonFlags )
        ,m_pImpl( new RoadmapWizardImpl )
    {
        DBG_CTOR( RoadmapWizard, CheckInvariants );
        impl_construct();
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::impl_construct()
    {
        SetLeftAlignedButtonCount( 1 );
        SetEmptyViewMargin();

        m_pImpl->pRoadmap = new ORoadmap( this, WB_TABSTOP );
        m_pImpl->pRoadmap->SetText( SVT_RESSTR( STR_WIZDLG_ROADMAP_TITLE ) );
        m_pImpl->pRoadmap->SetPosPixel( Point( 0, 0 ) );
        m_pImpl->pRoadmap->SetItemSelectHdl( LINK( this, RoadmapWizard, OnRoadmapItemSelected ) );

        Size aRoadmapSize =( LogicToPixel( Size( 85, 0 ), MAP_APPFONT ) );
        aRoadmapSize.Height() = GetSizePixel().Height();
        m_pImpl->pRoadmap->SetSizePixel( aRoadmapSize );

        m_pImpl->pFixedLine = new FixedLine( this, WB_VERT );
        m_pImpl->pFixedLine->Show();
        m_pImpl->pFixedLine->SetPosPixel( Point( aRoadmapSize.Width() + 1, 0 ) );
        m_pImpl->pFixedLine->SetSizePixel( Size( LogicToPixel( Size( 2, 0 ) ).Width(), aRoadmapSize.Height() ) );

        SetViewWindow( m_pImpl->pRoadmap );
        SetViewAlign( WINDOWALIGN_LEFT );
        m_pImpl->pRoadmap->Show();
    }

    //--------------------------------------------------------------------
    RoadmapWizard::~RoadmapWizard()
    {
        delete m_pImpl;
        DBG_DTOR( RoadmapWizard, CheckInvariants );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::SetRoadmapHelpId( const OString& _rId )
    {
        m_pImpl->pRoadmap->SetHelpId( _rId );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::SetRoadmapInteractive( sal_Bool _bInteractive )
    {
        m_pImpl->pRoadmap->SetRoadmapInteractive( _bInteractive );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::declarePath( PathId _nPathId, const WizardPath& _lWizardStates)
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        m_pImpl->aPaths.insert( Paths::value_type( _nPathId, _lWizardStates ) );

        if ( m_pImpl->aPaths.size() == 1 )
            // the very first path -> activate it
            activatePath( _nPathId, false );
        else
            implUpdateRoadmap( );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::declarePath( PathId _nPathId, WizardState _nFirstState, ... )
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        DBG_ASSERT( _nFirstState != WZS_INVALID_STATE, "RoadmapWizard::declarePath: there should be at least one state in the path!" );
        if ( _nFirstState == WZS_INVALID_STATE )
            return;

        WizardPath aNewPath;

        // collect the elements of the path
        va_list aStateList;
        va_start( aStateList, _nFirstState );

        WizardState nState = _nFirstState;
        while ( nState != WZS_INVALID_STATE )
        {
            aNewPath.push_back( nState );
            nState = sal::static_int_cast< WizardState >(
                va_arg( aStateList, int ));
        }
        va_end( aStateList );

        DBG_ASSERT( _nFirstState == 0, "RoadmapWizard::declarePath: first state must be NULL." );
            // The WizardDialog (our very base class) always starts with a mnCurLevel == 0

        declarePath( _nPathId, aNewPath );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::describeState( WizardState _nState, const String& _rStateDisplayName, RoadmapPageFactory _pPageFactory )
    {
        OSL_ENSURE( m_pImpl->aStateDescriptors.find( _nState ) == m_pImpl->aStateDescriptors.end(),
            "RoadmapWizard::describeState: there already is a descriptor for this state!" );
        m_pImpl->aStateDescriptors[ _nState ] = StateDescriptions::mapped_type( _rStateDisplayName, _pPageFactory );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::activatePath( PathId _nPathId, bool _bDecideForIt )
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

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

        DBG_ASSERT( (sal_Int32)aNewPathPos->second.size() > nCurrentStatePathIndex,
            "RoadmapWizard::activate: you cannot activate a path which has less states than we've already advanced!" );
            // If this asserts, this for instance means that we are already in state number, say, 5
            // of our current path, and the caller tries to activate a path which has less than 5
            // states
        if ( (sal_Int32)aNewPathPos->second.size() <= nCurrentStatePathIndex )
            return;

        // assert that the current and the new path are equal, up to nCurrentStatePathIndex
        Paths::const_iterator aActivePathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
        if ( aActivePathPos != m_pImpl->aPaths.end() )
        {
            if ( m_pImpl->getFirstDifferentIndex( aActivePathPos->second, aNewPathPos->second ) <= nCurrentStatePathIndex )
            {
                OSL_FAIL( "RoadmapWizard::activate: you cannot activate a path which conflicts with the current one *before* the current state!" );
                return;
            }
        }

        m_pImpl->nActivePath = _nPathId;
        m_pImpl->bActivePathIsDefinite = _bDecideForIt;

        implUpdateRoadmap( );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::implUpdateRoadmap( )
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        DBG_ASSERT( m_pImpl->aPaths.find( m_pImpl->nActivePath ) != m_pImpl->aPaths.end(),
            "RoadmapWizard::implUpdateRoadmap: there is no such path!" );
        const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );

        sal_Int32 nCurrentStatePathIndex = m_pImpl->getStateIndexInPath( getCurrentState(), rActivePath );

        // determine up to which index (in the new path) we have to display the items
        RoadmapTypes::ItemIndex nUpperStepBoundary = (RoadmapTypes::ItemIndex)rActivePath.size();
        sal_Bool bIncompletePath = sal_False;
        if ( !m_pImpl->bActivePathIsDefinite )
        {
            for ( Paths::const_iterator aPathPos = m_pImpl->aPaths.begin();
                  aPathPos != m_pImpl->aPaths.end();
                  ++aPathPos
                )
            {
                if ( aPathPos->first == m_pImpl->nActivePath )
                    // it's the path we are just activating -> no need to check anything
                    continue;
                // the index from which on both paths differ
                sal_Int32 nDivergenceIndex = m_pImpl->getFirstDifferentIndex( rActivePath, aPathPos->second );
                if ( nDivergenceIndex <= nCurrentStatePathIndex )
                    // they differ in an index which we have already left behind us
                    // -> this is no conflict anymore
                    continue;

                // the path conflicts with our new path -> don't activate the
                // *complete* new path, but only up to the step which is unambiguous
                nUpperStepBoundary = nDivergenceIndex;
                bIncompletePath = sal_True;
            }
        }

        // can we advance from the current page?
        bool bCurrentPageCanAdvance = true;
        TabPage* pCurrentPage = GetPage( getCurrentState() );
        if ( pCurrentPage )
        {
            const IWizardPageController* pController = getPageController( GetPage( getCurrentState() ) );
            OSL_ENSURE( pController != NULL, "RoadmapWizard::implUpdateRoadmap: no controller for the current page!" );
            bCurrentPageCanAdvance = !pController || pController->canAdvance();
        }

        // now, we have to remove all items after nCurrentStatePathIndex, and insert the items from the active
        // path, up to (excluding) nUpperStepBoundary
        RoadmapTypes::ItemIndex nLoopUntil = ::std::max( (RoadmapTypes::ItemIndex)nUpperStepBoundary, m_pImpl->pRoadmap->GetItemCount() );
        for ( RoadmapTypes::ItemIndex nItemIndex = nCurrentStatePathIndex; nItemIndex < nLoopUntil; ++nItemIndex )
        {
            bool bExistentItem = ( nItemIndex < m_pImpl->pRoadmap->GetItemCount() );
            bool bNeedItem = ( nItemIndex < nUpperStepBoundary );

            bool bInsertItem = false;
            if ( bExistentItem )
            {
                if ( !bNeedItem )
                {
                    while ( nItemIndex < m_pImpl->pRoadmap->GetItemCount() )
                        m_pImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
                    break;
                }
                else
                {
                    // there is an item with this index in the roadmap - does it match what is requested by
                    // the respective state in the active path?
                    RoadmapTypes::ItemId nPresentItemId = m_pImpl->pRoadmap->GetItemID( nItemIndex );
                    WizardState nRequiredState = rActivePath[ nItemIndex ];
                    if ( nPresentItemId != nRequiredState )
                    {
                        m_pImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
                        bInsertItem = true;
                    }
                }
            }
            else
            {
                DBG_ASSERT( bNeedItem, "RoadmapWizard::implUpdateRoadmap: ehm - none needed, none present - why did the loop not terminate?" );
                bInsertItem = bNeedItem;
            }

            WizardState nState( rActivePath[ nItemIndex ] );
            if ( bInsertItem )
            {
                m_pImpl->pRoadmap->InsertRoadmapItem(
                    nItemIndex,
                    getStateDisplayName( nState ),
                    nState
                );
            }

            // if the item is *after* the current state, but the current page does not
            // allow advancing, the disable the state. This relieves derived classes
            // from disabling all future states just because the current state does not
            // (yet) allow advancing.
            const bool nUnconditionedDisable = !bCurrentPageCanAdvance && ( nItemIndex > nCurrentStatePathIndex );
            const bool bEnable = !nUnconditionedDisable && ( m_pImpl->aDisabledStates.find( nState ) == m_pImpl->aDisabledStates.end() );

            m_pImpl->pRoadmap->EnableRoadmapItem( m_pImpl->pRoadmap->GetItemID( nItemIndex ), bEnable );
        }

        m_pImpl->pRoadmap->SetRoadmapComplete( !bIncompletePath );
    }

    //--------------------------------------------------------------------
    WizardTypes::WizardState RoadmapWizard::determineNextState( WizardState _nCurrentState ) const
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        sal_Int32 nCurrentStatePathIndex = -1;

        Paths::const_iterator aActivePathPos = m_pImpl->aPaths.find( m_pImpl->nActivePath );
        if ( aActivePathPos != m_pImpl->aPaths.end() )
            nCurrentStatePathIndex = m_pImpl->getStateIndexInPath( _nCurrentState, aActivePathPos->second );

        DBG_ASSERT( nCurrentStatePathIndex != -1, "RoadmapWizard::determineNextState: ehm - how can we travel if there is no (valid) active path?" );
        if ( nCurrentStatePathIndex == -1 )
            return WZS_INVALID_STATE;

        sal_Int32 nNextStateIndex = nCurrentStatePathIndex + 1;

        while   (   ( nNextStateIndex < (sal_Int32)aActivePathPos->second.size() )
                &&  ( m_pImpl->aDisabledStates.find( aActivePathPos->second[ nNextStateIndex ] ) != m_pImpl->aDisabledStates.end() )
                )
        {
            ++nNextStateIndex;
        }

        if ( nNextStateIndex >= (sal_Int32)aActivePathPos->second.size() )
            // there is no next state in the current path (at least none which is enabled)
            return WZS_INVALID_STATE;

        return aActivePathPos->second[ nNextStateIndex ];
    }

    //---------------------------------------------------------------------
    bool RoadmapWizard::canAdvance() const
    {
        if ( !m_pImpl->bActivePathIsDefinite )
        {
            // check how many paths are still allowed
            const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );
            sal_Int32 nCurrentStatePathIndex = m_pImpl->getStateIndexInPath( getCurrentState(), rActivePath );

            size_t nPossiblePaths(0);
            for (   Paths::const_iterator aPathPos = m_pImpl->aPaths.begin();
                    aPathPos != m_pImpl->aPaths.end();
                    ++aPathPos
                )
            {
                // the index from which on both paths differ
                sal_Int32 nDivergenceIndex = m_pImpl->getFirstDifferentIndex( rActivePath, aPathPos->second );

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
        if ( *rPath.rbegin() == getCurrentState() )
            return false;

        return true;
    }

    //---------------------------------------------------------------------
    void RoadmapWizard::updateTravelUI()
    {
        OWizardMachine::updateTravelUI();

        // disable the "Previous" button if all states in our history are disabled
        ::std::vector< WizardState > aHistory;
        getStateHistory( aHistory );
        bool bHaveEnabledState = false;
        for (   ::std::vector< WizardState >::const_iterator state = aHistory.begin();
                state != aHistory.end() && !bHaveEnabledState;
                ++state
            )
        {
            if ( isStateEnabled( *state ) )
                bHaveEnabledState = true;
        }

        enableButtons( WZB_PREVIOUS, bHaveEnabledState );

        implUpdateRoadmap();
    }

    //--------------------------------------------------------------------
    IMPL_LINK_NOARG(RoadmapWizard, OnRoadmapItemSelected)
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        RoadmapTypes::ItemId nCurItemId = m_pImpl->pRoadmap->GetCurrentRoadmapItemID();
        if ( nCurItemId == getCurrentState() )
            // nothing to do
            return 1L;

        if ( isTravelingSuspended() )
            return 0;

        WizardTravelSuspension aTravelGuard( *this );

        sal_Int32 nCurrentIndex = m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath );
        sal_Int32 nNewIndex     = m_pImpl->getStateIndexInPath( nCurItemId, m_pImpl->nActivePath );

        DBG_ASSERT( ( nCurrentIndex != -1 ) && ( nNewIndex != -1 ),
            "RoadmapWizard::OnRoadmapItemSelected: something's wrong here!" );
        if ( ( nCurrentIndex == -1 ) || ( nNewIndex == -1 ) )
        {
            return 0L;
        }

        sal_Bool bResult = sal_True;
        if ( nNewIndex > nCurrentIndex )
        {
            bResult = skipUntil( (WizardState)nCurItemId );
            WizardState nTemp = (WizardState)nCurItemId;
            while( nTemp )
            {
                if( m_pImpl->aDisabledStates.find( --nTemp ) != m_pImpl->aDisabledStates.end() )
                    removePageFromHistory( nTemp );
            }
        }
        else
            bResult = skipBackwardUntil( (WizardState)nCurItemId );

        if ( !bResult )
            m_pImpl->pRoadmap->SelectRoadmapItemByID( getCurrentState() );

        return 1L;
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::enterState( WizardState _nState )
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        OWizardMachine::enterState( _nState );

        // synchronize the roadmap
        implUpdateRoadmap( );
        m_pImpl->pRoadmap->SelectRoadmapItemByID( getCurrentState() );
    }

    //--------------------------------------------------------------------
    String RoadmapWizard::getStateDisplayName( WizardState _nState ) const
    {
        String sDisplayName;

        StateDescriptions::const_iterator pos = m_pImpl->aStateDescriptors.find( _nState );
        OSL_ENSURE( pos != m_pImpl->aStateDescriptors.end(),
            "RoadmapWizard::getStateDisplayName: no default implementation available for this state!" );
        if ( pos != m_pImpl->aStateDescriptors.end() )
            sDisplayName = pos->second.first;

        return sDisplayName;
    }

    //--------------------------------------------------------------------
    TabPage* RoadmapWizard::createPage( WizardState _nState )
    {
        TabPage* pPage( NULL );

        StateDescriptions::const_iterator pos = m_pImpl->aStateDescriptors.find( _nState );
        OSL_ENSURE( pos != m_pImpl->aStateDescriptors.end(),
            "RoadmapWizard::createPage: no default implementation available for this state!" );
        if ( pos != m_pImpl->aStateDescriptors.end() )
        {
            RoadmapPageFactory pFactory = pos->second.second;
            pPage = (*pFactory)( *this );
        }

        return pPage;
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::enableState( WizardState _nState, bool _bEnable )
    {
        DBG_CHKTHIS( RoadmapWizard, CheckInvariants );

        // remember this (in case the state appears in the roadmap later on)
        if ( _bEnable )
            m_pImpl->aDisabledStates.erase( _nState );
        else
        {
            m_pImpl->aDisabledStates.insert( _nState );
            removePageFromHistory( _nState );
        }

        // if the state is currently in the roadmap, reflect it's new status
        m_pImpl->pRoadmap->EnableRoadmapItem( (RoadmapTypes::ItemId)_nState, _bEnable );
    }

    //--------------------------------------------------------------------
    bool RoadmapWizard::knowsState( WizardState i_nState ) const
    {
        for (   Paths::const_iterator path = m_pImpl->aPaths.begin();
                path != m_pImpl->aPaths.end();
                ++path
            )
        {
            for (   WizardPath::const_iterator state = path->second.begin();
                    state != path->second.end();
                    ++state
                )
            {
                if ( *state == i_nState )
                    return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool RoadmapWizard::isStateEnabled( WizardState _nState ) const
    {
        return m_pImpl->aDisabledStates.find( _nState ) == m_pImpl->aDisabledStates.end();
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::Resize()
    {
        OWizardMachine::Resize();

        if ( IsReallyShown() && !IsInInitShow() )
            ResizeFixedLine();
    }


    //--------------------------------------------------------------------
    void RoadmapWizard::StateChanged( StateChangedType nType )
    {
        WizardDialog::StateChanged( nType );

        if ( nType == STATE_CHANGE_INITSHOW )
            ResizeFixedLine();
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::ResizeFixedLine()
    {
        Size aSize( m_pImpl->pRoadmap->GetSizePixel() );
        aSize.Width() = m_pImpl->pFixedLine->GetSizePixel().Width();
        m_pImpl->pFixedLine->SetSizePixel( aSize );
    }

    //--------------------------------------------------------------------
    void RoadmapWizard::updateRoadmapItemLabel( WizardState _nState )
    {
        const WizardPath& rActivePath( m_pImpl->aPaths[ m_pImpl->nActivePath ] );
        RoadmapTypes::ItemIndex nUpperStepBoundary = (RoadmapTypes::ItemIndex)rActivePath.size();
        RoadmapTypes::ItemIndex nLoopUntil = ::std::max( (RoadmapTypes::ItemIndex)nUpperStepBoundary, m_pImpl->pRoadmap->GetItemCount() );
        sal_Int32 nCurrentStatePathIndex = -1;
        if ( m_pImpl->nActivePath != -1 )
            nCurrentStatePathIndex = m_pImpl->getStateIndexInPath( getCurrentState(), m_pImpl->nActivePath );
        for ( RoadmapTypes::ItemIndex nItemIndex = nCurrentStatePathIndex; nItemIndex < nLoopUntil; ++nItemIndex )
        {
            bool bExistentItem = ( nItemIndex < m_pImpl->pRoadmap->GetItemCount() );
            if ( bExistentItem )
            {
                // there is an item with this index in the roadmap - does it match what is requested by
                // the respective state in the active path?
                RoadmapTypes::ItemId nPresentItemId = m_pImpl->pRoadmap->GetItemID( nItemIndex );
                WizardState nRequiredState = rActivePath[ nItemIndex ];
                if ( _nState == nRequiredState )
                {
                    m_pImpl->pRoadmap->ChangeRoadmapItemLabel( nPresentItemId, getStateDisplayName( nRequiredState ) );
                    break;
                }
            }
        }
    }

//........................................................................
}   // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
