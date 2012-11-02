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

#include <map>

#include <svl/poolcach.hxx>
#include <svl/zforlist.hxx>
#include <editeng/scripttypeitem.hxx>
#include <string.h>

#include "scitems.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "compiler.hxx"
#include "brdcst.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // for Notes in Sort/Swap
#include "postit.hxx"

//#pragma optimize ( "", off )
//  nur Search ohne Optimierung!

// STATIC DATA -----------------------------------------------------------
using ::editeng::SvxBorderLine;
using namespace formula;

inline bool IsAmbiguousScriptNonZero( sal_uInt8 nScript )
{
    //! move to a header file
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX &&
             nScript != 0 );
}

ScColumn::ScColumn() :
    nCol( 0 ),
    pAttrArray( NULL ),
    pDocument( NULL )
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
    // nothing:0, inside:1, bottom:2, left:4, top:8, right:16, open:32
    if ( maItems.empty() )
        return 0;
    if ( nRow1 == nRow2 )
    {
        SCSIZE nIndex;
        if ( Search( nRow1, nIndex ) )
        {
            ScBaseCell* pCell = maItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA
                && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
            {
                ScAddress aOrg( ScAddress::INITIALIZE_INVALID );
                return ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
            }
        }
        return 0;
    }
    else
    {
        ScAddress aOrg( ScAddress::INITIALIZE_INVALID );
        bool bOpen = false;
        sal_uInt16 nEdges = 0;
        SCSIZE nIndex;
        Search( nRow1, nIndex );
        while ( nIndex < maItems.size() && maItems[nIndex].nRow <= nRow2 )
        {
            ScBaseCell* pCell = maItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA
                && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
            {
                nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
                if ( nEdges )
                {
                    if ( nEdges & 8 )
                        bOpen = true;       // top edge opens, keep on looking
                    else if ( !bOpen )
                        return nEdges | 32; // there's something that wasn't opened
                    else if ( nEdges & 1 )
                        return nEdges;      // inside
                    // (nMask & 16 and  (4 and not 16)) or
                    // (nMask & 4  and (16 and not 4))
                    if ( ((nMask & 16) && (nEdges & 4)  && !(nEdges & 16))
                        || ((nMask & 4)  && (nEdges & 16) && !(nEdges & 4)) )
                        return nEdges;      // only left/right edge
                    if ( nEdges & 2 )
                        bOpen = false;      // bottom edge closes
                }
            }
            nIndex++;
        }
        if ( bOpen )
            nEdges |= 32;           // not closed, matrix continues
        return nEdges;
    }
}


