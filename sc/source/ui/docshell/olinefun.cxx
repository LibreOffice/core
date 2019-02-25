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

#include <sfx2/bindings.hxx>

#include <olinefun.hxx>

#include <docsh.hxx>
#include <olinetab.hxx>
#include <tabvwsh.hxx>
#include <undodat.hxx>
#include <globstr.hrc>
#include <sc.hrc>

#include <comphelper/lok.hxx>


static void lcl_InvalidateOutliner( SfxBindings* pBindings )
{
    if ( pBindings )
    {
        pBindings->Invalidate( SID_OUTLINE_SHOW );
        pBindings->Invalidate( SID_OUTLINE_HIDE );
        pBindings->Invalidate( SID_OUTLINE_REMOVE );

        pBindings->Invalidate( SID_STATUS_SUM );            // because of enabling/disabling
        pBindings->Invalidate( SID_ATTR_SIZE );
    }
}

//! Move PaintWidthHeight to DocShell ?

static void lcl_PaintWidthHeight( ScDocShell& rDocShell, SCTAB nTab,
                                    bool bColumns, SCCOLROW nStart, SCCOLROW nEnd )
{
    ScDocument& rDoc = rDocShell.GetDocument();

    PaintPartFlags nParts = PaintPartFlags::Grid;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;         // for testing if merged
    SCROW nEndRow = MAXROW;
    if ( bColumns )
    {
        nParts |= PaintPartFlags::Top;
        nStartCol = static_cast<SCCOL>(nStart);
        nEndCol = static_cast<SCCOL>(nEnd);
    }
    else
    {
        nParts |= PaintPartFlags::Left;
        nStartRow = nStart;
        nEndRow = nEnd;
    }
    if (rDoc.HasAttrib( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                            HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
    {
        nStartCol = 0;
        nStartRow = 0;
    }
    rDocShell.PostPaint( nStartCol,nStartRow,nTab, MAXCOL,MAXROW,nTab, nParts );
}

void ScOutlineDocFunc::MakeOutline( const ScRange& rRange, bool bColumns, bool bRecord, bool bApi )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument& rDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab, true );
    std::unique_ptr<ScOutlineTable> pUndoTab;

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        pUndoTab.reset(new ScOutlineTable( *pTable ));

    ScOutlineArray& rArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

    bool bRes;
    bool bSize = false;
    if ( bColumns )
        bRes = rArray.Insert( nStartCol, nEndCol, bSize );
    else
        bRes = rArray.Insert( nStartRow, nEndRow, bSize );

    if ( bRes )
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoMakeOutline>( &rDocShell,
                                        nStartCol,nStartRow,nTab,nEndCol,nEndRow,nTab,
                                        std::move(pUndoTab), bColumns, true ) );
        }

        rDoc.SetStreamValid(nTab, false);

        PaintPartFlags nParts = PaintPartFlags::NONE;   // Data range hasn't been changed
        if ( bColumns )
            nParts |= PaintPartFlags::Top;
        else
            nParts |= PaintPartFlags::Left;
        if ( bSize )
            nParts |= PaintPartFlags::Size;

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }
    else
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_MAKEOUTLINE_0); // "Grouping not possible"
    }
}

void ScOutlineDocFunc::RemoveOutline( const ScRange& rRange, bool bColumns, bool bRecord, bool bApi )
{
    bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
    if (pTable)
    {
        std::unique_ptr<ScOutlineTable> pUndoTab;
        if (bRecord)
            pUndoTab.reset(new ScOutlineTable( *pTable ));

        ScOutlineArray& rArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

        bool bRes;
        bool bSize = false;
        if ( bColumns )
            bRes = rArray.Remove( nStartCol, nEndCol, bSize );
        else
            bRes = rArray.Remove( nStartRow, nEndRow, bSize );

        if ( bRes )
        {
            if (bRecord)
            {
                rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoMakeOutline>( &rDocShell,
                                            nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                            std::move(pUndoTab), bColumns, false ) );
            }

            rDoc.SetStreamValid(nTab, false);

            PaintPartFlags nParts = PaintPartFlags::NONE;   // Data range hasn't been changed
            if ( bColumns )
                nParts |= PaintPartFlags::Top;
            else
                nParts |= PaintPartFlags::Left;
            if ( bSize )
                nParts |= PaintPartFlags::Size;

            rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
            rDocShell.SetDocumentModified();
            bDone = true;
            lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

            // we are not enabling again -> no UpdatePageBreaks
        }
    }

    if (!bDone && !bApi)
        rDocShell.ErrorMessage(STR_MSSG_REMOVEOUTLINE_0);   // "Ungrouping not possible"
}

