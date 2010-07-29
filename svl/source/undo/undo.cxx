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


sal_Bool SfxUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return sal_True;
}

//========================================================================


SfxUndoManager::SfxUndoManager( sal_uInt16 nMaxUndoActionCount )
 : pFatherUndoArray(0)
 , mbUndoEnabled( true )
{
    pUndoArray=new SfxUndoArray(nMaxUndoActionCount);
    pActUndoArray=pUndoArray;

}

//------------------------------------------------------------------------


SfxUndoManager::~SfxUndoManager()
{
    delete pUndoArray;
}

//------------------------------------------------------------------------

void SfxUndoManager::EnableUndo( bool bEnable )
{
    mbUndoEnabled = bEnable;
}

//------------------------------------------------------------------------


void SfxUndoManager::SetMaxUndoActionCount( sal_uInt16 nMaxUndoActionCount )
{
    // Remove entries from the pActUndoArray when we have to reduce
    // the number of entries due to a lower nMaxUndoActionCount.
    // Both redo and undo action entries will be removed until we reached the
    // new nMaxUndoActionCount.

    long nNumToDelete = pActUndoArray->aUndoActions.Count() - nMaxUndoActionCount;
    if ( nNumToDelete > 0 )
    {
        while ( nNumToDelete > 0 )
        {
            sal_uInt16 nPos = pActUndoArray->aUndoActions.Count();
            if ( nPos > pActUndoArray->nCurUndoAction )
            {
                if ( !pActUndoArray->aUndoActions[nPos-1]->IsLinked() )
                {
                    delete pActUndoArray->aUndoActions[nPos-1];
                    pActUndoArray->aUndoActions.Remove( nPos-1 );
                    --nNumToDelete;
                }
            }

            if ( nNumToDelete > 0 && pActUndoArray->nCurUndoAction > 0 )
            {
                if ( !pActUndoArray->aUndoActions[0]->IsLinked() )
                {
                    delete pActUndoArray->aUndoActions[0];
                    pActUndoArray->aUndoActions.Remove(0);
                    --pActUndoArray->nCurUndoAction;
                    --nNumToDelete;
                }
            }

            if ( nPos == pActUndoArray->aUndoActions.Count() )
                break; // Cannot delete more entries
        }
    }

    pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetMaxUndoActionCount() const
{
    return pActUndoArray->nMaxUndoActions;
}

//------------------------------------------------------------------------

void SfxUndoManager::Clear()
{
    while ( pActUndoArray->aUndoActions.Count() )
    {
        SfxUndoAction *pAction=
            pActUndoArray->aUndoActions[pActUndoArray->aUndoActions.Count() - 1];
        pActUndoArray->aUndoActions.Remove( pActUndoArray->aUndoActions.Count() - 1 );
        delete pAction;
    }

    pActUndoArray->nCurUndoAction = 0;
}

//------------------------------------------------------------------------

void SfxUndoManager::ClearRedo()
{
    while ( pActUndoArray->aUndoActions.Count() > pActUndoArray->nCurUndoAction )
    {
        SfxUndoAction *pAction=
            pActUndoArray->aUndoActions[pActUndoArray->aUndoActions.Count() - 1];
        pActUndoArray->aUndoActions.Remove( pActUndoArray->aUndoActions.Count() - 1 );
        delete pAction;
    }
}

//------------------------------------------------------------------------

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerge )
{
    if( mbUndoEnabled )
    {
        // Redo-Actions loeschen
        for ( sal_uInt16 nPos = pActUndoArray->aUndoActions.Count();
              nPos > pActUndoArray->nCurUndoAction; --nPos )
            delete pActUndoArray->aUndoActions[nPos-1];

        pActUndoArray->aUndoActions.Remove(
            pActUndoArray->nCurUndoAction,
            pActUndoArray->aUndoActions.Count() - pActUndoArray->nCurUndoAction );

        if ( pActUndoArray->nMaxUndoActions )
        {
            SfxUndoAction *pTmpAction = pActUndoArray->nCurUndoAction ?
                pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1] : 0;

            if ( !bTryMerge || !(pTmpAction && pTmpAction->Merge(pAction)) )
            {
                // auf Max-Anzahl anpassen
                if( pActUndoArray == pUndoArray )
                    while( pActUndoArray->aUndoActions.Count() >=
                           pActUndoArray->nMaxUndoActions &&
                           !pActUndoArray->aUndoActions[0]->IsLinked() )
                    {
                        delete pActUndoArray->aUndoActions[0];
                        pActUndoArray->aUndoActions.Remove(0);
                        --pActUndoArray->nCurUndoAction;
                    }

                // neue Action anh"angen
                const SfxUndoAction* pTemp = pAction;
                pActUndoArray->aUndoActions.Insert(
                    pTemp, pActUndoArray->nCurUndoAction++ );
                return;
            }
        }
    }
    delete pAction;
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetUndoActionCount() const
{
    return pActUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( sal_uInt16 nNo ) const
{
    DBG_ASSERT( nNo < pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionComment(), illegal id!" );
    if( nNo < pActUndoArray->nCurUndoAction )
    {
        return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetComment(); //!
    }
    else
    {
        XubString aEmpty;
        return aEmpty;
    }
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetUndoActionId( sal_uInt16 nNo ) const
{
    DBG_ASSERT( nNo < pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoActionId(), illegal id!" );
    if( nNo < pActUndoArray->nCurUndoAction )
    {
        return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetId(); //!
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------

SfxUndoAction* SfxUndoManager::GetUndoAction( sal_uInt16 nNo ) const
{
    DBG_ASSERT( nNo < pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::GetUndoAction(), illegal id!" );
    if( nNo < pActUndoArray->nCurUndoAction )
    {
        return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]; //!
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
    DBG_ASSERT( pActUndoArray->nCurUndoAction, "svl::SfxUndoManager::RemoveLastUndoAction(), no action to remove?!" );
    if( pActUndoArray->nCurUndoAction )
    {
        pActUndoArray->nCurUndoAction--;

        // delete redo-actions and top action
        sal_uInt16 nPos;
        for ( nPos = pActUndoArray->aUndoActions.Count(); nPos > pActUndoArray->nCurUndoAction; --nPos )
            delete pActUndoArray->aUndoActions[nPos-1];

        pActUndoArray->aUndoActions.Remove(
            pActUndoArray->nCurUndoAction,
            pActUndoArray->aUndoActions.Count() - pActUndoArray->nCurUndoAction );
    }
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::Undo( sal_uInt16 )
{
    bool bUndoWasEnabled =  mbUndoEnabled;
    mbUndoEnabled = false;

    sal_Bool bRet = sal_False;

    try
    {
        DBG_ASSERT( pActUndoArray == pUndoArray, "svl::SfxUndoManager::Undo(), LeaveListAction() not yet called!" );
        if ( pActUndoArray->nCurUndoAction )
        {
            Undo( *pActUndoArray->aUndoActions[ --pActUndoArray->nCurUndoAction ] );
            bRet = sal_True;
        }
    }
    catch( Exception& e )
    {
        mbUndoEnabled = bUndoWasEnabled;
        throw e;
    }
    mbUndoEnabled = bUndoWasEnabled;
    return bRet;
}

//------------------------------------------------------------------------

void SfxUndoManager::Undo( SfxUndoAction &rAction )
{
    bool bUndoWasEnabled =  mbUndoEnabled;
    mbUndoEnabled = false;
    try
    {
        rAction.Undo();
    }
    catch( Exception& e )
    {
        mbUndoEnabled = bUndoWasEnabled;
        throw e;
    }

    mbUndoEnabled = bUndoWasEnabled;
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetRedoActionCount() const
{
    return pActUndoArray->aUndoActions.Count() - pActUndoArray->nCurUndoAction; //!
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( sal_uInt16 nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetRedoActionId( sal_uInt16 nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetId(); //!
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::Redo( sal_uInt16 )
{
    bool bUndoWasEnabled =  mbUndoEnabled;
    mbUndoEnabled = false;

    sal_Bool bRet = sal_False;

    try
    {
        if ( pActUndoArray->aUndoActions.Count() > pActUndoArray->nCurUndoAction )
        {
            Redo( *pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction++] );
            bRet = sal_True;
        }
    }
    catch( Exception& e )
    {
        mbUndoEnabled = bUndoWasEnabled;
        throw e;
    }

    mbUndoEnabled = bUndoWasEnabled;
    return bRet;
}

//------------------------------------------------------------------------

void SfxUndoManager::Redo( SfxUndoAction &rAction )
{
    bool bUndoWasEnabled =  mbUndoEnabled;
    mbUndoEnabled = false;

    try
    {
        rAction.Redo();
    }
    catch( Exception& e )
    {
        mbUndoEnabled = bUndoWasEnabled;
        throw e;
    }

    mbUndoEnabled = bUndoWasEnabled;
}

//------------------------------------------------------------------------

sal_uInt16 SfxUndoManager::GetRepeatActionCount() const
{
    return pActUndoArray->aUndoActions.Count();
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget, sal_uInt16 nNo ) const
{
    return pActUndoArray->aUndoActions[ pActUndoArray->aUndoActions.Count() - 1 - nNo ]
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, sal_uInt16 /*nFrom*/, sal_uInt16 /*nCount*/ )
{
    if ( pActUndoArray->aUndoActions.Count() )
    {
        Repeat( rTarget, *pActUndoArray->aUndoActions[ pActUndoArray->aUndoActions.Count() - 1 ] );
        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

void SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction )
{
    if ( rAction.CanRepeat(rTarget) )
        rAction.Repeat(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const
{
    return rAction.CanRepeat(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget, sal_uInt16 nNo ) const
{
    if ( pActUndoArray->aUndoActions.Count() > nNo )
    {
        sal_uInt16 nActionNo = pActUndoArray->aUndoActions.Count() - 1 - nNo;
        return pActUndoArray->aUndoActions[nActionNo]->CanRepeat(rTarget);
    }

    return sal_False;
}

//------------------------------------------------------------------------

void SfxUndoManager::EnterListAction(
    const XubString& rComment, const XubString &rRepeatComment, sal_uInt16 nId )

/*  [Beschreibung]

    Fuegt eine ListUndoAction ein und setzt dessen UndoArray als aktuelles.
*/

{
    if( !mbUndoEnabled )
        return;

    if ( !pUndoArray->nMaxUndoActions )
        return;

    pFatherUndoArray=pActUndoArray;
    SfxListUndoAction *pAction=new SfxListUndoAction(
        rComment, rRepeatComment, nId, pActUndoArray);
    AddUndoAction( pAction );
    pActUndoArray=pAction;
}

//------------------------------------------------------------------------

void SfxUndoManager::LeaveListAction()

/*  [Beschreibung]

    Verlaesst die aktuelle ListAction und geht eine Ebene nach oben.
*/
{
    if ( !mbUndoEnabled )
        return;

    if ( !pUndoArray->nMaxUndoActions )
        return;

    if( pActUndoArray == pUndoArray )
    {
        DBG_ERROR( "svl::SfxUndoManager::LeaveListAction(), called without calling EnterListAction()!" );
        return;
    }

    DBG_ASSERT(pActUndoArray->pFatherUndoArray,"svl::SfxUndoManager::LeaveListAction(), no father undo array!?");

    SfxUndoArray* pTmp=pActUndoArray;
    pActUndoArray=pActUndoArray->pFatherUndoArray;

    // If no undo action where added, delete the undo list action
    SfxUndoAction *pTmpAction= pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1];
    if(!pTmp->nCurUndoAction)
    {
        pActUndoArray->aUndoActions.Remove( --pActUndoArray->nCurUndoAction);
        delete pTmpAction;
    }
    else
    {
        // if the undo array has no comment, try to get it from its children
        SfxListUndoAction* pList = dynamic_cast< SfxListUndoAction * >( pTmpAction );
        if( pList && pList->GetComment().Len() == 0 )
        {
            sal_uInt16 n;
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
    for(sal_Int16 i=nCurUndoAction-1;i>=0;i--)
        aUndoActions[i]->Undo();
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

void SfxListUndoAction::Redo()
{
    for(sal_uInt16 i=nCurUndoAction;i<aUndoActions.Count();i++)
        aUndoActions[i]->Redo();
    nCurUndoAction = aUndoActions.Count();
}

//------------------------------------------------------------------------

void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(sal_uInt16 i=0;i<nCurUndoAction;i++)
        aUndoActions[i]->Repeat(rTarget);
}

//------------------------------------------------------------------------

sal_Bool SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(sal_uInt16 i=0;i<nCurUndoAction;i++)
        if(!aUndoActions[i]->CanRepeat(r))
            return sal_False;
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SfxListUndoAction::Merge( SfxUndoAction *pNextAction )
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
        sal_uInt16 nPos = pManager->GetUndoActionCount()-1;
        pAction = pManager->pActUndoArray->aUndoActions[nPos];
        pAction->SetLinked();
    }
    else
        pAction = 0;
}

//------------------------------------------------------------------------

void SfxLinkUndoAction::Undo()
{
    if ( pAction )
        pUndoManager->Undo(1);
}

//------------------------------------------------------------------------

void SfxLinkUndoAction::Redo()
{
    if ( pAction )
        pUndoManager->Redo(1);
}

//------------------------------------------------------------------------


sal_Bool SfxLinkUndoAction::CanRepeat(SfxRepeatTarget& r) const
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
        pAction->SetLinked( sal_False );
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


sal_uInt16 SfxLinkUndoAction::GetId() const
{
      return pAction ? pAction->GetId() : 0;
}



