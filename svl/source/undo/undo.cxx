/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <com/sun/star/uno/Exception.hpp>

#include <comphelper/flagguard.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <svl/undo.hxx>

#include <vector>

using ::com::sun::star::uno::Exception;

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxUndoAction)

//========================================================================

TYPEINIT0(SfxUndoAction);
TYPEINIT0(SfxListUndoAction);
TYPEINIT0(SfxLinkUndoAction);
TYPEINIT0(SfxRepeatTarget);

//------------------------------------------------------------------------

SfxRepeatTarget::~SfxRepeatTarget()
{
}

//------------------------------------------------------------------------

BOOL SfxUndoAction::IsLinked()
{
    return bLinked;
}

//------------------------------------------------------------------------

void SfxUndoAction::SetLinked( BOOL bIsLinked )
{
    bLinked = bIsLinked;
}

//------------------------------------------------------------------------

SfxUndoAction::~SfxUndoAction()
{
    DBG_DTOR(SfxUndoAction, 0);
    DBG_ASSERT( !IsLinked(), "Gelinkte Action geloescht" );
}


SfxUndoAction::SfxUndoAction()
{
    DBG_CTOR(SfxUndoAction, 0);
    SetLinked( FALSE );
}

//------------------------------------------------------------------------

BOOL SfxUndoAction::Merge( SfxUndoAction * )
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return FALSE;
}

//------------------------------------------------------------------------

XubString SfxUndoAction::GetComment() const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return XubString();
}

//------------------------------------------------------------------------


USHORT SfxUndoAction::GetId() const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return 0;
}

//------------------------------------------------------------------------

XubString SfxUndoAction::GetRepeatComment(SfxRepeatTarget&) const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return GetComment();
}

//------------------------------------------------------------------------


void SfxUndoAction::Undo()
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Undo()" );
}

//------------------------------------------------------------------------

void SfxUndoAction::Redo()
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Redo()" );
}

//------------------------------------------------------------------------

void SfxUndoAction::Repeat(SfxRepeatTarget&)
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Repeat()" );
}

//------------------------------------------------------------------------


BOOL SfxUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return TRUE;
}

//========================================================================

typedef ::std::vector< SfxUndoListener* >   UndoListeners;

struct SfxUndoManager_Data
{
    SfxUndoArray*   pUndoArray;
    SfxUndoArray*   pActUndoArray;
    SfxUndoArray*   pFatherUndoArray;

    sal_Int32       mnLockCount;
    bool            mbDoing;
#ifdef DBG_UTIL
    bool            mbCompatibilityAssertions;
#endif

    UndoListeners   aListeners;

    SfxUndoManager_Data( USHORT i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( NULL )
        ,pFatherUndoArray( NULL )
        ,mnLockCount( 0 )
        ,mbDoing( false )
#ifdef DBG_UTIL
        ,mbCompatibilityAssertions( true )
#endif
    {
        pActUndoArray = pUndoArray;
    }

    ~SfxUndoManager_Data()
    {
        delete pUndoArray;
    }
};

//========================================================================

namespace
{
    //--------------------------------------------------------------------
    struct LockGuard
    {
        LockGuard( ::svl::IUndoManager& i_manager )
            :m_manager( i_manager )
        {
            m_manager.EnableUndo( false );
        }

        ~LockGuard()
        {
            m_manager.EnableUndo( true );
        }

    private:
        ::svl::IUndoManager& m_manager;
    };

    //--------------------------------------------------------------------
    struct NotifyUndoListener : public ::std::unary_function< SfxUndoListener*, void >
    {
        NotifyUndoListener( void ( SfxUndoListener::*i_notificationMethod )() )
            :m_notificationMethod( i_notificationMethod )
        {
        }

        void operator()( SfxUndoListener* i_listener ) const
        {
            ( i_listener->*m_notificationMethod )();
        }

