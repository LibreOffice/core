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

#include <attarray.hxx>
#include <scitems.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/justifyitem.hxx>
#include <osl/diagnose.h>
#include <svl/poolcach.hxx>

#include <global.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <patattr.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>
#include <markarr.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <segmenttree.hxx>
#include <editdataarray.hxx>
#include <cellvalue.hxx>
#include <editutil.hxx>
#include <memory>

using ::editeng::SvxBorderLine;

ScAttrArray::ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc, ScAttrArray* pDefaultColAttrArray ) :
    nCol( nNewCol ),
    nTab( nNewTab ),
    pDocument( pDoc )
{
    if ( nCol != -1 && pDefaultColAttrArray && !pDefaultColAttrArray->mvData.empty())
    {
        ScAddress aAdrStart( nCol, 0, nTab );
        ScAddress aAdrEnd( nCol, 0, nTab );
        mvData.resize( pDefaultColAttrArray->mvData.size() );
        for ( size_t nIdx = 0; nIdx < pDefaultColAttrArray->mvData.size(); ++nIdx )
        {
            mvData[nIdx].nEndRow = pDefaultColAttrArray->mvData[nIdx].nEndRow;
            ScPatternAttr aNewPattern( *(pDefaultColAttrArray->mvData[nIdx].pPattern) );
            mvData[nIdx].pPattern = static_cast<const ScPatternAttr*>( &pDocument->GetPool()->Put( aNewPattern ) );
            bool bNumFormatChanged = false;
            if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                 mvData[nIdx].pPattern->GetItemSet(), pDocument->GetDefPattern()->GetItemSet() ) )
            {
                aAdrStart.SetRow( nIdx ? mvData[nIdx-1].nEndRow+1 : 0 );
                aAdrEnd.SetRow( mvData[nIdx].nEndRow );
                pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
            }
        }
    }
}

ScAttrArray::~ScAttrArray()
{
#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif

    ScDocumentPool* pDocPool = pDocument->GetPool();
    for (auto const & rEntry : mvData)
        pDocPool->Remove(*rEntry.pPattern);
}

#if DEBUG_SC_TESTATTRARRAY
void ScAttrArray::TestData() const
{

    sal_uInt16 nErr = 0;
    SCSIZE nPos;
    for (nPos=0; nPos<nCount; nPos++)
    {
        if (nPos > 0)
            if (mvData[nPos].pPattern == mvData[nPos-1].pPattern || mvData[nPos].nRow <= mvData[nPos-1].nRow)
                ++nErr;
        if (mvData[nPos].pPattern->Which() != ATTR_PATTERN)
            ++nErr;
    }
    if ( nPos && mvData[nPos-1].nRow != MAXROW )
        ++nErr;

    SAL_WARN_IF( nErr, "sc", nErr << " errors in attribute array, column " << nCol );
}
#endif

void ScAttrArray::SetDefaultIfNotInit( SCSIZE nNeeded )
{
    if ( !mvData.empty() )
        return;

    SCSIZE nNewLimit = std::max<SCSIZE>( SC_ATTRARRAY_DELTA, nNeeded );
    mvData.reserve( nNewLimit );
    mvData.emplace_back();
    mvData[0].nEndRow = MAXROW;
    mvData[0].pPattern = pDocument->GetDefPattern(); // no put
}

void ScAttrArray::Reset( const ScPatternAttr* pPattern )
{
    ScDocumentPool*      pDocPool = pDocument->GetPool();
    ScAddress            aAdrStart( nCol, 0, nTab );
    ScAddress            aAdrEnd  ( nCol, 0, nTab );

    for (SCSIZE i=0; i<mvData.size(); i++)
    {
        // ensure that attributing changes text width of cell
        const ScPatternAttr* pOldPattern = mvData[i].pPattern;
        if ( nCol != -1 )
        {
            bool bNumFormatChanged;
            if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        pPattern->GetItemSet(), pOldPattern->GetItemSet() ) )
            {
                aAdrStart.SetRow( i ? mvData[i-1].nEndRow+1 : 0 );
                aAdrEnd  .SetRow( mvData[i].nEndRow );
                pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
            }
        }
        pDocPool->Remove(*pOldPattern);
    }
    mvData.resize(0);

    pDocument->SetStreamValid(nTab, false);

    mvData.resize(1);
    const ScPatternAttr* pNewPattern = static_cast<const ScPatternAttr*>( &pDocPool->Put(*pPattern) );
    mvData[0].nEndRow = MAXROW;
    mvData[0].pPattern = pNewPattern;
}

bool ScAttrArray::Concat(SCSIZE nPos)
{
    bool bRet = false;
    if (nPos < mvData.size())
    {
        if (nPos > 0)
        {
            if (mvData[nPos - 1].pPattern == mvData[nPos].pPattern)
            {
                mvData[nPos - 1].nEndRow = mvData[nPos].nEndRow;
                pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                mvData.erase(mvData.begin() + nPos);
                nPos--;
                bRet = true;
            }
        }
        if (nPos + 1 < mvData.size())
        {
            if (mvData[nPos + 1].pPattern == mvData[nPos].pPattern)
            {
                mvData[nPos].nEndRow = mvData[nPos + 1].nEndRow;
                pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                mvData.erase(mvData.begin() + nPos + 1);
                bRet = true;
            }
        }
    }
    return bRet;
}

/*
 * nCount is the number of runs of different attribute combinations;
 * no attribute in a column => nCount==1, one attribute somewhere => nCount == 3
 * (ie. one run with no attribute + one attribute + another run with no attribute)
 * so a range of identical attributes is only one entry in ScAttrArray.
 *
 * Iterative implementation of Binary Search
 * The same implementation was used inside ScMarkArray::Search().
 */

bool ScAttrArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
/*    auto it = std::lower_bound(mvData.begin(), mvData.end(), nRow,
                [] (const ScAttrEntry &r1, SCROW nRow)
                { return r1.nEndRow < nRow; } );
    if (it != mvData.end())
        nIndex = it - mvData.begin();
    return it != mvData.end(); */

    if (mvData.size() == 1)
    {
        nIndex = 0;
        return true;
    }

    long nHi = static_cast<long>(mvData.size()) - 1;
    long i = 0;
    long nLo = 0;

    while ( nLo <= nHi )
    {
        i = (nLo + nHi) / 2;

        if (mvData[i].nEndRow < nRow)
        {
            // If [nRow] greater, ignore left half
            nLo = i + 1;
        }
        else  if ((i > 0) && (mvData[i - 1].nEndRow >= nRow))
        {
            // If [nRow] is smaller, ignore right half
            nHi = i - 1;
        }
        else
        {
            // found
            nIndex=static_cast<SCSIZE>(i);
            return true;
        }
    }

    nIndex=0;
    return false;
}

const ScPatternAttr* ScAttrArray::GetPattern( SCROW nRow ) const
{
    if ( mvData.empty() )
    {
        if ( !ValidRow( nRow ) )
            return nullptr;
        return pDocument->GetDefPattern();
    }
    SCSIZE i;
    if (Search( nRow, i ))
        return mvData[i].pPattern;
    else
        return nullptr;
}

const ScPatternAttr* ScAttrArray::GetPatternRange( SCROW& rStartRow,
        SCROW& rEndRow, SCROW nRow ) const
{
    if ( mvData.empty() )
    {
        if ( !ValidRow( nRow ) )
            return nullptr;
        rStartRow = 0;
        rEndRow = MAXROW;
        return pDocument->GetDefPattern();
    }
    SCSIZE nIndex;
    if ( Search( nRow, nIndex ) )
    {
        if ( nIndex > 0 )
            rStartRow = mvData[nIndex-1].nEndRow + 1;
        else
            rStartRow = 0;
        rEndRow = mvData[nIndex].nEndRow;
        return mvData[nIndex].pPattern;
    }
    return nullptr;
}

void ScAttrArray::AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!ValidRow(nStartRow) || !ValidRow(nEndRow))
        return;

    if(nEndRow < nStartRow)
        return;

    SCROW nTempStartRow = nStartRow;
    SCROW nTempEndRow = nEndRow;

    do
    {
        const ScPatternAttr* pPattern = GetPattern(nTempStartRow);

        std::unique_ptr<ScPatternAttr> pNewPattern;
        if(pPattern)
        {
            pNewPattern.reset( new ScPatternAttr(*pPattern) );
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            const SfxPoolItem* pItem = nullptr;
            pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, true, &pItem );
            std::vector< sal_uInt32 > aCondFormatData;
            if(pItem)
                aCondFormatData = static_cast<const ScCondFormatItem*>(pItem)->GetCondFormatData();
            if (std::find(aCondFormatData.begin(), aCondFormatData.end(), nIndex)
                == aCondFormatData.end())
            {
                aCondFormatData.push_back(nIndex);

                ScCondFormatItem aItem;
                aItem.SetCondFormatData( aCondFormatData );
                pNewPattern->GetItemSet().Put( aItem );
            }
        }
        else
        {
            pNewPattern.reset( new ScPatternAttr( pDocument->GetPool() ) );
            ScCondFormatItem aItem;
            aItem.AddCondFormatData(nIndex);
            pNewPattern->GetItemSet().Put( aItem );
            nTempEndRow = nEndRow;
        }

        SetPatternArea( nTempStartRow, nTempEndRow, pNewPattern.get(), true );
        nTempStartRow = nTempEndRow + 1;
    }
    while(nTempEndRow < nEndRow);

}

void ScAttrArray::RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!ValidRow(nStartRow) || !ValidRow(nEndRow))
        return;

    if(nEndRow < nStartRow)
        return;

    SCROW nTempStartRow = nStartRow;
    SCROW nTempEndRow = nEndRow;

    do
    {
        const ScPatternAttr* pPattern = GetPattern(nTempStartRow);

        if(pPattern)
        {
            ScPatternAttr aPattern( *pPattern );
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            const SfxPoolItem* pItem = nullptr;
            pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, true, &pItem );
            if(pItem)
            {
                std::vector< sal_uInt32 > aCondFormatData = static_cast<const ScCondFormatItem*>(pItem)->GetCondFormatData();
                std::vector<sal_uInt32>::iterator itr = std::find(aCondFormatData.begin(), aCondFormatData.end(), nIndex);
                if(itr != aCondFormatData.end() || nIndex == 0)
                {
                    ScCondFormatItem aItem;
                    if (nIndex == 0)
                        aCondFormatData.clear();
                    else
                        aCondFormatData.erase(itr);
                    aItem.SetCondFormatData( aCondFormatData );
                    aPattern.GetItemSet().Put( aItem );
                    SetPatternArea( nTempStartRow, nTempEndRow, &aPattern, true );
                }
            }
        }
        else
        {
            return;
        }

        nTempStartRow = nTempEndRow + 1;
    }
    while(nTempEndRow < nEndRow);

}

