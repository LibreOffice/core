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

#include <comphelper/scopeguard.hxx>
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

    UndoListeners   aListeners;

    SfxUndoManager_Data( USHORT i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( NULL )
        ,pFatherUndoArray( NULL )
        ,mnLockCount( 0 )
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
        if ( GetRedoActionCount() > 0 )
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

USHORT SfxUndoManager::GetUndoActionCount() const
{
    return m_pData->pActUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( USHORT nNo ) const
{
    DBG_ASSERT( nNo < m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionComment(), illegal id!" );
    if( nNo < m_pData->pActUndoArray->nCurUndoAction )
    {
        return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1-nNo]->GetComment(); //!
    }
    else
    {
        XubString aEmpty;
        return aEmpty;
    }
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

    BOOL bRet = FALSE;

    DBG_ASSERT( m_pData->pActUndoArray == m_pData->pUndoArray, "svl::SfxUndoManager::Undo(), LeaveListAction() not yet called!" );
    if ( m_pData->pActUndoArray->nCurUndoAction )
    {
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[ --m_pData->pActUndoArray->nCurUndoAction ];
        pAction->Undo();
        bRet = TRUE;

        for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
                listener != m_pData->aListeners.end();
                ++listener
            )
        {
            (*listener)->actionUndone( *pAction );
        }
    }

    return bRet;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRedoActionCount() const
{
    return m_pData->pActUndoArray->aUndoActions.Count() - m_pData->pActUndoArray->nCurUndoAction; //!
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( USHORT nNo ) const
{
    return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction+nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Redo()
{
    DBG_TESTSOLARMUTEX();
    OSL_ENSURE( !IsDoing(), "SfxUndoManager::Redo: *nested* Undo/Redo actions? How this?" );

    ::comphelper::FlagGuard aGuard( m_pData->mbDoing );
    LockGuard aLockGuard( *this );

    BOOL bRet = FALSE;

    if ( m_pData->pActUndoArray->aUndoActions.Count() > m_pData->pActUndoArray->nCurUndoAction )
    {
        SfxUndoAction* pAction = m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction++];
        pAction->Redo();
        bRet = TRUE;

        for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
                listener != m_pData->aListeners.end();
                ++listener
            )
        {
            (*listener)->actionRedone( *pAction );
        }
    }

    return bRet;
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

/*  [Beschreibung]

    Verlaesst die aktuelle ListAction und geht eine Ebene nach oben.
*/
{
    if ( !IsUndoEnabled() )
        return 0;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return 0;

    if( !IsInListAction() )
    {
        DBG_ERROR( "svl::SfxUndoManager::LeaveListAction(), called without calling EnterListAction()!" );
        return 0;
    }

    DBG_ASSERT(m_pData->pActUndoArray->pFatherUndoArray,"svl::SfxUndoManager::LeaveListAction(), no father undo array!?");

    SfxUndoArray* pTmp = m_pData->pActUndoArray;
    m_pData->pActUndoArray = m_pData->pActUndoArray->pFatherUndoArray;

    // If no undo action where added, delete the undo list action
    SfxUndoAction *pTmpAction= m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1];
    const USHORT nListActionElements = pTmp->nCurUndoAction;
    if( nListActionElements == 0 )
    {
        m_pData->pActUndoArray->aUndoActions.Remove( --m_pData->pActUndoArray->nCurUndoAction);
        delete pTmpAction;
    }
    else
    {
        // if the undo array has no comment, try to get it from its children
        SfxListUndoAction* pList = dynamic_cast< SfxListUndoAction * >( pTmpAction );
        if( pList && pList->GetComment().Len() == 0 )
        {
            USHORT n;
            for( n = 0; n < pList->aUndoActions.Count(); n++ )
            {
                if( pList->aUndoActions[n]->GetComment().Len() )
                {
                    pList->SetComment( pList->aUndoActions[n]->GetComment() );
                    break;
                }
            }
        }
    }

    // notify listeners
    const bool leftContext = ( nListActionElements > 0 );
    ::std::for_each( m_pData->aListeners.begin(), m_pData->aListeners.end(),
        NotifyUndoListener( leftContext ? &SfxUndoListener::listActionLeft : &SfxUndoListener::listActionCancelled ) );

    return nListActionElements;
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
        USHORT nPos = pManager->GetUndoActionCount()-1;
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