bool ScOutlineDocFunc::RemoveAllOutlines( SCTAB nTab, bool bRecord )
{
    bool bSuccess = false;
    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
    if (pTable)
    {
        if (bRecord)
        {
            SCCOLROW nCol1, nCol2, nRow1, nRow2;
            pTable->GetColArray().GetRange( nCol1, nCol2 );
            pTable->GetRowArray().GetRange( nRow1, nRow2 );
            SCCOL nStartCol = static_cast<SCCOL>(nCol1);
            SCROW nStartRow = nRow1;
            SCCOL nEndCol = static_cast<SCCOL>(nCol2);
            SCROW nEndRow = nRow2;

            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
            rDoc.CopyToDocument(nStartCol, 0, nTab, nEndCol, MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
            rDoc.CopyToDocument(0, nStartRow, nTab, MAXCOL, nEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);

            std::unique_ptr<ScOutlineTable> pUndoTab(new ScOutlineTable( *pTable ));

            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoRemoveAllOutlines>( &rDocShell,
                                                nStartCol, nStartRow, nTab,
                                                nEndCol, nEndRow, nTab,
                                                std::move(pUndoDoc), std::move(pUndoTab) ) );
        }

        SelectLevel( nTab, true,  pTable->GetColArray().GetDepth(), false, false );
        SelectLevel( nTab, false, pTable->GetRowArray().GetDepth(), false, false );
        rDoc.SetOutlineTable( nTab, nullptr );

        rDoc.UpdatePageBreaks( nTab );

        rDoc.SetStreamValid(nTab, false);

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
        bSuccess = true;
    }

    return bSuccess;
}

void ScOutlineDocFunc::AutoOutline( const ScRange& rRange, bool bRecord )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScOutlineTable> pUndoTab;

    if ( pTable )
    {
        if ( bRecord )
        {
            pUndoTab.reset(new ScOutlineTable( *pTable ));

            SCCOLROW nCol1, nCol2, nRow1, nRow2;
            pTable->GetColArray().GetRange( nCol1, nCol2 );
            pTable->GetRowArray().GetRange( nRow1, nRow2 );
            SCCOL nOutStartCol = static_cast<SCCOL>(nCol1);
            SCROW nOutStartRow = nRow1;
            SCCOL nOutEndCol = static_cast<SCCOL>(nCol2);
            SCROW nOutEndRow = nRow2;

            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
            rDoc.CopyToDocument(nOutStartCol, 0, nTab, nOutEndCol, MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
            rDoc.CopyToDocument(0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        // enable
        SelectLevel( nTab, true,  pTable->GetColArray().GetDepth(), false, false );
        SelectLevel( nTab, false, pTable->GetRowArray().GetDepth(), false, false );
        rDoc.SetOutlineTable( nTab, nullptr );
    }

    rDoc.DoAutoOutline( nStartCol,nStartRow, nEndCol,nEndRow, nTab );

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoAutoOutline>( &rDocShell,
                                    nStartCol, nStartRow, nTab,
                                    nEndCol, nEndRow, nTab,
                                    std::move(pUndoDoc), std::move(pUndoTab) ) );
    }

    rDoc.SetStreamValid(nTab, false);

    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size );
    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
}

