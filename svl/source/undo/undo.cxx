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

#include <svl/undo.hxx>

#include <com/sun/star/uno/Exception.hpp>

#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <comphelper/flagguard.hxx>
#include <tools/diagnose_ex.h>
#include <libxml/xmlwriter.h>

#include <vector>
#include <list>
#include <limits.h>
#include <algorithm>

using ::com::sun::star::uno::Exception;


SfxRepeatTarget::~SfxRepeatTarget()
{
}


SfxUndoContext::~SfxUndoContext()
{
}


SfxUndoAction::~SfxUndoAction()
{
}


SfxUndoAction::SfxUndoAction()
{
}


bool SfxUndoAction::Merge( SfxUndoAction * )
{
    return false;
}


OUString SfxUndoAction::GetComment() const
{
    return OUString();
}


sal_uInt16 SfxUndoAction::GetId() const
{
    return 0;
}

sal_Int32 SfxUndoAction::GetViewShellId() const
{
    return -1;
}

OUString SfxUndoAction::GetRepeatComment(SfxRepeatTarget&) const
{
    return GetComment();
}


void SfxUndoAction::Undo()
{
    // These are only conceptually pure virtual
    assert(!"pure virtual function called: SfxUndoAction::Undo()");
}


void SfxUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    (void)i_context;
    Undo();
}


void SfxUndoAction::Redo()
{
    // These are only conceptually pure virtual
    assert(!"pure virtual function called: SfxUndoAction::Redo()");
}


void SfxUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    (void)i_context;
    Redo();
}


void SfxUndoAction::Repeat(SfxRepeatTarget&)
{
    // These are only conceptually pure virtual
    assert(!"pure virtual function called: SfxUndoAction::Repeat()");
}


bool SfxUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return true;
}

void SfxUndoAction::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sfxUndoAction"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("symbol"), BAD_CAST(typeid(*this).name()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("comment"), BAD_CAST(GetComment().toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("viewShellId"), BAD_CAST(OString::number(GetViewShellId()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

struct MarkedUndoAction
{
    SfxUndoAction*                  pAction;
    ::std::vector< UndoStackMark >  aMarks;

    explicit MarkedUndoAction( SfxUndoAction* i_action )
        :pAction( i_action )
        ,aMarks()
    {
    }
};

struct SfxUndoActions::Impl
{
    std::vector<MarkedUndoAction> maActions;
};

SfxUndoActions::SfxUndoActions() : mpImpl(new Impl) {}

SfxUndoActions::SfxUndoActions( const SfxUndoActions& r ) :
    mpImpl(new Impl)
{
    mpImpl->maActions = r.mpImpl->maActions;
}

SfxUndoActions::~SfxUndoActions()
{
}

bool SfxUndoActions::empty() const
{
    return mpImpl->maActions.empty();
}

size_t SfxUndoActions::size() const
{
    return mpImpl->maActions.size();
}

const MarkedUndoAction& SfxUndoActions::operator[]( size_t i ) const
{
    return mpImpl->maActions[i];
}

MarkedUndoAction& SfxUndoActions::operator[]( size_t i )
{
    return mpImpl->maActions[i];
}

const SfxUndoAction* SfxUndoActions::GetUndoAction( size_t i ) const
{
    return mpImpl->maActions[i].pAction;
}

SfxUndoAction* SfxUndoActions::GetUndoAction( size_t i )
{
    return mpImpl->maActions[i].pAction;
}

void SfxUndoActions::Remove( size_t i_pos )
{
    mpImpl->maActions.erase( mpImpl->maActions.begin() + i_pos );
}

void SfxUndoActions::Remove( size_t i_pos, size_t i_count )
{
    mpImpl->maActions.erase(
        mpImpl->maActions.begin() + i_pos, mpImpl->maActions.begin() + i_pos + i_count);
}

void SfxUndoActions::Insert( SfxUndoAction* i_action, size_t i_pos )
{
    mpImpl->maActions.insert(
        mpImpl->maActions.begin() + i_pos, MarkedUndoAction( i_action ) );
}

typedef ::std::vector< SfxUndoListener* >   UndoListeners;

struct SVL_DLLPRIVATE SfxUndoManager_Data
{
    ::osl::Mutex    aMutex;
    SfxUndoArray*   pUndoArray;
    SfxUndoArray*   pActUndoArray;
    SfxUndoArray*   pFatherUndoArray;

    sal_Int32       mnMarks;
    sal_Int32       mnEmptyMark;
    bool            mbUndoEnabled;
    bool            mbDoing;
    bool            mbClearUntilTopLevel;

