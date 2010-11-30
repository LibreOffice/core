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
#include <list>
#include <limits>

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

SfxUndoContext::~SfxUndoContext()
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

void SfxUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    (void)i_context;
    Undo();
}

//------------------------------------------------------------------------

void SfxUndoAction::Redo()
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Redo()" );
}

//------------------------------------------------------------------------

void SfxUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    (void)i_context;
    Redo();
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

struct SVL_DLLPRIVATE SfxUndoManager_Data
{
    ::osl::Mutex    aMutex;
    SfxUndoArray*   pUndoArray;
    SfxUndoArray*   pActUndoArray;
    SfxUndoArray*   pFatherUndoArray;

    sal_Int32       mnLockCount;
    sal_Int32       mnMarks;
    bool            mbDoing;

    UndoListeners   aListeners;

    SfxUndoManager_Data( USHORT i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( NULL )
        ,pFatherUndoArray( NULL )
        ,mnLockCount( 0 )
        ,mnMarks( 0 )
        ,mbDoing( false )

    {
        pActUndoArray = pUndoArray;
    }

    ~SfxUndoManager_Data()
    {
        delete pUndoArray;
    }
};

//========================================================================

namespace svl { namespace undo { namespace impl
{
    //--------------------------------------------------------------------
    class SVL_DLLPRIVATE LockGuard
    {
    public:
        LockGuard( SfxUndoManager& i_manager )
            :m_manager( i_manager )
        {
            m_manager.ImplEnableUndo_Lock( false );
        }

        ~LockGuard()
        {
            m_manager.ImplEnableUndo_Lock( true );
        }

    private:
        SfxUndoManager& m_manager;
    };

    //--------------------------------------------------------------------
    typedef void ( SfxUndoListener::*UndoListenerVoidMethod )();
    typedef void ( SfxUndoListener::*UndoListenerStringMethod )( const String& );

    //--------------------------------------------------------------------
    struct SVL_DLLPRIVATE NotifyUndoListener : public ::std::unary_function< SfxUndoListener*, void >
    {
        NotifyUndoListener()
            :m_notificationMethod( NULL )
            ,m_altNotificationMethod( NULL )
            ,m_sActionComment()
        {
        }

        NotifyUndoListener( UndoListenerVoidMethod i_notificationMethod )
            :m_notificationMethod( i_notificationMethod )
            ,m_altNotificationMethod( NULL )
            ,m_sActionComment()
        {
        }

        NotifyUndoListener( UndoListenerStringMethod i_notificationMethod, const String& i_actionComment )
            :m_notificationMethod( NULL )
            ,m_altNotificationMethod( i_notificationMethod )
            ,m_sActionComment( i_actionComment )
        {
        }

        bool is() const
        {
            return ( m_notificationMethod != NULL ) || ( m_altNotificationMethod != NULL );
        }

        void operator()( SfxUndoListener* i_listener ) const
        {
            OSL_PRECOND( is(), "NotifyUndoListener: this will crash!" );
            if ( m_altNotificationMethod != NULL )
            {
                ( i_listener->*m_altNotificationMethod )( m_sActionComment );
            }
            else
            {
                ( i_listener->*m_notificationMethod )();
            }
        }

    private:
        UndoListenerVoidMethod      m_notificationMethod;
        UndoListenerStringMethod    m_altNotificationMethod;
        String                      m_sActionComment;
    };

    //--------------------------------------------------------------------
    class SVL_DLLPRIVATE UndoManagerGuard
    {
    public:
        UndoManagerGuard( SfxUndoManager_Data& i_managerData )
            :m_rManagerData( i_managerData )
            ,m_aGuard( i_managerData.aMutex )
            ,m_notifiers()
        {
        }

        ~UndoManagerGuard();

        void clear()
        {
            m_aGuard.clear();
        }

        void reset()
        {
            m_aGuard.reset();
        }

        void cancelNotifications()
        {
            m_notifiers.clear();
        }