bool ScColumn::HasSelectionMatrixFragment(const ScMarkData& rMark) const
{
    if ( rMark.IsMultiMarked() )
    {
        bool bFound = false;

        ScAddress aOrg( ScAddress::INITIALIZE_INVALID );
        ScAddress aCurOrg( ScAddress::INITIALIZE_INVALID );
        SCROW nTop, nBottom;
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while ( !bFound && aMarkIter.Next( nTop, nBottom ) )
        {
            bool bOpen = false;
            sal_uInt16 nEdges;
            SCSIZE nIndex;
            Search( nTop, nIndex );
            while ( !bFound && nIndex < maItems.size() && maItems[nIndex].nRow <= nBottom )
            {
                ScBaseCell* pCell = maItems[nIndex].pCell;
                if ( pCell->GetCellType() == CELLTYPE_FORMULA
                    && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
                {
                    nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
                    if ( nEdges )
                    {
                        if ( nEdges & 8 )
                            bOpen = true;   // top edge opens, keep on looking
                        else if ( !bOpen )
                            return true;    // there's something that wasn't opened
                        else if ( nEdges & 1 )
                            bFound = true;  // inside, all selected?
                        // (4 and not 16) or (16 and not 4)
                        if ( (((nEdges & 4) | 16) ^ ((nEdges & 16) | 4)) )
                            bFound = true;  // only left/right edge, all selected?
                        if ( nEdges & 2 )
                            bOpen = false;  // bottom edge closes

                        if ( bFound )
                        {   // all selected?
                            if ( aCurOrg != aOrg )
                            {   // new matrix to check?
                                aCurOrg = aOrg;
                                ScFormulaCell* pFCell;
                                if ( ((ScFormulaCell*)pCell)->GetMatrixFlag()
                                        == MM_REFERENCE )
                                    pFCell = (ScFormulaCell*) pDocument->GetCell( aOrg );
                                else
                                    pFCell = (ScFormulaCell*)pCell;
                                SCCOL nC;
                                SCROW nR;
                                pFCell->GetMatColsRows( nC, nR );
                                ScRange aRange( aOrg, ScAddress(
                                    aOrg.Col() + nC - 1, aOrg.Row() + nR - 1,
                                    aOrg.Tab() ) );
                                if ( rMark.IsAllMarked( aRange ) )
                                    bFound = false;
                            }
                            else
                                bFound = false;     // done already
                        }
                    }
                }
                nIndex++;
            }
            if ( bOpen )
                return true;
        }
        return bFound;
    }
    else
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

    //  true = alten Eintrag behalten

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
            SCROW nNewRow1 = Max( nRow1, nRow );
            SCROW nNewRow2 = Min( nRow2, nEndRow );
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
        OSL_FAIL("ScColumn::GetSelectionStyle ohne Selektion");
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
                bEqual = false;                                             // unterschiedliche
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
            bEqual = false;                                             // unterschiedliche
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
    //  um nur ein neues SetItem zu erzeugen, brauchen wir keinen SfxItemPoolCache.
    //! Achtung: der SfxItemPoolCache scheint zuviele Refs fuer das neue SetItem zu erzeugen ??

    ScDocumentPool* pDocPool = pDocument->GetPool();

    const ScPatternAttr* pOldPattern = pAttrArray->GetPattern( nRow );
    ScPatternAttr* pTemp = new ScPatternAttr(*pOldPattern);
    pTemp->GetItemSet().Put(rAttr);
    const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pDocPool->Put( *pTemp );

    if ( pNewPattern != pOldPattern )
        pAttrArray->SetPattern( nRow, pNewPattern );
    else
        pDocPool->Remove( *pNewPattern );       // ausser Spesen nichts gewesen

    delete pTemp;

        // alte Version mit SfxItemPoolCache:
}

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif


bool ScColumn::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    if ( maItems.empty() )
    {
        nIndex = 0;
        return false;
    }
    SCROW nMinRow = maItems[0].nRow;
    if ( nRow <= nMinRow )
    {
        nIndex = 0;
        return nRow == nMinRow;
    }
    SCROW nMaxRow = maItems.back().nRow;
    if ( nRow >= nMaxRow )
    {
        if ( nRow == nMaxRow )
        {
            nIndex = maItems.size() - 1;
            return true;
        }
        else
        {
            nIndex = maItems.size();
            return false;
        }
    }

    long nOldLo, nOldHi;
    long    nLo     = nOldLo = 0;
    long    nHi     = nOldHi = Min(static_cast<long>(maItems.size())-1, static_cast<long>(nRow) );
    long    i       = 0;
    bool    bFound  = false;
    // quite continuous distribution? => interpolating search
    bool    bInterpol = (static_cast<SCSIZE>(nMaxRow - nMinRow) < maItems.size() * 2);
    SCROW   nR;

    while ( !bFound && nLo <= nHi )
    {
        if ( !bInterpol || nHi - nLo < 3 )
            i = (nLo+nHi) / 2;          // no effort, no division by zero
        else
        {   // interpolating search
            long nLoRow = maItems[nLo].nRow;     // no unsigned underflow upon substraction
            i = nLo + (long)((long)(nRow - nLoRow) * (nHi - nLo)
                / (maItems[nHi].nRow - nLoRow));
            if ( i < 0 || static_cast<SCSIZE>(i) >= maItems.size() )
            {   // oops ...
                i = (nLo+nHi) / 2;
                bInterpol = false;
            }
        }
        nR = maItems[i].nRow;
        if ( nR < nRow )
        {
            nLo = i+1;
            if ( bInterpol )
            {
                if ( nLo <= nOldLo )
                    bInterpol = false;
                else
                    nOldLo = nLo;
            }
        }
        else
        {
            if ( nR > nRow )
            {
                nHi = i-1;
                if ( bInterpol )
                {
                    if ( nHi >= nOldHi )
                        bInterpol = false;
                    else
                        nOldHi = nHi;
                }
            }
            else
                bFound = true;
        }
    }
    if (bFound)
        nIndex = static_cast<SCSIZE>(i);
    else
        nIndex = static_cast<SCSIZE>(nLo); // rear index
    return bFound;
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif


ScBaseCell* ScColumn::GetCell( SCROW nRow ) const
{
    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        return maItems[nIndex].pCell;
    return NULL;
}


void ScColumn::Resize( SCSIZE nSize )
{
    if (nSize > sal::static_int_cast<SCSIZE>(MAXROWCOUNT))
        nSize = MAXROWCOUNT;
    if (nSize < maItems.size())
        nSize = maItems.size();

    maItems.reserve(nSize);
}

//  SwapRow zum Sortieren

namespace {

/** Moves broadcaster from old cell to new cell if exists, otherwise creates a new note cell. */
void lclTakeBroadcaster( ScBaseCell*& rpCell, SvtBroadcaster* pBC )
{
    if( pBC )
    {
        if( rpCell )
            rpCell->TakeBroadcaster( pBC );
        else
            rpCell = new ScNoteCell( pBC );
    }
}

} // namespace

void ScColumn::SwapRow(SCROW nRow1, SCROW nRow2)
{
    /*  Simple swap of cell pointers does not work if broadcasters exist (crash
        if cell broadcasts directly or indirectly to itself). While swapping
        the cells, broadcasters have to remain at old positions! */

    /*  While cloning cells, do not clone notes, but move note pointers to new
        cells. This prevents creation of new caption drawing objects for every
        swap operation while sorting. */

    ScBaseCell* pCell1 = 0;
    SCSIZE nIndex1;
    if ( Search( nRow1, nIndex1 ) )
        pCell1 = maItems[nIndex1].pCell;

    ScBaseCell* pCell2 = 0;
    SCSIZE nIndex2;
    if ( Search( nRow2, nIndex2 ) )
        pCell2 = maItems[nIndex2].pCell;

    // no cells found, nothing to do
    if ( !pCell1 && !pCell2 )
        return ;

    // swap variables if first cell is empty, to save some code below
    if ( !pCell1 )
    {
        ::std::swap( nRow1, nRow2 );
        ::std::swap( nIndex1, nIndex2 );
        ::std::swap( pCell1, pCell2 );
    }

    // from here: first cell (pCell1, nIndex1) exists always

    ScAddress aPos1( nCol, nRow1, nTab );
    ScAddress aPos2( nCol, nRow2, nTab );

    CellType eType1 = pCell1->GetCellType();
    CellType eType2 = pCell2 ? pCell2->GetCellType() : CELLTYPE_NONE;

    ScFormulaCell* pFmlaCell1 = (eType1 == CELLTYPE_FORMULA) ? static_cast< ScFormulaCell* >( pCell1 ) : 0;
    ScFormulaCell* pFmlaCell2 = (eType2 == CELLTYPE_FORMULA) ? static_cast< ScFormulaCell* >( pCell2 ) : 0;

    // simple swap if no formula cells present
    if ( !pFmlaCell1 && !pFmlaCell2 )
    {
        // remember cell broadcasters, must remain at old position
        SvtBroadcaster* pBC1 = pCell1->ReleaseBroadcaster();

        if ( pCell2 )
        {
            /*  Both cells exist, no formula cells involved, a simple swap can
                be performed (but keep broadcasters and notes at old position). */
            maItems[nIndex1].pCell = pCell2;
            maItems[nIndex2].pCell = pCell1;

            SvtBroadcaster* pBC2 = pCell2->ReleaseBroadcaster();
            pCell1->TakeBroadcaster( pBC2 );
            pCell2->TakeBroadcaster( pBC1 );
        }
        else
        {
            ScNoteCell* pDummyCell = pBC1 ? new ScNoteCell( pBC1 ) : 0;
            if ( pDummyCell )
            {
                // insert dummy note cell (without note) containing old broadcaster
                maItems[nIndex1].pCell = pDummyCell;
            }
            else
            {
                // remove ColEntry at old position
                maItems.erase( maItems.begin() + nIndex1 );
            }

            // insert ColEntry at new position
            Insert( nRow2, pCell1 );
        }

        return;
    }

    // from here: at least one of the cells is a formula cell

    /*  Never move any array formulas. Disabling sort if parts of array
        formulas are contained is done at UI. */
    if ( (pFmlaCell1 && (pFmlaCell1->GetMatrixFlag() != 0)) || (pFmlaCell2 && (pFmlaCell2->GetMatrixFlag() != 0)) )
        return;

    // do not swap, if formulas are equal
    if ( pFmlaCell1 && pFmlaCell2 )
    {
        ScTokenArray* pCode1 = pFmlaCell1->GetCode();
        ScTokenArray* pCode2 = pFmlaCell2->GetCode();

        if (pCode1->GetLen() == pCode2->GetLen())       // nicht-UPN
        {
            bool bEqual = true;
            sal_uInt16 nLen = pCode1->GetLen();
            FormulaToken** ppToken1 = pCode1->GetArray();
            FormulaToken** ppToken2 = pCode2->GetArray();
            for (sal_uInt16 i=0; i<nLen; i++)
            {
                if ( !ppToken1[i]->TextEqual(*(ppToken2[i])) ||
                        ppToken1[i]->Is3DRef() || ppToken2[i]->Is3DRef() )
                {
                    bEqual = false;
                    break;
                }
            }

            // do not swap formula cells with equal formulas
            if (bEqual)
            {
                return;
            }
        }
    }

    /*  Create clone of pCell1 at position of pCell2 (pCell1 exists always, see
        variable swapping above). Do not clone the note, but move pointer of
        old note to new cell. */
    ScBaseCell* pNew2 = pCell1->Clone( *pDocument, aPos2, SC_CLONECELL_ADJUST3DREL );

    /*  Create clone of pCell2 at position of pCell1. Do not clone the note,
        but move pointer of old note to new cell. */
    ScBaseCell* pNew1 = 0;
    if ( pCell2 )
    {
        pNew1 = pCell2->Clone( *pDocument, aPos1, SC_CLONECELL_ADJUST3DREL );
    }

    // move old broadcasters new cells at the same old position
    SvtBroadcaster* pBC1 = pCell1->ReleaseBroadcaster();
    lclTakeBroadcaster( pNew1, pBC1 );
    SvtBroadcaster* pBC2 = pCell2 ? pCell2->ReleaseBroadcaster() : 0;
    lclTakeBroadcaster( pNew2, pBC2 );

    /*  Insert the new cells. Old cell has to be deleted, if there is no new
        cell (call to Insert deletes old cell by itself). */
    if ( !pNew1 )
        Delete( nRow1 );            // deletes pCell1
    else
        Insert( nRow1, pNew1 );     // deletes pCell1, inserts pNew1

    if ( pCell2 && !pNew2 )
        Delete( nRow2 );            // deletes pCell2
    else if ( pNew2 )
        Insert( nRow2, pNew2 );     // deletes pCell2 (if existing), inserts pNew2
}


void ScColumn::SwapCell( SCROW nRow, ScColumn& rCol)
{
    ScBaseCell* pCell1 = 0;
    SCSIZE nIndex1;
    if ( Search( nRow, nIndex1 ) )
        pCell1 = maItems[nIndex1].pCell;

    ScBaseCell* pCell2 = 0;
    SCSIZE nIndex2;
    if ( rCol.Search( nRow, nIndex2 ) )
        pCell2 = rCol.maItems[nIndex2].pCell;

    // reverse call if own cell is missing (ensures own existing cell in following code)
    if( !pCell1 )
    {
        if( pCell2 )
            rCol.SwapCell( nRow, *this );
        return;
    }

    // from here: own cell (pCell1, nIndex1) exists always

    ScFormulaCell* pFmlaCell1 = (pCell1->GetCellType() == CELLTYPE_FORMULA) ? static_cast< ScFormulaCell* >( pCell1 ) : 0;
    ScFormulaCell* pFmlaCell2 = (pCell2 && (pCell2->GetCellType() == CELLTYPE_FORMULA)) ? static_cast< ScFormulaCell* >( pCell2 ) : 0;

    if ( pCell2 )
    {
        // Tauschen
        maItems[nIndex1].pCell = pCell2;
        rCol.maItems[nIndex2].pCell = pCell1;
        // Referenzen aktualisieren
        SCsCOL dx = rCol.nCol - nCol;
        if ( pFmlaCell1 )
        {
            ScRange aRange( ScAddress( rCol.nCol, 0, nTab ),
                            ScAddress( rCol.nCol, MAXROW, nTab ) );
            pFmlaCell1->aPos.SetCol( rCol.nCol );
            pFmlaCell1->UpdateReference(URM_MOVE, aRange, dx, 0, 0);
        }
        if ( pFmlaCell2 )
        {
            ScRange aRange( ScAddress( nCol, 0, nTab ),
                            ScAddress( nCol, MAXROW, nTab ) );
            pFmlaCell2->aPos.SetCol( nCol );
            pFmlaCell2->UpdateReference(URM_MOVE, aRange, -dx, 0, 0);
        }
    }
    else
    {
        // Loeschen
        maItems.erase(maItems.begin() + nIndex1);
        // Referenzen aktualisieren
        SCsCOL dx = rCol.nCol - nCol;
        if ( pFmlaCell1 )
        {
            ScRange aRange( ScAddress( rCol.nCol, 0, nTab ),
                            ScAddress( rCol.nCol, MAXROW, nTab ) );
            pFmlaCell1->aPos.SetCol( rCol.nCol );
            pFmlaCell1->UpdateReference(URM_MOVE, aRange, dx, 0, 0);
        }
        // Einfuegen
        rCol.Insert(nRow, pCell1);
    }
}


bool ScColumn::TestInsertCol( SCROW nStartRow, SCROW nEndRow) const
{
    if (!IsEmpty())
    {
        bool bTest = true;
        if ( !maItems.empty() )
            for (SCSIZE i=0; (i<maItems.size()) && bTest; i++)
                bTest = (maItems[i].nRow < nStartRow) || (maItems[i].nRow > nEndRow)
                        || maItems[i].pCell->IsBlank();

        //  AttrArray testet nur zusammengefasste

        if ((bTest) && (pAttrArray))
            bTest = pAttrArray->TestInsertCol(nStartRow, nEndRow);

        //!     rausgeschobene Attribute bei Undo beruecksichtigen

        return bTest;
    }
    else
        return true;
}


bool ScColumn::TestInsertRow( SCSIZE nSize ) const
{
    //  AttrArray only looks for merged cells

    if ( !maItems.empty() )
        return ( nSize <= sal::static_int_cast<SCSIZE>(MAXROW) &&
                 maItems[maItems.size()-1].nRow <= MAXROW-(SCROW)nSize && pAttrArray->TestInsertRow( nSize ) );
    else
        return pAttrArray->TestInsertRow( nSize );
}


void ScColumn::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    pAttrArray->InsertRow( nStartRow, nSize );

    //! Search

    if ( maItems.empty() )
        return;

    SCSIZE i;
    Search( nStartRow, i );
    if ( i >= maItems.size() )
        return ;

    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden

    SCSIZE nNewCount = maItems.size();
    bool bCountChanged = false;
    ScAddress aAdr( nCol, 0, nTab );
    ScHint aHint( SC_HINT_DATACHANGED, aAdr, NULL );    // only areas (ScBaseCell* == NULL)
    ScAddress& rAddress = aHint.GetAddress();
    // for sparse occupation use single broadcasts, not ranges
    bool bSingleBroadcasts = (((maItems.back().nRow - maItems[i].nRow) /
                (maItems.size() - i)) > 1);
    if ( bSingleBroadcasts )
    {
        SCROW nLastBroadcast = MAXROW+1;
        for ( ; i < maItems.size(); i++)
        {
            SCROW nOldRow = maItems[i].nRow;
            // Change source broadcaster
            if ( nLastBroadcast != nOldRow )
            {   // Do not broadcast a direct sequence twice
                rAddress.SetRow( nOldRow );
                pDocument->AreaBroadcast( aHint );
            }
            SCROW nNewRow = (maItems[i].nRow += nSize);
            // Change target broadcaster
            rAddress.SetRow( nNewRow );
            pDocument->AreaBroadcast( aHint );
            nLastBroadcast = nNewRow;
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
            if ( nNewRow > MAXROW && !bCountChanged )
            {
                nNewCount = i;
                bCountChanged = true;
            }
        }
    }
    else
    {
        rAddress.SetRow( maItems[i].nRow );
        ScRange aRange( rAddress );
        for ( ; i < maItems.size(); i++)
        {
            SCROW nNewRow = (maItems[i].nRow += nSize);
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
            if ( nNewRow > MAXROW && !bCountChanged )
            {
                nNewCount = i;
                bCountChanged = true;
                aRange.aEnd.SetRow( MAXROW );
            }
        }
        if ( !bCountChanged )
            aRange.aEnd.SetRow( maItems.back().nRow );
        pDocument->AreaBroadcastInRange( aRange, aHint );
    }

    if (bCountChanged)
    {
        SCSIZE nDelCount = maItems.size() - nNewCount;
        ScBaseCell** ppDelCells = new ScBaseCell*[nDelCount];
        SCROW* pDelRows = new SCROW[nDelCount];
        for (i = 0; i < nDelCount; i++)
        {
            ppDelCells[i] = maItems[nNewCount+i].pCell;
            pDelRows[i] = maItems[nNewCount+i].nRow;
        }
        maItems.resize( nNewCount );

        for (i = 0; i < nDelCount; i++)
        {
            ScBaseCell* pCell = ppDelCells[i];
            OSL_ENSURE( pCell->IsBlank(), "sichtbare Zelle weggeschoben" );
            SvtBroadcaster* pBC = pCell->GetBroadcaster();
            if (pBC)
            {
                MoveListeners( *pBC, pDelRows[i] - nSize );
                pCell->DeleteBroadcaster();
                pCell->Delete();
            }
        }

        delete [] pDelRows;
        delete [] ppDelCells;
    }

    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::CopyToClip(SCROW nRow1, SCROW nRow2, ScColumn& rColumn, bool bKeepScenarioFlags)
{
    pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray,
                            bKeepScenarioFlags ? (SC_MF_ALL & ~SC_MF_SCENARIO) : SC_MF_ALL );

    SCSIZE i;
    SCSIZE nBlockCount = 0;
    SCSIZE nStartIndex = 0, nEndIndex = 0;
    for (i = 0; i < maItems.size(); i++)
        if ((maItems[i].nRow >= nRow1) && (maItems[i].nRow <= nRow2))
        {
            if (!nBlockCount)
                nStartIndex = i;
            nEndIndex = i;
            ++nBlockCount;

            //  im Clipboard muessen interpretierte Zellen stehen, um andere Formate
            //  (Text, Grafik...) erzueugen zu koennen

            if ( maItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                ScFormulaCell* pFCell = (ScFormulaCell*) maItems[i].pCell;
                if (pFCell->GetDirty() && pDocument->GetAutoCalc())
                    pFCell->Interpret();
            }
        }

    if (nBlockCount)
    {
        rColumn.Resize( rColumn.GetCellCount() + nBlockCount );
        ScAddress aOwnPos( nCol, 0, nTab );
        ScAddress aDestPos( rColumn.nCol, 0, rColumn.nTab );
        for (i = nStartIndex; i <= nEndIndex; i++)
        {
            aOwnPos.SetRow( maItems[i].nRow );
            aDestPos.SetRow( maItems[i].nRow );
            ScBaseCell* pNewCell = maItems[i].pCell->Clone( *rColumn.pDocument, aDestPos, SC_CLONECELL_DEFAULT );
            rColumn.Append( aDestPos.Row(), pNewCell );
        }
    }
}