        void ( SfxUndoListener::*m_notificationMethod )();
    };
}

//========================================================================

SfxUndoManager::SfxUndoManager( USHORT nMaxUndoActionCount )
    :m_pData( new SfxUndoManager_Data( nMaxUndoActionCount ) )
{
}

//------------------------------------------------------------------------

SfxUndoManager::~SfxUndoManager()
{
    ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
        NotifyUndoListener( &SfxUndoListener::undoManagerDying ) );
}

//------------------------------------------------------------------------

void SfxUndoManager::EnableUndo( bool bEnable )
{
    DBG_TESTSOLARMUTEX();
    if ( !bEnable )
        ++m_pData->mnLockCount;
    else
    {
        OSL_PRECOND( m_pData->mnLockCount > 0, "SfxUndoManager::EnableUndo: not disabled, so why enabling?" );
        if ( m_pData->mnLockCount > 0 )
            --m_pData->mnLockCount;
    }
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsUndoEnabled() const
{
    DBG_TESTSOLARMUTEX();
    return m_pData->mnLockCount == 0;
}

//------------------------------------------------------------------------

void SfxUndoManager::SetMaxUndoActionCount( USHORT nMaxUndoActionCount )
{
    // Remove entries from the pActUndoArray when we have to reduce
    // the number of entries due to a lower nMaxUndoActionCount.
    // Both redo and undo action entries will be removed until we reached the
    // new nMaxUndoActionCount.

    long nNumToDelete = m_pData->pActUndoArray->aUndoActions.Count() - nMaxUndoActionCount;
    if ( nNumToDelete > 0 )
    {
        while ( nNumToDelete > 0 )
        {
            USHORT nPos = m_pData->pActUndoArray->aUndoActions.Count();
            if ( nPos > m_pData->pActUndoArray->nCurUndoAction )
            {
                if ( !m_pData->pActUndoArray->aUndoActions[nPos-1]->IsLinked() )
                {
                    delete m_pData->pActUndoArray->aUndoActions[nPos-1];
                    m_pData->pActUndoArray->aUndoActions.Remove( nPos-1 );
                    --nNumToDelete;
                }
            }

            if ( nNumToDelete > 0 && m_pData->pActUndoArray->nCurUndoAction > 0 )
            {
                if ( !m_pData->pActUndoArray->aUndoActions[0]->IsLinked() )
                {
                    delete m_pData->pActUndoArray->aUndoActions[0];
                    m_pData->pActUndoArray->aUndoActions.Remove(0);
                    --m_pData->pActUndoArray->nCurUndoAction;
                    --nNumToDelete;
                }
            }

            if ( nPos == m_pData->pActUndoArray->aUndoActions.Count() )
                break; // Cannot delete more entries
        }
    }

    m_pData->pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetMaxUndoActionCount() const
{
    return m_pData->pActUndoArray->nMaxUndoActions;
}

//------------------------------------------------------------------------

void SfxUndoManager::Clear()
{
    while ( m_pData->pActUndoArray->aUndoActions.Count() )
    {
        SfxUndoAction *pAction=
            m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->aUndoActions.Count() - 1];
        m_pData->pActUndoArray->aUndoActions.Remove( m_pData->pActUndoArray->aUndoActions.Count() - 1 );
        delete pAction;
    }

    m_pData->pActUndoArray->nCurUndoAction = 0;

    // notify listeners
    ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
        NotifyUndoListener( &SfxUndoListener::cleared ) );
}

//------------------------------------------------------------------------

