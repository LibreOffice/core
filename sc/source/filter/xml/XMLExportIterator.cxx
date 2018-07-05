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

#include "XMLExportIterator.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <dociter.hxx>
#include <convuno.hxx>
#include "xmlexprt.hxx"
#include "XMLExportSharedData.hxx"
#include "XMLStylesExportHelper.hxx"
#include <document.hxx>

#include <algorithm>

using namespace ::com::sun::star;

ScMyIteratorBase::ScMyIteratorBase()
{
}

ScMyIteratorBase::~ScMyIteratorBase()
{
}

void ScMyIteratorBase::UpdateAddress( ScAddress& rCellAddress )
{
    ScAddress aNewAddr( rCellAddress );
    if( GetFirstAddress( aNewAddr ) )
    {
        if( ( aNewAddr.Tab() == rCellAddress.Tab() ) &&
            ( ( aNewAddr.Row() < rCellAddress.Row() ) ||
            ( ( aNewAddr.Row() == rCellAddress.Row() ) && ( aNewAddr.Col() < rCellAddress.Col() ) ) ) )
            rCellAddress = aNewAddr;
    }
}

inline bool ScMyShape::operator<(const ScMyShape& aShape) const
{
    return aAddress.lessThanByRow( aShape.aAddress );
}

ScMyShapesContainer::ScMyShapesContainer()
    : aShapeList()
{
}

ScMyShapesContainer::~ScMyShapesContainer()
{
}

void ScMyShapesContainer::AddNewShape( const ScMyShape& aShape )
{
    aShapeList.push_back(aShape);
}

bool ScMyShapesContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aShapeList.empty() )
    {
        rCellAddress = aShapeList.begin()->aAddress;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyShapesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aShapeList.clear();

    ScMyShapeList::iterator aItr(aShapeList.begin());
    ScMyShapeList::iterator aEndItr(aShapeList.end());
    while( (aItr != aEndItr) && (aItr->aAddress == rMyCell.maCellAddress) )
    {
        rMyCell.aShapeList.push_back(*aItr);
        aItr = aShapeList.erase(aItr);
    }
    rMyCell.bHasShape = !rMyCell.aShapeList.empty();
}

void ScMyShapesContainer::SkipTable(SCTAB nSkip)
{
    ScMyShapeList::iterator aItr = aShapeList.begin();
    while( ( aItr != aShapeList.end() ) && ( aItr->aAddress.Tab() == nSkip ) )
        aItr = aShapeList.erase(aItr);
}

void ScMyShapesContainer::Sort()
{
    aShapeList.sort();
}

inline bool ScMyNoteShape::operator<(const ScMyNoteShape& aNote) const
{
    return aPos.lessThanByRow( aNote.aPos );
}

ScMyNoteShapesContainer::ScMyNoteShapesContainer()
    : aNoteShapeList()
{
}

ScMyNoteShapesContainer::~ScMyNoteShapesContainer()
{
}

void ScMyNoteShapesContainer::AddNewNote( const ScMyNoteShape& aNote )
{
    aNoteShapeList.push_back(aNote);
}

