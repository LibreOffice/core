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

#include <comphelper/string.hxx>

#include <conflictsdlg.hxx>
#include <strings.hrc>
#include <scresid.hxx>
#include <viewdata.hxx>
#include <dbfunc.hxx>
#include <chgtrack.hxx>

// struct ScConflictsListEntry

bool ScConflictsListEntry::HasSharedAction( sal_uLong nSharedAction ) const
{
    auto aEnd = maSharedActions.cend();
    auto aItr = std::find(maSharedActions.cbegin(), aEnd, nSharedAction);

    return aItr != aEnd;
}

bool ScConflictsListEntry::HasOwnAction( sal_uLong nOwnAction ) const
{
    auto aEnd = maOwnActions.cend();
    auto aItr = std::find(maOwnActions.cbegin(), aEnd, nOwnAction);

    return aItr != aEnd;
}

// class ScConflictsListHelper

bool ScConflictsListHelper::HasOwnAction( ScConflictsList& rConflictsList, sal_uLong nOwnAction )
{
    return std::any_of(rConflictsList.begin(), rConflictsList.end(),
                       [nOwnAction](ScConflictsListEntry& rConflict) { return rConflict.HasOwnAction( nOwnAction ); });
}

ScConflictsListEntry* ScConflictsListHelper::GetSharedActionEntry( ScConflictsList& rConflictsList, sal_uLong nSharedAction )
{
    auto aEnd = rConflictsList.end();
    auto aItr = std::find_if(rConflictsList.begin(), aEnd,
        [nSharedAction](ScConflictsListEntry& rConflict) { return rConflict.HasSharedAction( nSharedAction ); });

    if (aItr != aEnd)
        return &(*aItr);

    return nullptr;
}

ScConflictsListEntry* ScConflictsListHelper::GetOwnActionEntry( ScConflictsList& rConflictsList, sal_uLong nOwnAction )
{
    auto aEnd = rConflictsList.end();
    auto aItr = std::find_if(rConflictsList.begin(), aEnd,
        [nOwnAction](ScConflictsListEntry& rConflict) { return rConflict.HasOwnAction( nOwnAction ); });

    if (aItr != aEnd)
        return &(*aItr);

    return nullptr;
}

void ScConflictsListHelper::Transform_Impl( std::vector<sal_uLong>& rActionList, ScChangeActionMergeMap* pMergeMap )
{
    if ( !pMergeMap )
    {
        return;
    }

    for ( auto aItr = rActionList.begin(); aItr != rActionList.end(); )
    {
        ScChangeActionMergeMap::iterator aItrMap = pMergeMap->find( *aItr );
        if ( aItrMap != pMergeMap->end() )
        {
            *aItr = aItrMap->second;
            ++aItr;
        }
        else
        {
            aItr = rActionList.erase( aItr );
            OSL_FAIL( "ScConflictsListHelper::Transform_Impl: erased action from conflicts list!" );
        }
    }
}

void ScConflictsListHelper::TransformConflictsList( ScConflictsList& rConflictsList,
    ScChangeActionMergeMap* pSharedMap, ScChangeActionMergeMap* pOwnMap )
{
    for ( auto& rConflictEntry : rConflictsList )
    {
        if ( pSharedMap )
        {
            ScConflictsListHelper::Transform_Impl( rConflictEntry.maSharedActions, pSharedMap );
        }

        if ( pOwnMap )
        {
            ScConflictsListHelper::Transform_Impl( rConflictEntry.maOwnActions, pOwnMap );
        }
    }
}

// class ScConflictsFinder

ScConflictsFinder::ScConflictsFinder( ScChangeTrack* pTrack, sal_uLong nStartShared, sal_uLong nEndShared,
        sal_uLong nStartOwn, sal_uLong nEndOwn, ScConflictsList& rConflictsList )
    :mpTrack( pTrack )
    ,mnStartShared( nStartShared )
    ,mnEndShared( nEndShared )
    ,mnStartOwn( nStartOwn )
    ,mnEndOwn( nEndOwn )
    ,mrConflictsList( rConflictsList )
{
}

ScConflictsFinder::~ScConflictsFinder()
{
}

bool ScConflictsFinder::DoActionsIntersect( const ScChangeAction* pAction1, const ScChangeAction* pAction2 )
{
    return pAction1 && pAction2 && pAction1->GetBigRange().Intersects( pAction2->GetBigRange() );
}