void ScAttrArray::SetPattern( SCROW nRow, const ScPatternAttr* pPattern, bool bPutToPool )
{
    SetPatternArea( nRow, nRow, pPattern, bPutToPool );
}

void ScAttrArray::RemoveCellCharAttribs( SCROW nStartRow, SCROW nEndRow,
                                       const ScPatternAttr* pPattern, ScEditDataArray* pDataArray )
{
    assert( nCol != -1 );
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        ScAddress aPos(nCol, nRow, nTab);
        ScRefCellValue aCell(*pDocument, aPos);
        if (aCell.meType != CELLTYPE_EDIT || !aCell.mpEditText)
            continue;

        std::unique_ptr<EditTextObject> pOldData;
        if (pDataArray)
            pOldData = aCell.mpEditText->Clone();

        // Direct modification of cell content - something to watch out for if
        // we decide to share edit text instances in the future.
        ScEditUtil::RemoveCharAttribs(const_cast<EditTextObject&>(*aCell.mpEditText), *pPattern);

        if (pDataArray)
        {
            std::unique_ptr<EditTextObject> pNewData = aCell.mpEditText->Clone();
            pDataArray->AddItem(nTab, nCol, nRow, std::move(pOldData), std::move(pNewData));
        }
    }
}

bool ScAttrArray::Reserve( SCSIZE nReserve )
{
    if ( mvData.empty() && nReserve )
    {
        try {
            mvData.reserve(nReserve);
            mvData.emplace_back();
            mvData[0].nEndRow = MAXROW;
            mvData[0].pPattern = pDocument->GetDefPattern(); // no put
            return true;
        } catch (std::bad_alloc const &) {
            return false;
        }
    }
    else if ( mvData.capacity() < nReserve )
    {
        try {
            mvData.reserve(nReserve);
            return true;
        } catch (std::bad_alloc const &) {
            return false;
        }
    }
    else
        return false;
}

void ScAttrArray::SetPatternArea(SCROW nStartRow, SCROW nEndRow, const ScPatternAttr *pPattern,
                                 bool bPutToPool, ScEditDataArray* pDataArray )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        if (bPutToPool)
            pPattern = static_cast<const ScPatternAttr*>(&pDocument->GetPool()->Put(*pPattern));

        if ((nStartRow == 0) && (nEndRow == MAXROW))
            Reset(pPattern);
        else
        {
            SCSIZE nNeeded = mvData.size() + 2;
            SetDefaultIfNotInit( nNeeded );
            if ( mvData.capacity() < nNeeded )
            {
                size_t nLimit = mvData.capacity() + SC_ATTRARRAY_DELTA;
                if ( nLimit < nNeeded )
                    nLimit = nNeeded;
                mvData.reserve(nLimit);
            }

            ScAddress       aAdrStart( nCol, 0, nTab );
            ScAddress       aAdrEnd  ( nCol, 0, nTab );

            SCSIZE ni = 0;      // number of entries in beginning
            SCSIZE nx = 0;      // track position
            SCROW ns = 0;      // start row of track position
            if ( nStartRow > 0 )
            {
                // skip beginning
                SCSIZE nIndex;
                Search( nStartRow, nIndex );
                ni = nIndex;

                if ( ni > 0 )
                {
                    nx = ni;
                    ns = mvData[ni-1].nEndRow+1;
                }
            }

            // ensure that attributing changes text width of cell
            // otherwise, conditional formats need to be reset or deleted
            while ( ns <= nEndRow )
            {
                if ( nCol != -1 )
                {
                    const SfxItemSet& rNewSet = pPattern->GetItemSet();
                    const SfxItemSet& rOldSet = mvData[nx].pPattern->GetItemSet();
                    bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( std::max(nStartRow,ns) );
                        aAdrEnd  .SetRow( std::min(nEndRow,mvData[nx].nEndRow) );
                        pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                    }
                }
                ns = mvData[nx].nEndRow + 1;
                nx++;
            }

            // continue modifying data array

            SCSIZE nInsert;     // insert position (MAXROWCOUNT := no insert)
            bool bCombined = false;
            bool bSplit = false;
            if ( nStartRow > 0 )
            {
                nInsert = MAXROWCOUNT;
                if ( mvData[ni].pPattern != pPattern )
                {
                    if ( ni == 0 || (mvData[ni-1].nEndRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( mvData[ni].nEndRow > nEndRow )
                            bSplit = true;
                        ni++;
                        nInsert = ni;
                    }
                    else if (mvData[ni - 1].nEndRow == nStartRow - 1)
                        nInsert = ni;
                }
                if ( ni > 0 && mvData[ni-1].pPattern == pPattern )
                {   // combine
                    mvData[ni-1].nEndRow = nEndRow;
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
            }
            else
                nInsert = 0;

            SCSIZE nj = ni;     // stop position of range to replace
            while ( nj < mvData.size() && mvData[nj].nEndRow <= nEndRow )
                nj++;
            if ( !bSplit )
            {
                if ( nj < mvData.size() && mvData[nj].pPattern == pPattern )
                {   // combine
                    if ( ni > 0 )
                    {
                        if ( mvData[ni-1].pPattern == pPattern )
                        {   // adjacent entries
                            mvData[ni-1].nEndRow = mvData[nj].nEndRow;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            mvData[ni-1].nEndRow = nStartRow - 1;   // shrink
                    }
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
                else if ( ni > 0 && ni == nInsert )
                    mvData[ni-1].nEndRow = nStartRow - 1;   // shrink
            }
            ScDocumentPool* pDocPool = pDocument->GetPool();
            if ( bSplit )
            {   // duplicate splitted entry in pool
                pDocPool->Put( *mvData[ni-1].pPattern );
            }
            if ( ni < nj )
            {   // remove middle entries
                for ( SCSIZE nk=ni; nk<nj; nk++)
                {   // remove entries from pool
                    pDocPool->Remove( *mvData[nk].pPattern );
                }
                if ( !bCombined )
                {   // replace one entry
                    mvData[ni].nEndRow = nEndRow;
                    mvData[ni].pPattern = pPattern;
                    ni++;
                    nInsert = MAXROWCOUNT;
                }
                if ( ni < nj )
                {   // remove entries
                    mvData.erase( mvData.begin() + ni, mvData.begin() + nj);
                }
            }

            if ( nInsert < sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
            {   // insert or append new entry
                if ( nInsert <= mvData.size() )
                {
                    if ( !bSplit )
                        mvData.emplace(mvData.begin() + nInsert);
                    else
                    {
                        mvData.insert(mvData.begin() + nInsert, 2, ScAttrEntry());
                        mvData[nInsert+1] = mvData[nInsert-1];
                    }
                }
                if ( nInsert )
                    mvData[nInsert-1].nEndRow = nStartRow - 1;
                mvData[nInsert].nEndRow = nEndRow;
                mvData[nInsert].pPattern = pPattern;

                // Remove character attributes from these cells if the pattern
                // is applied during normal session.
                if (pDataArray && nCol != -1)
                    RemoveCellCharAttribs(nStartRow, nEndRow, pPattern, pDataArray);
            }

            pDocument->SetStreamValid(nTab, false);
        }
    }

#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
}

void ScAttrArray::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SetDefaultIfNotInit();
        SCSIZE nPos;
        SCROW nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            OSL_FAIL("Search Failure");
            return;
        }

        ScAddress aAdrStart( nCol, 0, nTab );
        ScAddress aAdrEnd  ( nCol, 0, nTab );

        do
        {
            const ScPatternAttr* pOldPattern = mvData[nPos].pPattern;
            std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr(*pOldPattern));
            pNewPattern->SetStyleSheet(const_cast<ScStyleSheet*>(&rStyle));
            SCROW nY1 = nStart;
            SCROW nY2 = mvData[nPos].nEndRow;
            nStart = mvData[nPos].nEndRow + 1;

            if ( *pNewPattern == *pOldPattern )
            {
                // keep the original pattern (might be default)
                // pNewPattern is deleted below
                nPos++;
            }
            else if ( nY1 < nStartRow || nY2 > nEndRow )
            {
                if (nY1 < nStartRow) nY1=nStartRow;
                if (nY2 > nEndRow) nY2=nEndRow;
                SetPatternArea( nY1, nY2, pNewPattern.get(), true );
                Search( nStart, nPos );
            }
            else
            {
                if ( nCol != -1 )
                {
                    // ensure attributing changes text width of cell; otherwise
                    // there aren't (yet) template format changes
                    const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                    const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                    bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( nPos ? mvData[nPos-1].nEndRow+1 : 0 );
                        aAdrEnd  .SetRow( mvData[nPos].nEndRow );
                        pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                    }
                }

                pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                mvData[nPos].pPattern = static_cast<const ScPatternAttr*>(
                                            &pDocument->GetPool()->Put(*pNewPattern));
                if (Concat(nPos))
                    Search(nStart, nPos);
                else
                    nPos++;
            }
        }
        while ((nStart <= nEndRow) && (nPos < mvData.size()));

        pDocument->SetStreamValid(nTab, false);
    }

#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
}

    // const cast, otherwise it will be too inefficient/complicated
static void SetLineColor(SvxBorderLine const * dest, Color c)
{
    if (dest)
    {
        const_cast<SvxBorderLine*>(dest)->SetColor(c);
    }
}

static void SetLine(const SvxBorderLine* dest, const SvxBorderLine* src)
{
    if (dest)
    {
        SvxBorderLine* pCast = const_cast<SvxBorderLine*>(dest);
        pCast->SetBorderLineStyle( src->GetBorderLineStyle() );
        pCast->SetWidth( src->GetWidth() );
    }
}