bool ScMyNoteShapesContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable = rCellAddress.Tab();
    if( !aNoteShapeList.empty() )
    {
        rCellAddress = aNoteShapeList.begin()->aPos;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyNoteShapesContainer::SetCellData( ScMyCell& rMyCell )
{
    ScMyNoteShapeList::iterator aItr = aNoteShapeList.begin();
    while( (aItr != aNoteShapeList.end()) && (aItr->aPos == rMyCell.maCellAddress) )
    {
        aItr = aNoteShapeList.erase(aItr);
    }
}

void ScMyNoteShapesContainer::SkipTable(SCTAB nSkip)
{
    ScMyNoteShapeList::iterator aItr = aNoteShapeList.begin();
    while( (aItr != aNoteShapeList.end() ) && ( aItr->aPos.Tab() == nSkip ) )
        aItr = aNoteShapeList.erase(aItr);
}

void ScMyNoteShapesContainer::Sort()
{
    aNoteShapeList.sort();
}

inline bool ScMyMergedRange::operator<(const ScMyMergedRange& aRange) const
{
    return aCellRange.aStart.lessThanByRow( aRange.aCellRange.aStart );
}

ScMyMergedRangesContainer::ScMyMergedRangesContainer()
    : aRangeList()
{
}

ScMyMergedRangesContainer::~ScMyMergedRangesContainer()
{
}

void ScMyMergedRangesContainer::AddRange(const ScRange& rMergedRange)
{
    SCROW nStartRow( rMergedRange.aStart.Row() );
    SCROW nEndRow( rMergedRange.aEnd.Row() );

    ScMyMergedRange aRange;
    aRange.bIsFirst = true;

    aRange.aCellRange = rMergedRange;

    aRange.aCellRange.aEnd.SetRow( nStartRow );
    aRange.nRows = nEndRow - nStartRow + 1;
    aRangeList.push_back( aRange );

    aRange.bIsFirst = false;
    aRange.nRows = 0;
    for( SCROW nRow = nStartRow + 1; nRow <= nEndRow; ++nRow )
    {
        aRange.aCellRange.aStart.SetRow( nRow );
        aRange.aCellRange.aEnd.SetRow( nRow );
        aRangeList.push_back(aRange);
    }
}

bool ScMyMergedRangesContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aRangeList.empty() )
    {
        rCellAddress = aRangeList.begin()->aCellRange.aStart;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyMergedRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bIsMergedBase = rMyCell.bIsCovered = false;
    ScMyMergedRangeList::iterator aItr(aRangeList.begin());
    if( aItr != aRangeList.end() )
    {
        if( aItr->aCellRange.aStart == rMyCell.aCellAddress )
        {
            rMyCell.aMergeRange = aItr->aCellRange;
            if (aItr->bIsFirst)
                rMyCell.aMergeRange.aEnd.SetRow( rMyCell.aMergeRange.aStart.Row() + aItr->nRows - 1 );
            rMyCell.bIsMergedBase = aItr->bIsFirst;
            rMyCell.bIsCovered = !aItr->bIsFirst;
            if( aItr->aCellRange.aStart.Col() < aItr->aCellRange.aEnd.Col() )
            {
                aItr->aCellRange.aStart.IncCol( 1 );
                aItr->bIsFirst = false;
            }
            else
                aRangeList.erase(aItr);
        }
    }
}

void ScMyMergedRangesContainer::SkipTable(SCTAB nSkip)
{
    ScMyMergedRangeList::iterator aItr = aRangeList.begin();
    while( ( aItr != aRangeList.end() ) && ( aItr->aCellRange.aStart.Tab() == nSkip ) )
        aItr = aRangeList.erase(aItr);
}

void ScMyMergedRangesContainer::Sort()
{
    aRangeList.sort();
}

bool ScMyAreaLink::Compare( const ScMyAreaLink& rAreaLink ) const
{
    return  (GetRowCount() == rAreaLink.GetRowCount()) &&
            (sFilter == rAreaLink.sFilter) &&
            (sFilterOptions == rAreaLink.sFilterOptions) &&
            (sURL == rAreaLink.sURL) &&
            (sSourceStr == rAreaLink.sSourceStr);
}

inline bool ScMyAreaLink::operator<(const ScMyAreaLink& rAreaLink ) const
{
    return aDestRange.aStart.lessThanByRow( rAreaLink.aDestRange.aStart );
}

ScMyAreaLinksContainer::ScMyAreaLinksContainer() :
    aAreaLinkList()
{
}

ScMyAreaLinksContainer::~ScMyAreaLinksContainer()
{
}

bool ScMyAreaLinksContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aAreaLinkList.empty() )
    {
        rCellAddress = aAreaLinkList.begin()->aDestRange.aStart;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyAreaLinksContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasAreaLink = false;
    ScMyAreaLinkList::iterator aItr(aAreaLinkList.begin());
    if( aItr != aAreaLinkList.end() )
    {
        if( aItr->aDestRange.aStart == rMyCell.aCellAddress )
        {
            rMyCell.bHasAreaLink = true;
            rMyCell.aAreaLink = *aItr;
            aItr = aAreaLinkList.erase( aItr );
            bool bFound = true;
            while (aItr != aAreaLinkList.end() && bFound)
            {
                if ( aItr->aDestRange.aStart == rMyCell.aCellAddress )
                {
                    OSL_FAIL("more than one linked range on one cell");
                    aItr = aAreaLinkList.erase( aItr );
                }
                else
                    bFound = false;
            }
        }
    }
}

