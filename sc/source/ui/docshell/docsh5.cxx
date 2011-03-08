/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------




#include "scitems.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <svl/smplhint.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

// INCLUDE ---------------------------------------------------------------

#include "docsh.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "undodat.hxx"
#include "undotab.hxx"
#include "undoblk.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "consoli.hxx"
#include "dbcolect.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "docpool.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "waitoff.hxx"
#include "sizedev.hxx"
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>

#include <memory>
#include <vector>

// defined in docfunc.cxx
void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, String& sModuleName, String& sModuleSource );

using com::sun::star::script::XLibraryContainer;
using com::sun::star::script::vba::XVBACompatibility;
using com::sun::star::container::XNameContainer;
using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

using ::std::auto_ptr;
using ::std::vector;

// ---------------------------------------------------------------------------

//
//  former viewfunc/dbfunc methods
//

void ScDocShell::ErrorMessage( USHORT nGlobStrId )
{
    //! StopMarking an der (aktiven) View?

    Window* pParent = GetActiveDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    BOOL bFocus = pParent && pParent->HasFocus();

    if(nGlobStrId==STR_PROTECTIONERR)
    {
        if(IsReadOnly())
        {
            nGlobStrId=STR_READONLYERR;
        }
    }

    InfoBox aBox( pParent, ScGlobal::GetRscString( nGlobStrId ) );
    aBox.Execute();
    if (bFocus)
        pParent->GrabFocus();
}

BOOL ScDocShell::IsEditable() const
{
    // import into read-only document is possible - must be extended if other filters use api
    // #i108547# MSOOXML filter uses "IsChangeReadOnlyEnabled" property

    return !IsReadOnly() || aDocument.IsImportingXML() || aDocument.IsChangeReadOnlyEnabled();
}

void ScDocShell::DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW /* nY2 */ )
{
    ScDocShellModificator aModificator( *this );
    aDocument.RemoveFlagsTab( nX1, nY1, nX2, nY1, nTab, SC_MF_AUTO );
    PostPaint( nX1, nY1, nTab, nX2, nY1, nTab, PAINT_GRID );
    // No SetDocumentModified, as the unnamed database range might have to be restored later.
    // The UNO hint is broadcast directly instead, to keep UNO objects in valid state.
    aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
}

ScDBData* lcl_GetDBNearCursor( ScDBCollection* pColl, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    //! nach document/dbcolect verschieben

    if (!pColl)
        return NULL;

    ScDBData* pNoNameData = NULL;
    ScDBData* pNearData = NULL;
    USHORT nCount = pColl->GetCount();
    String aNoName = ScGlobal::GetRscString( STR_DB_NONAME );
    SCTAB nAreaTab;
    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    for (USHORT i = 0; i < nCount; i++)
    {
        ScDBData* pDB = (*pColl)[i];
        pDB->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab && nCol+1 >= nStartCol && nCol <= nEndCol+1 &&
                                 nRow+1 >= nStartRow && nRow <= nEndRow+1 )
        {
            if ( pDB->GetName() == aNoName )
                pNoNameData = pDB;
            else if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if (!pNearData)
                    pNearData = pDB;    // ersten angrenzenden Bereich merken
            }
            else
                return pDB;             // nicht "unbenannt" und Cursor steht wirklich drin
        }
    }
    if (pNearData)
        return pNearData;               // angrenzender, wenn nichts direkt getroffen
    return pNoNameData;                 // "unbenannt" nur zurueck, wenn sonst nichts gefunden
}