    UndoListeners   aListeners;

    explicit SfxUndoManager_Data( size_t i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( nullptr )
        ,pFatherUndoArray( nullptr )
        ,mnMarks( 0 )
        ,mnEmptyMark(MARK_INVALID)
        ,mbUndoEnabled( true )
        ,mbDoing( false )
        ,mbClearUntilTopLevel( false )
    {
        pActUndoArray = pUndoArray;
    }

    ~SfxUndoManager_Data()
    {
        delete pUndoArray;
    }

    // Copy assignment is forbidden and not implemented.
    SfxUndoManager_Data (const SfxUndoManager_Data &) = delete;
    SfxUndoManager_Data & operator= (const SfxUndoManager_Data &) = delete;
};

namespace svl { namespace undo { namespace impl
{
    class SVL_DLLPRIVATE LockGuard
    {
    public:
        explicit LockGuard( SfxUndoManager& i_manager )
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

    typedef void ( SfxUndoListener::*UndoListenerVoidMethod )();
    typedef void ( SfxUndoListener::*UndoListenerStringMethod )( const OUString& );

    struct SVL_DLLPRIVATE NotifyUndoListener : public ::std::unary_function< SfxUndoListener*, void >
    {
        explicit NotifyUndoListener( UndoListenerVoidMethod i_notificationMethod )
            :m_notificationMethod( i_notificationMethod )
            ,m_altNotificationMethod( nullptr )
            ,m_sActionComment()
        {
        }

        NotifyUndoListener( UndoListenerStringMethod i_notificationMethod, const OUString& i_actionComment )
            :m_notificationMethod( nullptr )
            ,m_altNotificationMethod( i_notificationMethod )
            ,m_sActionComment( i_actionComment )
        {
        }

        bool is() const
        {
            return ( m_notificationMethod != nullptr ) || ( m_altNotificationMethod != nullptr );
        }

        void operator()( SfxUndoListener* i_listener ) const
        {
            assert( is() && "NotifyUndoListener: this will crash!" );
            if ( m_altNotificationMethod != nullptr )
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
        OUString                    m_sActionComment;
    };

    class SVL_DLLPRIVATE UndoManagerGuard
    {
    public:
        explicit UndoManagerGuard( SfxUndoManager_Data& i_managerData )
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