void ScColumn::CopyToColumn(SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData, bool bAsLink )
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
                    CopyToColumn( Max(nRow1,nStart), Min(nRow2,nEnd),
                                    nFlags, false, rColumn, pMarkData, bAsLink );
            }
        }
        else
        {
            OSL_FAIL("CopyToColumn: bMarked, aber keine Markierung");
        }
        return;
    }

    if ( (nFlags & IDF_ATTRIB) != 0 )
    {
        if ( (nFlags & IDF_STYLES) != IDF_STYLES )
        {   // StyleSheets im Zieldokument bleiben erhalten
            // z.B. DIF und RTF Clipboard-Import
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
        SCSIZE i;
        SCSIZE nBlockCount = 0;
        SCSIZE nStartIndex = 0, nEndIndex = 0;
        for (i = 0; i < maItems.size(); i++)
            if ((maItems[i].nRow >= nRow1) && (maItems[i].nRow <= nRow2))
            {
                if (!nBlockCount)
                    nStartIndex = i;
                nEndIndex = i;
                ++nBlockCount;
            }

        if (nBlockCount)
        {
            rColumn.Resize( rColumn.GetCellCount() + nBlockCount );
            ScAddress aDestPos( rColumn.nCol, 0, rColumn.nTab );
            for (i = nStartIndex; i <= nEndIndex; i++)
            {
                aDestPos.SetRow( maItems[i].nRow );
                ScBaseCell* pNew = bAsLink ?
                    CreateRefCell( rColumn.pDocument, aDestPos, i, nFlags ) :
                    CloneCell( i, nFlags, *rColumn.pDocument, aDestPos );

                if (pNew)
                {
                    // Special case to allow removing of cell instances.  A
                    // string cell with empty content is used to indicate an
                    // empty cell.
                    if (pNew->GetCellType() == CELLTYPE_STRING)
                    {
                        rtl::OUString aStr = static_cast<ScStringCell*>(pNew)->GetString();
                        if (aStr.isEmpty())
                            // A string cell with empty string.  Delete the cell itself.
                            rColumn.Delete(maItems[i].nRow);
                        else
                            // non-empty string cell
                            rColumn.Insert(maItems[i].nRow, pNew);
                    }
                    else
                        rColumn.Insert(maItems[i].nRow, pNew);
                }
            }
        }
    }
}


