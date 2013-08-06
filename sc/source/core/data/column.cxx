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

#include "column.hxx"
#include "scitems.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "compiler.hxx"
#include "brdcst.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // for Notes in Sort/Swap
#include "postit.hxx"
#include "globalnames.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"
#include "cellform.hxx"
#include "clipcontext.hxx"
#include "types.hxx"
#include "editutil.hxx"
#include "mtvcellfunc.hxx"
#include "columnspanset.hxx"
#include "scopetools.hxx"
#include "sharedformula.hxx"
#include "refupdatecontext.hxx"

#include <svl/poolcach.hxx>
#include <svl/zforlist.hxx>
#include <editeng/scripttypeitem.hxx>
#include "editeng/fieldupdater.hxx"

#include <cstring>
#include <map>
#include <cstdio>
#include <boost/scoped_ptr.hpp>

using ::editeng::SvxBorderLine;
using namespace formula;

namespace {

inline bool IsAmbiguousScriptNonZero( sal_uInt8 nScript )
{
    //! move to a header file
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX &&
             nScript != 0 );
}

}

ScNeededSizeOptions::ScNeededSizeOptions() :
    pPattern(NULL), bFormula(false), bSkipMerged(true), bGetFont(true), bTotalSize(false)
{
}

ScColumn::ScColumn() :
    maCellTextAttrs(MAXROWCOUNT),
    maBroadcasters(MAXROWCOUNT),
    maCells(MAXROWCOUNT),
    nCol( 0 ),
    pAttrArray( NULL ),
    pDocument( NULL ),
    mbDirtyGroups(true)
{
}


ScColumn::~ScColumn()
{
    FreeAll();
    delete pAttrArray;
}


void ScColumn::Init(SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc)
{
    nCol = nNewCol;
    nTab = nNewTab;
    pDocument = pDoc;
    pAttrArray = new ScAttrArray( nCol, nTab, pDocument );
}


SCsROW ScColumn::GetNextUnprotected( SCROW nRow, bool bUp ) const
{
    return pAttrArray->GetNextUnprotected(nRow, bUp);
}


sal_uInt16 ScColumn::GetBlockMatrixEdges( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const
{
    using namespace sc;

    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return 0;

    ScAddress aOrigin(ScAddress::INITIALIZE_INVALID);

    if (nRow1 == nRow2)
    {
        std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow1);
        if (aPos.first->type != sc::element_type_formula)
            return 0;

        const ScFormulaCell* pCell = sc::formula_block::at(*aPos.first->data, aPos.second);
        if (!pCell->GetMatrixFlag())
            return 0;

        return pCell->GetMatrixEdge(aOrigin);
    }

    bool bOpen = false;
    sal_uInt16 nEdges = 0;

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow1);
    sc::CellStoreType::const_iterator it = aPos.first;
    size_t nOffset = aPos.second;
    SCROW nRow = nRow1;
    for (;it != maCells.end() && nRow <= nRow2; ++it, nOffset = 0)
    {
        if (it->type != sc::element_type_formula)
        {
            // Skip this block.
            nRow += it->size - nOffset;
            continue;
        }

        size_t nRowsToRead = nRow2 - nRow + 1;
        size_t nEnd = std::min(it->size, nRowsToRead);
        sc::formula_block::const_iterator itCell = sc::formula_block::begin(*it->data);
        std::advance(itCell, nOffset);
        for (size_t i = nOffset; i < nEnd; ++itCell, ++i)
        {
            // Loop inside the formula block.
            const ScFormulaCell* pCell = *itCell;
            if (!pCell->GetMatrixFlag())
                continue;

            nEdges = pCell->GetMatrixEdge(aOrigin);
            if (!nEdges)
                continue;

            if (nEdges & MatrixEdgeTop)
                bOpen = true;       // top edge opens, keep on looking
            else if (!bOpen)
                return nEdges | MatrixEdgeOpen; // there's something that wasn't opened
            else if (nEdges & MatrixEdgeInside)
                return nEdges;      // inside
            // (nMask & 16 and  (4 and not 16)) or
            // (nMask & 4  and (16 and not 4))
            if (((nMask & MatrixEdgeRight) && (nEdges & MatrixEdgeLeft)  && !(nEdges & MatrixEdgeRight)) ||
                ((nMask & MatrixEdgeLeft)  && (nEdges & MatrixEdgeRight) && !(nEdges & MatrixEdgeLeft)))
                return nEdges;      // only left/right edge

            if (nEdges & MatrixEdgeBottom)
                bOpen = false;      // bottom edge closes
        }

        nRow += nEnd;
    }
    if (bOpen)
        nEdges |= MatrixEdgeOpen; // not closed, matrix continues

    return nEdges;
}


bool ScColumn::HasSelectionMatrixFragment(const ScMarkData& rMark) const
{
    using namespace sc;

    if (!rMark.IsMultiMarked())
        return false;

    ScAddress aOrigin(ScAddress::INITIALIZE_INVALID);
    ScAddress aCurOrigin = aOrigin;

    bool bOpen = false;
    ScRangeList aRanges = rMark.GetMarkedRanges();
    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        const ScRange& r = *aRanges[i];
        if (nTab < r.aStart.Tab() || r.aEnd.Tab() < nTab)
            continue;

        if (nCol < r.aStart.Col() || r.aEnd.Col() < nCol)
            continue;

        SCROW nTop = r.aStart.Row(), nBottom = r.aEnd.Row();
        SCROW nRow = nTop;
        std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
        sc::CellStoreType::const_iterator it = aPos.first;
        size_t nOffset = aPos.second;

        for (;it != maCells.end() && nRow <= nBottom; ++it, nOffset = 0)
        {
            if (it->type != sc::element_type_formula)
            {
                // Skip this block.
                nRow += it->size - nOffset;
                continue;
            }

            // This is a formula cell block.
            size_t nRowsToRead = nBottom - nRow + 1;
            size_t nEnd = std::min(it->size, nRowsToRead);
            sc::formula_block::const_iterator itCell = sc::formula_block::begin(*it->data);
            std::advance(itCell, nOffset);
            for (size_t j = nOffset; j < nEnd; ++itCell, ++j)
            {
                // Loop inside the formula block.
                const ScFormulaCell* pCell = *itCell;
                if (!pCell->GetMatrixFlag())
                    // cell is not a part of a matrix.
                    continue;

                sal_uInt16 nEdges = pCell->GetMatrixEdge(aOrigin);
                if (!nEdges)
                    continue;

                bool bFound = false;

                if (nEdges & MatrixEdgeTop)
                    bOpen = true;   // top edge opens, keep on looking
                else if (!bOpen)
                    return true;    // there's something that wasn't opened
                else if (nEdges & MatrixEdgeInside)
                    bFound = true;  // inside, all selected?

                if ((((nEdges & MatrixEdgeLeft) | MatrixEdgeRight) ^ ((nEdges & MatrixEdgeRight) | MatrixEdgeLeft)))
                    // either left or right, but not both.
                    bFound = true;  // only left/right edge, all selected?

                if (nEdges & MatrixEdgeBottom)
                    bOpen = false;  // bottom edge closes

                if (bFound)
                {
                    // Check if the matrix is inside the selection in its entirety.
                    //
                    // TODO: It's more efficient to skip the matrix range if
                    // it's within selection, to avoid checking it again and
                    // again.

                    if (aCurOrigin != aOrigin)
                    {   // new matrix to check?
                        aCurOrigin = aOrigin;
                        const ScFormulaCell* pFCell;
                        if (pCell->GetMatrixFlag() == MM_REFERENCE)
                            pFCell = pDocument->GetFormulaCell(aOrigin);
                        else
                            pFCell = pCell;

                        SCCOL nC;
                        SCROW nR;
                        pFCell->GetMatColsRows(nC, nR);
                        ScRange aRange(aOrigin, ScAddress(aOrigin.Col()+nC-1, aOrigin.Row()+nR-1, aOrigin.Tab()));
                        if (rMark.IsAllMarked(aRange))
                            bFound = false;
                    }
                    else
                        bFound = false;     // done already
                }

                if (bFound)
                    return true;
            }

            nRow += nEnd;
        }
    }

    if (bOpen)
        return true;

    return false;
}


bool ScColumn::HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const
{
    return pAttrArray->HasAttrib( nRow1, nRow2, nMask );
}


bool ScColumn::HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const
{
    bool bFound = false;

    SCROW nTop;
    SCROW nBottom;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while (aMarkIter.Next( nTop, nBottom ) && !bFound)
        {
            if (pAttrArray->HasAttrib( nTop, nBottom, nMask ))
                bFound = true;
        }
    }

    return bFound;
}


bool ScColumn::ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                            SCCOL& rPaintCol, SCROW& rPaintRow,
                            bool bRefresh )
{
    return pAttrArray->ExtendMerge( nThisCol, nStartRow, nEndRow, rPaintCol, rPaintRow, bRefresh );
}


void ScColumn::MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, bool bDeep ) const
{
    SCROW nTop;
    SCROW nBottom;

    if ( rMark.IsMultiMarked() )
    {
        const ScMarkArray* pArray = rMark.GetArray() + nCol;
        if ( pArray->HasMarks() )
        {
            ScMarkArrayIter aMarkIter( pArray );
            while (aMarkIter.Next( nTop, nBottom ))
                pAttrArray->MergePatternArea( nTop, nBottom, rState, bDeep );
        }
    }
}


void ScColumn::MergePatternArea( ScMergePatternState& rState, SCROW nRow1, SCROW nRow2, bool bDeep ) const
{
    pAttrArray->MergePatternArea( nRow1, nRow2, rState, bDeep );
}


void ScColumn::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const
{
    pAttrArray->MergeBlockFrame( pLineOuter, pLineInner, rFlags, nStartRow, nEndRow, bLeft, nDistRight );
}


void ScColumn::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight )
{
    pAttrArray->ApplyBlockFrame( pLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight );
}


const ScPatternAttr* ScColumn::GetPattern( SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow );
}


const SfxPoolItem* ScColumn::GetAttr( SCROW nRow, sal_uInt16 nWhich ) const
{
    return &pAttrArray->GetPattern( nRow )->GetItemSet().Get(nWhich);
}


const ScPatternAttr* ScColumn::GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const
{
    ::std::map< const ScPatternAttr*, size_t > aAttrMap;
    const ScPatternAttr* pMaxPattern = 0;
    size_t nMaxCount = 0;

    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow );
    const ScPatternAttr* pPattern;
    SCROW nAttrRow1 = 0, nAttrRow2 = 0;

    while( (pPattern = aAttrIter.Next( nAttrRow1, nAttrRow2 )) != 0 )
    {
        size_t& rnCount = aAttrMap[ pPattern ];
        rnCount += (nAttrRow2 - nAttrRow1 + 1);
        if( rnCount > nMaxCount )
        {
            pMaxPattern = pPattern;
            nMaxCount = rnCount;
        }
    }

    return pMaxPattern;
}