void ScAttrArray::ApplyLineStyleArea( SCROW nStartRow, SCROW nEndRow,
                                      const SvxBorderLine* pLine, bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos;
        SCROW nStart=0;
        SetDefaultIfNotInit();
        if (!Search( nStartRow, nPos ))
        {
            OSL_FAIL("Search failure");
            return;
        }

        do
        {
            const ScPatternAttr*    pOldPattern = mvData[nPos].pPattern;
            const SfxItemSet&       rOldSet = pOldPattern->GetItemSet();
            const SfxPoolItem*      pBoxItem = nullptr;
            SfxItemState            eState = rOldSet.GetItemState( ATTR_BORDER, true, &pBoxItem );
            const SfxPoolItem*      pTLBRItem = nullptr;
            SfxItemState            eTLBRState = rOldSet.GetItemState( ATTR_BORDER_TLBR, true, &pTLBRItem );
            const SfxPoolItem*      pBLTRItem = nullptr;
            SfxItemState            eBLTRState = rOldSet.GetItemState( ATTR_BORDER_BLTR, true, &pBLTRItem );

            if ( (SfxItemState::SET == eState) || (SfxItemState::SET == eTLBRState) || (SfxItemState::SET == eBLTRState) )
            {
                std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr(*pOldPattern));
                SfxItemSet&     rNewSet = pNewPattern->GetItemSet();
                SCROW           nY1 = nStart;
                SCROW           nY2 = mvData[nPos].nEndRow;

                std::unique_ptr<SvxBoxItem>  pNewBoxItem( pBoxItem ? static_cast<SvxBoxItem*>(pBoxItem->Clone()) : nullptr);
                std::unique_ptr<SvxLineItem> pNewTLBRItem( pTLBRItem ? static_cast<SvxLineItem*>(pTLBRItem->Clone()) : nullptr);
                std::unique_ptr<SvxLineItem> pNewBLTRItem(pBLTRItem ? static_cast<SvxLineItem*>(pBLTRItem->Clone()) : nullptr);

                // fetch line and update attributes with parameters

                if ( !pLine )
                {
                    if( pNewBoxItem )
                    {
                        if ( pNewBoxItem->GetTop() )    pNewBoxItem->SetLine( nullptr, SvxBoxItemLine::TOP );
                        if ( pNewBoxItem->GetBottom() ) pNewBoxItem->SetLine( nullptr, SvxBoxItemLine::BOTTOM );
                        if ( pNewBoxItem->GetLeft() )   pNewBoxItem->SetLine( nullptr, SvxBoxItemLine::LEFT );
                        if ( pNewBoxItem->GetRight() )  pNewBoxItem->SetLine( nullptr, SvxBoxItemLine::RIGHT );
                    }
                    if( pNewTLBRItem && pNewTLBRItem->GetLine() )
                        pNewTLBRItem->SetLine( nullptr );
                    if( pNewBLTRItem && pNewBLTRItem->GetLine() )
                        pNewBLTRItem->SetLine( nullptr );
                }
                else
                {
                    if ( bColorOnly )
                    {
                        Color aColor( pLine->GetColor() );
                        if( pNewBoxItem )
                        {
                            SetLineColor( pNewBoxItem->GetTop(),    aColor );
                            SetLineColor( pNewBoxItem->GetBottom(), aColor );
                            SetLineColor( pNewBoxItem->GetLeft(),   aColor );
                            SetLineColor( pNewBoxItem->GetRight(),   aColor );
                        }
                        if( pNewTLBRItem )
                            SetLineColor( pNewTLBRItem->GetLine(), aColor );
                        if( pNewBLTRItem )
                            SetLineColor( pNewBLTRItem->GetLine(), aColor );
                    }
                    else
                    {
                        if( pNewBoxItem )
                        {
                            SetLine( pNewBoxItem->GetTop(),    pLine );
                            SetLine( pNewBoxItem->GetBottom(), pLine );
                            SetLine( pNewBoxItem->GetLeft(),   pLine );
                            SetLine( pNewBoxItem->GetRight(),   pLine );
                        }
                        if( pNewTLBRItem )
                            SetLine( pNewTLBRItem->GetLine(), pLine );
                        if( pNewBLTRItem )
                            SetLine( pNewBLTRItem->GetLine(), pLine );
                    }
                }
                if( pNewBoxItem )   rNewSet.Put( *pNewBoxItem );
                if( pNewTLBRItem )  rNewSet.Put( *pNewTLBRItem );
                if( pNewBLTRItem )  rNewSet.Put( *pNewBLTRItem );

                nStart = mvData[nPos].nEndRow + 1;

                if ( nY1 < nStartRow || nY2 > nEndRow )
                {
                    if (nY1 < nStartRow) nY1=nStartRow;
                    if (nY2 > nEndRow) nY2=nEndRow;
                    SetPatternArea( nY1, nY2, pNewPattern.get(), true );
                    Search( nStart, nPos );
                }
                else
                {
                    // remove from pool ?
                    pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                    mvData[nPos].pPattern = static_cast<const ScPatternAttr*>(
                                &pDocument->GetPool()->Put(*pNewPattern) );

                    if (Concat(nPos))
                        Search(nStart, nPos);
                    else
                        nPos++;
                }
            }
            else
            {
                nStart = mvData[nPos].nEndRow + 1;
                nPos++;
            }
        }
        while ((nStart <= nEndRow) && (nPos < mvData.size()));
    }
}

void ScAttrArray::ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache, ScEditDataArray* pDataArray, bool* const pIsChanged )
{
#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif

    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos;
        SCROW nStart=0;
        SetDefaultIfNotInit();
        if (!Search( nStartRow, nPos ))
        {
            OSL_FAIL("Search Failure");
            return;
        }

        ScAddress aAdrStart( nCol, 0, nTab );
        ScAddress aAdrEnd  ( nCol, 0, nTab );

        do
        {
            const ScPatternAttr* pOldPattern = mvData[nPos].pPattern;
            const ScPatternAttr* pNewPattern = static_cast<const ScPatternAttr*>( &pCache->ApplyTo( *pOldPattern ) );
            if (pNewPattern != pOldPattern)
            {
                SCROW nY1 = nStart;
                SCROW nY2 = mvData[nPos].nEndRow;
                nStart = mvData[nPos].nEndRow + 1;

                if(pIsChanged)
                    *pIsChanged = true;

                if ( nY1 < nStartRow || nY2 > nEndRow )
                {
                    if (nY1 < nStartRow) nY1=nStartRow;
                    if (nY2 > nEndRow) nY2=nEndRow;
                    SetPatternArea( nY1, nY2, pNewPattern, false, pDataArray );
                    Search( nStart, nPos );
                }
                else
                {
                    if ( nCol != -1 )
                    {
                        // ensure attributing changes text-width of cell

                        const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                        const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                        bool bNumFormatChanged;
                        if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                                rNewSet, rOldSet ) )
                        {
                            aAdrStart.SetRow( nPos ? mvData[nPos-1].nEndRow+1 : 0 );
                            aAdrEnd  .SetRow( mvData[nPos].nEndRow );
                            pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                        }
                    }

                    pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                    mvData[nPos].pPattern = pNewPattern;
                    if (Concat(nPos))
                        Search(nStart, nPos);
                    else
                        ++nPos;
                }
            }
            else
            {
                nStart = mvData[nPos].nEndRow + 1;
                ++nPos;
            }
        }
        while (nStart <= nEndRow);

        pDocument->SetStreamValid(nTab, false);
    }

#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
}

void ScAttrArray::SetAttrEntries(std::vector<ScAttrEntry> && vNewData)
{
    ScDocumentPool* pDocPool = pDocument->GetPool();
    for (auto const & rEntry : mvData)
        pDocPool->Remove(*rEntry.pPattern);

    mvData = std::move(vNewData);
}

static void lcl_MergeDeep( SfxItemSet& rMergeSet, const SfxItemSet& rSource )
{
    const SfxPoolItem* pNewItem;
    const SfxPoolItem* pOldItem;
    for (sal_uInt16 nId=ATTR_PATTERN_START; nId<=ATTR_PATTERN_END; nId++)
    {
        //  pMergeSet has no parent
        SfxItemState eOldState = rMergeSet.GetItemState( nId, false, &pOldItem );

        if ( eOldState == SfxItemState::DEFAULT )
        {
            SfxItemState eNewState = rSource.GetItemState( nId, true, &pNewItem );
            if ( eNewState == SfxItemState::SET )
            {
                if ( *pNewItem != rMergeSet.GetPool()->GetDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        else if ( eOldState == SfxItemState::SET ) // Item set
        {
            SfxItemState eNewState = rSource.GetItemState( nId, true, &pNewItem );
            if ( eNewState == SfxItemState::SET )
            {
                if ( pNewItem != pOldItem ) // Both pulled
                    rMergeSet.InvalidateItem( nId );
            }
            else // Default
            {
                if ( *pOldItem != rSource.GetPool()->GetDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        // Dontcare remains Dontcare
    }
}

void ScAttrArray::MergePatternArea( SCROW nStartRow, SCROW nEndRow,
                                    ScMergePatternState& rState, bool bDeep ) const
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos = 0;
        SCROW nStart=0;
        if ( !mvData.empty() && !Search( nStartRow, nPos ) )
        {
            OSL_FAIL("Search failure");
            return;
        }

        do
        {
            // similar patterns must not be repeated
            const ScPatternAttr* pPattern = nullptr;
            if ( !mvData.empty() )
                pPattern = mvData[nPos].pPattern;
            else
                pPattern = pDocument->GetDefPattern();
            if ( pPattern != rState.pOld1 && pPattern != rState.pOld2 )
            {
                const SfxItemSet& rThisSet = pPattern->GetItemSet();
                if (rState.pItemSet)
                {
                    rState.mbValidPatternId = false;
                    if (bDeep)
                        lcl_MergeDeep( *rState.pItemSet, rThisSet );
                    else
                        rState.pItemSet->MergeValues( rThisSet );
                }
                else
                {
                    // first pattern - copied from parent
                    rState.pItemSet = std::make_unique<SfxItemSet>( *rThisSet.GetPool(), rThisSet.GetRanges() );
                    rState.pItemSet->Set( rThisSet, bDeep );
                    rState.mnPatternId = pPattern->GetKey();
                }

                rState.pOld2 = rState.pOld1;
                rState.pOld1 = pPattern;
            }

            if ( !mvData.empty() )
                nStart = mvData[nPos].nEndRow + 1;
            else
                nStart = MAXROW + 1;
            ++nPos;
        }
        while (nStart <= nEndRow);
    }
}

// assemble border

static bool lcl_TestAttr( const SvxBorderLine* pOldLine, const SvxBorderLine* pNewLine,
                            sal_uInt8& rModified, const SvxBorderLine*& rpNew )
{
    if (rModified == SC_LINE_DONTCARE)
        return false;               // don't go again

    if (rModified == SC_LINE_EMPTY)
    {
        rModified = SC_LINE_SET;
        rpNew = pNewLine;
        return true;                // initial value
    }

    if (pOldLine == pNewLine)
    {
        rpNew = pOldLine;
        return false;
    }

    if (pOldLine && pNewLine)
        if (*pOldLine == *pNewLine)
        {
            rpNew = pOldLine;
            return false;
        }

    rModified = SC_LINE_DONTCARE;
    rpNew = nullptr;
    return true;              // another line -> don't care
}

static void lcl_MergeToFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                ScLineFlags& rFlags, const ScPatternAttr* pPattern,
                                bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom )
{
    // right/bottom border set when connected together
    const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
    if ( rMerge.GetColMerge() == nDistRight + 1 )
        nDistRight = 0;
    if ( rMerge.GetRowMerge() == nDistBottom + 1 )
        nDistBottom = 0;

    const SvxBoxItem* pCellFrame = &pPattern->GetItemSet().Get( ATTR_BORDER );
    const SvxBorderLine* pLeftAttr   = pCellFrame->GetLeft();
    const SvxBorderLine* pRightAttr  = pCellFrame->GetRight();
    const SvxBorderLine* pTopAttr    = pCellFrame->GetTop();
    const SvxBorderLine* pBottomAttr = pCellFrame->GetBottom();
    const SvxBorderLine* pNew;

    if (bTop)
    {
        if (lcl_TestAttr( pLineOuter->GetTop(), pTopAttr, rFlags.nTop, pNew ))
            pLineOuter->SetLine( pNew, SvxBoxItemLine::TOP );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetHori(), pTopAttr, rFlags.nHori, pNew ))
            pLineInner->SetLine( pNew, SvxBoxInfoItemLine::HORI );
    }

    if (nDistBottom == 0)
    {
        if (lcl_TestAttr( pLineOuter->GetBottom(), pBottomAttr, rFlags.nBottom, pNew ))
            pLineOuter->SetLine( pNew, SvxBoxItemLine::BOTTOM );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetHori(), pBottomAttr, rFlags.nHori, pNew ))
            pLineInner->SetLine( pNew, SvxBoxInfoItemLine::HORI );
    }

    if (bLeft)
    {
        if (lcl_TestAttr( pLineOuter->GetLeft(), pLeftAttr, rFlags.nLeft, pNew ))
            pLineOuter->SetLine( pNew, SvxBoxItemLine::LEFT );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetVert(), pLeftAttr, rFlags.nVert, pNew ))
            pLineInner->SetLine( pNew, SvxBoxInfoItemLine::VERT );
    }

    if (nDistRight == 0)
    {
        if (lcl_TestAttr( pLineOuter->GetRight(), pRightAttr, rFlags.nRight, pNew ))
            pLineOuter->SetLine( pNew, SvxBoxItemLine::RIGHT );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetVert(), pRightAttr, rFlags.nVert, pNew ))
            pLineInner->SetLine( pNew, SvxBoxInfoItemLine::VERT );
    }
}

