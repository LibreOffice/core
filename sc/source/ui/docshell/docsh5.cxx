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

#include <memory>
#include <sal/config.h>

#include <cassert>

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <unotools/charclass.hxx>

#include <com/sun/star/script/vba/XVBACompatibility.hpp>

#include <dociter.hxx>
#include <docsh.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <globalnames.hxx>
#include <undodat.hxx>
#include <undotab.hxx>
#include <undoblk.hxx>
#include <dpobject.hxx>
#include <dpshttab.hxx>
#include <dbdocfun.hxx>
#include <consoli.hxx>
#include <dbdata.hxx>
#include <progress.hxx>
#include <olinetab.hxx>
#include <patattr.hxx>
#include <attrib.hxx>
#include <docpool.hxx>
#include <uiitems.hxx>
#include <sc.hrc>
#include <sizedev.hxx>
#include <clipparam.hxx>
#include <rowheightcontext.hxx>
#include <refupdatecontext.hxx>

using com::sun::star::script::XLibraryContainer;
using com::sun::star::script::vba::XVBACompatibility;
using com::sun::star::container::XNameContainer;
using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;

using ::std::unique_ptr;
using ::std::vector;

//  former viewfunc/dbfunc methods

void ScDocShell::ErrorMessage(TranslateId pGlobStrId)
{
    //! StopMarking at the (active) view?

    weld::Window* pParent = GetActiveDialogParent();
    weld::WaitObject aWaitOff( pParent );
    bool bFocus = pParent && pParent->has_focus();

    if (pGlobStrId && pGlobStrId == STR_PROTECTIONERR)
    {
        if (IsReadOnly())
        {
            pGlobStrId = STR_READONLYERR;
        }
    }

    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  ScResId(pGlobStrId)));
    xInfoBox->run();

    if (bFocus)
        pParent->grab_focus();
}

bool ScDocShell::IsEditable() const
{
    // import into read-only document is possible - must be extended if other filters use api
    // #i108547# MSOOXML filter uses "IsChangeReadOnlyEnabled" property

    return !IsReadOnly() || m_pDocument->IsImportingXML() || m_pDocument->IsChangeReadOnlyEnabled();
}