void ScMyAreaLinksContainer::SkipTable(SCTAB nSkip)
{
    ScMyAreaLinkList::iterator aItr = aAreaLinkList.begin();
    while( ( aItr != aAreaLinkList.end() ) && ( aItr->aDestRange.aStart.Tab() == nSkip ) )
        aItr = aAreaLinkList.erase(aItr);
}

void ScMyAreaLinksContainer::Sort()
{
    aAreaLinkList.sort();
}

ScMyEmptyDatabaseRangesContainer::ScMyEmptyDatabaseRangesContainer()
    : aDatabaseList()
{
}

ScMyEmptyDatabaseRangesContainer::~ScMyEmptyDatabaseRangesContainer()
{
}

void ScMyEmptyDatabaseRangesContainer::AddNewEmptyDatabaseRange(const table::CellRangeAddress& aCellRange)
{
    SCROW nStartRow(aCellRange.StartRow);
    SCROW nEndRow(aCellRange.EndRow);
    ScRange aRange( aCellRange.StartColumn, aCellRange.StartRow, aCellRange.Sheet,
                      aCellRange.EndColumn, aCellRange.EndRow, aCellRange.Sheet );
    for( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
    {
        aRange.aStart.SetRow( nRow );
        aRange.aEnd.SetRow( nRow );
        aDatabaseList.push_back( aRange );
    }
}

bool ScMyEmptyDatabaseRangesContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aDatabaseList.empty() )
    {
        rCellAddress = aDatabaseList.begin()->aStart;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyEmptyDatabaseRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasEmptyDatabase = false;
    ScMyEmptyDatabaseRangeList::iterator aItr(aDatabaseList.begin());
    if( aItr != aDatabaseList.end() )
    {
        if( aItr->aStart == rMyCell.aCellAddress )
        {
            rMyCell.bHasEmptyDatabase = true;
            if( aItr->aStart.Col() < aItr->aEnd.Col() )
                aItr->aStart.SetCol( aItr->aStart.Col() + 1 );
            else
                aDatabaseList.erase(aItr);
        }
    }
}

void ScMyEmptyDatabaseRangesContainer::SkipTable(SCTAB nSkip)
{
    ScMyEmptyDatabaseRangeList::iterator aItr = aDatabaseList.begin();
    while( ( aItr != aDatabaseList.end() ) && ( aItr->aStart.Tab() == nSkip ) )
        aItr = aDatabaseList.erase(aItr);
}

void ScMyEmptyDatabaseRangesContainer::Sort()
{
    aDatabaseList.sort();
}

inline bool ScMyDetectiveObj::operator<( const ScMyDetectiveObj& rDetObj) const
{
    return aPosition.lessThanByRow( rDetObj.aPosition );
}

ScMyDetectiveObjContainer::ScMyDetectiveObjContainer() :
    aDetectiveObjList()
{
}

ScMyDetectiveObjContainer::~ScMyDetectiveObjContainer()
{
}