ScDBData* ScDocShell::GetDBData( const ScRange& rMarked, ScGetDBMode eMode, ScGetDBSelection eSel )
{
    SCCOL nCol = rMarked.aStart.Col();
    SCROW nRow = rMarked.aStart.Row();
    SCTAB nTab = rMarked.aStart.Tab();

    SCCOL nStartCol = nCol;
    SCROW nStartRow = nRow;
    SCTAB nStartTab = nTab;
    SCCOL nEndCol = rMarked.aEnd.Col();
    SCROW nEndRow = rMarked.aEnd.Row();
    SCTAB nEndTab = rMarked.aEnd.Tab();

    //  Nicht einfach GetDBAtCursor: Der zusammenhaengende Datenbereich
    //  fuer "unbenannt" (GetDataArea) kann neben dem Cursor legen, also muss auch ein
    //  benannter DB-Bereich dort gesucht werden.

    ScDBData* pData = aDocument.GetDBAtArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
    if (!pData)
        pData = lcl_GetDBNearCursor( aDocument.GetDBCollection(), nCol, nRow, nTab );

    BOOL bSelected = ( eSel == SC_DBSEL_FORCE_MARK ||
            (rMarked.aStart != rMarked.aEnd && eSel != SC_DBSEL_ROW_DOWN) );
    bool bOnlyDown = (!bSelected && eSel == SC_DBSEL_ROW_DOWN && rMarked.aStart.Row() == rMarked.aEnd.Row());

    BOOL bUseThis = FALSE;
    if (pData)
    {
        //      Bereich nehmen, wenn nichts anderes markiert

        SCTAB nDummy;
        SCCOL nOldCol1;
        SCROW nOldRow1;
        SCCOL nOldCol2;
        SCROW nOldRow2;
        pData->GetArea( nDummy, nOldCol1,nOldRow1, nOldCol2,nOldRow2 );
        BOOL bIsNoName = ( pData->GetName() == ScGlobal::GetRscString( STR_DB_NONAME ) );

        if (!bSelected)
        {
            bUseThis = TRUE;
            if ( bIsNoName && eMode == SC_DB_MAKE )
            {
                // If nothing marked or only one row marked, adapt
                // "unbenannt"/"unnamed" to contiguous area.
                nStartCol = nCol;
                nStartRow = nRow;
                if (bOnlyDown)
                {
                    nEndCol = rMarked.aEnd.Col();
                    nEndRow = rMarked.aEnd.Row();
                }
                else
                {
                    nEndCol = nStartCol;
                    nEndRow = nStartRow;
                }
                aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE, bOnlyDown );
                if ( nOldCol1 != nStartCol || nOldCol2 != nEndCol || nOldRow1 != nStartRow )
                    bUseThis = FALSE;               // passt gar nicht
                else if ( nOldRow2 != nEndRow )
                {
                    //  Bereich auf neue End-Zeile erweitern
                    pData->SetArea( nTab, nOldCol1,nOldRow1, nOldCol2,nEndRow );
                }
            }
        }
        else
        {
            if ( nOldCol1 == nStartCol && nOldRow1 == nStartRow &&
                 nOldCol2 == nEndCol && nOldRow2 == nEndRow )               // genau markiert?
                bUseThis = TRUE;
            else
                bUseThis = FALSE;           // immer Markierung nehmen (Bug 11964)
        }

        //      fuer Import nie "unbenannt" nehmen

        if ( bUseThis && eMode == SC_DB_IMPORT && bIsNoName )
            bUseThis = FALSE;
    }

    if ( bUseThis )
    {
        pData->GetArea( nStartTab, nStartCol,nStartRow, nEndCol,nEndRow );
        nEndTab = nStartTab;
    }
    else if ( eMode == SC_DB_OLD )
    {
        pData = NULL;                           // nichts gefunden
        nStartCol = nEndCol = nCol;
        nStartRow = nEndRow = nRow;
        nStartTab = nEndTab = nTab;
    }
    else
    {
        if ( !bSelected )
        {                                       // zusammenhaengender Bereich
            nStartCol = nCol;
            nStartRow = nRow;
            if (bOnlyDown)
            {
                nEndCol = rMarked.aEnd.Col();
                nEndRow = rMarked.aEnd.Row();
            }
            else
            {
                nEndCol = nStartCol;
                nEndRow = nStartRow;
            }
            aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE, bOnlyDown );
        }

        BOOL bHasHeader = aDocument.HasColHeader( nStartCol,nStartRow, nEndCol,nEndRow, nTab );

        ScDBData* pNoNameData;
        USHORT nNoNameIndex;
        ScDBCollection* pColl = aDocument.GetDBCollection();
        if ( eMode != SC_DB_IMPORT &&
                pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            pNoNameData = (*pColl)[nNoNameIndex];

            if ( !pOldAutoDBRange )
            {
                // store the old unnamed database range with its settings for undo
                // (store at the first change, get the state before all changes)
                pOldAutoDBRange = new ScDBData( *pNoNameData );
            }

            SCCOL nOldX1;                                   // alten Bereich sauber wegnehmen
            SCROW nOldY1;                                   //! (UNDO ???)
            SCCOL nOldX2;
            SCROW nOldY2;
            SCTAB nOldTab;
            pNoNameData->GetArea( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );
            DBAreaDeleted( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );

            pNoNameData->SetSortParam( ScSortParam() );             // Parameter zuruecksetzen
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( nTab, nStartCol,nStartRow, nEndCol,nEndRow );     // neu setzen
            pNoNameData->SetByRow( TRUE );
            pNoNameData->SetHeader( bHasHeader );
            pNoNameData->SetAutoFilter( FALSE );
        }
        else
        {
            ScDBCollection* pUndoColl = NULL;

            String aNewName;
            if (eMode==SC_DB_IMPORT)
            {
                aDocument.CompileDBFormula( TRUE );         // CreateFormulaString
                pUndoColl = new ScDBCollection( *pColl );   // Undo fuer Import1-Bereich

                String aImport = ScGlobal::GetRscString( STR_DBNAME_IMPORT );
                long nCount = 0;
                USHORT nDummy;
                do
                {
                    ++nCount;
                    aNewName = aImport;
                    aNewName += String::CreateFromInt32( nCount );
                }
                while (pColl->SearchName( aNewName, nDummy ));
            }
            else
                aNewName = ScGlobal::GetRscString( STR_DB_NONAME );
            pNoNameData = new ScDBData( aNewName, nTab,
                                nStartCol,nStartRow, nEndCol,nEndRow,
                                TRUE, bHasHeader );
            pColl->Insert( pNoNameData );

            if ( pUndoColl )
            {
                aDocument.CompileDBFormula( FALSE );        // CompileFormulaString

                ScDBCollection* pRedoColl = new ScDBCollection( *pColl );
                GetUndoManager()->AddUndoAction( new ScUndoDBData( this, pUndoColl, pRedoColl ) );
            }

            //  neuen Bereich am Sba anmelden nicht mehr noetig

            //  "Import1" etc am Navigator bekanntmachen
            if (eMode==SC_DB_IMPORT)
                SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        }
        pData = pNoNameData;
    }

    return pData;
}

