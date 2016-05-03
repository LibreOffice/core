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

#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <sfx2/viewfrm.hxx>

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include "dbdocfun.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "dbdata.hxx"
#include "markdata.hxx"
#include "undodat.hxx"
#include "progress.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "dbdocutl.hxx"
#include "editable.hxx"
#include "hints.hxx"
#include "miscuno.hxx"
#include "chgtrack.hxx"
#include <refupdatecontext.hxx>

using namespace com::sun::star;

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"

//! move to a header file?
#define SC_DBPROP_DATASOURCENAME    "DataSourceName"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"

void ScDBDocFunc::ShowInBeamer( const ScImportParam& rParam, SfxViewFrame* pFrame )
{
    //  called after opening the database beamer

    if ( !pFrame || !rParam.bImport )
        return;

    uno::Reference<frame::XFrame> xFrame = pFrame->GetFrame().GetFrameInterface();
    uno::Reference<frame::XDispatchProvider> xDP(xFrame, uno::UNO_QUERY);

    uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
                                        "_beamer",
                                        frame::FrameSearchFlag::CHILDREN);
    if (xBeamerFrame.is())
    {
        uno::Reference<frame::XController> xController = xBeamerFrame->getController();
        uno::Reference<view::XSelectionSupplier> xControllerSelection(xController, uno::UNO_QUERY);
        if (xControllerSelection.is())
        {
            sal_Int32 nType = rParam.bSql ? sdb::CommandType::COMMAND :
                        ( (rParam.nType == ScDbQuery) ? sdb::CommandType::QUERY :
                                                        sdb::CommandType::TABLE );

            svx::ODataAccessDescriptor aSelection;
            aSelection.setDataSource(rParam.aDBName);
            aSelection[svx::daCommand]      <<= rParam.aStatement;
            aSelection[svx::daCommandType]  <<= nType;

            xControllerSelection->select(uno::makeAny(aSelection.createPropertyValueSequence()));
        }
        else
        {
            OSL_FAIL("no selection supplier in the beamer!");
        }
    }
}

bool ScDBDocFunc::DoImportUno( const ScAddress& rPos,
                                const uno::Sequence<beans::PropertyValue>& aArgs )
{
    svx::ODataAccessDescriptor aDesc( aArgs );      // includes selection and result set

    //  create database range
    ScDBData* pDBData = rDocShell.GetDBData( ScRange(rPos), SC_DB_IMPORT, SC_DBSEL_KEEP );
    DBG_ASSERT(pDBData, "can't create DB data");
    OUString sTarget = pDBData->GetName();

    UpdateImport( sTarget, aDesc );

    return true;
}