ScConflictsListEntry* ScConflictsFinder::GetIntersectingEntry( const ScChangeAction* pAction ) const
{
    auto doActionsIntersect = [this, pAction](const sal_uLong& aAction) { return DoActionsIntersect( mpTrack->GetAction( aAction ), pAction ); };

    for ( auto& rConflict : mrConflictsList )
    {
        if (std::any_of( rConflict.maSharedActions.cbegin(), rConflict.maSharedActions.cend(), doActionsIntersect ))
            return &rConflict;

        if (std::any_of( rConflict.maOwnActions.cbegin(), rConflict.maOwnActions.cend(), doActionsIntersect ))
            return &rConflict;
    }

    return nullptr;
}

ScConflictsListEntry& ScConflictsFinder::GetEntry( sal_uLong nSharedAction, const std::vector<sal_uLong>& rOwnActions )
{
    // try to get a list entry which already contains the shared action
    ScConflictsListEntry* pEntry = ScConflictsListHelper::GetSharedActionEntry( mrConflictsList, nSharedAction );
    if ( pEntry )
    {
        return *pEntry;
    }

    // try to get a list entry for which the shared action intersects with any
    // other action of this entry
    pEntry = GetIntersectingEntry( mpTrack->GetAction( nSharedAction ) );
    if ( pEntry )
    {
        pEntry->maSharedActions.push_back( nSharedAction );
        return *pEntry;
    }

    // try to get a list entry for which any of the own actions intersects with
    // any other action of this entry
    for ( auto& rOwnAction : rOwnActions )
    {
        pEntry = GetIntersectingEntry( mpTrack->GetAction( rOwnAction ) );
        if ( pEntry )
        {
            pEntry->maSharedActions.push_back( nSharedAction );
            return *pEntry;
        }
    }

    // if no entry was found, create a new one
    ScConflictsListEntry aEntry;
    aEntry.meConflictAction = SC_CONFLICT_ACTION_NONE;
    aEntry.maSharedActions.push_back( nSharedAction );
    mrConflictsList.push_back( aEntry );
    return mrConflictsList.back();
}

bool ScConflictsFinder::Find()
{
    if ( !mpTrack )
    {
        return false;
    }

    bool bReturn = false;
    ScChangeAction* pSharedAction = mpTrack->GetAction( mnStartShared );
    while ( pSharedAction && pSharedAction->GetActionNumber() <= mnEndShared )
    {
        std::vector<sal_uLong> aOwnActions;
        ScChangeAction* pOwnAction = mpTrack->GetAction( mnStartOwn );
        while ( pOwnAction && pOwnAction->GetActionNumber() <= mnEndOwn )
        {
            if ( DoActionsIntersect( pSharedAction, pOwnAction ) )
            {
                aOwnActions.push_back( pOwnAction->GetActionNumber() );
            }
            pOwnAction = pOwnAction->GetNext();
        }

        if ( !aOwnActions.empty() )
        {
            ScConflictsListEntry& rEntry = GetEntry(pSharedAction->GetActionNumber(), aOwnActions);
            for ( auto& aOwnAction : aOwnActions )
            {
                if (!ScConflictsListHelper::HasOwnAction(mrConflictsList, aOwnAction))
                {
                    rEntry.maOwnActions.push_back(aOwnAction);
                }
            }
            bReturn = true;
        }

        pSharedAction = pSharedAction->GetNext();
    }

    return bReturn;
}

// class ScConflictsResolver

ScConflictsResolver::ScConflictsResolver( ScChangeTrack* pTrack, ScConflictsList& rConflictsList )
    :mpTrack ( pTrack )
    ,mrConflictsList ( rConflictsList )
{
    OSL_ENSURE( mpTrack, "ScConflictsResolver CTOR: mpTrack is null!" );
}

ScConflictsResolver::~ScConflictsResolver()
{
}