void ScAttrArray::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                    ScLineFlags& rFlags,
                    SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const
{
    const ScPatternAttr* pPattern;

    if (nStartRow == nEndRow)
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, true, 0 );
    }
    else if ( !mvData.empty() ) // non-default pattern
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, true,
                            nEndRow-nStartRow );

        SCSIZE nStartIndex;
        SCSIZE nEndIndex;
        Search( nStartRow+1, nStartIndex );
        Search( nEndRow-1, nEndIndex );
        for (SCSIZE i=nStartIndex; i<=nEndIndex; i++)
        {
            pPattern = mvData[i].pPattern;
            lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false,
                            nEndRow - std::min( mvData[i].nEndRow, static_cast<SCROW>(nEndRow-1) ) );
            // nDistBottom here always > 0
        }

        pPattern = GetPattern( nEndRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false, 0 );
    }
    else
    {
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pDocument->GetDefPattern(), bLeft, nDistRight, true, 0 );
    }
}

// apply border

// ApplyFrame - on an entry into the array

bool ScAttrArray::ApplyFrame( const SvxBoxItem*     pBoxItem,
                              const SvxBoxInfoItem* pBoxInfoItem,
                              SCROW nStartRow, SCROW nEndRow,
                              bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom )
{
    OSL_ENSURE( pBoxItem && pBoxInfoItem, "Missing line attributes!" );

    const ScPatternAttr* pPattern = GetPattern( nStartRow );
    const SvxBoxItem* pOldFrame = &pPattern->GetItemSet().Get( ATTR_BORDER );

    // right/bottom border set when connected together
    const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
    if ( rMerge.GetColMerge() == nDistRight + 1 )
        nDistRight = 0;
    if ( rMerge.GetRowMerge() == nDistBottom + 1 )
        nDistBottom = 0;

    SvxBoxItem aNewFrame( *pOldFrame );
    bool bRTL=pDocument->IsLayoutRTL(nTab);
    // fdo#37464 check if the sheet are RTL then replace right <=> left
    if (bRTL)
    {
        if( bLeft && nDistRight==0)
        {
            if ( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) )
                aNewFrame.SetLine( pBoxItem->GetLeft(), SvxBoxItemLine::RIGHT );
            if ( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) )
                aNewFrame.SetLine( pBoxItem->GetRight(), SvxBoxItemLine::LEFT );
        }
        else
        {
            if ( (nDistRight==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
                aNewFrame.SetLine( (nDistRight==0) ? pBoxItem->GetLeft() : pBoxInfoItem->GetVert(),
                    SvxBoxItemLine::RIGHT );
            if ( bLeft ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
                aNewFrame.SetLine( bLeft ? pBoxItem->GetRight() : pBoxInfoItem->GetVert(),
                    SvxBoxItemLine::LEFT );
        }
    }
    else
    {
        if ( bLeft ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            aNewFrame.SetLine( bLeft ? pBoxItem->GetLeft() : pBoxInfoItem->GetVert(),
                SvxBoxItemLine::LEFT );
        if ( (nDistRight==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            aNewFrame.SetLine( (nDistRight==0) ? pBoxItem->GetRight() : pBoxInfoItem->GetVert(),
                SvxBoxItemLine::RIGHT );
    }
    if ( bTop ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::TOP) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
        aNewFrame.SetLine( bTop ? pBoxItem->GetTop() : pBoxInfoItem->GetHori(),
            SvxBoxItemLine::TOP );
    if ( (nDistBottom==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::BOTTOM) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
        aNewFrame.SetLine( (nDistBottom==0) ? pBoxItem->GetBottom() : pBoxInfoItem->GetHori(),
            SvxBoxItemLine::BOTTOM );

    if (aNewFrame == *pOldFrame)
    {
        // nothing to do
        return false;
    }
    else
    {
        SfxItemPoolCache aCache( pDocument->GetPool(), &aNewFrame );
        ApplyCacheArea( nStartRow, nEndRow, &aCache );

        return true;
    }
}

void ScAttrArray::ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                                  SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight)
{
    if (nStartRow == nEndRow)
        ApplyFrame(&rLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, true, 0);
    else if ( !mvData.empty() )
    {
        ApplyFrame(&rLineOuter, pLineInner, nStartRow, nStartRow, bLeft, nDistRight,
                   true, nEndRow-nStartRow);

        if ( nEndRow > nStartRow+1 )     // inner part available?
        {
            SCSIZE nStartIndex;
            SCSIZE nEndIndex;
            Search( nStartRow+1, nStartIndex );
            Search( nEndRow-1, nEndIndex );
            SCROW nTmpStart = nStartRow+1;
            SCROW nTmpEnd;
            for (SCSIZE i=nStartIndex; i<=nEndIndex;)
            {
                nTmpEnd = std::min( static_cast<SCROW>(nEndRow-1), mvData[i].nEndRow );
                bool bChanged = ApplyFrame(&rLineOuter, pLineInner, nTmpStart, nTmpEnd,
                                           bLeft, nDistRight, false, nEndRow - nTmpEnd);
                nTmpStart = nTmpEnd+1;
                if (bChanged)
                {
                    Search(nTmpStart, i);
                    Search(nEndRow-1, nEndIndex);
                }
                else
                    i++;
            }
        }

        ApplyFrame(&rLineOuter, pLineInner, nEndRow, nEndRow, bLeft, nDistRight, false, 0);
    }
    else
    {
        ApplyFrame(&rLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, true, 0);
    }
}

bool ScAttrArray::HasAttrib_Impl(const ScPatternAttr* pPattern, HasAttrFlags nMask, SCROW nRow1, SCROW nRow2, SCSIZE i) const
{
    bool bFound = false;
    if ( nMask & HasAttrFlags::Merged )
    {
        const ScMergeAttr* pMerge = &pPattern->GetItem( ATTR_MERGE );
        if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
            bFound = true;
    }
    if ( nMask & ( HasAttrFlags::Overlapped | HasAttrFlags::NotOverlapped | HasAttrFlags::AutoFilter ) )
    {
        const ScMergeFlagAttr* pMergeFlag = &pPattern->GetItem( ATTR_MERGE_FLAG );
        if ( (nMask & HasAttrFlags::Overlapped) && pMergeFlag->IsOverlapped() )
            bFound = true;
        if ( (nMask & HasAttrFlags::NotOverlapped) && !pMergeFlag->IsOverlapped() )
            bFound = true;
        if ( (nMask & HasAttrFlags::AutoFilter) && pMergeFlag->HasAutoFilter() )
            bFound = true;
    }
    if ( nMask & HasAttrFlags::Lines )
    {
        const SvxBoxItem* pBox = &pPattern->GetItem( ATTR_BORDER );
        if ( pBox->GetLeft() || pBox->GetRight() || pBox->GetTop() || pBox->GetBottom() )
            bFound = true;
    }
    if ( nMask & HasAttrFlags::Shadow )
    {
        const SvxShadowItem* pShadow = &pPattern->GetItem( ATTR_SHADOW );
        if ( pShadow->GetLocation() != SvxShadowLocation::NONE )
            bFound = true;
    }
    if ( nMask & HasAttrFlags::Conditional )
    {
        bool bContainsCondFormat = pPattern->GetItem( ATTR_CONDITIONAL ).GetCondFormatData().empty();
        if ( bContainsCondFormat )
            bFound = true;
    }
    if ( nMask & HasAttrFlags::Protected )
    {
        const ScProtectionAttr* pProtect = &pPattern->GetItem( ATTR_PROTECTION );
        bool bFoundTemp = false;
        if ( pProtect->GetProtection() || pProtect->GetHideCell() )
            bFoundTemp = true;

        bool bContainsCondFormat = !mvData.empty() &&
            !pPattern->GetItem( ATTR_CONDITIONAL ).GetCondFormatData().empty();
        if ( bContainsCondFormat && nCol != -1 ) // pDocument->GetCondResult() is valid only for real columns.
        {
            SCROW nRowStartCond = std::max<SCROW>( nRow1, i ? mvData[i-1].nEndRow + 1: 0 );
            SCROW nRowEndCond = std::min<SCROW>( nRow2, mvData[i].nEndRow );
            bool bFoundCond = false;
            for(SCROW nRowCond = nRowStartCond; nRowCond <= nRowEndCond && !bFoundCond; ++nRowCond)
            {
                const SfxItemSet* pSet = pDocument->GetCondResult( nCol, nRowCond, nTab );

                const SfxPoolItem* pItem;
                if( pSet && pSet->GetItemState( ATTR_PROTECTION, true, &pItem ) == SfxItemState::SET )
                {
                    const ScProtectionAttr* pCondProtect = static_cast<const ScProtectionAttr*>(pItem);
                    if( pCondProtect->GetProtection() || pCondProtect->GetHideCell() )
                        bFoundCond = true;
                    else
                        break;
                }
                else
                {
                    // well it is not true that we found one
                    // but existing one + cell where conditional
                    // formatting does not remove it
                    // => we should use the existing protection setting
                    bFoundCond = bFoundTemp;
                }
            }
            bFoundTemp = bFoundCond;
        }

        if(bFoundTemp)
            bFound = true;
    }
    if ( nMask & HasAttrFlags::Rotate )
    {
        const SfxInt32Item* pRotate = &pPattern->GetItem( ATTR_ROTATE_VALUE );
        // 90 or 270 degrees is former SvxOrientationItem - only look for other values
        // (see ScPatternAttr::GetCellOrientation)
        sal_Int32 nAngle = pRotate->GetValue();
        if ( nAngle != 0 && nAngle != 9000 && nAngle != 27000 )
            bFound = true;
    }
    if ( nMask & HasAttrFlags::NeedHeight )
    {
        if (pPattern->GetCellOrientation() != SvxCellOrientation::Standard)
            bFound = true;
        else if (pPattern->GetItem( ATTR_LINEBREAK ).GetValue())
            bFound = true;
        else if (pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue() == SvxCellHorJustify::Block)
            bFound = true;

        else if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
            bFound = true;
        else if (pPattern->GetItem( ATTR_ROTATE_VALUE ).GetValue())
            bFound = true;
    }
    if ( nMask & ( HasAttrFlags::ShadowRight | HasAttrFlags::ShadowDown ) )
    {
        const SvxShadowItem* pShadow = &pPattern->GetItem( ATTR_SHADOW );
        SvxShadowLocation eLoc = pShadow->GetLocation();
        if ( nMask & HasAttrFlags::ShadowRight )
            if ( eLoc == SvxShadowLocation::TopRight || eLoc == SvxShadowLocation::BottomRight )
                bFound = true;
        if ( nMask & HasAttrFlags::ShadowDown )
            if ( eLoc == SvxShadowLocation::BottomLeft || eLoc == SvxShadowLocation::BottomRight )
                bFound = true;
    }
    if ( nMask & HasAttrFlags::RightOrCenter )
    {
        //  called only if the sheet is LTR, so physical=logical alignment can be assumed
        SvxCellHorJustify eHorJust = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();
        if ( eHorJust == SvxCellHorJustify::Right || eHorJust == SvxCellHorJustify::Center )
            bFound = true;
    }

    return bFound;
}

// Test if field contains specific attribute
bool ScAttrArray::HasAttrib( SCROW nRow1, SCROW nRow2, HasAttrFlags nMask ) const
{
    if (mvData.empty())
    {
        return HasAttrib_Impl(pDocument->GetDefPattern(), nMask, 0, MAXROW, 0);
    }

    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nRow1, nStartIndex );
    if (nRow1 != nRow2)
        Search( nRow2, nEndIndex );
    else
        nEndIndex = nStartIndex;
    bool bFound = false;

    for (SCSIZE i=nStartIndex; i<=nEndIndex && !bFound; i++)
    {
        const ScPatternAttr* pPattern = mvData[i].pPattern;
        bFound = HasAttrib_Impl(pPattern, nMask, nRow1, nRow2, i);
    }

    return bFound;
}

bool ScAttrArray::IsMerged( SCROW nRow ) const
{
    if ( !mvData.empty() )
    {
        SCSIZE nIndex;
        Search(nRow, nIndex);
        const ScMergeAttr& rItem = mvData[nIndex].pPattern->GetItem(ATTR_MERGE);

        return rItem.IsMerged();
    }

    return pDocument->GetDefPattern()->GetItem(ATTR_MERGE).IsMerged();
}

/**
 * Area around any given summaries expand and adapt any MergeFlag (bRefresh)
 */
bool ScAttrArray::ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh )
{
    assert( nCol != -1 );
    SetDefaultIfNotInit();
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nStartRow, nStartIndex );
    Search( nEndRow, nEndIndex );
    bool bFound = false;

    for (SCSIZE i=nStartIndex; i<=nEndIndex; i++)
    {
        pPattern = mvData[i].pPattern;
        pItem = &pPattern->GetItem( ATTR_MERGE );
        SCCOL  nCountX = pItem->GetColMerge();
        SCROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            SCROW nThisRow = (i>0) ? mvData[i-1].nEndRow+1 : 0;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisRow + nCountY - 1;
            if (nMergeEndCol > rPaintCol && nMergeEndCol <= MAXCOL)
                rPaintCol = nMergeEndCol;
            if (nMergeEndRow > rPaintRow && nMergeEndRow <= MAXROW)
                rPaintRow = nMergeEndRow;
            bFound = true;

            if (bRefresh)
            {
                if ( nMergeEndCol > nThisCol )
                    pDocument->ApplyFlagsTab( nThisCol+1, nThisRow, nMergeEndCol, mvData[i].nEndRow,
                                nTab, ScMF::Hor );
                if ( nMergeEndRow > nThisRow )
                    pDocument->ApplyFlagsTab( nThisCol, nThisRow+1, nThisCol, nMergeEndRow,
                                nTab, ScMF::Ver );
                if ( nMergeEndCol > nThisCol && nMergeEndRow > nThisRow )
                    pDocument->ApplyFlagsTab( nThisCol+1, nThisRow+1, nMergeEndCol, nMergeEndRow,
                                nTab, ScMF::Hor | ScMF::Ver );

                Search( nThisRow, i );    // Data changed
                Search( nStartRow, nStartIndex );
                Search( nEndRow, nEndIndex );
            }
        }
    }

    return bFound;
}