void ScMyDetectiveObjContainer::AddObject( ScDetectiveObjType eObjType, const SCTAB nSheet,
                                            const ScAddress& rPosition, const ScRange& rSourceRange,
                                            bool bHasError )
{
    if( (eObjType == SC_DETOBJ_ARROW) ||
        (eObjType == SC_DETOBJ_FROMOTHERTAB) ||
        (eObjType == SC_DETOBJ_TOOTHERTAB) ||
        (eObjType == SC_DETOBJ_CIRCLE) )
    {
        ScMyDetectiveObj aDetObj;
        aDetObj.eObjType = eObjType;
        if( eObjType == SC_DETOBJ_TOOTHERTAB )
            aDetObj.aPosition = rSourceRange.aStart;
        else
            aDetObj.aPosition = rPosition;
        aDetObj.aSourceRange = rSourceRange;

        // #111064#; take the sheet where the object is found and not the sheet given in the ranges, because they are not always true
        if (eObjType != SC_DETOBJ_FROMOTHERTAB)
        {
            // if the ObjType == SC_DETOBJ_FROMOTHERTAB then the SourceRange is not used and so it has not to be tested and changed
            OSL_ENSURE(aDetObj.aPosition.Tab() == aDetObj.aSourceRange.aStart.Tab(), "It seems to be possible to have different sheets");
            aDetObj.aSourceRange.aStart.SetTab( nSheet );
            aDetObj.aSourceRange.aEnd.SetTab( nSheet );
        }
        aDetObj.aPosition.SetTab( nSheet );

        aDetObj.bHasError = bHasError;
        aDetectiveObjList.push_back( aDetObj );
    }
}

bool ScMyDetectiveObjContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aDetectiveObjList.empty() )
    {
        rCellAddress = aDetectiveObjList.begin()->aPosition;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyDetectiveObjContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveObjVec.clear();
    ScMyDetectiveObjList::iterator aItr(aDetectiveObjList.begin());
    ScMyDetectiveObjList::iterator aEndItr(aDetectiveObjList.end());
    while( (aItr != aEndItr) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveObjVec.push_back( *aItr );
        aItr = aDetectiveObjList.erase( aItr );
    }
    rMyCell.bHasDetectiveObj = (rMyCell.aDetectiveObjVec.size() != 0);
}

void ScMyDetectiveObjContainer::SkipTable(SCTAB nSkip)
{
    ScMyDetectiveObjList::iterator aItr = aDetectiveObjList.begin();
    while( ( aItr != aDetectiveObjList.end() ) && ( aItr->aPosition.Tab() == nSkip ) )
        aItr = aDetectiveObjList.erase(aItr);
}

void ScMyDetectiveObjContainer::Sort()
{
    aDetectiveObjList.sort();
}

inline bool ScMyDetectiveOp::operator<( const ScMyDetectiveOp& rDetOp) const
{
    return aPosition.lessThanByRow( rDetOp.aPosition );
}

ScMyDetectiveOpContainer::ScMyDetectiveOpContainer() :
    aDetectiveOpList()
{
}

ScMyDetectiveOpContainer::~ScMyDetectiveOpContainer()
{
}

void ScMyDetectiveOpContainer::AddOperation( ScDetOpType eOpType, const ScAddress& rPosition, sal_uInt32 nIndex )
{
    ScMyDetectiveOp aDetOp;
    aDetOp.eOpType = eOpType;
    aDetOp.aPosition = rPosition;
    aDetOp.nIndex = nIndex;
    aDetectiveOpList.push_back( aDetOp );
}

bool ScMyDetectiveOpContainer::GetFirstAddress( ScAddress& rCellAddress )
{
    SCTAB nTable( rCellAddress.Tab() );
    if( !aDetectiveOpList.empty() )
    {
        rCellAddress = aDetectiveOpList.begin()->aPosition;
        return ( nTable == rCellAddress.Tab() );
    }
    return false;
}

void ScMyDetectiveOpContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveOpVec.clear();
    ScMyDetectiveOpList::iterator aItr(aDetectiveOpList.begin());
    ScMyDetectiveOpList::iterator aEndItr(aDetectiveOpList.end());
    while( (aItr != aEndItr) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveOpVec.push_back( *aItr );
        aItr = aDetectiveOpList.erase( aItr );
    }
    rMyCell.bHasDetectiveOp = (rMyCell.aDetectiveOpVec.size() != 0);
}

void ScMyDetectiveOpContainer::SkipTable(SCTAB nSkip)
{
    ScMyDetectiveOpList::iterator aItr = aDetectiveOpList.begin();
    while( (aItr != aDetectiveOpList.end()) && (aItr->aPosition.Tab() == nSkip) )
        aItr = aDetectiveOpList.erase(aItr);
}

void ScMyDetectiveOpContainer::Sort()
{
    aDetectiveOpList.sort();
}