        void    scheduleNotification( UndoListenerStringMethod i_notificationMethod, const OUString& i_actionComment )
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
            delete pAction;
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


SfxUndoManager::SfxUndoManager( size_t nMaxUndoActionCount )
    :m_xData( new SfxUndoManager_Data( nMaxUndoActionCount ) )
{
}


SfxUndoManager::~SfxUndoManager()
{
    UndoListeners aListenersCopy;
    {
        UndoManagerGuard aGuard( *m_xData );
        aListenersCopy = m_xData->aListeners;
    }

    ::std::for_each( aListenersCopy.begin(), aListenersCopy.end(),
        NotifyUndoListener( &SfxUndoListener::undoManagerDying ) );
}


void SfxUndoManager::EnableUndo( bool i_enable )
{
    UndoManagerGuard aGuard( *m_xData );
    ImplEnableUndo_Lock( i_enable );

}


void SfxUndoManager::ImplEnableUndo_Lock( bool const i_enable )
{
    if ( m_xData->mbUndoEnabled == i_enable )
        return;
    m_xData->mbUndoEnabled = i_enable;
}


bool SfxUndoManager::IsUndoEnabled() const
{
    UndoManagerGuard aGuard( *m_xData );
    return ImplIsUndoEnabled_Lock();
}


bool SfxUndoManager::ImplIsUndoEnabled_Lock() const
{
    return m_xData->mbUndoEnabled;
}


void SfxUndoManager::SetMaxUndoActionCount( size_t nMaxUndoActionCount )
{
    UndoManagerGuard aGuard( *m_xData );

    // Remove entries from the pActUndoArray when we have to reduce
    // the number of entries due to a lower nMaxUndoActionCount.
    // Both redo and undo action entries will be removed until we reached the
    // new nMaxUndoActionCount.

    long nNumToDelete = m_xData->pActUndoArray->aUndoActions.size() - nMaxUndoActionCount;
    while ( nNumToDelete > 0 )
    {
        size_t nPos = m_xData->pActUndoArray->aUndoActions.size();
        if ( nPos > m_xData->pActUndoArray->nCurUndoAction )
        {
            SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[nPos-1].pAction;
            aGuard.markForDeletion( pAction );
            m_xData->pActUndoArray->aUndoActions.Remove( nPos-1 );
            --nNumToDelete;
        }

        if ( nNumToDelete > 0 && m_xData->pActUndoArray->nCurUndoAction > 0 )
        {
            SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[0].pAction;
            aGuard.markForDeletion( pAction );
            m_xData->pActUndoArray->aUndoActions.Remove(0);
            --m_xData->pActUndoArray->nCurUndoAction;
            --nNumToDelete;
        }

        if ( nPos == m_xData->pActUndoArray->aUndoActions.size() )
            break; // Cannot delete more entries
    }

    m_xData->pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
}


void SfxUndoManager::ImplClearCurrentLevel_NoNotify( UndoManagerGuard& i_guard )
{
    // clear array
    while ( !m_xData->pActUndoArray->aUndoActions.empty() )
    {
        size_t deletePos = m_xData->pActUndoArray->aUndoActions.size() - 1;
        SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[ deletePos ].pAction;
        i_guard.markForDeletion( pAction );
        m_xData->pActUndoArray->aUndoActions.Remove( deletePos );
    }

    m_xData->pActUndoArray->nCurUndoAction = 0;

    m_xData->mnMarks = 0;
    m_xData->mnEmptyMark = MARK_INVALID;
}


void SfxUndoManager::Clear()
{
    UndoManagerGuard aGuard( *m_xData );

    SAL_WARN_IF( ImplIsInListAction_Lock(), "svl",
        "SfxUndoManager::Clear: suspicious call - do you really wish to clear the current level?" );
    ImplClearCurrentLevel_NoNotify( aGuard );

    // notify listeners
    aGuard.scheduleNotification( &SfxUndoListener::cleared );
}


void SfxUndoManager::ClearAllLevels()
{
    UndoManagerGuard aGuard( *m_xData );
    ImplClearCurrentLevel_NoNotify( aGuard );

    if ( ImplIsInListAction_Lock() )
    {
        m_xData->mbClearUntilTopLevel = true;
    }
    else
    {
        aGuard.scheduleNotification( &SfxUndoListener::cleared );
    }
}


void SfxUndoManager::ImplClearRedo_NoLock( bool const i_currentLevel )
{
    UndoManagerGuard aGuard( *m_xData );
    ImplClearRedo( aGuard, i_currentLevel );
}


void SfxUndoManager::ClearRedo()
{
    SAL_WARN_IF( IsInListAction(), "svl",
        "SfxUndoManager::ClearRedo: suspicious call - do you really wish to clear the current level?" );
    ImplClearRedo_NoLock( CurrentLevel );
}


void SfxUndoManager::Reset()
{
    UndoManagerGuard aGuard( *m_xData );

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


void SfxUndoManager::ImplClearUndo( UndoManagerGuard& i_guard )
{
    while ( m_xData->pActUndoArray->nCurUndoAction > 0 )
    {
        SfxUndoAction* pUndoAction = m_xData->pActUndoArray->aUndoActions[0].pAction;
        m_xData->pActUndoArray->aUndoActions.Remove( 0 );
        i_guard.markForDeletion( pUndoAction );
        --m_xData->pActUndoArray->nCurUndoAction;
    }
    // TODO: notifications? We don't have clearedUndo, only cleared and clearedRedo at the SfxUndoListener
}


void SfxUndoManager::ImplClearRedo( UndoManagerGuard& i_guard, bool const i_currentLevel )
{
    SfxUndoArray* pUndoArray = ( i_currentLevel == IUndoManager::CurrentLevel ) ? m_xData->pActUndoArray : m_xData->pUndoArray;

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


bool SfxUndoManager::ImplAddUndoAction_NoNotify( SfxUndoAction *pAction, bool bTryMerge, bool bClearRedo, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() || ( m_xData->pActUndoArray->nMaxUndoActions == 0 ) )
    {
        i_guard.markForDeletion( pAction );
        return false;
    }

    // merge, if required
    SfxUndoAction* pMergeWithAction = m_xData->pActUndoArray->nCurUndoAction ?
        m_xData->pActUndoArray->aUndoActions[m_xData->pActUndoArray->nCurUndoAction-1].pAction : nullptr;
    if ( bTryMerge && pMergeWithAction )
    {
        bool bMerged = pMergeWithAction->Merge( pAction );
        if ( bMerged )
        {
            i_guard.markForDeletion( pAction );
            return false;
        }
    }

    // clear redo stack, if requested
    if ( bClearRedo && ( ImplGetRedoActionCount_Lock() > 0 ) )
        ImplClearRedo( i_guard, IUndoManager::CurrentLevel );

    // respect max number
    if( m_xData->pActUndoArray == m_xData->pUndoArray )
    {
        while(m_xData->pActUndoArray->aUndoActions.size() >= m_xData->pActUndoArray->nMaxUndoActions)
        {
            i_guard.markForDeletion( m_xData->pActUndoArray->aUndoActions[0].pAction );
            m_xData->pActUndoArray->aUndoActions.Remove(0);
            if (m_xData->pActUndoArray->nCurUndoAction > 0)
            {
                --m_xData->pActUndoArray->nCurUndoAction;
            }
            else
            {
                assert(!"CurrentUndoAction going negative (!)");
            }
            // fdo#66071 invalidate the current empty mark when removing
            --m_xData->mnEmptyMark;
        }
    }

    // append new action
    m_xData->pActUndoArray->aUndoActions.Insert( pAction, m_xData->pActUndoArray->nCurUndoAction++ );
    return true;
}


void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, bool bTryMerge )
{
    UndoManagerGuard aGuard( *m_xData );

    // add
    if ( ImplAddUndoAction_NoNotify( pAction, bTryMerge, true, aGuard ) )
    {
        // notify listeners
        aGuard.scheduleNotification( &SfxUndoListener::undoActionAdded, pAction->GetComment() );
    }
}


size_t SfxUndoManager::GetUndoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_xData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray;
    return pUndoArray->nCurUndoAction;
}


OUString SfxUndoManager::GetUndoActionComment( size_t nNo, bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_xData );