void ScAttrArray::RemoveAreaMerge(SCROW nStartRow, SCROW nEndRow)
{
    assert( nCol != -1 );
    SetDefaultIfNotInit();
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    SCSIZE nIndex;

    Search( nStartRow, nIndex );
    SCROW nThisStart = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisStart < nStartRow)
        nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        SCROW nThisEnd = mvData[nIndex].nEndRow;
        if (nThisEnd > nEndRow)
            nThisEnd = nEndRow;

        pPattern = mvData[nIndex].pPattern;
        pItem = &pPattern->GetItem( ATTR_MERGE );
        SCCOL  nCountX = pItem->GetColMerge();
        SCROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            const ScMergeAttr* pAttr = &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
            const ScMergeFlagAttr* pFlagAttr = &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE_FLAG );

            OSL_ENSURE( nCountY==1 || nThisStart==nThisEnd, "What's up?" );

            SCCOL nThisCol = nCol;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisEnd + nCountY - 1;

            // ApplyAttr for areas
            for (SCROW nThisRow = nThisStart; nThisRow <= nThisEnd; nThisRow++)
                pDocument->ApplyAttr( nThisCol, nThisRow, nTab, *pAttr );

            std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr( pDocument->GetPool() ));
            SfxItemSet*     pSet = &pNewPattern->GetItemSet();
            pSet->Put( *pFlagAttr );
            pDocument->ApplyPatternAreaTab( nThisCol, nThisStart, nMergeEndCol, nMergeEndRow,
                                                nTab, *pNewPattern );
            pNewPattern.reset();

            Search( nThisEnd, nIndex );    // data changed
        }

        ++nIndex;
        if ( nIndex < mvData.size() )
            nThisStart = mvData[nIndex-1].nEndRow+1;
        else
            nThisStart = MAXROW+1;   // End
    }
}

void ScAttrArray::SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                        const ScPatternAttr* pWantedPattern, bool bDefault )
{
    SetDefaultIfNotInit();
    const ScPatternAttr*    pOldPattern;
    const ScMergeFlagAttr*  pItem;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bFirstUse = true;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    while ( nThisRow <= nEndRow )
    {
        pOldPattern = mvData[nIndex].pPattern;
        if (pOldPattern != pWantedPattern) // FIXME: else-branch?
        {
            if (nThisRow < nStartRow) nThisRow = nStartRow;
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            pItem = &pOldPattern->GetItem( ATTR_MERGE_FLAG );

            if (pItem->IsOverlapped() || pItem->HasAutoFilter())
            {
                //  default-constructing a ScPatternAttr for DeleteArea doesn't work
                //  because it would have no cell style information.
                //  Instead, the document's GetDefPattern is copied. Since it is passed as
                //  pWantedPattern, no special treatment of default is needed here anymore.
                std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr( *pWantedPattern ));
                SfxItemSet*     pSet = &pNewPattern->GetItemSet();
                pSet->Put( *pItem );
                SetPatternArea( nThisRow, nAttrRow, pNewPattern.get(), true );
            }
            else
            {
                if ( !bDefault )
                {
                    if (bFirstUse)
                        bFirstUse = false;
                    else
                        // it's in the pool
                        pDocument->GetPool()->Put( *pWantedPattern );
                }
                SetPatternArea( nThisRow, nAttrRow, pWantedPattern );
            }

            Search( nThisRow, nIndex );   // data changed
        }

        ++nIndex;
        nThisRow = mvData[nIndex-1].nEndRow+1;
    }
}

bool ScAttrArray::ApplyFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
{
    SetDefaultIfNotInit();
    const ScPatternAttr* pOldPattern;

    ScMF    nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bChanged = false;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = mvData[nIndex].pPattern;
        nOldValue = pOldPattern->GetItem( ATTR_MERGE_FLAG ).GetValue();
        if ( (nOldValue | nFlags) != nOldValue )
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue | nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
            bChanged = true;
        }

        ++nIndex;
        nThisRow = mvData[nIndex-1].nEndRow+1;
    }

    return bChanged;
}