bool ScOutlineDocFunc::SelectLevel( SCTAB nTab, bool bColumns, sal_uInt16 nLevel,
                                    bool bRecord, bool bPaint )
{
    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );             // already there
    if (!pTable)
        return false;
    ScOutlineArray& rArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

    SCCOLROW nStart, nEnd;
    rArray.GetRange( nStart, nEnd );

    // TODO undo can mess things up when another view is editing a cell in the range of group entry
    // this is a temporarily workaround
    if (!comphelper::LibreOfficeKit::isActive() && bRecord )
    {
        std::unique_ptr<ScOutlineTable> pUndoTab(new ScOutlineTable( *pTable ));
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        if (bColumns)
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true );
            rDoc.CopyToDocument(static_cast<SCCOL>(nStart), 0, nTab,
                                static_cast<SCCOL>(nEnd), MAXROW, nTab, InsertDeleteFlags::NONE, false,
                                *pUndoDoc);
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument(0, nStart, nTab, MAXCOL, nEnd, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoOutlineLevel>( &rDocShell,
                                    nStart, nEnd, nTab,             //! calculate start and end
                                    std::move(pUndoDoc), std::move(pUndoTab),
                                    bColumns, nLevel ) );
    }

    ScSubOutlineIterator aIter( &rArray );                   // all entries
    ScOutlineEntry* pEntry;
    while ((pEntry=aIter.GetNext()) != nullptr)
    {
        SCCOLROW nThisStart = pEntry->GetStart();
        SCCOLROW nThisEnd   = pEntry->GetEnd();

        sal_uInt16 nThisLevel = aIter.LastLevel();
        bool bShow = (nThisLevel < nLevel);

        if (!bShow && ScTabViewShell::isAnyEditViewInRange(bColumns, nThisStart, nThisEnd))
            continue;

        if (bShow)                                          // enable
        {
            pEntry->SetHidden( false );
            pEntry->SetVisible( true );
        }
        else if ( nThisLevel == nLevel )                    // disable
        {
            pEntry->SetHidden( true );
            pEntry->SetVisible( true );
        }
        else                                                // hidden below
        {
            if (comphelper::LibreOfficeKit::isActive() && nThisLevel > 0)
            {
                pEntry->SetHidden( true );
                const ScOutlineEntry* pParentEntry = rArray.GetEntryByPos(nThisLevel - 1, nThisStart);
                if (pParentEntry && pParentEntry->IsHidden())
                    pEntry->SetVisible( false );
            }
            else
            {
                pEntry->SetVisible( false );
            }
        }

        for (SCCOLROW i=nThisStart; i<=nThisEnd; i++)
        {
            if ( bColumns )
                rDoc.ShowCol( static_cast<SCCOL>(i), nTab, bShow );
            else
            {
                // show several rows together, don't show filtered rows
                SCROW nFilterEnd = i;
                bool bFiltered = rDoc.RowFiltered( i, nTab, nullptr, &nFilterEnd );
                nFilterEnd = std::min( nThisEnd, nFilterEnd );
                if ( !bShow || !bFiltered )
                    rDoc.ShowRows( i, nFilterEnd, nTab, bShow );
                i = nFilterEnd;
            }
        }
    }

    rDoc.SetDrawPageSize(nTab);
    rDoc.UpdatePageBreaks( nTab );

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if ( pViewSh )
        pViewSh->OnLOKShowHideColRow(bColumns, nStart - 1);

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return true;
}