ScDBData* ScDocShell::GetOldAutoDBRange()
{
    ScDBData* pRet = pOldAutoDBRange;
    pOldAutoDBRange = NULL;
    return pRet;                    // has to be deleted by caller!
}

void ScDocShell::CancelAutoDBRange()
{
    // called when dialog is cancelled
    if ( pOldAutoDBRange )
    {
        USHORT nNoNameIndex;
        ScDBCollection* pColl = aDocument.GetDBCollection();
        if ( pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            ScDBData* pNoNameData = (*pColl)[nNoNameIndex];

            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

            *pNoNameData = *pOldAutoDBRange;    // restore old settings

            if ( pOldAutoDBRange->HasAutoFilter() )
            {
                // restore AutoFilter buttons
                pOldAutoDBRange->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
                aDocument.ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, SC_MF_AUTO );
                PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PAINT_GRID );
            }
        }

        delete pOldAutoDBRange;
        pOldAutoDBRange = NULL;
    }
}


        //  Hoehen anpassen
        //! mit docfunc zusammenfassen

BOOL ScDocShell::AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab )
{
    ScSizeDeviceProvider aProv(this);
    Fraction aZoom(1,1);
    BOOL bChange = aDocument.SetOptimalHeight( nStartRow,nEndRow, nTab, 0, aProv.GetDevice(),
                                                aProv.GetPPTX(),aProv.GetPPTY(), aZoom,aZoom, FALSE );
    if (bChange)
        PostPaint( 0,nStartRow,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID|PAINT_LEFT );

    return bChange;
}

void ScDocShell::UpdateAllRowHeights( const ScMarkData* pTabMark )
{
    // update automatic row heights

    ScSizeDeviceProvider aProv(this);
    Fraction aZoom(1,1);
    aDocument.UpdateAllRowHeights( aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), aZoom, aZoom, pTabMark );
}