        /** marks the given Undo action for deletion

            The Undo action will be put into a list, whose members will be deleted from within the destructor of the
            UndoManagerGuard. This deletion will happen without the UndoManager's mutex locked.
        */
        void    markForDeletion( SfxUndoAction* i_action )
        {
            // remember
            if ( i_action )
                m_aUndoActionsCleanup.push_back( i_action );
        }

        /** schedules the given SfxUndoListener method to be called for all registered listeners.

            The notification will happen after the Undo manager's mutex has been released, and after all pending
            deletions of Undo actions are done.
        */
        void    scheduleNotification( UndoListenerVoidMethod i_notificationMethod )
        {
            m_notifiers.push_back( NotifyUndoListener( i_notificationMethod ) );
        }

        void    scheduleNotification( UndoListenerStringMethod i_notificationMethod, const String& i_actionComment )
        {
            m_notifiers.push_back( NotifyUndoListener( i_notificationMethod, i_actionComment ) );
        }

    private:
        SfxUndoManager_Data&                m_rManagerData;
        ::osl::ResettableMutexGuard         m_aGuard;
        ::std::list< SfxUndoAction* >       m_aUndoActionsCleanup;
        ::std::list< NotifyUndoListener >   m_notifiers;
    };

    UndoManagerGuard::~UndoManagerGuard()
    {
        // copy members
        UndoListeners aListenersCopy( m_rManagerData.aListeners );

        // release mutex
        m_aGuard.clear();

        // delete all actions
        while ( !m_aUndoActionsCleanup.empty() )
        {
            SfxUndoAction* pAction = m_aUndoActionsCleanup.front();
            m_aUndoActionsCleanup.pop_front();
            try
            {
                delete pAction;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        // handle scheduled notification
        for (   ::std::list< NotifyUndoListener >::const_iterator notifier = m_notifiers.begin();
                notifier != m_notifiers.end();
                ++notifier
             )
        {
            if ( notifier->is() )
                ::std::for_each( aListenersCopy.begin(), aListenersCopy.end(), *notifier );
        }
    }
} } }

using namespace ::svl::undo::impl;

//========================================================================

SfxUndoManager::SfxUndoManager( USHORT nMaxUndoActionCount )
    :m_pData( new SfxUndoManager_Data( nMaxUndoActionCount ) )
{
}

//------------------------------------------------------------------------

SfxUndoManager::~SfxUndoManager()
{
    UndoListeners aListenersCopy;
    {
        UndoManagerGuard aGuard( *m_pData );
        aListenersCopy = m_pData->aListeners;
    }

    ::std::for_each( aListenersCopy.begin(), aListenersCopy.end(),
        NotifyUndoListener( &SfxUndoListener::undoManagerDying ) );
}

//------------------------------------------------------------------------

void SfxUndoManager::EnableUndo( bool i_enable )
{
    UndoManagerGuard aGuard( *m_pData );
    ImplEnableUndo_Lock( i_enable );

}

//------------------------------------------------------------------------

void SfxUndoManager::ImplEnableUndo_Lock( bool const i_enable )
{
    if ( !i_enable )
        ++m_pData->mnLockCount;
    else
    {
        OSL_PRECOND( m_pData->mnLockCount > 0, "SfxUndoManager::ImplEnableUndo_NoNotify: not disabled, so why enabling?" );
        if ( m_pData->mnLockCount > 0 )
            --m_pData->mnLockCount;
    }
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsUndoEnabled() const
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplIsUndoEnabled_Lock();
}

//------------------------------------------------------------------------

bool SfxUndoManager::ImplIsUndoEnabled_Lock() const
{
    return m_pData->mnLockCount == 0;
}

//------------------------------------------------------------------------

void SfxUndoManager::SetMaxUndoActionCount( USHORT nMaxUndoActionCount )
{
    UndoManagerGuard aGuard( *m_pData );

    // Remove entries from the pActUndoArray when we have to reduce
    // the number of entries due to a lower nMaxUndoActionCount.
    // Both redo and undo action entries will be removed until we reached the
    // new nMaxUndoActionCount.

    long nNumToDelete = m_pData->pActUndoArray->aUndoActions.size() - nMaxUndoActionCount;
    while ( nNumToDelete > 0 )
    {
        size_t nPos = m_pData->pActUndoArray->aUndoActions.size();
        if ( nPos > m_pData->pActUndoArray->nCurUndoAction )
        {
            SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[nPos-1].pAction;
            if ( !pAction->IsLinked() )
            {
                aGuard.markForDeletion( pAction );
                m_pData->pActUndoArray->aUndoActions.Remove( nPos-1 );
                --nNumToDelete;
            }
        }

        if ( nNumToDelete > 0 && m_pData->pActUndoArray->nCurUndoAction > 0 )
        {
            SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[0].pAction;
            if ( !pAction->IsLinked() )
            {
                aGuard.markForDeletion( pAction );
                m_pData->pActUndoArray->aUndoActions.Remove(0);
                --m_pData->pActUndoArray->nCurUndoAction;
                --nNumToDelete;
            }
        }

        if ( nPos == m_pData->pActUndoArray->aUndoActions.size() )
            break; // Cannot delete more entries
    }

    m_pData->pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetMaxUndoActionCount() const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->pActUndoArray->nMaxUndoActions;
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplClear( UndoManagerGuard& i_guard )
{
    // clear array
    while ( !m_pData->pActUndoArray->aUndoActions.empty() )
    {
        size_t deletePos = m_pData->pActUndoArray->aUndoActions.size() - 1;
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ deletePos ].pAction;
        i_guard.markForDeletion( pAction );
        m_pData->pActUndoArray->aUndoActions.Remove( deletePos );
    }

    m_pData->pActUndoArray->nCurUndoAction = 0;

    m_pData->mnMarks = 0;

    // notify listeners
    i_guard.scheduleNotification( &SfxUndoListener::cleared );
}

//------------------------------------------------------------------------

void SfxUndoManager::Clear()
{
    UndoManagerGuard aGuard( *m_pData );
    ENSURE_OR_RETURN_VOID( !IsInListAction(), "SfxUndoManager::Clear: not allowed when within a list action!" );
    ImplClear( aGuard );
}

//------------------------------------------------------------------------

void SfxUndoManager::ClearRedo()
{
    UndoManagerGuard aGuard( *m_pData );
    ENSURE_OR_RETURN_VOID( !IsInListAction(), "SfxUndoManager::ClearRedo: not allowed when within a list action!" );
    ImplClearRedo( aGuard );
}

//------------------------------------------------------------------------

void SfxUndoManager::Reset()
{
    UndoManagerGuard aGuard( *m_pData );

    // clear all locks
    while ( !ImplIsUndoEnabled_Lock() )
        ImplEnableUndo_Lock( true );

    // cancel all list actions
    while ( IsInListAction() )
        ImplLeaveListAction( false, aGuard );

    // clear both stacks
    ImplClear( aGuard );

    // cancel the notifications scheduled by ImplLeaveListAction resp. ImplClear,
    // as we want to do an own, dedicated notification
    aGuard.cancelNotifications();

    // schedule notification
    aGuard.scheduleNotification( &SfxUndoListener::resetAll );
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplClearUndo( UndoManagerGuard& i_guard )
{
    while ( m_pData->pActUndoArray->nCurUndoAction > 0 )
    {
        SfxUndoAction* pUndoAction = m_pData->pActUndoArray->aUndoActions[0].pAction;
        m_pData->pActUndoArray->aUndoActions.Remove( 0 );
        i_guard.markForDeletion( pUndoAction );
        --m_pData->pActUndoArray->nCurUndoAction;
    }
    // TODO: notifications? We don't have clearedUndo, only cleared and clearedRedo at the SfxUndoListener
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplClearRedo( UndoManagerGuard& i_guard )
{
    // clearance
    while ( m_pData->pActUndoArray->aUndoActions.size() > m_pData->pActUndoArray->nCurUndoAction )
    {
        size_t deletePos = m_pData->pActUndoArray->aUndoActions.size() - 1;
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ deletePos ].pAction;
        m_pData->pActUndoArray->aUndoActions.Remove( deletePos );
        i_guard.markForDeletion( pAction );
    }

    // notification
    i_guard.scheduleNotification( &SfxUndoListener::clearedRedo );
}

//------------------------------------------------------------------------

bool SfxUndoManager::ImplAddUndoAction_NoNotify( SfxUndoAction *pAction, BOOL bTryMerge, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() || ( m_pData->pActUndoArray->nMaxUndoActions == 0 ) )
    {
        i_guard.markForDeletion( pAction );
        return false;
    }

    // Redo-Actions loeschen
    if ( ImplGetRedoActionCount_Lock() > 0 )
        ImplClearRedo( i_guard );

    // merge, if required
    SfxUndoAction* pMergeWithAction = m_pData->pActUndoArray->nCurUndoAction ?
        m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1].pAction : NULL;
    if ( bTryMerge && ( !pMergeWithAction || !pMergeWithAction->Merge( pAction ) ) )
    {
        i_guard.markForDeletion( pAction );
        return false;
    }

