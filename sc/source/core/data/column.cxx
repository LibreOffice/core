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
#include "detfunc.hxx"
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
#include <listenercontext.hxx>
#include <refhint.hxx>
#include <stlalgorithm.hxx>
#include <formulagroup.hxx>

#include <svl/poolcach.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fieldupdater.hxx>
#include <osl/diagnose.h>

#include <cstring>
#include <map>
#include <cstdio>
#include <boost/checked_delete.hpp>
#include <memory>

using ::editeng::SvxBorderLine;
using namespace formula;

namespace {

inline bool IsAmbiguousScriptNonZero( SvtScriptType nScript )
{
    //TODO: move to a header file
    return ( nScript != SvtScriptType::LATIN &&
             nScript != SvtScriptType::ASIAN &&
             nScript != SvtScriptType::COMPLEX &&
             nScript != SvtScriptType::NONE );
}

}

ScNeededSizeOptions::ScNeededSizeOptions() :
    pPattern(nullptr), bFormula(false), bSkipMerged(true), bGetFont(true), bTotalSize(false)
{
}

ScColumn::ScColumn() :
    maCellTextAttrs(MAXROWCOUNT),
    maCellNotes(MAXROWCOUNT),
    maBroadcasters(MAXROWCOUNT),
    maCells(MAXROWCOUNT),
    nCol( 0 ),
    nTab( 0 ),
    pAttrArray( nullptr ),
    pDocument( nullptr ),
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
        size_t nEnd = std::min(it->size, nOffset+nRowsToRead); // last row + 1
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
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ) && !bFound)
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
        const ScMultiSel& rMultiSel = rMark.GetMultiSelData();
        if ( rMultiSel.HasMarks( nCol ) )
        {
            ScMultiSelIter aMultiIter( rMultiSel, nCol );
            while (aMultiIter.Next( nTop, nBottom ))
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

const SfxPoolItem& ScColumn::GetAttr( SCROW nRow, sal_uInt16 nWhich ) const
{
    return pAttrArray->GetPattern( nRow )->GetItemSet().Get(nWhich);
}

const ScPatternAttr* ScColumn::GetMostUsedPattern( SCROW nStartRow, SCROW nEndRow ) const
{
    ::std::map< const ScPatternAttr*, size_t > aAttrMap;
    const ScPatternAttr* pMaxPattern = nullptr;
    size_t nMaxCount = 0;

    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow );
    const ScPatternAttr* pPattern;
    SCROW nAttrRow1 = 0, nAttrRow2 = 0;

    while( (pPattern = aAttrIter.Next( nAttrRow1, nAttrRow2 )) != nullptr )
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
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
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
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
            pAttrArray->ChangeIndent(nTop, nBottom, bIncrement);
    }
}

void ScColumn::ClearSelectionItems( const sal_uInt16* pWhich,const ScMarkData& rMark )
{
    SCROW nTop;
    SCROW nBottom;

    if ( pAttrArray && rMark.IsMultiMarked() )
    {
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
            pAttrArray->ClearItems(nTop, nBottom, pWhich);
    }
}

void ScColumn::DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast )
{
    SCROW nTop;
    SCROW nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
            DeleteArea(nTop, nBottom, nDelFlag, bBroadcast);
    }
}

void ScColumn::ApplyPattern( SCROW nRow, const ScPatternAttr& rPatAttr )
{
    const SfxItemSet* pSet = &rPatAttr.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );

    const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );

    //  true = keep old content

    const ScPatternAttr* pNewPattern = static_cast<const ScPatternAttr*>( &aCache.ApplyTo( *pPattern, true ) );
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
    std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr(*pPattern));
    if (pNewPattern)
    {
        pNewPattern->SetStyleSheet(const_cast<ScStyleSheet*>(&rStyle));
        pAttrArray->SetPattern(nRow, pNewPattern.get(), true);
    }
}

void ScColumn::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    pAttrArray->ApplyStyleArea(nStartRow, nEndRow, const_cast<ScStyleSheet*>(&rStyle));
}

void ScColumn::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    SCROW nTop;
    SCROW nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
            pAttrArray->ApplyStyleArea(nTop, nBottom, const_cast<ScStyleSheet*>(&rStyle));
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
        ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
        while (aMultiIter.Next( nTop, nBottom ))
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
        return nullptr;
    }

    bool bEqual = true;

    const ScStyleSheet* pStyle = nullptr;
    const ScStyleSheet* pNewStyle;

    ScMultiSelIter aMultiIter( rMark.GetMultiSelData(), nCol );
    SCROW nTop;
    SCROW nBottom;
    while (bEqual && aMultiIter.Next( nTop, nBottom ))
    {
        ScAttrIterator aAttrIter( pAttrArray, nTop, nBottom );
        SCROW nRow;
        SCROW nDummy;
        const ScPatternAttr* pPattern;
        while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ) != nullptr)
        {
            pNewStyle = pPattern->GetStyleSheet();
            rFound = true;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = false;                                             // difference
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : nullptr;
}

const ScStyleSheet* ScColumn::GetAreaStyle( bool& rFound, SCROW nRow1, SCROW nRow2 ) const
{
    rFound = false;

    bool bEqual = true;

    const ScStyleSheet* pStyle = nullptr;
    const ScStyleSheet* pNewStyle;

    ScAttrIterator aAttrIter( pAttrArray, nRow1, nRow2 );
    SCROW nRow;
    SCROW nDummy;
    const ScPatternAttr* pPattern;
    while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ) != nullptr)
    {
        pNewStyle = pPattern->GetStyleSheet();
        rFound = true;
        if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
            bEqual = false;                                             // difference
        pStyle = pNewStyle;
    }

    return bEqual ? pStyle : nullptr;
}

void ScColumn::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset )
{
    pAttrArray->FindStyleSheet( pStyleSheet, rUsedRows, bReset );
}

bool ScColumn::IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const
{
    return pAttrArray->IsStyleSheetUsed( rStyle, bGatherAllStyles );
}

bool ScColumn::ApplyFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
{
    return pAttrArray->ApplyFlags( nStartRow, nEndRow, nFlags );
}

bool ScColumn::RemoveFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
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
    //TODO: Warning: SfxItemPoolCache seems to create to many Refs for the new SetItem ??

    ScDocumentPool* pDocPool = pDocument->GetPool();

    const ScPatternAttr* pOldPattern = pAttrArray->GetPattern( nRow );
    std::unique_ptr<ScPatternAttr> pTemp(new ScPatternAttr(*pOldPattern));
    pTemp->GetItemSet().Put(rAttr);
    const ScPatternAttr* pNewPattern = static_cast<const ScPatternAttr*>( &pDocPool->Put( *pTemp ) );

    if ( pNewPattern != pOldPattern )
        pAttrArray->SetPattern( nRow, pNewPattern );
    else
        pDocPool->Remove( *pNewPattern );       // free up resources
}

ScRefCellValue ScColumn::GetCellValue( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    if (aPos.first == maCells.end())
        return ScRefCellValue();

    return GetCellValue(aPos.first, aPos.second);
}

ScRefCellValue ScColumn::GetCellValue( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rBlockPos.miCellPos, nRow);
    if (aPos.first == maCells.end())
        return ScRefCellValue();

    rBlockPos.miCellPos = aPos.first; // Store this for next call.
    return GetCellValue(aPos.first, aPos.second);
}

ScRefCellValue ScColumn::GetCellValue( const sc::CellStoreType::const_iterator& itPos, size_t nOffset )
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

const sc::CellTextAttr* ScColumn::GetCellTextAttr( SCROW nRow ) const
{
    sc::ColumnBlockConstPosition aBlockPos;
    aBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    return GetCellTextAttr(aBlockPos, nRow);
}