void ScDocShell::DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2 )
{
    ScDocShellModificator aModificator( *this );
    // the auto-filter is in the first row of the area
    m_pDocument->RemoveFlagsTab( nX1, nY1, nX2, nY1, nTab, ScMF::Auto );
    PostPaint( nX1, nY1, nTab, nX2, nY1, nTab, PaintPartFlags::Grid );
    // No SetDocumentModified, as the unnamed database range might have to be restored later.
    // The UNO hint is broadcast directly instead, to keep UNO objects in valid state.
    m_pDocument->BroadcastUno( SfxHint( SfxHintId::DataChanged ) );
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
    //  Not simply GetDBAtCursor: The continuous data range for "unnamed" (GetDataArea) may be
    //  located next to the cursor; so a named DB range needs to be searched for there as well.
    ScDBCollection* pColl = m_pDocument->GetDBCollection();
    ScDBData* pData = m_pDocument->GetDBAtArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
    if (!pData)
        pData = pColl->GetDBNearCursor(nCol, nRow, nTab );

    bool bSelected = ( eSel == ScGetDBSelection::ForceMark ||
            (rMarked.aStart != rMarked.aEnd && eSel != ScGetDBSelection::RowDown) );
    bool bOnlyDown = (!bSelected && eSel == ScGetDBSelection::RowDown && rMarked.aStart.Row() == rMarked.aEnd.Row());

    bool bUseThis = false;
    if (pData)
    {
        //      take range, if nothing else is marked

        SCTAB nDummy;
        SCCOL nOldCol1;
        SCROW nOldRow1;
        SCCOL nOldCol2;
        SCROW nOldRow2;
        pData->GetArea( nDummy, nOldCol1,nOldRow1, nOldCol2,nOldRow2 );
        bool bIsNoName = ( pData->GetName() == STR_DB_LOCAL_NONAME );

        if (!bSelected)
        {
            bUseThis = true;
            if ( bIsNoName && (eMode == SC_DB_MAKE || eMode == SC_DB_AUTOFILTER) )
            {
                // If nothing marked or only one row marked, adapt
                // "unnamed" to contiguous area.
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
                m_pDocument->GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, false, bOnlyDown );
                if ( nOldCol1 != nStartCol || nOldCol2 != nEndCol || nOldRow1 != nStartRow )
                    bUseThis = false;               // doesn't fit at all
                else if ( nOldRow2 != nEndRow )
                {
                    //  extend range to new end row
                    pData->SetArea( nTab, nOldCol1,nOldRow1, nOldCol2,nEndRow );
                }
            }
        }
        else
        {
            if ( nOldCol1 == nStartCol && nOldRow1 == nStartRow &&
                 nOldCol2 == nEndCol && nOldRow2 == nEndRow )               // marked precisely?
                bUseThis = true;
            else
                bUseThis = false;           // always take marking (Bug 11964)
        }

        //      never take "unnamed" for import

        if ( bUseThis && eMode == SC_DB_IMPORT && bIsNoName )
            bUseThis = false;
    }

    if ( bUseThis )
    {
        pData->GetArea( nStartTab, nStartCol,nStartRow, nEndCol,nEndRow );
    }
    else if ( eMode == SC_DB_OLD )
    {
        pData = nullptr;                           // nothing found
    }
    else
    {
        if ( !bSelected )
        {                                       // continuous range
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
            m_pDocument->GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, false, bOnlyDown );
        }

        bool bHasHeader = m_pDocument->HasColHeader( nStartCol,nStartRow, nEndCol,nEndRow, nTab );

        ScDBData* pNoNameData = m_pDocument->GetAnonymousDBData(nTab);
        if ( eMode != SC_DB_IMPORT && pNoNameData)
        {
            // Do not reset AutoFilter range during temporary operations on
            // other ranges, use the document global temporary anonymous range
            // instead. But, if AutoFilter is to be toggled then do use the
            // sheet-local DB range.
            bool bSheetLocal = true;
            if (eMode != SC_DB_AUTOFILTER && pNoNameData->HasAutoFilter())
            {
                bSheetLocal = false;
                pNoNameData = m_pDocument->GetAnonymousDBData();
                if (!pNoNameData)
                {
                    m_pDocument->SetAnonymousDBData( std::unique_ptr<ScDBData>(new ScDBData( STR_DB_LOCAL_NONAME,
                            nTab, nStartCol, nStartRow, nEndCol, nEndRow, true, bHasHeader) ) );
                    pNoNameData = m_pDocument->GetAnonymousDBData();
                }
                // ScDocShell::CancelAutoDBRange() would restore the
                // sheet-local anonymous DBData from pOldAutoDBRange, unset so
                // that won't happen with data of a previous sheet-local
                // DBData.
                m_pOldAutoDBRange.reset();
            }
            else if (!m_pOldAutoDBRange)
            {
                // store the old unnamed database range with its settings for undo
                // (store at the first change, get the state before all changes)
                m_pOldAutoDBRange.reset( new ScDBData( *pNoNameData ) );
            }
            else if (m_pOldAutoDBRange->GetTab() != pNoNameData->GetTab())
            {
                // Different sheet-local unnamed DB range than the previous one.
                *m_pOldAutoDBRange = *pNoNameData;
            }

            SCCOL nOldX1;                                   // take old range away cleanly
            SCROW nOldY1;                                   //! (UNDO ???)
            SCCOL nOldX2;
            SCROW nOldY2;
            SCTAB nOldTab;
            pNoNameData->GetArea( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );

            // If previously bHasHeader was set and the new range starts on the
            // same row and intersects the old column range, then don't reset
            // bHasHeader but assume that the new range still has headers, just
            // some are empty or numeric.
            if (!bHasHeader && pNoNameData->HasHeader() && nTab == nOldTab && nStartRow == nOldY1 &&
                    nStartCol <= nOldY2 && nOldY1 <= nEndCol)
                bHasHeader = true;

            // Remove AutoFilter button flags only for sheet-local DB range,
            // not if a temporary is used.
            if (bSheetLocal)
                DBAreaDeleted( nOldTab, nOldX1, nOldY1, nOldX2 );

            pNoNameData->SetSortParam( ScSortParam() );             // reset parameter
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( nTab, nStartCol,nStartRow, nEndCol,nEndRow );     // set anew
            pNoNameData->SetByRow( true );
            pNoNameData->SetHeader( bHasHeader );
            pNoNameData->SetAutoFilter( false );
        }
        else
        {
            std::unique_ptr<ScDBCollection> pUndoColl;

            if (eMode==SC_DB_IMPORT)
            {
                m_pDocument->PreprocessDBDataUpdate();
                pUndoColl.reset( new ScDBCollection( *pColl ) );   // Undo for import range

                OUString aImport = ScResId( STR_DBNAME_IMPORT );
                tools::Long nCount = 0;
                const ScDBData* pDummy = nullptr;
                ScDBCollection::NamedDBs& rDBs = pColl->getNamedDBs();
                OUString aNewName;
                do
                {
                    ++nCount;
                    aNewName = aImport + OUString::number( nCount );
                    pDummy = rDBs.findByUpperName(ScGlobal::getCharClass().uppercase(aNewName));
                }
                while (pDummy);
                pNoNameData = new ScDBData( aNewName, nTab,
                                nStartCol,nStartRow, nEndCol,nEndRow,
                                true, bHasHeader );
                bool ins = rDBs.insert(std::unique_ptr<ScDBData>(pNoNameData));
                assert(ins); (void)ins;
            }
            else
            {
                pNoNameData = new ScDBData(STR_DB_LOCAL_NONAME, nTab,
                                nStartCol,nStartRow, nEndCol,nEndRow,
                                true, bHasHeader );
                m_pDocument->SetAnonymousDBData(nTab, std::unique_ptr<ScDBData>(pNoNameData));
            }

            if ( pUndoColl )
            {
                m_pDocument->CompileHybridFormula();

                GetUndoManager()->AddUndoAction( std::make_unique<ScUndoDBData>( this,
                        std::move(pUndoColl),
                        std::make_unique<ScDBCollection>( *pColl ) ) );
            }

            //  no longer needed to register new range at the Sba

            //  announce "Import1", etc., at the Navigator
            if (eMode==SC_DB_IMPORT)
                SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
        }
        pData = pNoNameData;
    }

    return pData;
}

