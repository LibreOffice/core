/*************************************************************************
 *
 *  $RCSfile: undo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:06 $
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

#include "segmentc.hxx"

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

#ifndef ENABLEUNICODE
                            SfxListUndoAction( const String &rComment,
                                const String rRepeatComment, USHORT Id, SfxUndoArray *pFather);
#else
                            SfxListUndoAction( const UniString &rComment,
                                const UniString rRepeatComment, USHORT Id, SfxUndoArray *pFather);
#endif
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual BOOL            CanRepeat(SfxRepeatTarget&) const;

#ifndef ENABLEUNICODE
    virtual String          GetComment() const;
    virtual String          GetRepeatComment(SfxRepeatTarget&) const;
#else
    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
#endif
    virtual USHORT          GetId() const;


    private:

    USHORT                  nId;
#ifndef ENABLEUNICODE
    String                  aComment, aRepeatComment;
#else
    UniString                   aComment, aRepeatComment;
#endif

};

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxUndoAction);

SEG_EOFGLOBALS()

//========================================================================

TYPEINIT0(SfxUndoAction);
TYPEINIT0(SfxListUndoAction);
TYPEINIT0(SfxLinkUndoAction);
TYPEINIT0(SfxRepeatTarget);

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_01)

SfxRepeatTarget::~SfxRepeatTarget()
{
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_34)

BOOL SfxUndoAction::IsLinked()
{
    return bLinked;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_35)

void SfxUndoAction::SetLinked( BOOL bIsLinked )
{
    bLinked = bIsLinked;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_36)

SfxUndoAction::~SfxUndoAction()
{
    DBG_DTOR(SfxUndoAction, 0);
    DBG_ASSERT( !IsLinked(), "Gelinkte Action geloescht" )
}


#pragma SEG_FUNCDEF(undo_02)

SfxUndoAction::SfxUndoAction()
{
    DBG_CTOR(SfxUndoAction, 0);
    SetLinked( FALSE );
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_03)

BOOL SfxUndoAction::Merge( SfxUndoAction * )
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return FALSE;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_04)

XubString SfxUndoAction::GetComment() const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return XubString();
}

//------------------------------------------------------------------------


#pragma SEG_FUNCDEF(undo_05)

USHORT SfxUndoAction::GetId() const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return 0;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_06)

XubString SfxUndoAction::GetRepeatComment(SfxRepeatTarget&) const
{
    DBG_CHKTHIS(SfxUndoAction, 0);
    return GetComment();
}

//------------------------------------------------------------------------


#pragma SEG_FUNCDEF(undo_07)

void SfxUndoAction::Undo()
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Undo()" );
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_08)

void SfxUndoAction::Redo()
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Redo()" );
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_09)

void SfxUndoAction::Repeat(SfxRepeatTarget&)
{
    // die sind nur konzeptuell pure virtual
    DBG_ERROR( "pure virtual function called: SfxUndoAction::Repeat()" );
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_0A)

BOOL SfxUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return TRUE;
}

//========================================================================

#pragma SEG_FUNCDEF(undo_0B)

SfxUndoManager::SfxUndoManager( USHORT nMaxUndoActionCount )
 : pFatherUndoArray(0)
{
    pUndoArray=new SfxUndoArray(nMaxUndoActionCount);
    pActUndoArray=pUndoArray;

}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_0C)

SfxUndoManager::~SfxUndoManager()
{
    delete pUndoArray;
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_0D)

void SfxUndoManager::SetMaxUndoActionCount( USHORT nMaxUndoActionCount )
{
    // Redo-Actions loeschen
    for ( USHORT nPos = pActUndoArray->aUndoActions.Count();
          nPos > pActUndoArray->nCurUndoAction &&
              !pActUndoArray->aUndoActions[nPos - 1 ]->IsLinked();
          --nPos )
    {
        delete pActUndoArray->aUndoActions[nPos-1];
        pActUndoArray->aUndoActions.Remove(
            nPos - 1 );
    }

    while ( nMaxUndoActionCount < pActUndoArray->aUndoActions.Count() &&
            !pActUndoArray->aUndoActions[0]->IsLinked())
    {
        delete pActUndoArray->aUndoActions[0];
        pActUndoArray->aUndoActions.Remove(0);
        --pActUndoArray->nCurUndoAction;
    }

    pActUndoArray->nMaxUndoActions = nMaxUndoActionCount;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_0E)

USHORT SfxUndoManager::GetMaxUndoActionCount() const
{
    return pActUndoArray->nMaxUndoActions;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_0F)

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

#pragma SEG_FUNCDEF(undo_10)

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

#pragma SEG_FUNCDEF(undo_11)

USHORT SfxUndoManager::GetUndoActionCount() const
{
    return pActUndoArray->nCurUndoAction;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_12)

XubString SfxUndoManager::GetUndoActionComment( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_13)

USHORT SfxUndoManager::GetUndoActionId( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction-1-nNo]->GetId(); //!
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_14)

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

#pragma SEG_FUNCDEF(undo_15)

void SfxUndoManager::Undo( SfxUndoAction &rAction )
{
    rAction.Undo();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_16)

USHORT SfxUndoManager::GetRedoActionCount() const
{
    return pActUndoArray->aUndoActions.Count() - pActUndoArray->nCurUndoAction; //!
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_17)

XubString SfxUndoManager::GetRedoActionComment( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetComment(); //!
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_18)

USHORT SfxUndoManager::GetRedoActionId( USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[pActUndoArray->nCurUndoAction+nNo]->GetId(); //!
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_19)

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

#pragma SEG_FUNCDEF(undo_1A)

void SfxUndoManager::Redo( SfxUndoAction &rAction )
{
    rAction.Redo();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_1B)

USHORT SfxUndoManager::GetRepeatActionCount() const
{
    return pActUndoArray->aUndoActions.Count();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_1C)

XubString SfxUndoManager::GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo ) const
{
    return pActUndoArray->aUndoActions[ pActUndoArray->aUndoActions.Count() - 1 - nNo ]
        ->GetRepeatComment(rTarget);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_1D)

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

#pragma SEG_FUNCDEF(undo_1E)

void SfxUndoManager::Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction )
{
    if ( rAction.CanRepeat(rTarget) )
        rAction.Repeat(rTarget);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_1F)

BOOL SfxUndoManager::CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const
{
    return rAction.CanRepeat(rTarget);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_20)

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

#pragma SEG_FUNCDEF(undo_21)

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

#pragma SEG_FUNCDEF(undo_22)

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

#pragma SEG_FUNCDEF(undo_23)

USHORT SfxListUndoAction::GetId() const
{
    return nId;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_24)

XubString SfxListUndoAction::GetComment() const
{
    return aComment;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_25)

XubString SfxListUndoAction::GetRepeatComment(SfxRepeatTarget &r) const
{
    return aRepeatComment;
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_26)

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

#pragma SEG_FUNCDEF(undo_27)

void SfxListUndoAction::Undo()
{
    for(INT16 i=nCurUndoAction-1;i>=0;i--)
        aUndoActions[i]->Undo();
    nCurUndoAction=0;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_28)

void SfxListUndoAction::Redo()
{
    for(USHORT i=nCurUndoAction;i<aUndoActions.Count();i++)
        aUndoActions[i]->Redo();
    nCurUndoAction = aUndoActions.Count();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_29)

void SfxListUndoAction::Repeat(SfxRepeatTarget&rTarget)
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        aUndoActions[i]->Repeat(rTarget);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_2A)

BOOL SfxListUndoAction::CanRepeat(SfxRepeatTarget&r)  const
{
    for(USHORT i=0;i<nCurUndoAction;i++)
        if(!aUndoActions[i]->CanRepeat(r))
            return FALSE;
    return TRUE;
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_2B)

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

#pragma SEG_FUNCDEF(undo_2C)

void SfxLinkUndoAction::Undo()
{
    if ( pAction )
        pUndoManager->Undo(1);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_2D)

void SfxLinkUndoAction::Redo()
{
    if ( pAction )
        pUndoManager->Redo(1);
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_2E)

BOOL SfxLinkUndoAction::CanRepeat(SfxRepeatTarget& r) const
{
    return pAction && pUndoManager->CanRepeat(r,*pAction);
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_2F)

void SfxLinkUndoAction::Repeat(SfxRepeatTarget&r)
{
    if ( pAction )
        pUndoManager->Repeat(r,*pAction);
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_30)

XubString SfxLinkUndoAction::GetComment() const
{
    if ( pAction )
        return pAction->GetComment();
    else
        return XubString();
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_31)

XubString SfxLinkUndoAction::GetRepeatComment(SfxRepeatTarget&r) const
{
    if ( pAction )
        return pAction->GetRepeatComment(r);
    else
        return XubString();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_37)

SfxLinkUndoAction::~SfxLinkUndoAction()
{
    if( pAction )
        pAction->SetLinked( FALSE );
}


//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(undo_32)

USHORT SfxLinkUndoAction::GetId() const
{
    return pAction ? pAction->GetId() : 0;
}

#pragma SEG_FUNCDEF(undo_33)

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



/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.44  2000/09/18 14:13:48  willem.vandorp
    OpenOffice header added.

    Revision 1.43  2000/08/31 13:53:28  willem.vandorp
    Header and footer replaced

    Revision 1.42  2000/02/09 16:29:42  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.41  1998/01/20 09:08:18  MH
    chg: header


      Rev 1.40   20 Jan 1998 10:08:18   MH
   chg: header

      Rev 1.39   07 Aug 1997 16:32:38   TJ
   include

      Rev 1.38   01 Oct 1996 13:29:00   MI
   ClearRedo()

      Rev 1.37   13 Jun 1996 10:47:00   MI
   weniger Compiler-Keys

      Rev 1.36   21 Mar 1996 14:56:34   MI
   alte C_VErSION

      Rev 1.35   19 Mar 1996 12:05:34   TLX
   #25200# noch vorsichtiger mit gelinten Actions

      Rev 1.34   14 Mar 1996 17:42:58   MH
   del: S_VERSION

      Rev 1.33   09 Feb 1996 18:43:38   TLX
   #25200# Bei Remove von Undoactions Linking beachten

      Rev 1.32   01 Dec 1995 17:40:34   TLX
   Loesche gemergte Actions

      Rev 1.31   29 Nov 1995 12:09:28   TLX
   Destruktor fuer UndoArrays

      Rev 1.30   23 Nov 1995 16:39:42   AB
   Unicode-Anpassung

      Rev 1.29   13 Nov 1995 10:48:38   TLX
   SetMaxUndoCount loescht nur noch ueberzaehlige Actions

      Rev 1.27   06 Nov 1995 16:18:18   MI
   Bugfix Absturz bei ListActions und zu kleinem MaxCount

      Rev 1.26   04 Oct 1995 16:28:10   MI
   CanRepeat-Syntax-Fehler in 257

      Rev 1.25   29 Sep 1995 08:30:08   MI
   neue Methode SfxUndoManager::CanRepeat() mit Index

      Rev 1.24   07 Aug 1995 19:49:42   TLX
   Auschneiden gibt keinen gpf mehr

      Rev 1.23   31 Jul 1995 14:28:26   MT
   Undo/Redo in ListUndoAction behoben (nCurPos)

      Rev 1.22   31 Jul 1995 13:09:04   PB

      Rev 1.21   31 Jul 1995 12:50:36   PB

      Rev 1.20   31 Jul 1995 12:46:14   PB

      Rev 1.19   31 Jul 1995 12:38:54   PB

      Rev 1.18   31 Jul 1995 11:22:20   TLX

      Rev 1.17   05 Jul 1995 16:10:48   TLX
   Neue Undofeatures

      Rev 1.16   06 Jun 1995 18:48:26   TRI
   ~SfxRepeatTarget impl.

      Rev 1.15   06 Jun 1995 13:26:54   MI
   SfxRepeatTarget -- INKOMPATIBEL

      Rev 1.14   02 May 1995 18:40:36   TRI
   WATCOM Anpassung

      Rev 1.13   18 Apr 1995 19:03:50   MI
   neue Methode Clear() -- kompatibel

      Rev 1.12   29 Mar 1995 17:30:48   MI
   Bugfix: AddUndoAction

      Rev 1.11   23 Mar 1995 18:45:20   MI
   Begrenzung implementiert

      Rev 1.10   15 Dec 1994 09:32:38   MI
   svmem

      Rev 1.9   31 Oct 1994 11:22:54   MI
   CanRepeat

      Rev 1.8   25 Oct 1994 12:00:48   VB
   hdrstop

      Rev 1.7   20 Oct 1994 17:10:54   MI
   Jobsetup

      Rev 1.6   28 Sep 1994 19:29:30   MI
   pch-Probleme und Printing

      Rev 1.5   23 Sep 1994 09:43:14   MI
   Bugfix: Syntaxfehler

      Rev 1.4   23 Sep 1994 09:10:56   MI
   repeat dummy ersetzt

      Rev 1.3   23 Sep 1994 09:01:52   MI
   repeat-dummy

      Rev 1.2   22 Sep 1994 17:37:02   MI
   Undo und ItemSet immer mit Pool

      Rev 1.1   21 Sep 1994 10:49:04   MI
   pch

      Rev 1.0   19 Sep 1994 17:04:18   MI
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