void ScColumn::UndoToColumn(SCROW nRow1, SCROW nRow2, sal_uInt16 nFlags, bool bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData )
{
    if (nRow1 > 0)
        CopyToColumn( 0, nRow1-1, IDF_FORMULA, false, rColumn );

    CopyToColumn( nRow1, nRow2, nFlags, bMarked, rColumn, pMarkData );      //! bMarked ????

    if (nRow2 < MAXROW)
        CopyToColumn( nRow2+1, MAXROW, IDF_FORMULA, false, rColumn );
}


void ScColumn::CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const
{
    ScDocument& rDestDoc = *rDestCol.pDocument;
    ScAddress aOwnPos( nCol, 0, nTab );
    ScAddress aDestPos( rDestCol.nCol, 0, rDestCol.nTab );

    SCSIZE nPosCount = rPosCol.maItems.size();
    for (SCSIZE nPosIndex = 0; nPosIndex < nPosCount; nPosIndex++)
    {
        aOwnPos.SetRow( rPosCol.maItems[nPosIndex].nRow );
        aDestPos.SetRow( aOwnPos.Row() );
        SCSIZE nThisIndex;
        if ( Search( aDestPos.Row(), nThisIndex ) )
        {
            ScBaseCell* pNew = maItems[nThisIndex].pCell->Clone( rDestDoc, aDestPos );
            rDestCol.Insert( aDestPos.Row(), pNew );
        }
    }

}