void ScDocShell::UpdatePendingRowHeights( SCTAB nUpdateTab, bool bBefore )
{
    BOOL bIsUndoEnabled = aDocument.IsUndoEnabled();
    aDocument.EnableUndo( FALSE );
    aDocument.LockStreamValid( true );      // ignore draw page size (but not formula results)
    if ( bBefore )          // check all sheets up to nUpdateTab
    {
        SCTAB nTabCount = aDocument.GetTableCount();
        if ( nUpdateTab >= nTabCount )
            nUpdateTab = nTabCount-1;     // nUpdateTab is inclusive

        ScMarkData aUpdateSheets;
        SCTAB nTab;
        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( aDocument.IsPendingRowHeights( nTab ) )
                aUpdateSheets.SelectTable( nTab, TRUE );

        if (aUpdateSheets.GetSelectCount())
            UpdateAllRowHeights(&aUpdateSheets);        // update with a single progress bar

        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( aUpdateSheets.GetTableSelect( nTab ) )
            {
                aDocument.UpdatePageBreaks( nTab );
                aDocument.SetPendingRowHeights( nTab, FALSE );
            }
    }
    else                    // only nUpdateTab
    {
        if ( aDocument.IsPendingRowHeights( nUpdateTab ) )
        {
            AdjustRowHeight( 0, MAXROW, nUpdateTab );
            aDocument.UpdatePageBreaks( nUpdateTab );
            aDocument.SetPendingRowHeights( nUpdateTab, FALSE );
        }
    }
    aDocument.LockStreamValid( false );
    aDocument.EnableUndo( bIsUndoEnabled );
}

void ScDocShell::RefreshPivotTables( const ScRange& rSource )
{
    //! rename to RefreshDataPilotTables?

    ScDPCollection* pColl = aDocument.GetDPCollection();
    if ( pColl )
    {
        //  DataPilotUpdate doesn't modify the collection order like PivotUpdate did,
        //  so a simple loop can be used.

        USHORT nCount = pColl->GetCount();
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDPObject* pOld = (*pColl)[i];
            if ( pOld )
            {
                const ScSheetSourceDesc* pSheetDesc = pOld->GetSheetDesc();
                if ( pSheetDesc && pSheetDesc->GetSourceRange().Intersects( rSource ) )
                {
                    ScDPObject* pNew = new ScDPObject( *pOld );
                    ScDBDocFunc aFunc( *this );
                    aFunc.DataPilotUpdate( pOld, pNew, TRUE, FALSE );
                    delete pNew;    // DataPilotUpdate copies settings from "new" object
                }
            }
        }
    }
}

String lcl_GetAreaName( ScDocument* pDoc, ScArea* pArea )
{
    String aName;
    BOOL bOk = FALSE;
    ScDBData* pData = pDoc->GetDBAtArea( pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                        pArea->nColEnd, pArea->nRowEnd );
    if (pData)
    {
        pData->GetName( aName );
        if ( aName != ScGlobal::GetRscString( STR_DB_NONAME ) )
            bOk = TRUE;
    }

    if (!bOk)
        pDoc->GetName( pArea->nTab, aName );

    return aName;
}

