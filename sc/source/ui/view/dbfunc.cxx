/*************************************************************************
 *
 *  $RCSfile: dbfunc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-10 19:07:03 $
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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>

#include "dbfunc.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "sc.hrc"
#include "undodat.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"

//==================================================================

ScDBFunc::ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScViewFunc( pParent, rDocSh, pViewShell )
{
}

ScDBFunc::ScDBFunc( Window* pParent, const ScDBFunc& rDBFunc, ScTabViewShell* pViewShell ) :
    ScViewFunc( pParent, rDBFunc, pViewShell )
{
}

ScDBFunc::~ScDBFunc()
{
}

//
//      Hilfsfunktionen
//

void ScDBFunc::GotoDBArea( const String& rDBName )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDBCollection* pDBCol = pDoc->GetDBCollection();

    USHORT nFoundAt = 0;
    if ( pDBCol->SearchName( rDBName, nFoundAt ) )
    {
        ScDBData* pData = (*pDBCol)[nFoundAt];
        DBG_ASSERT( pData, "GotoDBArea: Datenbankbereich nicht gefunden!" );

        if ( pData )
        {
            USHORT nTab = 0;
            USHORT nStartCol = 0;
            USHORT nStartRow = 0;
            USHORT nEndCol = 0;
            USHORT nEndRow = 0;

            pData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
            SetTabNo( nTab );

            MoveCursorAbs( nStartCol, nStartRow, ScFollowMode( SC_FOLLOW_JUMP ),
                               FALSE, FALSE );  // bShift,bControl
            DoneBlockMode();
            InitBlockMode( nStartCol, nStartRow, nTab );
            MarkCursor( nEndCol, nEndRow, nTab );
            SelectionChanged();
        }
    }
}

//  aktuellen Datenbereich fuer Sortieren / Filtern suchen

ScDBData* ScDBFunc::GetDBData( BOOL bMark, ScGetDBMode eMode )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDBData* pData = NULL;
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea(aRange) )
        pData = pDocSh->GetDBData( aRange, eMode, FALSE );
    else if ( eMode != SC_DB_OLD )
        pData = pDocSh->GetDBData(
                    ScRange( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                             GetViewData()->GetTabNo() ),
                    eMode, FALSE );

    if ( pData && bMark )
    {
        ScRange aFound;
        pData->GetArea(aFound);
        MarkRange( aFound, FALSE );
    }
    return pData;
}

//  Datenbankbereiche aendern (Dialog)

void ScDBFunc::NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const List& rDelAreaList )
{
    const BOOL bRecord = TRUE;

    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocShell );
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBCollection* pOldColl = pDoc->GetDBCollection();
    ScDBCollection* pUndoColl = NULL;
    ScDBCollection* pRedoColl = NULL;

    long nDelCount = rDelAreaList.Count();
    for (long nDelPos=0; nDelPos<nDelCount; nDelPos++)
    {
        ScRange* pEntry = (ScRange*) rDelAreaList.GetObject(nDelPos);

        if ( pEntry )
        {
            ScAddress& rStart = pEntry->aStart;
            ScAddress& rEnd   = pEntry->aEnd;
            pDocShell->DBAreaDeleted( rStart.Tab(),
                                       rStart.Col(), rStart.Row(),
                                       rEnd.Col(),   rEnd.Row() );

            //  Targets am SBA abmelden nicht mehr noetig
        }
    }

    if (bRecord)
        pUndoColl = new ScDBCollection( *pOldColl );

    //  neue Targets am SBA anmelden nicht mehr noetig

    pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection( rNewColl ) );
    pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
    pOldColl = NULL;
    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    if (bRecord)
    {
        pRedoColl = new ScDBCollection( rNewColl );
        pDocShell->GetUndoManager()->AddUndoAction(
            new ScUndoDBData( pDocShell, pUndoColl, pRedoColl ) );
    }
}

//
//      wirkliche Funktionen
//

// Sortieren

void ScDBFunc::UISort( const ScSortParam& rSortParam, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        DBG_ERROR( "Sort: keine DBData" );
        return;
    }

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    if (aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly)
    {
        //  Subtotals wiederholen, mit neuer Sortierung

        ScSubTotalParam aSubTotalParam;
        pDBData->GetSubTotalParam( aSubTotalParam );
        DoSubTotals( aSubTotalParam, bRecord, &rSortParam );
    }
    else
    {
        Sort( rSortParam, bRecord );        // nur sortieren
    }
}

void ScDBFunc::Sort( const ScSortParam& rSortParam, BOOL bRecord, BOOL bPaint )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    USHORT nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    BOOL bSuccess = aDBDocFunc.Sort( nTab, rSortParam, bRecord, bPaint, FALSE );
    if ( bSuccess && !rSortParam.bInplace )
    {
        //  Ziel markieren
        ScRange aDestRange( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab,
                            rSortParam.nDestCol + rSortParam.nCol2 - rSortParam.nCol1,
                            rSortParam.nDestRow + rSortParam.nRow2 - rSortParam.nRow1,
                            rSortParam.nDestTab );
        MarkRange( aDestRange );
    }
}

//  Filtern

void ScDBFunc::Query( const ScQueryParam& rQueryParam, const ScRange* pAdvSource, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    USHORT nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    BOOL bSuccess = aDBDocFunc.Query( nTab, rQueryParam, pAdvSource, bRecord, FALSE );

    if (bSuccess)
    {
        BOOL bCopy = !rQueryParam.bInplace;
        if (bCopy)
        {
            //  Zielbereich markieren (DB-Bereich wurde ggf. angelegt)
            ScDocument* pDoc = pDocSh->GetDocument();
            ScDBData* pDestData = pDoc->GetDBAtCursor(
                                            rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, TRUE );
            if (pDestData)
            {
                ScRange aDestRange;
                pDestData->GetArea(aDestRange);
                MarkRange( aDestRange );
            }
        }

        if (!bCopy)
            UpdateScrollBars();

        GetViewData()->GetBindings().Invalidate( SID_UNFILTER );
    }
}

//  Autofilter-Knoepfe ein-/ausblenden

void ScDBFunc::ToggleAutoFilter()
{
    ScDocShellModificator aModificator( *(GetViewData()->GetDocShell()) );

    ScQueryParam    aParam;
    ScDocument*     pDoc    = GetViewData()->GetDocument();
    ScDBData*       pDBData = GetDBData( FALSE );

    pDBData->SetByRow( TRUE );              //! Undo, vorher abfragen ??
    pDBData->GetQueryParam( aParam );


    USHORT  nCol;
    USHORT  nRow = aParam.nRow1;
    USHORT  nTab = GetViewData()->GetTabNo();
    INT16   nFlag;
    BOOL    bHasAuto = TRUE;
    BOOL    bHeader  = pDBData->HasHeader();
    BOOL    bPaint   = FALSE;

    //!     stattdessen aus DB-Bereich abfragen?

    for (nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAuto; nCol++)
    {
        nFlag = ((ScMergeFlagAttr*) pDoc->
                GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();

        if ( (nFlag & SC_MF_AUTO) == 0 )
            bHasAuto = FALSE;
    }

    if (bHasAuto)                               // aufheben
    {
        //  Filterknoepfe ausblenden

        for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
        {
            nFlag = ((ScMergeFlagAttr*) pDoc->
                    GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
        }

        pDBData->SetAutoFilter(FALSE);

        //  Filter aufheben (incl. Paint / Undo)

        USHORT nEC = aParam.GetEntryCount();
        for (USHORT i=0; i<nEC; i++)
            aParam.GetEntry(i).bDoQuery = FALSE;
        aParam.bDuplicate = TRUE;
        Query( aParam, NULL, TRUE );

        bPaint = TRUE;
    }
    else                                    // Filterknoepfe einblenden
    {
        if ( !pDoc->IsBlockEmpty( nTab,
                                  aParam.nCol1, aParam.nRow1,
                                  aParam.nCol2, aParam.nRow2 ) )
        {
            if (!bHeader)
            {
                if ( MessBox( GetViewData()->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       // "StarCalc"
                        ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 )     // Koepfe aus erster Zeile?
                    ).Execute() == RET_YES )
                {
                    pDBData->SetHeader( TRUE );     //! Undo ??
                    bHeader = TRUE;
                }
            }

            pDBData->SetAutoFilter(TRUE);

            for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
            {
                nFlag = ((ScMergeFlagAttr*) pDoc->
                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
                pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag | SC_MF_AUTO ) );
            }
            GetViewData()->GetDocShell()->PostPaint( aParam.nCol1, nRow, nTab,
                                                     aParam.nCol2, nRow, nTab,
                                                     PAINT_GRID );
            bPaint = TRUE;
        }
        else
        {
            ErrorBox aErrorBox( GetViewData()->GetDialogParent(), WinBits( WB_OK | WB_DEF_OK ),
                                ScGlobal::GetRscString( STR_ERR_AUTOFILTER ) );
            aErrorBox.Execute();
        }
    }

    if ( bPaint )
    {
        aModificator.SetDocumentModified();

        SfxBindings& rBindings = GetViewData()->GetBindings();
        rBindings.Invalidate( SID_AUTO_FILTER );
        rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    }
}

//      nur ausblenden, keine Daten veraendern

void ScDBFunc::HideAutoFilter()
{
    ScDocument* pDoc = GetViewData()->GetDocument();

    ScQueryParam aParam;
    ScDBData* pDBData = GetDBData( FALSE );

    USHORT nTab, nCol1, nRow1, nCol2, nRow2;
    pDBData->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);

    for (USHORT nCol=nCol1; nCol<=nCol2; nCol++)
    {
        INT16 nFlag = ((ScMergeFlagAttr*) pDoc->
                                GetAttr( nCol, nRow1, nTab, ATTR_MERGE_FLAG ))->GetValue();
        pDoc->ApplyAttr( nCol, nRow1, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
    }

    pDBData->SetAutoFilter(FALSE);

    GetViewData()->GetDocShell()->PostPaint( nCol1,nRow1,nTab, nCol2,nRow1,nTab, PAINT_GRID );

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_AUTO_FILTER );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
}

//      Re-Import

BOOL ScDBFunc::ImportData( const ScImportParam& rParam, BOOL bRecord )
{
    BOOL bSuccess = FALSE;

    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( !pDoc->IsBlockEditable( GetViewData()->GetTabNo(), rParam.nCol1,rParam.nRow1,
                                                            rParam.nCol2,rParam.nRow2 ) )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScDBDocFunc aDBDocFunc( *GetViewData()->GetDocShell() );
    return aDBDocFunc.DoImport( GetViewData()->GetTabNo(), rParam, NULL, bRecord );
}