sal_uInt32 ScColumn::GetNumberFormat( SCROW nStartRow, SCROW nEndRow ) const
{
    SCROW nPatStartRow, nPatEndRow;
    const ScPatternAttr* pPattern = pAttrArray->GetPatternRange(nPatStartRow, nPatEndRow, nStartRow);
    sal_uInt32 nFormat = pPattern->GetNumberFormat(pDocument->GetFormatTable());
    while (nEndRow > nPatEndRow)
    {
        nStartRow = nPatEndRow + 1;
        pPattern = pAttrArray->GetPatternRange(nPatStartRow, nPatEndRow, nStartRow);
        sal_uInt32 nTmpFormat = pPattern->GetNumberFormat(pDocument->GetFormatTable());
        if (nFormat != nTmpFormat)
            return 0;
    }
    return nFormat;
}


sal_uInt32 ScColumn::GetNumberFormat( SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetNumberFormat( pDocument->GetFormatTable() );
}


SCsROW ScColumn::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark, ScEditDataArray* pDataArray )
{
    SCROW nTop = 0;
    SCROW nBottom = 0;
    bool bFound = false;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
        {
            pAttrArray->ApplyCacheArea( nTop, nBottom, pCache, pDataArray );
            bFound = true;
        }
    }

    if (!bFound)
        return -1;
    else if (nTop==0 && nBottom==MAXROW)
        return 0;
    else
        return nBottom;
}


void ScColumn::ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark )
{
    SCROW nTop;
    SCROW nBottom;

    if ( pAttrArray && rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ChangeIndent(nTop, nBottom, bIncrement);
    }
}

void ScColumn::ClearSelectionItems( const sal_uInt16* pWhich,const ScMarkData& rMark )
{
    SCROW nTop;
    SCROW nBottom;

    if ( pAttrArray && rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ClearItems(nTop, nBottom, pWhich);
    }
}


void ScColumn::DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    SCROW nTop;
    SCROW nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            DeleteArea(nTop, nBottom, nDelFlag);
    }
}


void ScColumn::ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr )
{
    const SfxItemSet* pSet = &rPatAttr.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );

    const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );

    //  true = keep old content

    ScPatternAttr* pNewPattern = (ScPatternAttr*) &aCache.ApplyTo( *pPattern, true );
    ScDocumentPool::CheckRef( *pPattern );
    ScDocumentPool::CheckRef( *pNewPattern );

    if (pNewPattern != pPattern)
      pAttrArray->SetPattern( nRow, pNewPattern );
}


void ScColumn::ApplyPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr& rPatAttr,
                                 ScEditDataArray* pDataArray )
{
    const SfxItemSet* pSet = &rPatAttr.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );
    pAttrArray->ApplyCacheArea( nStartRow, nEndRow, &aCache, pDataArray );
}

bool ScColumn::SetAttrEntries(ScAttrEntry* pData, SCSIZE nSize)
{
    return pAttrArray->SetAttrEntries(pData, nSize);
}

void ScColumn::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScPatternAttr& rPattern, short nNewType )
{
    const SfxItemSet* pSet = &rPattern.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
    SCROW nEndRow = rRange.aEnd.Row();
    for ( SCROW nRow = rRange.aStart.Row(); nRow <= nEndRow; nRow++ )
    {
        SCROW nRow1, nRow2;
        const ScPatternAttr* pPattern = pAttrArray->GetPatternRange(
            nRow1, nRow2, nRow );
        sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
        short nOldType = pFormatter->GetType( nFormat );
        if ( nOldType == nNewType || pFormatter->IsCompatible( nOldType, nNewType ) )
            nRow = nRow2;
        else
        {
            SCROW nNewRow1 = std::max( nRow1, nRow );
            SCROW nNewRow2 = std::min( nRow2, nEndRow );
            pAttrArray->ApplyCacheArea( nNewRow1, nNewRow2, &aCache );
            nRow = nNewRow2;
        }
    }
}

void ScColumn::AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    pAttrArray->AddCondFormat( nStartRow, nEndRow, nIndex );
}

void ScColumn::RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    pAttrArray->RemoveCondFormat( nStartRow, nEndRow, nIndex );
}


void ScColumn::ApplyStyle( SCROW nRow, const ScStyleSheet& rStyle )
{
    const ScPatternAttr* pPattern = pAttrArray->GetPattern(nRow);
    ScPatternAttr* pNewPattern = new ScPatternAttr(*pPattern);
    if (pNewPattern)
    {
        pNewPattern->SetStyleSheet((ScStyleSheet*)&rStyle);
        pAttrArray->SetPattern(nRow, pNewPattern, true);
        delete pNewPattern;
    }
}


void ScColumn::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    pAttrArray->ApplyStyleArea(nStartRow, nEndRow, (ScStyleSheet*)&rStyle);
}


void ScColumn::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    SCROW nTop;
    SCROW nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ApplyStyleArea(nTop, nBottom, (ScStyleSheet*)&rStyle);
    }
}


void ScColumn::ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    SCROW nTop;
    SCROW nBottom;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ApplyLineStyleArea(nTop, nBottom, pLine, bColorOnly );
    }
}


const ScStyleSheet* ScColumn::GetStyle( SCROW nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetStyleSheet();
}


const ScStyleSheet* ScColumn::GetSelectionStyle( const ScMarkData& rMark, bool& rFound ) const
{
    rFound = false;
    if (!rMark.IsMultiMarked())
    {
        OSL_FAIL("No selection in ScColumn::GetSelectionStyle");
        return NULL;
    }

    bool bEqual = true;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
    SCROW nTop;
    SCROW nBottom;
    while (bEqual && aMarkIter.Next( nTop, nBottom ))
    {
        ScAttrIterator aAttrIter( pAttrArray, nTop, nBottom );
        SCROW nRow;
        SCROW nDummy;
        const ScPatternAttr* pPattern;
        while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ) != NULL)
        {
            pNewStyle = pPattern->GetStyleSheet();
            rFound = true;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = false;                                             // difference
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : NULL;
}


const ScStyleSheet* ScColumn::GetAreaStyle( bool& rFound, SCROW nRow1, SCROW nRow2 ) const
{
    rFound = false;

    bool bEqual = true;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    ScAttrIterator aAttrIter( pAttrArray, nRow1, nRow2 );
    SCROW nRow;
    SCROW nDummy;
    const ScPatternAttr* pPattern;
    while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ) != NULL)
    {
        pNewStyle = pPattern->GetStyleSheet();
        rFound = true;
        if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
            bEqual = false;                                             // difference
        pStyle = pNewStyle;
    }

    return bEqual ? pStyle : NULL;
}

void ScColumn::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset )
{
    pAttrArray->FindStyleSheet( pStyleSheet, rUsedRows, bReset );
}

bool ScColumn::IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const
{
    return pAttrArray->IsStyleSheetUsed( rStyle, bGatherAllStyles );
}


bool ScColumn::ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    return pAttrArray->ApplyFlags( nStartRow, nEndRow, nFlags );
}


bool ScColumn::RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    return pAttrArray->RemoveFlags( nStartRow, nEndRow, nFlags );
}


void ScColumn::ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich )
{
    pAttrArray->ClearItems( nStartRow, nEndRow, pWhich );
}


void ScColumn::SetPattern( SCROW nRow, const ScPatternAttr& rPatAttr, bool bPutToPool )
{
    pAttrArray->SetPattern( nRow, &rPatAttr, bPutToPool );
}


void ScColumn::SetPatternArea( SCROW nStartRow, SCROW nEndRow,
                                const ScPatternAttr& rPatAttr, bool bPutToPool )
{
    pAttrArray->SetPatternArea( nStartRow, nEndRow, &rPatAttr, bPutToPool );
}


void ScColumn::ApplyAttr( SCROW nRow, const SfxPoolItem& rAttr )
{
    //  in order to only create a new SetItem, we don't need SfxItemPoolCache.
    //! Warning: SfxItemPoolCache seems to create to many Refs for the new SetItem ??

    ScDocumentPool* pDocPool = pDocument->GetPool();

    const ScPatternAttr* pOldPattern = pAttrArray->GetPattern( nRow );
    ScPatternAttr* pTemp = new ScPatternAttr(*pOldPattern);
    pTemp->GetItemSet().Put(rAttr);
    const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pDocPool->Put( *pTemp );

    if ( pNewPattern != pOldPattern )
        pAttrArray->SetPattern( nRow, pNewPattern );
    else
        pDocPool->Remove( *pNewPattern );       // free up resources

    delete pTemp;
}

ScDocument& ScColumn::GetDoc()
{
    return *pDocument;
}

const ScDocument& ScColumn::GetDoc() const
{
    return *pDocument;
}

ScRefCellValue ScColumn::GetCellValue( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    if (aPos.first == maCells.end())
        return ScRefCellValue();

    return GetCellValue(aPos.first, aPos.second);
}

ScRefCellValue ScColumn::GetCellValue( const sc::CellStoreType::const_iterator& itPos, size_t nOffset ) const
{
    ScRefCellValue aVal; // Defaults to empty cell.
    switch (itPos->type)
    {
        case sc::element_type_numeric:
            // Numeric cell
            aVal.mfValue = sc::numeric_block::at(*itPos->data, nOffset);
            aVal.meType = CELLTYPE_VALUE;
        break;
        case sc::element_type_string:
            // String cell
            aVal.mpString = &sc::string_block::at(*itPos->data, nOffset);
            aVal.meType = CELLTYPE_STRING;
        break;
        case sc::element_type_edittext:
            // Edit cell
            aVal.mpEditText = sc::edittext_block::at(*itPos->data, nOffset);
            aVal.meType = CELLTYPE_EDIT;
        break;
        case sc::element_type_formula:
            // Formula cell
            aVal.mpFormula = sc::formula_block::at(*itPos->data, nOffset);
            aVal.meType = CELLTYPE_FORMULA;
        break;
        default:
            ;
    }

    return aVal;
}

namespace {

ScFormulaCell* cloneFormulaCell(ScDocument* pDoc, const ScAddress& rNewPos, ScFormulaCell& rOldCell)
{
    ScFormulaCell* pNew = new ScFormulaCell(rOldCell, *pDoc, rNewPos, SC_CLONECELL_ADJUST3DREL);
    rOldCell.EndListeningTo(pDoc);
    pNew->StartListeningTo(pDoc);
    pNew->SetDirty();
    return pNew;
}

}