const sc::CellTextAttr* ScColumn::GetCellTextAttr( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const
{
    sc::CellTextAttrStoreType::const_position_type aPos = maCellTextAttrs.position(rBlockPos.miCellTextAttrPos, nRow);
    if (aPos.first == maCellTextAttrs.end())
        return nullptr;

    rBlockPos.miCellTextAttrPos = aPos.first;

    if (aPos.first->type != sc::element_type_celltextattr)
        return nullptr;

    return &sc::celltextattr_block::at(*aPos.first->data, aPos.second);
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

    return pAttrArray == nullptr || pAttrArray->TestInsertCol(nStartRow, nEndRow);
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

    maCellNotes.insert_empty(nStartRow, nSize);
    maCellNotes.resize(MAXROWCOUNT);

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
    bool mbCopyNotes;

    void setDefaultAttrsToDest(size_t nRow, size_t nSize)
    {
        std::vector<sc::CellTextAttr> aAttrs(nSize); // default values
        maDestPos.miCellTextAttrPos = mrDestCol.GetCellAttrStore().set(
            maDestPos.miCellTextAttrPos, nRow, aAttrs.begin(), aAttrs.end());
    }

    void duplicateNotes(SCROW nStartRow, size_t nDataSize )
    {
        mrSrcCol.DuplicateNotes(nStartRow, nDataSize, mrDestCol, maDestPos, false);
    }

public:
    CopyToClipHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos, bool bCopyNotes) :
        mrSrcCol(rSrcCol), mrDestCol(rDestCol), mpDestPos(pDestPos), mbCopyNotes(bCopyNotes)
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

        bool bSet = true;

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nTopRow, it, itEnd);
            }
            break;
            case sc::element_type_string:
            {
                sc::string_block::const_iterator it = sc::string_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                maDestPos.miCellPos = mrDestCol.GetCellStore().set(maDestPos.miCellPos, nTopRow, it, itEnd);

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
            }
            break;
            default:
                bSet = false;
        }

        if (bSet)
            setDefaultAttrsToDest(nTopRow, nDataSize);

        if (mbCopyNotes)
            duplicateNotes(nTopRow, nDataSize);
    }
};

class CopyTextAttrToClipHandler
{
    sc::CellTextAttrStoreType& mrDestAttrs;
    sc::CellTextAttrStoreType::iterator miPos;

public:
    explicit CopyTextAttrToClipHandler( sc::CellTextAttrStoreType& rAttrs ) :
        mrDestAttrs(rAttrs), miPos(mrDestAttrs.begin()) {}

    void operator() ( const sc::CellTextAttrStoreType::value_type& aNode, size_t nOffset, size_t nDataSize )
    {
        if (aNode.type != sc::element_type_celltextattr)
            return;

        sc::celltextattr_block::const_iterator it = sc::celltextattr_block::begin(*aNode.data);
        std::advance(it, nOffset);
        sc::celltextattr_block::const_iterator itEnd = it;
        std::advance(itEnd, nDataSize);

        size_t nPos = aNode.position + nOffset;
        miPos = mrDestAttrs.set(miPos, nPos, it, itEnd);
    }
};


}

void ScColumn::CopyToClip(
    sc::CopyToClipContext& rCxt, SCROW nRow1, SCROW nRow2, ScColumn& rColumn ) const
{
    pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray,
                          rCxt.isKeepScenarioFlags() ? (ScMF::All & ~ScMF::Scenario) : ScMF::All );

    {
        CopyToClipHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol), rCxt.isCloneNotes());
        sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
    }

    {
        CopyTextAttrToClipHandler aFunc(rColumn.maCellTextAttrs);
        sc::ParseBlock(maCellTextAttrs.begin(), maCellTextAttrs, aFunc, nRow1, nRow2);
    }

    rColumn.CellStorageModified();
}

void ScColumn::CopyStaticToDocument(
    SCROW nRow1, SCROW nRow2, const SvNumberFormatterMergeMap& rMap, ScColumn& rDestCol )
{
    if (nRow1 > nRow2)
        return;

    sc::ColumnBlockPosition aDestPos;
    CopyCellTextAttrsToDocument(nRow1, nRow2, rDestCol);
    CopyCellNotesToDocument(nRow1, nRow2, rDestCol);

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
                std::vector<svl::SharedString> aConverted;
                aConverted.reserve(nDataSize);
                for (; itData != itDataEnd; ++itData)
                {
                    const EditTextObject& rObj = **itData;
                    svl::SharedString aSS = pDocument->GetSharedStringPool().intern(ScEditUtil::GetString(rObj, pDocument));
                    aConverted.push_back(aSS);
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
                    {
                        svl::SharedString aSS = rFC.GetString();
                        if (aSS.isValid())
                            aDestPos.miCellPos = rDestCol.maCells.set(aDestPos.miCellPos, nRow, aSS);
                    }
                }
            }
            break;
            default:
                ;
        }

        if (bLastBlock)
            break;
    }

    // Dont' forget to copy the number formats over.  Charts may reference them.
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        sal_uInt32 nNumFmt = GetNumberFormat(nRow);
        SvNumberFormatterMergeMap::const_iterator itNum = rMap.find(nNumFmt);
        if (itNum != rMap.end())
            nNumFmt = itNum->second;

        rDestCol.SetNumberFormat(nRow, nNumFmt);
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
    {
        rDestCol.maCellTextAttrs.set(nDestRow, maCellTextAttrs.get<sc::CellTextAttr>(nSrcRow));
        ScPostIt* pNote = maCellNotes.get<ScPostIt*>(nSrcRow);
        if (pNote)
        {
            pNote = pNote->Clone(ScAddress(nCol, nSrcRow, nTab),
                                 rDestCol.GetDoc(),
                                 ScAddress(rDestCol.nCol, nDestRow, rDestCol.nTab),
                                 false);
            rDestCol.maCellNotes.set(nDestRow, pNote);
            pNote->UpdateCaptionPos(ScAddress(rDestCol.nCol, nDestRow, rDestCol.nTab));
        }
        else
            rDestCol.maCellNotes.set_empty(nDestRow, nDestRow);
    }
    else
    {
        rDestCol.maCellTextAttrs.set_empty(nDestRow, nDestRow);
        rDestCol.maCellNotes.set_empty(nDestRow, nDestRow);
    }

    rDestCol.CellStorageModified();
}

namespace {

bool canCopyValue(const ScDocument& rDoc, const ScAddress& rPos, InsertDeleteFlags nFlags)
{
    sal_uInt32 nNumIndex = static_cast<const SfxUInt32Item*>(rDoc.GetAttr(rPos, ATTR_VALUE_FORMAT))->GetValue();
    short nType = rDoc.GetFormatTable()->GetType(nNumIndex);
    if ((nType == css::util::NumberFormat::DATE) || (nType == css::util::NumberFormat::TIME) || (nType == css::util::NumberFormat::DATETIME))
        return ((nFlags & InsertDeleteFlags::DATETIME) != InsertDeleteFlags::NONE);

    return (nFlags & InsertDeleteFlags::VALUE) != InsertDeleteFlags::NONE;
}

class CopyAsLinkHandler
{
    const ScColumn& mrSrcCol;
    ScColumn& mrDestCol;
    sc::ColumnBlockPosition maDestPos;
    sc::ColumnBlockPosition* mpDestPos;
    InsertDeleteFlags mnCopyFlags;

    sc::StartListeningType meListenType;

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
        return new ScFormulaCell(&mrDestCol.GetDoc(), ScAddress(mrDestCol.GetCol(), nRow, mrDestCol.GetTab()), aArr);
    }

    void createRefBlock(const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nTopRow = aNode.position + nOffset;

        for (size_t i = 0; i < nDataSize; ++i)
        {
            SCROW nRow = nTopRow + i;
            mrDestCol.SetFormulaCell(maDestPos, nRow, createRefCell(nRow), meListenType);
        }

        setDefaultAttrsToDest(nTopRow, nDataSize);
    }