ScDBData* ScDocShell::GetAnonymousDBData(const ScRange& rRange)
{
    ScDBCollection* pColl = m_pDocument->GetDBCollection();
    if (!pColl)
        return nullptr;

    ScDBData* pData = pColl->getAnonDBs().getByRange(rRange);
    if (!pData)
        return nullptr;

    if (!pData->HasHeader())
    {
        bool bHasHeader = m_pDocument->HasColHeader(
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aStart.Tab());
        pData->SetHeader(bHasHeader);
    }

    return pData;
}

std::unique_ptr<ScDBData> ScDocShell::GetOldAutoDBRange()
{
    return std::move(m_pOldAutoDBRange);
}

void ScDocShell::CancelAutoDBRange()
{
    // called when dialog is cancelled
//moggi:TODO
    if ( !m_pOldAutoDBRange )
        return;

    SCTAB nTab = GetCurTab();
    ScDBData* pDBData = m_pDocument->GetAnonymousDBData(nTab);
    if ( pDBData )
    {
        SCCOL nRangeX1;
        SCROW nRangeY1;
        SCCOL nRangeX2;
        SCROW nRangeY2;
        SCTAB nRangeTab;
        pDBData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
        DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2 );

        *pDBData = *m_pOldAutoDBRange;    // restore old settings

        if ( m_pOldAutoDBRange->HasAutoFilter() )
        {
            // restore AutoFilter buttons
            m_pOldAutoDBRange->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            m_pDocument->ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, ScMF::Auto );
            PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PaintPartFlags::Grid );
        }
    }

    m_pOldAutoDBRange.reset();
}

        //  adjust height
        //! merge with docfunc

bool ScDocShell::AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab )
{
    ScSizeDeviceProvider aProv(this);
    Fraction aZoom(1,1);
    sc::RowHeightContext aCxt(m_pDocument->MaxRow(), aProv.GetPPTX(), aProv.GetPPTY(), aZoom, aZoom, aProv.GetDevice());
    bool bChange = m_pDocument->SetOptimalHeight(aCxt, nStartRow,nEndRow, nTab, true);

    if (bChange)
    {
        // tdf#76183: recalculate objects' positions
        m_pDocument->SetDrawPageSize(nTab);

        PostPaint( 0,nStartRow,nTab, m_pDocument->MaxCol(),m_pDocument->MaxRow(),nTab, PaintPartFlags::Grid|PaintPartFlags::Left );
    }

    return bChange;
}

void ScDocShell::UpdateAllRowHeights( const ScMarkData* pTabMark )
{
    // update automatic row heights

    ScSizeDeviceProvider aProv(this);
    Fraction aZoom(1,1);
    sc::RowHeightContext aCxt(m_pDocument->MaxRow(), aProv.GetPPTX(), aProv.GetPPTY(), aZoom, aZoom, aProv.GetDevice());
    m_pDocument->UpdateAllRowHeights(aCxt, pTabMark);
}

void ScDocShell::UpdateAllRowHeights(const bool bOnlyUsedRows)
{
    // update automatic row heights on all sheets using the newer ScDocRowHeightUpdater
    ScSizeDeviceProvider aProv(this);
    ScDocRowHeightUpdater aUpdater(*m_pDocument, aProv.GetDevice(), aProv.GetPPTX(),
                                   aProv.GetPPTY(), nullptr);
    aUpdater.update(bOnlyUsedRows);
}