ScMyCell::ScMyCell() :
    aShapeList(),
    aDetectiveObjVec(),
    pNote(nullptr),
    nValidationIndex(-1),
    nStyleIndex(-1),
    nNumberFormat(-1),
    nType(table::CellContentType_EMPTY),
    bIsAutoStyle( false ),
    bHasShape( false ),
    bIsMergedBase( false ),
    bIsCovered( false ),
    bHasAreaLink( false ),
    bHasEmptyDatabase( false ),
    bHasDetectiveObj( false ),
    bHasDetectiveOp( false ),
    bIsMatrixBase( false ),
    bIsMatrixCovered( false ),
    bHasAnnotation( false )
{
}

ScMyNotEmptyCellsIterator::ScMyNotEmptyCellsIterator(ScXMLExport& rTempXMLExport)
    : pShapes(nullptr),
    pNoteShapes(nullptr),
    pEmptyDatabaseRanges(nullptr),
    pMergedRanges(nullptr),
    pAreaLinks(nullptr),
    pDetectiveObj(nullptr),
    pDetectiveOp(nullptr),
    rExport(rTempXMLExport),
    nCellCol(0),
    nCellRow(0),
    nCurrentTable(SCTAB_MAX)
{
}

ScMyNotEmptyCellsIterator::~ScMyNotEmptyCellsIterator()
{
    Clear();
}

void ScMyNotEmptyCellsIterator::Clear()
{
    mpCellItr.reset();
    pShapes = nullptr;
    pNoteShapes = nullptr;
    pMergedRanges = nullptr;
    pAreaLinks = nullptr;
    pEmptyDatabaseRanges = nullptr;
    pDetectiveObj = nullptr;
    pDetectiveOp = nullptr;
    nCurrentTable = SCTAB_MAX;
}

void ScMyNotEmptyCellsIterator::UpdateAddress( ScAddress& rAddress )
{
    if (mpCellItr->GetPos(nCellCol, nCellRow))
    {
        rAddress.SetCol( nCellCol );
        rAddress.SetRow( nCellRow );
    }
}

void ScMyNotEmptyCellsIterator::SetCellData( ScMyCell& rMyCell, const ScAddress& rAddress )
{
    rMyCell.maBaseCell.clear();
    rMyCell.aCellAddress = rAddress;
    rMyCell.maCellAddress = rMyCell.aCellAddress;

    if( ( nCellCol == rAddress.Col() ) && ( nCellRow == rAddress.Row() ) )
    {
        const ScRefCellValue* pCell = mpCellItr->GetNext(nCellCol, nCellRow);
        if (pCell)
            rMyCell.maBaseCell = *pCell;
    }

    rMyCell.bIsMatrixCovered = false;
    rMyCell.bIsMatrixBase = false;

    switch (rMyCell.maBaseCell.meType)
    {
        case CELLTYPE_VALUE:
            rMyCell.nType = table::CellContentType_VALUE;
            break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            rMyCell.nType = table::CellContentType_TEXT;
            break;
        case CELLTYPE_FORMULA:
            rMyCell.nType = table::CellContentType_FORMULA;
            break;
        default:
            rMyCell.nType = table::CellContentType_EMPTY;
    }

    if (rMyCell.maBaseCell.meType == CELLTYPE_FORMULA)
    {
        bool bIsMatrixBase = false;
        if (rExport.IsMatrix(rMyCell.maCellAddress, rMyCell.aMatrixRange, bIsMatrixBase))
        {
            rMyCell.bIsMatrixBase = bIsMatrixBase;
            rMyCell.bIsMatrixCovered = !bIsMatrixBase;
        }
    }
}

void ScMyNotEmptyCellsIterator::HasAnnotation(ScMyCell& aCell)
{
    aCell.bHasAnnotation = false;
    ScPostIt* pNote = rExport.GetDocument()->GetNote(aCell.maCellAddress);

    if(pNote)
    {
        aCell.bHasAnnotation = true;
        aCell.pNote = pNote;
    }
}