void ScDocShell::DoConsolidate( const ScConsolidateParam& rParam, BOOL bRecord )
{
    ScConsData aData;

    USHORT nPos;
    SCCOL nColSize = 0;
    SCROW nRowSize = 0;
    BOOL bErr = FALSE;
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea* pArea = rParam.ppDataAreas[nPos];
        nColSize = Max( nColSize, SCCOL( pArea->nColEnd - pArea->nColStart + 1 ) );
        nRowSize = Max( nRowSize, SCROW( pArea->nRowEnd - pArea->nRowStart + 1 ) );

                                        // Test, ob Quelldaten verschoben wuerden
        if (rParam.bReferenceData)
            if (pArea->nTab == rParam.nTab && pArea->nRowEnd >= rParam.nRow)
                bErr = TRUE;
    }

    if (bErr)
    {
        InfoBox aBox( GetActiveDialogParent(),
                ScGlobal::GetRscString( STR_CONSOLIDATE_ERR1 ) );
        aBox.Execute();
        return;
    }

    //      ausfuehren

    WaitObject aWait( GetActiveDialogParent() );
    ScDocShellModificator aModificator( *this );

    ScRange aOldDest;
    ScDBData* pDestData = aDocument.GetDBAtCursor( rParam.nCol, rParam.nRow, rParam.nTab, TRUE );
    if (pDestData)
        pDestData->GetArea(aOldDest);

    aData.SetSize( nColSize, nRowSize );
    aData.SetFlags( rParam.eFunction, rParam.bByCol, rParam.bByRow, rParam.bReferenceData );
    if ( rParam.bByCol || rParam.bByRow )
        for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
        {
            ScArea* pArea = rParam.ppDataAreas[nPos];
            aData.AddFields( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                        pArea->nColEnd, pArea->nRowEnd );
        }
    aData.DoneFields();
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea* pArea = rParam.ppDataAreas[nPos];
        aData.AddData( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                    pArea->nColEnd, pArea->nRowEnd );
        aData.AddName( lcl_GetAreaName(&aDocument,pArea) );
    }

    aData.GetSize( nColSize, nRowSize );
    if (bRecord && nColSize > 0 && nRowSize > 0)
    {
        ScDBData* pUndoData = pDestData ? new ScDBData(*pDestData) : NULL;

        SCTAB nDestTab = rParam.nTab;
        ScArea aDestArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol+nColSize-1, rParam.nRow+nRowSize-1 );
        if (rParam.bByCol) ++aDestArea.nColEnd;
        if (rParam.bByRow) ++aDestArea.nRowEnd;

        if (rParam.bReferenceData)
        {
            SCTAB nTabCount = aDocument.GetTableCount();
            SCROW nInsertCount = aData.GetInsertCount();

            // alte Outlines
            ScOutlineTable* pTable = aDocument.GetOutlineTable( nDestTab );
            ScOutlineTable* pUndoTab = pTable ? new ScOutlineTable( *pTable ) : NULL;

            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( &aDocument, 0, nTabCount-1, FALSE, TRUE );

            // Zeilenstatus
            aDocument.CopyToDocument( 0,0,nDestTab, MAXCOL,MAXROW,nDestTab,
                                    IDF_NONE, FALSE, pUndoDoc );

            // alle Formeln
            aDocument.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        IDF_FORMULA, FALSE, pUndoDoc );

            // komplette Ausgangszeilen
            aDocument.CopyToDocument( 0,aDestArea.nRowStart,nDestTab,
                                    MAXCOL,aDestArea.nRowEnd,nDestTab,
                                    IDF_ALL, FALSE, pUndoDoc );

            // alten Ausgabebereich
            if (pDestData)
                aDocument.CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );

            GetUndoManager()->AddUndoAction(
                    new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
                                            TRUE, nInsertCount, pUndoTab, pUndoData ) );
        }
        else
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( &aDocument, aDestArea.nTab, aDestArea.nTab );

            aDocument.CopyToDocument( aDestArea.nColStart, aDestArea.nRowStart, aDestArea.nTab,
                                    aDestArea.nColEnd, aDestArea.nRowEnd, aDestArea.nTab,
                                    IDF_ALL, FALSE, pUndoDoc );

            // alten Ausgabebereich
            if (pDestData)
                aDocument.CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );

            GetUndoManager()->AddUndoAction(
                    new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
                                            FALSE, 0, NULL, pUndoData ) );
        }
    }

    if (pDestData)                                      // Zielbereich loeschen / anpassen
    {
        aDocument.DeleteAreaTab(aOldDest, IDF_CONTENTS);
        pDestData->SetArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol + nColSize - 1, rParam.nRow + nRowSize - 1 );
        pDestData->SetHeader( rParam.bByRow );
    }

    aData.OutputToDocument( &aDocument, rParam.nCol, rParam.nRow, rParam.nTab );

    SCCOL nPaintStartCol = rParam.nCol;
    SCROW nPaintStartRow = rParam.nRow;
    SCCOL nPaintEndCol = nPaintStartCol + nColSize - 1;
    SCROW nPaintEndRow = nPaintStartRow + nRowSize - 1;
    USHORT nPaintFlags = PAINT_GRID;
    if (rParam.bByCol)
        ++nPaintEndRow;
    if (rParam.bByRow)
        ++nPaintEndCol;
    if (rParam.bReferenceData)
    {
        nPaintStartCol = 0;
        nPaintEndCol = MAXCOL;
        nPaintEndRow = MAXROW;
        nPaintFlags |= PAINT_LEFT | PAINT_SIZE;
    }
    if (pDestData)
    {
        if ( aOldDest.aEnd.Col() > nPaintEndCol )
            nPaintEndCol = aOldDest.aEnd.Col();
        if ( aOldDest.aEnd.Row() > nPaintEndRow )
            nPaintEndRow = aOldDest.aEnd.Row();
    }
    PostPaint( nPaintStartCol, nPaintStartRow, rParam.nTab,
                nPaintEndCol, nPaintEndRow, rParam.nTab, nPaintFlags );
    aModificator.SetDocumentModified();
}