void ScColumn::SwapRow(SCROW nRow1, SCROW nRow2)
{
    if (nRow1 == nRow2)
        // Nothing to swap.
        return;

    // Ensure that nRow1 < nRow2.
    if (nRow2 < nRow1)
        std::swap(nRow1, nRow2);

    // Broadcasters (if exist) should NOT be swapped.

    sc::CellStoreType::position_type aPos1 = maCells.position(nRow1);
    if (aPos1.first == maCells.end())
        return;

    sc::CellStoreType::position_type aPos2 = maCells.position(aPos1.first, nRow2);
    if (aPos2.first == maCells.end())
        return;

    std::vector<SCROW> aRows;
    aRows.reserve(2);
    aRows.push_back(nRow1);
    aRows.push_back(nRow2);

    sc::CellStoreType::iterator it1 = aPos1.first, it2 = aPos2.first;

    if (it1->type == it2->type)
    {
        // Both positions are of the same type. Do a simple value swap.
        switch (it1->type)
        {
            case sc::element_type_empty:
                // Both are empty. Nothing to swap.
                return;
            case sc::element_type_numeric:
                std::swap(
                    sc::numeric_block::at(*it1->data, aPos1.second),
                    sc::numeric_block::at(*it2->data, aPos2.second));
            break;
            case sc::element_type_string:
                std::swap(
                    sc::string_block::at(*it1->data, aPos1.second),
                    sc::string_block::at(*it2->data, aPos2.second));
            break;
            case sc::element_type_edittext:
                std::swap(
                    sc::edittext_block::at(*it1->data, aPos1.second),
                    sc::edittext_block::at(*it2->data, aPos2.second));
            break;
            case sc::element_type_formula:
            {
                // Swapping of formula cells involve adjustment of references wrt their positions.
                sc::formula_block::iterator itf1 = sc::formula_block::begin(*it1->data);
                sc::formula_block::iterator itf2 = sc::formula_block::begin(*it2->data);
                std::advance(itf1, aPos1.second);
                std::advance(itf2, aPos2.second);

                // TODO: Find out a way to adjust references without cloning new instances.
                boost::scoped_ptr<ScFormulaCell> pOld1(*itf1);
                boost::scoped_ptr<ScFormulaCell> pOld2(*itf2);
                DetachFormulaCell(aPos1, **itf1);
                DetachFormulaCell(aPos2, **itf2);
                ScFormulaCell* pNew1 = cloneFormulaCell(pDocument, ScAddress(nCol, nRow1, nTab), *pOld2);
                ScFormulaCell* pNew2 = cloneFormulaCell(pDocument, ScAddress(nCol, nRow2, nTab), *pOld1);
                *itf1 = pNew1;
                *itf2 = pNew2;

                ActivateNewFormulaCell(aPos1, *pNew1);
                ActivateNewFormulaCell(aPos2, *pNew2);
            }
            break;
            default:
                ;
        }

        SwapCellTextAttrs(nRow1, nRow2);
        CellStorageModified();
        BroadcastCells(aRows);
        return;
    }

    // The two cells are of different types.

    ScRefCellValue aCell1 = GetCellValue(aPos1.first, aPos1.second);
    ScRefCellValue aCell2 = GetCellValue(aPos2.first, aPos2.second);

    // Make sure to put cells in row 1 first then row 2!

    if (aCell1.meType == CELLTYPE_NONE)
    {
        // cell 1 is empty and cell 2 is not.
        switch (aCell2.meType)
        {
            case CELLTYPE_VALUE:
                it1 = maCells.set(it1, nRow1, aCell2.mfValue); // it2 becomes invalid.
                maCells.set_empty(it1, nRow2, nRow2);
            break;
            case CELLTYPE_STRING:
                it1 = maCells.set(it1, nRow1, *aCell2.mpString);
                maCells.set_empty(it1, nRow2, nRow2);
            break;
            case CELLTYPE_EDIT:
            {
                it1 = maCells.set(it1, nRow1, aCell2.mpEditText);
                EditTextObject* p;
                maCells.release(it1, nRow2, p);
            }
            break;
            case CELLTYPE_FORMULA:
            {
                // cell 1 is empty and cell 2 is a formula cell.
                ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow1, nTab), *aCell2.mpFormula);
                DetachFormulaCell(aPos2, *aCell2.mpFormula);
                it1 = maCells.set(it1, nRow1, pNew);
                maCells.set_empty(it1, nRow2, nRow2); // original formula cell gets deleted.
                ActivateNewFormulaCell(it1, nRow1, *pNew);
            }
            break;
            default:
                ;
        }

        SwapCellTextAttrs(nRow1, nRow2);
        CellStorageModified();
        BroadcastCells(aRows);
        return;
    }

    if (aCell2.meType == CELLTYPE_NONE)
    {
        // cell 1 is not empty and cell 2 is empty.
        switch (aCell1.meType)
        {
            case CELLTYPE_VALUE:
                // Value is copied in Cell1.
                it1 = maCells.set_empty(it1, nRow1, nRow1);
                maCells.set(it1, nRow2, aCell1.mfValue);
            break;
            case CELLTYPE_STRING:
            {
                OUString aStr = *aCell1.mpString; // make a copy.
                it1 = maCells.set_empty(it1, nRow1, nRow1); // original string is gone.
                maCells.set(it1, nRow2, aStr);
            }
            break;
            case CELLTYPE_EDIT:
            {
                EditTextObject* p;
                it1 = maCells.release(it1, nRow1, p);
                maCells.set(it1, nRow2, p);
            }
            break;
            case CELLTYPE_FORMULA:
            {
                // cell 1 is a formula cell and cell 2 is empty.
                ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow2, nTab), *aCell1.mpFormula);
                DetachFormulaCell(aPos1, *aCell1.mpFormula);
                it1 = maCells.set_empty(it1, nRow1, nRow1); // original formula cell is gone.
                it1 = maCells.set(it1, nRow2, pNew);
                ActivateNewFormulaCell(it1, nRow2, *pNew);
            }
            break;
            default:
                ;
        }

        SwapCellTextAttrs(nRow1, nRow2);
        CellStorageModified();
        BroadcastCells(aRows);
        return;
    }

    // Neither cells are empty, and they are of different types.
    switch (aCell1.meType)
    {
        case CELLTYPE_VALUE:
        {
            switch (aCell2.meType)
            {
                case CELLTYPE_STRING:
                    it1 = maCells.set(it1, nRow1, *aCell2.mpString);
                break;
                case CELLTYPE_EDIT:
                {
                    it1 = maCells.set(it1, nRow1, aCell2.mpEditText);
                    EditTextObject* p;
                    it1 = maCells.release(it1, nRow2, p);
                }
                break;
                case CELLTYPE_FORMULA:
                {
                    DetachFormulaCell(aPos2, *aCell2.mpFormula);
                    ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow1, nTab), *aCell2.mpFormula);
                    it1 = maCells.set(it1, nRow1, pNew);
                    ActivateNewFormulaCell(it1, nRow1, *pNew);
                    // The old formula cell will get overwritten below.
                }
                break;
                default:
                    ;
            }

            maCells.set(it1, nRow2, aCell1.mfValue);

        }
        break;
        case CELLTYPE_STRING:
        {
            OUString aStr = *aCell1.mpString; // make a copy.
            switch (aCell2.meType)
            {
                case CELLTYPE_VALUE:
                    it1 = maCells.set(it1, nRow1, aCell2.mfValue);
                break;
                case CELLTYPE_EDIT:
                {
                    it1 = maCells.set(it1, nRow1, aCell2.mpEditText);
                    EditTextObject* p;
                    it1 = maCells.release(it1, nRow2, p); // prevent it being overwritten.
                }
                break;
                case CELLTYPE_FORMULA:
                {
                    // cell 1 - string, cell 2 - formula
                    DetachFormulaCell(aPos2, *aCell2.mpFormula);
                    ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow1, nTab), *aCell2.mpFormula);
                    it1 = maCells.set(it1, nRow1, pNew);
                    ActivateNewFormulaCell(it1, nRow1, *pNew);
                    // Old formula cell will get overwritten below.
                }
                break;
                default:
                    ;
            }

            maCells.set(it1, nRow2, aStr);
        }
        break;
        case CELLTYPE_EDIT:
        {
            EditTextObject* p;
            it1 = maCells.release(it1, nRow1, p);

            switch (aCell2.meType)
            {
                case CELLTYPE_VALUE:
                    it1 = maCells.set(it1, nRow1, aCell2.mfValue);
                break;
                case CELLTYPE_STRING:
                    it1 = maCells.set(it1, nRow1, *aCell2.mpString);
                break;
                case CELLTYPE_FORMULA:
                {
                    DetachFormulaCell(aPos2, *aCell2.mpFormula);
                    ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow1, nTab), *aCell2.mpFormula);
                    it1 = maCells.set(it1, nRow1, pNew);
                    ActivateNewFormulaCell(it1, nRow1, *pNew);
                    // Old formula cell will get overwritten below.
                }
                break;
                default:
                    ;
            }

            maCells.set(it1, nRow2, aCell1.mpEditText);
        }
        break;
        case CELLTYPE_FORMULA:
        {
            // cell 1 is a formula cell and cell 2 is not.
            DetachFormulaCell(aPos1, *aCell1.mpFormula);
            ScFormulaCell* pNew = cloneFormulaCell(pDocument, ScAddress(nCol, nRow2, nTab), *aCell1.mpFormula);
            switch (aCell2.meType)
            {
                case CELLTYPE_VALUE:
                    it1 = maCells.set(it1, nRow1, aCell2.mfValue);
                break;
                case CELLTYPE_STRING:
                    it1 = maCells.set(it1, nRow1, *aCell2.mpString);
                break;
                case CELLTYPE_EDIT:
                {
                    it1 = maCells.set(it1, nRow1, aCell2.mpEditText);
                    EditTextObject* p;
                    it1 = maCells.release(it1, nRow2, p);
                }
                break;
                default:
                    ;
            }

            it1 = maCells.set(it1, nRow2, pNew);
            ActivateNewFormulaCell(it1, nRow2, *pNew);
        }
        break;
        default:
            ;
    }

    SwapCellTextAttrs(nRow1, nRow2);
    CellStorageModified();
    BroadcastCells(aRows);
}

namespace {

/**
 * Adjust references in formula cell with respect to column-wise relocation.
 */
void updateRefInFormulaCell( ScDocument* pDoc, ScFormulaCell& rCell, SCCOL nCol, SCTAB nTab, SCCOL nColDiff )
{
    rCell.aPos.SetCol(nCol);
    sc::RefUpdateContext aCxt(*pDoc);
    aCxt.meMode = URM_MOVE;
    aCxt.maRange = ScRange(ScAddress(nCol, 0, nTab), ScAddress(nCol, MAXROW, nTab));
    aCxt.mnColDelta = nColDiff;
    rCell.UpdateReference(aCxt);
}

}

void ScColumn::SwapCell( SCROW nRow, ScColumn& rCol)
{
    sc::CellStoreType::position_type aPos1 = maCells.position(nRow);
    sc::CellStoreType::position_type aPos2 = rCol.maCells.position(nRow);

    if (aPos1.first->type == sc::element_type_formula)
    {
        ScFormulaCell& rCell = *sc::formula_block::at(*aPos1.first->data, aPos1.second);
        updateRefInFormulaCell(pDocument, rCell, rCol.nCol, nTab, rCol.nCol - nCol);
        sc::SharedFormulaUtil::unshareFormulaCell(aPos1, rCell);
    }

    if (aPos2.first->type == sc::element_type_formula)
    {
        ScFormulaCell& rCell = *sc::formula_block::at(*aPos2.first->data, aPos2.second);
        updateRefInFormulaCell(pDocument, rCell, nCol, nTab, nCol - rCol.nCol);
        sc::SharedFormulaUtil::unshareFormulaCell(aPos2, rCell);
    }

    maCells.swap(nRow, nRow, rCol.maCells, nRow);
    maCellTextAttrs.swap(nRow, nRow, rCol.maCellTextAttrs, nRow);

    aPos1 = maCells.position(nRow);
    aPos2 = rCol.maCells.position(nRow);

    if (aPos1.first->type == sc::element_type_formula)
    {
        ScFormulaCell& rCell = *sc::formula_block::at(*aPos1.first->data, aPos1.second);
        JoinNewFormulaCell(aPos1, rCell);
    }

    if (aPos2.first->type == sc::element_type_formula)
    {
        ScFormulaCell& rCell = *sc::formula_block::at(*aPos2.first->data, aPos2.second);
        rCol.JoinNewFormulaCell(aPos2, rCell);
    }

    CellStorageModified();
    rCol.CellStorageModified();
}


