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

sal_Bool SfxUndoAction::IsLinked()
{
    return bLinked;
}

//------------------------------------------------------------------------

void SfxUndoAction::SetLinked( sal_Bool bIsLinked )
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
    SetLinked( sal_False );
}

//------------------------------------------------------------------------

sal_Bool SfxUndoAction::Merge( SfxUndoAction * )
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return sal_False;
}

//------------------------------------------------------------------------

XubString SfxUndoAction::GetComment() const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return XubString();
}

//------------------------------------------------------------------------


sal_uInt16 SfxUndoAction::GetId() const
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


sal_Bool SfxUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return sal_True;
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
    sal_Int32       mnEmptyMark;
    bool            mbDoing;
    bool            mbClearUntilTopLevel;

    UndoListeners   aListeners;

    SfxUndoManager_Data( size_t i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( NULL )
        ,pFatherUndoArray( NULL )
        ,mnLockCount( 0 )
        ,mnMarks( 0 )
        ,mnEmptyMark(MARK_INVALID)
        ,mbDoing( false )
        ,mbClearUntilTopLevel( false )
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

SfxUndoManager::SfxUndoManager( size_t nMaxUndoActionCount )
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

void SfxUndoManager::SetMaxUndoActionCount( size_t nMaxUndoActionCount )
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

size_t SfxUndoManager::GetMaxUndoActionCount() const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->pActUndoArray->nMaxUndoActions;
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplClearCurrentLevel_NoNotify( UndoManagerGuard& i_guard )
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
    m_pData->mnEmptyMark = MARK_INVALID;
}

//------------------------------------------------------------------------

void SfxUndoManager::Clear()
{
    UndoManagerGuard aGuard( *m_pData );

    OSL_ENSURE( !ImplIsInListAction_Lock(), "SfxUndoManager::Clear: suspicious call - do you really wish to clear the current level?" );
    ImplClearCurrentLevel_NoNotify( aGuard );

    // notify listeners
    aGuard.scheduleNotification( &SfxUndoListener::cleared );
}

//------------------------------------------------------------------------

void SfxUndoManager::ClearAllLevels()
{
    UndoManagerGuard aGuard( *m_pData );
    ImplClearCurrentLevel_NoNotify( aGuard );

    if ( ImplIsInListAction_Lock() )
    {
        m_pData->mbClearUntilTopLevel = true;
    }
    else
    {
        aGuard.scheduleNotification( &SfxUndoListener::cleared );
    }
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplClearRedo_NoLock( bool const i_currentLevel )
{
    UndoManagerGuard aGuard( *m_pData );
    ImplClearRedo( aGuard, i_currentLevel );
}

//------------------------------------------------------------------------

void SfxUndoManager::ClearRedo()
{
    OSL_ENSURE( !IsInListAction(), "SfxUndoManager::ClearRedo: suspicious call - do you really wish to clear the current level?" );
    ImplClearRedo_NoLock( CurrentLevel );
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
    ImplClearCurrentLevel_NoNotify( aGuard );

    // cancel the notifications scheduled by ImplLeaveListAction,
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

void SfxUndoManager::ImplClearRedo( UndoManagerGuard& i_guard, bool const i_currentLevel )
{
    SfxUndoArray* pUndoArray = ( i_currentLevel == IUndoManager::CurrentLevel ) ? m_pData->pActUndoArray : m_pData->pUndoArray;

    // clearance
    while ( pUndoArray->aUndoActions.size() > pUndoArray->nCurUndoAction )
    {
        size_t deletePos = pUndoArray->aUndoActions.size() - 1;
        SfxUndoAction* pAction = pUndoArray->aUndoActions[ deletePos ].pAction;
        pUndoArray->aUndoActions.Remove( deletePos );
        i_guard.markForDeletion( pAction );
    }

    // notification - only if the top level's stack was cleared
    if ( i_currentLevel == IUndoManager::TopLevel )
        i_guard.scheduleNotification( &SfxUndoListener::clearedRedo );
}

//------------------------------------------------------------------------

bool SfxUndoManager::ImplAddUndoAction_NoNotify( SfxUndoAction *pAction, bool bTryMerge, bool bClearRedo, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() || ( m_pData->pActUndoArray->nMaxUndoActions == 0 ) )
    {
        i_guard.markForDeletion( pAction );
        return false;
    }

    // merge, if required
    SfxUndoAction* pMergeWithAction = m_pData->pActUndoArray->nCurUndoAction ?
        m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1].pAction : NULL;
    if ( bTryMerge && ( !pMergeWithAction || !pMergeWithAction->Merge( pAction ) ) )
    {
        i_guard.markForDeletion( pAction );
        return false;
    }

    // clear redo stack, if requested
    if ( bClearRedo && ( ImplGetRedoActionCount_Lock( CurrentLevel ) > 0 ) )
        ImplClearRedo( i_guard, IUndoManager::CurrentLevel );

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

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerge )
{
    UndoManagerGuard aGuard( *m_pData );

    // add
    if ( ImplAddUndoAction_NoNotify( pAction, bTryMerge, true, aGuard ) )
    {
        // notify listeners
        aGuard.scheduleNotification( &SfxUndoListener::undoActionAdded, pAction->GetComment() );
    }
}