void ScColumn::CopyScenarioFrom( const ScColumn& rSrcCol )
{
    //  Dies ist die Szenario-Tabelle, die Daten werden hineinkopiert

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            DeleteArea( nStart, nEnd, IDF_CONTENTS );
            ((ScColumn&)rSrcCol).
                CopyToColumn( nStart, nEnd, IDF_CONTENTS, false, *this );

            //  UpdateUsed nicht noetig, schon in TestCopyScenario passiert

            SCsTAB nDz = nTab - rSrcCol.nTab;
            UpdateReference(URM_COPY, nCol, nStart, nTab,
                                      nCol, nEnd,   nTab,
                                      0, 0, nDz, NULL);
            UpdateCompile();
        }

        //! CopyToColumn "const" machen !!!

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}


void ScColumn::CopyScenarioTo( ScColumn& rDestCol ) const
{
    //  Dies ist die Szenario-Tabelle, die Daten werden in die andere kopiert

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    SCROW nStart = -1, nEnd = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            rDestCol.DeleteArea( nStart, nEnd, IDF_CONTENTS );
            ((ScColumn*)this)->
                CopyToColumn( nStart, nEnd, IDF_CONTENTS, false, rDestCol );

            //  UpdateUsed nicht noetig, schon in TestCopyScenario passiert

            SCsTAB nDz = rDestCol.nTab - nTab;
            rDestCol.UpdateReference(URM_COPY, rDestCol.nCol, nStart, rDestCol.nTab,
                                               rDestCol.nCol, nEnd,   rDestCol.nTab,
                                               0, 0, nDz, NULL);
            rDestCol.UpdateCompile();
        }

        //! CopyToColumn "const" machen !!!

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