bool ScColumn::TestInsertCol( SCROW nStartRow, SCROW nEndRow) const
{
    if (IsEmpty())
        return true;

    // Return false if we have any non-empty cells between nStartRow and nEndRow inclusive.
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it->type != sc::element_type_empty)
        return false;

    // Get the length of the remaining empty segment.
    size_t nLen = it->size - aPos.second;
    SCROW nNextNonEmptyRow = nStartRow + nLen;
    if (nNextNonEmptyRow <= nEndRow)
        return false;

    //  AttrArray only looks for merged cells

    return pAttrArray ? pAttrArray->TestInsertCol(nStartRow, nEndRow) : true;
}


bool ScColumn::TestInsertRow( SCROW nStartRow, SCSIZE nSize ) const
{
    //  AttrArray only looks for merged cells
    {
        std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
        sc::CellStoreType::const_iterator it = aPos.first;
        if (it->type == sc::element_type_empty && maCells.block_size() == 1)
            // The entire cell array is empty.
            return pAttrArray->TestInsertRow(nSize);
    }

    // See if there would be any non-empty cell that gets pushed out.

    // Find the position of the last non-empty cell below nStartRow.
    size_t nLastNonEmptyRow = MAXROW;
    sc::CellStoreType::const_reverse_iterator it = maCells.rbegin();
    if (it->type == sc::element_type_empty)
        nLastNonEmptyRow -= it->size;

    if (nLastNonEmptyRow < static_cast<size_t>(nStartRow))
        // No cells would get pushed out.
        return pAttrArray->TestInsertRow(nSize);

    if (nLastNonEmptyRow + nSize > static_cast<size_t>(MAXROW))
        // At least one cell would get pushed out. Not good.
        return false;

    return pAttrArray->TestInsertRow(nSize);
}


void ScColumn::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    pAttrArray->InsertRow( nStartRow, nSize );

    maBroadcasters.insert_empty(nStartRow, nSize);
    maBroadcasters.resize(MAXROWCOUNT);

    maCellTextAttrs.insert_empty(nStartRow, nSize);
    maCellTextAttrs.resize(MAXROWCOUNT);

    maCells.insert_empty(nStartRow, nSize);
    maCells.resize(MAXROWCOUNT);

    CellStorageModified();

    // We *probably* don't need to broadcast here since the parent call seems
    // to take care of it.
}

namespace {

class CopyToClipHandler
{
    const ScColumn& mrSrcCol;
    ScColumn& mrDestCol;
    sc::ColumnBlockPosition maDestPos;
    sc::ColumnBlockPosition* mpDestPos;

    void setDefaultAttrsToDest(size_t nRow, size_t nSize)
    {
        std::vector<sc::CellTextAttr> aAttrs(nSize); // default values
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, aAttrs.begin(), aAttrs.end());
    }

    void groupFormulaCells(std::vector<ScFormulaCell*>& rCells)
    {
        if (rCells.empty())
            return;

        std::vector<ScFormulaCell*>::iterator it = rCells.begin(), itEnd = rCells.end();
        ScFormulaCell* pPrev = *it;
        ScFormulaCell* pCur = NULL;
        for (++it; it != itEnd; ++it, pPrev = pCur)
        {
            pCur = *it;
            ScFormulaCell::CompareState eState = pPrev->CompareByTokenArray(*pPrev);
            if (eState == ScFormulaCell::NotEqual)
                continue;

            ScFormulaCellGroupRef xGroup = pPrev->GetCellGroup();
            if (xGroup)
            {
                // Extend the group.
                ++xGroup->mnLength;
                pCur->SetCellGroup(xGroup);
                continue;
            }

            // Create a new group.
            xGroup = pPrev->CreateCellGroup(pPrev->aPos.Row(), 2, eState == ScFormulaCell::EqualInvariant);
            pCur->SetCellGroup(xGroup);
        }
    }

public:
    CopyToClipHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos) :
        mrSrcCol(rSrcCol), mrDestCol(rDestCol), mpDestPos(pDestPos)
    {
        if (mpDestPos)
            maDestPos = *mpDestPos;
        else
            mrDestCol.InitBlockPosition(maDestPos);
    }

    ~CopyToClipHandler()
    {
        if (mpDestPos)
            *mpDestPos = maDestPos;
    }

    void operator() (const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nTopRow = aNode.position + nOffset;

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nTopRow, it, itEnd);
                setDefaultAttrsToDest(nTopRow, nDataSize);
            }
            break;
            case sc::element_type_string:
            {
                sc::string_block::const_iterator it = sc::string_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nTopRow, it, itEnd);
                setDefaultAttrsToDest(nTopRow, nDataSize);
            }
            break;
            case sc::element_type_edittext:
            {
                sc::edittext_block::const_iterator it = sc::edittext_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::edittext_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                std::vector<EditTextObject*> aCloned;
                aCloned.reserve(nDataSize);
                for (; it != itEnd; ++it)
                    aCloned.push_back(ScEditUtil::Clone(**it, mrDestCol.GetDoc()));

                maDestPos.miCellPos = mrDestCol.GetCellStore().set(
                    maDestPos.miCellPos, nTopRow, aCloned.begin(), aCloned.end());

                setDefaultAttrsToDest(nTopRow, nDataSize);
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator it = sc::formula_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                std::vector<ScFormulaCell*> aCloned;
                aCloned.reserve(nDataSize);
                ScAddress aDestPos(mrDestCol.GetCol(), nTopRow, mrDestCol.GetTab());
                for (; it != itEnd; ++it, aDestPos.IncRow())
                {
                    const ScFormulaCell& rOld = **it;
                    if (rOld.GetDirty() && mrSrcCol.GetDoc().GetAutoCalc())
                        const_cast<ScFormulaCell&>(rOld).Interpret();

                    aCloned.push_back(new ScFormulaCell(rOld, mrDestCol.GetDoc(), aDestPos));
                }

                // Group the cloned formula cells.
                if (!aCloned.empty())
                    sc::SharedFormulaUtil::groupFormulaCells(aCloned.begin(), aCloned.end());

                sc::CellStoreType& rDestCells = mrDestCol.GetCellStore();
                maDestPos.miCellPos = rDestCells.set(
                    maDestPos.miCellPos, nTopRow, aCloned.begin(), aCloned.end());

                // Merge adjacent formula cell groups (if applicable).
                sc::CellStoreType::position_type aPos =
                    rDestCells.position(maDestPos.miCellPos, nTopRow);
                maDestPos.miCellPos = aPos.first;
                sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
                size_t nLastRow = nTopRow + nDataSize;
                if (nLastRow < static_cast<size_t>(MAXROW))
                {
                    aPos = rDestCells.position(maDestPos.miCellPos, nLastRow+1);
                    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
                }

                setDefaultAttrsToDest(nTopRow, nDataSize);
            }
            break;
            default:
                ;
        }
    }
};

}

void ScColumn::CopyToClip(
    sc::CopyToClipContext& rCxt, SCROW nRow1, SCROW nRow2, ScColumn& rColumn ) const
{
    pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray,
                          rCxt.isKeepScenarioFlags() ? (SC_MF_ALL & ~SC_MF_SCENARIO) : SC_MF_ALL );

    CopyToClipHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol));
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
    rColumn.CellStorageModified();
}

void ScColumn::CopyStaticToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol)
{
    if (nRow1 > nRow2)
        return;

    sc::ColumnBlockPosition aDestPos;
    CopyCellTextAttrsToDocument(nRow1, nRow2, rDestCol);

    // First, clear the destination column for the specified row range.
    rDestCol.maCells.set_empty(nRow1, nRow2);

    aDestPos.miCellPos = rDestCol.maCells.begin();

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow1);
    sc::CellStoreType::const_iterator it = aPos.first;
    size_t nOffset = aPos.second;
    size_t nDataSize = 0;
    size_t nCurRow = nRow1;

    for (; it != maCells.end() && nCurRow <= static_cast<size_t>(nRow2); ++it, nOffset = 0, nCurRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nCurRow + nDataSize - 1 > static_cast<size_t>(nRow2))
        {
            // Truncate the block to copy to clipboard.
            nDataSize = nRow2 - nCurRow + 1;
            bLastBlock = true;
        }

        switch (it->type)
        {
            case sc::element_type_numeric:
            {
                sc::numeric_block::const_iterator itData = sc::numeric_block::begin(*it->data);
                std::advance(itData, nOffset);
                sc::numeric_block::const_iterator itDataEnd = itData;
                std::advance(itDataEnd, nDataSize);
                aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nCurRow, itData, itDataEnd);
            }
            break;
            case sc::element_type_string:
            {
                sc::string_block::const_iterator itData = sc::string_block::begin(*it->data);
                std::advance(itData, nOffset);
                sc::string_block::const_iterator itDataEnd = itData;
                std::advance(itDataEnd, nDataSize);
                aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nCurRow, itData, itDataEnd);
            }
            break;
            case sc::element_type_edittext:
            {
                sc::edittext_block::const_iterator itData = sc::edittext_block::begin(*it->data);
                std::advance(itData, nOffset);
                sc::edittext_block::const_iterator itDataEnd = itData;
                std::advance(itDataEnd, nDataSize);

                // Convert to simple strings.
                std::vector<OUString> aConverted;
                aConverted.reserve(nDataSize);
                for (; itData != itDataEnd; ++itData)
                {
                    const EditTextObject& rObj = **itData;
                    aConverted.push_back(ScEditUtil::GetString(rObj, pDocument));
                }
                aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nCurRow, aConverted.begin(), aConverted.end());
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator itData = sc::formula_block::begin(*it->data);
                std::advance(itData, nOffset);
                sc::formula_block::const_iterator itDataEnd = itData;
                std::advance(itDataEnd, nDataSize);

                // Interpret and convert to raw values.
                for (SCROW i = 0; itData != itDataEnd; ++itData, ++i)
                {
                    SCROW nRow = nCurRow + i;

                    ScFormulaCell& rFC = const_cast<ScFormulaCell&>(**itData);
                    if (rFC.GetDirty() && pDocument->GetAutoCalc())
                        rFC.Interpret();

                    if (rFC.GetErrCode())
                        // Skip cells with error.
                        break;

                    if (rFC.IsValue())
                        aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nRow, rFC.GetValue());
                    else
                        aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nRow, rFC.GetString());
                }
            }
            break;
            default:
                ;
        }

        if (bLastBlock)
            break;
    }

    rDestCol.CellStorageModified();
}