    OUString sComment;
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray;
    assert(nNo < pUndoArray->nCurUndoAction);
    if( nNo < pUndoArray->nCurUndoAction )
        sComment = pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction - 1 - nNo ].pAction->GetComment();
    return sComment;
}


sal_uInt16 SfxUndoManager::GetUndoActionId() const
{
    UndoManagerGuard aGuard( *m_xData );

    assert(m_xData->pActUndoArray->nCurUndoAction > 0);
    if ( m_xData->pActUndoArray->nCurUndoAction == 0 )
        return 0;
    return m_xData->pActUndoArray->aUndoActions[m_xData->pActUndoArray->nCurUndoAction-1].pAction->GetId();
}


SfxUndoAction* SfxUndoManager::GetUndoAction( size_t nNo ) const
{
    UndoManagerGuard aGuard( *m_xData );

    assert(nNo < m_xData->pActUndoArray->nCurUndoAction);
    if( nNo >= m_xData->pActUndoArray->nCurUndoAction )
        return nullptr;
    return m_xData->pActUndoArray->aUndoActions[m_xData->pActUndoArray->nCurUndoAction-1-nNo].pAction;
}


/** clears the redo stack and removes the top undo action */
void SfxUndoManager::RemoveLastUndoAction()
{
    UndoManagerGuard aGuard( *m_xData );

    ENSURE_OR_RETURN_VOID( m_xData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::RemoveLastUndoAction(), no action to remove?!" );

    m_xData->pActUndoArray->nCurUndoAction--;

    // delete redo-actions and top action
    for ( size_t nPos = m_xData->pActUndoArray->aUndoActions.size(); nPos > m_xData->pActUndoArray->nCurUndoAction; --nPos )
    {
        aGuard.markForDeletion( m_xData->pActUndoArray->aUndoActions[nPos-1].pAction );
    }

    m_xData->pActUndoArray->aUndoActions.Remove(
        m_xData->pActUndoArray->nCurUndoAction,
        m_xData->pActUndoArray->aUndoActions.size() - m_xData->pActUndoArray->nCurUndoAction );
}


bool SfxUndoManager::IsDoing() const
{
    UndoManagerGuard aGuard( *m_xData );
    return m_xData->mbDoing;
}


bool SfxUndoManager::Undo()
{
    return ImplUndo( nullptr );
}


bool SfxUndoManager::UndoWithContext( SfxUndoContext& i_context )
{
    return ImplUndo( &i_context );
}


bool SfxUndoManager::ImplUndo( SfxUndoContext* i_contextOrNull )
{
    UndoManagerGuard aGuard( *m_xData );
    assert( !IsDoing() && "SfxUndoManager::Undo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aDoingGuard( m_xData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( ImplIsInListAction_Lock() )
    {
        assert(!"SfxUndoManager::Undo: not possible when within a list action!");
        return false;
    }

    if ( m_xData->pActUndoArray->nCurUndoAction == 0 )
    {
        SAL_WARN("svl", "SfxUndoManager::Undo: undo stack is empty!" );
        return false;
    }

    SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[ --m_xData->pActUndoArray->nCurUndoAction ].pAction;
    const OUString sActionComment = pAction->GetComment();
    try
    {
        // clear the guard/mutex before calling into the SfxUndoAction - this can be an extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( i_contextOrNull != nullptr )
            pAction->UndoWithContext( *i_contextOrNull );
        else
            pAction->Undo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_xData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        size_t nCurAction = 0;
        while ( nCurAction < m_xData->pActUndoArray->aUndoActions.size() )
        {
            if ( m_xData->pActUndoArray->aUndoActions[ nCurAction++ ].pAction == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearUndo( aGuard );
                throw;
            }
        }
        SAL_WARN("svl", "SfxUndoManager::Undo: can't clear the Undo stack after the failure - some other party was faster ..." );
        throw;
    }

    aGuard.scheduleNotification( &SfxUndoListener::actionUndone, sActionComment );

    return true;
}


size_t SfxUndoManager::GetRedoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_xData );
    return ImplGetRedoActionCount_Lock( i_currentLevel );
}


size_t SfxUndoManager::ImplGetRedoActionCount_Lock( bool const i_currentLevel ) const
{
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray;
    return pUndoArray->aUndoActions.size() - pUndoArray->nCurUndoAction;
}


SfxUndoAction* SfxUndoManager::GetRedoAction() const
{
    UndoManagerGuard aGuard( *m_xData );

    const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
    if ( (pUndoArray->nCurUndoAction) > pUndoArray->aUndoActions.size() )
    {
        return nullptr;
    }
    return pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction ].pAction;
}


OUString SfxUndoManager::GetRedoActionComment( size_t nNo, bool const i_currentLevel ) const
{
    OUString sComment;
    UndoManagerGuard aGuard( *m_xData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray;
    if ( (pUndoArray->nCurUndoAction + nNo) < pUndoArray->aUndoActions.size() )
    {
        sComment = pUndoArray->aUndoActions[ pUndoArray->nCurUndoAction + nNo ].pAction->GetComment();
    }
    return sComment;
}


bool SfxUndoManager::Redo()
{
    return ImplRedo( nullptr );
}


bool SfxUndoManager::RedoWithContext( SfxUndoContext& i_context )
{
    return ImplRedo( &i_context );
}


bool SfxUndoManager::ImplRedo( SfxUndoContext* i_contextOrNull )
{
    UndoManagerGuard aGuard( *m_xData );
    assert( !IsDoing() && "SfxUndoManager::Redo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aDoingGuard( m_xData->mbDoing );
    LockGuard aLockGuard( *this );

    if ( ImplIsInListAction_Lock() )
    {
        assert(!"SfxUndoManager::Redo: not possible when within a list action!");
        return false;
    }

    if ( m_xData->pActUndoArray->nCurUndoAction >= m_xData->pActUndoArray->aUndoActions.size() )
    {
        SAL_WARN("svl", "SfxUndoManager::Redo: redo stack is empty!");
        return false;
    }

    SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->nCurUndoAction++ ].pAction;
    const OUString sActionComment = pAction->GetComment();
    try
    {
        // clear the guard/mutex before calling into the SfxUndoAction - this can be a extension-implemented UNO component
        // nowadays ...
        aGuard.clear();
        if ( i_contextOrNull != nullptr )
            pAction->RedoWithContext( *i_contextOrNull );
        else
            pAction->Redo();
        aGuard.reset();
    }
    catch( ... )
    {
        aGuard.reset();

        // in theory, somebody might have tampered with all of *m_xData while the mutex was unlocked. So, see if
        // we still find pAction in our current Undo array
        size_t nCurAction = 0;
        while ( nCurAction < m_xData->pActUndoArray->aUndoActions.size() )
        {
            if ( m_xData->pActUndoArray->aUndoActions[ nCurAction ].pAction == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearRedo( aGuard, IUndoManager::CurrentLevel );
                throw;
            }
            ++nCurAction;
        }
        SAL_WARN("svl", "SfxUndoManager::Redo: can't clear the Undo stack after the failure - some other party was faster ..." );
        throw;
    }

    aGuard.scheduleNotification( &SfxUndoListener::actionRedone, sActionComment );

    return true;
}


size_t SfxUndoManager::GetRepeatActionCount() const
{
    UndoManagerGuard aGuard( *m_xData );
    return m_xData->pActUndoArray->aUndoActions.size();
}


OUString SfxUndoManager::GetRepeatActionComment(SfxRepeatTarget &rTarget) const
{
    UndoManagerGuard aGuard( *m_xData );
    return m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->aUndoActions.size() - 1 ].pAction
        ->GetRepeatComment(rTarget);
}


bool SfxUndoManager::Repeat( SfxRepeatTarget &rTarget )
{
    UndoManagerGuard aGuard( *m_xData );
    if ( !m_xData->pActUndoArray->aUndoActions.empty() )
    {
        SfxUndoAction* pAction = m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->aUndoActions.size() - 1 ].pAction;
        aGuard.clear();
        if ( pAction->CanRepeat( rTarget ) )
            pAction->Repeat( rTarget );
        return true;
    }

