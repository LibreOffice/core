/*************************************************************************
 *
 *  $RCSfile: undotab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

// ?#define _MACRODLG_HXX
// ? #define _BIGINT_HXX
// ? #define _SVDXOUT_HXX
// ? #define _SVDATTR_HXX
// ? #define _SVDSURO_HXX

// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/smplhint.hxx>

#include "undotab.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "sc.hrc"
#include "undoolk.hxx"
#include "target.hxx"
#include "uiitems.hxx"
#include "prnsave.hxx"
#include "printfun.hxx"
#include "chgtrack.hxx"

extern BOOL bDrawIsInUndo;          //! irgendwo als Member !!!

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoInsertTab,      SfxUndoAction);
TYPEINIT1(ScUndoInsertTables,   SfxUndoAction);
TYPEINIT1(ScUndoDeleteTab,      SfxUndoAction);
TYPEINIT1(ScUndoRenameTab,      SfxUndoAction);
TYPEINIT1(ScUndoMoveTab,        SfxUndoAction);
TYPEINIT1(ScUndoCopyTab,        SfxUndoAction);
TYPEINIT1(ScUndoMakeScenario,   SfxUndoAction);
TYPEINIT1(ScUndoImportTab,      SfxUndoAction);
TYPEINIT1(ScUndoRemoveLink,     SfxUndoAction);
TYPEINIT1(ScUndoShowHideTab,    SfxUndoAction);
TYPEINIT1(ScUndoProtect,        SfxUndoAction);
TYPEINIT1(ScUndoPrintRange,     SfxUndoAction);
TYPEINIT1(ScUndoScenarioFlags,  SfxUndoAction);


// -----------------------------------------------------------------------
//
//      Tabelle einfuegen
//

ScUndoInsertTab::ScUndoInsertTab( ScDocShell* pNewDocShell,
                                  USHORT nTabNum,
                                  BOOL bApp,
                                  const String& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nTabNum ),
    bAppend( bApp ),
    sNewName( rNewName ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
    SetChangeTrack();
}

__EXPORT ScUndoInsertTab::~ScUndoInsertTab()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoInsertTab::GetComment() const
{
    if (bAppend)
        return ScGlobal::GetRscString( STR_UNDO_APPEND_TAB );
    else
        return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTab::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
        pChangeTrack->AppendInsert( aRange );
        nEndChangeAction = pChangeTrack->GetActionMax();
    }
    else
        nEndChangeAction = 0;
}

void __EXPORT ScUndoInsertTab::Undo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo(nTab);

    pDocShell->SetInUndo( TRUE );               //! BeginUndo
    bDrawIsInUndo = TRUE;
    pViewShell->DeleteTable( nTab, FALSE );
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndUndo

    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void __EXPORT ScUndoInsertTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pDrawUndo)
        RedoSdrUndoAction( pDrawUndo );         // Draw Redo vorneweg

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    if (bAppend)
        pViewShell->AppendTable( sNewName, FALSE );
    else
    {
        pViewShell->SetTabNo(nTab);
        pViewShell->InsertTable( sNewName, nTab, FALSE );
    }
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndRedo

    SetChangeTrack();
}

void __EXPORT ScUndoInsertTab::Repeat(SfxRepeatTarget& rTarget)
{
    SFX_DISPATCHER().Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL __EXPORT ScUndoInsertTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Tabellen einfuegen
//

ScUndoInsertTables::ScUndoInsertTables( ScDocShell* pNewDocShell,
                                        USHORT nTabNum,
                                        BOOL bApp,SvStrings *pNewNameList) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nTabNum ),
    bAppend( bApp ),
    pDrawUndo( NULL )
{
    pNameList = pNewNameList;
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    SetChangeTrack();
}

__EXPORT ScUndoInsertTables::~ScUndoInsertTables()
{
    String *pStr=NULL;
    if(pNameList!=NULL)
    {
        for(int i=0;i<pNameList->Count();i++)
        {
            pStr=pNameList->GetObject(i);
            delete pStr;
        }
        pNameList->Remove(0,pNameList->Count());
        delete pNameList;
    }
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoInsertTables::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTables::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
        for( int i = 0; i < pNameList->Count(); i++ )
        {
            aRange.aStart.SetTab( nTab + i );
            aRange.aEnd.SetTab( nTab + i );
            pChangeTrack->AppendInsert( aRange );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void __EXPORT ScUndoInsertTables::Undo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo(nTab);

    pDocShell->SetInUndo( TRUE );               //! BeginUndo
    bDrawIsInUndo = TRUE;

    SvUShorts TheTabs;
    for(int i=0;i<pNameList->Count();i++)
    {
        TheTabs.Insert(nTab+i,TheTabs.Count());
    }

    pViewShell->DeleteTables( TheTabs, FALSE );
    TheTabs.Remove(0,TheTabs.Count());

    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndUndo

    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void __EXPORT ScUndoInsertTables::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pDrawUndo)
        RedoSdrUndoAction( pDrawUndo );         // Draw Redo vorneweg

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    pViewShell->SetTabNo(nTab);
    pViewShell->InsertTables( pNameList, nTab,pNameList->Count(),FALSE );

    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndRedo

    SetChangeTrack();
}

void __EXPORT ScUndoInsertTables::Repeat(SfxRepeatTarget& rTarget)
{
    SFX_DISPATCHER().Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL __EXPORT ScUndoInsertTables::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//----------------------------------------------------------------------------------
//
//      Tabelle loeschen
//

ScUndoDeleteTab::ScUndoDeleteTab( ScDocShell* pNewDocShell,const SvUShorts &aTab, //USHORT nNewTab,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST )
{
        for(int i=0;i<aTab.Count();i++)
            theTabs.Insert(aTab[i],theTabs.Count());

        SetChangeTrack();
}

__EXPORT ScUndoDeleteTab::~ScUndoDeleteTab()
{
    theTabs.Remove(0,theTabs.Count());
}

String __EXPORT ScUndoDeleteTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETE_TAB );
}

void ScUndoDeleteTab::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        ULONG nTmpChangeAction;
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, 0, MAXCOL, MAXROW, 0 );
        for ( int i = 0; i < theTabs.Count(); i++ )
        {
            aRange.aStart.SetTab( theTabs[i] );
            aRange.aEnd.SetTab( theTabs[i] );
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc,
                nTmpChangeAction, nEndChangeAction, (short) i );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void __EXPORT ScUndoDeleteTab::Undo()
{
    BeginUndo();
    int i=0;
    ScDocument* pDoc = pDocShell->GetDocument();

    BOOL bLink = FALSE;
    String aName;

    for(i=0;i<theTabs.Count();i++)
    {
        pRefUndoDoc->GetName( theTabs[i], aName );

        bDrawIsInUndo = TRUE;
        BOOL bOk = pDoc->InsertTab( theTabs[i], aName );
        bDrawIsInUndo = FALSE;
        if (bOk)
        {
            //  Ref-Undo passiert in EndUndo
    //      pUndoDoc->UndoToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pDoc );
            pRefUndoDoc->CopyToDocument(0,0,theTabs[i], MAXCOL,MAXROW,theTabs[i], IDF_ALL,FALSE, pDoc );

            String aOldName;
            pRefUndoDoc->GetName( theTabs[i], aOldName );
            pDoc->RenameTab( theTabs[i], aOldName, FALSE );
            if (pRefUndoDoc->IsLinked(theTabs[i]))
            {
                pDoc->SetLink( theTabs[i], pRefUndoDoc->GetLinkMode(theTabs[i]), pRefUndoDoc->GetLinkDoc(theTabs[i]),
                                    pRefUndoDoc->GetLinkFlt(theTabs[i]), pRefUndoDoc->GetLinkOpt(theTabs[i]),
                                    pRefUndoDoc->GetLinkTab(theTabs[i]) );
                bLink = TRUE;
            }

            if ( pRefUndoDoc->IsScenario(theTabs[i]) )
            {
                pDoc->SetScenario( theTabs[i], TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pRefUndoDoc->GetScenarioData( theTabs[i], aComment, aColor, nScenFlags );
                pDoc->SetScenarioData( theTabs[i], aComment, aColor, nScenFlags );
                BOOL bActive = pRefUndoDoc->IsActiveScenario( theTabs[i] );
                pDoc->SetActiveScenario( theTabs[i], bActive );
            }

            //  Drawing-Layer passiert beim MoveUndo::EndUndo
    //      pDoc->TransferDrawPage(pRefUndoDoc, nTab,nTab);
        }
    }
    if (bLink)
    {
        pDocShell->UpdateLinks();               // Link-Manager updaten
        SFX_BINDINGS().Invalidate(SID_LINKS);
    }

    EndUndo();      // Draw-Undo muss vor dem Broadcast kommen!

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    for(i=0;i<theTabs.Count();i++)
    {
        pDocShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, theTabs[i]) );
    }
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaint(0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_ALL );  // incl. Extras

    //  nicht ShowTable wegen SetTabNo(..., TRUE):
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( theTabs[0], TRUE );

//  EndUndo();
}

void __EXPORT ScUndoDeleteTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo( theTabs[0] );

    if (pDrawUndo)
        RedoSdrUndoAction( pDrawUndo );         // Draw Redo vorneweg

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    pViewShell->DeleteTables( theTabs, FALSE );
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( TRUE );               //! EndRedo

    SetChangeTrack();

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void __EXPORT ScUndoDeleteTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell* pViewShell = ((ScTabViewTarget&)rTarget).GetViewShell();
        pViewShell->DeleteTable( pViewShell->GetViewData()->GetTabNo(), TRUE );
    }
}

BOOL __EXPORT ScUndoDeleteTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//---------------------------------------------------------------------------------
//
//      Tabelle umbenennen
//

ScUndoRenameTab::ScUndoRenameTab( ScDocShell* pNewDocShell,
                                  USHORT nT,
                                  const String& rOldName,
                                  const String& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    nTab     ( nT )
{
    sOldName = rOldName;
    sNewName = rNewName;
}

__EXPORT ScUndoRenameTab::~ScUndoRenameTab()
{
}

String __EXPORT ScUndoRenameTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_RENAME_TAB );
}

void ScUndoRenameTab::DoChange( USHORT nTab, const String& rName ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->RenameTab( nTab, rName );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();

    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();
}

void __EXPORT ScUndoRenameTab::Undo()
{
    DoChange(nTab, sOldName);
}

void __EXPORT ScUndoRenameTab::Redo()
{
    DoChange(nTab, sNewName);
}

void __EXPORT ScUndoRenameTab::Repeat(SfxRepeatTarget& rTarget)
{
    //  Repeat macht keinen Sinn
}

BOOL __EXPORT ScUndoRenameTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//----------------------------------------------------------------------------------
//
//      Tabelle verschieben
//

ScUndoMoveTab::ScUndoMoveTab( ScDocShell* pNewDocShell,
                                  const SvUShorts &aOldTab,
                                  const SvUShorts &aNewTab) :
    ScSimpleUndo( pNewDocShell )
{
    int i;
    for(i=0;i<aOldTab.Count();i++)
            theOldTabs.Insert(aOldTab[i],theOldTabs.Count());

    for(i=0;i<aNewTab.Count();i++)
            theNewTabs.Insert(aNewTab[i],theNewTabs.Count());
}

__EXPORT ScUndoMoveTab::~ScUndoMoveTab()
{
    theNewTabs.Remove(0,theNewTabs.Count());
    theOldTabs.Remove(0,theOldTabs.Count());
}

String __EXPORT ScUndoMoveTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MOVE_TAB );
}

void ScUndoMoveTab::DoChange( BOOL bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (bUndo)                                      // UnDo
    {
        for(int i=theNewTabs.Count()-1;i>=0;i--)
        {
            USHORT nDestTab = theNewTabs[i];
            USHORT nNewTab = theNewTabs[i];
            USHORT nOldTab = theOldTabs[i];
            if (nDestTab > MAXCOL)                          // angehaengt ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nDestTab, nOldTab );
            pViewShell->GetViewData()->MoveTab( nDestTab, nOldTab );
            pViewShell->SetTabNo( nOldTab, TRUE );
        }
    }
    else
    {
        for(int i=0;i<theNewTabs.Count();i++)
        {
            USHORT nDestTab = theNewTabs[i];
            USHORT nNewTab = theNewTabs[i];
            USHORT nOldTab = theOldTabs[i];
            if (nDestTab > MAXCOL)                          // angehaengt ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nOldTab, nNewTab );
            pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );
            pViewShell->SetTabNo( nDestTab, TRUE );
        }
    }

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void __EXPORT ScUndoMoveTab::Undo()
{
    DoChange( TRUE );
}

void __EXPORT ScUndoMoveTab::Redo()
{
    DoChange( FALSE );
}

void __EXPORT ScUndoMoveTab::Repeat(SfxRepeatTarget& rTarget)
{
        // kein Repeat ! ? !
}

BOOL __EXPORT ScUndoMoveTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//----------------------------------------------------------------------------------
//
//      Tabelle kopieren
//

ScUndoCopyTab::ScUndoCopyTab( ScDocShell* pNewDocShell,
                                  const SvUShorts &aOldTab,
                                  const SvUShorts &aNewTab) :
    ScSimpleUndo( pNewDocShell ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    int i;
    for(i=0;i<aOldTab.Count();i++)
            theOldTabs.Insert(aOldTab[i],theOldTabs.Count());

    for(i=0;i<aNewTab.Count();i++)
            theNewTabs.Insert(aNewTab[i],theNewTabs.Count());
}

__EXPORT ScUndoCopyTab::~ScUndoCopyTab()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoCopyTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_COPY_TAB );
}

void ScUndoCopyTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->SetTabNo(theOldTabs[0],TRUE);

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void __EXPORT ScUndoCopyTab::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );                   // before the sheets are deleted

    int i;
    for(i=theNewTabs.Count()-1;i>=0;i--)
    {
        USHORT nDestTab = theNewTabs[i];
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = TRUE;
        pDoc->DeleteTab(nDestTab);
        bDrawIsInUndo = FALSE;
    }

    //  ScTablesHint broadcasts after all sheets have been deleted,
    //  so sheets and draw pages are in sync!

    for(i=theNewTabs.Count()-1;i>=0;i--)
    {
        USHORT nDestTab = theNewTabs[i];
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = pDoc->GetTableCount() - 1;

        pDocShell->Broadcast( ScTablesHint( SC_TAB_DELETED, nDestTab ) );
    }

    DoChange();
}

void __EXPORT ScUndoCopyTab::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    USHORT nDestTab = 0;
    for(int i=0;i<theNewTabs.Count();i++)
    {
        nDestTab = theNewTabs[i];
        USHORT nNewTab = theNewTabs[i];
        USHORT nOldTab = theOldTabs[i];
        if (nDestTab > MAXCOL)                          // angehaengt ?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = TRUE;
        pDoc->CopyTab( nOldTab, nNewTab );
        bDrawIsInUndo = FALSE;

        pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );

        if ( pDoc->IsScenario(nOldTab) )
        {
            pDoc->SetScenario(nNewTab, TRUE );
            String aComment;
            Color  aColor;
            USHORT nScenFlags;
            pDoc->GetScenarioData(nOldTab, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nNewTab, aComment, aColor, nScenFlags );
            BOOL bActive = pDoc->IsActiveScenario(nOldTab);
            pDoc->SetActiveScenario(nNewTab, bActive );
            BOOL bVisible=pDoc->IsVisible(nOldTab);
            pDoc->SetVisible(nNewTab,bVisible );
        }
    }

    if (pDrawUndo)
        RedoSdrUndoAction( pDrawUndo );         // after the sheets are inserted

    pViewShell->SetTabNo( nDestTab, TRUE );     // after draw-undo

    DoChange();

}

void __EXPORT ScUndoCopyTab::Repeat(SfxRepeatTarget& rTarget)
{
        // kein Repeat ! ? !
}

BOOL __EXPORT ScUndoCopyTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Szenario anlegen
//

ScUndoMakeScenario::ScUndoMakeScenario( ScDocShell* pNewDocShell,
                        USHORT nSrc, USHORT nDest, ScDocument* pUndo,
                        const String& rN, const String& rC,
                        const Color& rCol, USHORT nF,
                        const ScMarkData& rMark ) :
    ScSimpleUndo( pNewDocShell ),
    nSrcTab( nSrc ),
    nDestTab( nDest ),
    pUndoDoc( pUndo ),
    aName( rN ),
    aComment( rC ),
    aColor( rCol ),
    nFlags( nF ),
    aMarkData( rMark )
{
}

__EXPORT ScUndoMakeScenario::~ScUndoMakeScenario()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoMakeScenario::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MAKESCENARIO );
}

void __EXPORT ScUndoMakeScenario::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->DeleteTab( nDestTab );
    pDocShell->PostPaint(0,0,nDestTab,MAXCOL,MAXROW,MAXTAB, PAINT_ALL);
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nSrcTab, TRUE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void __EXPORT ScUndoMakeScenario::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
    }

    pDocShell->MakeScenario( nSrcTab, aName, aComment, aColor, nFlags, aMarkData, FALSE );

    if (pViewShell)
        pViewShell->SetTabNo( nDestTab, TRUE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void __EXPORT ScUndoMakeScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ((ScTabViewTarget&)rTarget).GetViewShell()->MakeScenario( aName, aComment, aColor, nFlags );
    }
}

BOOL __EXPORT ScUndoMakeScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Tabelle einfuegen
//

ScUndoImportTab::ScUndoImportTab( ScDocShell* pShell,
                        USHORT nNewTab, USHORT nNewCount, BOOL bNewLink ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    nCount( nNewCount ),
    bLink( bNewLink ),
    pRedoDoc( NULL ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

__EXPORT ScUndoImportTab::~ScUndoImportTab()
{
    delete pRedoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoImportTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoImportTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nCount = pDoc->GetTableCount();
    if (pViewShell)
    {
        if(nTab<nCount)
        {
            pViewShell->SetTabNo(nTab,TRUE);
        }
        else
        {
            pViewShell->SetTabNo(nTab-1,TRUE);
        }
    }

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator
    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS );
}

void __EXPORT ScUndoImportTab::Undo()
{
    //! eingefuegte Bereichsnamen etc.

    USHORT i;
    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
    {
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab,nTab+nCount-1, TRUE,TRUE );

        String aOldName;
        for (i=0; i<nCount; i++)
        {
            USHORT nTabPos=nTab+i;

            pDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,FALSE, pRedoDoc );
            pDoc->GetName( nTabPos, aOldName );
            pRedoDoc->RenameTab( nTabPos, aOldName, FALSE );

            if ( pDoc->IsScenario(nTabPos) )
            {
                pRedoDoc->SetScenario(nTabPos, TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                pRedoDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                BOOL bActive = pDoc->IsActiveScenario(nTabPos);
                pRedoDoc->SetActiveScenario(nTabPos, bActive );
                BOOL bVisible=pDoc->IsVisible(nTabPos);
                pRedoDoc->SetVisible(nTabPos,bVisible );
            }
        }

    }

    if (pDrawUndo)
        DoSdrUndoAction( pDrawUndo );                   // before the sheets are deleted

    bDrawIsInUndo = TRUE;
    for (i=0; i<nCount; i++)
        pDoc->DeleteTab( nTab );
    bDrawIsInUndo = FALSE;

    DoChange();
}

void __EXPORT ScUndoImportTab::Redo()
{
    if (!pRedoDoc)
    {
        DBG_ERROR("wo ist mein Redo-Document?");
        return;
    }

    ScDocument* pDoc = pDocShell->GetDocument();
    String aName;
    USHORT i;
    for (i=0; i<nCount; i++)                // first insert all sheets (#63304#)
    {
        USHORT nTabPos=nTab+i;
        pRedoDoc->GetName(nTabPos,aName);
        bDrawIsInUndo = TRUE;
        pDoc->InsertTab(nTabPos,aName);
        bDrawIsInUndo = FALSE;
    }
    for (i=0; i<nCount; i++)                // then copy into inserted sheets
    {
        USHORT nTabPos=nTab+i;
        pRedoDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,FALSE, pDoc );

        if ( pRedoDoc->IsScenario(nTabPos) )
        {
            pDoc->SetScenario(nTabPos, TRUE );
            String aComment;
            Color  aColor;
            USHORT nScenFlags;
            pRedoDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            BOOL bActive = pRedoDoc->IsActiveScenario(nTabPos);
            pDoc->SetActiveScenario(nTabPos, bActive );
            BOOL bVisible=pRedoDoc->IsVisible(nTabPos);
            pDoc->SetVisible(nTabPos,bVisible );
        }
    }

    if (pDrawUndo)
        RedoSdrUndoAction( pDrawUndo );     // after the sheets are inserted

    DoChange();
}

void __EXPORT ScUndoImportTab::Repeat(SfxRepeatTarget& rTarget)
{
    SFX_DISPATCHER().Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL __EXPORT ScUndoImportTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Tabellen-Verknuepfung aufheben
//

ScUndoRemoveLink::ScUndoRemoveLink( ScDocShell* pShell, const String& rDoc ) :
    ScSimpleUndo( pShell ),
    aDocName( rDoc ),
    nCount( 0 )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    pTabs     = new USHORT[nTabCount];
    pModes    = new BYTE[nTabCount];
    pTabNames = new String[nTabCount];

    for (USHORT i=0; i<nTabCount; i++)
    {
        BYTE nMode = pDoc->GetLinkMode(i);
        if (nMode)
            if (pDoc->GetLinkDoc(i) == aDocName)
            {
                if (!nCount)
                {
                    aFltName = pDoc->GetLinkFlt(i);
                    aOptions = pDoc->GetLinkOpt(i);
                }
                else
                    DBG_ASSERT(aFltName == pDoc->GetLinkFlt(i) &&
                               aOptions == pDoc->GetLinkOpt(i),
                                    "verschiedene Filter fuer ein Dokument?");
                pTabs[nCount] = i;
                pModes[nCount] = nMode;
                pTabNames[nCount] = pDoc->GetLinkTab(i);
                ++nCount;
            }
    }
}

__EXPORT ScUndoRemoveLink::~ScUndoRemoveLink()
{
    delete pTabs;
    delete pModes;
    delete[] pTabNames;
}

String __EXPORT ScUndoRemoveLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVELINK );
}

void ScUndoRemoveLink::DoChange( BOOL bLink ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    String aEmpty;
    for (USHORT i=0; i<nCount; i++)
        if (bLink)      // establish link
            pDoc->SetLink( pTabs[i], pModes[i], aDocName, aFltName, aOptions, pTabNames[i] );
        else            // remove link
            pDoc->SetLink( pTabs[i], SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty );
    pDocShell->UpdateLinks();
    SFX_BINDINGS().Invalidate(SID_LINKS);
}

void __EXPORT ScUndoRemoveLink::Undo()
{
    DoChange( TRUE );
}

void __EXPORT ScUndoRemoveLink::Redo()
{
    DoChange( FALSE );
}

void __EXPORT ScUndoRemoveLink::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}

BOOL __EXPORT ScUndoRemoveLink::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Tabellen ein-/ausblenden
//

ScUndoShowHideTab::ScUndoShowHideTab( ScDocShell* pShell, USHORT nNewTab, BOOL bNewShow ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bShow( bNewShow )
{
}

__EXPORT ScUndoShowHideTab::~ScUndoShowHideTab()
{
}

void ScUndoShowHideTab::DoChange( BOOL bShow ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetVisible( nTab, bShow );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,TRUE);

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pDocShell->SetDocumentModified();
}

void __EXPORT ScUndoShowHideTab::Undo()
{
    DoChange(!bShow);
}

void __EXPORT ScUndoShowHideTab::Redo()
{
    DoChange(bShow);
}

void __EXPORT ScUndoShowHideTab::Repeat(SfxRepeatTarget& rTarget)
{
    SFX_DISPATCHER().Execute( bShow ? FID_TABLE_SHOW : FID_TABLE_HIDE,
                                SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL __EXPORT ScUndoShowHideTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

String __EXPORT ScUndoShowHideTab::GetComment() const
{
    USHORT nId = bShow ? STR_UNDO_SHOWTAB : STR_UNDO_HIDETAB;
    return ScGlobal::GetRscString( nId );
}

// -----------------------------------------------------------------------
//
//      Tabelle/Dokument schuetzen oder Schutz aufheben
//

ScUndoProtect::ScUndoProtect( ScDocShell* pShell, USHORT nNewTab,
                            BOOL bNewProtect, const String& rNewPassword ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bProtect( bNewProtect ),
    aPassword( rNewPassword )
{
}

__EXPORT ScUndoProtect::~ScUndoProtect()
{
}

void ScUndoProtect::DoProtect( BOOL bDo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (bDo)
    {
        if ( nTab == TABLEID_DOC )
            pDoc->SetDocProtection( TRUE, aPassword );
        else
            pDoc->SetTabProtection( nTab, TRUE, aPassword );
    }
    else
    {
        if ( nTab == TABLEID_DOC )
            pDoc->SetDocProtection( FALSE, EMPTY_STRING );
        else
            pDoc->SetTabProtection( nTab, FALSE, EMPTY_STRING );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateLayerLocks();
        pViewShell->UpdateInputHandler(TRUE);   // damit sofort wieder eingegeben werden kann
    }

    pDocShell->PostPaintGridAll();
}

void __EXPORT ScUndoProtect::Undo()
{
    BeginUndo();
    DoProtect( !bProtect );
    EndUndo();
}

void __EXPORT ScUndoProtect::Redo()
{
    BeginRedo();
    DoProtect( bProtect );
    EndRedo();
}

void __EXPORT ScUndoProtect::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}

BOOL __EXPORT ScUndoProtect::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;       // gippsnich
}

String __EXPORT ScUndoProtect::GetComment() const
{
    USHORT nId;
    if ( nTab == TABLEID_DOC )
        nId = bProtect ? STR_UNDO_PROTECT_DOC : STR_UNDO_UNPROTECT_DOC;
    else
        nId = bProtect ? STR_UNDO_PROTECT_TAB : STR_UNDO_UNPROTECT_TAB;
    return ScGlobal::GetRscString( nId );
}

// -----------------------------------------------------------------------
//
//      Druck-/Wiederholungsbereiche aendern
//

ScUndoPrintRange::ScUndoPrintRange( ScDocShell* pShell, USHORT nNewTab,
                                    ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    pOldRanges( pOld ),
    pNewRanges( pNew )
{
}

__EXPORT ScUndoPrintRange::~ScUndoPrintRange()
{
    delete pOldRanges;
    delete pNewRanges;
}

void ScUndoPrintRange::DoChange(BOOL bUndo)
{
    ScDocument* pDoc = pDocShell->GetDocument();
    if (bUndo)
        pDoc->RestorePrintRanges( *pOldRanges );
    else
        pDoc->RestorePrintRanges( *pNewRanges );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nTab );

    ScPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab ).UpdatePages();

    pDocShell->PostPaint( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab), PAINT_GRID );
}

void __EXPORT ScUndoPrintRange::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void __EXPORT ScUndoPrintRange::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoPrintRange::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}

BOOL __EXPORT ScUndoPrintRange::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;       // gippsnich
}

String __EXPORT ScUndoPrintRange::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTRANGES );
}


//------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//
//      Szenario-Flags
//

ScUndoScenarioFlags::ScUndoScenarioFlags( ScDocShell* pNewDocShell, USHORT nT,
                    const String& rON, const String& rNN, const String& rOC, const String& rNC,
                    const Color& rOCol, const Color& rNCol, USHORT nOF, USHORT nNF ) :
    ScSimpleUndo( pNewDocShell ),
    nTab        ( nT ),
    aOldName    ( rON ),
    aNewName    ( rNN ),
    aOldComment ( rOC ),
    aNewComment ( rNC ),
    aOldColor   ( rOCol ),
    aNewColor   ( rNCol ),
    nOldFlags   ( nOF ),
    nNewFlags   ( nNF )
{
}

__EXPORT ScUndoScenarioFlags::~ScUndoScenarioFlags()
{
}

String __EXPORT ScUndoScenarioFlags::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_EDITSCENARIO );
}

void __EXPORT ScUndoScenarioFlags::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->RenameTab( nTab, aOldName );
    pDoc->SetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );

    pDocShell->PostPaintGridAll();
    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void __EXPORT ScUndoScenarioFlags::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->RenameTab( nTab, aNewName );
    pDoc->SetScenarioData( nTab, aNewComment, aNewColor, nNewFlags );

    pDocShell->PostPaintGridAll();
    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void __EXPORT ScUndoScenarioFlags::Repeat(SfxRepeatTarget& rTarget)
{
    //  Repeat macht keinen Sinn
}

BOOL __EXPORT ScUndoScenarioFlags::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}