void ScColumn::CopyCellToDocument( SCROW nSrcRow, SCROW nDestRow, ScColumn& rDestCol )
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nSrcRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    bool bSet = true;
    switch (it->type)
    {
        case sc::element_type_numeric:
            rDestCol.maCells.set(nDestRow, sc::numeric_block::at(*it->data, aPos.second));
        break;
        case sc::element_type_string:
            rDestCol.maCells.set(nDestRow, sc::string_block::at(*it->data, aPos.second));
        break;
        case sc::element_type_edittext:
        {
            EditTextObject* p = sc::edittext_block::at(*it->data, aPos.second);
            if (pDocument == rDestCol.pDocument)
                rDestCol.maCells.set(nDestRow, p->Clone());
            else
                rDestCol.maCells.set(nDestRow, ScEditUtil::Clone(*p, *rDestCol.pDocument));
        }
        break;
        case sc::element_type_formula:
        {
            ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
            if (p->GetDirty() && pDocument->GetAutoCalc())
                p->Interpret();

            ScAddress aDestPos = p->aPos;
            aDestPos.SetRow(nDestRow);
            ScFormulaCell* pNew = new ScFormulaCell(*p, *rDestCol.pDocument, aDestPos);
            rDestCol.SetFormulaCell(nDestRow, pNew);
        }
        break;
        case sc::element_type_empty:
        default:
            // empty
            rDestCol.maCells.set_empty(nDestRow, nDestRow);
            bSet = false;
    }

    if (bSet)
        rDestCol.maCellTextAttrs.set(nDestRow, maCellTextAttrs.get<sc::CellTextAttr>(nSrcRow));
    else
        rDestCol.maCellTextAttrs.set_empty(nDestRow, nDestRow);

    rDestCol.CellStorageModified();
}

namespace {

bool canCopyValue(const ScDocument& rDoc, const ScAddress& rPos, sal_uInt16 nFlags)
{
    sal_uInt32 nNumIndex = static_cast<const SfxUInt32Item*>(rDoc.GetAttr(rPos, ATTR_VALUE_FORMAT))->GetValue();
    short nType = rDoc.GetFormatTable()->GetType(nNumIndex);
    if ((nType == NUMBERFORMAT_DATE) || (nType == NUMBERFORMAT_TIME) || (nType == NUMBERFORMAT_DATETIME))
        return ((nFlags & IDF_DATETIME) != 0);

    return ((nFlags & IDF_VALUE) != 0);
}

class CopyAsLinkHandler
{
    const ScColumn& mrSrcCol;
    ScColumn& mrDestCol;
    sc::ColumnBlockPosition maDestPos;
    sc::ColumnBlockPosition* mpDestPos;
    sal_uInt16 mnCopyFlags;

    void setDefaultAttrToDest(size_t nRow)
    {
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    }

    void setDefaultAttrsToDest(size_t nRow, size_t nSize)
    {
        std::vector<sc::CellTextAttr> aAttrs(nSize); // default values
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, aAttrs.begin(), aAttrs.end());
    }

    ScFormulaCell* createRefCell(size_t nRow)
    {
        ScSingleRefData aRef;
        aRef.InitAddress(ScAddress(mrSrcCol.GetCol(), nRow, mrSrcCol.GetTab())); // Absolute reference.
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);
        return new ScFormulaCell(&mrDestCol.GetDoc(), ScAddress(mrDestCol.GetCol(), nRow, mrDestCol.GetTab()), &aArr);
    }

    void createRefBlock(const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nTopRow = aNode.position + nOffset;

        for (size_t i = 0; i < nDataSize; ++i)
        {
            SCROW nRow = nTopRow + i;
            mrDestCol.SetFormulaCell(maDestPos, nRow, createRefCell(nRow));
        }

        setDefaultAttrsToDest(nTopRow, nDataSize);
    }

public:
    CopyAsLinkHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos, sal_uInt16 nCopyFlags) :
        mrSrcCol(rSrcCol), mrDestCol(rDestCol), mpDestPos(pDestPos), mnCopyFlags(nCopyFlags)
    {
        if (mpDestPos)
            maDestPos = *mpDestPos;
    }

    ~CopyAsLinkHandler()
    {
        if (mpDestPos)
            *mpDestPos = maDestPos;
    }

    void operator() (const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nRow = aNode.position + nOffset;

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                if ((mnCopyFlags & (IDF_DATETIME|IDF_VALUE)) == 0)
                    return;

                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                ScAddress aSrcPos(mrSrcCol.GetCol(), nRow, mrSrcCol.GetTab());
                for (; it != itEnd; ++it, aSrcPos.IncRow(), ++nRow)
                {
                    if (!canCopyValue(mrSrcCol.GetDoc(), aSrcPos, mnCopyFlags))
                        continue;

                    maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, createRefCell(nRow));
                    setDefaultAttrToDest(nRow);
                }
            }
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
            {
                if (!(mnCopyFlags & IDF_STRING))
                    return;

                createRefBlock(aNode, nOffset, nDataSize);
            }
            break;
            case sc::element_type_formula:
            {
                if (!(mnCopyFlags & IDF_FORMULA))
                    return;

                createRefBlock(aNode, nOffset, nDataSize);
            }
            break;
            default:
                ;
        }
    }
};

class CopyByCloneHandler
{
    const ScColumn& mrSrcCol;
    ScColumn& mrDestCol;
    sc::ColumnBlockPosition maDestPos;
    sc::ColumnBlockPosition* mpDestPos;
    sal_uInt16 mnCopyFlags;

    void setDefaultAttrToDest(size_t nRow)
    {
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    }

    void setDefaultAttrsToDest(size_t nRow, size_t nSize)
    {
        std::vector<sc::CellTextAttr> aAttrs(nSize); // default values
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, aAttrs.begin(), aAttrs.end());
    }

    void cloneFormulaCell(size_t nRow, ScFormulaCell& rSrcCell)
    {
        ScAddress aDestPos(mrDestCol.GetCol(), nRow, mrDestCol.GetTab());

        bool bCloneValue          = (mnCopyFlags & IDF_VALUE) != 0;
        bool bCloneDateTime       = (mnCopyFlags & IDF_DATETIME) != 0;
        bool bCloneString         = (mnCopyFlags & IDF_STRING) != 0;
        bool bCloneSpecialBoolean = (mnCopyFlags & IDF_SPECIAL_BOOLEAN) != 0;
        bool bCloneFormula        = (mnCopyFlags & IDF_FORMULA) != 0;

        bool bForceFormula = false;

        if (bCloneSpecialBoolean)
        {
            // See if the formula consists of =TRUE() or =FALSE().
            ScTokenArray* pCode = rSrcCell.GetCode();
            if (pCode && pCode->GetLen() == 1)
            {
                const formula::FormulaToken* p = pCode->First();
                if (p->GetOpCode() == ocTrue || p->GetOpCode() == ocFalse)
                    // This is a boolean formula.
                    bForceFormula = true;
            }
        }

        if (bForceFormula || bCloneFormula)
        {
            // Clone as formula cell.
            ScFormulaCell* pCell = new ScFormulaCell(rSrcCell, mrDestCol.GetDoc(), aDestPos);
            pCell->SetDirtyVar();
            mrDestCol.SetFormulaCell(maDestPos, nRow, pCell);
            setDefaultAttrToDest(nRow);
            return;
        }

        if (mrDestCol.GetDoc().IsUndo())
            return;

        if (bCloneValue)
        {
            sal_uInt16 nErr = rSrcCell.GetErrCode();
            if (nErr)
            {
                // error codes are cloned with values
                ScFormulaCell* pErrCell = new ScFormulaCell(&mrDestCol.GetDoc(), aDestPos);
                pErrCell->SetErrCode(nErr);
                mrDestCol.SetFormulaCell(maDestPos, nRow, pErrCell);
                setDefaultAttrToDest(nRow);
                return;
            }
        }

        if (bCloneValue || bCloneDateTime)
        {
            if (rSrcCell.IsValue())
            {
                if (canCopyValue(mrSrcCol.GetDoc(), ScAddress(mrSrcCol.GetCol(), nRow, mrSrcCol.GetTab()), mnCopyFlags))
                {
                    maDestPos.miCellPos = mrDestCol.GetCellStore().set(
                        maDestPos.miCellPos, nRow, rSrcCell.GetValue());
                    setDefaultAttrToDest(nRow);
                }

                return;
            }
        }

        if (bCloneString)
        {
            OUString aStr = rSrcCell.GetString();
            if (aStr.isEmpty())
                // Don't create empty string cells.
                return;

            if (rSrcCell.IsMultilineResult())
            {
                // Clone as an edit text object.
                EditEngine& rEngine = mrDestCol.GetDoc().GetEditEngine();
                rEngine.SetText(aStr);
                maDestPos.miCellPos =
                    mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, rEngine.CreateTextObject());
            }
            else
            {
                maDestPos.miCellPos =
                    mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, aStr);
            }

            setDefaultAttrToDest(nRow);
        }
    }

public:
    CopyByCloneHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos, sal_uInt16 nCopyFlags) :
        mrSrcCol(rSrcCol), mrDestCol(rDestCol), mpDestPos(pDestPos), mnCopyFlags(nCopyFlags)
    {
        if (mpDestPos)
            maDestPos = *mpDestPos;
    }

    ~CopyByCloneHandler()
    {
        if (mpDestPos)
            *mpDestPos = maDestPos;
    }

    void operator() (const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nRow = aNode.position + nOffset;

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                if ((mnCopyFlags & (IDF_DATETIME|IDF_VALUE)) == 0)
                    return;

                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                ScAddress aSrcPos(mrSrcCol.GetCol(), nRow, mrSrcCol.GetTab());
                for (; it != itEnd; ++it, aSrcPos.IncRow(), ++nRow)
                {
                    if (!canCopyValue(mrSrcCol.GetDoc(), aSrcPos, mnCopyFlags))
                        continue;

                    maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, *it);
                    setDefaultAttrToDest(nRow);
                }
            }
            break;
            case sc::element_type_string:
            {
                if (!(mnCopyFlags & IDF_STRING))
                    return;

                sc::string_block::const_iterator it = sc::string_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it, ++nRow)
                {
                    const OUString& rStr = *it;
                    if (rStr.isEmpty())
                    {
                        // String cell with empty value is used to special-case cell value removal.
                        maDestPos.miCellPos = mrDestCol.GetCellStore().set_empty(
                            maDestPos.miCellPos, nRow, nRow);
                        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set_empty(
                            maDestPos.miCellTextAttrPos, nRow, nRow);
                    }
                    else
                    {
                        maDestPos.miCellPos =
                            mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, rStr);
                        setDefaultAttrToDest(nRow);
                    }
                }
            }
            break;
            case sc::element_type_edittext:
            {
                if (!(mnCopyFlags & IDF_STRING))
                    return;

                sc::edittext_block::const_iterator it = sc::edittext_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::edittext_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                std::vector<EditTextObject*> aCloned;
                aCloned.reserve(nDataSize);
                for (; it != itEnd; ++it)
                    aCloned.push_back(ScEditUtil::Clone(**it, mrDestCol.GetDoc()));

                maDestPos.miCellPos = mrDestCol.GetCellStore().set(
                    maDestPos.miCellPos, nRow, aCloned.begin(), aCloned.end());

                setDefaultAttrsToDest(nRow, nDataSize);
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator it = sc::formula_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it, ++nRow)
                    cloneFormulaCell(nRow, const_cast<ScFormulaCell&>(**it));
            }
            break;
            default:
                ;
        }
    }
};

}