    return false;
}


bool SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget ) const
{
    UndoManagerGuard aGuard( *m_xData );
    if ( !m_xData->pActUndoArray->aUndoActions.empty() )
    {
        size_t nActionNo = m_xData->pActUndoArray->aUndoActions.size() - 1;
        return m_xData->pActUndoArray->aUndoActions[nActionNo].pAction->CanRepeat(rTarget);
    }
    return false;
}


void SfxUndoManager::AddUndoListener( SfxUndoListener& i_listener )
{
    UndoManagerGuard aGuard( *m_xData );
    m_xData->aListeners.push_back( &i_listener );
}


void SfxUndoManager::RemoveUndoListener( SfxUndoListener& i_listener )
{
    UndoManagerGuard aGuard( *m_xData );
    for (   UndoListeners::iterator lookup = m_xData->aListeners.begin();
            lookup != m_xData->aListeners.end();
            ++lookup
        )
    {
        if ( (*lookup) == &i_listener )
        {
            m_xData->aListeners.erase( lookup );
            break;
        }
    }
}

/**
 * Inserts a ListUndoAction and sets its UndoArray as current.
 */
void SfxUndoManager::EnterListAction( const OUString& rComment,
                                      const OUString &rRepeatComment, sal_uInt16 nId )
{
    UndoManagerGuard aGuard( *m_xData );

    if( !ImplIsUndoEnabled_Lock() )
        return;

    if ( !m_xData->pUndoArray->nMaxUndoActions )
        return;

    m_xData->pFatherUndoArray = m_xData->pActUndoArray;
    SfxListUndoAction* pAction = new SfxListUndoAction( rComment, rRepeatComment, nId, m_xData->pActUndoArray );
    OSL_VERIFY( ImplAddUndoAction_NoNotify( pAction, false, false, aGuard ) );
    // expected to succeed: all conditions under which it could fail should have been checked already
    m_xData->pActUndoArray = pAction;

    // notification
    aGuard.scheduleNotification( &SfxUndoListener::listActionEntered, rComment );
}


bool SfxUndoManager::IsInListAction() const
{
    UndoManagerGuard aGuard( *m_xData );
    return ImplIsInListAction_Lock();
}


bool SfxUndoManager::ImplIsInListAction_Lock() const
{
    return ( m_xData->pActUndoArray != m_xData->pUndoArray );
}


size_t SfxUndoManager::GetListActionDepth() const
{
    UndoManagerGuard aGuard( *m_xData );
    size_t nDepth(0);

    SfxUndoArray* pLookup( m_xData->pActUndoArray );
    while ( pLookup != m_xData->pUndoArray )
    {
        pLookup = pLookup->pFatherUndoArray;
        ++nDepth;
    }

    return nDepth;
}


size_t SfxUndoManager::LeaveListAction()
{
    UndoManagerGuard aGuard( *m_xData );
    size_t nCount = ImplLeaveListAction( false, aGuard );

    if ( m_xData->mbClearUntilTopLevel )
    {
        ImplClearCurrentLevel_NoNotify( aGuard );
        if ( !ImplIsInListAction_Lock() )
        {
            m_xData->mbClearUntilTopLevel = false;
            aGuard.scheduleNotification( &SfxUndoListener::cleared );
        }
        nCount = 0;
    }

    return nCount;
}


size_t SfxUndoManager::LeaveAndMergeListAction()
{
    UndoManagerGuard aGuard( *m_xData );
    return ImplLeaveListAction( true, aGuard );
}


size_t SfxUndoManager::ImplLeaveListAction( const bool i_merge, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() )
        return 0;