void ScConflictsResolver::HandleAction( ScChangeAction* pAction, bool bIsSharedAction,
    bool bHandleContentAction, bool bHandleNonContentAction )
{
    if ( !mpTrack || !pAction )
    {
        return;
    }

    if ( bIsSharedAction )
    {
        ScConflictsListEntry* pConflictEntry = ScConflictsListHelper::GetSharedActionEntry(
            mrConflictsList, pAction->GetActionNumber() );
        if ( pConflictEntry )
        {
            ScConflictAction eConflictAction = pConflictEntry->meConflictAction;
            if ( eConflictAction == SC_CONFLICT_ACTION_KEEP_MINE )
            {
                if ( pAction->GetType() == SC_CAT_CONTENT )
                {
                    if ( bHandleContentAction )
                    {
                        mpTrack->Reject( pAction );
                    }
                }
                else
                {
                    if ( bHandleNonContentAction )
                    {
                        mpTrack->Reject( pAction );
                    }
                }
            }
        }
    }
    else
    {
        ScConflictsListEntry* pConflictEntry = ScConflictsListHelper::GetOwnActionEntry(
            mrConflictsList, pAction->GetActionNumber() );
        if ( pConflictEntry )
        {
            ScConflictAction eConflictAction = pConflictEntry->meConflictAction;
            if ( eConflictAction == SC_CONFLICT_ACTION_KEEP_MINE )
            {
                if ( pAction->GetType() == SC_CAT_CONTENT )
                {
                    if ( bHandleContentAction )
                    {
                        // do nothing
                        //mpTrack->SelectContent( pAction );
                    }
                }
                else
                {
                    if ( bHandleNonContentAction )
                    {
                        // do nothing
                        //mpTrack->Accept( pAction );
                    }
                }
            }
            else if ( eConflictAction == SC_CONFLICT_ACTION_KEEP_OTHER )
            {
                if ( pAction->GetType() == SC_CAT_CONTENT )
                {
                    if ( bHandleContentAction )
                    {
                        mpTrack->Reject( pAction );
                    }
                }
                else
                {
                    if ( bHandleNonContentAction )
                    {
                        mpTrack->Reject( pAction );
                    }
                }
            }
        }
    }
}

// class ScConflictsDlg

ScConflictsDlg::ScConflictsDlg( vcl::Window* pParent, ScViewData* pViewData, ScDocument* pSharedDoc, ScConflictsList& rConflictsList )
    :ModalDialog( pParent, "ConflictsDialog", "modules/scalc/ui/conflictsdialog.ui" )
    ,m_pLbConflictsContainer  ( get<SvSimpleTableContainer>("container") )
    ,m_pLbConflicts     ( VclPtr<SvxRedlinTable>::Create(*m_pLbConflictsContainer) )
    ,maStrTitleConflict ( ScResId( STR_TITLE_CONFLICT ) )
    ,maStrUnknownUser   ( ScResId( STR_UNKNOWN_USER_CONFLICT ) )
    ,mpViewData         ( pViewData )
    ,mpOwnDoc           ( nullptr )
    ,mpOwnTrack         ( nullptr )
    ,mpSharedDoc        ( pSharedDoc )
    ,mpSharedTrack      ( nullptr )
    ,mrConflictsList    ( rConflictsList )
    ,maSelectionIdle    ( "ScConflictsDlg SelectionIdle" )
    ,mbInSelectHdl      ( false )
    ,mbInDeselectHdl    ( false )
{
    get(m_pBtnKeepMine, "keepmine");
    get(m_pBtnKeepOther, "keepother");
    get(m_pBtnKeepAllMine, "keepallmine");
    get(m_pBtnKeepAllOthers, "keepallothers");

    OSL_ENSURE( mpViewData, "ScConflictsDlg CTOR: mpViewData is null!" );
    mpOwnDoc = ( mpViewData ? mpViewData->GetDocument() : nullptr );
    OSL_ENSURE( mpOwnDoc, "ScConflictsDlg CTOR: mpOwnDoc is null!" );
    mpOwnTrack = ( mpOwnDoc ? mpOwnDoc->GetChangeTrack() : nullptr );
    OSL_ENSURE( mpOwnTrack, "ScConflictsDlg CTOR: mpOwnTrack is null!" );
    OSL_ENSURE( mpSharedDoc, "ScConflictsDlg CTOR: mpSharedDoc is null!" );
    mpSharedTrack = ( mpSharedDoc ? mpSharedDoc->GetChangeTrack() : nullptr );
    OSL_ENSURE( mpSharedTrack, "ScConflictsDlg CTOR: mpSharedTrack is null!" );

    SetMinOutputSizePixel( GetSizePixel() );

    long const nTabs[] = { 10, 216, 266 };
    m_pLbConflicts->SetTabs( SAL_N_ELEMENTS(nTabs), nTabs );

    OUString aTab('\t');
    OUString aHeader( maStrTitleConflict );
    aHeader += aTab;
    aHeader += ScResId( STR_TITLE_AUTHOR );
    aHeader += aTab;
    aHeader += ScResId( STR_TITLE_DATE );
    m_pLbConflicts->InsertHeaderEntry( aHeader, HEADERBAR_APPEND, HeaderBarItemBits::LEFT | HeaderBarItemBits::LEFTIMAGE );

    m_pLbConflicts->SetStyle( m_pLbConflicts->GetStyle() | WB_HASLINES | WB_CLIPCHILDREN | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL );
    m_pLbConflicts->SetSelectionMode( SelectionMode::Multiple );
    m_pLbConflicts->SetHighlightRange();

    maSelectionIdle.SetInvokeHandler( LINK( this, ScConflictsDlg, UpdateSelectionHdl ) );
    maSelectionIdle.SetDebugName( "ScConflictsDlg maSelectionIdle" );

    m_pLbConflicts->SetSelectHdl( LINK( this, ScConflictsDlg, SelectHandle ) );
    m_pLbConflicts->SetDeselectHdl( LINK( this, ScConflictsDlg, DeselectHandle ) );

    m_pBtnKeepMine->SetClickHdl( LINK( this, ScConflictsDlg, KeepMineHandle ) );
    m_pBtnKeepOther->SetClickHdl( LINK( this, ScConflictsDlg, KeepOtherHandle ) );
    m_pBtnKeepAllMine->SetClickHdl( LINK( this, ScConflictsDlg, KeepAllMineHandle ) );
    m_pBtnKeepAllOthers->SetClickHdl( LINK( this, ScConflictsDlg, KeepAllOthersHandle ) );

    UpdateView();

    SvTreeListEntry* pEntry = m_pLbConflicts->First();
    if ( pEntry != nullptr )
    {
        m_pLbConflicts->Select( pEntry );
    }
}

