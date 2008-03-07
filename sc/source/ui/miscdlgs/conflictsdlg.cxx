/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conflictsdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:22:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

//-----------------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "conflictsdlg.hxx"
#include "conflictsdlg.hrc"
#include "scresid.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "tabview.hxx"


//=============================================================================
// struct ScConflictsListEntry
//=============================================================================

bool ScConflictsListEntry::HasSharedAction( ULONG nSharedAction ) const
{
    ScChangeActionList::const_iterator aEnd = maSharedActions.end();
    for ( ScChangeActionList::const_iterator aItr = maSharedActions.begin(); aItr != aEnd; ++aItr )
    {
        if ( *aItr == nSharedAction )
        {
            return true;
        }
    }

    return false;
}

bool ScConflictsListEntry::HasOwnAction( ULONG nOwnAction ) const
{
    ScChangeActionList::const_iterator aEnd = maOwnActions.end();
    for ( ScChangeActionList::const_iterator aItr = maOwnActions.begin(); aItr != aEnd; ++aItr )
    {
        if ( *aItr == nOwnAction )
        {
            return true;
        }
    }

    return false;
}


//=============================================================================
// class ScConflictsListHelper
//=============================================================================

bool ScConflictsListHelper::HasSharedAction( ScConflictsList& rConflictsList, ULONG nSharedAction )
{
    ScConflictsList::const_iterator aEnd = rConflictsList.end();
    for ( ScConflictsList::const_iterator aItr = rConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        if ( aItr->HasSharedAction( nSharedAction ) )
        {
            return true;
        }
    }

    return false;
}

bool ScConflictsListHelper::HasOwnAction( ScConflictsList& rConflictsList, ULONG nOwnAction )
{
    ScConflictsList::const_iterator aEnd = rConflictsList.end();
    for ( ScConflictsList::const_iterator aItr = rConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        if ( aItr->HasOwnAction( nOwnAction ) )
        {
            return true;
        }
    }

    return false;
}

ScConflictsListEntry* ScConflictsListHelper::GetSharedActionEntry( ScConflictsList& rConflictsList, ULONG nSharedAction )
{
    ScConflictsList::iterator aEnd = rConflictsList.end();
    for ( ScConflictsList::iterator aItr = rConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        if ( aItr->HasSharedAction( nSharedAction ) )
        {
            return &(*aItr);
        }
    }

    return NULL;
}

ScConflictsListEntry* ScConflictsListHelper::GetOwnActionEntry( ScConflictsList& rConflictsList, ULONG nOwnAction )
{
    ScConflictsList::iterator aEnd = rConflictsList.end();
    for ( ScConflictsList::iterator aItr = rConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        if ( aItr->HasOwnAction( nOwnAction ) )
        {
            return &(*aItr);
        }
    }

    return NULL;
}


//=============================================================================
// class ScConflictsFinder
//=============================================================================

ScConflictsFinder::ScConflictsFinder( ScChangeTrack* pSharedTrack, ULONG nStartShared, ULONG nEndShared,
        ScChangeTrack* pOwnTrack, ULONG nStartOwn, ULONG nEndOwn,
        ScConflictsList& rConflictsList )
    :mpSharedTrack( pSharedTrack )
    ,mnStartShared( nStartShared )
    ,mnEndShared( nEndShared )
    ,mpOwnTrack( pOwnTrack )
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
    if ( pAction1 && pAction2 && pAction1->GetBigRange().Intersects( pAction2->GetBigRange() ) )
    {
        return true;
    }
    return false;
}

ScConflictsListEntry* ScConflictsFinder::GetIntersectingEntry( const ScChangeAction* pAction ) const
{
    ScConflictsList::iterator aEnd = mrConflictsList.end();
    for ( ScConflictsList::iterator aItr = mrConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        ScChangeActionList::const_iterator aEndShared = aItr->maSharedActions.end();
        for ( ScChangeActionList::const_iterator aItrShared = aItr->maSharedActions.begin(); aItrShared != aEndShared; ++aItrShared )
        {
            if ( DoActionsIntersect( mpSharedTrack->GetAction( *aItrShared ), pAction ) )
            {
                return &(*aItr);
            }
        }

        ScChangeActionList::const_iterator aEndOwn = aItr->maOwnActions.end();
        for ( ScChangeActionList::const_iterator aItrOwn = aItr->maOwnActions.begin(); aItrOwn != aEndOwn; ++aItrOwn )
        {
            if ( DoActionsIntersect( mpOwnTrack->GetAction( *aItrOwn ), pAction ) )
            {
                return &(*aItr);
            }
        }
    }

    return NULL;
}