bool ScDBDocFunc::DoImport( SCTAB nTab, const ScImportParam& rParam,
        const svx::ODataAccessDescriptor* pDescriptor, bool bRecord, bool bAddrInsert )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    ScChangeTrack *pChangeTrack = nullptr;
    ScRange aChangedRange;

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDBData* pDBData = nullptr;
    if ( !bAddrInsert )
    {
        pDBData = rDoc.GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                            rParam.nCol2, rParam.nRow2 );
        if (!pDBData)
        {
            OSL_FAIL( "DoImport: no DBData" );
            return false;
        }
    }

    vcl::Window* pWaitWin = ScDocShell::GetActiveDialogParent();
    if (pWaitWin)
        pWaitWin->EnterWait();
    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    bool bApi = false;                      //! pass as argument
    bool bTruncated = false;                // for warning
    sal_uInt16 nErrStringId = 0;
    OUString aErrorMessage;

    SCCOL nCol = rParam.nCol1;
    SCROW nRow = rParam.nRow1;
    SCCOL nEndCol = nCol;                   // end of resulting database area
    SCROW nEndRow = nRow;

    bool bDoSelection = false;
    bool bRealSelection = false;            // sal_True if not everything is selected
    bool bBookmarkSelection = false;
    sal_Int32 nListPos = 0;
    sal_Int32 nRowsRead = 0;
    sal_Int32 nListCount = 0;

    uno::Sequence<uno::Any> aSelection;
    if ( pDescriptor && pDescriptor->has(svx::daSelection) )
    {
        (*pDescriptor)[svx::daSelection] >>= aSelection;
        nListCount = aSelection.getLength();
        if ( nListCount > 0 )
        {
            bDoSelection = true;
            if ( pDescriptor->has(svx::daBookmarkSelection) )
                bBookmarkSelection = ScUnoHelpFunctions::GetBoolFromAny( (*pDescriptor)[svx::daBookmarkSelection] );
            if ( bBookmarkSelection )
            {
                // From bookmarks, there's no way to detect if all records are selected.
                // Rely on base to pass no selection in that case.
                bRealSelection = true;
            }
        }
    }

    uno::Reference<sdbc::XResultSet> xResultSet;
    if ( pDescriptor && pDescriptor->has(svx::daCursor) )
        xResultSet.set((*pDescriptor)[svx::daCursor], uno::UNO_QUERY);

    // ImportDoc - also used for Redo
    ScDocument* pImportDoc = new ScDocument( SCDOCMODE_UNDO );
    pImportDoc->InitUndo( &rDoc, nTab, nTab );

    //  get data from database into import document

    try
    {
        //  progress bar
        //  only text (title is still needed, for the cancel button)
        ScProgress aProgress( &rDocShell, ScGlobal::GetRscString(STR_UNDO_IMPORTDATA), 0 );

        uno::Reference<sdbc::XRowSet> xRowSet( xResultSet, uno::UNO_QUERY );
        bool bDispose = false;
        if ( !xRowSet.is() )
        {
            bDispose = true;
            xRowSet.set(comphelper::getProcessServiceFactory()->createInstance(
                            SC_SERVICE_ROWSET ),
                        uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
            OSL_ENSURE( xRowProp.is(), "can't get RowSet" );
            if ( xRowProp.is() )
            {

                //  set source parameters

                sal_Int32 nType = rParam.bSql ? sdb::CommandType::COMMAND :
                            ( (rParam.nType == ScDbQuery) ? sdb::CommandType::QUERY :
                                                            sdb::CommandType::TABLE );

                xRowProp->setPropertyValue( SC_DBPROP_DATASOURCENAME, uno::Any(rParam.aDBName) );

                xRowProp->setPropertyValue( SC_DBPROP_COMMAND, uno::Any(rParam.aStatement) );

                xRowProp->setPropertyValue( SC_DBPROP_COMMANDTYPE, uno::Any(nType) );

                uno::Reference<sdb::XCompletedExecution> xExecute( xRowSet, uno::UNO_QUERY );
                if ( xExecute.is() )
                {
                    uno::Reference<task::XInteractionHandler> xHandler(
                        task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), nullptr),
                        uno::UNO_QUERY_THROW);
                    xExecute->executeWithCompletion( xHandler );
                }
                else
                    xRowSet->execute();
            }
        }
        if ( xRowSet.is() )
        {

            //  get column descriptions

            long nColCount = 0;
            uno::Reference<sdbc::XResultSetMetaData> xMeta;
            uno::Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp( xRowSet, uno::UNO_QUERY );
            if ( xMetaSupp.is() )
                xMeta = xMetaSupp->getMetaData();
            if ( xMeta.is() )
                nColCount = xMeta->getColumnCount();    // this is the number of real columns

            if ( rParam.nCol1 + nColCount - 1 > MAXCOL )
            {
                nColCount = 0;
                //! error message
            }

            uno::Reference<sdbcx::XRowLocate> xLocate;
            if ( bBookmarkSelection )
            {
                xLocate.set( xRowSet, uno::UNO_QUERY );
                if ( !xLocate.is() )
                {
                    SAL_WARN( "sc.ui","can't get XRowLocate");
                    bDoSelection = bRealSelection = bBookmarkSelection = false;
                }
            }

            uno::Reference<sdbc::XRow> xRow( xRowSet, uno::UNO_QUERY );
            if ( nColCount > 0 && xRow.is() )
            {
                nEndCol = (SCCOL)( rParam.nCol1 + nColCount - 1 );

                uno::Sequence<sal_Int32> aColTypes( nColCount );    // column types
                uno::Sequence<sal_Bool> aColCurr( nColCount );      // currency flag is not in types
                sal_Int32* pTypeArr = aColTypes.getArray();
                sal_Bool* pCurrArr = aColCurr.getArray();
                for (long i=0; i<nColCount; i++)
                {
                    pTypeArr[i] = xMeta->getColumnType( i+1 );
                    pCurrArr[i] = xMeta->isCurrency( i+1 );
                }

                if ( !bAddrInsert )                 // read column names
                {
                    nCol = rParam.nCol1;
                    for (long i=0; i<nColCount; i++)
                    {
                        pImportDoc->SetString( nCol, nRow, nTab,
                                                xMeta->getColumnLabel( i+1 ) );
                        ++nCol;
                    }
                    ++nRow;
                }

                bool bEnd = false;
                if ( !bDoSelection )
                    xRowSet->beforeFirst();
                sal_uInt16 nInserted = 0;
                while ( !bEnd )
                {
                    //  skip rows that are not selected
                    if ( !bDoSelection )
                    {
                        if ( !(bEnd = !xRowSet->next()) )
                            ++nRowsRead;
                    }
                    else
                    {
                        if (nListPos < nListCount)
                        {
                            if ( bBookmarkSelection )
                            {
                                bEnd = !xLocate->moveToBookmark(aSelection[nListPos]);
                            }
                            else    // use record numbers
                            {
                                sal_Int32 nNextRow = 0;
                                aSelection[nListPos] >>= nNextRow;
                                if ( nRowsRead+1 < nNextRow )
                                    bRealSelection = true;
                                bEnd = !xRowSet->absolute(nRowsRead = nNextRow);
                            }
                            ++nListPos;
                        }
                        else
                        {
                            if ( !bBookmarkSelection && xRowSet->next() )
                                bRealSelection = true;                      // more data available but not used
                            bEnd = true;
                        }
                    }

                    if ( !bEnd )
                    {
                        if ( ValidRow(nRow) )
                        {
                            nCol = rParam.nCol1;
                            for (long i=0; i<nColCount; i++)
                            {
                                ScDatabaseDocUtil::PutData( pImportDoc, nCol, nRow, nTab,
                                                xRow, i+1, pTypeArr[i], pCurrArr[i] );
                                ++nCol;
                            }
                            nEndRow = nRow;
                            ++nRow;

                            //  progress bar

                            ++nInserted;
                            if (!(nInserted & 15))
                            {
                                OUString aPict = ScGlobal::GetRscString( STR_PROGRESS_IMPORT );
                                OUString aText = aPict.getToken(0,'#');
                                aText += OUString::number( nInserted );
                                aText += aPict.getToken(1,'#');

                                if (!aProgress.SetStateText( 0, aText ))    // stopped by user?
                                {
                                    bEnd = true;
                                    bSuccess = false;
                                    nErrStringId = STR_DATABASE_ABORTED;
                                }
                            }
                        }
                        else        // past the end of the spreadsheet
                        {
                            bEnd = true;            // don't continue
                            bTruncated = true;      // warning flag
                        }
                    }
                }

                bSuccess = true;
            }

            if ( bDispose )
                ::comphelper::disposeComponent( xRowSet );
        }
    }
    catch ( const sdbc::SQLException& rError )
    {
        aErrorMessage = rError.Message;
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL("Unexpected exception in database");
    }

    //  test for cell protection

    bool bKeepFormat = !bAddrInsert && pDBData->IsKeepFmt();
    bool bMoveCells = !bAddrInsert && pDBData->IsDoSize();
    SCCOL nFormulaCols = 0; // columns to be filled with formulas
    if (bMoveCells && nEndCol == rParam.nCol2)
    {
        //  if column count changes, formulas would become invalid anyway
        //  -> only set nFormulaCols for unchanged column count

        SCCOL nTestCol = rParam.nCol2 + 1;      // right of the data
        SCROW nTestRow = rParam.nRow1 + 1;      // below the title row
        while ( nTestCol <= MAXCOL &&
                rDoc.GetCellType(ScAddress( nTestCol, nTestRow, nTab )) == CELLTYPE_FORMULA )
            ++nTestCol, ++nFormulaCols;
    }

    if (bSuccess)
    {
        //  old and new range editable?
        ScEditableTester aTester;
        aTester.TestBlock( &rDoc, nTab, rParam.nCol1,rParam.nRow1,rParam.nCol2,rParam.nRow2 );
        aTester.TestBlock( &rDoc, nTab, rParam.nCol1,rParam.nRow1,nEndCol,nEndRow );
        if ( !aTester.IsEditable() )
        {
            nErrStringId = aTester.GetMessageId();
            bSuccess = false;
        }
        else if ( (pChangeTrack = rDoc.GetChangeTrack()) != nullptr )
            aChangedRange = ScRange(rParam.nCol1, rParam.nRow1, nTab,
                        nEndCol+nFormulaCols, nEndRow, nTab );
    }

    if ( bSuccess && bMoveCells )
    {
        ScRange aOld( rParam.nCol1, rParam.nRow1, nTab,
                        rParam.nCol2+nFormulaCols, rParam.nRow2, nTab );
        ScRange aNew( rParam.nCol1, rParam.nRow1, nTab,
                        nEndCol+nFormulaCols, nEndRow, nTab );
        if (!rDoc.CanFitBlock( aOld, aNew ))
        {
            nErrStringId = STR_MSSG_DOSUBTOTALS_2;      // can't insert cells
            bSuccess = false;
        }
    }

    //  copy data from import doc into real document

    if ( bSuccess )
    {
        if (bKeepFormat)
        {
            //  keep formatting of title and first data row from the document
            //  CopyToDocument also copies styles, Apply... needs separate calls

            SCCOL nMinEndCol = std::min( rParam.nCol2, nEndCol );    // not too much
            nMinEndCol = sal::static_int_cast<SCCOL>( nMinEndCol + nFormulaCols );  // only if column count unchanged
            pImportDoc->DeleteAreaTab( 0,0, MAXCOL,MAXROW, nTab, InsertDeleteFlags::ATTRIB );
            rDoc.CopyToDocument( rParam.nCol1, rParam.nRow1, nTab,
                                    nMinEndCol, rParam.nRow1, nTab,
                                    InsertDeleteFlags::ATTRIB, false, pImportDoc );

            SCROW nDataStartRow = rParam.nRow1+1;
            for (SCCOL nCopyCol=rParam.nCol1; nCopyCol<=nMinEndCol; nCopyCol++)
            {
                const ScPatternAttr* pSrcPattern = rDoc.GetPattern(
                                                    nCopyCol, nDataStartRow, nTab );
                pImportDoc->ApplyPatternAreaTab( nCopyCol, nDataStartRow, nCopyCol, nEndRow,
                                                    nTab, *pSrcPattern );
                const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                if (pStyle)
                    pImportDoc->ApplyStyleAreaTab( nCopyCol, nDataStartRow, nCopyCol, nEndRow,
                                                    nTab, *pStyle );
            }
        }

        //  don't set cell protection attribute if table is protected
        if (rDoc.IsTabProtected(nTab))
        {
            ScPatternAttr aPattern(pImportDoc->GetPool());
            aPattern.GetItemSet().Put( ScProtectionAttr( false,false,false,false ) );
            pImportDoc->ApplyPatternAreaTab( 0,0,MAXCOL,MAXROW, nTab, aPattern );
        }

        //  copy old data for undo

        SCCOL nUndoEndCol = std::max( nEndCol, rParam.nCol2 );       // rParam = old end
        SCROW nUndoEndRow = std::max( nEndRow, rParam.nRow2 );

        ScDocument* pUndoDoc = nullptr;
        ScDBData* pUndoDBData = nullptr;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( &rDoc, nTab, nTab );

            if ( !bAddrInsert )
                pUndoDBData = new ScDBData( *pDBData );
        }

        ScMarkData aNewMark;
        aNewMark.SelectOneTable( nTab );

        if (bRecord)
        {
            // do not touch notes (ScUndoImportData does not support drawing undo)
            InsertDeleteFlags nCopyFlags = InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE;

            //  nFormulaCols is set only if column count is unchanged
            rDoc.CopyToDocument( rParam.nCol1, rParam.nRow1, nTab,
                                    nEndCol+nFormulaCols, nEndRow, nTab,
                                    nCopyFlags, false, pUndoDoc );
            if ( rParam.nCol2 > nEndCol )
                rDoc.CopyToDocument( nEndCol+1, rParam.nRow1, nTab,
                                        nUndoEndCol, nUndoEndRow, nTab,
                                        nCopyFlags, false, pUndoDoc );
            if ( rParam.nRow2 > nEndRow )
                rDoc.CopyToDocument( rParam.nCol1, nEndRow+1, nTab,
                                        nUndoEndCol+nFormulaCols, nUndoEndRow, nTab,
                                        nCopyFlags, false, pUndoDoc );
        }

        //  move new data

        if (bMoveCells)
        {
            //  clear only the range without the formulas,
            //  so the formula title and first row are preserved

            ScRange aDelRange( rParam.nCol1, rParam.nRow1, nTab,
                                rParam.nCol2, rParam.nRow2, nTab );
            rDoc.DeleteAreaTab( aDelRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );  // ohne die Formeln

            ScRange aOld( rParam.nCol1, rParam.nRow1, nTab,
                            rParam.nCol2+nFormulaCols, rParam.nRow2, nTab );
            ScRange aNew( rParam.nCol1, rParam.nRow1, nTab,
                            nEndCol+nFormulaCols, nEndRow, nTab );
            rDoc.FitBlock( aOld, aNew, false );        // Formeln nicht loeschen
        }
        else if ( nEndCol < rParam.nCol2 )      // DeleteArea calls PutInOrder
            rDoc.DeleteArea( nEndCol+1, rParam.nRow1, rParam.nCol2, rParam.nRow2,
                                aNewMark, InsertDeleteFlags::CONTENTS & ~InsertDeleteFlags::NOTE );

        //  CopyToDocument doesn't remove contents
        rDoc.DeleteAreaTab( rParam.nCol1, rParam.nRow1, nEndCol, nEndRow, nTab, InsertDeleteFlags::CONTENTS & ~InsertDeleteFlags::NOTE );

        //  remove each column from ImportDoc after copying to reduce memory usage
        bool bOldAutoCalc = rDoc.GetAutoCalc();
        rDoc.SetAutoCalc( false );             // outside of the loop
        for (SCCOL nCopyCol = rParam.nCol1; nCopyCol <= nEndCol; nCopyCol++)
        {
            pImportDoc->CopyToDocument( nCopyCol, rParam.nRow1, nTab, nCopyCol, nEndRow, nTab,
                                        InsertDeleteFlags::ALL, false, &rDoc );
            pImportDoc->DeleteAreaTab( nCopyCol, rParam.nRow1, nCopyCol, nEndRow, nTab, InsertDeleteFlags::CONTENTS );
        }
        rDoc.SetAutoCalc( bOldAutoCalc );

        if (nFormulaCols > 0)               // copy formulas
        {
            if (bKeepFormat)            // formats for formulas
                pImportDoc->CopyToDocument( nEndCol+1, rParam.nRow1, nTab,
                                            nEndCol+nFormulaCols, nEndRow, nTab,
                                            InsertDeleteFlags::ATTRIB, false, &rDoc );
            // fill formulas
            ScMarkData aMark;
            aMark.SelectOneTable(nTab);

            sal_uLong nProgCount = nFormulaCols;
            nProgCount *= nEndRow-rParam.nRow1-1;
            ScProgress aProgress( rDoc.GetDocumentShell(),
                    ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

            rDoc.Fill( nEndCol+1, rParam.nRow1+1, nEndCol+nFormulaCols, rParam.nRow1+1,
                            &aProgress, aMark, nEndRow-rParam.nRow1-1, FILL_TO_BOTTOM, FILL_SIMPLE );
        }

        //  if new range is smaller, clear old contents

        if (!bMoveCells)        // move has happened above
        {
            if ( rParam.nCol2 > nEndCol )
                rDoc.DeleteArea( nEndCol+1, rParam.nRow1, rParam.nCol2, rParam.nRow2,
                                    aNewMark, InsertDeleteFlags::CONTENTS );
            if ( rParam.nRow2 > nEndRow )
                rDoc.DeleteArea( rParam.nCol1, nEndRow+1, rParam.nCol2, rParam.nRow2,
                                    aNewMark, InsertDeleteFlags::CONTENTS );
        }

        if( !bAddrInsert )      // update database range
        {
            pDBData->SetImportParam( rParam );
            pDBData->SetHeader( true );
            pDBData->SetByRow( true );
            pDBData->SetArea( nTab, rParam.nCol1,rParam.nRow1, nEndCol,nEndRow );
            pDBData->SetImportSelection( bRealSelection );
            rDoc.CompileDBFormula();
        }

        if (bRecord)
        {
            ScDocument* pRedoDoc = pImportDoc;
            pImportDoc = nullptr;

            if (nFormulaCols > 0)                   // include filled formulas for redo
                rDoc.CopyToDocument( rParam.nCol1, rParam.nRow1, nTab,
                                        nEndCol+nFormulaCols, nEndRow, nTab,
                                        InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, pRedoDoc );

            ScDBData* pRedoDBData = pDBData ? new ScDBData( *pDBData ) : nullptr;

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoImportData( &rDocShell, nTab,
                                        rParam, nUndoEndCol, nUndoEndRow,
                                        nFormulaCols,
                                        pUndoDoc, pRedoDoc, pUndoDBData, pRedoDBData ) );
        }

        sc::SetFormulaDirtyContext aCxt;
        rDoc.SetAllFormulasDirty(aCxt);
        rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab), PAINT_GRID);
        aModificator.SetDocumentModified();

        ScDBRangeRefreshedHint aHint( rParam );
        rDoc.BroadcastUno( aHint );

        if (pWaitWin)
            pWaitWin->LeaveWait();

        if ( bTruncated && !bApi )          // show warning
            ErrorHandler::HandleError(SCWARN_IMPORT_RANGE_OVERFLOW);
    }
    else if ( !bApi )
    {
        if (pWaitWin)
            pWaitWin->LeaveWait();

        if (aErrorMessage.isEmpty())
        {
            if (!nErrStringId)
                nErrStringId = STR_MSSG_IMPORTDATA_0;
            aErrorMessage = ScGlobal::GetRscString( nErrStringId );
        }
        ScopedVclPtrInstance< InfoBox > aInfoBox( ScDocShell::GetActiveDialogParent(), aErrorMessage );
        aInfoBox->Execute();
    }

    delete pImportDoc;

    if (bSuccess && pChangeTrack)
        pChangeTrack->AppendInsert ( aChangedRange );

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