void ScDocShell::UseScenario( SCTAB nTab, const String& rName, BOOL bRecord )
{
    if (!aDocument.IsScenario(nTab))
    {
        SCTAB   nTabCount = aDocument.GetTableCount();
        SCTAB   nSrcTab = SCTAB_MAX;
        SCTAB   nEndTab = nTab;
        String aCompare;
        while ( nEndTab+1 < nTabCount && aDocument.IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            if (nSrcTab > MAXTAB)           // noch auf der Suche nach dem Szenario?
            {
                aDocument.GetName( nEndTab, aCompare );
                if (aCompare == rName)
                    nSrcTab = nEndTab;      // gefunden
            }
        }
        if (ValidTab(nSrcTab))
        {
            if ( aDocument.TestCopyScenario( nSrcTab, nTab ) )          // Zellschutz testen
            {
                ScDocShellModificator aModificator( *this );
                ScMarkData aScenMark;
                aDocument.MarkScenario( nSrcTab, nTab, aScenMark );
                ScRange aMultiRange;
                aScenMark.GetMultiMarkArea( aMultiRange );
                SCCOL nStartCol = aMultiRange.aStart.Col();
                SCROW nStartRow = aMultiRange.aStart.Row();
                SCCOL nEndCol = aMultiRange.aEnd.Col();
                SCROW nEndRow = aMultiRange.aEnd.Row();

                if (bRecord)
                {
                    ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pUndoDoc->InitUndo( &aDocument, nTab,nEndTab );             // auch alle Szenarien
                    //  angezeigte Tabelle:
                    aDocument.CopyToDocument( nStartCol,nStartRow,nTab,
                                    nEndCol,nEndRow,nTab, IDF_ALL,TRUE, pUndoDoc, &aScenMark );
                    //  Szenarien
                    for (SCTAB i=nTab+1; i<=nEndTab; i++)
                    {
                        pUndoDoc->SetScenario( i, TRUE );
                        String aComment;
                        Color  aColor;
                        USHORT nScenFlags;
                        aDocument.GetScenarioData( i, aComment, aColor, nScenFlags );
                        pUndoDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
                        BOOL bActive = aDocument.IsActiveScenario( i );
                        pUndoDoc->SetActiveScenario( i, bActive );
                        //  Bei Zurueckkopier-Szenarios auch Inhalte
                        if ( nScenFlags & SC_SCENARIO_TWOWAY )
                            aDocument.CopyToDocument( 0,0,i, MAXCOL,MAXROW,i,
                                                        IDF_ALL,FALSE, pUndoDoc );
                    }

                    GetUndoManager()->AddUndoAction(
                        new ScUndoUseScenario( this, aScenMark,
                                        ScArea( nTab,nStartCol,nStartRow,nEndCol,nEndRow ),
                                        pUndoDoc, rName ) );
                }

                aDocument.CopyScenario( nSrcTab, nTab );
                aDocument.SetDirty();

                //  alles painten, weil in anderen Bereichen das aktive Szenario
                //  geaendert sein kann
                //! nur, wenn sichtbare Rahmen vorhanden?
                PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
                aModificator.SetDocumentModified();
            }
            else
            {
                InfoBox aBox(GetActiveDialogParent(),
                    ScGlobal::GetRscString( STR_PROTECTIONERR ) );
                aBox.Execute();
            }
        }
        else
        {
            InfoBox aBox(GetActiveDialogParent(),
                ScGlobal::GetRscString( STR_SCENARIO_NOTFOUND ) );
            aBox.Execute();
        }
    }
    else
    {
        OSL_FAIL( "UseScenario auf Szenario-Blatt" );
    }
}

void ScDocShell::ModifyScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags )
{
    //  Undo
    String aOldName;
    aDocument.GetName( nTab, aOldName );
    String aOldComment;
    Color aOldColor;
    USHORT nOldFlags;
    aDocument.GetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );
    GetUndoManager()->AddUndoAction(
        new ScUndoScenarioFlags( this, nTab,
                aOldName, rName, aOldComment, rComment,
                aOldColor, rColor, nOldFlags, nFlags ) );

    //  ausfuehren
    ScDocShellModificator aModificator( *this );
    aDocument.RenameTab( nTab, rName );
    aDocument.SetScenarioData( nTab, rComment, rColor, nFlags );
    PostPaintGridAll();
    aModificator.SetDocumentModified();

    if ( rName != aOldName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_SELECT_SCENARIO );
}