void ScColumn::SwapCol(ScColumn& rCol)
{
    maItems.swap(rCol.maItems);

    ScAttrArray* pTempAttr = rCol.pAttrArray;
    rCol.pAttrArray = pAttrArray;
    pAttrArray = pTempAttr;

    // AttrArray muss richtige Spaltennummer haben
    pAttrArray->SetCol(nCol);
    rCol.pAttrArray->SetCol(rCol.nCol);

    SCSIZE i;
    for (i = 0; i < maItems.size(); i++)
    {
        ScFormulaCell* pCell = (ScFormulaCell*) maItems[i].pCell;
        if( pCell->GetCellType() == CELLTYPE_FORMULA)
            pCell->aPos.SetCol(nCol);
    }
    for (i = 0; i < rCol.maItems.size(); i++)
    {
        ScFormulaCell* pCell = (ScFormulaCell*) rCol.maItems[i].pCell;
        if( pCell->GetCellType() == CELLTYPE_FORMULA)
            pCell->aPos.SetCol(rCol.nCol);
    }
}

void ScColumn::MoveTo(SCROW nStartRow, SCROW nEndRow, ScColumn& rCol)
{
    pAttrArray->MoveTo(nStartRow, nEndRow, *rCol.pAttrArray);

    if (maItems.empty())
        // No cells to move.
        return;

    ::std::vector<SCROW> aRows;
    SCSIZE i;
    Search( nStartRow, i);  // i points to start row or position thereafter
    SCSIZE nStartPos = i;
    for ( ; i < maItems.size() && maItems[i].nRow <= nEndRow; ++i)
    {
        SCROW nRow = maItems[i].nRow;
        aRows.push_back( nRow);
        rCol.Insert( nRow, maItems[i].pCell);
    }
    SCSIZE nStopPos = i;
    if (nStartPos < nStopPos)
    {
        // Create list of ranges of cell entry positions
        typedef ::std::pair<SCSIZE,SCSIZE> PosPair;
        typedef ::std::vector<PosPair> EntryPosPairs;
        EntryPosPairs aEntries;
        {
            bool bFirst = true;
            nStopPos = 0;
            for (::std::vector<SCROW>::const_iterator it( aRows.begin());
                    it != aRows.end() && nStopPos < maItems.size(); ++it,
                    ++nStopPos)
            {
                if (!bFirst && *it != maItems[nStopPos].nRow)
                {
                    aEntries.push_back( PosPair(nStartPos, nStopPos));
                    bFirst = true;
                }
                if (bFirst && Search( *it, nStartPos))
                {
                    bFirst = false;
                    nStopPos = nStartPos;
                }
            }
            if (!bFirst && nStartPos < nStopPos)
                aEntries.push_back( PosPair(nStartPos, nStopPos));
        }
        // Broadcast changes
        ScAddress aAdr( nCol, 0, nTab );
        ScHint aHint( SC_HINT_DYING, aAdr, NULL );  // areas only
        ScAddress& rAddress = aHint.GetAddress();
        ScNoteCell* pNoteCell = new ScNoteCell;     // Dummy like in DeleteRange

        // must iterate backwards, because indexes of following cells become invalid
        for (EntryPosPairs::reverse_iterator it( aEntries.rbegin());
                it != aEntries.rend(); ++it)
        {
            nStartPos = (*it).first;
            nStopPos = (*it).second;
            for (i=nStartPos; i<nStopPos; ++i)
                maItems[i].pCell = pNoteCell;
            for (i=nStartPos; i<nStopPos; ++i)
            {
                rAddress.SetRow( maItems[i].nRow );
                pDocument->AreaBroadcast( aHint );
            }
            maItems.erase(maItems.begin() + nStartPos, maItems.begin() + nStopPos - 1);
        }
        pNoteCell->Delete();
    }
}

bool ScColumn::UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
             SCCOL nCol2, SCROW nRow2, SCTAB nTab2, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
             ScDocument* pUndoDoc )
{
    bool bUpdated = false;
    if ( !maItems.empty() )
    {
        ScRange aRange( ScAddress( nCol1, nRow1, nTab1 ),
                        ScAddress( nCol2, nRow2, nTab2 ) );
        if ( eUpdateRefMode == URM_COPY && nRow1 == nRow2 )
        {   // z.B. eine einzelne Zelle aus dem Clipboard eingefuegt
            SCSIZE nIndex;
            if ( Search( nRow1, nIndex ) )
            {
                ScFormulaCell* pCell = (ScFormulaCell*) maItems[nIndex].pCell;
                if( pCell->GetCellType() == CELLTYPE_FORMULA)
                    bUpdated |= pCell->UpdateReference(
                        eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc );
            }
        }
        else
        {
            // For performance reasons two loop bodies instead of
            // testing for update mode in each iteration.
            // Anyways, this is still a bottleneck on large arrays with few
            // formulas cells.
            if ( eUpdateRefMode == URM_COPY )
            {
                SCSIZE i;
                Search( nRow1, i );
                for ( ; i < maItems.size(); i++ )
                {
                    SCROW nRow = maItems[i].nRow;
                    if ( nRow > nRow2 )
                        break;
                    ScBaseCell* pCell = maItems[i].pCell;
                    if( pCell->GetCellType() == CELLTYPE_FORMULA)
                    {
                        bUpdated |= ((ScFormulaCell*)pCell)->UpdateReference(
                            eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc );
                        if ( nRow != maItems[i].nRow )
                            Search( nRow, i );  // Listener removed/inserted?
                    }
                }
            }
            else
            {
                SCSIZE i = 0;
                for ( ; i < maItems.size(); i++ )
                {
                    ScBaseCell* pCell = maItems[i].pCell;
                    if( pCell->GetCellType() == CELLTYPE_FORMULA)
                    {
                        SCROW nRow = maItems[i].nRow;
                        // When deleting rows on several sheets, the formula's position may be updated with the first call,
                        // so the undo position must be passed from here.
                        ScAddress aUndoPos( nCol, nRow, nTab );
                        bUpdated |= ((ScFormulaCell*)pCell)->UpdateReference(
                            eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc, &aUndoPos );
                        if ( nRow != maItems[i].nRow )
                            Search( nRow, i );  // Listener removed/inserted?
                    }
                }
            }
        }
    }
    return bUpdated;
}


