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

#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <comphelper/flagguard.hxx>
#include <tools/diagnose_ex.h>
#include <tools/long.hxx>
#include <libxml/xmlwriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <unotools/datetime.hxx>

#include <memory>
#include <vector>
#include <limits.h>
#include <algorithm>


SfxRepeatTarget::~SfxRepeatTarget()
{
}


SfxUndoContext::~SfxUndoContext()
{
}


SfxUndoAction::~SfxUndoAction() COVERITY_NOEXCEPT_FALSE
{
}


SfxUndoAction::SfxUndoAction()
    : m_aDateTime(DateTime::SYSTEM)
{
    m_aDateTime.ConvertToUTC();
}


bool SfxUndoAction::Merge( SfxUndoAction * )
{
    return false;
}


OUString SfxUndoAction::GetComment() const
{
    return OUString();
}


ViewShellId SfxUndoAction::GetViewShellId() const
{
    return ViewShellId(-1);
}

const DateTime& SfxUndoAction::GetDateTime() const
{
    return m_aDateTime;
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


void SfxUndoAction::UndoWithContext( SfxUndoContext& )
{
    Undo();
}


void SfxUndoAction::Redo()
{
    // These are only conceptually pure virtual
    assert(!"pure virtual function called: SfxUndoAction::Redo()");
}


void SfxUndoAction::RedoWithContext( SfxUndoContext& )
{
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
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxUndoAction"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("symbol"), BAD_CAST(typeid(*this).name()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("comment"), BAD_CAST(GetComment().toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("viewShellId"), BAD_CAST(OString::number(static_cast<sal_Int32>(GetViewShellId())).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("dateTime"), BAD_CAST(utl::toISO8601(m_aDateTime.GetUNODateTime()).toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

std::unique_ptr<SfxUndoAction> SfxUndoArray::Remove(int idx)
{
    auto ret = std::move(maUndoActions[idx].pAction);
    maUndoActions.erase(maUndoActions.begin() + idx);
    return ret;
}

void SfxUndoArray::Remove( size_t i_pos, size_t i_count )
{
    maUndoActions.erase(maUndoActions.begin() + i_pos, maUndoActions.begin() + i_pos + i_count);
}

void SfxUndoArray::Insert( std::unique_ptr<SfxUndoAction> i_action, size_t i_pos )
{
    maUndoActions.insert( maUndoActions.begin() + i_pos, MarkedUndoAction(std::move(i_action)) );
}

typedef ::std::vector< SfxUndoListener* >   UndoListeners;

struct SfxUndoManager_Data
{
    ::osl::Mutex    aMutex;
    std::unique_ptr<SfxUndoArray>
                    pUndoArray;
    SfxUndoArray*   pActUndoArray;

    sal_Int32       mnMarks;
    sal_Int32       mnEmptyMark;
    bool            mbUndoEnabled;
    bool            mbDoing;
    bool            mbClearUntilTopLevel;
    bool            mbEmptyActions;

    UndoListeners   aListeners;

    explicit SfxUndoManager_Data( size_t i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( nullptr )
        ,mnMarks( 0 )
        ,mnEmptyMark(MARK_INVALID)
        ,mbUndoEnabled( true )
        ,mbDoing( false )
        ,mbClearUntilTopLevel( false )
        ,mbEmptyActions( true )
    {
        pActUndoArray = pUndoArray.get();
    }

    // Copy assignment is forbidden and not implemented.
    SfxUndoManager_Data (const SfxUndoManager_Data &) = delete;
    SfxUndoManager_Data & operator= (const SfxUndoManager_Data &) = delete;
};

namespace svl::undo::impl
{
    class LockGuard
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

    namespace {

    struct NotifyUndoListener
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

    }

    class UndoManagerGuard
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
        void    markForDeletion( std::unique_ptr<SfxUndoAction> i_action )
        {
            // remember
            assert ( i_action );
            m_aUndoActionsCleanup.emplace_back( std::move(i_action) );
        }

        /** schedules the given SfxUndoListener method to be called for all registered listeners.

            The notification will happen after the Undo manager's mutex has been released, and after all pending
            deletions of Undo actions are done.
        */
        void    scheduleNotification( UndoListenerVoidMethod i_notificationMethod )
        {
            m_notifiers.emplace_back( i_notificationMethod );
        }

        void    scheduleNotification( UndoListenerStringMethod i_notificationMethod, const OUString& i_actionComment )
        {
            m_notifiers.emplace_back( i_notificationMethod, i_actionComment );
        }

    private:
        SfxUndoManager_Data&                m_rManagerData;
        ::osl::ResettableMutexGuard         m_aGuard;
        ::std::vector< std::unique_ptr<SfxUndoAction> > m_aUndoActionsCleanup;
        ::std::vector< NotifyUndoListener > m_notifiers;
    };

    UndoManagerGuard::~UndoManagerGuard()
    {
        // copy members
        UndoListeners aListenersCopy( m_rManagerData.aListeners );

        // release mutex
        m_aGuard.clear();

        // delete all actions
        m_aUndoActionsCleanup.clear();

        // handle scheduled notification
        for (auto const& notifier : m_notifiers)
        {
            if ( notifier.is() )
                ::std::for_each( aListenersCopy.begin(), aListenersCopy.end(), notifier );
        }
    }
}

using namespace ::svl::undo::impl;


SfxUndoManager::SfxUndoManager( size_t nMaxUndoActionCount )
    :m_xData( new SfxUndoManager_Data( nMaxUndoActionCount ) )
{
    m_xData->mbEmptyActions = !ImplIsEmptyActions();
}


SfxUndoManager::~SfxUndoManager()
{
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

    tools::Long nNumToDelete = m_xData->pActUndoArray->maUndoActions.size() - nMaxUndoActionCount;
    while ( nNumToDelete > 0 )
    {
        size_t nPos = m_xData->pActUndoArray->maUndoActions.size();
        if ( nPos > m_xData->pActUndoArray->nCurUndoAction )
        {
            aGuard.markForDeletion( m_xData->pActUndoArray->Remove( nPos-1 ) );
            --nNumToDelete;
        }

        if ( nNumToDelete > 0 && m_xData->pActUndoArray->nCurUndoAction > 0 )
        {
            aGuard.markForDeletion( m_xData->pActUndoArray->Remove(0) );
            --m_xData->pActUndoArray->nCurUndoAction;
            --nNumToDelete;
        }

        if ( nPos == m_xData->pActUndoArray->maUndoActions.size() )
            break; // Cannot delete more entries
    }

    m_xData->pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
    ImplCheckEmptyActions();
}


void SfxUndoManager::ImplClearCurrentLevel_NoNotify( UndoManagerGuard& i_guard )
{
    // clear array
    while ( !m_xData->pActUndoArray->maUndoActions.empty() )
    {
        size_t deletePos = m_xData->pActUndoArray->maUndoActions.size() - 1;
        i_guard.markForDeletion( m_xData->pActUndoArray->Remove( deletePos ) );
    }

    m_xData->pActUndoArray->nCurUndoAction = 0;

    m_xData->mnMarks = 0;
    m_xData->mnEmptyMark = MARK_INVALID;
    ImplCheckEmptyActions();
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
        i_guard.markForDeletion( m_xData->pActUndoArray->Remove( 0 ) );
        --m_xData->pActUndoArray->nCurUndoAction;
    }
    ImplCheckEmptyActions();
    // TODO: notifications? We don't have clearedUndo, only cleared and clearedRedo at the SfxUndoListener
}


void SfxUndoManager::ImplClearRedo( UndoManagerGuard& i_guard, bool const i_currentLevel )
{
    SfxUndoArray* pUndoArray = ( i_currentLevel == SfxUndoManager::CurrentLevel ) ? m_xData->pActUndoArray : m_xData->pUndoArray.get();

    // clearance
    while ( pUndoArray->maUndoActions.size() > pUndoArray->nCurUndoAction )
    {
        size_t deletePos = pUndoArray->maUndoActions.size() - 1;
        i_guard.markForDeletion( pUndoArray->Remove( deletePos ) );
    }

    ImplCheckEmptyActions();
    // notification - only if the top level's stack was cleared
    if ( i_currentLevel == SfxUndoManager::TopLevel )
        i_guard.scheduleNotification( &SfxUndoListener::clearedRedo );
}


bool SfxUndoManager::ImplAddUndoAction_NoNotify( std::unique_ptr<SfxUndoAction> pAction, bool bTryMerge, bool bClearRedo, UndoManagerGuard& i_guard )
{
    if ( !ImplIsUndoEnabled_Lock() || ( m_xData->pActUndoArray->nMaxUndoActions == 0 ) )
    {
        i_guard.markForDeletion( std::move(pAction) );
        return false;
    }

    // merge, if required
    SfxUndoAction* pMergeWithAction = m_xData->pActUndoArray->nCurUndoAction ?
        m_xData->pActUndoArray->maUndoActions[m_xData->pActUndoArray->nCurUndoAction-1].pAction.get() : nullptr;
    if ( bTryMerge && pMergeWithAction )
    {
        bool bMerged = pMergeWithAction->Merge( pAction.get() );
        if ( bMerged )
        {
            i_guard.markForDeletion( std::move(pAction) );
            return false;
        }
    }

    // clear redo stack, if requested
    if ( bClearRedo && ( ImplGetRedoActionCount_Lock() > 0 ) )
        ImplClearRedo( i_guard, SfxUndoManager::CurrentLevel );

    // respect max number
    if( m_xData->pActUndoArray == m_xData->pUndoArray.get() )
    {
        while(m_xData->pActUndoArray->maUndoActions.size() >= m_xData->pActUndoArray->nMaxUndoActions)
        {
            i_guard.markForDeletion( m_xData->pActUndoArray->Remove(0) );
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
    m_xData->pActUndoArray->Insert( std::move(pAction), m_xData->pActUndoArray->nCurUndoAction++ );
    ImplCheckEmptyActions();
    return true;
}


void SfxUndoManager::AddUndoAction( std::unique_ptr<SfxUndoAction> pAction, bool bTryMerge )
{
    UndoManagerGuard aGuard( *m_xData );

    // add
    auto pActionTmp = pAction.get();
    if ( ImplAddUndoAction_NoNotify( std::move(pAction), bTryMerge, true, aGuard ) )
    {
        // notify listeners
        aGuard.scheduleNotification( &SfxUndoListener::undoActionAdded, pActionTmp->GetComment() );
    }
}


size_t SfxUndoManager::GetUndoActionCount( bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_xData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray.get();
    return pUndoArray->nCurUndoAction;
}


OUString SfxUndoManager::GetUndoActionComment( size_t nNo, bool const i_currentLevel ) const
{
    UndoManagerGuard aGuard( *m_xData );

    OUString sComment;
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray.get();
    assert(nNo < pUndoArray->nCurUndoAction);
    if( nNo < pUndoArray->nCurUndoAction )
        sComment = pUndoArray->maUndoActions[ pUndoArray->nCurUndoAction - 1 - nNo ].pAction->GetComment();
    return sComment;
}


SfxUndoAction* SfxUndoManager::GetUndoAction( size_t nNo ) const
{
    UndoManagerGuard aGuard( *m_xData );

    assert(nNo < m_xData->pActUndoArray->nCurUndoAction);
    if( nNo >= m_xData->pActUndoArray->nCurUndoAction )
        return nullptr;
    return m_xData->pActUndoArray->maUndoActions[m_xData->pActUndoArray->nCurUndoAction-1-nNo].pAction.get();
}


/** clears the redo stack and removes the top undo action */
void SfxUndoManager::RemoveLastUndoAction()
{
    UndoManagerGuard aGuard( *m_xData );

    ENSURE_OR_RETURN_VOID( m_xData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::RemoveLastUndoAction(), no action to remove?!" );

    m_xData->pActUndoArray->nCurUndoAction--;

    // delete redo-actions and top action
    for ( size_t nPos = m_xData->pActUndoArray->maUndoActions.size(); nPos > m_xData->pActUndoArray->nCurUndoAction; --nPos )
    {
        aGuard.markForDeletion( std::move(m_xData->pActUndoArray->maUndoActions[nPos-1].pAction) );
    }

    m_xData->pActUndoArray->Remove(
        m_xData->pActUndoArray->nCurUndoAction,
        m_xData->pActUndoArray->maUndoActions.size() - m_xData->pActUndoArray->nCurUndoAction );
    ImplCheckEmptyActions();
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

    SfxUndoAction* pAction = m_xData->pActUndoArray->maUndoActions[ --m_xData->pActUndoArray->nCurUndoAction ].pAction.get();
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
        while ( nCurAction < m_xData->pActUndoArray->maUndoActions.size() )
        {
            if ( m_xData->pActUndoArray->maUndoActions[ nCurAction++ ].pAction.get() == pAction )
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
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray.get();
    return pUndoArray->maUndoActions.size() - pUndoArray->nCurUndoAction;
}


SfxUndoAction* SfxUndoManager::GetRedoAction() const
{
    UndoManagerGuard aGuard( *m_xData );

    const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
    if ( (pUndoArray->nCurUndoAction) > pUndoArray->maUndoActions.size() )
    {
        return nullptr;
    }
    return pUndoArray->maUndoActions[ pUndoArray->nCurUndoAction ].pAction.get();
}


OUString SfxUndoManager::GetRedoActionComment( size_t nNo, bool const i_currentLevel ) const
{
    OUString sComment;
    UndoManagerGuard aGuard( *m_xData );
    const SfxUndoArray* pUndoArray = i_currentLevel ? m_xData->pActUndoArray : m_xData->pUndoArray.get();
    if ( (pUndoArray->nCurUndoAction + nNo) < pUndoArray->maUndoActions.size() )
    {
        sComment = pUndoArray->maUndoActions[ pUndoArray->nCurUndoAction + nNo ].pAction->GetComment();
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

    if ( m_xData->pActUndoArray->nCurUndoAction >= m_xData->pActUndoArray->maUndoActions.size() )
    {
        SAL_WARN("svl", "SfxUndoManager::Redo: redo stack is empty!");
        return false;
    }

    SfxUndoAction* pAction = m_xData->pActUndoArray->maUndoActions[ m_xData->pActUndoArray->nCurUndoAction++ ].pAction.get();
    const OUString sActionComment = pAction->GetComment();
    try
    {
        // clear the guard/mutex before calling into the SfxUndoAction - this can be an extension-implemented UNO component
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
        while ( nCurAction < m_xData->pActUndoArray->maUndoActions.size() )
        {
            if ( m_xData->pActUndoArray->maUndoActions[ nCurAction ].pAction.get() == pAction )
            {
                // the Undo action is still there ...
                // assume the error is a permanent failure, and clear the Undo stack
                ImplClearRedo( aGuard, SfxUndoManager::CurrentLevel );
                throw;
            }
            ++nCurAction;
        }
        SAL_WARN("svl", "SfxUndoManager::Redo: can't clear the Undo stack after the failure - some other party was faster ..." );
        throw;
    }

    ImplCheckEmptyActions();
    aGuard.scheduleNotification( &SfxUndoListener::actionRedone, sActionComment );

    return true;
}


size_t SfxUndoManager::GetRepeatActionCount() const
{
    UndoManagerGuard aGuard( *m_xData );
    return m_xData->pActUndoArray->maUndoActions.size();
}


OUString SfxUndoManager::GetRepeatActionComment(SfxRepeatTarget &rTarget) const
{
    UndoManagerGuard aGuard( *m_xData );
    return m_xData->pActUndoArray->maUndoActions[ m_xData->pActUndoArray->maUndoActions.size() - 1 ].pAction
        ->GetRepeatComment(rTarget);
}


bool SfxUndoManager::Repeat( SfxRepeatTarget &rTarget )
{
    UndoManagerGuard aGuard( *m_xData );
    if ( !m_xData->pActUndoArray->maUndoActions.empty() )
    {
        SfxUndoAction* pAction = m_xData->pActUndoArray->maUndoActions.back().pAction.get();
        aGuard.clear();
        if ( pAction->CanRepeat( rTarget ) )
            pAction->Repeat( rTarget );
        aGuard.reset(); // allow clearing in guard dtor
        return true;
    }

    return false;
}


bool SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget ) const
{
    UndoManagerGuard aGuard( *m_xData );
    if ( !m_xData->pActUndoArray->maUndoActions.empty() )
    {
        size_t nActionNo = m_xData->pActUndoArray->maUndoActions.size() - 1;
        return m_xData->pActUndoArray->maUndoActions[nActionNo].pAction->CanRepeat(rTarget);
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
    auto lookup = std::find(m_xData->aListeners.begin(), m_xData->aListeners.end(), &i_listener);
    if (lookup != m_xData->aListeners.end())
        m_xData->aListeners.erase( lookup );
}

/**
 * Inserts a ListUndoAction and sets its UndoArray as current.
 */
void SfxUndoManager::EnterListAction( const OUString& rComment,
                                      const OUString &rRepeatComment, sal_uInt16 nId,
                                      ViewShellId nViewShellId )
{
    UndoManagerGuard aGuard( *m_xData );

    if( !ImplIsUndoEnabled_Lock() )
        return;

    if ( !m_xData->pUndoArray->nMaxUndoActions )
        return;

    SfxListUndoAction* pAction = new SfxListUndoAction( rComment, rRepeatComment, nId, nViewShellId, m_xData->pActUndoArray );
    OSL_VERIFY( ImplAddUndoAction_NoNotify( std::unique_ptr<SfxUndoAction>(pAction), false, false, aGuard ) );
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
    return ( m_xData->pActUndoArray != m_xData->pUndoArray.get() );
}


size_t SfxUndoManager::GetListActionDepth() const
{
    UndoManagerGuard aGuard( *m_xData );
    size_t nDepth(0);

    SfxUndoArray* pLookup( m_xData->pActUndoArray );
    while ( pLookup != m_xData->pUndoArray.get() )
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
        i_guard.markForDeletion( m_xData->pActUndoArray->Remove( --m_xData->pActUndoArray->nCurUndoAction ) );
        i_guard.scheduleNotification( &SfxUndoListener::listActionCancelled );
        return 0;
    }

    // now that it is finally clear the list action is non-trivial, and does participate in the Undo stack, clear
    // the redo stack
    ImplClearRedo( i_guard, SfxUndoManager::CurrentLevel );

    SfxUndoAction* pCurrentAction= m_xData->pActUndoArray->maUndoActions[ m_xData->pActUndoArray->nCurUndoAction-1 ].pAction.get();
    SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction * >( pCurrentAction );
    ENSURE_OR_RETURN( pListAction, "SfxUndoManager::ImplLeaveListAction: list action expected at this position!", nListActionElements );

    if ( i_merge )
    {
        // merge the list action with its predecessor on the same level
        SAL_WARN_IF( m_xData->pActUndoArray->nCurUndoAction <= 1, "svl",
            "SfxUndoManager::ImplLeaveListAction: cannot merge the list action if there's no other action on the same level - check this beforehand!" );
        if ( m_xData->pActUndoArray->nCurUndoAction > 1 )
        {
            std::unique_ptr<SfxUndoAction> pPreviousAction = m_xData->pActUndoArray->Remove( m_xData->pActUndoArray->nCurUndoAction - 2 );
            --m_xData->pActUndoArray->nCurUndoAction;
            pListAction->SetComment( pPreviousAction->GetComment() );
            pListAction->Insert( std::move(pPreviousAction), 0 );
            ++pListAction->nCurUndoAction;
        }
    }

    // if the undo array has no comment, try to get it from its children
    if ( pListAction->GetComment().isEmpty() )
    {
        for( size_t n = 0; n < pListAction->maUndoActions.size(); n++ )
        {
            if (!pListAction->maUndoActions[n].pAction->GetComment().isEmpty())
            {
                pListAction->SetComment( pListAction->maUndoActions[n].pAction->GetComment() );
                break;
            }
        }
    }

    ImplIsEmptyActions();
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

    m_xData->pUndoArray->maUndoActions[ nActionPos-1 ].aMarks.push_back(
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

    for ( size_t i=0; i<m_xData->pUndoArray->maUndoActions.size(); ++i )
    {
        MarkedUndoAction& rAction = m_xData->pUndoArray->maUndoActions[i];
        auto markPos = std::find(rAction.aMarks.begin(), rAction.aMarks.end(), i_mark);
        if (markPos != rAction.aMarks.end())
        {
            rAction.aMarks.erase( markPos );
            return;
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
            m_xData->pUndoArray->maUndoActions[ nActionPos-1 ];

    return std::find(rAction.aMarks.begin(), rAction.aMarks.end(), i_mark) != rAction.aMarks.end();
}


void SfxUndoManager::RemoveOldestUndoAction()
{
    UndoManagerGuard aGuard( *m_xData );

    if ( IsInListAction() && ( m_xData->pUndoArray->nCurUndoAction == 1 ) )
    {
        assert(!"SfxUndoManager::RemoveOldestUndoActions: cannot remove a not-yet-closed list action!");
        return;
    }

    aGuard.markForDeletion( m_xData->pUndoArray->Remove( 0 ) );
    --m_xData->pUndoArray->nCurUndoAction;
    ImplCheckEmptyActions();
}

void SfxUndoManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    UndoManagerGuard aGuard(*m_xData);

    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("undo.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        (void)xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxUndoManager"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nUndoActionCount"), BAD_CAST(OString::number(GetUndoActionCount()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nRedoActionCount"), BAD_CAST(OString::number(GetRedoActionCount()).getStr()));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("undoActions"));
    for (size_t i = 0; i < GetUndoActionCount(); ++i)
    {
        const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
        pUndoArray->maUndoActions[pUndoArray->nCurUndoAction - 1 - i].pAction->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("redoActions"));
    for (size_t i = 0; i < GetRedoActionCount(); ++i)
    {
        const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
        pUndoArray->maUndoActions[pUndoArray->nCurUndoAction + i].pAction->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
    if (bOwns)
    {
        (void)xmlTextWriterEndDocument(pWriter);
        xmlFreeTextWriter(pWriter);
    }
}

/// Returns a JSON representation of pAction.
static boost::property_tree::ptree lcl_ActionToJson(size_t nIndex, SfxUndoAction const * pAction)
{
    boost::property_tree::ptree aRet;
    aRet.put("index", nIndex);
    aRet.put("comment", pAction->GetComment().toUtf8().getStr());
    aRet.put("viewId", static_cast<sal_Int32>(pAction->GetViewShellId()));
    aRet.put("dateTime", utl::toISO8601(pAction->GetDateTime().GetUNODateTime()).toUtf8().getStr());
    return aRet;
}

OUString SfxUndoManager::GetUndoActionsInfo() const
{
    boost::property_tree::ptree aActions;
    const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
    for (size_t i = 0; i < GetUndoActionCount(); ++i)
    {
        boost::property_tree::ptree aAction = lcl_ActionToJson(i, pUndoArray->maUndoActions[pUndoArray->nCurUndoAction - 1 - i].pAction.get());
        aActions.push_back(std::make_pair("", aAction));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("actions", aActions);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    return OUString::fromUtf8(aStream.str().c_str());
}

OUString SfxUndoManager::GetRedoActionsInfo() const
{
    boost::property_tree::ptree aActions;
    const SfxUndoArray* pUndoArray = m_xData->pActUndoArray;
    size_t nCount = GetRedoActionCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        size_t nIndex = nCount - i - 1;
        boost::property_tree::ptree aAction = lcl_ActionToJson(nIndex, pUndoArray->maUndoActions[pUndoArray->nCurUndoAction + nIndex].pAction.get());
        aActions.push_back(std::make_pair("", aAction));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("actions", aActions);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    return OUString::fromUtf8(aStream.str().c_str());
}

bool SfxUndoManager::IsEmptyActions() const
{
    UndoManagerGuard aGuard(*m_xData);

    return ImplIsEmptyActions();
}

inline bool SfxUndoManager::ImplIsEmptyActions() const
{
    return m_xData->pUndoArray->nCurUndoAction || m_xData->pUndoArray->maUndoActions.size() - m_xData->pUndoArray->nCurUndoAction;
}

void SfxUndoManager::ImplCheckEmptyActions()
{
    bool bEmptyActions = ImplIsEmptyActions();
    if (m_xData->mbEmptyActions != bEmptyActions)
    {
        m_xData->mbEmptyActions = bEmptyActions;
        EmptyActionsChanged();
    }
}

void SfxUndoManager::EmptyActionsChanged()
{

}

struct SfxListUndoAction::Impl
{
    sal_uInt16 mnId;
    ViewShellId mnViewShellId;

    OUString maComment;
    OUString maRepeatComment;

    Impl( sal_uInt16 nId, ViewShellId nViewShellId, const OUString& rComment, const OUString& rRepeatComment ) :
        mnId(nId), mnViewShellId(nViewShellId), maComment(rComment), maRepeatComment(rRepeatComment) {}
};

sal_uInt16 SfxListUndoAction::GetId() const
{
    return mpImpl->mnId;
}

OUString SfxListUndoAction::GetComment() const
{
    return mpImpl->maComment;
}

ViewShellId SfxListUndoAction::GetViewShellId() const
{
    return mpImpl->mnViewShellId;
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
    ViewShellId nViewShellId,
    SfxUndoArray *pFather ) :
    mpImpl(new Impl(nId, nViewShellId, rComment, rRepeatComment))
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
        maUndoActions[--i].pAction->Undo();
    nCurUndoAction=0;
}


void SfxListUndoAction::UndoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i>0;)
        maUndoActions[--i].pAction->UndoWithContext( i_context );
    nCurUndoAction=0;
}


void SfxListUndoAction::Redo()
{
    for(size_t i=nCurUndoAction;i<maUndoActions.size();i++)
        maUndoActions[i].pAction->Redo();
    nCurUndoAction = maUndoActions.size();
}


void SfxListUndoAction::RedoWithContext( SfxUndoContext& i_context )
{
    for(size_t i=nCurUndoAction;i<maUndoActions.size();i++)
        maUndoActions[i].pAction->RedoWithContext( i_context );
    nCurUndoAction = maUndoActions.size();
}


void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(size_t i=0;i<nCurUndoAction;i++)
        maUndoActions[i].pAction->Repeat(rTarget);
}


bool SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(size_t i=0;i<nCurUndoAction;i++)
    {
        if(!maUndoActions[i].pAction->CanRepeat(r))
            return false;
    }
    return true;
}


bool SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
{
    return !maUndoActions.empty() && maUndoActions[maUndoActions.size()-1].pAction->Merge( pNextAction );
}

void SfxListUndoAction::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxListUndoAction"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("size"), BAD_CAST(OString::number(maUndoActions.size()).getStr()));
    SfxUndoAction::dumpAsXml(pWriter);

    for (size_t i = 0; i < maUndoActions.size(); ++i)
        maUndoActions[i].pAction->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SfxUndoArray::~SfxUndoArray()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