ScConflictsDlg::~ScConflictsDlg()
{
    disposeOnce();
}

void ScConflictsDlg::dispose()
{
    m_pLbConflictsContainer.clear();
    m_pLbConflicts.disposeAndClear();
    m_pBtnKeepMine.clear();
    m_pBtnKeepOther.clear();
    m_pBtnKeepAllMine.clear();
    m_pBtnKeepAllOthers.clear();
    ModalDialog::dispose();
}

OUString ScConflictsDlg::GetConflictString( const ScConflictsListEntry& rConflictEntry )
{
    OUString aString;
    if ( mpOwnTrack )
    {
        const ScChangeAction* pAction = mpOwnTrack->GetAction( rConflictEntry.maOwnActions[ 0 ] );
        if ( pAction && mpOwnDoc )
        {
            SCTAB nTab = pAction->GetBigRange().MakeRange().aStart.Tab();
            mpOwnDoc->GetName( nTab, aString );
        }
    }
    return aString;
}

OUString ScConflictsDlg::GetActionString( const ScChangeAction* pAction, ScDocument* pDoc )
{
    OUString aString;

    OSL_ENSURE( pAction, "ScConflictsDlg::GetActionString(): pAction is null!" );
    OSL_ENSURE( pDoc, "ScConflictsDlg::GetActionString(): pDoc is null!" );
    if ( pAction && pDoc )
    {
        OUString aDesc;
        pAction->GetDescription(aDesc, pDoc, true, false);
        aString += aDesc;
        aString += "\t";

        OUString aUser = comphelper::string::strip(pAction->GetUser(), ' ');
        if ( aUser.isEmpty() )
        {
            aUser = maStrUnknownUser;
        }
        aString += aUser;
        aString += "\t";

        DateTime aDateTime = pAction->GetDateTime();
        aString += ScGlobal::pLocaleData->getDate( aDateTime );
        aString += " ";
        aString += ScGlobal::pLocaleData->getTime( aDateTime, false );
        aString += "\t";
    }

    return aString;
}