void ScColumn::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    if ( !maItems.empty() )
        for (SCSIZE i=0; i<maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                SCROW nRow = maItems[i].nRow;
                ((ScFormulaCell*)pCell)->UpdateTranspose( rSource, rDest, pUndoDoc );
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );              // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    if ( !maItems.empty() )
        for (SCSIZE i=0; i<maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                SCROW nRow = maItems[i].nRow;
                ((ScFormulaCell*)pCell)->UpdateGrow( rArea, nGrowX, nGrowY );
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );              // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateInsertTab(SCTAB nInsPos, SCTAB nNewSheets)
{
    if (nTab >= nInsPos)
    {
        nTab += nNewSheets;
        pAttrArray->SetTab(nTab);
    }

    UpdateInsertTabOnlyCells(nInsPos, nNewSheets);
}

void ScColumn::UpdateInsertTabOnlyCells(SCTAB nInsPos, SCTAB nNewSheets)
{
    if (maItems.empty())
        return;

    for (size_t i = 0; i < maItems.size(); ++i)
    {
        switch (maItems[i].pCell->GetCellType())
        {
            case CELLTYPE_FORMULA:
            {
                SCROW nRow = maItems[i].nRow;
                ScFormulaCell* p = static_cast<ScFormulaCell*>(maItems[i].pCell);
                p->UpdateInsertTab(nInsPos, nNewSheets);
                if (nRow != maItems[i].nRow)
                    Search(nRow, i);      // Listener deleted/inserted?
            }
            break;
            case CELLTYPE_EDIT:
            {
                ScEditCell* p = static_cast<ScEditCell*>(maItems[i].pCell);
                p->UpdateFields(nTab);
            }
            break;
            default:
                ;
        }
    }
}

void ScColumn::UpdateInsertTabAbs(SCTAB nNewPos)
{
    if (maItems.empty())
        return;

    for (size_t i = 0; i < maItems.size(); ++i)
    {
        switch (maItems[i].pCell->GetCellType())
        {
            case CELLTYPE_FORMULA:
            {
                SCROW nRow = maItems[i].nRow;
                ScFormulaCell* p = static_cast<ScFormulaCell*>(maItems[i].pCell);
                p->UpdateInsertTabAbs(nNewPos);
                if (nRow != maItems[i].nRow)
                    Search(nRow, i);      // Listener deleted/inserted?
            }
            break;
            case CELLTYPE_EDIT:
            {
                ScEditCell* p = static_cast<ScEditCell*>(maItems[i].pCell);
                p->UpdateFields(nTab);
            }
            break;
            default:
                ;
        }
    }
}

void ScColumn::UpdateDeleteTab(SCTAB nDelPos, bool bIsMove, ScColumn* pRefUndo, SCTAB nSheets)
{
    if (nTab > nDelPos)
    {
        nTab -= nSheets;
        pAttrArray->SetTab(nTab);
    }

    if (maItems.empty())
        return;

    for (size_t i = 0; i < maItems.size(); ++i)
    {
        switch (maItems[i].pCell->GetCellType())
        {
            case CELLTYPE_FORMULA:
            {
                SCROW nRow = maItems[i].nRow;
                ScFormulaCell* pOld = static_cast<ScFormulaCell*>(maItems[i].pCell);

                /*  Do not copy cell note to the undo document. Undo will copy
                    back the formula cell while keeping the original note. */
                ScBaseCell* pSave = pRefUndo ? pOld->Clone( *pDocument ) : 0;

                bool bChanged = pOld->UpdateDeleteTab(nDelPos, bIsMove, nSheets);
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?

                if (pRefUndo)
                {
                    if (bChanged)
                        pRefUndo->Insert( nRow, pSave );
                    else if(pSave)
                        pSave->Delete();
                }
            }
            break;
            case CELLTYPE_EDIT:
            {
                ScEditCell* p = static_cast<ScEditCell*>(maItems[i].pCell);
                p->UpdateFields(nTab);
            }
            break;
            default:
                ;
        }
    }
}

void ScColumn::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo )
{
    nTab = nTabNo;
    pAttrArray->SetTab( nTabNo );
    if (maItems.empty())
        return;

    for (size_t i = 0; i < maItems.size(); ++i)
    {
        switch (maItems[i].pCell->GetCellType())
        {
            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* p = static_cast<ScFormulaCell*>(maItems[i].pCell);
                SCROW nRow = maItems[i].nRow;
                p->UpdateMoveTab(nOldPos, nNewPos, nTabNo);
                if (nRow != maItems[i].nRow)
                    Search(nRow, i);      // Listener deleted/inserted?
            }
            break;
            case CELLTYPE_EDIT:
            {
                ScEditCell* p = static_cast<ScEditCell*>(maItems[i].pCell);
                p->UpdateFields(nTab);
            }
            break;
            default:
                ;
        }
    }
}


void ScColumn::UpdateCompile( bool bForceIfNameInUse )
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
            if( p->GetCellType() == CELLTYPE_FORMULA )
            {
                SCROW nRow = maItems[i].nRow;
                p->UpdateCompile( bForceIfNameInUse );
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::SetTabNo(SCTAB nNewTab)
{
    nTab = nNewTab;
    pAttrArray->SetTab( nNewTab );
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
            if( p->GetCellType() == CELLTYPE_FORMULA )
                p->aPos.SetTab( nNewTab );
        }
}

void ScColumn::FindRangeNamesInUse(SCROW nRow1, SCROW nRow2, std::set<sal_uInt16>& rIndexes) const
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
            if ((maItems[i].nRow >= nRow1) &&
                (maItems[i].nRow <= nRow2) &&
                (maItems[i].pCell->GetCellType() == CELLTYPE_FORMULA))
                    ((ScFormulaCell*)maItems[i].pCell)->FindRangeNamesInUse(rIndexes);
}

void ScColumn::SetDirtyVar()
{
    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
        if( p->GetCellType() == CELLTYPE_FORMULA )
            p->SetDirtyVar();
    }
}