bool ScOutlineDocFunc::ShowMarkedOutlines( const ScRange& rRange, bool bRecord )
{
    bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );

    if (pTable)
    {
        ScOutlineEntry* pEntry;
        SCCOLROW nStart;
        SCCOLROW nEnd;
        SCCOLROW nMin;
        SCCOLROW nMax;
        SCCOLROW i;

        // TODO undo can mess things up when another view is editing a cell in the range of group entry
        // this is a temporarily workaround
        if ( !comphelper::LibreOfficeKit::isActive() && bRecord )
        {
            std::unique_ptr<ScOutlineTable> pUndoTab(new ScOutlineTable( *pTable ));
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
            rDoc.CopyToDocument(nStartCol, 0, nTab, nEndCol, MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
            rDoc.CopyToDocument(0, nStartRow, nTab, MAXCOL, nEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);

            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoOutlineBlock>( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        std::move(pUndoDoc), std::move(pUndoTab), true ) );
        }

        //  Columns

        nMin=MAXCOL;
        nMax=0;
        ScOutlineArray& rColArray = pTable->GetColArray();
        ScSubOutlineIterator aColIter( &rColArray );
        while ((pEntry=aColIter.GetNext()) != nullptr)
        {
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();
            if ( nStart>=nStartCol && nEnd<=nEndCol )
            {
                pEntry->SetHidden( false );
                pEntry->SetVisible( true );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        const SCCOLROW nMinStartCol = nMin;
        for ( i=nMin; i<=nMax; i++ )
            rDoc.ShowCol( static_cast<SCCOL>(i), nTab, true );

        //  Rows

        nMin=MAXROW;
        nMax=0;
        ScOutlineArray& rRowArray = pTable->GetRowArray();
        ScSubOutlineIterator aRowIter( &rRowArray );
        while ((pEntry=aRowIter.GetNext()) != nullptr)
        {
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();
            if ( nStart>=nStartRow && nEnd<=nEndRow )
            {
                pEntry->SetHidden( false );
                pEntry->SetVisible( true );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        const SCCOLROW nMinStartRow = nMin;
        for ( i=nMin; i<=nMax; i++ )
        {
            // show several rows together, don't show filtered rows
            SCROW nFilterEnd = i;
            bool bFiltered = rDoc.RowFiltered( i, nTab, nullptr, &nFilterEnd );
            nFilterEnd = std::min( nMax, nFilterEnd );
            if ( !bFiltered )
                rDoc.ShowRows( i, nFilterEnd, nTab, true );
            i = nFilterEnd;
        }


        rDoc.SetDrawPageSize(nTab);
        rDoc.UpdatePageBreaks( nTab );

        ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
        if ( pViewSh )
        {
            pViewSh->OnLOKShowHideColRow(/*columns: */ true, nMinStartCol - 1);
            pViewSh->OnLOKShowHideColRow(/*columns: */ false, nMinStartRow - 1);
        }

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top );
        rDocShell.SetDocumentModified();
        bDone = true;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    return bDone;
}

bool ScOutlineDocFunc::HideMarkedOutlines( const ScRange& rRange, bool bRecord )
{
    bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );

    if (pTable)
    {
        const ScOutlineEntry* pEntry;
        size_t nColLevel;
        size_t nRowLevel;
        sal_uInt16 nCount;
        SCCOLROW nStart;
        SCCOLROW nEnd;
        sal_uInt16 i;

        SCCOLROW nEffStartCol = nStartCol;
        SCCOLROW nEffEndCol   = nEndCol;
        ScOutlineArray& rColArray = pTable->GetColArray();
        rColArray.FindTouchedLevel( nStartCol, nEndCol, nColLevel );
        rColArray.ExtendBlock( nColLevel, nEffStartCol, nEffEndCol );
        SCCOLROW nEffStartRow = nStartRow;
        SCCOLROW nEffEndRow   = nEndRow;
        ScOutlineArray& rRowArray = pTable->GetRowArray();
        rRowArray.FindTouchedLevel( nStartRow, nEndRow, nRowLevel );
        rRowArray.ExtendBlock( nRowLevel, nEffStartRow, nEffEndRow );

        // TODO undo can mess things up when another view is editing a cell in the range of group entry
        // this is a temporarily workaround
        if ( !comphelper::LibreOfficeKit::isActive() && bRecord )
        {
            std::unique_ptr<ScOutlineTable> pUndoTab(new ScOutlineTable( *pTable ));
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
            rDoc.CopyToDocument(static_cast<SCCOL>(nEffStartCol), 0, nTab,
                                static_cast<SCCOL>(nEffEndCol), MAXROW, nTab, InsertDeleteFlags::NONE,
                                false, *pUndoDoc);
            rDoc.CopyToDocument(0, nEffStartRow, nTab, MAXCOL, nEffEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);

            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoOutlineBlock>( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        std::move(pUndoDoc), std::move(pUndoTab), false ) );
        }

        //  Columns

        nCount = rColArray.GetCount(nColLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = rColArray.GetEntry(nColLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( static_cast<SCCOLROW>(nStartCol)<=nEnd && static_cast<SCCOLROW>(nEndCol)>=nStart )
                HideOutline( nTab, true, nColLevel, i, false, false );
        }

        //  Rows

        nCount = rRowArray.GetCount(nRowLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = rRowArray.GetEntry(nRowLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( nStartRow<=nEnd && nEndRow>=nStart )
                HideOutline( nTab, false, nRowLevel, i, false, false );
        }

        rDoc.SetDrawPageSize(nTab);
        rDoc.UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top );

        rDocShell.SetDocumentModified();
        bDone = true;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    return bDone;
}

void ScOutlineDocFunc::ShowOutline( SCTAB nTab, bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    bool bRecord, bool bPaint )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
    ScOutlineArray& rArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = rArray.GetEntry( nLevel, nEntry );
    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd   = pEntry->GetEnd();

    // TODO undo can mess things up when another view is editing a cell in the range of group entry
    // this is a temporarily workaround
    if ( !comphelper::LibreOfficeKit::isActive() && bRecord )
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        if (bColumns)
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true );
            rDoc.CopyToDocument(static_cast<SCCOL>(nStart), 0, nTab,
                                static_cast<SCCOL>(nEnd), MAXROW, nTab, InsertDeleteFlags::NONE, false,
                                *pUndoDoc);
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument(0, nStart, nTab, MAXCOL, nEnd, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDoOutline>( &rDocShell,
                                    nStart, nEnd, nTab, std::move(pUndoDoc),  //! calc start and end
                                    bColumns, nLevel, nEntry, true ) );
    }

    pEntry->SetHidden(false);
    SCCOLROW i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            rDoc.ShowCol( static_cast<SCCOL>(i), nTab, true );
        else
        {
            // show several rows together, don't show filtered rows
            SCROW nFilterEnd = i;
            bool bFiltered = rDoc.RowFiltered( i, nTab, nullptr, &nFilterEnd );
            nFilterEnd = std::min( nEnd, nFilterEnd );
            if ( !bFiltered )
                rDoc.ShowRows( i, nFilterEnd, nTab, true );
            i = nFilterEnd;
        }
    }

    ScSubOutlineIterator aIter( &rArray, nLevel, nEntry );
    while ((pEntry=aIter.GetNext()) != nullptr)
    {
        if ( pEntry->IsHidden() )
        {
            SCCOLROW nSubStart = pEntry->GetStart();
            SCCOLROW nSubEnd   = pEntry->GetEnd();
            if ( bColumns )
                for ( i = nSubStart; i <= nSubEnd; i++ )
                    rDoc.ShowCol( static_cast<SCCOL>(i), nTab, false );
            else
                rDoc.ShowRows( nSubStart, nSubEnd, nTab, false );
        }
    }

    rArray.SetVisibleBelow( nLevel, nEntry, true, true );

    rDoc.SetDrawPageSize(nTab);
    rDoc.InvalidatePageBreaks(nTab);
    rDoc.UpdatePageBreaks( nTab );

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if ( pViewSh )
        pViewSh->OnLOKShowHideColRow(bColumns, nStart - 1);

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
}