void ScConflictsDlg::HandleListBoxSelection( bool bSelectHandle )
{
    SvTreeListEntry* pSelEntry = m_pLbConflicts->GetCurEntry();
    if ( !pSelEntry )
    {
        pSelEntry = m_pLbConflicts->FirstSelected();
    }
    if ( !pSelEntry )
    {
        return;
    }

    SvTreeListEntry* pRootEntry = m_pLbConflicts->GetRootLevelParent( pSelEntry );
    if ( pRootEntry )
    {
        if ( bSelectHandle )
        {
            m_pLbConflicts->SelectAll( false );
        }
        if ( !m_pLbConflicts->IsSelected( pRootEntry ) )
        {
            m_pLbConflicts->Select( pRootEntry );
        }
        SvTreeListEntry* pEntry = m_pLbConflicts->FirstChild( pRootEntry );
        while ( pEntry )
        {
            if ( !m_pLbConflicts->IsSelected( pEntry ) )
            {
                m_pLbConflicts->Select( pEntry );
            }
            pEntry = pEntry->NextSibling();
        }
    }
}

IMPL_LINK_NOARG(ScConflictsDlg, SelectHandle, SvTreeListBox*, void)
{
    if ( mbInSelectHdl || mbInDeselectHdl )
    {
        return;
    }

    mbInSelectHdl = true;
    HandleListBoxSelection( true );
    maSelectionIdle.Start();
    mbInSelectHdl = false;
}

IMPL_LINK_NOARG(ScConflictsDlg, DeselectHandle, SvTreeListBox*, void)
{
    if ( mbInDeselectHdl || mbInSelectHdl )
    {
        return;
    }

    mbInDeselectHdl = true;
    HandleListBoxSelection( false );
    mbInDeselectHdl = false;
}

IMPL_LINK_NOARG(ScConflictsDlg, UpdateSelectionHdl, Timer *, void)
{
    if ( !mpViewData || !mpOwnDoc )
    {
        return;
    }

    ScTabView* pTabView = mpViewData->GetView();
    pTabView->DoneBlockMode();
    bool bContMark = false;
    SvTreeListEntry* pEntry = m_pLbConflicts->FirstSelected();
    while ( pEntry )
    {
        if ( pEntry != m_pLbConflicts->GetRootLevelParent( pEntry ) )
        {
            RedlinData* pUserData = static_cast< RedlinData* >( pEntry->GetUserData() );
            if  ( pUserData )
            {
                ScChangeAction* pAction = static_cast< ScChangeAction* >( pUserData->pData );
                if ( pAction && ( pAction->GetType() != SC_CAT_DELETE_TABS ) &&
                     ( pAction->IsClickable() || pAction->IsVisible() ) )
                {
                    const ScBigRange& rBigRange = static_cast< const ScChangeAction* >( pAction )->GetBigRange();
                    if ( rBigRange.IsValid( mpOwnDoc ) )
                    {
                        bool bSetCursor = !m_pLbConflicts->NextSelected( pEntry );
                        pTabView->MarkRange( rBigRange.MakeRange(), bSetCursor, bContMark );
                        bContMark = true;
                    }
                }
            }
        }
        pEntry = m_pLbConflicts->NextSelected( pEntry );
    }
}

void ScConflictsDlg::SetConflictAction( const SvTreeListEntry* pRootEntry, ScConflictAction eConflictAction )
{
    RedlinData* pUserData = static_cast< RedlinData* >( pRootEntry ? pRootEntry->GetUserData() : nullptr );
    ScConflictsListEntry* pConflictEntry = static_cast< ScConflictsListEntry* >( pUserData ? pUserData->pData : nullptr );
    if ( pConflictEntry )
    {
        pConflictEntry->meConflictAction = eConflictAction;
    }
}

void ScConflictsDlg::KeepHandler( bool bMine )
{
    SvTreeListEntry* pEntry = m_pLbConflicts->FirstSelected();
    SvTreeListEntry* pRootEntry = ( pEntry ? m_pLbConflicts->GetRootLevelParent( pEntry ) : nullptr );
    if ( !pRootEntry )
    {
        return;
    }
    SetPointer( PointerStyle::Wait );
    ScConflictAction eConflictAction = ( bMine ? SC_CONFLICT_ACTION_KEEP_MINE : SC_CONFLICT_ACTION_KEEP_OTHER );
    SetConflictAction( pRootEntry, eConflictAction );
    m_pLbConflicts->RemoveEntry( pRootEntry );
    SetPointer( PointerStyle::Arrow );
    if ( m_pLbConflicts->GetEntryCount() == 0 )
    {
        EndDialog( RET_OK );
    }
}