ScConflictsListEntry* ScConflictsFinder::GetEntry( ULONG nSharedAction, const ScChangeActionList& rOwnActions )
{
    // try to get a list entry which already contains the shared action
    ScConflictsListEntry* pEntry = ScConflictsListHelper::GetSharedActionEntry( mrConflictsList, nSharedAction );
    if ( pEntry )
    {
        return pEntry;
    }

    // try to get a list entry for which the shared action intersects with any
    // other action of this entry
    pEntry = GetIntersectingEntry( mpSharedTrack->GetAction( nSharedAction ) );
    if ( pEntry )
    {
        pEntry->maSharedActions.push_back( nSharedAction );
        return pEntry;
    }

    // try to get a list entry for which any of the own actions intersects with
    // any other action of this entry
    ScChangeActionList::const_iterator aEnd = rOwnActions.end();
    for ( ScChangeActionList::const_iterator aItr = rOwnActions.begin(); aItr != aEnd; ++aItr )
    {
        pEntry = GetIntersectingEntry( mpOwnTrack->GetAction( *aItr ) );
        if ( pEntry )
        {
            pEntry->maSharedActions.push_back( nSharedAction );
            return pEntry;
        }
    }

    // if no entry was found, create a new one
    ScConflictsListEntry aEntry;
    aEntry.meConflictAction = SC_CONFLICT_ACTION_NONE;
    aEntry.maSharedActions.push_back( nSharedAction );
    mrConflictsList.push_back( aEntry );
    return &(mrConflictsList.back());
}

void ScConflictsFinder::RemovePrevContentEntries()
{
    ScConflictsList::iterator aEnd = mrConflictsList.end();
    for ( ScConflictsList::iterator aItr = mrConflictsList.begin(); aItr != aEnd; ++aItr )
    {
        for ( ScChangeActionList::iterator aItrShared = aItr->maSharedActions.begin(); aItrShared != aItr->maSharedActions.end(); )
        {
            bool bPrevContent = false;
            ScChangeAction* pAction = mpSharedTrack->GetAction( *aItrShared );
            if ( pAction && pAction->GetType() == SC_CAT_CONTENT )
            {
                ScChangeActionContent* pNextContent = ( dynamic_cast< ScChangeActionContent* >( pAction ) )->GetNextContent();
                if ( pNextContent && aItr->HasSharedAction( pNextContent->GetActionNumber() ) )
                {
                    bPrevContent = true;
                }
            }
            if ( bPrevContent )
            {
                aItrShared = aItr->maSharedActions.erase( aItrShared );
            }
            else
            {
                ++aItrShared;
            }
        }

        for ( ScChangeActionList::iterator aItrOwn = aItr->maOwnActions.begin(); aItrOwn != aItr->maOwnActions.end(); )
        {
            bool bPrevContent = false;
            ScChangeAction* pAction = mpOwnTrack->GetAction( *aItrOwn );
            if ( pAction && pAction->GetType() == SC_CAT_CONTENT )
            {
                ScChangeActionContent* pNextContent = ( dynamic_cast< ScChangeActionContent* >( pAction ) )->GetNextContent();
                if ( pNextContent && aItr->HasOwnAction( pNextContent->GetActionNumber() ) )
                {
                    bPrevContent = true;
                }
            }
            if ( bPrevContent )
            {
                aItrOwn = aItr->maOwnActions.erase( aItrOwn );
            }
            else
            {
                ++aItrOwn;
            }
        }
    }
}