bool ScAttrArray::RemoveFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags )
{
    SetDefaultIfNotInit();
    const ScPatternAttr* pOldPattern;

    ScMF    nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bChanged = false;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = mvData[nIndex].pPattern;
        nOldValue = pOldPattern->GetItem( ATTR_MERGE_FLAG ).GetValue();
        if ( (nOldValue & ~nFlags) != nOldValue )
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue & ~nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
            bChanged = true;
        }

        ++nIndex;
        nThisRow = mvData[nIndex-1].nEndRow+1;
    }

    return bChanged;
}

void ScAttrArray::ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich )
{
    SetDefaultIfNotInit();
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = mvData[nIndex].pPattern;
        if ( pOldPattern->HasItemsSet( pWhich ) )
        {
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.ClearItems( pWhich );

            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
        }

        ++nIndex;
        nThisRow = mvData[nIndex-1].nEndRow+1;
    }
}

void ScAttrArray::ChangeIndent( SCROW nStartRow, SCROW nEndRow, bool bIncrement )
{
    SetDefaultIfNotInit();
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisStart < nStartRow) nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = mvData[nIndex].pPattern;
        const SfxItemSet& rOldSet = pOldPattern->GetItemSet();
        const SfxPoolItem* pItem;

        bool bNeedJust = ( rOldSet.GetItemState( ATTR_HOR_JUSTIFY, false, &pItem ) != SfxItemState::SET
                           || (static_cast<const SvxHorJustifyItem*>(pItem)->GetValue() != SvxCellHorJustify::Left &&
                               static_cast<const SvxHorJustifyItem*>(pItem)->GetValue() != SvxCellHorJustify::Right ));
        sal_uInt16 nOldValue = rOldSet.Get( ATTR_INDENT ).GetValue();
        sal_uInt16 nNewValue = nOldValue;
        // To keep Increment indent from running outside the cell1659
        long nColWidth = static_cast<long>(pDocument->GetColWidth(nCol,nTab));
        if ( bIncrement )
        {
            if ( nNewValue < nColWidth-SC_INDENT_STEP )
            {
                nNewValue += SC_INDENT_STEP;
                if ( nNewValue > nColWidth-SC_INDENT_STEP ) nNewValue = nColWidth-SC_INDENT_STEP;
            }
        }
        else
        {
            if ( nNewValue > 0 )
            {
                if ( nNewValue > SC_INDENT_STEP )
                    nNewValue -= SC_INDENT_STEP;
                else
                    nNewValue = 0;
            }
        }

        if ( bNeedJust || nNewValue != nOldValue )
        {
            SCROW nThisEnd = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nThisEnd, nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, nNewValue ) );
            if ( bNeedJust )
                aNewPattern.GetItemSet().Put(
                                SvxHorJustifyItem( SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY ) );
            SetPatternArea( nThisStart, nAttrRow, &aNewPattern, true );

            nThisStart = nThisEnd + 1;
            Search( nThisStart, nIndex ); // data changed
        }
        else
        {
            nThisStart = mvData[nIndex].nEndRow + 1;
            ++nIndex;
        }
    }
}

SCROW ScAttrArray::GetNextUnprotected( SCROW nRow, bool bUp ) const
{
    long nRet = nRow;
    if (ValidRow(nRow))
    {
        if ( mvData.empty() )
        {
            if ( bUp )
                return -1;
            else
                return MAXROW+1;
        }

        SCSIZE nIndex;
        Search(nRow, nIndex);
        while (mvData[nIndex].pPattern->
                GetItem(ATTR_PROTECTION).GetProtection())
        {
            if (bUp)
            {
                if (nIndex==0)
                    return -1;   // not found
                --nIndex;
                nRet = mvData[nIndex].nEndRow;
            }
            else
            {
                nRet = mvData[nIndex].nEndRow+1;
                ++nIndex;
                if (nIndex >= mvData.size())
                    return MAXROW+1; // not found
            }
        }
    }
    return nRet;
}

void ScAttrArray::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset )
{
    SetDefaultIfNotInit();
    SCROW nStart = 0;
    SCSIZE nPos = 0;
    while (nPos < mvData.size())
    {
        SCROW nEnd = mvData[nPos].nEndRow;
        if (mvData[nPos].pPattern->GetStyleSheet() == pStyleSheet)
        {
            rUsedRows.setTrue(nStart, nEnd);

            if (bReset)
            {
                std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr(*mvData[nPos].pPattern));
                pDocument->GetPool()->Remove(*mvData[nPos].pPattern);
                pNewPattern->SetStyleSheet( static_cast<ScStyleSheet*>(
                    pDocument->GetStyleSheetPool()->
                        Find( ScResId(STR_STYLENAME_STANDARD),
                              SfxStyleFamily::Para,
                              SfxStyleSearchBits::Auto | SfxStyleSearchBits::ScStandard ) ) );
                mvData[nPos].pPattern = static_cast<const ScPatternAttr*>(
                                            &pDocument->GetPool()->Put(*pNewPattern));
                pNewPattern.reset();

                if (Concat(nPos))
                {
                    Search(nStart, nPos);
                    --nPos;   // because ++ at end
                }
            }
        }
        nStart = nEnd + 1;
        ++nPos;
    }
}

bool ScAttrArray::IsStyleSheetUsed( const ScStyleSheet& rStyle ) const
{
    if ( mvData.empty() )
    {
        const ScStyleSheet* pStyle = pDocument->GetDefPattern()->GetStyleSheet();
        if ( pStyle )
        {
            pStyle->SetUsage( ScStyleSheet::Usage::USED );
            if ( pStyle == &rStyle )
                return true;
        }
        return false;
    }

    bool    bIsUsed = false;
    SCSIZE  nPos    = 0;

    while ( nPos < mvData.size() )
    {
        const ScStyleSheet* pStyle = mvData[nPos].pPattern->GetStyleSheet();
        if ( pStyle )
        {
            pStyle->SetUsage( ScStyleSheet::Usage::USED );
            if ( pStyle == &rStyle )
            {
                bIsUsed = true;
            }
        }
        nPos++;
    }

    return bIsUsed;
}

bool ScAttrArray::IsEmpty() const
{
    if ( mvData.empty() )
        return true;

    if (mvData.size() == 1)
    {
        return mvData[0].pPattern == pDocument->GetDefPattern();
    }
    else
        return false;
}

bool ScAttrArray::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    if ( mvData.empty() )
        return false;

    bool bFound = false;
    SCSIZE nStart = 0;

    // Skip first entry if more than 1 row.
    // Entries at the end are not skipped, GetFirstVisibleAttr may be larger than GetLastVisibleAttr.

    SCSIZE nVisStart = 1;
    while ( nVisStart < mvData.size() && mvData[nVisStart].pPattern->IsVisibleEqual(*mvData[nVisStart-1].pPattern) )
        ++nVisStart;
    if ( nVisStart >= mvData.size() || mvData[nVisStart-1].nEndRow > 0 )   // more than 1 row?
        nStart = nVisStart;

    while ( nStart < mvData.size() && !bFound )
    {
        if ( mvData[nStart].pPattern->IsVisible() )
        {
            rFirstRow = nStart ? ( mvData[nStart-1].nEndRow + 1 ) : 0;
            bFound = true;
        }
        else
            ++nStart;
    }

    return bFound;
}

// size (rows) of a range of attributes after cell content where the search is stopped
// (more than a default page size, 2*42 because it's as good as any number)

const SCROW SC_VISATTR_STOP = 84;

bool ScAttrArray::GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData ) const
{
    if ( mvData.empty() )
    {
        rLastRow = nLastData;
        return false;
    }

    //  #i30830# changed behavior:
    //  ignore all attributes starting with the first run of SC_VISATTR_STOP equal rows
    //  below the last content cell

    if ( nLastData == MAXROW )
    {
        rLastRow = MAXROW;      // can't look for attributes below MAXROW
        return true;
    }

    // Quick check: last data row in or immediately preceding a run that is the
    // last attribution down to the end, e.g. default style or column style.
    SCSIZE nPos = mvData.size() - 1;
    SCROW nStartRow = (nPos ? mvData[nPos-1].nEndRow + 1 : 0);
    if (nStartRow <= nLastData + 1)
    {
        // Ignore here a few rows if data happens to end within
        // SC_VISATTR_STOP rows before MAXROW.
        rLastRow = nLastData;
        return false;
    }

    // Find a run below last data row.
    bool bFound = false;
    Search( nLastData, nPos );
    while ( nPos < mvData.size() )
    {
        // find range of visually equal formats
        SCSIZE nEndPos = nPos;
        while ( nEndPos < mvData.size()-1 &&
                mvData[nEndPos].pPattern->IsVisibleEqual( *mvData[nEndPos+1].pPattern))
            ++nEndPos;
        SCROW nAttrStartRow = ( nPos > 0 ) ? ( mvData[nPos-1].nEndRow + 1 ) : 0;
        if ( nAttrStartRow <= nLastData )
            nAttrStartRow = nLastData + 1;
        SCROW nAttrSize = mvData[nEndPos].nEndRow + 1 - nAttrStartRow;
        if ( nAttrSize >= SC_VISATTR_STOP )
            break;  // while, ignore this range and below
        else if ( mvData[nEndPos].pPattern->IsVisible() )
        {
            rLastRow = mvData[nEndPos].nEndRow;
            bFound = true;
        }
        nPos = nEndPos + 1;
    }

    return bFound;
}

bool ScAttrArray::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( mvData.empty() )
        return pDocument->GetDefPattern()->IsVisible();

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = nStartRow;
    bool bFound = false;
    while ( nIndex < mvData.size() && nThisStart <= nEndRow && !bFound )
    {
        if ( mvData[nIndex].pPattern->IsVisible() )
            bFound = true;

        nThisStart = mvData[nIndex].nEndRow + 1;
        ++nIndex;
    }

    return bFound;
}