void ScColumn::SetDirty()
{
    // wird nur dokumentweit verwendet, kein FormulaTrack
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
        if( p->GetCellType() == CELLTYPE_FORMULA )
        {
            p->SetDirtyVar();
            if ( !pDocument->IsInFormulaTree( p ) )
                pDocument->PutInFormulaTree( p );
        }
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetDirty( const ScRange& rRange )
{   // broadcastet alles innerhalb eines Range, mit FormulaTrack
    if ( maItems.empty() )
        return ;
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    SCROW nRow2 = rRange.aEnd.Row();
    ScAddress aPos( nCol, 0, nTab );
    ScHint aHint( SC_HINT_DATACHANGED, aPos, NULL );
    SCROW nRow;
    SCSIZE nIndex;
    Search( rRange.aStart.Row(), nIndex );
    while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRow2 )
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetDirty();
        else
        {
            aHint.GetAddress().SetRow( nRow );
            aHint.SetCell( pCell );
            pDocument->Broadcast( aHint );
        }
        nIndex++;
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetTableOpDirty( const ScRange& rRange )
{
    if ( maItems.empty() )
        return ;
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // no multiple recalculation
    SCROW nRow2 = rRange.aEnd.Row();
    ScAddress aPos( nCol, 0, nTab );
    ScHint aHint( SC_HINT_TABLEOPDIRTY, aPos, NULL );
    SCROW nRow;
    SCSIZE nIndex;
    Search( rRange.aStart.Row(), nIndex );
    while ( nIndex < maItems.size() && (nRow = maItems[nIndex].nRow) <= nRow2 )
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetTableOpDirty();
        else
        {
            aHint.GetAddress().SetRow( nRow );
            aHint.SetCell( pCell );
            pDocument->Broadcast( aHint );
        }
        nIndex++;
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetDirtyAfterLoad()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
#if 1
        // Simply set dirty and append to FormulaTree, without broadcasting,
        // which is a magnitude faster. This is used to calculate the entire
        // document, e.g. when loading alien file formats.
        if ( p->GetCellType() == CELLTYPE_FORMULA )
            p->SetDirtyAfterLoad();
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
        if ( p->GetCellType() == CELLTYPE_FORMULA && p->GetDirty() )
            p->SetDirty();
#endif
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetRelNameDirty()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) maItems[i].pCell;
        if( p->GetCellType() == CELLTYPE_FORMULA && p->HasRelNameReference() )
            p->SetDirty();
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::CalcAll()
{
    if ( !maItems.empty() )
        for (SCSIZE i=0; i<maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
#if OSL_DEBUG_LEVEL > 1
                // nach F9 ctrl-F9: ueberprueft die Berechnung per FormulaTree
                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                double nOldVal, nNewVal;
                nOldVal = pFCell->GetValue();
#endif
                ((ScFormulaCell*)pCell)->Interpret();
#if OSL_DEBUG_LEVEL > 1
                if ( pFCell->GetCode()->IsRecalcModeNormal() )
                    nNewVal = pFCell->GetValue();
                else
                    nNewVal = nOldVal;  // random(), jetzt() etc.
                OSL_ENSURE( nOldVal==nNewVal, "CalcAll: nOldVal != nNewVal" );
#endif
            }
        }
}


void ScColumn::CompileAll()
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                SCROW nRow = maItems[i].nRow;
                // fuer unbedingtes kompilieren
                // bCompile=true und pCode->nError=0
                ((ScFormulaCell*)pCell)->GetCode()->SetCodeError( 0 );
                ((ScFormulaCell*)pCell)->SetCompile( true );
                ((ScFormulaCell*)pCell)->CompileTokenArray();
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::CompileXML( ScProgress& rProgress )
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                SCROW nRow = maItems[i].nRow;
                ((ScFormulaCell*)pCell)->CompileXML( rProgress );
                if ( nRow != maItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::CalcAfterLoad()
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->CalcAfterLoad();
        }
}


void ScColumn::ResetChanged( SCROW nStartRow, SCROW nEndRow )
{
    if ( !maItems.empty() )
    {
        SCSIZE nIndex;
        Search(nStartRow,nIndex);
        while (nIndex<maItems.size() && maItems[nIndex].nRow <= nEndRow)
        {
            ScBaseCell* pCell = maItems[nIndex].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
                ((ScFormulaCell*)pCell)->ResetChanged();
            ++nIndex;
        }
    }
}


bool ScColumn::HasEditCells(SCROW nStartRow, SCROW nEndRow, SCROW& rFirst) const
{
    //  used in GetOptimalHeight - ambiguous script type counts as edit cell

    SCROW nRow = 0;
    SCSIZE nIndex;
    Search(nStartRow,nIndex);
    while ( (nIndex < maItems.size()) ? ((nRow=maItems[nIndex].nRow) <= nEndRow) : false )
    {
        ScBaseCell* pCell = maItems[nIndex].pCell;
        CellType eCellType = pCell->GetCellType();
        if ( eCellType == CELLTYPE_EDIT ||
             IsAmbiguousScriptNonZero( pDocument->GetScriptType(nCol, nRow, nTab, pCell) ) ||
             ((eCellType == CELLTYPE_FORMULA) && ((ScFormulaCell*)pCell)->IsMultilineResult()) )
        {
            rFirst = nRow;
            return true;
        }
        ++nIndex;
    }

    return false;
}


SCsROW ScColumn::SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                                bool bUp, bool bInSelection, const ScMarkData& rMark )
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
            return pAttrArray->SearchStyle( nRow, pSearchStyle, bUp,
                                    (ScMarkArray*) rMark.GetArray()+nCol );     //! const
        else
            return -1;
    }
    else
        return pAttrArray->SearchStyle( nRow, pSearchStyle, bUp, NULL );
}


bool ScColumn::SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle,
                                    bool bUp, bool bInSelection, const ScMarkData& rMark )
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
            return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp,
                                    (ScMarkArray*) rMark.GetArray()+nCol );     //! const
        else
            return false;
    }
    else
        return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp, NULL );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