SCTAB ScDocShell::MakeScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags,
                                    ScMarkData& rMark, BOOL bRecord )
{
    rMark.MarkToMulti();
    if (rMark.IsMultiMarked())
    {
        SCTAB nNewTab = nTab + 1;
        while (aDocument.IsScenario(nNewTab))
            ++nNewTab;

        BOOL bCopyAll = ( (nFlags & SC_SCENARIO_COPYALL) != 0 );
        const ScMarkData* pCopyMark = NULL;
        if (!bCopyAll)
            pCopyMark = &rMark;

        ScDocShellModificator aModificator( *this );

        if (bRecord)
            aDocument.BeginDrawUndo();      // drawing layer must do its own undo actions

        if (aDocument.CopyTab( nTab, nNewTab, pCopyMark ))
        {
            if (bRecord)
            {
                GetUndoManager()->AddUndoAction(
                        new ScUndoMakeScenario( this, nTab, nNewTab,
                                                rName, rComment, rColor, nFlags, rMark ));
            }

            aDocument.RenameTab( nNewTab, rName, FALSE );           // ohne Formel-Update
            aDocument.SetScenario( nNewTab, TRUE );
            aDocument.SetScenarioData( nNewTab, rComment, rColor, nFlags );

            ScMarkData aDestMark = rMark;
            aDestMark.SelectOneTable( nNewTab );

            //!     auf Filter / Buttons / Merging testen !

            ScPatternAttr aProtPattern( aDocument.GetPool() );
            aProtPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
            aDocument.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nNewTab, aProtPattern );

            ScPatternAttr aPattern( aDocument.GetPool() );
            aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
            aPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
            aDocument.ApplySelectionPattern( aPattern, aDestMark );

            if (!bCopyAll)
                aDocument.SetVisible( nNewTab, FALSE );

            //  dies ist dann das aktive Szenario
            aDocument.CopyScenario( nNewTab, nTab, TRUE );  // TRUE - nicht aus Szenario kopieren

            if (nFlags & SC_SCENARIO_SHOWFRAME)
                PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );  // Rahmen painten
            PostPaintExtras();                                          // Tabellenreiter
            aModificator.SetDocumentModified();

            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

            return nNewTab;
        }
    }
    return nTab;
}

ULONG ScDocShell::TransferTab( ScDocShell& rSrcDocShell, SCTAB nSrcPos,
                                SCTAB nDestPos, BOOL bInsertNew,
                                BOOL bNotifyAndPaint )
{
    ScDocument* pSrcDoc = rSrcDocShell.GetDocument();

    ULONG nErrVal =  aDocument.TransferTab( pSrcDoc, nSrcPos, nDestPos,
                    bInsertNew );       // no insert

    // TransferTab doesn't copy drawing objects with bInsertNew=FALSE
    if ( nErrVal > 0 && !bInsertNew)
        aDocument.TransferDrawPage( pSrcDoc, nSrcPos, nDestPos );

    if(nErrVal>0 && pSrcDoc->IsScenario( nSrcPos ))
    {
        String aComment;
        Color  aColor;
        USHORT nFlags;

        pSrcDoc->GetScenarioData( nSrcPos, aComment,aColor, nFlags);
        aDocument.SetScenario(nDestPos,TRUE);
        aDocument.SetScenarioData(nDestPos,aComment,aColor,nFlags);
        BOOL bActive = pSrcDoc->IsActiveScenario(nSrcPos);
        aDocument.SetActiveScenario(nDestPos, bActive );

        BOOL bVisible=pSrcDoc->IsVisible(nSrcPos);
        aDocument.SetVisible(nDestPos,bVisible );

    }

    if ( nErrVal > 0 && pSrcDoc->IsTabProtected( nSrcPos ) )
        aDocument.SetTabProtection(nDestPos, pSrcDoc->GetTabProtection(nSrcPos));
    if ( bNotifyAndPaint )
    {
            Broadcast( ScTablesHint( SC_TAB_INSERTED, nDestPos ) );
            PostPaintExtras();
            PostPaintGridAll();
    }
    return nErrVal;
}