//------------------------------------------------------------------------

size_t SfxUndoManager::GetUndoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( size_t nNo, bool const i_currentLevel ) const
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

sal_uInt16 SfxUndoManager::GetUndoActionId() const
{
    UndoManagerGuard aGuard( *m_pData );

    DBG_ASSERT( m_pData->pActUndoArray->nCurUndoAction > 0, "svl::SfxUndoManager::GetUndoActionId(), illegal id!" );
    if ( m_pData->pActUndoArray->nCurUndoAction == 0 )
        return NULL;
    return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1].pAction->GetId();
}

//------------------------------------------------------------------------

SfxUndoAction* SfxUndoManager::GetUndoAction( size_t nNo ) const
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
    return ImplUndo( NULL );
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::UndoWithContext( SfxUndoContext& i_context )
{
    return ImplUndo( &i_context );
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::ImplUndo( SfxUndoContext* i_contextOrNull )
{
    UndoManagerGuard aGuard( *m_pData );
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Undo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aDoingGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( ImplIsInListAction_Lock() )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: not possible when within a list action!" );
        return sal_False;
    }

    if ( m_pData->pActUndoArray->nCurUndoAction == 0 )
    {
        OSL_ENSURE( false, "SfxUndoManager::Undo: undo stack is empty!" );
        return sal_False;
    }

    SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ --m_pData->pActUndoArray->nCurUndoAction ].pAction;
    const String sActionComment = pAction->GetComment();
    try
    {
        // clear the guard/mutex before calling into the SfxUndoAction - this can be an extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( i_contextOrNull != NULL )
            pAction->UndoWithContext( *i_contextOrNull );
        else
            pAction->Undo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_pData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        size_t nCurAction = 0;
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

size_t SfxUndoManager::GetRedoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplGetRedoActionCount_Lock( i_currentLevel );
}

//------------------------------------------------------------------------