void ScConflictsDlg::KeepAllHandler( bool bMine )
{
    SvTreeListEntry* pEntry = m_pLbConflicts->First();
    SvTreeListEntry* pRootEntry = ( pEntry ? m_pLbConflicts->GetRootLevelParent( pEntry ) : nullptr );
    if ( !pRootEntry )
    {
        return;
    }
    SetPointer( PointerStyle::Wait );
    ScConflictAction eConflictAction = ( bMine ? SC_CONFLICT_ACTION_KEEP_MINE : SC_CONFLICT_ACTION_KEEP_OTHER );
    while ( pRootEntry )
    {
        SetConflictAction( pRootEntry, eConflictAction );
        pRootEntry = pRootEntry->NextSibling();
    }
    m_pLbConflicts->SetUpdateMode( false );
    m_pLbConflicts->Clear();
    m_pLbConflicts->SetUpdateMode( true );
    SetPointer( PointerStyle::Arrow );
    EndDialog( RET_OK );
}

IMPL_LINK_NOARG(ScConflictsDlg, KeepMineHandle, Button*, void)
{
    KeepHandler( true );
}

IMPL_LINK_NOARG(ScConflictsDlg, KeepOtherHandle, Button*, void)
{
    KeepHandler( false );
}

IMPL_LINK_NOARG(ScConflictsDlg, KeepAllMineHandle, Button*, void)
{
    KeepAllHandler( true );
}

IMPL_LINK_NOARG(ScConflictsDlg, KeepAllOthersHandle, Button*, void)
{
    KeepAllHandler( false );
}

void ScConflictsDlg::UpdateView()
{
    for ( ScConflictsListEntry& rConflictEntry : mrConflictsList )
    {
        if (rConflictEntry.meConflictAction == SC_CONFLICT_ACTION_NONE)
        {
            std::unique_ptr<RedlinData> pRootUserData(new RedlinData());
            pRootUserData->pData = static_cast<void*>(&rConflictEntry);
            SvTreeListEntry* pRootEntry = m_pLbConflicts->InsertEntry( GetConflictString( rConflictEntry ), std::move(pRootUserData) );

            for ( auto& aSharedAction : rConflictEntry.maSharedActions )
            {
                ScChangeAction* pAction = mpSharedTrack ? mpSharedTrack->GetAction(aSharedAction) : nullptr;
                if ( pAction )
                {
                    // only display shared top content entries
                    if ( pAction->GetType() == SC_CAT_CONTENT )
                    {
                        ScChangeActionContent* pNextContent = dynamic_cast<ScChangeActionContent&>(*pAction).GetNextContent();
                        if ( pNextContent && rConflictEntry.HasSharedAction( pNextContent->GetActionNumber() ) )
                        {
                            continue;
                        }
                    }

                    OUString aString( GetActionString( pAction, mpSharedDoc ) );
                    m_pLbConflicts->InsertEntry( aString, std::unique_ptr<RedlinData>(), pRootEntry );
                }
            }

            for ( auto& aOwnAction : rConflictEntry.maOwnActions )
            {
                ScChangeAction* pAction = mpOwnTrack ? mpOwnTrack->GetAction(aOwnAction) : nullptr;
                if ( pAction )
                {
                    // only display own top content entries
                    if ( pAction->GetType() == SC_CAT_CONTENT )
                    {
                        ScChangeActionContent* pNextContent = dynamic_cast<ScChangeActionContent&>(*pAction).GetNextContent();
                        if ( pNextContent && rConflictEntry.HasOwnAction( pNextContent->GetActionNumber() ) )
                        {
                            continue;
                        }
                    }

                    OUString aString( GetActionString( pAction, mpOwnDoc ) );
                    std::unique_ptr<RedlinData> pUserData(new RedlinData());
                    pUserData->pData = static_cast< void* >( pAction );
                    m_pLbConflicts->InsertEntry( aString, std::move(pUserData), pRootEntry );
                }
            }

            m_pLbConflicts->Expand( pRootEntry );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