void ScColumn::CopyToColumn(
    sc::CopyToDocContext& rCxt,
    SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked, ScColumn& rColumn,
    const ScMarkData* pMarkData, bool bAsLink) const
{
    if (bMarked)
    {
        SCROW nStart, nEnd;
        if (pMarkData && pMarkData->IsMultiMarked())
        {
            ScMarkArrayIter aIter( pMarkData->GetArray()+nCol );

            while ( aIter.Next( nStart, nEnd ) && nStart <= nRow2 )
            {
                if ( nEnd >= nRow1 )
                    CopyToColumn(rCxt, std::max(nRow1,nStart), std::min(nRow2,nEnd),
                                    nFlags, false, rColumn, pMarkData, bAsLink );
            }
        }
        else
        {
            OSL_FAIL("CopyToColumn: bMarked, but no mark");
        }
        return;
    }

    if ( (nFlags & IDF_ATTRIB) != 0 )
    {
        if ( (nFlags & IDF_STYLES) != IDF_STYLES )
        {   // keep the StyleSheets in the target document
            // e.g. DIF and RTF Clipboard-Import
            for ( SCROW nRow = nRow1; nRow <= nRow2; nRow++ )
            {
                const ScStyleSheet* pStyle =
                    rColumn.pAttrArray->GetPattern( nRow )->GetStyleSheet();
                const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
                ScPatternAttr* pNewPattern = new ScPatternAttr( *pPattern );
                pNewPattern->SetStyleSheet( (ScStyleSheet*)pStyle );
                rColumn.pAttrArray->SetPattern( nRow, pNewPattern, true );
                delete pNewPattern;
            }
        }
        else
            pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray);
    }

    if ((nFlags & IDF_CONTENTS) != 0)
    {
        if (bAsLink)
        {
            CopyAsLinkHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol), nFlags);
            sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
        }
        else
        {
            CopyByCloneHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol), nFlags);
            sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
        }

        rColumn.CellStorageModified();
    }
}


void ScColumn::UndoToColumn(
    sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked,
    ScColumn& rColumn, const ScMarkData* pMarkData ) const
{
    if (nRow1 > 0)
        CopyToColumn(rCxt, 0, nRow1-1, IDF_FORMULA, false, rColumn);

    CopyToColumn(rCxt, nRow1, nRow2, nFlags, bMarked, rColumn, pMarkData);      //! bMarked ????

    if (nRow2 < MAXROW)
        CopyToColumn(rCxt, nRow2+1, MAXROW, IDF_FORMULA, false, rColumn);
}


void ScColumn::CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const
{
    // Copy cells from this column to the destination column only for those
    // rows that are present in the position column (rPosCol).

    // First, mark all the non-empty cell ranges from the position column.
    sc::SingleColumnSpanSet aRangeSet;
    aRangeSet.scan(rPosCol);

    // Now, copy cells from this column to the destination column for those
    // marked row ranges.
    sc::SingleColumnSpanSet::SpansType aRanges;
    aRangeSet.getSpans(aRanges);

    CopyToClipHandler aFunc(*this, rDestCol, NULL);
    sc::CellStoreType::const_iterator itPos = maCells.begin();
    sc::SingleColumnSpanSet::SpansType::const_iterator it = aRanges.begin(), itEnd = aRanges.end();
    for (; it != itEnd; ++it)
        itPos = sc::ParseBlock(itPos, maCells, aFunc, it->mnRow1, it->mnRow2);

    rDestCol.CellStorageModified();
}


void ScColumn::CopyScenarioFrom( const ScColumn& rSrcCol )
{
    //  This is the scenario table, the data is copied into it
    sc::CopyToDocContext aCxt(*pDocument);
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            DeleteArea( nStart, nEnd, IDF_CONTENTS );
            ((ScColumn&)rSrcCol).
                CopyToColumn(aCxt, nStart, nEnd, IDF_CONTENTS, false, *this);

            //  UpdateUsed not needed, already done in TestCopyScenario (obsolete comment ?)

            sc::RefUpdateContext aRefCxt(*pDocument);
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(nCol, nStart, nTab, nCol, nEnd, nTab);
            aRefCxt.mnTabDelta = nTab - rSrcCol.nTab;
            UpdateReferenceOnCopy(aRefCxt, NULL);
            UpdateCompile();
        }

        //! make CopyToColumn "const" !!! (obsolete comment ?)

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}


void ScColumn::CopyScenarioTo( ScColumn& rDestCol ) const
{
    //  This is the scenario table, the data is copied to the other
    sc::CopyToDocContext aCxt(*rDestCol.pDocument);
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            rDestCol.DeleteArea( nStart, nEnd, IDF_CONTENTS );
            CopyToColumn(aCxt, nStart, nEnd, IDF_CONTENTS, false, rDestCol);

            //  UpdateUsed not needed, is already done in TestCopyScenario (obsolete comment ?)

            sc::RefUpdateContext aRefCxt(*pDocument);
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(rDestCol.nCol, nStart, rDestCol.nTab, rDestCol.nCol, nEnd, rDestCol.nTab);
            aRefCxt.mnTabDelta = rDestCol.nTab - nTab;
            rDestCol.UpdateReferenceOnCopy(aRefCxt, NULL);
            rDestCol.UpdateCompile();
        }

        //! make CopyToColumn "const" !!! (obsolete comment ?)

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}


bool ScColumn::TestCopyScenarioTo( const ScColumn& rDestCol ) const
{
    bool bOk = true;
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = 0, nEnd = 0;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern && bOk)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
            if ( rDestCol.pAttrArray->HasAttrib( nStart, nEnd, HASATTR_PROTECTED ) )
                bOk = false;

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
    return bOk;
}


void ScColumn::MarkScenarioIn( ScMarkData& rDestMark ) const
{
    ScRange aRange( nCol, 0, nTab );

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            aRange.aStart.SetRow( nStart );
            aRange.aEnd.SetRow( nEnd );
            rDestMark.SetMultiMarkArea( aRange, true );
        }

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}

namespace {

void resetColumnPosition(sc::CellStoreType& rCells, SCCOL nCol)
{
    sc::CellStoreType::iterator it = rCells.begin(), itEnd = rCells.end();
    for (; it != itEnd; ++it)
    {
        if (it->type != sc::element_type_formula)
            continue;

        sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
        sc::formula_block::iterator itCellEnd = sc::formula_block::end(*it->data);
        for (; itCell != itCellEnd; ++itCell)
        {
            ScFormulaCell& rCell = **itCell;
            rCell.aPos.SetCol(nCol);
        }
    }
}

}

void ScColumn::SwapCol(ScColumn& rCol)
{
    maBroadcasters.swap(rCol.maBroadcasters);
    maCells.swap(rCol.maCells);
    maCellTextAttrs.swap(rCol.maCellTextAttrs);

    ScAttrArray* pTempAttr = rCol.pAttrArray;
    rCol.pAttrArray = pAttrArray;
    pAttrArray = pTempAttr;

    // AttrArray needs to have the right column number
    pAttrArray->SetCol(nCol);
    rCol.pAttrArray->SetCol(rCol.nCol);

    std::swap(mbDirtyGroups, rCol.mbDirtyGroups);

    // Reset column positions in formula cells.
    resetColumnPosition(maCells, nCol);
    resetColumnPosition(rCol.maCells, rCol.nCol);

    CellStorageModified();
    rCol.CellStorageModified();
}

void ScColumn::MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol)
{
    pAttrArray->MoveTo(nStartRow, nEndRow, *rCol.pAttrArray);

    // Mark the non-empty cells within the specified range, for later broadcasting.
    sc::SingleColumnSpanSet aNonEmpties;
    aNonEmpties.scan(*this, nStartRow, nEndRow);
    sc::SingleColumnSpanSet::SpansType aRanges;
    aNonEmpties.getSpans(aRanges);

    // Split the formula grouping at the top and bottom boundaries.
    sc::CellStoreType::position_type aPos = maCells.position(nStartRow);
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);
    aPos = maCells.position(aPos.first, nEndRow+1);
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);

    // Do the same with the destination column.
    aPos = rCol.maCells.position(nStartRow);
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);
    aPos = rCol.maCells.position(aPos.first, nEndRow+1);
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);

    // Move the broadcasters to the destination column.
    maBroadcasters.transfer(nStartRow, nEndRow, rCol.maBroadcasters, nStartRow);
    maCells.transfer(nStartRow, nEndRow, rCol.maCells, nStartRow);
    maCellTextAttrs.transfer(nStartRow, nEndRow, rCol.maCellTextAttrs, nStartRow);

    // Re-group transferred formula cells.
    aPos = rCol.maCells.position(nStartRow);
    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    aPos = rCol.maCells.position(aPos.first, nEndRow+1);
    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);

    CellStorageModified();
    rCol.CellStorageModified();

    // Broadcast on moved ranges. Area-broadcast only.
    ScHint aHint(SC_HINT_DATACHANGED, ScAddress(nCol, 0, nTab));
    ScAddress& rPos = aHint.GetAddress();
    sc::SingleColumnSpanSet::SpansType::const_iterator itRange = aRanges.begin(), itRangeEnd = aRanges.end();
    for (; itRange != itRangeEnd; ++itRange)
    {
        for (SCROW nRow = itRange->mnRow1; nRow <= itRange->mnRow2; ++nRow)
        {
            rPos.SetRow(nRow);
            pDocument->AreaBroadcast(aHint);
        }
    }
}

namespace {

class SubTotalCellPicker
{
    sc::ColumnSpanSet& mrSet;
    SCTAB mnTab;
    SCCOL mnCol;
    bool mbVal;
public:
    SubTotalCellPicker(sc::ColumnSpanSet& rSet, SCTAB nTab, SCCOL nCol, bool bVal) :
        mrSet(rSet), mnTab(nTab), mnCol(nCol), mbVal(bVal) {}

    void operator() (size_t nRow, const ScFormulaCell* pCell)
    {
        if (pCell->IsSubTotal())
            mrSet.set(mnTab, mnCol, nRow, mbVal);
    }
};

}

void ScColumn::MarkSubTotalCells( sc::ColumnSpanSet& rSet, SCROW nRow1, SCROW nRow2, bool bVal ) const
{
    SubTotalCellPicker aFunc(rSet, nTab, nCol, bVal);
    sc::ParseFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

namespace {

class UpdateRefOnCopy
{
    const sc::RefUpdateContext& mrCxt;
    ScDocument* mpUndoDoc;
    bool mbUpdated;

public:
    UpdateRefOnCopy(const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc) :
        mrCxt(rCxt), mpUndoDoc(pUndoDoc), mbUpdated(false) {}

    bool isUpdated() const { return mbUpdated; }

    void operator() (sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        if (node.type != sc::element_type_formula)
            return;

        sc::formula_block::iterator it = sc::formula_block::begin(*node.data);
        std::advance(it, nOffset);
        sc::formula_block::iterator itEnd = it;
        std::advance(itEnd, nDataSize);

        for (; it != itEnd; ++it)
        {
            ScFormulaCell& rCell = **it;
            mbUpdated |= rCell.UpdateReference(mrCxt, mpUndoDoc);
        }
    }
};

class UpdateRefOnNonCopy
{
    SCCOL mnCol;
    SCROW mnTab;
    const sc::RefUpdateContext& mrCxt;
    ScDocument* mpUndoDoc;
    bool mbUpdated;

public:
    UpdateRefOnNonCopy(
        SCCOL nCol, SCTAB nTab, const sc::RefUpdateContext& rCxt,
        ScDocument* pUndoDoc) :
        mnCol(nCol), mnTab(nTab), mrCxt(rCxt),
        mpUndoDoc(pUndoDoc), mbUpdated(false) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        ScAddress aUndoPos(mnCol, nRow, mnTab);
        mbUpdated |= pCell->UpdateReference(mrCxt, mpUndoDoc, &aUndoPos);
    }

    bool isUpdated() const { return mbUpdated; }
};

}