    if ( !m_xData->pUndoArray->nMaxUndoActions )
        return 0;

    if( !ImplIsInListAction_Lock() )
    {
        SAL_WARN("svl", "svl::SfxUndoManager::ImplLeaveListAction, called without calling EnterListAction()!" );
        return 0;
    }

    assert(m_xData->pActUndoArray->pFatherUndoArray);

    // the array/level which we're about to leave
    SfxUndoArray* pArrayToLeave = m_xData->pActUndoArray;
    // one step up
    m_xData->pActUndoArray = m_xData->pActUndoArray->pFatherUndoArray;

    // If no undo actions were added to the list, delete the list action
    const size_t nListActionElements = pArrayToLeave->nCurUndoAction;
    if ( nListActionElements == 0 )
    {
        SfxUndoAction* pCurrentAction= m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->nCurUndoAction-1 ].pAction;
        m_xData->pActUndoArray->aUndoActions.Remove( --m_xData->pActUndoArray->nCurUndoAction );
        i_guard.markForDeletion( pCurrentAction );

        i_guard.scheduleNotification( &SfxUndoListener::listActionCancelled );
        return 0;
    }

    // now that it is finally clear the list action is non-trivial, and does participate in the Undo stack, clear
    // the redo stack
    ImplClearRedo( i_guard, IUndoManager::CurrentLevel );

    SfxUndoAction* pCurrentAction= m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->nCurUndoAction-1 ].pAction;
    SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction * >( pCurrentAction );
    ENSURE_OR_RETURN( pListAction, "SfxUndoManager::ImplLeaveListAction: list action expected at this position!", nListActionElements );

    if ( i_merge )
    {
        // merge the list action with its predecessor on the same level
        SAL_WARN_IF( m_xData->pActUndoArray->nCurUndoAction <= 1, "svl",
            "SfxUndoManager::ImplLeaveListAction: cannot merge the list action if there's no other action on the same level - check this beforehand!" );
        if ( m_xData->pActUndoArray->nCurUndoAction > 1 )
        {
            SfxUndoAction* pPreviousAction = m_xData->pActUndoArray->aUndoActions[ m_xData->pActUndoArray->nCurUndoAction - 2 ].pAction;
            m_xData->pActUndoArray->aUndoActions.Remove( m_xData->pActUndoArray->nCurUndoAction - 2 );
            --m_xData->pActUndoArray->nCurUndoAction;
            pListAction->aUndoActions.Insert( pPreviousAction, 0 );
            ++pListAction->nCurUndoAction;

            pListAction->SetComment( pPreviousAction->GetComment() );
        }
    }

    // if the undo array has no comment, try to get it from its children
    if ( pListAction->GetComment().isEmpty() )
    {
        for( size_t n = 0; n < pListAction->aUndoActions.size(); n++ )
        {
            if (!pListAction->aUndoActions[n].pAction->GetComment().isEmpty())
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

UndoStackMark SfxUndoManager::MarkTopUndoAction()
{
    UndoManagerGuard aGuard( *m_xData );

    SAL_WARN_IF( IsInListAction(), "svl",
            "SfxUndoManager::MarkTopUndoAction(): suspicious call!" );
    assert((m_xData->mnMarks + 1) < (m_xData->mnEmptyMark - 1) &&
            "SfxUndoManager::MarkTopUndoAction(): mark overflow!");

    size_t const nActionPos = m_xData->pUndoArray->nCurUndoAction;
    if (0 == nActionPos)
    {
        --m_xData->mnEmptyMark;
        return m_xData->mnEmptyMark;
    }

    m_xData->pUndoArray->aUndoActions[ nActionPos-1 ].aMarks.push_back(
            ++m_xData->mnMarks );
    return m_xData->mnMarks;
}

void SfxUndoManager::RemoveMark( UndoStackMark const i_mark )
{
    UndoManagerGuard aGuard( *m_xData );

    if ((m_xData->mnEmptyMark < i_mark) || (MARK_INVALID == i_mark))
    {
        return; // nothing to remove
    }
    else if (i_mark == m_xData->mnEmptyMark)
    {
        --m_xData->mnEmptyMark; // never returned from MarkTop => invalid
        return;
    }

    for ( size_t i=0; i<m_xData->pUndoArray->aUndoActions.size(); ++i )
    {
        MarkedUndoAction& rAction = m_xData->pUndoArray->aUndoActions[i];
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
    SAL_WARN("svl", "SfxUndoManager::RemoveMark: mark not found!");
        // TODO: this might be too offensive. There are situations where we implicitly remove marks
        // without our clients, in particular the client which created the mark, having a chance to know
        // about this.
}

bool SfxUndoManager::HasTopUndoActionMark( UndoStackMark const i_mark )
{
    UndoManagerGuard aGuard( *m_xData );

    size_t nActionPos = m_xData->pUndoArray->nCurUndoAction;
    if ( nActionPos == 0 )
    {
        return (i_mark == m_xData->mnEmptyMark);
    }

    const MarkedUndoAction& rAction =
            m_xData->pUndoArray->aUndoActions[ nActionPos-1 ];
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


void SfxUndoManager::RemoveOldestUndoAction()
{
    UndoManagerGuard aGuard( *m_xData );

    SfxUndoAction* pActionToRemove = m_xData->pUndoArray->aUndoActions[0].pAction;

    if ( IsInListAction() && ( m_xData->pUndoArray->nCurUndoAction == 1 ) )
    {
        assert(!"SfxUndoManager::RemoveOldestUndoActions: cannot remove a not-yet-closed list action!");
        return;
    }

    aGuard.markForDeletion( pActionToRemove );
    m_xData->pUndoArray->aUndoActions.Remove( 0 );
    --m_xData->pUndoArray->nCurUndoAction;
}

void SfxUndoManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sfxUndoManager"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUndoActionCount"), BAD_CAST(OString::number(GetUndoActionCount()).getStr()));

    for (size_t i = 0; i < GetUndoActionCount(); ++i)
        GetUndoAction(i)->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

struct SfxListUndoAction::Impl
{
    sal_uInt16 mnId;

    OUString maComment;
    OUString maRepeatComment;

    Impl( sal_uInt16 nId, const OUString& rComment, const OUString& rRepeatComment ) :
        mnId(nId), maComment(rComment), maRepeatComment(rRepeatComment) {}
};

sal_uInt16 SfxListUndoAction::GetId() const
{
    return mpImpl->mnId;
}

OUString SfxListUndoAction::GetComment() const
{
    return mpImpl->maComment;
}

void SfxListUndoAction::SetComment(const OUString& rComment)
{
    mpImpl->maComment = rComment;
}

OUString SfxListUndoAction::GetRepeatComment(SfxRepeatTarget &) const
{
    return mpImpl->maRepeatComment;
}

SfxListUndoAction::SfxListUndoAction(
    const OUString &rComment,
    const OUString &rRepeatComment,
    sal_uInt16 nId,
    SfxUndoArray *pFather ) :
    mpImpl(new Impl(nId, rComment, rRepeatComment))
{
    pFatherUndoArray = pFather;
    nMaxUndoActions = USHRT_MAX;
}

SfxListUndoAction::~SfxListUndoAction()
{
}

void SfxListUndoAction::Undo()
{
    for(size_t i=nCurUndoAction;i>0;)
        aUndoActions[--i].pAction->Undo();
    nCurUndoAction=0;
}


void SfxListUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i>0;)
        aUndoActions[--i].pAction->UndoWithContext( i_context );
    nCurUndoAction=0;
}