    void duplicateNotes(SCROW nStartRow, size_t nDataSize, bool bCloneCaption )
    {
        mrSrcCol.DuplicateNotes(nStartRow, nDataSize, mrDestCol, maDestPos, bCloneCaption);
    }

public:
    CopyAsLinkHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos, InsertDeleteFlags nCopyFlags) :
        mrSrcCol(rSrcCol),
        mrDestCol(rDestCol),
        mpDestPos(pDestPos),
        mnCopyFlags(nCopyFlags),
        meListenType(sc::SingleCellListening)
    {
        if (mpDestPos)
            maDestPos = *mpDestPos;
    }

    ~CopyAsLinkHandler()
    {
        if (mpDestPos)
            *mpDestPos = maDestPos;
    }

    void setStartListening( bool b )
    {
        meListenType = b ? sc::SingleCellListening : sc::NoListening;
    }

    void operator() (const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nRow = aNode.position + nOffset;

        if (mnCopyFlags & (InsertDeleteFlags::NOTE|InsertDeleteFlags::ADDNOTES))
        {
            bool bCloneCaption = (mnCopyFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
            duplicateNotes(nRow, nDataSize, bCloneCaption );
        }

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                if ((mnCopyFlags & (InsertDeleteFlags::DATETIME|InsertDeleteFlags::VALUE)) == InsertDeleteFlags::NONE)
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
                if (!(mnCopyFlags & InsertDeleteFlags::STRING))
                    return;

                createRefBlock(aNode, nOffset, nDataSize);
            }
            break;
            case sc::element_type_formula:
            {
                if (!(mnCopyFlags & InsertDeleteFlags::FORMULA))
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
    svl::SharedStringPool* mpSharedStringPool;
    InsertDeleteFlags mnCopyFlags;

    sc::StartListeningType meListenType;

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

        bool bCloneValue          = (mnCopyFlags & InsertDeleteFlags::VALUE) != InsertDeleteFlags::NONE;
        bool bCloneDateTime       = (mnCopyFlags & InsertDeleteFlags::DATETIME) != InsertDeleteFlags::NONE;
        bool bCloneString         = (mnCopyFlags & InsertDeleteFlags::STRING) != InsertDeleteFlags::NONE;
        bool bCloneSpecialBoolean = (mnCopyFlags & InsertDeleteFlags::SPECIAL_BOOLEAN) != InsertDeleteFlags::NONE;
        bool bCloneFormula        = (mnCopyFlags & InsertDeleteFlags::FORMULA) != InsertDeleteFlags::NONE;

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
            mrDestCol.SetFormulaCell(maDestPos, nRow, pCell, meListenType);
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
                mrDestCol.SetFormulaCell(maDestPos, nRow, pErrCell, meListenType);
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
            svl::SharedString aStr = rSrcCell.GetString();
            if (aStr.isEmpty())
                // Don't create empty string cells.
                return;

            if (rSrcCell.IsMultilineResult())
            {
                // Clone as an edit text object.
                EditEngine& rEngine = mrDestCol.GetDoc().GetEditEngine();
                rEngine.SetText(aStr.getString());
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

    void duplicateNotes(SCROW nStartRow, size_t nDataSize, bool bCloneCaption )
    {
        mrSrcCol.DuplicateNotes(nStartRow, nDataSize, mrDestCol, maDestPos, bCloneCaption);
    }

public:
    CopyByCloneHandler(const ScColumn& rSrcCol, ScColumn& rDestCol, sc::ColumnBlockPosition* pDestPos,
            InsertDeleteFlags nCopyFlags, svl::SharedStringPool* pSharedStringPool) :
        mrSrcCol(rSrcCol),
        mrDestCol(rDestCol),
        mpDestPos(pDestPos),
        mpSharedStringPool(pSharedStringPool),
        mnCopyFlags(nCopyFlags),
        meListenType(sc::SingleCellListening)
    {
        if (mpDestPos)
            maDestPos = *mpDestPos;
    }

    ~CopyByCloneHandler()
    {
        if (mpDestPos)
            *mpDestPos = maDestPos;
    }

    void setStartListening( bool b )
    {
        meListenType = b ? sc::SingleCellListening : sc::NoListening;
    }

    void operator() (const sc::CellStoreType::value_type& aNode, size_t nOffset, size_t nDataSize)
    {
        size_t nRow = aNode.position + nOffset;

        if (mnCopyFlags & (InsertDeleteFlags::NOTE|InsertDeleteFlags::ADDNOTES))
        {
            bool bCloneCaption = (mnCopyFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
            duplicateNotes(nRow, nDataSize, bCloneCaption );
        }

        switch (aNode.type)
        {
            case sc::element_type_numeric:
            {
                if ((mnCopyFlags & (InsertDeleteFlags::DATETIME|InsertDeleteFlags::VALUE)) == InsertDeleteFlags::NONE)
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
                if (!(mnCopyFlags & InsertDeleteFlags::STRING))
                    return;

                sc::string_block::const_iterator it = sc::string_block::begin(*aNode.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it, ++nRow)
                {
                    const svl::SharedString& rStr = *it;
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
                        if (mpSharedStringPool)
                        {
                            // Re-intern the string if source is a different document.
                            svl::SharedString aInterned = mpSharedStringPool->intern( rStr.getString());
                            maDestPos.miCellPos =
                                mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, aInterned);
                        }
                        else
                        {
                            maDestPos.miCellPos =
                                mrDestCol.GetCellStore().set(maDestPos.miCellPos, nRow, rStr);
                        }
                        setDefaultAttrToDest(nRow);
                    }
                }
            }
            break;
            case sc::element_type_edittext:
            {
                if (!(mnCopyFlags & InsertDeleteFlags::STRING))
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
    SCROW nRow1, SCROW nRow2, InsertDeleteFlags nFlags, bool bMarked, ScColumn& rColumn,
    const ScMarkData* pMarkData, bool bAsLink) const
{
    if (bMarked)
    {
        SCROW nStart, nEnd;
        if (pMarkData && pMarkData->IsMultiMarked())
        {
            ScMultiSelIter aIter( pMarkData->GetMultiSelData(), nCol );

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

    if ( (nFlags & InsertDeleteFlags::ATTRIB) != InsertDeleteFlags::NONE )
    {
        if ( (nFlags & InsertDeleteFlags::STYLES) != InsertDeleteFlags::STYLES )
        {   // keep the StyleSheets in the target document
            // e.g. DIF and RTF Clipboard-Import
            for ( SCROW nRow = nRow1; nRow <= nRow2; nRow++ )
            {
                const ScStyleSheet* pStyle =
                    rColumn.pAttrArray->GetPattern( nRow )->GetStyleSheet();
                const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
                std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr( *pPattern ));
                pNewPattern->SetStyleSheet( const_cast<ScStyleSheet*>(pStyle) );
                rColumn.pAttrArray->SetPattern( nRow, pNewPattern.get(), true );
            }
        }
        else
            pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray);
    }

    if ((nFlags & InsertDeleteFlags::CONTENTS) != InsertDeleteFlags::NONE)
    {
        if (bAsLink)
        {
            CopyAsLinkHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol), nFlags);
            aFunc.setStartListening(rCxt.isStartListening());
            sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
        }
        else
        {
            // Compare the ScDocumentPool* to determine if we are copying
            // within the same document. If not, re-intern shared strings.
            svl::SharedStringPool* pSharedStringPool =
                (pDocument->GetPool() != rColumn.pDocument->GetPool()) ?
                &rColumn.pDocument->GetSharedStringPool() : nullptr;
            CopyByCloneHandler aFunc(*this, rColumn, rCxt.getBlockPosition(rColumn.nTab, rColumn.nCol), nFlags,
                    pSharedStringPool);
            aFunc.setStartListening(rCxt.isStartListening());
            sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
        }

        rColumn.CellStorageModified();
    }
}

void ScColumn::UndoToColumn(
    sc::CopyToDocContext& rCxt, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nFlags, bool bMarked,
    ScColumn& rColumn, const ScMarkData* pMarkData ) const
{
    if (nRow1 > 0)
        CopyToColumn(rCxt, 0, nRow1-1, InsertDeleteFlags::FORMULA, false, rColumn);

    CopyToColumn(rCxt, nRow1, nRow2, nFlags, bMarked, rColumn, pMarkData);      //TODO: bMarked ????

    if (nRow2 < MAXROW)
        CopyToColumn(rCxt, nRow2+1, MAXROW, InsertDeleteFlags::FORMULA, false, rColumn);
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

    bool bCopyNotes = true;
    CopyToClipHandler aFunc(*this, rDestCol, nullptr, bCopyNotes);
    sc::CellStoreType::const_iterator itPos = maCells.begin();
    sc::SingleColumnSpanSet::SpansType::const_iterator it = aRanges.begin(), itEnd = aRanges.end();
    for (; it != itEnd; ++it)
        itPos = sc::ParseBlock(itPos, maCells, aFunc, it->mnRow1, it->mnRow2);

    rDestCol.CellStorageModified();
}

void ScColumn::CopyScenarioFrom( const ScColumn& rSrcCol )
{
    //  This is the scenario table, the data is copied into it
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( static_cast<const ScMergeFlagAttr&>(pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            DeleteArea( nStart, nEnd, InsertDeleteFlags::CONTENTS );
            sc::CopyToDocContext aCxt(*pDocument);
            ((ScColumn&)rSrcCol).
                CopyToColumn(aCxt, nStart, nEnd, InsertDeleteFlags::CONTENTS, false, *this);

            //  UpdateUsed not needed, already done in TestCopyScenario (obsolete comment ?)

            sc::RefUpdateContext aRefCxt(*pDocument);
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(nCol, nStart, nTab, nCol, nEnd, nTab);
            aRefCxt.mnTabDelta = nTab - rSrcCol.nTab;
            UpdateReferenceOnCopy(aRefCxt);
            UpdateCompile();
        }

        //TODO: make CopyToColumn "const" !!! (obsolete comment ?)

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}

void ScColumn::CopyScenarioTo( ScColumn& rDestCol ) const
{
    //  This is the scenario table, the data is copied to the other
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( static_cast<const ScMergeFlagAttr&>(pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            rDestCol.DeleteArea( nStart, nEnd, InsertDeleteFlags::CONTENTS );
            sc::CopyToDocContext aCxt(*rDestCol.pDocument);
            CopyToColumn(aCxt, nStart, nEnd, InsertDeleteFlags::CONTENTS, false, rDestCol);

            //  UpdateUsed not needed, is already done in TestCopyScenario (obsolete comment ?)

            sc::RefUpdateContext aRefCxt(*pDocument);
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(rDestCol.nCol, nStart, rDestCol.nTab, rDestCol.nCol, nEnd, rDestCol.nTab);
            aRefCxt.mnTabDelta = rDestCol.nTab - nTab;
            rDestCol.UpdateReferenceOnCopy(aRefCxt);
            rDestCol.UpdateCompile();
        }

        //TODO: make CopyToColumn "const" !!! (obsolete comment ?)

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
        if ( static_cast<const ScMergeFlagAttr&>(pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
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
        if ( static_cast<const ScMergeFlagAttr&>(pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            aRange.aStart.SetRow( nStart );
            aRange.aEnd.SetRow( nEnd );
            rDestMark.SetMultiMarkArea( aRange );
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

class NoteCaptionUpdater
{
    SCCOL mnCol;
    SCTAB mnTab;
public:
    NoteCaptionUpdater( SCCOL nCol, SCTAB nTab ) : mnCol(nCol), mnTab(nTab) {}

    void operator() ( size_t nRow, ScPostIt* p )
    {
        p->UpdateCaptionPos(ScAddress(mnCol,nRow,mnTab));
    }
};

}

void ScColumn::UpdateNoteCaptions( SCROW nRow1, SCROW nRow2 )
{
    NoteCaptionUpdater aFunc(nCol, nTab);
    sc::ProcessNote(maCellNotes.begin(), maCellNotes, nRow1, nRow2, aFunc);
}

void ScColumn::SwapCol(ScColumn& rCol)
{
    maBroadcasters.swap(rCol.maBroadcasters);
    maCells.swap(rCol.maCells);
    maCellTextAttrs.swap(rCol.maCellTextAttrs);
    maCellNotes.swap(rCol.maCellNotes);

    // notes update caption
    UpdateNoteCaptions(0, MAXROW);
    rCol.UpdateNoteCaptions(0, MAXROW);

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
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, nullptr);
    if (ValidRow(nEndRow+1))
    {
        aPos = maCells.position(aPos.first, nEndRow+1);
        sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, nullptr);
    }

    // Do the same with the destination column.
    aPos = rCol.maCells.position(nStartRow);
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, nullptr);
    if (ValidRow(nEndRow+1))
    {
        aPos = rCol.maCells.position(aPos.first, nEndRow+1);
        sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, nullptr);
    }

    // Move the broadcasters to the destination column.
    maBroadcasters.transfer(nStartRow, nEndRow, rCol.maBroadcasters, nStartRow);
    maCells.transfer(nStartRow, nEndRow, rCol.maCells, nStartRow);
    maCellTextAttrs.transfer(nStartRow, nEndRow, rCol.maCellTextAttrs, nStartRow);

    // move the notes to the destination column
    maCellNotes.transfer(nStartRow, nEndRow, rCol.maCellNotes, nStartRow);
    UpdateNoteCaptions(0, MAXROW);

    // Re-group transferred formula cells.
    aPos = rCol.maCells.position(nStartRow);
    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    if (ValidRow(nEndRow+1))
    {
        aPos = rCol.maCells.position(aPos.first, nEndRow+1);
        sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    }

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

class SharedTopFormulaCellPicker : public std::unary_function<sc::CellStoreType::value_type, void>
{
public:
    virtual ~SharedTopFormulaCellPicker() {}

    void operator() ( sc::CellStoreType::value_type& node )
    {
        if (node.type != sc::element_type_formula)
            return;

        size_t nTopRow = node.position;

        sc::formula_block::iterator itBeg = sc::formula_block::begin(*node.data);
        sc::formula_block::iterator itEnd = sc::formula_block::end(*node.data);

        // Only pick shared formula cells that are the top cells of their
        // respective shared ranges.
        for (sc::formula_block::iterator it = itBeg; it != itEnd; ++it)
        {
            ScFormulaCell* pCell = *it;
            size_t nRow = nTopRow + std::distance(itBeg, it);
            if (!pCell->IsShared())
            {
                processNonShared(pCell, nRow);
                continue;
            }

            if (pCell->IsSharedTop())
            {
                ScFormulaCell** pp = &(*it);
                processSharedTop(pp, nRow, pCell->GetSharedLength());

                // Move to the last cell in the group, to get incremented to
                // the next cell in the next iteration.
                size_t nOffsetToLast = pCell->GetSharedLength() - 1;
                std::advance(it, nOffsetToLast);
            }
        }
    }

    virtual void processNonShared( ScFormulaCell* /*pCell*/, size_t /*nRow*/ ) {}
    virtual void processSharedTop( ScFormulaCell** /*ppCells*/, size_t /*nRow*/, size_t /*nLength*/ ) {}
};

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

class UpdateRefOnNonCopy : public std::unary_function<sc::FormulaGroupEntry, void>
{
    SCCOL mnCol;
    SCROW mnTab;
    const sc::RefUpdateContext* mpCxt;
    ScDocument* mpUndoDoc;
    bool mbUpdated;
    bool mbClipboardSource;

    void recompileTokenArray( ScFormulaCell& rTopCell )
    {
        // We need to re-compile the token array when a range name is
        // modified, to correctly reflect the new references in the
        // name.
        ScCompiler aComp(&mpCxt->mrDoc, rTopCell.aPos, *rTopCell.GetCode());
        aComp.SetGrammar(mpCxt->mrDoc.GetGrammar());
        aComp.CompileTokenArray();
    }

    void updateRefOnShift( sc::FormulaGroupEntry& rGroup )
    {
        if (!rGroup.mbShared)
        {
            ScAddress aUndoPos(mnCol, rGroup.mnRow, mnTab);
            mbUpdated |= rGroup.mpCell->UpdateReferenceOnShift(*mpCxt, mpUndoDoc, &aUndoPos);
            return;
        }

        // Update references of a formula group.
        ScFormulaCell** pp = rGroup.mpCells;
        ScFormulaCell** ppEnd = pp + rGroup.mnLength;
        ScFormulaCell* pTop = *pp;
        ScTokenArray* pCode = pTop->GetCode();
        std::unique_ptr<ScTokenArray> pOldCode(pCode->Clone());
        ScAddress aOldPos = pTop->aPos;

        // Run this before the position gets updated.
        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnShift(*mpCxt, aOldPos);

        if (pTop->UpdatePosOnShift(*mpCxt))
        {
            ScAddress aErrorPos( ScAddress::UNINITIALIZED );
            // Update the positions of all formula cells.
            for (++pp; pp != ppEnd; ++pp) // skip the top cell.
            {
                ScFormulaCell* pFC = *pp;
                if (!pFC->aPos.Move(mpCxt->mnColDelta, mpCxt->mnRowDelta, mpCxt->mnTabDelta, aErrorPos))
                {
                    assert(!"can't move formula cell");
                }
            }

            if (pCode->IsRecalcModeOnRefMove())
                aRes.mbValueChanged = true;
        }
        else if (aRes.mbReferenceModified && pCode->IsRecalcModeOnRefMove())
        {
            // The cell itself hasn't shifted. But it may have ROW or COLUMN
            // referencing another cell that has.
            aRes.mbValueChanged = true;
        }

        if (aRes.mbNameModified)
            recompileTokenArray(*pTop);

        if (aRes.mbReferenceModified || aRes.mbNameModified)
        {
            sc::EndListeningContext aEndCxt(mpCxt->mrDoc, pOldCode.get());
            aEndCxt.setPositionDelta(
                ScAddress(-mpCxt->mnColDelta, -mpCxt->mnRowDelta, -mpCxt->mnTabDelta));

            for (pp = rGroup.mpCells; pp != ppEnd; ++pp)
            {
                ScFormulaCell* p = *pp;
                p->EndListeningTo(aEndCxt);
                p->SetNeedsListening(true);
            }

            mbUpdated = true;

            fillUndoDoc(aOldPos, rGroup.mnLength, *pOldCode);
        }

        if (aRes.mbValueChanged)
        {
            for (pp = rGroup.mpCells; pp != ppEnd; ++pp)
            {
                ScFormulaCell* p = *pp;
                p->SetNeedsDirty(true);
            }
        }
    }

    void updateRefOnMove( sc::FormulaGroupEntry& rGroup )
    {
        if (!rGroup.mbShared)
        {
            ScAddress aUndoPos(mnCol, rGroup.mnRow, mnTab);
            mbUpdated |= rGroup.mpCell->UpdateReferenceOnMove(*mpCxt, mpUndoDoc, &aUndoPos);
            return;
        }

        // Update references of a formula group.
        ScFormulaCell** pp = rGroup.mpCells;
        ScFormulaCell** ppEnd = pp + rGroup.mnLength;
        ScFormulaCell* pTop = *pp;
        ScTokenArray* pCode = pTop->GetCode();
        std::unique_ptr<ScTokenArray> pOldCode(pCode->Clone());

        ScAddress aPos = pTop->aPos;
        ScAddress aOldPos = aPos;

        bool bCellMoved;
        if (mpCxt->maRange.In(aPos))
        {
            bCellMoved = true;

            // The cell is being moved or copied to a new position. The
            // position has already been updated prior to this call.
            // Determine its original position before the move which will be
            // used to adjust relative references later.

            aOldPos.Set(
                aPos.Col() - mpCxt->mnColDelta,
                aPos.Row() - mpCxt->mnRowDelta,
                aPos.Tab() - mpCxt->mnTabDelta);
        }
        else
        {
            bCellMoved = false;
        }

        bool bRecalcOnMove = pCode->IsRecalcModeOnRefMove();
        if (bRecalcOnMove)
            bRecalcOnMove = aPos != aOldPos;

        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMove(*mpCxt, aOldPos, aPos);

        if (aRes.mbReferenceModified || aRes.mbNameModified || bRecalcOnMove)
        {
            sc::AutoCalcSwitch aACSwitch(mpCxt->mrDoc, false);

            if (aRes.mbNameModified)
                recompileTokenArray(*pTop);

            // Perform end-listening, start-listening, and dirtying on all
            // formula cells in the group.

            // Make sure that the start and end listening contexts share the
            // same block position set, else an invalid iterator may ensue.
            std::shared_ptr<sc::ColumnBlockPositionSet> pPosSet(
                new sc::ColumnBlockPositionSet(mpCxt->mrDoc));

            sc::StartListeningContext aStartCxt(mpCxt->mrDoc, pPosSet);
            sc::EndListeningContext aEndCxt(mpCxt->mrDoc, pPosSet, pOldCode.get());

            aEndCxt.setPositionDelta(
                ScAddress(-mpCxt->mnColDelta, -mpCxt->mnRowDelta, -mpCxt->mnTabDelta));

            for (; pp != ppEnd; ++pp)
            {
                ScFormulaCell* p = *pp;
                p->EndListeningTo(aEndCxt);
                p->StartListeningTo(aStartCxt);
                p->SetDirty();
            }

            // Move from clipboard is Cut&Paste, then do not copy the original
            // positions' formula cells to the Undo document.
            if (!mbClipboardSource || !bCellMoved)
                fillUndoDoc(aOldPos, rGroup.mnLength, *pOldCode);
        }
    }

    void fillUndoDoc( const ScAddress& rOldPos, SCROW nLength, const ScTokenArray& rOldCode )
    {
        if (!mpUndoDoc || nLength <= 0)
            return;

        // Insert the old formula group into the undo document.
        ScAddress aUndoPos = rOldPos;
        ScFormulaCell* pFC = new ScFormulaCell(mpUndoDoc, aUndoPos, rOldCode.Clone());

        if (nLength == 1)
        {
            mpUndoDoc->SetFormulaCell(aUndoPos, pFC);
            return;
        }

        std::vector<ScFormulaCell*> aCells;
        aCells.reserve(nLength);
        ScFormulaCellGroupRef xGroup = pFC->CreateCellGroup(nLength, false);
        aCells.push_back(pFC);
        aUndoPos.IncRow();
        for (SCROW i = 1; i < nLength; ++i, aUndoPos.IncRow())
        {
            pFC = new ScFormulaCell(mpUndoDoc, aUndoPos, xGroup);
            aCells.push_back(pFC);
        }

        if (!mpUndoDoc->SetFormulaCells(rOldPos, aCells))
            // Insertion failed.  Delete all formula cells.
            std::for_each(aCells.begin(), aCells.end(), boost::checked_deleter<ScFormulaCell>());
    }

public:
    UpdateRefOnNonCopy(
        SCCOL nCol, SCTAB nTab, const sc::RefUpdateContext* pCxt,
        ScDocument* pUndoDoc) :
        mnCol(nCol), mnTab(nTab), mpCxt(pCxt),
        mpUndoDoc(pUndoDoc), mbUpdated(false),
        mbClipboardSource(pCxt->mrDoc.IsClipboardSource()){}

    void operator() ( sc::FormulaGroupEntry& rGroup )
    {
        switch (mpCxt->meMode)
        {
            case URM_INSDEL:
                updateRefOnShift(rGroup);
                return;
            case URM_MOVE:
                updateRefOnMove(rGroup);
                return;
            default:
                ;
        }

        if (rGroup.mbShared)
        {
            ScAddress aUndoPos(mnCol, rGroup.mnRow, mnTab);
            ScFormulaCell** pp = rGroup.mpCells;
            ScFormulaCell** ppEnd = pp + rGroup.mnLength;
            for (; pp != ppEnd; ++pp, aUndoPos.IncRow())
            {
                ScFormulaCell* p = *pp;
                mbUpdated |= p->UpdateReference(*mpCxt, mpUndoDoc, &aUndoPos);
            }
        }
        else
        {
            ScAddress aUndoPos(mnCol, rGroup.mnRow, mnTab);
            mbUpdated |= rGroup.mpCell->UpdateReference(*mpCxt, mpUndoDoc, &aUndoPos);
        }
    }

    bool isUpdated() const { return mbUpdated; }
};

class UpdateRefGroupBoundChecker : public SharedTopFormulaCellPicker
{
    const sc::RefUpdateContext& mrCxt;
    std::vector<SCROW>& mrBounds;

public:
    UpdateRefGroupBoundChecker(const sc::RefUpdateContext& rCxt, std::vector<SCROW>& rBounds) :
        mrCxt(rCxt), mrBounds(rBounds) {}

    virtual ~UpdateRefGroupBoundChecker() {}

    virtual void processSharedTop( ScFormulaCell** ppCells, size_t /*nRow*/, size_t /*nLength*/ ) override
    {
        // Check its tokens and record its reference boundaries.
        ScFormulaCell& rCell = **ppCells;
        const ScTokenArray& rCode = *rCell.GetCode();
        rCode.CheckRelativeReferenceBounds(
            mrCxt, rCell.aPos, rCell.GetSharedLength(), mrBounds);
    }
};

class UpdateRefExpandGroupBoundChecker : public SharedTopFormulaCellPicker
{
    const sc::RefUpdateContext& mrCxt;
    std::vector<SCROW>& mrBounds;

public:
    UpdateRefExpandGroupBoundChecker(const sc::RefUpdateContext& rCxt, std::vector<SCROW>& rBounds) :
        mrCxt(rCxt), mrBounds(rBounds) {}

    virtual ~UpdateRefExpandGroupBoundChecker() {}

    virtual void processSharedTop( ScFormulaCell** ppCells, size_t /*nRow*/, size_t /*nLength*/ ) override
    {
        // Check its tokens and record its reference boundaries.
        ScFormulaCell& rCell = **ppCells;
        const ScTokenArray& rCode = *rCell.GetCode();
        rCode.CheckExpandReferenceBounds(
            mrCxt, rCell.aPos, rCell.GetSharedLength(), mrBounds);
    }
};

class FormulaGroupPicker : public SharedTopFormulaCellPicker
{
    std::vector<sc::FormulaGroupEntry>& mrGroups;

public:
    explicit FormulaGroupPicker( std::vector<sc::FormulaGroupEntry>& rGroups ) : mrGroups(rGroups) {}

    virtual ~FormulaGroupPicker() {}

    virtual void processNonShared( ScFormulaCell* pCell, size_t nRow ) override
    {
        mrGroups.push_back(sc::FormulaGroupEntry(pCell, nRow));
    }

    virtual void processSharedTop( ScFormulaCell** ppCells, size_t nRow, size_t nLength ) override
    {
        mrGroups.push_back(sc::FormulaGroupEntry(ppCells, nRow, nLength));
    }
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

bool ScColumn::UpdateReference( sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc )
{
    if (rCxt.meMode == URM_COPY)
        return UpdateReferenceOnCopy(rCxt, pUndoDoc);

    if (IsEmptyData() || pDocument->IsClipOrUndo())
        // Cells in this column are all empty, or clip or undo doc. No update needed.
        return false;

    std::vector<SCROW> aBounds;

    bool bThisColShifted = (rCxt.maRange.aStart.Tab() <= nTab && nTab <= rCxt.maRange.aEnd.Tab() &&
                            rCxt.maRange.aStart.Col() <= nCol && nCol <= rCxt.maRange.aEnd.Col());
    if (bThisColShifted)
    {
        // Cells in this column is being shifted.  Split formula grouping at
        // the top and bottom boundaries before they get shifted.
        // Also, for deleted rows split at the top of the deleted area to adapt
        // the affected group length.
        SCROW nSplitPos;
        if (rCxt.mnRowDelta < 0)
        {
            nSplitPos = rCxt.maRange.aStart.Row() + rCxt.mnRowDelta;
            if (ValidRow(nSplitPos))
                aBounds.push_back(nSplitPos);
        }
        nSplitPos = rCxt.maRange.aStart.Row();
        if (ValidRow(nSplitPos))
        {
            aBounds.push_back(nSplitPos);
            nSplitPos = rCxt.maRange.aEnd.Row() + 1;
            if (ValidRow(nSplitPos))
                aBounds.push_back(nSplitPos);
        }
    }

    // Check the row positions at which the group must be split per relative
    // references.
    UpdateRefGroupBoundChecker aBoundChecker(rCxt, aBounds);
    std::for_each(maCells.begin(), maCells.end(), aBoundChecker);

    // If expand reference edges is on, splitting groups may happen anywhere
    // where a reference points to an adjacent row of the insertion.
    if (rCxt.mnRowDelta > 0 && rCxt.mrDoc.IsExpandRefs())
    {
        UpdateRefExpandGroupBoundChecker aExpandChecker(rCxt, aBounds);
        std::for_each(maCells.begin(), maCells.end(), aExpandChecker);
    }

    // Do the actual splitting.
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);

    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    // Process all collected formula groups.
    UpdateRefOnNonCopy aHandler(nCol, nTab, &rCxt, pUndoDoc);
    aHandler = std::for_each(aGroups.begin(), aGroups.end(), aHandler);
    if (aHandler.isUpdated())
        rCxt.maRegroupCols.set(nTab, nCol);

    return aHandler.isUpdated();
}

std::vector<sc::FormulaGroupEntry> ScColumn::GetFormulaGroupEntries()
{
    std::vector<sc::FormulaGroupEntry> aGroups;
    std::for_each(maCells.begin(), maCells.end(), FormulaGroupPicker(aGroups));
    return aGroups;
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
        ScColumn::JoinNewFormulaCell(aPos, *pCell);
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
        ScColumn::JoinNewFormulaCell(aPos, *pCell);
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
    explicit UpdateCompileHandler(bool bForceIfNameInUse) :
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
    explicit TabNoSetter(SCTAB nTab) : mnTab(nTab) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->aPos.SetTab(mnTab);
    }
};

class UsedRangeNameFinder
{
    std::set<sal_uInt16>& mrIndexes;
public:
    explicit UsedRangeNameFinder(std::set<sal_uInt16>& rIndexes) : mrIndexes(rIndexes) {}

    void operator() (size_t /*nRow*/, const ScFormulaCell* pCell)
    {
        pCell->FindRangeNamesInUse(mrIndexes);
    }
};

class CheckVectorizationHandler
{
public:
    CheckVectorizationHandler()
    {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        ScTokenArray* pCode = p->GetCode();
        if (pCode != nullptr && pCode->GetVectorState() == FormulaVectorDisabled)
        {
            pCode->ResetVectorState();
            FormulaToken* pFT = pCode->First();
            while (pFT != nullptr)
            {
                pCode->CheckToken(*pFT);
                pFT = pCode->Next();
            }
        }
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
    const sc::SetFormulaDirtyContext& mrCxt;
public:
    SetDirtyHandler( ScDocument& rDoc, const sc::SetFormulaDirtyContext& rCxt ) :
        mrDoc(rDoc), mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        if (mrCxt.mbClearTabDeletedFlag)
        {
            if (!p->IsShared() || p->IsSharedTop())
            {
                ScTokenArray* pCode = p->GetCode();
                pCode->ClearTabDeleted(
                    p->aPos, mrCxt.mnTabDeletedStart, mrCxt.mnTabDeletedEnd);
            }
        }

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
    explicit SetDirtyOnRangeHandler(ScColumn& rColumn) : mrColumn(rColumn) {}

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
        mrColumn.BroadcastCells(aRows, SC_HINT_DATACHANGED);
    }

    void fillBroadcastSpans( sc::ColumnSpanSet& rBroadcastSpans ) const
    {
        SCCOL nCol = mrColumn.GetCol();
        SCTAB nTab = mrColumn.GetTab();
        sc::SingleColumnSpanSet::SpansType aSpans;
        maValueRanges.getSpans(aSpans);

        sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
        for (; it != itEnd; ++it)
            rBroadcastSpans.set(nTab, nCol, it->mnRow1, it->mnRow2, true);
    }
};

class SetTableOpDirtyOnRangeHandler
{
    sc::SingleColumnSpanSet maValueRanges;
    ScColumn& mrColumn;
public:
    explicit SetTableOpDirtyOnRangeHandler(ScColumn& rColumn) : mrColumn(rColumn) {}

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
        mrColumn.BroadcastCells(aRows, SC_HINT_TABLEOPDIRTY);
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

        // If the cell was already dirty because of CalcAfterLoad,
        // FormulaTracking has to take place.
        if (pCell->GetDirty())
            pCell->SetDirty();
#endif
    }
};

struct SetDirtyIfPostponedHandler
{
    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        if (pCell->IsPostponedDirty() || pCell->HasRelNameReference())
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

class CompileAllHandler
{
    sc::CompileFormulaContext& mrCxt;
public:
    explicit CompileAllHandler( sc::CompileFormulaContext& rCxt ) : mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        // for unconditional compilation
        // bCompile=true and pCode->nError=0
        pCell->GetCode()->SetCodeError(0);
        pCell->SetCompile(true);
        pCell->CompileTokenArray(mrCxt);
    }
};

class CompileXMLHandler
{
    sc::CompileFormulaContext& mrCxt;
    ScProgress& mrProgress;
    const ScColumn& mrCol;
public:
    CompileXMLHandler( sc::CompileFormulaContext& rCxt, ScProgress& rProgress, const ScColumn& rCol) :
        mrCxt(rCxt),
        mrProgress(rProgress),
        mrCol(rCol) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        sal_uInt32 nFormat = mrCol.GetNumberFormat(nRow);
        if( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
            // Non-default number format is set.
            pCell->SetNeedNumberFormat(false);
        else if (pCell->NeedsNumberFormat())
            pCell->SetDirtyVar();

        if (pCell->GetMatrixFlag())
            pCell->SetDirtyVar();

        pCell->CompileXML(mrCxt, mrProgress);
    }
};

class CompileErrorCellsHandler
{
    sc::CompileFormulaContext& mrCxt;
    ScColumn& mrColumn;
    sc::CellStoreType::iterator miPos;
    sal_uInt16 mnErrCode;
    bool mbCompiled;
public:
    CompileErrorCellsHandler( sc::CompileFormulaContext& rCxt, ScColumn& rColumn, sal_uInt16 nErrCode ) :
        mrCxt(rCxt),
        mrColumn(rColumn),
        miPos(mrColumn.GetCellStore().begin()),
        mnErrCode(nErrCode),
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
        OUString aFormula = pCell->GetFormula(mrCxt);
        pCell->Compile(mrCxt, aFormula);
        ScColumn::JoinNewFormulaCell(aPos, *pCell);

        mbCompiled = true;
    }

    bool isCompiled() const { return mbCompiled; }
};

class CalcAfterLoadHandler
{
    sc::CompileFormulaContext& mrCxt;
    bool mbStartListening;

public:
    CalcAfterLoadHandler( sc::CompileFormulaContext& rCxt, bool bStartListening ) :
        mrCxt(rCxt), mbStartListening(bStartListening) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->CalcAfterLoad(mrCxt, mbStartListening);
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
    sc::CellStoreType::iterator miCellPos;

public:
    explicit FindEditCellsHandler(ScColumn& rCol) :
        mrColumn(rCol),
        miAttrPos(rCol.GetCellAttrStore().begin()),
        miCellPos(rCol.GetCellStore().begin()) {}

    bool operator() (size_t, const EditTextObject*)
    {
        // This is definitely an edit text cell.
        return true;
    }

    bool operator() (size_t nRow, const ScFormulaCell* p)
    {
        // With a formula cell, it's considered an edit text cell when either
        // the result is multi-line or it has more than one script types.
        SvtScriptType nScriptType = mrColumn.GetRangeScriptType(miAttrPos, nRow, nRow, miCellPos);
        if (IsAmbiguousScriptNonZero(nScriptType))
            return true;

        return const_cast<ScFormulaCell*>(p)->IsMultilineResult();
    }

    /**
     * Callback for a block of other types.
     */
    std::pair<size_t,bool> operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        typedef std::pair<size_t,bool> RetType;

        if (node.type == sc::element_type_empty)
            // Ignore empty blocks.
            return RetType(0, false);

        // Check the script type of a non-empty element and see if it has
        // multiple script types.
        for (size_t i = 0; i < nDataSize; ++i)
        {
            SCROW nRow = node.position + i + nOffset;
            SvtScriptType nScriptType = mrColumn.GetRangeScriptType(miAttrPos, nRow, nRow, miCellPos);
            if (IsAmbiguousScriptNonZero(nScriptType))
                // Return the offset from the first row.
                return RetType(i+nOffset, true);
        }

        // No edit text cell found.
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

void ScColumn::CheckVectorizationState()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    CheckVectorizationHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt )
{
    // is only done documentwide, no FormulaTracking
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    SetDirtyHandler aFunc(*pDocument, rCxt);
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::SetDirtyFromClip( SCROW nRow1, SCROW nRow2, sc::ColumnSpanSet& rBroadcastSpans )
{
    // Set all formula cells in the range dirty, and pick up all non-formula
    // cells for later broadcasting.  We don't broadcast here.
    sc::AutoCalcSwitch aSwitch(*pDocument, false);

    SetDirtyOnRangeHandler aHdl(*this);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl, aHdl);
    aHdl.fillBroadcastSpans(rBroadcastSpans);
}

namespace {

class BroadcastBroadcastersHandler
{
    ScHint&     mrHint;
    ScAddress&  mrAddress;
    bool        mbBroadcasted;

public:
    explicit BroadcastBroadcastersHandler( ScHint& rHint )
        : mrHint(rHint)
        , mrAddress(mrHint.GetAddress())
        , mbBroadcasted(false)
    {
    }

    void operator() ( size_t nRow, SvtBroadcaster* pBroadcaster )
    {
        mrAddress.SetRow(nRow);
        pBroadcaster->Broadcast(mrHint);
        mbBroadcasted = true;
    }

    bool wasBroadcasted() { return mbBroadcasted; }
};

}

bool ScColumn::BroadcastBroadcasters( SCROW nRow1, SCROW nRow2, ScHint& rHint )
{
    rHint.GetAddress().SetCol(nCol);
    BroadcastBroadcastersHandler aBroadcasterHdl( rHint);
    sc::ProcessBroadcaster(maBroadcasters.begin(), maBroadcasters, nRow1, nRow2, aBroadcasterHdl);
    return aBroadcasterHdl.wasBroadcasted();
}

void ScColumn::SetDirty( SCROW nRow1, SCROW nRow2, BroadcastMode eMode )
{
    // broadcasts everything within the range, with FormulaTracking
    sc::AutoCalcSwitch aSwitch(*pDocument, false);

    switch (eMode)
    {
        case BROADCAST_NONE:
            {
                // Handler only used with formula cells.
                SetDirtyOnRangeHandler aHdl(*this);
                sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl);
            }
            break;
        case BROADCAST_DATA_POSITIONS:
            {
                // Handler used with both, formula and non-formula cells.
                SetDirtyOnRangeHandler aHdl(*this);
                sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl, aHdl);
                aHdl.broadcast();
            }
            break;
        case BROADCAST_BROADCASTERS:
            {
                // Handler only used with formula cells.
                SetDirtyOnRangeHandler aHdl(*this);
                sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aHdl);
                // Broadcast all broadcasters in range.
                ScHint aHint( SC_HINT_DATACHANGED, ScAddress( nCol, nRow1, nTab));
                if (BroadcastBroadcasters( nRow1, nRow2, aHint))
                {
                    // SetDirtyOnRangeHandler implicitly tracks notified
                    // formulas via ScDocument::Broadcast(), which
                    // BroadcastBroadcastersHandler doesn't, so explicitly
                    // track them here.
                    pDocument->TrackFormulas();
                }
            }
            break;
    }
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