void ScDocShell::UpdatePendingRowHeights( SCTAB nUpdateTab, bool bBefore )
{
    bool bIsUndoEnabled = m_pDocument->IsUndoEnabled();
    m_pDocument->EnableUndo( false );
    m_pDocument->LockStreamValid( true );      // ignore draw page size (but not formula results)
    if ( bBefore )          // check all sheets up to nUpdateTab
    {
        SCTAB nTabCount = m_pDocument->GetTableCount();
        if ( nUpdateTab >= nTabCount )
            nUpdateTab = nTabCount-1;     // nUpdateTab is inclusive

        ScMarkData aUpdateSheets(m_pDocument->GetSheetLimits());
        SCTAB nTab;
        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( m_pDocument->IsPendingRowHeights( nTab ) )
                aUpdateSheets.SelectTable( nTab, true );

        if (aUpdateSheets.GetSelectCount())
            UpdateAllRowHeights(&aUpdateSheets);        // update with a single progress bar

        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( aUpdateSheets.GetTableSelect( nTab ) )
            {
                m_pDocument->UpdatePageBreaks( nTab );
                m_pDocument->SetPendingRowHeights( nTab, false );
            }
    }
    else                    // only nUpdateTab
    {
        if ( m_pDocument->IsPendingRowHeights( nUpdateTab ) )
        {
            AdjustRowHeight( 0, m_pDocument->MaxRow(), nUpdateTab );
            m_pDocument->UpdatePageBreaks( nUpdateTab );
            m_pDocument->SetPendingRowHeights( nUpdateTab, false );
        }
    }
    m_pDocument->LockStreamValid( false );
    m_pDocument->EnableUndo( bIsUndoEnabled );
}

void ScDocShell::RefreshPivotTables( const ScRange& rSource )
{
    ScDPCollection* pColl = m_pDocument->GetDPCollection();
    if (!pColl)
        return;

    ScDBDocFunc aFunc(*this);
    for (size_t i = 0, n = pColl->GetCount(); i < n; ++i)
    {
        ScDPObject& rOld = (*pColl)[i];

        const ScSheetSourceDesc* pSheetDesc = rOld.GetSheetDesc();
        if (pSheetDesc && pSheetDesc->GetSourceRange().Intersects(rSource))
            aFunc.UpdatePivotTable(rOld, true, false);
    }
}

static OUString lcl_GetAreaName( ScDocument* pDoc, const ScArea* pArea )
{
    ScDBData* pData = pDoc->GetDBAtArea( pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                        pArea->nColEnd, pArea->nRowEnd );
    if (pData)
        return pData->GetName();

    OUString aName;
    pDoc->GetName( pArea->nTab, aName );
    return aName;
}

static ScDBData* getUndoData(ScDBData* pDestData)
{
    if (!pDestData)
        return nullptr;
    return new ScDBData(*pDestData);
}