void ScMyNotEmptyCellsIterator::SetCurrentTable(const SCTAB nTable,
    const uno::Reference<sheet::XSpreadsheet>& rxTable)
{
    aLastAddress.SetRow( 0 );
    aLastAddress.SetCol( 0 );
    aLastAddress.SetTab( nTable );
    if (nCurrentTable != nTable)
    {
        nCurrentTable = nTable;

        mpCellItr.reset(
            new ScHorizontalCellIterator(
                rExport.GetDocument(), nCurrentTable, 0, 0,
                static_cast<SCCOL>(rExport.GetSharedData()->GetLastColumn(nCurrentTable)),
                static_cast<SCROW>(rExport.GetSharedData()->GetLastRow(nCurrentTable))));

        xTable.set(rxTable);
        xCellRange.set(xTable, uno::UNO_QUERY);
    }
}

void ScMyNotEmptyCellsIterator::SkipTable(SCTAB nSkip)
{
    // Skip entries for a sheet that is copied instead of saving normally.
    // Cells are handled separately in SetCurrentTable.

    if( pShapes )
        pShapes->SkipTable(nSkip);
    if( pNoteShapes )
        pNoteShapes->SkipTable(nSkip);
    if( pEmptyDatabaseRanges )
        pEmptyDatabaseRanges->SkipTable(nSkip);
    if( pMergedRanges )
        pMergedRanges->SkipTable(nSkip);
    if( pAreaLinks )
        pAreaLinks->SkipTable(nSkip);
    if( pDetectiveObj )
        pDetectiveObj->SkipTable(nSkip);
    if( pDetectiveOp )
        pDetectiveOp->SkipTable(nSkip);
}

bool ScMyNotEmptyCellsIterator::GetNext(ScMyCell& aCell, ScFormatRangeStyles* pCellStyles)
{
    ScAddress  aAddress( MAXCOL + 1, MAXROW + 1, nCurrentTable );

    UpdateAddress( aAddress );

    if( pShapes )
        pShapes->UpdateAddress( aAddress );
    if( pNoteShapes )
        pNoteShapes->UpdateAddress( aAddress );
    if( pEmptyDatabaseRanges )
        pEmptyDatabaseRanges->UpdateAddress( aAddress );
    if( pMergedRanges )
        pMergedRanges->UpdateAddress( aAddress );
    if( pAreaLinks )
        pAreaLinks->UpdateAddress( aAddress );
    if( pDetectiveObj )
        pDetectiveObj->UpdateAddress( aAddress );
    if( pDetectiveOp )
        pDetectiveOp->UpdateAddress( aAddress );

    bool bFoundCell( ( aAddress.Col() <= MAXCOL ) && ( aAddress.Row() <= MAXROW ) );
    if( bFoundCell )
    {
        SetCellData( aCell, aAddress );
        if( pShapes )
            pShapes->SetCellData( aCell );
        if( pNoteShapes )
            pNoteShapes->SetCellData( aCell );
        if( pEmptyDatabaseRanges )
            pEmptyDatabaseRanges->SetCellData( aCell );
        if( pMergedRanges )
            pMergedRanges->SetCellData( aCell );
        if( pAreaLinks )
            pAreaLinks->SetCellData( aCell );
        if( pDetectiveObj )
            pDetectiveObj->SetCellData( aCell );
        if( pDetectiveOp )
            pDetectiveOp->SetCellData( aCell );

        HasAnnotation( aCell );
        bool bIsAutoStyle;
        // Ranges before the previous cell are not needed by ExportFormatRanges anymore and can be removed
        SCROW nRemoveBeforeRow = aLastAddress.Row();
        aCell.nStyleIndex = pCellStyles->GetStyleNameIndex(aCell.maCellAddress.Tab(),
            aCell.maCellAddress.Col(), aCell.maCellAddress.Row(),
            bIsAutoStyle, aCell.nValidationIndex, aCell.nNumberFormat, nRemoveBeforeRow);
        aLastAddress = aCell.aCellAddress;
        aCell.bIsAutoStyle = bIsAutoStyle;

        //#102799#; if the cell is in a DatabaseRange which should saved empty, the cell should have the type empty
        if (aCell.bHasEmptyDatabase)
            aCell.nType = table::CellContentType_EMPTY;
    }
    return bFoundCell;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