void ScColumn::SetDirtyIfPostponed()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    SetDirtyIfPostponedHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::BroadcastRecalcOnRefMove()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    RecalcOnRefMoveCollector aFunc;
    sc::ProcessFormula(maCells, aFunc);
    BroadcastCells(aFunc.getDirtyRows(), SC_HINT_DATACHANGED);
}

namespace {

class TransferListenersHandler
{
public:
    typedef std::vector<SvtListener*> ListenersType;
    struct Entry
    {
        size_t mnRow;
        ListenersType maListeners;
    };
    typedef std::vector<Entry> ListenerListType;

    void swapListeners( std::vector<Entry>& rListenerList )
    {
        maListenerList.swap(rListenerList);
    }

    void operator() ( size_t nRow, SvtBroadcaster* pBroadcaster )
    {
        assert(pBroadcaster);

        // It's important to make a copy here.
        SvtBroadcaster::ListenersType aLis = pBroadcaster->GetAllListeners();
        if (aLis.empty())
            // No listeners to transfer.
            return;

        Entry aEntry;
        aEntry.mnRow = nRow;

        SvtBroadcaster::ListenersType::iterator it = aLis.begin(), itEnd = aLis.end();
        for (; it != itEnd; ++it)
        {
            SvtListener* pLis = *it;
            pLis->EndListening(*pBroadcaster);
            aEntry.maListeners.push_back(pLis);
        }

        maListenerList.push_back(aEntry);

        // At this point, the source broadcaster should have no more listeners.
        assert(!pBroadcaster->HasListeners());
    }

private:
    ListenerListType maListenerList;
};

class RemoveEmptyBroadcasterHandler
{
    sc::ColumnSpanSet maSet;
    ScDocument& mrDoc;
    SCCOL mnCol;
    SCTAB mnTab;

public:
    RemoveEmptyBroadcasterHandler( ScDocument& rDoc, SCCOL nCol, SCTAB nTab ) :
        maSet(false), mrDoc(rDoc), mnCol(nCol), mnTab(nTab) {}