size_t SfxUndoManager::ImplGetRedoActionCount_Lock( bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->aUndoActions.size() - pUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( size_t nNo, bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_pData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_pData->pActUndoArray : m_pData->pUndoArray;
    return pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction + nNo ].pAction->GetComment();
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::Redo()
{
    return ImplRedo( NULL );
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::RedoWithContext( SfxUndoContext& i_context )
{
    return ImplRedo( &i_context );
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::ImplRedo( SfxUndoContext* i_contextOrNull )
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
        // clear the guard/mutex before calling into the SfxUndoAction - this can be a extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( i_contextOrNull != NULL )
            pAction->RedoWithContext( *i_contextOrNull );
        else
            pAction->Redo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_pData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        size_t nCurAction = 0;
        while ( nCurAction < m_pData->pActUndoArray->aUndoActions.size() )
        {
            if ( m_pData->pActUndoArray->aUndoActions[ nCurAction ].pAction == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearRedo( aGuard, IUndoManager::CurrentLevel );
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

size_t SfxUndoManager::GetRepeatActionCount() const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->pActUndoArray->aUndoActions.size();
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget) const
{
    UndoManagerGuard aGuard( *m_pData );
    return m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.size() - 1 ].pAction
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, sal_uInt16 /*nFrom*/, sal_uInt16 /*nCount*/ )
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

    return sal_False;
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
    const XubString& rComment, const XubString &rRepeatComment, sal_uInt16 nId )

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
    OSL_VERIFY( ImplAddUndoAction_NoNotify( pAction, false, false, aGuard ) );
        // expected to succeed: all conditions under which it could fail should have been checked already
    m_pData->pActUndoArray = pAction;

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

size_t SfxUndoManager::GetListActionDepth() const
{
    UndoManagerGuard aGuard( *m_pData );
    size_t nDepth(0);

    SfxUndoArray* pLookup( m_pData->pActUndoArray );
    while ( pLookup != m_pData->pUndoArray )
    {
        pLookup = pLookup->pFatherUndoArray;
        ++nDepth;
    }

    return nDepth;
}

//------------------------------------------------------------------------

size_t SfxUndoManager::LeaveListAction()
{
    UndoManagerGuard aGuard( *m_pData );
    size_t nCount = ImplLeaveListAction( false, aGuard );

    if ( m_pData->mbClearUntilTopLevel )
    {
        ImplClearCurrentLevel_NoNotify( aGuard );
        if ( !ImplIsInListAction_Lock() )
        {
            m_pData->mbClearUntilTopLevel = false;
            aGuard.scheduleNotification( &SfxUndoListener::cleared );
        }
        nCount = 0;
    }

    return nCount;
}

//------------------------------------------------------------------------

size_t SfxUndoManager::LeaveAndMergeListAction()
{
    UndoManagerGuard aGuard( *m_pData );
    return ImplLeaveListAction( true, aGuard );
}

//------------------------------------------------------------------------

size_t SfxUndoManager::ImplLeaveListAction( const bool i_merge, UndoManagerGuard& i_guard )
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

    DBG_ASSERT( m_pData->pActUndoArray->pFatherUndoArray, "SfxUndoManager::ImplLeaveListAction, no father undo array!?" );

    // the array/level which we're about to leave
    SfxUndoArray* pArrayToLeave = m_pData->pActUndoArray;
    // one step up
    m_pData->pActUndoArray = m_pData->pActUndoArray->pFatherUndoArray;

    // If no undo actions were added to the list, delete the list action
    const size_t nListActionElements = pArrayToLeave->nCurUndoAction;
    if ( nListActionElements == 0 )
    {
        SfxUndoAction* pCurrentAction= m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->nCurUndoAction-1 ].pAction;
        m_pData->pActUndoArray->aUndoActions.Remove( --m_pData->pActUndoArray->nCurUndoAction );
        i_guard.markForDeletion( pCurrentAction );

        i_guard.scheduleNotification( &SfxUndoListener::listActionCancelled );
        return 0;
    }

    // now that it is finally clear the list action is non-trivial, and does participate in the Undo stack, clear
    // the redo stack
    ImplClearRedo( i_guard, IUndoManager::CurrentLevel );

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
        for( size_t n = 0; n < pListAction->aUndoActions.size(); n++ )
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

    OSL_ENSURE( !IsInListAction(),
            "SfxUndoManager::MarkTopUndoAction(): suspicious call!" );
    OSL_ENSURE((m_pData->mnMarks + 1) < (m_pData->mnEmptyMark - 1),
            "SfxUndoManager::MarkTopUndoAction(): mark overflow!");

    size_t const nActionPos = m_pData->pUndoArray->nCurUndoAction;
    if (0 == nActionPos)
    {
        --m_pData->mnEmptyMark;
        return m_pData->mnEmptyMark;
    }

    m_pData->pUndoArray->aUndoActions[ nActionPos-1 ].aMarks.push_back(
            ++m_pData->mnMarks );
    return m_pData->mnMarks;
}

//------------------------------------------------------------------------
void SfxUndoManager::RemoveMark( UndoStackMark const i_mark )
{
    UndoManagerGuard aGuard( *m_pData );

    if ((m_pData->mnEmptyMark < i_mark) || (MARK_INVALID == i_mark))
    {
        return; // nothing to remove
    }
    else if (i_mark == m_pData->mnEmptyMark)
    {
        --m_pData->mnEmptyMark; // never returned from MarkTop => invalid
        return;
    }

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

    size_t nActionPos = m_pData->pUndoArray->nCurUndoAction;
    if ( nActionPos == 0 )
    {
        return (i_mark == m_pData->mnEmptyMark);
    }

    const MarkedUndoAction& rAction =
            m_pData->pUndoArray->aUndoActions[ nActionPos-1 ];
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

void SfxUndoManager::RemoveOldestUndoActions( size_t const i_count )
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

sal_uInt16 SfxListUndoAction::GetId() const
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
    sal_uInt16 Id,
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
    for(size_t i=nCurUndoAction;i>0;)
        aUndoActions[--i].pAction->Undo();
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i>0;)
        aUndoActions[--i].pAction->UndoWithContext( i_context );
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::Redo()
{
    for(size_t i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->Redo();
    nCurUndoAction = aUndoActions.size();
}

//------------------------------------------------------------------------

void SfxListUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->RedoWithContext( i_context );
    nCurUndoAction = aUndoActions.size();
}

//------------------------------------------------------------------------

void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(size_t i=0;i<nCurUndoAction;i++)
        aUndoActions[i].pAction->Repeat(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(size_t i=0;i<nCurUndoAction;i++)
        if(!aUndoActions[i].pAction->CanRepeat(r))
            return sal_False;
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
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
        size_t nPos = pManager->GetUndoActionCount()-1;
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


sal_Bool SfxLinkUndoAction::CanRepeat(SfxRepeatTarget& r) const
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
        pAction->SetLinked( sal_False );
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


sal_uInt16 SfxLinkUndoAction::GetId() const
{
      return pAction ? pAction->GetId() : 0;
}