void ScDocShell::DoConsolidate( const ScConsolidateParam& rParam, bool bRecord )
{
    ScConsData aData;

    sal_uInt16 nPos;
    SCCOL nColSize = 0;
    SCROW nRowSize = 0;
    bool bErr = false;
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea const & rArea = rParam.pDataAreas[nPos];
        nColSize = std::max( nColSize, SCCOL( rArea.nColEnd - rArea.nColStart + 1 ) );
        nRowSize = std::max( nRowSize, SCROW( rArea.nRowEnd - rArea.nRowStart + 1 ) );

                                        // test if source data were moved
        if (rParam.bReferenceData)
            if (rArea.nTab == rParam.nTab && rArea.nRowEnd >= rParam.nRow)
                bErr = true;
    }

    if (bErr)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_CONSOLIDATE_ERR1)));
        xInfoBox->run();
        return;
    }

    //      execute

    weld::WaitObject aWait( GetActiveDialogParent() );
    ScDocShellModificator aModificator( *this );

    ScRange aOldDest;
    ScDBData* pDestData = m_pDocument->GetDBAtCursor( rParam.nCol, rParam.nRow, rParam.nTab, ScDBDataPortion::TOP_LEFT );
    if (pDestData)
        pDestData->GetArea(aOldDest);

    aData.SetSize( nColSize, nRowSize );
    aData.SetFlags( rParam.eFunction, rParam.bByCol, rParam.bByRow, rParam.bReferenceData );
    if ( rParam.bByCol || rParam.bByRow )
        for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
        {
            ScArea const & rArea = rParam.pDataAreas[nPos];
            aData.AddFields( m_pDocument.get(), rArea.nTab, rArea.nColStart, rArea.nRowStart,
                                                       rArea.nColEnd, rArea.nRowEnd );
        }
    aData.DoneFields();
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea const & rArea = rParam.pDataAreas[nPos];
        aData.AddData( m_pDocument.get(), rArea.nTab, rArea.nColStart, rArea.nRowStart,
                                                 rArea.nColEnd, rArea.nRowEnd );
        aData.AddName( lcl_GetAreaName(m_pDocument.get(), &rArea) );
    }

    aData.GetSize( nColSize, nRowSize );
    if (bRecord && nColSize > 0 && nRowSize > 0)
    {
        std::unique_ptr<ScDBData> pUndoData(getUndoData(pDestData));

        SCTAB nDestTab = rParam.nTab;
        ScArea aDestArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol+nColSize-1, rParam.nRow+nRowSize-1 );
        if (rParam.bByCol) ++aDestArea.nColEnd;
        if (rParam.bByRow) ++aDestArea.nRowEnd;

        if (rParam.bReferenceData)
        {
            SCTAB nTabCount = m_pDocument->GetTableCount();
            SCROW nInsertCount = aData.GetInsertCount();

            // old outlines
            ScOutlineTable* pTable = m_pDocument->GetOutlineTable( nDestTab );
            std::unique_ptr<ScOutlineTable> pUndoTab(pTable ? new ScOutlineTable( *pTable ) : nullptr);

            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( *m_pDocument, 0, nTabCount-1, false, true );

            // row state
            m_pDocument->CopyToDocument(0, 0, nDestTab, m_pDocument->MaxCol(), m_pDocument->MaxRow(), nDestTab,
                                     InsertDeleteFlags::NONE, false, *pUndoDoc);

            // all formulas
            m_pDocument->CopyToDocument(0, 0, 0, m_pDocument->MaxCol(), m_pDocument->MaxRow(), nTabCount-1,
                                     InsertDeleteFlags::FORMULA, false, *pUndoDoc);

            // complete output rows
            m_pDocument->CopyToDocument(0, aDestArea.nRowStart, nDestTab,
                                     m_pDocument->MaxCol(),aDestArea.nRowEnd, nDestTab,
                                     InsertDeleteFlags::ALL, false, *pUndoDoc);

            // old output range
            if (pDestData)
                m_pDocument->CopyToDocument(aOldDest, InsertDeleteFlags::ALL, false, *pUndoDoc);

            GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoConsolidate>( this, aDestArea, rParam, std::move(pUndoDoc),
                                            true, nInsertCount, std::move(pUndoTab), std::move(pUndoData) ) );
        }
        else
        {
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( *m_pDocument, aDestArea.nTab, aDestArea.nTab );

            m_pDocument->CopyToDocument(aDestArea.nColStart, aDestArea.nRowStart, aDestArea.nTab,
                                     aDestArea.nColEnd, aDestArea.nRowEnd, aDestArea.nTab,
                                     InsertDeleteFlags::ALL, false, *pUndoDoc);

            // old output range
            if (pDestData)
                m_pDocument->CopyToDocument(aOldDest, InsertDeleteFlags::ALL, false, *pUndoDoc);

            GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoConsolidate>( this, aDestArea, rParam, std::move(pUndoDoc),
                                            false, 0, nullptr, std::move(pUndoData) ) );
        }
    }

    if (pDestData)                                      // delete / adjust destination range
    {
        m_pDocument->DeleteAreaTab(aOldDest, InsertDeleteFlags::CONTENTS);
        pDestData->SetArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol + nColSize - 1, rParam.nRow + nRowSize - 1 );
        pDestData->SetHeader( rParam.bByRow );
    }

    aData.OutputToDocument( *m_pDocument, rParam.nCol, rParam.nRow, rParam.nTab );

    SCCOL nPaintStartCol = rParam.nCol;
    SCROW nPaintStartRow = rParam.nRow;
    SCCOL nPaintEndCol = nPaintStartCol + nColSize - 1;
    SCROW nPaintEndRow = nPaintStartRow + nRowSize - 1;
    PaintPartFlags nPaintFlags = PaintPartFlags::Grid;
    if (rParam.bByCol)
        ++nPaintEndRow;
    if (rParam.bByRow)
        ++nPaintEndCol;
    if (rParam.bReferenceData)
    {
        nPaintStartCol = 0;
        nPaintEndCol = m_pDocument->MaxCol();
        nPaintEndRow = m_pDocument->MaxRow();
        nPaintFlags |= PaintPartFlags::Left | PaintPartFlags::Size;
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

void ScDocShell::UseScenario( SCTAB nTab, const OUString& rName, bool bRecord )
{
    if (!m_pDocument->IsScenario(nTab))
    {
        SCTAB   nTabCount = m_pDocument->GetTableCount();
        SCTAB   nSrcTab = SCTAB_MAX;
        SCTAB   nEndTab = nTab;
        OUString aCompare;
        while ( nEndTab+1 < nTabCount && m_pDocument->IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            if (nSrcTab > MAXTAB)           // still searching for the scenario?
            {
                m_pDocument->GetName( nEndTab, aCompare );
                if (aCompare == rName)
                    nSrcTab = nEndTab;      // found
            }
        }
        if (ValidTab(nSrcTab))
        {
            if ( m_pDocument->TestCopyScenario( nSrcTab, nTab ) )          // test cell protection
            {
                ScDocShellModificator aModificator( *this );
                ScMarkData aScenMark(m_pDocument->GetSheetLimits());
                m_pDocument->MarkScenario( nSrcTab, nTab, aScenMark );
                const ScRange& aMultiRange = aScenMark.GetMultiMarkArea();
                SCCOL nStartCol = aMultiRange.aStart.Col();
                SCROW nStartRow = aMultiRange.aStart.Row();
                SCCOL nEndCol = aMultiRange.aEnd.Col();
                SCROW nEndRow = aMultiRange.aEnd.Row();

                if (bRecord)
                {
                    ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
                    pUndoDoc->InitUndo( *m_pDocument, nTab,nEndTab );             // also all scenarios
                    //  shown table:
                    m_pDocument->CopyToDocument(nStartCol, nStartRow, nTab,
                                             nEndCol, nEndRow, nTab, InsertDeleteFlags::ALL,
                                             true, *pUndoDoc, &aScenMark);
                    //  scenarios
                    for (SCTAB i=nTab+1; i<=nEndTab; i++)
                    {
                        pUndoDoc->SetScenario( i, true );
                        OUString aComment;
                        Color  aColor;
                        ScScenarioFlags nScenFlags;
                        m_pDocument->GetScenarioData( i, aComment, aColor, nScenFlags );
                        pUndoDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
                        bool bActive = m_pDocument->IsActiveScenario( i );
                        pUndoDoc->SetActiveScenario( i, bActive );
                        //  At copy-back scenarios also contents
                        if ( nScenFlags & ScScenarioFlags::TwoWay )
                            m_pDocument->CopyToDocument(0, 0, i, m_pDocument->MaxCol(), m_pDocument->MaxRow(), i,
                                                     InsertDeleteFlags::ALL, false, *pUndoDoc );
                    }

                    GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoUseScenario>( this, aScenMark,
                                        ScArea( nTab,nStartCol,nStartRow,nEndCol,nEndRow ),
                                        std::move(pUndoDoc), rName ) );
                }

                m_pDocument->CopyScenario( nSrcTab, nTab );

                sc::SetFormulaDirtyContext aCxt;
                m_pDocument->SetAllFormulasDirty(aCxt);

                //  paint all, because the active scenario may be modified in other ranges;
                //! only if there are visible frames?
                PostPaint( 0,0,nTab, m_pDocument->MaxCol(),m_pDocument->MaxRow(),nTab, PaintPartFlags::Grid );
                aModificator.SetDocumentModified();
            }
            else
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              ScResId(STR_PROTECTIONERR)));
                xInfoBox->run();
            }
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          ScResId(STR_SCENARIO_NOTFOUND)));
            xInfoBox->run();
        }
    }
    else
    {
        OSL_FAIL( "UseScenario on Scenario-Sheet" );
    }
}