bool ScConflictsFinder::Find()
{
    if ( !mpSharedTrack || !mpOwnTrack )
    {
        return false;
    }

    bool bReturn = false;
    ScChangeAction* pSharedAction = mpSharedTrack->GetAction( mnStartShared );
    while ( pSharedAction && pSharedAction->GetActionNumber() <= mnEndShared )
    {
        ScChangeActionList aOwnActions;
        ScChangeAction* pOwnAction = mpOwnTrack->GetAction( mnStartOwn );
        while ( pOwnAction && pOwnAction->GetActionNumber() <= mnEndOwn )
        {
            if ( DoActionsIntersect( pSharedAction, pOwnAction ) )
            {
                aOwnActions.push_back( pOwnAction->GetActionNumber() );
            }
            pOwnAction = pOwnAction->GetNext();
        }

        if ( aOwnActions.size() )
        {
            ScConflictsListEntry* pEntry = GetEntry( pSharedAction->GetActionNumber(), aOwnActions );;
            ScChangeActionList::iterator aEnd = aOwnActions.end();
            for ( ScChangeActionList::iterator aItr = aOwnActions.begin(); aItr != aEnd; ++aItr )
            {
                if ( pEntry && !ScConflictsListHelper::HasOwnAction( mrConflictsList, *aItr ) )
                {
                    pEntry->maOwnActions.push_back( *aItr );
                }
            }
            bReturn = true;
        }

        pSharedAction = pSharedAction->GetNext();
    }

    RemovePrevContentEntries();

    return bReturn;
}

//=============================================================================
// class ScConflictsResolver
//=============================================================================

ScConflictsResolver::ScConflictsResolver( ScChangeTrack* pTrack, ScConflictsList& rConflictsList )
    :mpTrack ( pTrack )
    ,mrConflictsList ( rConflictsList )
{
    DBG_ASSERT( mpTrack, "ScConflictsResolver CTOR: mpTrack is null!" );
}

ScConflictsResolver::~ScConflictsResolver()
{
}