void SfxListUndoAction::Redo()
{
    for(size_t i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->Redo();
    nCurUndoAction = aUndoActions.size();
}


void SfxListUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i<aUndoActions.size();i++)
        aUndoActions[i].pAction->RedoWithContext( i_context );
    nCurUndoAction = aUndoActions.size();
}


void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(size_t i=0;i<nCurUndoAction;i++)
        aUndoActions[i].pAction->Repeat(rTarget);
}


bool SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(size_t i=0;i<nCurUndoAction;i++)
    {
        if(!aUndoActions[i].pAction->CanRepeat(r))
            return false;
    }
    return true;
}


bool SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
{
    return !aUndoActions.empty() && aUndoActions[aUndoActions.size()-1].pAction->Merge( pNextAction );
}

void SfxListUndoAction::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sfxListUndoAction"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(OString::number(aUndoActions.size()).getStr()));
    SfxUndoAction::dumpAsXml(pWriter);

    for (size_t i = 0; i < aUndoActions.size(); ++i)
        aUndoActions.GetUndoAction(i)->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

SfxUndoArray::~SfxUndoArray()
{
    while ( !aUndoActions.empty() )
    {
        SfxUndoAction *pAction = aUndoActions[ aUndoActions.size() - 1 ].pAction;
        aUndoActions.Remove( aUndoActions.size() - 1 );
        delete pAction;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