void ScDocShell::ModifyScenario( SCTAB nTab, const OUString& rName, const OUString& rComment,
                                    const Color& rColor, ScScenarioFlags nFlags )
{
    //  Undo
    OUString aOldName;
    m_pDocument->GetName( nTab, aOldName );
    OUString aOldComment;
    Color aOldColor;
    ScScenarioFlags nOldFlags;
    m_pDocument->GetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );
    GetUndoManager()->AddUndoAction(
        std::make_unique<ScUndoScenarioFlags>(this, nTab,
                aOldName, rName, aOldComment, rComment,
                aOldColor, rColor, nOldFlags, nFlags) );

    //  execute
    ScDocShellModificator aModificator( *this );
    m_pDocument->RenameTab( nTab, rName );
    m_pDocument->SetScenarioData( nTab, rComment, rColor, nFlags );
    PostPaintGridAll();
    aModificator.SetDocumentModified();

    if (aOldName != rName)
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_SELECT_SCENARIO );
}

SCTAB ScDocShell::MakeScenario( SCTAB nTab, const OUString& rName, const OUString& rComment,
                                    const Color& rColor, ScScenarioFlags nFlags,
                                    ScMarkData& rMark, bool bRecord )
{
    rMark.MarkToMulti();
    if (rMark.IsMultiMarked())
    {
        SCTAB nNewTab = nTab + 1;
        while (m_pDocument->IsScenario(nNewTab))
            ++nNewTab;

        bool bCopyAll = ( (nFlags & ScScenarioFlags::CopyAll) != ScScenarioFlags::NONE );
        const ScMarkData* pCopyMark = nullptr;
        if (!bCopyAll)
            pCopyMark = &rMark;

        ScDocShellModificator aModificator( *this );

        if (bRecord)
            m_pDocument->BeginDrawUndo();      // drawing layer must do its own undo actions

        if (m_pDocument->CopyTab( nTab, nNewTab, pCopyMark ))
        {
            if (bRecord)
            {
                GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoMakeScenario>( this, nTab, nNewTab,
                                                rName, rComment, rColor, nFlags, rMark ));
            }

            m_pDocument->RenameTab( nNewTab, rName);
            m_pDocument->SetScenario( nNewTab, true );
            m_pDocument->SetScenarioData( nNewTab, rComment, rColor, nFlags );

            ScMarkData aDestMark = rMark;
            aDestMark.SelectOneTable( nNewTab );

            //!     test for filter / buttons / merging

            ScPatternAttr aProtPattern(m_pDocument->getCellAttributeHelper());
            aProtPattern.GetItemSet().Put( ScProtectionAttr( true ) );
            m_pDocument->ApplyPatternAreaTab( 0,0, m_pDocument->MaxCol(),m_pDocument->MaxRow(), nNewTab, aProtPattern );

            ScPatternAttr aPattern(m_pDocument->getCellAttributeHelper());
            aPattern.GetItemSet().Put( ScMergeFlagAttr( ScMF::Scenario ) );
            aPattern.GetItemSet().Put( ScProtectionAttr( true ) );
            m_pDocument->ApplySelectionPattern( aPattern, aDestMark );

            if (!bCopyAll)
                m_pDocument->SetVisible( nNewTab, false );

            //  this is the active scenario, then
            m_pDocument->CopyScenario( nNewTab, nTab, true );  // sal_True - don't copy anything from scenario

            if (nFlags & ScScenarioFlags::ShowFrame)
                PostPaint( 0,0,nTab, m_pDocument->MaxCol(),m_pDocument->MaxRow(),nTab, PaintPartFlags::Grid );  // paint frames
            PostPaintExtras();                                          // table tab
            aModificator.SetDocumentModified();

            // A scenario tab is like a hidden sheet, broadcasting also
            // notifies ScTabViewShell to add an ScViewData::maTabData entry.
            Broadcast( ScTablesHint( SC_TAB_INSERTED, nNewTab ));
            SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

            return nNewTab;
        }
    }
    return nTab;
}