bool ScColumn::UpdateReferenceOnCopy( const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc )
{
    // When copying, the range equals the destination range where cells
    // are pasted, and the dx, dy, dz refer to the distance from the
    // source range.

    UpdateRefOnCopy aHandler(rCxt, pUndoDoc);
    sc::CellStoreType::position_type aPos = maCells.position(rCxt.maRange.aStart.Row());
    sc::ProcessBlock(aPos.first, maCells, aHandler, rCxt.maRange.aStart.Row(), rCxt.maRange.aEnd.Row());

    // The formula groups at the top and bottom boundaries are expected to
    // have been split prior to this call. Here, we only do the joining.
    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    if (rCxt.maRange.aEnd.Row() < MAXROW)
    {
        aPos = maCells.position(aPos.first, rCxt.maRange.aEnd.Row()+1);
        sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    }

    return aHandler.isUpdated();
}

bool ScColumn::UpdateReference( const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc )
{
    if (rCxt.meMode == URM_COPY)
        return UpdateReferenceOnCopy(rCxt, pUndoDoc);

    bool bThisColShifted = (rCxt.maRange.aStart.Tab() <= nTab && nTab <= rCxt.maRange.aEnd.Tab() &&
                            rCxt.maRange.aStart.Col() <= nCol && nCol <= rCxt.maRange.aEnd.Col());
    if (bThisColShifted)
    {
        // Cells in this column is being shifted.  Split formula grouping at
        // the top and bottom boundaries before they get shifted.
        SCROW nSplitPos = rCxt.maRange.aStart.Row();
        if (ValidRow(nSplitPos))
        {
            sc::CellStoreType::position_type aPos = maCells.position(nSplitPos);
            sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);
            nSplitPos = rCxt.maRange.aEnd.Row() + 1;
            if (ValidRow(nSplitPos))
            {
                aPos = maCells.position(aPos.first, nSplitPos);
                sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);
            }
        }
    }

    UpdateRefOnNonCopy aHandler(nCol, nTab, rCxt, pUndoDoc);
    sc::ProcessFormula(maCells, aHandler);
    return aHandler.isUpdated();
}

namespace {

class UpdateTransHandler
{
    ScColumn& mrColumn;
    sc::CellStoreType::iterator miPos;
    ScRange maSource;
    ScAddress maDest;
    ScDocument* mpUndoDoc;
public:
    UpdateTransHandler(ScColumn& rColumn, const ScRange& rSource, const ScAddress& rDest, ScDocument* pUndoDoc) :
        mrColumn(rColumn),
        miPos(rColumn.GetCellStore().begin()),
        maSource(rSource), maDest(rDest), mpUndoDoc(pUndoDoc) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        sc::CellStoreType::position_type aPos = mrColumn.GetCellStore().position(miPos, nRow);
        miPos = aPos.first;
        sc::SharedFormulaUtil::unshareFormulaCell(aPos, *pCell);
        pCell->UpdateTranspose(maSource, maDest, mpUndoDoc);
        mrColumn.JoinNewFormulaCell(aPos, *pCell);
    }
};

class UpdateGrowHandler
{
    ScColumn& mrColumn;
    sc::CellStoreType::iterator miPos;
    ScRange maArea;
    SCCOL mnGrowX;
    SCROW mnGrowY;
public:
    UpdateGrowHandler(ScColumn& rColumn, const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY) :
        mrColumn(rColumn),
        miPos(rColumn.GetCellStore().begin()),
        maArea(rArea), mnGrowX(nGrowX), mnGrowY(nGrowY) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        sc::CellStoreType::position_type aPos = mrColumn.GetCellStore().position(miPos, nRow);
        miPos = aPos.first;
        sc::SharedFormulaUtil::unshareFormulaCell(aPos, *pCell);
        pCell->UpdateGrow(maArea, mnGrowX, mnGrowY);
        mrColumn.JoinNewFormulaCell(aPos, *pCell);
    }
};

class InsertTabUpdater
{
    sc::RefUpdateInsertTabContext& mrCxt;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miAttrPos;
    SCTAB mnTab;
    bool mbModified;

public:
    InsertTabUpdater(sc::RefUpdateInsertTabContext& rCxt, sc::CellTextAttrStoreType& rTextAttrs, SCTAB nTab) :
        mrCxt(rCxt),
        mrTextAttrs(rTextAttrs),
        miAttrPos(rTextAttrs.begin()),
        mnTab(nTab),
        mbModified(false) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->UpdateInsertTab(mrCxt);
        mbModified = true;
    }

    void operator() (size_t nRow, EditTextObject* pCell)
    {
        editeng::FieldUpdater aUpdater = pCell->GetFieldUpdater();
        aUpdater.updateTableFields(mnTab);
        miAttrPos = mrTextAttrs.set(miAttrPos, nRow, sc::CellTextAttr());
        mbModified = true;
    }

    bool isModified() const { return mbModified; }
};

class DeleteTabUpdater
{
    sc::RefUpdateDeleteTabContext& mrCxt;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miAttrPos;
    SCTAB mnTab;
    bool mbModified;
public:
    DeleteTabUpdater(sc::RefUpdateDeleteTabContext& rCxt, sc::CellTextAttrStoreType& rTextAttrs, SCTAB nTab) :
        mrCxt(rCxt),
        mrTextAttrs(rTextAttrs),
        miAttrPos(rTextAttrs.begin()),
        mnTab(nTab),
        mbModified(false) {}

    void operator() (size_t, ScFormulaCell* pCell)
    {
        pCell->UpdateDeleteTab(mrCxt);
        mbModified = true;
    }

    void operator() (size_t nRow, EditTextObject* pCell)
    {
        editeng::FieldUpdater aUpdater = pCell->GetFieldUpdater();
        aUpdater.updateTableFields(mnTab);
        miAttrPos = mrTextAttrs.set(miAttrPos, nRow, sc::CellTextAttr());
        mbModified = true;
    }

    bool isModified() const { return mbModified; }
};

class InsertAbsTabUpdater
{
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miAttrPos;
    SCTAB mnTab;
    SCTAB mnNewPos;
    bool mbModified;
public:
    InsertAbsTabUpdater(sc::CellTextAttrStoreType& rTextAttrs, SCTAB nTab, SCTAB nNewPos) :
        mrTextAttrs(rTextAttrs),
        miAttrPos(rTextAttrs.begin()),
        mnTab(nTab),
        mnNewPos(nNewPos),
        mbModified(false) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->UpdateInsertTabAbs(mnNewPos);
        mbModified = true;
    }

    void operator() (size_t nRow, EditTextObject* pCell)
    {
        editeng::FieldUpdater aUpdater = pCell->GetFieldUpdater();
        aUpdater.updateTableFields(mnTab);
        miAttrPos = mrTextAttrs.set(miAttrPos, nRow, sc::CellTextAttr());
        mbModified = true;
    }

    bool isModified() const { return mbModified; }
};

class MoveTabUpdater
{
    sc::RefUpdateMoveTabContext& mrCxt;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miAttrPos;
    SCTAB mnTab;
    bool mbModified;
public:
    MoveTabUpdater(sc::RefUpdateMoveTabContext& rCxt, sc::CellTextAttrStoreType& rTextAttrs, SCTAB nTab) :
        mrCxt(rCxt),
        mrTextAttrs(rTextAttrs),
        miAttrPos(rTextAttrs.begin()),
        mnTab(nTab),
        mbModified(false) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->UpdateMoveTab(mrCxt, mnTab);
        mbModified = true;
    }

    void operator() (size_t nRow, EditTextObject* pCell)
    {
        editeng::FieldUpdater aUpdater = pCell->GetFieldUpdater();
        aUpdater.updateTableFields(mnTab);
        miAttrPos = mrTextAttrs.set(miAttrPos, nRow, sc::CellTextAttr());
        mbModified = true;
    }

    bool isModified() const { return mbModified; }
};

class UpdateCompileHandler
{
    bool mbForceIfNameInUse:1;
public:
    UpdateCompileHandler(bool bForceIfNameInUse) :
        mbForceIfNameInUse(bForceIfNameInUse) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->UpdateCompile(mbForceIfNameInUse);
    }
};

class TabNoSetter
{
    SCTAB mnTab;
public:
    TabNoSetter(SCTAB nTab) : mnTab(nTab) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->aPos.SetTab(mnTab);
    }
};

class UsedRangeNameFinder
{
    std::set<sal_uInt16>& mrIndexes;
public:
    UsedRangeNameFinder(std::set<sal_uInt16>& rIndexes) : mrIndexes(rIndexes) {}

    void operator() (size_t /*nRow*/, const ScFormulaCell* pCell)
    {
        pCell->FindRangeNamesInUse(mrIndexes);
    }
};

struct SetDirtyVarHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        p->SetDirtyVar();
    }
};

class SetDirtyHandler
{
    ScDocument& mrDoc;
public:
    SetDirtyHandler(ScDocument& rDoc) : mrDoc(rDoc) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        p->SetDirtyVar();
        if (!mrDoc.IsInFormulaTree(p))
            mrDoc.PutInFormulaTree(p);
    }
};

class SetDirtyOnRangeHandler
{
    sc::SingleColumnSpanSet maValueRanges;
    ScColumn& mrColumn;
public:
    SetDirtyOnRangeHandler(ScColumn& rColumn) : mrColumn(rColumn) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        p->SetDirty();
    }

    void operator() (mdds::mtv::element_t type, size_t nTopRow, size_t nDataSize)
    {
        if (type == sc::element_type_empty)
            // Ignore empty blocks.
            return;

        // Non-formula cells.
        SCROW nRow1 = nTopRow;
        SCROW nRow2 = nTopRow + nDataSize - 1;
        maValueRanges.set(nRow1, nRow2, true);
    }

    void broadcast()
    {
        std::vector<SCROW> aRows;
        maValueRanges.getRows(aRows);
        mrColumn.BroadcastCells(aRows);
    }
};

class SetTableOpDirtyOnRangeHandler
{
    sc::SingleColumnSpanSet maValueRanges;
    ScColumn& mrColumn;
public:
    SetTableOpDirtyOnRangeHandler(ScColumn& rColumn) : mrColumn(rColumn) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        p->SetTableOpDirty();
    }

    void operator() (mdds::mtv::element_t type, size_t nTopRow, size_t nDataSize)
    {
        if (type == sc::element_type_empty)
            // Ignore empty blocks.
            return;

        // Non-formula cells.
        SCROW nRow1 = nTopRow;
        SCROW nRow2 = nTopRow + nDataSize - 1;
        maValueRanges.set(nRow1, nRow2, true);
    }

    void broadcast()
    {
        std::vector<SCROW> aRows;
        maValueRanges.getRows(aRows);
        mrColumn.BroadcastCells(aRows);
    }
};

struct SetDirtyAfterLoadHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
#if 1
        // Simply set dirty and append to FormulaTree, without broadcasting,
        // which is a magnitude faster. This is used to calculate the entire
        // document, e.g. when loading alien file formats.
        pCell->SetDirtyAfterLoad();