BOOL ScDocShell::MoveTable( SCTAB nSrcTab, SCTAB nDestTab, BOOL bCopy, BOOL bRecord )
{
    ScDocShellModificator aModificator( *this );

    // #i92477# be consistent with ScDocFunc::InsertTable: any index past the last sheet means "append"
    // #i101139# nDestTab must be the target position, not APPEND (for CopyTabProtection etc.)
    if ( nDestTab >= aDocument.GetTableCount() )
        nDestTab = aDocument.GetTableCount();

    if (bCopy)
    {
        if (bRecord)
            aDocument.BeginDrawUndo();          // drawing layer must do its own undo actions

        String sSrcCodeName;
        aDocument.GetCodeName( nSrcTab, sSrcCodeName );
        if (!aDocument.CopyTab( nSrcTab, nDestTab ))
        {
            //! EndDrawUndo?
            return FALSE;
        }
        else
        {
            SCTAB nAdjSource = nSrcTab;
            if ( nDestTab <= nSrcTab )
                ++nAdjSource;               // new position of source table after CopyTab

            if ( aDocument.IsTabProtected( nAdjSource ) )
                aDocument.CopyTabProtection(nAdjSource, nDestTab);

            if (bRecord)
            {
                auto_ptr< vector<SCTAB> > pSrcList(new vector<SCTAB>(1, nSrcTab));
                auto_ptr< vector<SCTAB> > pDestList(new vector<SCTAB>(1, nDestTab));
                GetUndoManager()->AddUndoAction(
                        new ScUndoCopyTab(this, pSrcList.release(), pDestList.release()));
            }

            BOOL bVbaEnabled = aDocument.IsInVBAMode();
                        if ( bVbaEnabled )
                        {
                            String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
                            Reference< XLibraryContainer > xLibContainer = GetBasicContainer();
                            Reference< XVBACompatibility > xVBACompat( xLibContainer, UNO_QUERY );

                            if ( xVBACompat.is() )
                            {
                                aLibName = xVBACompat->getProjectName();
                            }

                            SCTAB nTabToUse = nDestTab;
                            if ( nDestTab == SC_TAB_APPEND )
                                nTabToUse = aDocument.GetMaxTableNumber() - 1;
                            String sCodeName;
                            String sSource;
                            Reference< XNameContainer > xLib;
                            if( xLibContainer.is() )
                            {
                                com::sun::star::uno::Any aLibAny = xLibContainer->getByName( aLibName );
                                aLibAny >>= xLib;
                            }
                            if( xLib.is() )
                            {
                                rtl::OUString sRTLSource;
                                xLib->getByName( sSrcCodeName ) >>= sRTLSource;
                                sSource = sRTLSource;
                            }
                            VBA_InsertModule( aDocument, nTabToUse, sCodeName, sSource );
                        }
                }
        Broadcast( ScTablesHint( SC_TAB_COPIED, nSrcTab, nDestTab ) );
    }
    else
    {
        if ( aDocument.GetChangeTrack() )
            return FALSE;

        if ( nSrcTab<nDestTab && nDestTab!=SC_TAB_APPEND )
            nDestTab--;

        if ( nSrcTab == nDestTab )
        {
            //! allow only for api calls?
            return TRUE;    // nothing to do, but valid
        }

        if (!aDocument.MoveTab( nSrcTab, nDestTab ))
            return FALSE;
        else if (bRecord)
        {
            auto_ptr< vector<SCTAB> > pSrcList(new vector<SCTAB>(1, nSrcTab));
            auto_ptr< vector<SCTAB> > pDestList(new vector<SCTAB>(1, nDestTab));
            GetUndoManager()->AddUndoAction(
                    new ScUndoMoveTab(this, pSrcList.release(), pDestList.release()));
        }

        Broadcast( ScTablesHint( SC_TAB_MOVED, nSrcTab, nDestTab ) );
    }

    PostPaintGridAll();
    PostPaintExtras();
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    return TRUE;
}


IMPL_LINK( ScDocShell, RefreshDBDataHdl, ScRefreshTimer*, pRefreshTimer )
{
    ScDBDocFunc aFunc(*this);

    BOOL bContinue = TRUE;
    ScDBData* pDBData = static_cast<ScDBData*>(pRefreshTimer);
    ScImportParam aImportParam;
    pDBData->GetImportParam( aImportParam );
    if (aImportParam.bImport && !pDBData->HasImportSelection())
    {
        ScRange aRange;
        pDBData->GetArea( aRange );
                Reference< ::com::sun::star::sdbc::XResultSet> xResultSet;
        bContinue = aFunc.DoImport( aRange.aStart.Tab(), aImportParam, xResultSet, NULL, TRUE, FALSE ); //! Api-Flag as parameter
        // internal operations (sort, query, subtotal) only if no error
        if (bContinue)
        {
            aFunc.RepeatDB( pDBData->GetName(), TRUE, TRUE );
            RefreshPivotTables(aRange);
        }
    }

    return bContinue != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