bool ScDocShell::TransferTab( ScDocShell& rSrcDocShell, SCTAB nSrcPos,
                                SCTAB nDestPos, bool bInsertNew,
                                bool bNotifyAndPaint )
{
    ScDocument& rSrcDoc = rSrcDocShell.GetDocument();

    // set the transferred area to the copyparam to make adjusting formulas possible
    ScClipParam aParam;
    ScRange aRange(0, 0, nSrcPos, m_pDocument->MaxCol(), m_pDocument->MaxRow(), nSrcPos);
    aParam.maRanges.push_back(aRange);
    rSrcDoc.SetClipParam(aParam);

    bool bValid =  m_pDocument->TransferTab( rSrcDoc, nSrcPos, nDestPos,
                    bInsertNew );       // no insert

    // TransferTab doesn't copy drawing objects with bInsertNew=FALSE
    if ( bValid && !bInsertNew)
        m_pDocument->TransferDrawPage( rSrcDoc, nSrcPos, nDestPos );

    if(bValid && rSrcDoc.IsScenario( nSrcPos ))
    {
        OUString aComment;
        Color  aColor;
        ScScenarioFlags nFlags;

        rSrcDoc.GetScenarioData( nSrcPos, aComment,aColor, nFlags);
        m_pDocument->SetScenario(nDestPos,true);
        m_pDocument->SetScenarioData(nDestPos,aComment,aColor,nFlags);
        bool bActive = rSrcDoc.IsActiveScenario(nSrcPos);
        m_pDocument->SetActiveScenario(nDestPos, bActive );

        bool bVisible = rSrcDoc.IsVisible(nSrcPos);
        m_pDocument->SetVisible(nDestPos,bVisible );

    }

    if ( bValid && rSrcDoc.IsTabProtected( nSrcPos ) )
        m_pDocument->SetTabProtection(nDestPos, rSrcDoc.GetTabProtection(nSrcPos));
    if ( bNotifyAndPaint )
    {
            Broadcast( ScTablesHint( SC_TAB_INSERTED, nDestPos ) );
            PostPaintExtras();
            PostPaintGridAll();
    }
    return bValid;
}