#else
/* This was used with the binary file format that stored results, where only
 * newly compiled and volatile functions and their dependents had to be
 * recalculated, which was faster then. Since that was moved to 'binfilter' to
 * convert to an XML file this isn't needed anymore, and not used for other
 * file formats. Kept for reference in case mechanism needs to be reactivated
 * for some file formats, we'd have to introduce a controlling parameter to
 * this method here then.
*/

        // If the cell was alsready dirty because of CalcAfterLoad,
        // FormulaTracking has to take place.
        if (pCell->GetDirty())
            pCell->SetDirty();
#endif
    }
};

struct SetRelNameDirtyHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        if (pCell->HasRelNameReference())
            pCell->SetDirty();
    }
};

struct CalcAllHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
#if OSL_DEBUG_LEVEL > 1
        // after F9 ctrl-F9: check the calculation for each FormulaTree
        double nOldVal, nNewVal;
        nOldVal = pCell->GetValue();
#endif
        pCell->Interpret();
#if OSL_DEBUG_LEVEL > 1
        if (pCell->GetCode()->IsRecalcModeNormal())
            nNewVal = pCell->GetValue();
        else
            nNewVal = nOldVal;  // random(), jetzt() etc.

        OSL_ENSURE(nOldVal == nNewVal, "CalcAll: nOldVal != nNewVal");
#endif
    }
};

struct CompileAllHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        // for unconditional compilation
        // bCompile=true and pCode->nError=0
        pCell->GetCode()->SetCodeError(0);
        pCell->SetCompile(true);
        pCell->CompileTokenArray();
    }
};

class CompileXMLHandler
{
    ScProgress& mrProgress;
    const ScColumn& mrCol;
public:
    CompileXMLHandler(ScProgress& rProgress, const ScColumn& rCol) :
        mrProgress(rProgress),
        mrCol(rCol) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        sal_uInt32 nFormat = mrCol.GetNumberFormat(nRow);
        if( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
            pCell->SetNeedNumberFormat(false);
        else
            pCell->SetDirty(true);

        pCell->CompileXML(mrProgress);
    }
};

class CompileErrorCellsHandler
{
    ScColumn& mrColumn;
    sc::CellStoreType::iterator miPos;
    sal_uInt16 mnErrCode;
    FormulaGrammar::Grammar meGram;
    bool mbCompiled;
public:
    CompileErrorCellsHandler(ScColumn& rColumn, sal_uInt16 nErrCode, FormulaGrammar::Grammar eGram) :
        mrColumn(rColumn),
        miPos(mrColumn.GetCellStore().begin()),
        mnErrCode(nErrCode),
        meGram(eGram),
        mbCompiled(false)
    {
    }

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        sal_uInt16 nCurError = pCell->GetRawError();
        if (!nCurError)
            // It's not an error cell. Skip it.
            return;

        if (mnErrCode && nCurError != mnErrCode)
            // Error code is specified, and it doesn't match. Skip it.
            return;

        sc::CellStoreType::position_type aPos = mrColumn.GetCellStore().position(miPos, nRow);
        miPos = aPos.first;
        sc::SharedFormulaUtil::unshareFormulaCell(aPos, *pCell);
        pCell->GetCode()->SetCodeError(0);
        OUStringBuffer aBuf;
        pCell->GetFormula(aBuf, meGram);
        pCell->Compile(aBuf.makeStringAndClear(), false, meGram);
        mrColumn.JoinNewFormulaCell(aPos, *pCell);

        mbCompiled = true;
    }

    bool isCompiled() const { return mbCompiled; }
};

struct CalcAfterLoadHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->CalcAfterLoad();
    }
};

struct ResetChangedHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->SetChanged(false);
    }
};

/**
 * Ambiguous script type counts as edit cell.
 */
class FindEditCellsHandler
{
    ScColumn& mrColumn;
    sc::CellTextAttrStoreType::iterator miAttrPos;

public:
    FindEditCellsHandler(ScColumn& rColumn, sc::CellTextAttrStoreType& rAttrs) :
        mrColumn(rColumn), miAttrPos(rAttrs.begin()) {}

    bool operator() (size_t, const EditTextObject*)
    {
        return true;
    }

    bool operator() (size_t nRow, const ScFormulaCell* p)
    {
        sal_uInt8 nScriptType = mrColumn.GetRangeScriptType(miAttrPos, nRow, nRow);
        if (IsAmbiguousScriptNonZero(nScriptType))
            return true;

        return const_cast<ScFormulaCell*>(p)->IsMultilineResult();
    }

    std::pair<size_t,bool> operator() (mdds::mtv::element_t type, size_t nTopRow, size_t nDataSize)
    {
        typedef std::pair<size_t,bool> RetType;

        if (type == sc::element_type_empty)
            return RetType(0, false);

        for (size_t i = 0; i < nDataSize; ++i)
        {
            SCROW nRow = nTopRow + i;
            sal_uInt8 nScriptType = mrColumn.GetRangeScriptType(miAttrPos, nRow, nRow);
            if (IsAmbiguousScriptNonZero(nScriptType))
                // Return the offset from the first row.
                return RetType(i, true);
        }

        return RetType(0, false);
    }
};

}

void ScColumn::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    UpdateTransHandler aFunc(*this, rSource, rDest, pUndoDoc);
    sc::ProcessFormula(maCells, aFunc);
}


void ScColumn::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    UpdateGrowHandler aFunc(*this, rArea, nGrowX, nGrowY);
    sc::ProcessFormula(maCells, aFunc);
}


void ScColumn::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    if (nTab >= rCxt.mnInsertPos)
    {
        nTab += rCxt.mnSheets;
        pAttrArray->SetTab(nTab);
    }

    UpdateInsertTabOnlyCells(rCxt);
}

void ScColumn::UpdateInsertTabOnlyCells( sc::RefUpdateInsertTabContext& rCxt )
{
    InsertTabUpdater aFunc(rCxt, maCellTextAttrs, nTab);
    sc::ProcessFormulaEditText(maCells, aFunc);
    if (aFunc.isModified())
        CellStorageModified();
}

void ScColumn::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    if (nTab > rCxt.mnDeletePos)
    {
        nTab -= rCxt.mnSheets;
        pAttrArray->SetTab(nTab);
    }

    DeleteTabUpdater aFunc(rCxt, maCellTextAttrs, nTab);
    sc::ProcessFormulaEditText(maCells, aFunc);
    if (aFunc.isModified())
        CellStorageModified();
}

void ScColumn::UpdateInsertTabAbs(SCTAB nNewPos)
{
    InsertAbsTabUpdater aFunc(maCellTextAttrs, nTab, nNewPos);
    sc::ProcessFormulaEditText(maCells, aFunc);
    if (aFunc.isModified())
        CellStorageModified();
}

void ScColumn::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo )
{
    nTab = nTabNo;
    pAttrArray->SetTab( nTabNo );

    MoveTabUpdater aFunc(rCxt, maCellTextAttrs, nTab);
    sc::ProcessFormulaEditText(maCells, aFunc);
    if (aFunc.isModified())
        CellStorageModified();
}


void ScColumn::UpdateCompile( bool bForceIfNameInUse )
{
    UpdateCompileHandler aFunc(bForceIfNameInUse);
    sc::ProcessFormula(maCells, aFunc);
}


void ScColumn::SetTabNo(SCTAB nNewTab)
{
    nTab = nNewTab;
    pAttrArray->SetTab( nNewTab );

    TabNoSetter aFunc(nTab);
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, std::set<sal_uInt16>& rIndexes) const
{
    UsedRangeNameFinder aFunc(rIndexes);
    sc::ParseFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

void ScColumn::SetDirtyVar()
{
    SetDirtyVarHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

bool ScColumn::IsFormulaDirty( SCROW nRow ) const
{
    if (!ValidRow(nRow))
        return false;

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
        // This is not a formula cell block.
        return false;

    const ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
    return p->GetDirty();
}

void ScColumn::SetDirty()
{
    // is only done documentwide, no FormulaTracking
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    SetDirtyHandler aFunc(*pDocument);
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::SetDirty( SCROW nRow1, SCROW nRow2 )
{
    // broadcasts everything within the range, with FormulaTracking
    sc::AutoCalcSwitch aSwitch(*pDocument, false);

    SetDirtyOnRangeHandler aHdl(*this);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl, aHdl);
    aHdl.broadcast();
}

void ScColumn::SetTableOpDirty( const ScRange& rRange )
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);

    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SetTableOpDirtyOnRangeHandler aHdl(*this);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl, aHdl);
    aHdl.broadcast();
}

void ScColumn::SetDirtyAfterLoad()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    SetDirtyAfterLoadHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

namespace {

class RecalcOnRefMoveCollector
{
    std::vector<SCROW> maDirtyRows;
public:
    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        if (pCell->GetDirty() && pCell->GetCode()->IsRecalcModeOnRefMove())
            maDirtyRows.push_back(nRow);
    }

    const std::vector<SCROW>& getDirtyRows() const
    {
        return maDirtyRows;
    }
};

}

void ScColumn::SetRelNameDirty()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    SetRelNameDirtyHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::BroadcastRecalcOnRefMove()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    RecalcOnRefMoveCollector aFunc;
    sc::ProcessFormula(maCells, aFunc);
    BroadcastCells(aFunc.getDirtyRows());
}

void ScColumn::CalcAll()
{
    CalcAllHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::CompileAll()
{
    CompileAllHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::CompileXML( ScProgress& rProgress )
{
    CompileXMLHandler aFunc(rProgress, *this);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

bool ScColumn::CompileErrorCells(sal_uInt16 nErrCode)
{
    CompileErrorCellsHandler aHdl(*this, nErrCode, pDocument->GetGrammar());
    sc::ProcessFormula(maCells, aHdl);
    return aHdl.isCompiled();
}

void ScColumn::CalcAfterLoad()
{
    CalcAfterLoadHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::ResetChanged( SCROW nStartRow, SCROW nEndRow )
{
    ResetChangedHandler aFunc;
    sc::ProcessFormula(maCells.begin(), maCells, nStartRow, nEndRow, aFunc);
}

bool ScColumn::HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst)
{
    //  used in GetOptimalHeight - ambiguous script type counts as edit cell

    FindEditCellsHandler aFunc(*this, maCellTextAttrs);
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos =
        sc::FindFormulaEditText(maCells, nStartRow, nEndRow, aFunc);

    if (aPos.first == maCells.end())
        return false;

    rFirst = aPos.first->position + aPos.second;
    return true;
}


SCsROW ScColumn::SearchStyle(
    SCsROW nRow, const ScStyleSheet* pSearchStyle, bool bUp, bool bInSelection,
    const ScMarkData& rMark) const
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
            return pAttrArray->SearchStyle(nRow, pSearchStyle, bUp, rMark.GetArray()+nCol);
        else
            return -1;
    }
    else
        return pAttrArray->SearchStyle( nRow, pSearchStyle, bUp, NULL );
}


bool ScColumn::SearchStyleRange(
    SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
    bool bInSelection, const ScMarkData& rMark) const
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
            return pAttrArray->SearchStyleRange(
                rRow, rEndRow, pSearchStyle, bUp, rMark.GetArray() + nCol);
        else
            return false;
    }
    else
        return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp, NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