bool ScAttrArray::IsVisibleEqual( const ScAttrArray& rOther,
                                    SCROW nStartRow, SCROW nEndRow ) const
{
    if ( mvData.empty() && rOther.mvData.empty() )
    {
        const ScPatternAttr* pDefPattern1 = pDocument->GetDefPattern();
        const ScPatternAttr* pDefPattern2 = rOther.pDocument->GetDefPattern();
        return ( pDefPattern1 == pDefPattern2 || pDefPattern1->IsVisibleEqual( *pDefPattern2 ) );
    }

    {
        const ScAttrArray* pNonDefault = nullptr;
        const ScPatternAttr* pDefPattern = nullptr;
        bool bDefNonDefCase = false;
        if ( mvData.empty() && !rOther.mvData.empty() )
        {
            pNonDefault = &rOther;
            pDefPattern = pDocument->GetDefPattern();
            bDefNonDefCase = true;
        }
        else if ( !mvData.empty() && rOther.mvData.empty() )
        {
            pNonDefault = this;
            pDefPattern = rOther.pDocument->GetDefPattern();
            bDefNonDefCase = true;
        }

        if ( bDefNonDefCase )
        {
            bool bEqual = true;
            SCSIZE nPos = 0;
            if ( nStartRow > 0 )
                pNonDefault->Search( nStartRow, nPos );

            while ( nPos < pNonDefault->Count() && bEqual )
            {
                const ScPatternAttr* pNonDefPattern = pNonDefault->mvData[nPos].pPattern;
                bEqual = ( pNonDefPattern == pDefPattern ||
                           pNonDefPattern->IsVisibleEqual( *pDefPattern ) );

                if ( pNonDefault->mvData[nPos].nEndRow >= nEndRow ) break;
                ++nPos;
            }
            return bEqual;
        }
    }

    bool bEqual = true;
    SCSIZE nThisPos = 0;
    SCSIZE nOtherPos = 0;
    if ( nStartRow > 0 )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<mvData.size() && nOtherPos<rOther.Count() && bEqual )
    {
        SCROW nThisRow = mvData[nThisPos].nEndRow;
        SCROW nOtherRow = rOther.mvData[nOtherPos].nEndRow;
        const ScPatternAttr* pThisPattern = mvData[nThisPos].pPattern;
        const ScPatternAttr* pOtherPattern = rOther.mvData[nOtherPos].pPattern;
        bEqual = ( pThisPattern == pOtherPattern ||
                    pThisPattern->IsVisibleEqual(*pOtherPattern) );

        if ( nThisRow >= nOtherRow )
        {
            if ( nOtherRow >= nEndRow ) break;
            ++nOtherPos;
        }
        if ( nThisRow <= nOtherRow )
        {
            if ( nThisRow >= nEndRow ) break;
            ++nThisPos;
        }
    }

    return bEqual;
}

bool ScAttrArray::IsAllEqual( const ScAttrArray& rOther, SCROW nStartRow, SCROW nEndRow ) const
{
    // summarised with IsVisibleEqual
    if ( mvData.empty() && rOther.mvData.empty() )
    {
        const ScPatternAttr* pDefPattern1 = pDocument->GetDefPattern();
        const ScPatternAttr* pDefPattern2 = rOther.pDocument->GetDefPattern();
        return ( pDefPattern1 == pDefPattern2 );
    }

    {
        const ScAttrArray* pNonDefault = nullptr;
        const ScPatternAttr* pDefPattern = nullptr;
        bool bDefNonDefCase = false;
        if ( mvData.empty() && !rOther.mvData.empty() )
        {
            pNonDefault = &rOther;
            pDefPattern = pDocument->GetDefPattern();
            bDefNonDefCase = true;
        }
        else if ( !mvData.empty() && rOther.mvData.empty() )
        {
            pNonDefault = this;
            pDefPattern = rOther.pDocument->GetDefPattern();
            bDefNonDefCase = true;
        }

        if ( bDefNonDefCase )
        {
            bool bEqual = true;
            SCSIZE nPos = 0;
            if ( nStartRow > 0 )
                pNonDefault->Search( nStartRow, nPos );

            while ( nPos < pNonDefault->Count() && bEqual )
            {
                const ScPatternAttr* pNonDefPattern = pNonDefault->mvData[nPos].pPattern;
                bEqual = ( pNonDefPattern == pDefPattern );

                if ( pNonDefault->mvData[nPos].nEndRow >= nEndRow ) break;
                ++nPos;
            }
            return bEqual;
        }
    }

    bool bEqual = true;
    SCSIZE nThisPos = 0;
    SCSIZE nOtherPos = 0;
    if ( nStartRow > 0 )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<mvData.size() && nOtherPos<rOther.Count() && bEqual )
    {
        SCROW nThisRow = mvData[nThisPos].nEndRow;
        SCROW nOtherRow = rOther.mvData[nOtherPos].nEndRow;
        const ScPatternAttr* pThisPattern = mvData[nThisPos].pPattern;
        const ScPatternAttr* pOtherPattern = rOther.mvData[nOtherPos].pPattern;
        bEqual = ( pThisPattern == pOtherPattern );

        if ( nThisRow >= nOtherRow )
        {
            if ( nOtherRow >= nEndRow ) break;
            ++nOtherPos;
        }
        if ( nThisRow <= nOtherRow )
        {
            if ( nThisRow >= nEndRow ) break;
            ++nThisPos;
        }
    }

    return bEqual;
}

bool ScAttrArray::TestInsertCol( SCROW nStartRow, SCROW nEndRow) const
{
    // Horizontal aggregate are not allowed to be moved out; if whole summary,
    // here is not recognized

    bool bTest = true;
    if (!IsEmpty())
    {
        SCSIZE nIndex = 0;
        if ( nStartRow > 0 )
            Search( nStartRow, nIndex );

        for ( ; nIndex < mvData.size(); nIndex++ )
        {
            if ( mvData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG).IsHorOverlapped() )
            {
                bTest = false;  // may not be pushed out
                break;
            }
            if ( mvData[nIndex].nEndRow >= nEndRow ) // end of range
                break;
        }
    }
    return bTest;
}

bool ScAttrArray::TestInsertRow( SCSIZE nSize ) const
{
    // if 1st row pushed out is vertically overlapped, summary would be broken

    // MAXROW + 1 - nSize   = 1st row pushed out

    if ( mvData.empty() )
        return !pDocument->GetDefPattern()->
                       GetItem(ATTR_MERGE_FLAG).IsVerOverlapped();

    SCSIZE nFirstLost = mvData.size()-1;
    while ( nFirstLost && mvData[nFirstLost-1].nEndRow >= sal::static_int_cast<SCROW>(MAXROW + 1 - nSize) )
        --nFirstLost;

    return !mvData[nFirstLost].pPattern->
                GetItem(ATTR_MERGE_FLAG).IsVerOverlapped();
}

void ScAttrArray::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    SetDefaultIfNotInit();

    SCROW nSearch = nStartRow > 0 ? nStartRow - 1 : 0;  // expand predecessor
    SCSIZE nIndex;
    Search( nSearch, nIndex );

    // set ScMergeAttr may not be extended (so behind delete again)

    bool bDoMerge = mvData[nIndex].pPattern->GetItem(ATTR_MERGE).IsMerged();

    assert( !bDoMerge || nCol != -1 );

    SCSIZE nRemove = 0;
    SCSIZE i;
    for (i = nIndex; i < mvData.size()-1; i++)
    {
        SCROW nNew = mvData[i].nEndRow + nSize;
        if ( nNew >= MAXROW )    // at end?
        {
            nNew = MAXROW;
            if (!nRemove)
                nRemove = i+1;  // remove the following?
        }
        mvData[i].nEndRow = nNew;
    }

    // Remove entries at end ?

    if (nRemove && nRemove < mvData.size())
        DeleteRange( nRemove, mvData.size()-1 );

    if (bDoMerge)   // extensively repair (again) ScMergeAttr
    {
            //  ApplyAttr for areas

        const SfxPoolItem& rDef = pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
        for (SCSIZE nAdd=0; nAdd<nSize; nAdd++)
            pDocument->ApplyAttr( nCol, nStartRow+nAdd, nTab, rDef );

        // reply inserts in this area not summarized
    }

    // Don't duplicate the merge flags in the inserted row.
    // #i108488# ScMF::Scenario has to be allowed.
    RemoveFlags( nStartRow, nStartRow+nSize-1, ScMF::Hor | ScMF::Ver | ScMF::Auto | ScMF::Button );
}

void ScAttrArray::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    SetDefaultIfNotInit();
    bool bFirst=true;
    SCSIZE nStartIndex = 0;
    SCSIZE nEndIndex = 0;
    SCSIZE i;

    for ( i = 0; i < mvData.size()-1; i++)
        if (mvData[i].nEndRow >= nStartRow && mvData[i].nEndRow <= sal::static_int_cast<SCROW>(nStartRow+nSize-1))
        {
            if (bFirst)
            {
                nStartIndex = i;
                bFirst = false;
            }
            nEndIndex = i;
        }
    if (!bFirst)
    {
        SCROW nStart;
        if (nStartIndex==0)
            nStart = 0;
        else
            nStart = mvData[nStartIndex-1].nEndRow + 1;

        if (nStart < nStartRow)
        {
            mvData[nStartIndex].nEndRow = nStartRow - 1;
            ++nStartIndex;
        }
        if (nEndIndex >= nStartIndex)
        {
            DeleteRange( nStartIndex, nEndIndex );
            if (nStartIndex > 0)
                if ( mvData[nStartIndex-1].pPattern == mvData[nStartIndex].pPattern )
                    DeleteRange( nStartIndex-1, nStartIndex-1 );
        }
    }
    for (i = 0; i < mvData.size()-1; i++)
        if (mvData[i].nEndRow >= nStartRow)
            mvData[i].nEndRow -= nSize;

    // Below does not follow the pattern to detect pressure ranges;
    // instead, only remove merge flags.
    RemoveFlags( MAXROW-nSize+1, MAXROW, ScMF::Hor | ScMF::Ver | ScMF::Auto );
}

void ScAttrArray::DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex )
{
    SetDefaultIfNotInit();
    ScDocumentPool* pDocPool = pDocument->GetPool();
    for (SCSIZE i = nStartIndex; i <= nEndIndex; i++)
        pDocPool->Remove(*mvData[i].pPattern);

    mvData.erase(mvData.begin() + nStartIndex, mvData.begin() + nEndIndex + 1);
}

void ScAttrArray::DeleteArea(SCROW nStartRow, SCROW nEndRow)
{
    SetDefaultIfNotInit();
    if ( nCol != -1 )
        RemoveAreaMerge( nStartRow, nEndRow );  // remove from combined flags

    if ( !HasAttrib( nStartRow, nEndRow, HasAttrFlags::Overlapped | HasAttrFlags::AutoFilter) )
        SetPatternArea( nStartRow, nEndRow, pDocument->GetDefPattern() );
    else
        SetPatternAreaSafe( nStartRow, nEndRow, pDocument->GetDefPattern(), true ); // leave merge flags
}