    // respect max number
    if( m_pData->pActUndoArray == m_pData->pUndoArray )
    {
        while( m_pData->pActUndoArray->aUndoActions.size() >=
               m_pData->pActUndoArray->nMaxUndoActions &&
               !m_pData->pActUndoArray->aUndoActions[0].pAction->IsLinked() )
        {
            i_guard.markForDeletion( m_pData->pActUndoArray->aUndoActions[0].pAction );
            m_pData->pActUndoArray->aUndoActions.Remove(0);
            --m_pData->pActUndoArray->nCurUndoAction;
        }
    }

    // append new action
    m_pData->pActUndoArray->aUndoActions.Insert( pAction, m_pData->pActUndoArray->nCurUndoAction++ );
    return true;
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerge )
{
    UndoManagerGuard aGuard( *m_pData );

    // add
    if ( ImplAddUndoAction_NoNotify( pAction, bTryMerge, aGuard ) )
    {
        // notify listeners
        aGuard.scheduleNotification( &SfxUndoListener::undoActionAdded, pAction->GetComment() );
    }
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( USHORT nNo, bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );

    String sComment;
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    DBG_ASSERT( nNo < pUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionComment: illegal index!" );
    if( nNo < pUndoArray->nCurUndoAction )
    {
        sComment = pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction - 1 - nNo ].pAction->GetComment();
    }
    return sComment;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionId() const
{
    UndoManagerGuard aGuard( *m_pData );

    DBG_ASSERT( m_pData->pActUndoArray->nCurUndoAction > 0, "svl::SfxUndoManager::GetUndoActionId(), illegal id!" );
    if ( m_pData->pActUndoArray->nCurUndoAction == 0 )
        return NULL;
    return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1].pAction->GetId();
}

//------------------------------------------------------------------------

SfxUndoAction* SfxUndoManager::GetUndoAction( USHORT nNo ) const
{
    UndoManagerGuard aGuard( *m_pData );

    DBG_ASSERT( nNo < m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoAction(), illegal id!" );
    if( nNo >= m_pData->pActUndoArray->nCurUndoAction )
        return NULL;
    return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1-nNo].pAction;
}

//------------------------------------------------------------------------

/** clears the redo stack and removes the top undo action */
void SfxUndoManager::RemoveLastUndoAction()
{
    UndoManagerGuard aGuard( *m_pData );

    ENSURE_OR_RETURN_VOID( m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::RemoveLastUndoAction(), no action to remove?!" );

    m_pData->pActUndoArray->nCurUndoAction--;

    // delete redo-actions and top action
    for ( size_t nPos = m_pData->pActUndoArray->aUndoActions.size(); nPos > m_pData->pActUndoArray->nCurUndoAction; --nPos )
    {
        aGuard.markForDeletion( m_pData->pActUndoArray->aUndoActions[nPos-1].pAction );
    }

    m_pData->pActUndoArray->aUndoActions.Remove(
        m_pData->pActUndoArray->nCurUndoAction,
        m_pData->pActUndoArray->aUndoActions.size() - m_pData->pActUndoArray->nCurUndoAction );
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsDoing() const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->mbDoing;
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Undo()
{
    UndoManagerGuard aGuard( *m_pData );
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Undo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aDoingGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( ImplIsInListAction_Lock() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: not possible when within a list action!" );
        return FALSE;
    }

    if ( m_pData->pActUndoArray->nCurUndoAction == 0 )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: undo stack is empty!" );
        return FALSE;
    }

    SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ --m_pData->pActUndoArray->nCurUndoAction ].pAction;
    const String sActionComment = pAction->GetComment();
    try
    {
        ::std::auto_ptr< SfxUndoContext > pUndoContext( GetUndoContext() );
        // clear the guard/mutex before calling into the SfxUndoAction - this can be an extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( pUndoContext.get() != NULL )
            pAction->UndoWithContext( *pUndoContext );
        else
            pAction->Undo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_pData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        USHORT nCurAction = 0;
        while ( nCurAction < m_pData->pActUndoArray->aUndoActions.size() )
        {
            if ( m_pData->pActUndoArray->aUndoActions[ nCurAction++ ].pAction == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearUndo( aGuard );
                throw;
            }
        }
        OSL_ENSURE( false, "SfxUndoManager::Undo: can't clear the Undo stack after the failure - some other party was faster ..." );
        throw;
    }

    aGuard.scheduleNotification( &SfxUndoListener::actionUndone, sActionComment );

    return TRUE;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRedoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplGetRedoActionCount_Lock( i_currentLevel );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::ImplGetRedoActionCount_Lock( bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return USHORT( pUndoArray->aUndoActions.size() - pUndoArray->nCurUndoAction );
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( USHORT nNo, bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction + nNo ].pAction->GetComment();
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Redo()
{
    UndoManagerGuard aGuard( *m_pData );
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Redo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aDoingGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( ImplIsInListAction_Lock() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Redo: not possible when within a list action!" );
        return FALSE;
    }

    if ( m_pData->pActUndoArray->nCurUndoAction >= m_pData->pActUndoArray->aUndoActions.size() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Redo: redo stack is empty!" );
        return FALSE;
    }

    SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction++ ].pAction;
    const String sActionComment = pAction->GetComment();
    try
    {
        ::std::auto_ptr< SfxUndoContext > pUndoContext( GetUndoContext() );
        // clear the guard/mutex before calling into the SfxUndoAction - this can be a extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( pUndoContext.get() != NULL )
            pAction->RedoWithContext( *pUndoContext );
        else
            pAction->Redo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_pData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        USHORT nCurAction = 0;
        while ( nCurAction < m_pData->pActUndoArray->aUndoActions.size() )
        {
            if ( m_pData->pActUndoArray->aUndoActions[ nCurAction ].pAction == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearRedo( aGuard );
                throw;
            }
            ++nCurAction;
        }
        OSL_ENSURE( false, "SfxUndoManager::Redo: can't clear the Undo stack after the failure - some other party was faster ..." );
        throw;
    }

    aGuard.scheduleNotification( &SfxUndoListener::actionRedone, sActionComment );

    return TRUE;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRepeatActionCount() const
{
    UndoManagerGuard aGuard( *m_pData );
    return USHORT( m_pData->pActUndoArray->aUndoActions.size() );
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget) const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.size() - 1 ].pAction
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Repeat( SfxRepeatTarget &rTarget )
{
    UndoManagerGuard aGuard( *m_pData );
    if ( !m_pData->pActUndoArray->aUndoActions.empty() )
    {
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.size() - 1 ].pAction;
        aGuard.clear();
        if ( pAction->CanRepeat( rTarget ) )
            pAction->Repeat( rTarget );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget ) const
{
    UndoManagerGuard aGuard( *m_pData );
    if ( !m_pData->pActUndoArray->aUndoActions.empty() )
    {
        size_t nActionNo = m_pData->pActUndoArray->aUndoActions.size() - 1;
        return m_pData->pActUndoArray->aUndoActions[nActionNo].pAction->CanRepeat(rTarget);
    }
    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoListener( SfxUndoListener& i_listener )
{
    UndoManagerGuard aGuard( *m_pData );
    m_pData->aListeners.push_back( &i_listener );
}

//------------------------------------------------------------------------

void SfxUndoManager::RemoveUndoListener( SfxUndoListener& i_listener )
{
    UndoManagerGuard aGuard( *m_pData );
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
    UndoManagerGuard aGuard( *m_pData );

    if( !ImplIsUndoEnabled_Lock() )
        return;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return;

    m_pData->pFatherUndoArray = m_pData->pActUndoArray;
    SfxListUndoAction* pAction = new SfxListUndoAction( rComment, rRepeatComment, nId, m_pData->pActUndoArray );
    ImplAddUndoAction_NoNotify( pAction, FALSE, aGuard );
    m_pData->pActUndoArray=pAction;

    // notification
    aGuard.scheduleNotification( &SfxUndoListener::listActionEntered, rComment );
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsInListAction() const
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplIsInListAction_Lock();
}

//------------------------------------------------------------------------

bool SfxUndoManager::ImplIsInListAction_Lock() const
{
    return ( m_pData->pActUndoArray != m_pData->pUndoArray );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetListActionDepth() const
{
    UndoManagerGuard aGuard( *m_pData );
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
    UndoManagerGuard aGuard( *m_pData );
    return ImplLeaveListAction( false, aGuard );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::LeaveAndMergeListAction()
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplLeaveListAction( true, aGuard );
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::ImplLeaveListAction( const bool i_merge, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() )
        return 0;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return 0;

    if( !ImplIsInListAction_Lock() )
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
    if ( nListActionElements == 0 )
    {
        SfxUndoAction* pCurrentAction= m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction-1 ].pAction;
        m_pData->pActUndoArray->aUndoActions.Remove( --m_pData->pActUndoArray->nCurUndoAction );
        i_guard.markForDeletion( pCurrentAction );

        i_guard.scheduleNotification( &SfxUndoListener::listActionCancelled );
        return 0;
    }


    SfxUndoAction* pCurrentAction= m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction-1 ].pAction;
    SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction * >( pCurrentAction );
    ENSURE_OR_RETURN( pListAction, "SfxUndoManager::ImplLeaveListAction: list action expected at this position!", nListActionElements );

    if ( i_merge )
    {
        // merge the list action with its predecessor on the same level
        OSL_ENSURE( m_pData->pActUndoArray->nCurUndoAction > 1,
            "SfxUndoManager::ImplLeaveListAction: cannot merge the list action if there's no other action on the same level - check this beforehand!" );
        if ( m_pData->pActUndoArray->nCurUndoAction > 1 )
        {
            SfxUndoAction* pPreviousAction = m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction - 2 ].pAction;
            m_pData->pActUndoArray->aUndoActions.Remove( m_pData->pActUndoArray->nCurUndoAction - 2 );
            --m_pData->pActUndoArray->nCurUndoAction;
            pListAction->aUndoActions.Insert( pPreviousAction, 0 );
            ++pListAction->nCurUndoAction;

            pListAction->SetComment( pPreviousAction->GetComment() );
        }
    }

    // if the undo array has no comment, try to get it from its children
    if ( pListAction->GetComment().Len() == 0 )
    {
        for( USHORT n = 0; n < pListAction->aUndoActions.size(); n++ )
        {
            if( pListAction->aUndoActions[n].pAction->GetComment().Len() )
            {
                pListAction->SetComment( pListAction->aUndoActions[n].pAction->GetComment() );
                break;
            }
        }
    }

    // notify listeners
    i_guard.scheduleNotification( &SfxUndoListener::listActionLeft, pListAction->GetComment() );

    // outta here
    return nListActionElements;
}

//------------------------------------------------------------------------
UndoStackMark SfxUndoManager::MarkTopUndoAction()
{
    UndoManagerGuard aGuard( *m_pData );

    USHORT nActionPos = m_pData->pUndoArray->nCurUndoAction;
    ENSURE_OR_RETURN( nActionPos > 0, "SfxUndoManager::MarkTopUndoAction: undo stack is empty!", MARK_INVALID );

    OSL_ENSURE( !IsInListAction(), "SfxUndoManager::MarkTopUndoAction: suspicious call!" );

    m_pData->pUndoArray->aUndoActions[ nActionPos ].aMarks.push_back( ++m_pData->mnMarks );
    return m_pData->mnMarks;
}

//------------------------------------------------------------------------
void SfxUndoManager::RemoveMark( UndoStackMark const i_mark )
{
    UndoManagerGuard aGuard( *m_pData );

    for ( size_t i=0; i<m_pData->pUndoArray->aUndoActions.size(); ++i )
    {
        MarkedUndoAction& rAction = m_pData->pUndoArray->aUndoActions[i];
        for (   ::std::vector< UndoStackMark >::iterator markPos = rAction.aMarks.begin();
                markPos != rAction.aMarks.end();
                ++markPos
            )
        {
            if ( *markPos == i_mark )
            {
                rAction.aMarks.erase( markPos );
                return;
            }
        }
    }
    OSL_ENSURE( false, "SfxUndoManager::RemoveMark: mark not found!" );
        // TODO: this might be too offensive. There are situations where we implicitly remove marks
        // without our clients, in particular the client which created the mark, having a chance to know
        // about this.
}

//------------------------------------------------------------------------
bool SfxUndoManager::HasTopUndoActionMark( UndoStackMark const i_mark )
{
    UndoManagerGuard aGuard( *m_pData );

    USHORT nActionPos = m_pData->pUndoArray->nCurUndoAction;
    if ( nActionPos == 0 )
        return false;

    const MarkedUndoAction& rAction = m_pData->pUndoArray->aUndoActions[ nActionPos ];
    for (   ::std::vector< UndoStackMark >::const_iterator markPos = rAction.aMarks.begin();
            markPos != rAction.aMarks.end();
            ++markPos
        )
    {
        if ( *markPos == i_mark )
            return true;
    }

    return false;
}

//------------------------------------------------------------------------

void SfxUndoManager::RemoveOldestUndoActions( USHORT const i_count )
{
    UndoManagerGuard aGuard( *m_pData );

    size_t nActionsToRemove = i_count;
    while ( nActionsToRemove )
    {
        SfxUndoAction* pActionToRemove = m_pData->pUndoArray->aUndoActions[0].pAction;

        if ( IsInListAction() && ( m_pData->pUndoArray->nCurUndoAction == 1 ) )
        {
            OSL_ENSURE( false, "SfxUndoManager::RemoveOldestUndoActions: cannot remove a not-yet-closed list action!" );
            return;
        }

        aGuard.markForDeletion( pActionToRemove );
        m_pData->pUndoArray->aUndoActions.Remove( 0 );
        --m_pData->pUndoArray->nCurUndoAction;
        --nActionsToRemove;
    }
}

//------------------------------------------------------------------------

::std::auto_ptr< SfxUndoContext > SfxUndoManager::GetUndoContext()
{
    return ::std::auto_ptr< SfxUndoContext >();
}

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
        aUndoActions[i].pAction->Undo();
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    for(INT16 i=nCurUndoAction-1;i>=0;i--)
        aUndoActions[i].pAction->UndoWithContext( i_context );
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::Redo()
{
    for(USHORT i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->Redo();
    nCurUndoAction = USHORT( aUndoActions.size() );
}

//------------------------------------------------------------------------

void SfxListUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    for(USHORT i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->RedoWithContext( i_context );
    nCurUndoAction = USHORT( aUndoActions.size() );
}

//------------------------------------------------------------------------

void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        aUndoActions[i].pAction->Repeat(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        if(!aUndoActions[i].pAction->CanRepeat(r))
            return FALSE;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
{
    return !aUndoActions.empty() && aUndoActions[aUndoActions.size()-1].pAction->Merge( pNextAction );
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
        USHORT nPos = pManager->GetUndoActionCount()-1;
        pAction = pUndoManagerImplementation->m_pData->pActUndoArray->aUndoActions[nPos].pAction;
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
    while ( !aUndoActions.empty() )
    {
        SfxUndoAction *pAction = aUndoActions[ aUndoActions.size() - 1 ].pAction;
        aUndoActions.Remove( aUndoActions.size() - 1 );
        delete pAction;
    }
}


USHORT SfxLinkUndoAction::GetId() const
{
      return pAction ? pAction->GetId() : 0;
}