    void operator() ( size_t nRow, SvtBroadcaster* pBroadcaster )
    {
        if (!pBroadcaster->HasListeners())
            maSet.set(mnTab, mnCol, nRow, true);
    }

    void purge()
    {
        sc::PurgeListenerAction aAction(mrDoc);
        maSet.executeAction(aAction);
    }
};

}

void ScColumn::TransferListeners(
    ScColumn& rDestCol, SCROW nRow1, SCROW nRow2, SCROW nRowDelta )
{
    if (nRow2 < nRow1)
        return;

    if (!ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    if (nRowDelta <= 0 && !ValidRow(nRow1+nRowDelta))
        return;

    if (nRowDelta >= 0 && !ValidRow(nRow2+nRowDelta))
        return;

    // Collect all listeners from the source broadcasters. The listeners will
    // be removed from their broadcasters as they are collected.
    TransferListenersHandler aFunc;
    sc::ProcessBroadcaster(maBroadcasters.begin(), maBroadcasters, nRow1, nRow2, aFunc);

    TransferListenersHandler::ListenerListType aListenerList;
    aFunc.swapListeners(aListenerList);

    // Re-register listeners with their destination broadcasters.
    sc::BroadcasterStoreType::iterator itDestPos = rDestCol.maBroadcasters.begin();
    TransferListenersHandler::ListenerListType::iterator it = aListenerList.begin(), itEnd = aListenerList.end();
    for (; it != itEnd; ++it)
    {
        TransferListenersHandler::Entry& rEntry = *it;

        SCROW nDestRow = rEntry.mnRow + nRowDelta;

        sc::BroadcasterStoreType::position_type aPos =
            rDestCol.maBroadcasters.position(itDestPos, nDestRow);

        itDestPos = aPos.first;
        SvtBroadcaster* pDestBrd = nullptr;
        if (aPos.first->type == sc::element_type_broadcaster)
        {
            // Existing broadcaster.
            pDestBrd = sc::broadcaster_block::at(*aPos.first->data, aPos.second);
        }
        else
        {
            // No existing broadcaster. Create a new one.
            assert(aPos.first->type == sc::element_type_empty);
            pDestBrd = new SvtBroadcaster;
            itDestPos = rDestCol.maBroadcasters.set(itDestPos, nDestRow, pDestBrd);
        }

        // Transfer all listeners from the source to the destination.
        SvtBroadcaster::ListenersType::iterator it2 = rEntry.maListeners.begin(), it2End = rEntry.maListeners.end();
        for (; it2 != it2End; ++it2)
        {
            SvtListener* pLis = *it2;
            pLis->StartListening(*pDestBrd);
        }
    }

    // Remove any broadcasters that have no listeners.
    RemoveEmptyBroadcasterHandler aFuncRemoveEmpty(*pDocument, nCol, nTab);
    sc::ProcessBroadcaster(maBroadcasters.begin(), maBroadcasters, nRow1, nRow2, aFuncRemoveEmpty);
    aFuncRemoveEmpty.purge();
}

void ScColumn::CalcAll()
{
    CalcAllHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::CompileAll( sc::CompileFormulaContext& rCxt )
{
    CompileAllHandler aFunc(rCxt);
    sc::ProcessFormula(maCells, aFunc);
}

void ScColumn::CompileXML( sc::CompileFormulaContext& rCxt, ScProgress& rProgress )
{
    CompileXMLHandler aFunc(rCxt, rProgress, *this);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

bool ScColumn::CompileErrorCells( sc::CompileFormulaContext& rCxt, sal_uInt16 nErrCode )
{
    CompileErrorCellsHandler aHdl(rCxt, *this, nErrCode);
    sc::ProcessFormula(maCells, aHdl);
    return aHdl.isCompiled();
}

void ScColumn::CalcAfterLoad( sc::CompileFormulaContext& rCxt, bool bStartListening )
{
    CalcAfterLoadHandler aFunc(rCxt, bStartListening);
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

    FindEditCellsHandler aFunc(*this);
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
        {
            ScMarkArray aArray(rMark.GetMarkArray(nCol));
            return pAttrArray->SearchStyle(nRow, pSearchStyle, bUp, &aArray);
        }
        else
            return -1;
    }
    else
        return pAttrArray->SearchStyle( nRow, pSearchStyle, bUp );
}

bool ScColumn::SearchStyleRange(
    SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
    bool bInSelection, const ScMarkData& rMark) const
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
        {
            ScMarkArray aArray(rMark.GetMarkArray(nCol));
            return pAttrArray->SearchStyleRange(
                rRow, rEndRow, pSearchStyle, bUp, &aArray);
        }
        else
            return false;
    }
    else
        return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
