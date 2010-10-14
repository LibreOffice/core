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

#include <tools/debug.hxx>

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

    bool            mbUndoEnabled;

    UndoListeners   aListeners;

    SfxUndoManager_Data( USHORT i_nMaxUndoActionCount )
        :pUndoArray( new SfxUndoArray( i_nMaxUndoActionCount ) )
        ,pActUndoArray( NULL )
        ,pFatherUndoArray( NULL )
        ,mbUndoEnabled( true )

    {
        pActUndoArray = pUndoArray;
    }

    ~SfxUndoManager_Data()
    {
        delete pUndoArray;
    }
};

//========================================================================

SfxUndoManager::SfxUndoManager( USHORT nMaxUndoActionCount )
    :m_pData( new SfxUndoManager_Data( nMaxUndoActionCount ) )
{
}

//------------------------------------------------------------------------

SfxUndoManager::~SfxUndoManager()
{
    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->undoManagerDying();
    }
}

//------------------------------------------------------------------------

void SfxUndoManager::EnableUndo( bool bEnable )
{
    m_pData->mbUndoEnabled = bEnable;
}

//------------------------------------------------------------------------

bool SfxUndoManager::IsUndoEnabled() const
{
    return m_pData->mbUndoEnabled;
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

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->cleared();
    }
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

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->clearedRedo();
    }
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerge )
{
    if( m_pData->mbUndoEnabled )
    {
        // Redo-Actions loeschen
        for ( USHORT nPos = m_pData->pActUndoArray->aUndoActions.Count();
              nPos > m_pData->pActUndoArray->nCurUndoAction; --nPos )
            delete m_pData->pActUndoArray->aUndoActions[nPos-1];

        m_pData->pActUndoArray->aUndoActions.Remove(
            m_pData->pActUndoArray->nCurUndoAction,
            m_pData->pActUndoArray->aUndoActions.Count() - m_pData->pActUndoArray->nCurUndoAction );

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

USHORT SfxUndoManager::GetUndoActionId( USHORT nNo ) const
{
    DBG_ASSERT( nNo < m_pData->pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionId(), illegal id!" );
    if( nNo < m_pData->pActUndoArray->nCurUndoAction )
    {
        return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1-nNo]->GetId(); //!
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

BOOL SfxUndoManager::Undo()
{
    bool bUndoWasEnabled =  m_pData->mbUndoEnabled;
    m_pData->mbUndoEnabled = false;

    BOOL bRet = FALSE;

    try
    {
        DBG_ASSERT( m_pData->pActUndoArray == m_pData->pUndoArray, "svl::SfxUndoManager::Undo(), LeaveListAction() not yet called!" );
        if ( m_pData->pActUndoArray->nCurUndoAction )
        {
            ImplUndo( *m_pData->pActUndoArray->aUndoActions[ --m_pData->pActUndoArray->nCurUndoAction ] );
            bRet = TRUE;
        }
    }
    catch( const Exception& )
    {
        m_pData->mbUndoEnabled = bUndoWasEnabled;
        throw;
    }
    m_pData->mbUndoEnabled = bUndoWasEnabled;
    return bRet;
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplUndo( SfxUndoAction &rAction )
{
    bool bUndoWasEnabled =  m_pData->mbUndoEnabled;
    m_pData->mbUndoEnabled = false;
    try
    {
        rAction.Undo();
        for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
                listener != m_pData->aListeners.end();
                ++listener
            )
        {
            (*listener)->actionUndone( rAction );
        }
    }
    catch( const Exception& )
    {
        m_pData->mbUndoEnabled = bUndoWasEnabled;
        throw;
    }

    m_pData->mbUndoEnabled = bUndoWasEnabled;
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

USHORT SfxUndoManager::GetRedoActionId( USHORT nNo ) const
{
    return m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction+nNo]->GetId(); //!
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Redo()
{
    bool bUndoWasEnabled =  m_pData->mbUndoEnabled;
    m_pData->mbUndoEnabled = false;

    BOOL bRet = FALSE;

    try
    {
        if ( m_pData->pActUndoArray->aUndoActions.Count() > m_pData->pActUndoArray->nCurUndoAction )
        {
            ImplRedo( *m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction++] );
            bRet = TRUE;
        }
    }
    catch( const Exception& )
    {
        m_pData->mbUndoEnabled = bUndoWasEnabled;
        throw;
    }

    m_pData->mbUndoEnabled = bUndoWasEnabled;
    return bRet;
}

//------------------------------------------------------------------------

void SfxUndoManager::ImplRedo( SfxUndoAction &rAction )
{
    bool bUndoWasEnabled =  m_pData->mbUndoEnabled;
    m_pData->mbUndoEnabled = false;

    try
    {
        rAction.Redo();
        for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
                listener != m_pData->aListeners.end();
                ++listener
            )
        {
            (*listener)->actionRedone( rAction );
        }
    }
    catch( const Exception& )
    {
        m_pData->mbUndoEnabled = bUndoWasEnabled;
        throw;
    }

    m_pData->mbUndoEnabled = bUndoWasEnabled;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRepeatActionCount() const
{
    return m_pData->pActUndoArray->aUndoActions.Count();
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo ) const
{
    return m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.Count() - 1 - nNo ]
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, USHORT /*nFrom*/, USHORT /*nCount*/ )
{
    if ( m_pData->pActUndoArray->aUndoActions.Count() )
    {
        Repeat( rTarget, *m_pData->pActUndoArray->aUndoActions[ m_pData->pActUndoArray->aUndoActions.Count() - 1 ] );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction )
{
    if ( rAction.CanRepeat(rTarget) )
        rAction.Repeat(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const
{
    return rAction.CanRepeat(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget, USHORT nNo ) const
{
    if ( m_pData->pActUndoArray->aUndoActions.Count() > nNo )
    {
        USHORT nActionNo = m_pData->pActUndoArray->aUndoActions.Count() - 1 - nNo;
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
    if( !m_pData->mbUndoEnabled )
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

void SfxUndoManager::LeaveListAction()

/*  [Beschreibung]

    Verlaesst die aktuelle ListAction und geht eine Ebene nach oben.
*/
{
    if ( !m_pData->mbUndoEnabled )
        return;

    if ( !m_pData->pUndoArray->nMaxUndoActions )
        return;

    if( !IsInListAction() )
    {
        DBG_ERROR( "svl::SfxUndoManager::LeaveListAction(), called without calling EnterListAction()!" );
        return;
    }

    DBG_ASSERT(m_pData->pActUndoArray->pFatherUndoArray,"svl::SfxUndoManager::LeaveListAction(), no father undo array!?");

    SfxUndoArray* pTmp = m_pData->pActUndoArray;
    m_pData->pActUndoArray = m_pData->pActUndoArray->pFatherUndoArray;

    // If no undo action where added, delete the undo list action
    SfxUndoAction *pTmpAction= m_pData->pActUndoArray->aUndoActions[m_pData->pActUndoArray->nCurUndoAction-1];
    if(!pTmp->nCurUndoAction)
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

    for (   UndoListeners::const_iterator listener = m_pData->aListeners.begin();
            listener != m_pData->aListeners.end();
            ++listener
        )
    {
        (*listener)->listActionLeft();
    }
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

SfxLinkUndoAction::SfxLinkUndoAction(SfxUndoManager *pManager)
/*  [Beschreibung]

    Richtet eine LinkAction ein, die auf einen weiteren UndoManager zeigt.
    Holt sich als zugehoerige Action des weiteren UndoManagers dessen
    aktuelle Action.
*/

{
    pUndoManager = pManager;
    if ( pManager->GetMaxUndoActionCount() )
    {
        USHORT nPos = pManager->GetUndoActionCount()-1;
        pAction = pManager->m_pData->pActUndoArray->aUndoActions[nPos];
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
    return pAction && pUndoManager->CanRepeat(r,*pAction);
}


//------------------------------------------------------------------------


void SfxLinkUndoAction::Repeat(SfxRepeatTarget&r)
{
    if ( pAction )
        pUndoManager->Repeat(r,*pAction);
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