void ScConflictsResolver::HandleAction( ScChangeAction* pAction, ULONG nOffset,
    bool bIsSharedAction, bool bHandleContentAction, bool bHandleNonContentAction )
{
    if ( !mpTrack || !pAction )
    {
        return;
    }

    if ( bIsSharedAction )
    {
        ScConflictsListEntry* pConflictEntry = ScConflictsListHelper::GetSharedActionEntry(
            mrConflictsList, pAction->GetActionNumber() - nOffset );
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
            else if ( eConflictAction == SC_CONFLICT_ACTION_KEEP_OTHER )
            {
                if ( pAction->GetType() == SC_CAT_CONTENT )
                {
                    if ( bHandleContentAction )
                    {
                        mpTrack->SelectContent( pAction );
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
        }
    }
    else
    {
        ScConflictsListEntry* pConflictEntry = ScConflictsListHelper::GetOwnActionEntry(
            mrConflictsList, pAction->GetActionNumber() - nOffset );
        if ( pConflictEntry )
        {
            ScConflictAction eConflictAction = pConflictEntry->meConflictAction;
            if ( eConflictAction == SC_CONFLICT_ACTION_KEEP_MINE )
            {
                if ( pAction->GetType() == SC_CAT_CONTENT )
                {
                    if ( bHandleContentAction )
                    {
                        mpTrack->SelectContent( pAction );
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


//=============================================================================
// class ScConflictsListBox
//=============================================================================

ScConflictsListBox::ScConflictsListBox( Window* pParent, WinBits nBits )
    :SvxRedlinTable( pParent, nBits )
{
}

ScConflictsListBox::ScConflictsListBox( Window* pParent, const ResId& rResId )
    :SvxRedlinTable( pParent, rResId )
{
}

ScConflictsListBox::~ScConflictsListBox()
{
}

ULONG ScConflictsListBox::GetRootEntryPos( const SvLBoxEntry* pRootEntry ) const
{
    ULONG nPos = 0;
    SvLBoxEntry* pEntry = GetRootLevelParent( First() );
    while ( pEntry )
    {
        if ( pEntry == pRootEntry )
        {
            return nPos;
        }
        pEntry = NextSibling( pEntry );
        ++nPos;
    }
    return 0xffffffff;
}


//=============================================================================
// class ScConflictsDlg
//=============================================================================

ScConflictsDlg::ScConflictsDlg( Window* pParent, ScViewData* pViewData, ScDocument* pSharedDoc, ScConflictsList& rConflictsList )
    :ModalDialog( pParent, ScResId( RID_SCDLG_CONFLICTS ) )
    ,maFtConflicts      ( this, ScResId( FT_CONFLICTS ) )
    ,maLbConflicts      ( this, ScResId( LB_CONFLICTS ) )
    ,maBtnKeepMine      ( this, ScResId( BTN_KEEPMINE ) )
    ,maBtnKeepOther     ( this, ScResId( BTN_KEEPOTHER ) )
    ,maFlConflicts      ( this, ScResId( FL_CONFLICTS ) )
    ,maBtnKeepAllMine   ( this, ScResId( BTN_KEEPALLMINE ) )
    ,maBtnKeepAllOthers ( this, ScResId( BTN_KEEPALLOTHERS ) )
    ,maBtnCancel        ( this, ScResId( BTN_CANCEL ) )
    ,maBtnHelp          ( this, ScResId( BTN_HELP ) )
    ,maStrTitleConflict ( ScResId( STR_TITLE_CONFLICT ) )
    ,maStrTitleAuthor   ( ScResId( STR_TITLE_AUTHOR ) )
    ,maStrTitleDate     ( ScResId( STR_TITLE_DATE ) )
    ,maStrUnknownUser   ( ScResId( STR_UNKNOWN_USER ) )
    ,mpViewData         ( pViewData )
    ,mpOwnDoc           ( NULL )
    ,mpOwnTrack         ( NULL )
    ,mpSharedDoc        ( pSharedDoc )
    ,mpSharedTrack      ( NULL )
    ,mrConflictsList    ( rConflictsList )
    ,maDialogSize       ( GetSizePixel() )
    ,mbInSelectHdl      ( false )
    ,mbInDeselectHdl    ( false )
{
    DBG_ASSERT( mpViewData, "ScConflictsDlg CTOR: mpViewData is null!" );
    mpOwnDoc = ( mpViewData ? mpViewData->GetDocument() : NULL );
    DBG_ASSERT( mpOwnDoc, "ScConflictsDlg CTOR: mpOwnDoc is null!" );
    mpOwnTrack = ( mpOwnDoc ? mpOwnDoc->GetChangeTrack() : NULL );
    DBG_ASSERT( mpOwnTrack, "ScConflictsDlg CTOR: mpOwnTrack is null!" );
    DBG_ASSERT( mpSharedDoc, "ScConflictsDlg CTOR: mpSharedDoc is null!" );
    mpSharedTrack = ( mpSharedDoc ? mpSharedDoc->GetChangeTrack() : NULL );
    DBG_ASSERT( mpSharedTrack, "ScConflictsDlg CTOR: mpSharedTrack is null!" );

    FreeResource();

    SetMinOutputSizePixel( maDialogSize );

    long nTabs[] = { 3, 10, 160, 210 };
    maLbConflicts.SetTabs( nTabs );

    String aTab( sal_Unicode( '\t' ) );
    String aHeader( maStrTitleConflict );
    aHeader += aTab;
    aHeader += maStrTitleAuthor;
    aHeader += aTab;
    aHeader += maStrTitleDate;
    maLbConflicts.InsertHeaderEntry( aHeader, HEADERBAR_APPEND, HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER );

    maLbConflicts.SetWindowBits( WB_HASLINES | WB_CLIPCHILDREN | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL );
    maLbConflicts.SetSelectionMode( MULTIPLE_SELECTION );
    maLbConflicts.SetHighlightRange();

    maSelectionTimer.SetTimeout( 100 );
    maSelectionTimer.SetTimeoutHdl( LINK( this, ScConflictsDlg, UpdateSelectionHdl ) );

    maLbConflicts.SetSelectHdl( LINK( this, ScConflictsDlg, SelectHandle ) );
    maLbConflicts.SetDeselectHdl( LINK( this, ScConflictsDlg, DeselectHandle ) );

    maBtnKeepMine.SetClickHdl( LINK( this, ScConflictsDlg, KeepMineHandle ) );
    maBtnKeepOther.SetClickHdl( LINK( this, ScConflictsDlg, KeepOtherHandle ) );
    maBtnKeepAllMine.SetClickHdl( LINK( this, ScConflictsDlg, KeepAllMineHandle ) );
    maBtnKeepAllOthers.SetClickHdl( LINK( this, ScConflictsDlg, KeepAllOthersHandle ) );

    UpdateView();

    SvLBoxEntry* pEntry = maLbConflicts.First();
    if ( pEntry != NULL )
    {
        maLbConflicts.Select( pEntry );
    }
}

ScConflictsDlg::~ScConflictsDlg()
{
}

String ScConflictsDlg::GetConflictString( const ScConflictsListEntry& rConflictEntry )
{
    String aString;
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

String ScConflictsDlg::GetActionString( const ScChangeAction* pAction, ScDocument* pDoc )
{
    String aString;

    DBG_ASSERT( pAction, "ScConflictsDlg::GetActionString(): pAction is null!" );
    DBG_ASSERT( pDoc, "ScConflictsDlg::GetActionString(): pDoc is null!" );
    if ( pAction && pDoc )
    {
        String aDesc;
        pAction->GetDescription( aDesc, pDoc, TRUE );
        aString += aDesc;
        aString += '\t';

        String aUser = pAction->GetUser();
        aUser.EraseLeadingAndTrailingChars();
        if ( aUser.Len() == 0 )
        {
            aUser = maStrUnknownUser;
        }
        aString += aUser;
        aString += '\t';

        DateTime aDateTime = pAction->GetDateTime();
        aString += ScGlobal::pLocaleData->getDate( aDateTime );
        aString += ' ';
        aString += ScGlobal::pLocaleData->getTime( aDateTime, FALSE );
        aString += '\t';
    }

    return aString;
}

void ScConflictsDlg::HandleListBoxSelection( bool bSelectHandle )
{
    SvLBoxEntry* pSelEntry = maLbConflicts.GetCurEntry();
    if ( !pSelEntry )
    {
        pSelEntry = maLbConflicts.FirstSelected();
    }
    if ( !pSelEntry )
    {
        return;
    }

    SvLBoxEntry* pRootEntry = maLbConflicts.GetRootLevelParent( pSelEntry );
    if ( pRootEntry )
    {
        if ( bSelectHandle )
        {
            maLbConflicts.SelectAll( FALSE );
        }
        if ( !maLbConflicts.IsSelected( pRootEntry ) )
        {
            maLbConflicts.Select( pRootEntry );
        }
        SvLBoxEntry* pEntry = maLbConflicts.FirstChild( pRootEntry );
        while ( pEntry )
        {
            if ( !maLbConflicts.IsSelected( pEntry ) )
            {
                maLbConflicts.Select( pEntry );
            }
            pEntry = maLbConflicts.NextSibling( pEntry );
        }
    }
}

IMPL_LINK( ScConflictsDlg, SelectHandle, SvxRedlinTable*, EMPTYARG )
{
    if ( mbInSelectHdl || mbInDeselectHdl )
    {
        return 0;
    }

    mbInSelectHdl = true;
    HandleListBoxSelection( true );
    maSelectionTimer.Start();
    mbInSelectHdl = false;

    return 0;
}

IMPL_LINK( ScConflictsDlg, DeselectHandle, SvxRedlinTable*, EMPTYARG )
{
    if ( mbInDeselectHdl || mbInSelectHdl )
    {
        return 0;
    }

    mbInDeselectHdl = true;
    HandleListBoxSelection( false );
    mbInDeselectHdl = false;

    return 0;
}

IMPL_LINK( ScConflictsDlg, UpdateSelectionHdl, Timer*, EMPTYARG )
{
    if ( !mpViewData || !mpOwnDoc )
    {
        return 0;
    }

    ScTabView* pTabView = reinterpret_cast< ScTabView* >( mpViewData->GetView() );
    pTabView->DoneBlockMode();
    BOOL bContMark = FALSE;
    SvLBoxEntry* pEntry = maLbConflicts.FirstSelected();
    while ( pEntry )
    {
        if ( pEntry != maLbConflicts.GetRootLevelParent( pEntry ) )
        {
            RedlinData* pUserData = static_cast< RedlinData* >( pEntry->GetUserData() );
            if  ( pUserData )
            {
                ScChangeAction* pAction = static_cast< ScChangeAction* >( pUserData->pData );
                if ( pAction && ( pAction->GetType() != SC_CAT_DELETE_TABS ) &&
                     ( pAction->IsClickable() || pAction->IsVisible() ) )
                {
                    const ScBigRange& rBigRange = ( static_cast< const ScChangeAction* >( pAction ) )->GetBigRange();
                    if ( rBigRange.IsValid( mpOwnDoc ) )
                    {
                        BOOL bSetCursor = !maLbConflicts.NextSelected( pEntry );
                        pTabView->MarkRange( rBigRange.MakeRange(), bSetCursor, bContMark );
                        bContMark = TRUE;
                    }
                }
            }
        }
        pEntry = maLbConflicts.NextSelected( pEntry );
    }

    return 0;
}

void ScConflictsDlg::SetConflictAction( SvLBoxEntry* pRootEntry, ScConflictAction eConflictAction )
{
    RedlinData* pUserData = static_cast< RedlinData* >( pRootEntry ? pRootEntry->GetUserData() : NULL );
    ScConflictsListEntry* pConflictEntry = static_cast< ScConflictsListEntry* >( pUserData ? pUserData->pData : NULL );
    if ( pConflictEntry )
    {
        pConflictEntry->meConflictAction = eConflictAction;
    }
}

void ScConflictsDlg::KeepHandler( bool bMine )
{
    SvLBoxEntry* pEntry = maLbConflicts.FirstSelected();
    SvLBoxEntry* pRootEntry = ( pEntry ? maLbConflicts.GetRootLevelParent( pEntry ) : NULL );
    if ( !pRootEntry )
    {
        return;
    }
    SetPointer( Pointer( POINTER_WAIT ) );
    ScConflictAction eConflictAction = ( bMine ? SC_CONFLICT_ACTION_KEEP_MINE : SC_CONFLICT_ACTION_KEEP_OTHER );
    SetConflictAction( pRootEntry, eConflictAction );
    maLbConflicts.RemoveEntry( pRootEntry );
    SetPointer( Pointer( POINTER_ARROW ) );
    if ( maLbConflicts.GetEntryCount() == 0 )
    {
        EndDialog( RET_OK );
    }
}

void ScConflictsDlg::KeepAllHandler( bool bMine )
{
    SvLBoxEntry* pEntry = maLbConflicts.First();
    SvLBoxEntry* pRootEntry = ( pEntry ? maLbConflicts.GetRootLevelParent( pEntry ) : NULL );
    if ( !pRootEntry )
    {
        return;
    }
    SetPointer( Pointer( POINTER_WAIT ) );
    ScConflictAction eConflictAction = ( bMine ? SC_CONFLICT_ACTION_KEEP_MINE : SC_CONFLICT_ACTION_KEEP_OTHER );
    while ( pRootEntry )
    {
        SetConflictAction( pRootEntry, eConflictAction );
        pRootEntry = maLbConflicts.NextSibling( pRootEntry );
    }
    maLbConflicts.SetUpdateMode( FALSE );
    maLbConflicts.Clear();
    maLbConflicts.SetUpdateMode( TRUE );
    SetPointer( Pointer( POINTER_ARROW ) );
    EndDialog( RET_OK );
}

IMPL_LINK( ScConflictsDlg, KeepMineHandle, void*, EMPTYARG )
{
    KeepHandler( true );

    return 0;
}

IMPL_LINK( ScConflictsDlg, KeepOtherHandle, void*, EMPTYARG )
{
    KeepHandler( false );

    return 0;
}

IMPL_LINK( ScConflictsDlg, KeepAllMineHandle, void*, EMPTYARG )
{
    KeepAllHandler( true );

    return 0;
}

IMPL_LINK( ScConflictsDlg, KeepAllOthersHandle, void*, EMPTYARG )
{
    KeepAllHandler( false );

    return 0;
}

void lcl_MoveControlX( Window& rWindow, long nDelta )
{
    Point aPos( rWindow.GetPosPixel() );
    aPos.X() += nDelta;
    rWindow.SetPosPixel( aPos );
}

void lcl_MoveControlY( Window& rWindow, long nDelta )
{
    Point aPos( rWindow.GetPosPixel() );
    aPos.Y() += nDelta;
    rWindow.SetPosPixel( aPos );
}

void lcl_ChangeControlWidth( Window& rWindow, long nDelta )
{
    Size aSize( rWindow.GetSizePixel() );
    aSize.Width() += nDelta;
    rWindow.SetSizePixel( aSize );
}

void lcl_ChangeControlHeight( Window& rWindow, long nDelta )
{
    Size aSize( rWindow.GetSizePixel() );
    aSize.Height() += nDelta;
    rWindow.SetSizePixel( aSize );
}

void ScConflictsDlg::Resize()
{
    Size aSize( GetSizePixel() );
    long nDeltaWidth = aSize.Width() - maDialogSize.Width();
    long nDeltaHeight = aSize.Height() - maDialogSize.Height();
    maDialogSize = aSize;

    lcl_ChangeControlWidth( maFtConflicts, nDeltaWidth );

    lcl_ChangeControlWidth( maLbConflicts, nDeltaWidth );
    lcl_ChangeControlHeight( maLbConflicts, nDeltaHeight );

    lcl_MoveControlX( maBtnKeepMine, nDeltaWidth / 2 );
    lcl_MoveControlY( maBtnKeepMine, nDeltaHeight );

    lcl_MoveControlX( maBtnKeepOther, nDeltaWidth / 2 );
    lcl_MoveControlY( maBtnKeepOther, nDeltaHeight );

    lcl_MoveControlY( maFlConflicts, nDeltaHeight );
    lcl_ChangeControlWidth( maFlConflicts, nDeltaWidth );

    lcl_MoveControlX( maBtnKeepAllMine, nDeltaWidth );
    lcl_MoveControlY( maBtnKeepAllMine, nDeltaHeight );

    lcl_MoveControlX( maBtnKeepAllOthers, nDeltaWidth );
    lcl_MoveControlY( maBtnKeepAllOthers, nDeltaHeight );

    lcl_MoveControlX( maBtnCancel, nDeltaWidth );
    lcl_MoveControlY( maBtnCancel, nDeltaHeight );

    lcl_MoveControlX( maBtnHelp, nDeltaWidth );
    lcl_MoveControlY( maBtnHelp, nDeltaHeight );
}

void ScConflictsDlg::UpdateView()
{
    ScConflictsList::iterator aEndItr = mrConflictsList.end();
    for ( ScConflictsList::iterator aItr = mrConflictsList.begin(); aItr != aEndItr; ++aItr )
    {
        ScConflictsListEntry* pConflictEntry = &(*aItr);
        if ( pConflictEntry && pConflictEntry->meConflictAction == SC_CONFLICT_ACTION_NONE )
        {
            RedlinData* pRootUserData = new RedlinData();
            pRootUserData->pData = static_cast< void* >( pConflictEntry );
            SvLBoxEntry* pRootEntry = maLbConflicts.InsertEntry( GetConflictString( *aItr ), pRootUserData );

            ScChangeActionList::const_iterator aEndShared = aItr->maSharedActions.end();
            for ( ScChangeActionList::const_iterator aItrShared = aItr->maSharedActions.begin(); aItrShared != aEndShared; ++aItrShared )
            {
                ScChangeAction* pAction = mpSharedTrack->GetAction( *aItrShared );
                if ( pAction )
                {
                    String aString( GetActionString( pAction, mpSharedDoc ) );
                    RedlinData* pUserData = new RedlinData();
                    pUserData->pData = static_cast< void* >( pAction );
                    maLbConflicts.InsertEntry( aString, pUserData, pRootEntry );
                }
            }

            ScChangeActionList::const_iterator aEndOwn = aItr->maOwnActions.end();
            for ( ScChangeActionList::const_iterator aItrOwn = aItr->maOwnActions.begin(); aItrOwn != aEndOwn; ++aItrOwn )
            {
                ScChangeAction* pAction = mpOwnTrack->GetAction( *aItrOwn );
                if ( pAction )
                {
                    String aString( GetActionString( pAction, mpOwnDoc ) );
                    RedlinData* pUserData = new RedlinData();
                    pUserData->pData = static_cast< void* >( pAction );
                    maLbConflicts.InsertEntry( aString, pUserData, pRootEntry );
                }
            }

            maLbConflicts.Expand( pRootEntry );
        }
    }
}