void SfxUndoManager::ClearRedo()
{
    while ( m_pData->pActUndoArray->aUndoActions.Count() > m_pData->pActUndoArray->nCurUndoAction )
    {
        SfxUndoAction *pAction=
            m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->aUndoActions.Count() - 1];
        m_pData->pActUndoArray->aUndoActions.Remove( m_pData->pActUndoArray->aUndoActions.Count() - 1 );
        delete pAction;
    }

    ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
        NotifyUndoListener( &SfxUndoListener::clearedRedo ) );
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerge )
{
    if ( IsUndoEnabled() )
    {
        // Redo-Actions loeschen
        if ( ImplGetRedoActionCount() > 0 )
            ClearRedo();

        if ( m_pData->pActUndoArray->nMaxUndoActions )
        {
            SfxUndoAction *pTmpAction = m_pData->pActUndoArray->nCurUndoAction ?
                m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1] : 0;

            if ( !bTryMerge || !(pTmpAction && pTmpAction->Merge(pAction)) )
            {
                // respect max number
                if( m_pData->pActUndoArray == m_pData->pUndoArray )
                    while( m_pData->pActUndoArray->aUndoActions.Count() >=
                           m_pData->pActUndoArray->nMaxUndoActions &&
                           !m_pData->pActUndoArray->aUndoActions[0]->IsLinked() )
                    {
                        delete m_pData->pActUndoArray->aUndoActions[0];
                        m_pData->pActUndoArray->aUndoActions.Remove(0);
                        --m_pData->pActUndoArray->nCurUndoAction;
                    }

                // append new action
                const SfxUndoAction* pTemp = pAction;
                m_pData->pActUndoArray->aUndoActions.Insert(
                    pTemp, m_pData->pActUndoArray->nCurUndoAction++ );

                // notify listeners
                for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
                        listener != m_pData->aListeners.end();
                        ++listener
                    )
                {
                    (*listener)->undoActionAdded( *pAction );
                }

                // outta here
                return;
            }
        }
    }
    delete pAction;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionCount( bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( USHORT nNo, bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;

    String sComment;
    DBG_ASSERT( nNo < pUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionComment: illegal index!" );
    if( nNo < pUndoArray->nCurUndoAction )
    {
        sComment = pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction - 1 - nNo ]->GetComment();
    }
    return sComment;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionId() const
{
    DBG_ASSERT( m_pData->pActUndoArray->nCurUndoAction > 0, "svl::SfxUndoManager::GetUndoActionId(), illegal id!" );
    if( m_pData->pActUndoArray->nCurUndoAction > 0 )
    {
        return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1]->GetId();
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------

SfxUndoAction* SfxUndoManager::GetUndoAction( USHORT nNo ) const
{
    DBG_ASSERT( nNo < m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoAction(), illegal id!" );
    if( nNo < m_pData->pActUndoArray->nCurUndoAction )
    {
        return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1-nNo]; //!
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------

/** clears the redo stack and removes the top undo action */
void SfxUndoManager::RemoveLastUndoAction()
{
    DBG_ASSERT( m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::RemoveLastUndoAction(), no action to remove?!" );
    if( m_pData->pActUndoArray->nCurUndoAction )
    {
        m_pData->pActUndoArray->nCurUndoAction--;

        // delete redo-actions and top action
        USHORT nPos;
        for ( nPos = m_pData->pActUndoArray->aUndoActions.Count(); nPos > m_pData->pActUndoArray->nCurUndoAction; --nPos )
            delete m_pData->pActUndoArray->aUndoActions[nPos-1];

        m_pData->pActUndoArray->aUndoActions.Remove(
            m_pData->pActUndoArray->nCurUndoAction,
            m_pData->pActUndoArray->aUndoActions.Count() - m_pData->pActUndoArray->nCurUndoAction );
    }
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsDoing() const
{
    DBG_TESTSOLARMUTEX();
    return m_pData->mbDoing;
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Undo()
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Undo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( IsInListAction() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: not possible when within a list action!" );
        return FALSE;
    }

    if ( m_pData->pActUndoArray->nCurUndoAction == 0 )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: undo stack is empty!" );
        return FALSE;
    }

    SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction - 1 ];
    try
    {
        pAction->Undo();
    }
    catch( const Exception& )
    {
        // assume that this is a permanent failure, and clear the Undo stack.
        while ( m_pData->pActUndoArray->nCurUndoAction > 0 )
        {
            SfxUndoAction* pUndoAction = m_pData->pActUndoArray->aUndoActions[0];
            m_pData->pActUndoArray->aUndoActions.Remove( 0 );
            delete pUndoAction;
            --m_pData->pActUndoArray->nCurUndoAction;
        }
        // TODO: notifications? We don't have clearedUndo, only cleared and clearedRedo at the SfxUndoListener
        throw;
    }
    --m_pData->pActUndoArray->nCurUndoAction;

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->actionUndone( *pAction );
    }

    return TRUE;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRedoActionCount( bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->aUndoActions.Count() - pUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( USHORT nNo, bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction + nNo ]->GetComment();
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Redo()
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Redo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( IsInListAction() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Redo: not possible when within a list action!" );
        return FALSE;
    }

    if ( m_pData->pActUndoArray->nCurUndoAction >= m_pData->pActUndoArray->aUndoActions.Count() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Redo: redo stack is empty!" );
        return FALSE;
    }

    SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction];
    try
    {
        pAction->Redo();
    }
    catch( const Exception& )
    {
        ClearRedo();
        throw;
    }
    ++m_pData->pActUndoArray->nCurUndoAction;

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->actionRedone( *pAction );
    }

    return TRUE;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRepeatActionCount() const
{
    return m_pData->pActUndoArray->aUndoActions.Count();
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget) const
{
    return m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.Count() - 1 ]
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Repeat( SfxRepeatTarget &rTarget )
{
    if ( m_pData->pActUndoArray->aUndoActions.Count() )
    {
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.Count() - 1 ];
        if ( pAction->CanRepeat( rTarget ) )
            pAction->Repeat( rTarget );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget ) const
{
    if ( m_pData->pActUndoArray->aUndoActions.Count() > 0 )
    {
        USHORT nActionNo = m_pData->pActUndoArray->aUndoActions.Count() - 1;
        return m_pData->pActUndoArray->aUndoActions[nActionNo]->CanRepeat(rTarget);
    }

    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoListener( SfxUndoListener& i_listener )
{
    DBG_TESTSOLARMUTEX();
    m_pData->aListeners.push_back( &i_listener );
}

//------------------------------------------------------------------------

void SfxUndoManager::RemoveUndoListener( SfxUndoListener& i_listener )
{
    DBG_TESTSOLARMUTEX();
    for (   UndoListeners::iterator lookup = m_pData->aListeners.begin();
            lookup != m_pData->aListeners.end();
            ++lookup
        )
    {
        if ( (*lookup) == &i_listener )
        {
            m_pData->aListeners.erase( lookup );
            break;
        }
    }
}

//------------------------------------------------------------------------

void SfxUndoManager::EnterListAction(
    const XubString& rComment, const XubString &rRepeatComment, USHORT nId )

/*  [Beschreibung]

    Fuegt eine ListUndoAction ein und setzt dessen UndoArray als aktuelles.
*/

{
    if( !IsUndoEnabled() )
        return;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return;

    m_pData->pFatherUndoArray = m_pData->pActUndoArray;
    SfxListUndoAction *pAction=new SfxListUndoAction(
        rComment, rRepeatComment, nId, m_pData->pActUndoArray);
    AddUndoAction( pAction );
    m_pData->pActUndoArray=pAction;

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->listActionEntered( rComment );
    }
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsInListAction() const
{
    return ( m_pData->pActUndoArray != m_pData->pUndoArray );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetListActionDepth() const
{
    USHORT nDepth(0);

    SfxUndoArray* pLookup( m_pData->pActUndoArray );
    while ( pLookup != m_pData->pUndoArray )
    {
        pLookup = pLookup->pFatherUndoArray;
        ++nDepth;
    }

    return nDepth;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::LeaveListAction()
{
    return ImplLeaveListAction( false );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::LeaveAndMergeListAction()
{
    return ImplLeaveListAction( true );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::ImplLeaveListAction( const bool i_merge )
{
    if ( !IsUndoEnabled() )
        return 0;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return 0;

    if( !IsInListAction() )
    {
        DBG_ERROR( "svl::SfxUndoManager::ImplLeaveListAction, called without calling EnterListAction()!" );
        return 0;
    }

    DBG_ASSERT(m_pData->pActUndoArray->pFatherUndoArray,"svl::SfxUndoManager::ImplLeaveListAction, no father undo array!?");

    // the array/level which we're about to leave
    SfxUndoArray* pArrayToLeave = m_pData->pActUndoArray;
    // one step up
    m_pData->pActUndoArray = m_pData->pActUndoArray->pFatherUndoArray;

    // If no undo action where added, delete the undo list action
    const USHORT nListActionElements = pArrayToLeave->nCurUndoAction;
    if( nListActionElements == 0 )
    {
        SfxUndoAction* pCurrentAction= m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction-1 ];
        m_pData->pActUndoArray->aUndoActions.Remove( --m_pData->pActUndoArray->nCurUndoAction );
        delete pCurrentAction;

        ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
            NotifyUndoListener( &SfxUndoListener::listActionCancelled ) );
        return 0;
    }


    SfxUndoAction* pCurrentAction= m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction-1 ];
    SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction * >( pCurrentAction );
    OSL_ENSURE( pListAction, "SfxUndoManager::ImplLeaveListAction: list action expected at this position!" );

    if ( pListAction && i_merge )
    {
        // merge the list action with its predecessor on the same level
        OSL_ENSURE( m_pData->pActUndoArray->nCurUndoAction > 1,
            "SfxUndoManager::ImplLeaveListAction: cannot merge the list action if there's no other action on the same level - check this beforehand!" );
        if ( m_pData->pActUndoArray->nCurUndoAction > 1 )
        {
            const SfxUndoAction* pPreviousAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction - 2 ];
            m_pData->pActUndoArray->aUndoActions.Remove( m_pData->pActUndoArray->nCurUndoAction - 2 );
            --m_pData->pActUndoArray->nCurUndoAction;
            pListAction->aUndoActions.Insert( pPreviousAction, 0 );
            ++pListAction->nCurUndoAction;

            pListAction->SetComment( pPreviousAction->GetComment() );
        }
    }

    // if the undo array has no comment, try to get it from its children
    if ( pListAction && pListAction->GetComment().Len() == 0 )
    {
        for( USHORT n = 0; n < pListAction->aUndoActions.Count(); n++ )
        {
            if( pListAction->aUndoActions[n]->GetComment().Len() )
            {
                pListAction->SetComment( pListAction->aUndoActions[n]->GetComment() );
                break;
            }
        }
    }

    // notify listeners
    ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
        NotifyUndoListener( &SfxUndoListener::listActionLeft ) );

    return nListActionElements;
}

//------------------------------------------------------------------------
#ifdef DBG_UTIL
void SfxUndoManager::DbgEnableCompatibilityAssertions( bool const i_enable )
{
    OSL_ENSURE( m_pData->mbCompatibilityAssertions != i_enable,
        "SfxUndoManager::DbgEnableCompatibilityAssertions: nested calls not supported, this is expected to be with each call!" );
    m_pData->mbCompatibilityAssertions = i_enable;
}
#endif

//------------------------------------------------------------------------

USHORT SfxListUndoAction::GetId() const
{
    return nId;
}

//------------------------------------------------------------------------

XubString SfxListUndoAction::GetComment() const
{
    return aComment;
}

//------------------------------------------------------------------------

void SfxListUndoAction::SetComment( const UniString& rComment )
{
    aComment = rComment;
}

//------------------------------------------------------------------------

XubString SfxListUndoAction::GetRepeatComment(SfxRepeatTarget &) const
{
    return aRepeatComment;
}


//------------------------------------------------------------------------

SfxListUndoAction::SfxListUndoAction
(
    const XubString &rComment,
    const XubString rRepeatComment,
    USHORT Id,
    SfxUndoArray *pFather
)
: nId(Id), aComment(rComment), aRepeatComment(rRepeatComment)
{
    pFatherUndoArray = pFather;
    nMaxUndoActions = USHRT_MAX;
}

//------------------------------------------------------------------------

void SfxListUndoAction::Undo()
{
    for(INT16 i=nCurUndoAction-1;i>=0;i--)
        aUndoActions[i]->Undo();
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::Redo()
{
    for(USHORT i=nCurUndoAction;i<aUndoActions.Count();i++)
        aUndoActions[i]->Redo();
    nCurUndoAction = aUndoActions.Count();
}

//------------------------------------------------------------------------

void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        aUndoActions[i]->Repeat(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        if(!aUndoActions[i]->CanRepeat(r))
            return FALSE;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
{
    return aUndoActions.Count() && aUndoActions[aUndoActions.Count()-1]->Merge( pNextAction );
}

//------------------------------------------------------------------------

SfxLinkUndoAction::SfxLinkUndoAction(::svl::IUndoManager *pManager)
/*  [Beschreibung]

    Richtet eine LinkAction ein, die auf einen weiteren UndoManager zeigt.
    Holt sich als zugehoerige Action des weiteren UndoManagers dessen
    aktuelle Action.
*/

{
    pUndoManager = pManager;
    SfxUndoManager* pUndoManagerImplementation = dynamic_cast< SfxUndoManager* >( pManager );
    ENSURE_OR_THROW( pUndoManagerImplementation != NULL, "unsupported undo manager implementation!" );
        // yes, this cast is dirty. But reaching into the the SfxUndoManager's implementation,
        // directly accessing its internal stack, and tampering with an action on that stack
        // is dirty, too.
    if ( pManager->GetMaxUndoActionCount() )
    {
        USHORT nPos = pUndoManagerImplementation->ImplGetUndoActionCount()-1;
        pAction = pUndoManagerImplementation->m_pData->pActUndoArray->aUndoActions[nPos];
        pAction->SetLinked();
    }
    else
        pAction = 0;
}

//------------------------------------------------------------------------

void SfxLinkUndoAction::Undo()
{
    if ( pAction )
        pUndoManager->Undo();
}

//------------------------------------------------------------------------

void SfxLinkUndoAction::Redo()
{
    if ( pAction )
        pUndoManager->Redo();
}

//------------------------------------------------------------------------


BOOL SfxLinkUndoAction::CanRepeat(SfxRepeatTarget& r) const
{
    return pAction && pAction->CanRepeat(r);
}


//------------------------------------------------------------------------


void SfxLinkUndoAction::Repeat(SfxRepeatTarget&r)
{
    if ( pAction && pAction->CanRepeat( r ) )
        pAction->Repeat( r );
}


//------------------------------------------------------------------------

XubString SfxLinkUndoAction::GetComment() const
{
    if ( pAction )
        return pAction->GetComment();
    else
        return XubString();
}


//------------------------------------------------------------------------

XubString SfxLinkUndoAction::GetRepeatComment(SfxRepeatTarget&r) const
{
    if ( pAction )
        return pAction->GetRepeatComment(r);
    else
        return XubString();
}

//------------------------------------------------------------------------

SfxLinkUndoAction::~SfxLinkUndoAction()
{
    if( pAction )
        pAction->SetLinked( FALSE );
}


//------------------------------------------------------------------------

SfxUndoArray::~SfxUndoArray()
{
    while ( aUndoActions.Count() )
    {
        SfxUndoAction *pAction =
            aUndoActions[ aUndoActions.Count() - 1 ];
        aUndoActions.Remove( aUndoActions.Count() - 1 );
        delete pAction;
    }
}


USHORT SfxLinkUndoAction::GetId() const
{
      return pAction ? pAction->GetId() : 0;
}



