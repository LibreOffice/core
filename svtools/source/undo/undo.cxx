/*************************************************************************
 *
 *  $RCSfile: undo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#pragma hdrstop

#include "undo.hxx"
#include "svarray.hxx"

//========================================================================

SV_DECL_PTRARR( SfxUndoActions, SfxUndoAction*, 20, 8 )

//====================================================================

struct SfxUndoArray
{
    SfxUndoActions          aUndoActions;
    USHORT                  nMaxUndoActions;
    USHORT                  nCurUndoAction;
    SfxUndoArray            *pFatherUndoArray;
                            SfxUndoArray(USHORT nMax=0) : pFatherUndoArray(0),
                                nCurUndoAction(0),nMaxUndoActions(nMax){}
                           ~SfxUndoArray();
};

//=========================================================================

class SfxListUndoAction : public SfxUndoAction, public SfxUndoArray

/*  [Beschreibung]

    UndoAction zur Klammerung mehrerer Undos in einer UndoAction.
    Diese Actions werden vom SfxUndoManager verwendet. Dort
    wird mit < SfxUndoManager::EnterListAction > eine Klammerebene
    geoeffnet und mit <SfxUndoManager::LeaveListAction > wieder
    geschlossen. Redo und Undo auf SfxListUndoActions wirken
    Elementweise.

*/




{
    public:
                            TYPEINFO();

                            SfxListUndoAction( const UniString &rComment,
                                const UniString rRepeatComment, USHORT Id, SfxUndoArray *pFather);
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual BOOL            CanRepeat(SfxRepeatTarget&) const;

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT          GetId() const;


    private:

    USHORT                  nId;
    UniString                   aComment, aRepeatComment;

};

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxUndoAction);

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
    DBG_ASSERT( !IsLinked(), "Gelinkte Action geloescht" )
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


SfxUndoManager::SfxUndoManager( USHORT nMaxUndoActionCount )
 : pFatherUndoArray(0)
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


void SfxUndoManager::SetMaxUndoActionCount( USHORT nMaxUndoActionCount )
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
            USHORT nPos = pActUndoArray->aUndoActions.Count();
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

USHORT SfxUndoManager::GetMaxUndoActionCount() const
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

void SfxUndoManager::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerge )
{
    // Redo-Actions loeschen
    for ( USHORT nPos = pActUndoArray->aUndoActions.Count();
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

    delete pAction;
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionCount() const
{
    return pActUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetUndoActionComment( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetUndoActionId( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetId(); //!
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Undo( USHORT nCount )
{
    if ( pActUndoArray->nCurUndoAction )
    {
        Undo( *pActUndoArray->aUndoActions[ --pActUndoArray->nCurUndoAction ] );
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::Undo( SfxUndoAction &rAction )
{
    rAction.Undo();
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRedoActionCount() const
{
    return pActUndoArray->aUndoActions.Count() - pActUndoArray->nCurUndoAction; //!
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRedoActionComment( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRedoActionId( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetId(); //!
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Redo( USHORT nNumber )
{
    if ( pActUndoArray->aUndoActions.Count() > pActUndoArray->nCurUndoAction )
    {
        Redo( *pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction++] );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::Redo( SfxUndoAction &rAction )
{
    rAction.Redo();
}

//------------------------------------------------------------------------

USHORT SfxUndoManager::GetRepeatActionCount() const
{
    return pActUndoArray->aUndoActions.Count();
}

//------------------------------------------------------------------------

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[ pActUndoArray->aUndoActions.Count() - 1 - nNo ]
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

BOOL SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, USHORT nFrom, USHORT nCount )
{
    if ( pActUndoArray->aUndoActions.Count() )
    {
        Repeat( rTarget, *pActUndoArray->aUndoActions[ pActUndoArray->aUndoActions.Count() - 1 ] );
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
    if ( pActUndoArray->aUndoActions.Count() > nNo )
    {
        USHORT nActionNo = pActUndoArray->aUndoActions.Count() - 1 - nNo;
        return pActUndoArray->aUndoActions[nActionNo]->CanRepeat(rTarget);
    }

    return FALSE;
}

//------------------------------------------------------------------------

void SfxUndoManager::EnterListAction(
    const XubString& rComment, const XubString &rRepeatComment, USHORT nId )

/*  [Beschreibung]

    Fuegt eine ListUndoAction ein und setzt dessen UndoArray als aktuelles.
*/

{
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
    if ( !pUndoArray->nMaxUndoActions )
        return;

    DBG_ASSERT(pActUndoArray->pFatherUndoArray,"Keine hoehere Undo Ebene existent");

    SfxUndoArray* pTmp=pActUndoArray;
    pActUndoArray=pActUndoArray->pFatherUndoArray;

//Falls keine UndoAction eingefuegt wurde, entferne die UndoListAction

    if(!pTmp->nCurUndoAction)
    {
        SfxUndoAction *pTmpAction=
            pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1];
        pActUndoArray->aUndoActions.Remove( --pActUndoArray->nCurUndoAction);
        delete pTmpAction;
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

XubString SfxListUndoAction::GetRepeatComment(SfxRepeatTarget &r) const
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
: aComment(rComment), aRepeatComment(rRepeatComment), nId(Id)
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