bool ScOutlineDocFunc::HideOutline( SCTAB nTab, bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    bool bRecord, bool bPaint )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
    ScOutlineArray& rArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = rArray.GetEntry( nLevel, nEntry );
    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd   = pEntry->GetEnd();


    if (ScTabViewShell::isAnyEditViewInRange(bColumns, nStart, nEnd))
        return false;

    // TODO undo can mess things up when another view is editing a cell in the range of group entry
    // this is a temporarily workaround
    if ( !comphelper::LibreOfficeKit::isActive() &&  bRecord )
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        if (bColumns)
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true );
            rDoc.CopyToDocument(static_cast<SCCOL>(nStart), 0, nTab,
                                static_cast<SCCOL>(nEnd), MAXROW, nTab, InsertDeleteFlags::NONE, false,
                                *pUndoDoc);
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument(0, nStart, nTab, MAXCOL, nEnd, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDoOutline>( &rDocShell,
                                    nStart, nEnd, nTab, std::move(pUndoDoc),
                                    bColumns, nLevel, nEntry, false ) );
    }

    pEntry->SetHidden(true);
    SCCOLROW i;
    if ( bColumns )
        for ( i = nStart; i <= nEnd; i++ )
            rDoc.ShowCol( static_cast<SCCOL>(i), nTab, false );
    else
        rDoc.ShowRows( nStart, nEnd, nTab, false );

    rArray.SetVisibleBelow( nLevel, nEntry, false );

    rDoc.SetDrawPageSize(nTab);
    rDoc.InvalidatePageBreaks(nTab);
    rDoc.UpdatePageBreaks( nTab );

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if ( pViewSh )
        pViewSh->OnLOKShowHideColRow(bColumns, nStart - 1);

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );


    return true;        //! always ???
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