void ScAttrArray::DeleteHardAttr(SCROW nStartRow, SCROW nEndRow)
{
    SetDefaultIfNotInit();
    const ScPatternAttr* pDefPattern = pDocument->GetDefPattern();

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = mvData[nIndex].pPattern;

        if ( pOldPattern->GetItemSet().Count() )  // hard attributes ?
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );

            ScPatternAttr aNewPattern(*pOldPattern);
            SfxItemSet& rSet = aNewPattern.GetItemSet();
            for (sal_uInt16 nId = ATTR_PATTERN_START; nId <= ATTR_PATTERN_END; nId++)
                if (nId != ATTR_MERGE && nId != ATTR_MERGE_FLAG)
                    rSet.ClearItem(nId);

            if ( aNewPattern == *pDefPattern )
                SetPatternArea( nThisRow, nAttrRow, pDefPattern );
            else
                SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );

            Search( nThisRow, nIndex );  // data changed
        }

        ++nIndex;
        nThisRow = mvData[nIndex-1].nEndRow+1;
    }
}

/**
 * Move within a document
 */
void ScAttrArray::MoveTo(SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray)
{
    SetDefaultIfNotInit();
    SCROW nStart = nStartRow;
    for (SCSIZE i = 0; i < mvData.size(); i++)
    {
        if ((mvData[i].nEndRow >= nStartRow) && (i == 0 || mvData[i-1].nEndRow < nEndRow))
        {
            // copy (bPutToPool=TRUE)
            rAttrArray.SetPatternArea( nStart, std::min( mvData[i].nEndRow, nEndRow ),
                                        mvData[i].pPattern, true );
        }
        nStart = std::max( nStart, mvData[i].nEndRow + 1 );
    }
    DeleteArea(nStartRow, nEndRow);
}

/**
 * Copy between documents (Clipboard)
 */
void ScAttrArray::CopyArea(
    SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray, ScMF nStripFlags) const
{
    nStartRow -= nDy;   // Source
    nEndRow -= nDy;

    SCROW nDestStart = std::max(static_cast<long>(static_cast<long>(nStartRow) + nDy), long(0));
    SCROW nDestEnd = std::min(static_cast<long>(static_cast<long>(nEndRow) + nDy), long(MAXROW));

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    bool bSamePool = (pSourceDocPool==pDestDocPool);

    if ( mvData.empty() )
    {
        const ScPatternAttr* pNewPattern = &pDestDocPool->GetDefaultItem( ATTR_PATTERN );
        rAttrArray.SetPatternArea(nDestStart, nDestEnd, pNewPattern);
        return;
    }

    for (SCSIZE i = 0; (i < mvData.size()) && (nDestStart <= nDestEnd); i++)
    {
        if (mvData[i].nEndRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = mvData[i].pPattern;
            const ScPatternAttr* pNewPattern;

            if (IsDefaultItem( pOldPattern ))
            {
                // default: nothing changed

                pNewPattern = &pDestDocPool->GetDefaultItem( ATTR_PATTERN );
            }
            else if ( nStripFlags != ScMF::NONE )
            {
                std::unique_ptr<ScPatternAttr> pTmpPattern(new ScPatternAttr( *pOldPattern ));
                ScMF nNewFlags = ScMF::NONE;
                if ( nStripFlags != ScMF::All )
                    nNewFlags = pTmpPattern->GetItem(ATTR_MERGE_FLAG).GetValue() & ~nStripFlags;

                if ( nNewFlags != ScMF::NONE )
                    pTmpPattern->GetItemSet().Put( ScMergeFlagAttr( nNewFlags ) );
                else
                    pTmpPattern->GetItemSet().ClearItem( ATTR_MERGE_FLAG );

                if (bSamePool)
                    pNewPattern = static_cast<const ScPatternAttr*>( &pDestDocPool->Put(*pTmpPattern) );
                else
                    pNewPattern = pTmpPattern->PutInPool( rAttrArray.pDocument, pDocument );
            }
            else
            {
                if (bSamePool)
                    pNewPattern = static_cast<const ScPatternAttr*>( &pDestDocPool->Put(*pOldPattern) );
                else
                    pNewPattern = pOldPattern->PutInPool( rAttrArray.pDocument, pDocument );
            }

            rAttrArray.SetPatternArea(nDestStart,
                            std::min(static_cast<SCROW>(mvData[i].nEndRow + nDy), nDestEnd), pNewPattern);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = std::max(static_cast<long>(nDestStart), static_cast<long>(mvData[i].nEndRow + nDy + 1));
    }
}

/**
 * Leave flags
 * summarized with CopyArea
 */
void ScAttrArray::CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray )
{
    nStartRow -= nDy;  // Source
    nEndRow -= nDy;

    SCROW nDestStart = std::max(static_cast<long>(static_cast<long>(nStartRow) + nDy), long(0));
    SCROW nDestEnd = std::min(static_cast<long>(static_cast<long>(nEndRow) + nDy), long(MAXROW));

    if ( !rAttrArray.HasAttrib( nDestStart, nDestEnd, HasAttrFlags::Overlapped ) )
    {
        CopyArea( nStartRow+nDy, nEndRow+nDy, nDy, rAttrArray );
        return;
    }

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    bool bSamePool = (pSourceDocPool==pDestDocPool);

    if ( mvData.empty() )
    {
        const ScPatternAttr* pNewPattern;
        if (bSamePool)
            pNewPattern = static_cast<const ScPatternAttr*>(
                             &pDestDocPool->Put(*pDocument->GetDefPattern()));
        else
            pNewPattern = pDocument->GetDefPattern()->PutInPool( rAttrArray.pDocument, pDocument );

        rAttrArray.SetPatternAreaSafe(nDestStart, nDestEnd, pNewPattern, false);
        return;
    }


    for (SCSIZE i = 0; (i < mvData.size()) && (nDestStart <= nDestEnd); i++)
    {
        if (mvData[i].nEndRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = mvData[i].pPattern;
            const ScPatternAttr* pNewPattern;

            if (bSamePool)
                pNewPattern = static_cast<const ScPatternAttr*>( &pDestDocPool->Put(*pOldPattern) );
            else
                pNewPattern = pOldPattern->PutInPool( rAttrArray.pDocument, pDocument );

            rAttrArray.SetPatternAreaSafe(nDestStart,
                            std::min(static_cast<SCROW>(mvData[i].nEndRow + nDy), nDestEnd), pNewPattern, false);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = std::max(static_cast<long>(nDestStart), static_cast<long>(mvData[i].nEndRow + nDy + 1));
    }
}

SCROW ScAttrArray::SearchStyle(
    SCROW nRow, const ScStyleSheet* pSearchStyle, bool bUp,
    const ScMarkArray* pMarkArray) const
{
    bool bFound = false;

    if (pMarkArray)
    {
        nRow = pMarkArray->GetNextMarked( nRow, bUp );
        if (!ValidRow(nRow))
            return nRow;
    }

    if ( mvData.empty() )
    {
        if (pDocument->GetDefPattern()->GetStyleSheet() == pSearchStyle)
            return nRow;

        nRow = bUp ? -1 : MAXROW + 1;
        return nRow;
    }

    SCSIZE nIndex;
    Search(nRow, nIndex);
    const ScPatternAttr* pPattern = mvData[nIndex].pPattern;

    while (nIndex < mvData.size() && !bFound)
    {
        if (pPattern->GetStyleSheet() == pSearchStyle)
        {
            if (pMarkArray)
            {
                nRow = pMarkArray->GetNextMarked( nRow, bUp );
                SCROW nStart = nIndex ? mvData[nIndex-1].nEndRow+1 : 0;
                if (nRow >= nStart && nRow <= mvData[nIndex].nEndRow)
                    bFound = true;
            }
            else
                bFound = true;
        }

        if (!bFound)
        {
            if (bUp)
            {
                if (nIndex==0)
                {
                    nIndex = mvData.size();
                    nRow = -1;
                }
                else
                {
                    --nIndex;
                    nRow = mvData[nIndex].nEndRow;
                    pPattern = mvData[nIndex].pPattern;
                }
            }
            else
            {
                nRow = mvData[nIndex].nEndRow+1;
                ++nIndex;
                if (nIndex<mvData.size())
                    pPattern = mvData[nIndex].pPattern;
            }
        }
    }

    OSL_ENSURE( bFound || !ValidRow(nRow), "Internal failure in ScAttrArray::SearchStyle" );

    return nRow;
}

bool ScAttrArray::SearchStyleRange(
    SCROW& rRow, SCROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
    const ScMarkArray* pMarkArray) const
{
    SCROW nStartRow = SearchStyle( rRow, pSearchStyle, bUp, pMarkArray );
    if (ValidRow(nStartRow))
    {
        if ( mvData.empty() )
        {
            rRow = nStartRow;
            if (bUp)
            {
                rEndRow = 0;
                if (pMarkArray)
                {
                    SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, true );
                    if (nMarkEnd>rEndRow)
                        rEndRow = nMarkEnd;
                }
            }
            else
            {
                rEndRow = MAXROW;
                if (pMarkArray)
                {
                    SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, false );
                    if (nMarkEnd<rEndRow)
                        rEndRow = nMarkEnd;
                }
            }

            return true;
        }

        SCSIZE nIndex;
        Search(nStartRow,nIndex);

        rRow = nStartRow;
        if (bUp)
        {
            if (nIndex>0)
                rEndRow = mvData[nIndex-1].nEndRow + 1;
            else
                rEndRow = 0;
            if (pMarkArray)
            {
                SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, true );
                if (nMarkEnd>rEndRow)
                    rEndRow = nMarkEnd;
            }
        }
        else
        {
            rEndRow = mvData[nIndex].nEndRow;
            if (pMarkArray)
            {
                SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, false );
                if (nMarkEnd<rEndRow)
                    rEndRow = nMarkEnd;
            }
        }

        return true;
    }
    else
        return false;
}

SCSIZE ScAttrArray::Count( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( mvData.empty() )
        return 1;

    SCSIZE  nIndex1, nIndex2;

    if( !Search( nStartRow, nIndex1 ) )
        return 0;

    if( !Search( nEndRow, nIndex2 ) )
        nIndex2 = mvData.size() - 1;

    return nIndex2 - nIndex1 + 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