bool ScDocShell::MoveTable( SCTAB nSrcTab, SCTAB nDestTab, bool bCopy, bool bRecord )
{
    ScDocShellModificator aModificator( *this );

    // #i92477# be consistent with ScDocFunc::InsertTable: any index past the last sheet means "append"
    // #i101139# nDestTab must be the target position, not APPEND (for CopyTabProtection etc.)
    if ( nDestTab >= m_pDocument->GetTableCount() )
        nDestTab = m_pDocument->GetTableCount();

    if (bCopy)
    {
        if (bRecord)
            m_pDocument->BeginDrawUndo();          // drawing layer must do its own undo actions

        OUString sSrcCodeName;
        m_pDocument->GetCodeName( nSrcTab, sSrcCodeName );
        if (!m_pDocument->CopyTab( nSrcTab, nDestTab ))
        {
            //! EndDrawUndo?
            return false;
        }
        else
        {
            SCTAB nAdjSource = nSrcTab;
            if ( nDestTab <= nSrcTab )
                ++nAdjSource;               // new position of source table after CopyTab

            if ( m_pDocument->IsTabProtected( nAdjSource ) )
                m_pDocument->CopyTabProtection(nAdjSource, nDestTab);

            if (bRecord)
            {
                unique_ptr< vector<SCTAB> > pSrcList(new vector<SCTAB>(1, nSrcTab));
                unique_ptr< vector<SCTAB> > pDestList(new vector<SCTAB>(1, nDestTab));
                GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoCopyTab>(this, std::move(pSrcList), std::move(pDestList)));
            }

            bool bVbaEnabled = m_pDocument->IsInVBAMode();
            if ( bVbaEnabled )
            {
                OUString aLibName( u"Standard"_ustr );
                Reference< XLibraryContainer > xLibContainer = GetBasicContainer();
                Reference< XVBACompatibility > xVBACompat( xLibContainer, UNO_QUERY );

                if ( xVBACompat.is() )
                {
                    aLibName = xVBACompat->getProjectName();
                }

                SCTAB nTabToUse = nDestTab;
                if ( nDestTab == SC_TAB_APPEND )
                    nTabToUse = m_pDocument->GetMaxTableNumber() - 1;
                OUString sSource;
                try
                {
                    Reference< XNameContainer > xLib;
                    if( xLibContainer.is() )
                    {
                        css::uno::Any aLibAny = xLibContainer->getByName( aLibName );
                        aLibAny >>= xLib;
                    }
                    if( xLib.is() )
                    {
                        xLib->getByName( sSrcCodeName ) >>= sSource;
                    }
                }
                catch ( const css::uno::Exception& )
                {
                }
                VBA_InsertModule( *m_pDocument, nTabToUse, sSource );
            }
        }
        Broadcast( ScTablesHint( SC_TAB_COPIED, nSrcTab, nDestTab ) );
    }
    else
    {
        if ( m_pDocument->GetChangeTrack() )
            return false;

        if ( nSrcTab<nDestTab && nDestTab!=SC_TAB_APPEND )
            nDestTab--;

        if ( nSrcTab == nDestTab )
        {
            //! allow only for api calls?
            return true;    // nothing to do, but valid
        }

        std::optional<ScProgress> pProgress(std::in_place, this, ScResId(STR_UNDO_MOVE_TAB),
                                                m_pDocument->GetCodeCount(), true);
        bool bDone = m_pDocument->MoveTab( nSrcTab, nDestTab, &*pProgress );
        pProgress.reset();
        if (!bDone)
        {
            return false;
        }
        else if (bRecord)
        {
            unique_ptr< vector<SCTAB> > pSrcList(new vector<SCTAB>(1, nSrcTab));
            unique_ptr< vector<SCTAB> > pDestList(new vector<SCTAB>(1, nDestTab));
            GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoMoveTab>(this, std::move(pSrcList), std::move(pDestList)));
        }

        Broadcast( ScTablesHint( SC_TAB_MOVED, nSrcTab, nDestTab ) );
    }

    PostPaintGridAll();
    PostPaintExtras();
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

    return true;
}

IMPL_LINK( ScDocShell, RefreshDBDataHdl, Timer*, pRefreshTimer, void )
{
    ScDBDocFunc aFunc(*this);

    ScDBData* pDBData = static_cast<ScDBData*>(pRefreshTimer);
    ScImportParam aImportParam;
    pDBData->GetImportParam( aImportParam );
    if (aImportParam.bImport && !pDBData->HasImportSelection())
    {
        ScRange aRange;
        pDBData->GetArea( aRange );
        bool bContinue = aFunc.DoImport( aRange.aStart.Tab(), aImportParam, nullptr ); //! Api-Flag as parameter
        // internal operations (sort, query, subtotal) only if no error
        if (bContinue)
        {
            aFunc.RepeatDB( pDBData->GetName(), true, true );
            RefreshPivotTables(aRange);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
