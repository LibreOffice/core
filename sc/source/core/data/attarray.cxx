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
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/justifyitem.hxx>
#include <osl/diagnose.h>
#include <poolcach.hxx>

#include <global.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <docsh.hxx>
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
#include <mtvelements.hxx>
#include <memory>

using ::editeng::SvxBorderLine;

ScAttrArray::ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument& rDoc, ScAttrArray* pDefaultColAttrArray ) :
    nCol( nNewCol ),
    nTab( nNewTab ),
    rDocument( rDoc )
{
    if ( nCol == -1 || !pDefaultColAttrArray || pDefaultColAttrArray->mvData.empty() )
        return;

    ScAddress aAdrStart( nCol, 0, nTab );
    ScAddress aAdrEnd( nCol, 0, nTab );
    mvData.resize( pDefaultColAttrArray->mvData.size() );
    for ( size_t nIdx = 0; nIdx < pDefaultColAttrArray->mvData.size(); ++nIdx )
    {
        mvData[nIdx].nEndRow = pDefaultColAttrArray->mvData[nIdx].nEndRow;
        mvData[nIdx].setScPatternAttr(pDefaultColAttrArray->mvData[nIdx].getScPatternAttr());
        bool bNumFormatChanged = false;
        if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
             mvData[nIdx].getScPatternAttr()->GetItemSet(),
             rDocument.getCellAttributeHelper().getDefaultCellAttribute().GetItemSet() ) )
        {
            aAdrStart.SetRow( nIdx ? mvData[nIdx-1].nEndRow+1 : 0 );
            aAdrEnd.SetRow( mvData[nIdx].nEndRow );
            rDocument.InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
        }
    }
}

ScAttrArray::~ScAttrArray()
{
#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
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
    }
    if ( nPos && mvData[nPos-1].nRow != rDocument.MaxRow() )
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
    mvData[0].nEndRow = rDocument.MaxRow();
    mvData[0].setScPatternAttr(&rDocument.getCellAttributeHelper().getDefaultCellAttribute()); // no put
}

void ScAttrArray::Reset(const CellAttributeHolder& rPattern)
{
    const ScPatternAttr* pPattern(rPattern.getScPatternAttr());
    if (nullptr == pPattern)
        return;

    ScAddress            aAdrStart( nCol, 0, nTab );
    ScAddress            aAdrEnd  ( nCol, 0, nTab );

    for (SCSIZE i=0; i<mvData.size(); i++)
    {
        // ensure that attributing changes text width of cell
        const ScPatternAttr* pOldPattern(mvData[i].getScPatternAttr());
        if ( nCol != -1 )
        {
            bool bNumFormatChanged;
            if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        pPattern->GetItemSet(), pOldPattern->GetItemSet() ) )
            {
                aAdrStart.SetRow( i ? mvData[i-1].nEndRow+1 : 0 );
                aAdrEnd  .SetRow( mvData[i].nEndRow );
                rDocument.InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
            }
        }
    }
    mvData.resize(0);

    rDocument.SetStreamValid(nTab, false);

    mvData.resize(1);
    mvData[0].nEndRow = rDocument.MaxRow();
    mvData[0].setScPatternAttr(pPattern);
}

bool ScAttrArray::Concat(SCSIZE nPos)
{
    bool bRet = false;
    if (nPos < mvData.size())
    {
        if (nPos > 0)
        {
            if (ScPatternAttr::areSame(mvData[nPos - 1].getScPatternAttr(), mvData[nPos].getScPatternAttr()))
            {
                mvData[nPos - 1].nEndRow = mvData[nPos].nEndRow;
                mvData.erase(mvData.begin() + nPos);
                nPos--;
                bRet = true;
            }
        }
        if (nPos + 1 < mvData.size())
        {
            if (ScPatternAttr::areSame(mvData[nPos + 1].getScPatternAttr(), mvData[nPos].getScPatternAttr()))
            {
                mvData[nPos].nEndRow = mvData[nPos + 1].nEndRow;
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
 *
 * @param oIndexHint, hint for the start of the search, useful when searching twice for successive values
 */

bool ScAttrArray::Search( SCROW nRow, SCSIZE& nIndex, std::optional<SCROW> oIndexHint ) const
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

    tools::Long nHi = static_cast<tools::Long>(mvData.size()) - 1;
    tools::Long i = 0;
    assert((!oIndexHint || *oIndexHint <= nHi) && "bad index hint");
    tools::Long nLo = oIndexHint ? *oIndexHint : 0;

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
        if ( !rDocument.ValidRow(nRow) )
            return nullptr;
        return &rDocument.getCellAttributeHelper().getDefaultCellAttribute();
    }
    SCSIZE i;
    if (Search( nRow, i ))
        return mvData[i].getScPatternAttr();
    else
        return nullptr;
}

const ScPatternAttr* ScAttrArray::GetPatternRange( SCROW& rStartRow,
        SCROW& rEndRow, SCROW nRow ) const
{
    if ( mvData.empty() )
    {
        if ( !rDocument.ValidRow( nRow ) )
            return nullptr;
        rStartRow = 0;
        rEndRow = rDocument.MaxRow();
        return &rDocument.getCellAttributeHelper().getDefaultCellAttribute();
    }
    SCSIZE nIndex;
    if ( Search( nRow, nIndex ) )
    {
        if ( nIndex > 0 )
            rStartRow = mvData[nIndex-1].nEndRow + 1;
        else
            rStartRow = 0;
        rEndRow = mvData[nIndex].nEndRow;
        return mvData[nIndex].getScPatternAttr();
    }
    return nullptr;
}

void ScAttrArray::AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!rDocument.ValidRow(nStartRow) || !rDocument.ValidRow(nEndRow))
        return;

    if(nEndRow < nStartRow)
        return;

    SCROW nTempStartRow = nStartRow;
    SCROW nTempEndRow = nEndRow;

    do
    {
        const ScPatternAttr* pPattern = GetPattern(nTempStartRow);

        // changed to create pNewPattern only if needed, else use already
        // existing pPattern. This shows by example how to avoid that special
        // handling of ScPatternAttr in SC and massive
        // incarnations/destructions of that Item (which contains an ItemSet)
        std::unique_ptr<ScPatternAttr> pNewPattern;
        if(pPattern)
        {
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            if (const ScCondFormatItem* pItem = pPattern->GetItemSet().GetItemIfSet( ATTR_CONDITIONAL ))
            {
                ScCondFormatIndexes const & rCondFormatData = pItem->GetCondFormatData();
                if (rCondFormatData.find(nIndex) == rCondFormatData.end())
                {
                    ScCondFormatIndexes aNewCondFormatData;
                    aNewCondFormatData.reserve(rCondFormatData.size()+1);
                    aNewCondFormatData = rCondFormatData;
                    aNewCondFormatData.insert(nIndex);
                    ScCondFormatItem aItem( std::move(aNewCondFormatData) );
                    pNewPattern.reset( new ScPatternAttr(*pPattern) );
                    pNewPattern->GetItemSet().Put( aItem );
                }
            }
            else
            {
                ScCondFormatItem aItem(nIndex);
                pNewPattern.reset( new ScPatternAttr(*pPattern) );
                pNewPattern->GetItemSet().Put( aItem );
            }
        }
        else
        {
            pNewPattern.reset( new ScPatternAttr( rDocument.getCellAttributeHelper() ) );
            ScCondFormatItem aItem(nIndex);
            pNewPattern->GetItemSet().Put( aItem );
            nTempEndRow = nEndRow;
        }

        if (pNewPattern)
            SetPatternArea( nTempStartRow, nTempEndRow, CellAttributeHolder(pNewPattern.release(), true) );
        else
            SetPatternArea( nTempStartRow, nTempEndRow, CellAttributeHolder(pPattern) );

        nTempStartRow = nTempEndRow + 1;
    }
    while(nTempEndRow < nEndRow);

}

void ScAttrArray::RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!rDocument.ValidRow(nStartRow) || !rDocument.ValidRow(nEndRow))
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
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            if (const ScCondFormatItem* pItem = pPattern->GetItemSet().GetItemIfSet( ATTR_CONDITIONAL ))
            {
                if (nIndex == 0)
                {
                    ScCondFormatItem aItem;
                    ScPatternAttr* pTemp(new ScPatternAttr(*pPattern));
                    pTemp->GetItemSet().Put( aItem );
                    SetPatternArea( nTempStartRow, nTempEndRow, CellAttributeHolder(pTemp, true) );
                }
                else
                {
                    ScCondFormatIndexes const & rCondFormatData = pItem->GetCondFormatData();
                    auto itr = rCondFormatData.find(nIndex);
                    if(itr != rCondFormatData.end())
                    {
                        ScCondFormatIndexes aNewCondFormatData(rCondFormatData);
                        aNewCondFormatData.erase_at(std::distance(rCondFormatData.begin(), itr));
                        ScCondFormatItem aItem( std::move(aNewCondFormatData) );
                        ScPatternAttr* pTemp(new ScPatternAttr(*pPattern));
                        pTemp->GetItemSet().Put( aItem );
                        SetPatternArea( nTempStartRow, nTempEndRow, CellAttributeHolder(pTemp, true) );
                    }
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

void ScAttrArray::RemoveCellCharAttribs( SCROW nStartRow, SCROW nEndRow,
                                       const ScPatternAttr* pPattern, ScEditDataArray* pDataArray )
{
    assert( nCol != -1 );
    // cache mdds position, this doesn't modify the mdds container, just EditTextObject's
    sc::ColumnBlockPosition blockPos;
    rDocument.InitColumnBlockPosition( blockPos, nTab, nCol );
    nEndRow = rDocument.GetLastDataRow(nTab, nCol, nCol, nEndRow);
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        ScAddress aPos(nCol, nRow, nTab);
        ScRefCellValue aCell(rDocument, aPos, blockPos);
        if (aCell.getType() != CELLTYPE_EDIT || !aCell.getEditText())
            continue;

        std::unique_ptr<EditTextObject> pOldData;
        if (pDataArray)
            pOldData = aCell.getEditText()->Clone();

        // Direct modification of cell content - something to watch out for if
        // we decide to share edit text instances in the future.
        ScEditUtil::RemoveCharAttribs(const_cast<EditTextObject&>(*aCell.getEditText()), *pPattern);

        if (pDataArray)
        {
            std::unique_ptr<EditTextObject> pNewData = aCell.getEditText()->Clone();
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
            mvData[0].nEndRow = rDocument.MaxRow();
            mvData[0].setScPatternAttr(&rDocument.getCellAttributeHelper().getDefaultCellAttribute()); // no put
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

const ScPatternAttr* ScAttrArray::SetPatternAreaImpl(
    SCROW nStartRow, SCROW nEndRow, const CellAttributeHolder& rPattern, ScEditDataArray* pDataArray)
{
    const ScPatternAttr* pPattern(rPattern.getScPatternAttr());
    if (nullptr == pPattern)
        return nullptr;

    if (rDocument.ValidRow(nStartRow) && rDocument.ValidRow(nEndRow))
    {
        if ((nStartRow == 0) && (nEndRow == rDocument.MaxRow()))
            Reset(rPattern);
        else
        {
            SCSIZE nNeeded = mvData.size() + 2;
            SetDefaultIfNotInit( nNeeded );

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
            bool bIsLoading = !rDocument.GetDocumentShell() || rDocument.GetDocumentShell()->IsLoading();
            while ( ns <= nEndRow )
            {
                if ( nCol != -1 && !bIsLoading )
                {
                    const SfxItemSet& rNewSet = pPattern->GetItemSet();
                    const SfxItemSet& rOldSet = mvData[nx].getScPatternAttr()->GetItemSet();
                    bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( std::max(nStartRow,ns) );
                        aAdrEnd  .SetRow( std::min(nEndRow,mvData[nx].nEndRow) );
                        rDocument.InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
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
                nInsert = rDocument.MaxRow() + 1;
                if ( !ScPatternAttr::areSame(mvData[ni].getScPatternAttr(), pPattern ) )
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
                if ( ni > 0 && ScPatternAttr::areSame(mvData[ni-1].getScPatternAttr(), pPattern) )
                {   // combine
                    mvData[ni-1].nEndRow = nEndRow;
                    nInsert = rDocument.MaxRow() + 1;
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
                if ( nj < mvData.size() && ScPatternAttr::areSame(mvData[nj].getScPatternAttr(), pPattern ) )
                {   // combine
                    if ( ni > 0 )
                    {
                        if ( ScPatternAttr::areSame(mvData[ni-1].getScPatternAttr(), pPattern ) )
                        {   // adjacent entries
                            mvData[ni-1].nEndRow = mvData[nj].nEndRow;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            mvData[ni-1].nEndRow = nStartRow - 1;   // shrink
                    }
                    nInsert = rDocument.MaxRow() + 1;
                    bCombined = true;
                }
                else if ( ni > 0 && ni == nInsert )
                    mvData[ni-1].nEndRow = nStartRow - 1;   // shrink
            }
            if ( ni < nj )
            {   // remove middle entries
                if ( !bCombined )
                {   // replace one entry
                    mvData[ni].nEndRow = nEndRow;
                    mvData[ni].setScPatternAttr(pPattern);
                    ni++;
                    nInsert = rDocument.MaxRow() + 1;
                }
                if ( ni < nj )
                {   // remove entries
                    mvData.erase( mvData.begin() + ni, mvData.begin() + nj);
                }
            }

            if ( nInsert < sal::static_int_cast<SCSIZE>(rDocument.MaxRow() + 1) )
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
                mvData[nInsert].setScPatternAttr(pPattern);

                // Remove character attributes from these cells if the pattern
                // is applied during normal session.
                if (pDataArray && nCol != -1)
                    RemoveCellCharAttribs(nStartRow, nEndRow, pPattern, pDataArray);
            }

            rDocument.SetStreamValid(nTab, false);
        }
    }

#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
    return pPattern;
}

void ScAttrArray::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    if (!(rDocument.ValidRow(nStartRow) && rDocument.ValidRow(nEndRow)))
        return;

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
        const ScPatternAttr* pOldPattern = mvData[nPos].getScPatternAttr();
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
            SetPatternArea( nY1, nY2, CellAttributeHolder(pNewPattern.release(), true) );
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
                    rDocument.InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                }
            }

            mvData[nPos].setScPatternAttr(pNewPattern.release(), true);
            if (Concat(nPos))
                Search(nStart, nPos);
            else
                nPos++;
        }
    }
    while ((nStart <= nEndRow) && (nPos < mvData.size()));

    rDocument.SetStreamValid(nTab, false);

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

    if (!(rDocument.ValidRow(nStartRow) && rDocument.ValidRow(nEndRow)))
        return;

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
        const ScPatternAttr*    pOldPattern = mvData[nPos].getScPatternAttr();
        const SfxItemSet&       rOldSet = pOldPattern->GetItemSet();
        const SvxBoxItem*       pBoxItem = rOldSet.GetItemIfSet( ATTR_BORDER );
        const SvxLineItem*      pTLBRItem = rOldSet.GetItemIfSet( ATTR_BORDER_TLBR );
        const SvxLineItem*      pBLTRItem = rOldSet.GetItemIfSet( ATTR_BORDER_BLTR );

        if ( pBoxItem || pTLBRItem || pBLTRItem )
        {
            std::unique_ptr<ScPatternAttr> pNewPattern(new ScPatternAttr(*pOldPattern));
            SfxItemSet&     rNewSet = pNewPattern->GetItemSet();
            SCROW           nY1 = nStart;
            SCROW           nY2 = mvData[nPos].nEndRow;

            std::unique_ptr<SvxBoxItem>  pNewBoxItem( pBoxItem ? pBoxItem->Clone() : nullptr);
            std::unique_ptr<SvxLineItem> pNewTLBRItem( pTLBRItem ? pTLBRItem->Clone() : nullptr);
            std::unique_ptr<SvxLineItem> pNewBLTRItem(pBLTRItem ? pBLTRItem->Clone() : nullptr);

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
            if( pNewBoxItem )   rNewSet.Put( std::move(pNewBoxItem) );
            if( pNewTLBRItem )  rNewSet.Put( std::move(pNewTLBRItem) );
            if( pNewBLTRItem )  rNewSet.Put( std::move(pNewBLTRItem) );

            nStart = mvData[nPos].nEndRow + 1;

            if ( nY1 < nStartRow || nY2 > nEndRow )
            {
                if (nY1 < nStartRow) nY1=nStartRow;
                if (nY2 > nEndRow) nY2=nEndRow;
                SetPatternArea( nY1, nY2, CellAttributeHolder(pNewPattern.release(), true) );
                Search( nStart, nPos );
            }
            else
            {
                // remove from pool ?
                mvData[nPos].setScPatternAttr(pNewPattern.release(), true);

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

void ScAttrArray::ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, ScItemPoolCache& rCache, ScEditDataArray* pDataArray, bool* const pIsChanged )
{
#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif

    if (!(rDocument.ValidRow(nStartRow) && rDocument.ValidRow(nEndRow)))
        return;

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
        const CellAttributeHolder& rOldPattern(mvData[nPos].getCellAttributeHolder());
        const CellAttributeHolder& rNewPattern(rCache.ApplyTo( rOldPattern ));

        if (!CellAttributeHolder::areSame(&rNewPattern, &rOldPattern))
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
                SetPatternArea( nY1, nY2, rNewPattern, pDataArray );
                Search( nStart, nPos );
            }
            else
            {
                if ( nCol != -1 )
                {
                    // ensure attributing changes text-width of cell

                    const SfxItemSet& rNewSet = rNewPattern.getScPatternAttr()->GetItemSet();
                    const SfxItemSet& rOldSet = rOldPattern.getScPatternAttr()->GetItemSet();

                    bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( nPos ? mvData[nPos-1].nEndRow+1 : 0 );
                        aAdrEnd  .SetRow( mvData[nPos].nEndRow );
                        rDocument.InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                    }
                }

                mvData[nPos].setCellAttributeHolder(rNewPattern);
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

    rDocument.SetStreamValid(nTab, false);

#if DEBUG_SC_TESTATTRARRAY
    TestData();
#endif
}

void ScAttrArray::SetAttrEntries(std::vector<ScAttrEntry> && vNewData)
{
    mvData = std::move(vNewData);

#ifdef DBG_UTIL
    SCROW lastEndRow = -1;
    for(const auto& entry : mvData)
    {   // Verify that the data is not corrupted.
        assert(entry.nEndRow > lastEndRow);
        lastEndRow = entry.nEndRow;
    }
#endif
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
                if ( *pNewItem != rMergeSet.GetPool()->GetUserOrPoolDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        else if ( eOldState == SfxItemState::SET ) // Item set
        {
            SfxItemState eNewState = rSource.GetItemState( nId, true, &pNewItem );
            if ( eNewState == SfxItemState::SET )
            {
                if ( !SfxPoolItem::areSame(pNewItem, pOldItem) ) // Both pulled
                    rMergeSet.InvalidateItem( nId );
            }
            else // Default
            {
                if ( *pOldItem != rSource.GetPool()->GetUserOrPoolDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        // Dontcare remains Dontcare
    }
}

void ScAttrArray::MergePatternArea( SCROW nStartRow, SCROW nEndRow,
                                    ScMergePatternState& rState, bool bDeep ) const
{
    if (!(rDocument.ValidRow(nStartRow) && rDocument.ValidRow(nEndRow)))
        return;

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
        const ScPatternAttr* pPattern(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        if ( !mvData.empty() )
            pPattern = mvData[nPos].getScPatternAttr();

        if ( !ScPatternAttr::areSame(pPattern, rState.aOld1.getScPatternAttr())
            && !ScPatternAttr::areSame(pPattern, rState.aOld2.getScPatternAttr()) )
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
                rState.pItemSet.emplace( *rThisSet.GetPool(), rThisSet.GetRanges() );
                rState.pItemSet->Set( rThisSet, bDeep );
                rState.mnPatternId = pPattern->GetPAKey();
            }

            rState.aOld2 = rState.aOld1;
            rState.aOld1 = pPattern;
        }

        if ( !mvData.empty() )
            nStart = mvData[nPos].nEndRow + 1;
        else
            nStart = rDocument.MaxRow() + 1;
        ++nPos;
    }
    while (nStart <= nEndRow);
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

    const SvxBoxItem* pCellFrame = &pPattern->GetItem( ATTR_BORDER );
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
            pPattern = mvData[i].getScPatternAttr();
            lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false,
                            nEndRow - std::min( mvData[i].nEndRow, static_cast<SCROW>(nEndRow-1) ) );
            // nDistBottom here always > 0
        }

        pPattern = GetPattern( nEndRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false, 0 );
    }
    else
    {
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, &rDocument.getCellAttributeHelper().getDefaultCellAttribute(), bLeft, nDistRight, true, 0 );
    }
}

// apply border

// ApplyFrame - on an entry into the array

bool ScAttrArray::ApplyFrame( const SvxBoxItem&     rBoxItem,
                              const SvxBoxInfoItem* pBoxInfoItem,
                              SCROW nStartRow, SCROW nEndRow,
                              bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom )
{
    OSL_ENSURE( pBoxInfoItem, "Missing line attributes!" );

    const ScPatternAttr* pPattern = GetPattern( nStartRow );
    const SvxBoxItem* pOldFrame = &pPattern->GetItem( ATTR_BORDER );

    // right/bottom border set when connected together
    const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
    if ( rMerge.GetColMerge() == nDistRight + 1 )
        nDistRight = 0;
    if ( rMerge.GetRowMerge() == nDistBottom + 1 )
        nDistBottom = 0;

    SvxBoxItem aNewFrame( *pOldFrame );
    bool bRTL=rDocument.IsLayoutRTL(nTab);
    // fdo#37464 check if the sheet are RTL then replace right <=> left
    if (bRTL)
    {
        if( bLeft && nDistRight==0)
        {
            if ( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) )
                aNewFrame.SetLine( rBoxItem.GetLeft(), SvxBoxItemLine::RIGHT );
            if ( pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) )
                aNewFrame.SetLine( rBoxItem.GetRight(), SvxBoxItemLine::LEFT );
        }
        else
        {
            if ( (nDistRight==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
                aNewFrame.SetLine( (nDistRight==0) ? rBoxItem.GetLeft() : pBoxInfoItem->GetVert(),
                    SvxBoxItemLine::RIGHT );
            if ( bLeft ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
                aNewFrame.SetLine( bLeft ? rBoxItem.GetRight() : pBoxInfoItem->GetVert(),
                    SvxBoxItemLine::LEFT );
        }
    }
    else
    {
        if ( bLeft ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::LEFT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            aNewFrame.SetLine( bLeft ? rBoxItem.GetLeft() : pBoxInfoItem->GetVert(),
                SvxBoxItemLine::LEFT );
        if ( (nDistRight==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::RIGHT) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::VERT) )
            aNewFrame.SetLine( (nDistRight==0) ? rBoxItem.GetRight() : pBoxInfoItem->GetVert(),
                SvxBoxItemLine::RIGHT );
    }
    if ( bTop ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::TOP) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
        aNewFrame.SetLine( bTop ? rBoxItem.GetTop() : pBoxInfoItem->GetHori(),
            SvxBoxItemLine::TOP );
    if ( (nDistBottom==0) ? pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::BOTTOM) : pBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::HORI) )
        aNewFrame.SetLine( (nDistBottom==0) ? rBoxItem.GetBottom() : pBoxInfoItem->GetHori(),
            SvxBoxItemLine::BOTTOM );

    if (aNewFrame == *pOldFrame)
    {
        // nothing to do
        return false;
    }
    else
    {
        ScItemPoolCache aCache( rDocument.getCellAttributeHelper(), aNewFrame );
        ApplyCacheArea( nStartRow, nEndRow, aCache );

        return true;
    }
}

void ScAttrArray::ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                                  SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight)
{
    SetDefaultIfNotInit();
    if (nStartRow == nEndRow)
        ApplyFrame(rLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, true, 0);
    else
    {
        ApplyFrame(rLineOuter, pLineInner, nStartRow, nStartRow, bLeft, nDistRight,
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
                bool bChanged = ApplyFrame(rLineOuter, pLineInner, nTmpStart, nTmpEnd,
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

        ApplyFrame(rLineOuter, pLineInner, nEndRow, nEndRow, bLeft, nDistRight, false, 0);
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
        if ( !pPattern->GetItem( ATTR_CONDITIONAL ).GetCondFormatData().empty())
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
        if ( bContainsCondFormat && nCol != -1 ) // rDocument.GetCondResult() is valid only for real columns.
        {
            SCROW nRowStartCond = std::max<SCROW>( nRow1, i ? mvData[i-1].nEndRow + 1: 0 );
            SCROW nRowEndCond = std::min<SCROW>( nRow2, mvData[i].nEndRow );
            bool bFoundCond = false;
            for(SCROW nRowCond = nRowStartCond; nRowCond <= nRowEndCond && !bFoundCond; ++nRowCond)
            {
                const SfxItemSet* pSet = rDocument.GetCondResult( nCol, nRowCond, nTab );

                const ScProtectionAttr* pCondProtect;
                if( pSet && (pCondProtect = pSet->GetItemIfSet( ATTR_PROTECTION )) )
                {
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
        const ScRotateValueItem* pRotate = &pPattern->GetItem( ATTR_ROTATE_VALUE );
        // 90 or 270 degrees is former SvxOrientationItem - only look for other values
        // (see ScPatternAttr::GetCellOrientation)
        Degree100 nAngle = pRotate->GetValue();
        if ( nAngle && nAngle != 9000_deg100 && nAngle != 27000_deg100 )
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
        return HasAttrib_Impl(&rDocument.getCellAttributeHelper().getDefaultCellAttribute(), nMask, 0, rDocument.MaxRow(), 0);
    }

    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nRow1, nStartIndex );
    if (nRow1 != nRow2)
        Search( nRow2, nEndIndex, /*hint*/nStartIndex );
    else
        nEndIndex = nStartIndex;
    bool bFound = false;

    for (SCSIZE i=nStartIndex; i<=nEndIndex && !bFound; i++)
    {
        const ScPatternAttr* pPattern = mvData[i].getScPatternAttr();
        bFound = HasAttrib_Impl(pPattern, nMask, nRow1, nRow2, i);
    }

    return bFound;
}

bool ScAttrArray::HasAttrib( SCROW nRow, HasAttrFlags nMask, SCROW* nStartRow, SCROW* nEndRow ) const
{
    if (mvData.empty())
    {
        if( nStartRow )
            *nStartRow = 0;
        if( nEndRow )
            *nEndRow = rDocument.MaxRow();
        return HasAttrib_Impl(&rDocument.getCellAttributeHelper().getDefaultCellAttribute(), nMask, 0, rDocument.MaxRow(), 0);
    }

    SCSIZE nIndex;
    Search( nRow, nIndex );
    if( nStartRow )
        *nStartRow = nIndex > 0 ? mvData[nIndex-1].nEndRow+1 : 0;
    if( nEndRow )
        *nEndRow = mvData[nIndex].nEndRow;
    const ScPatternAttr* pPattern = mvData[nIndex].getScPatternAttr();
    return HasAttrib_Impl(pPattern, nMask, nRow, nRow, nIndex);
}

bool ScAttrArray::IsMerged( SCROW nRow ) const
{
    if ( !mvData.empty() )
    {
        SCSIZE nIndex;
        Search(nRow, nIndex);
        const ScMergeAttr& rItem = mvData[nIndex].getScPatternAttr()->GetItem(ATTR_MERGE);

        return rItem.IsMerged();
    }

    return rDocument.getCellAttributeHelper().getDefaultCellAttribute().GetItem(ATTR_MERGE).IsMerged();
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
        pPattern = mvData[i].getScPatternAttr();
        pItem = &pPattern->GetItem( ATTR_MERGE );
        SCCOL  nCountX = pItem->GetColMerge();
        SCROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            SCROW nThisRow = (i>0) ? mvData[i-1].nEndRow+1 : 0;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisRow + nCountY - 1;
            if (nMergeEndCol > rPaintCol && nMergeEndCol <= rDocument.MaxCol())
                rPaintCol = nMergeEndCol;
            if (nMergeEndRow > rPaintRow && nMergeEndRow <= rDocument.MaxRow())
                rPaintRow = nMergeEndRow;
            bFound = true;

            if (bRefresh)
            {
                if ( nMergeEndCol > nThisCol )
                    rDocument.ApplyFlagsTab( nThisCol+1, nThisRow, nMergeEndCol, mvData[i].nEndRow,
                                nTab, ScMF::Hor );
                if ( nMergeEndRow > nThisRow )
                    rDocument.ApplyFlagsTab( nThisCol, nThisRow+1, nThisCol, nMergeEndRow,
                                nTab, ScMF::Ver );
                if ( nMergeEndCol > nThisCol && nMergeEndRow > nThisRow )
                    rDocument.ApplyFlagsTab( nThisCol+1, nThisRow+1, nMergeEndCol, nMergeEndRow,
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

        pPattern = mvData[nIndex].getScPatternAttr();
        pItem = &pPattern->GetItem( ATTR_MERGE );
        SCCOL  nCountX = pItem->GetColMerge();
        SCROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            const ScMergeAttr* pAttr = &rDocument.GetPool()->GetUserOrPoolDefaultItem( ATTR_MERGE );
            const ScMergeFlagAttr* pFlagAttr = &rDocument.GetPool()->GetUserOrPoolDefaultItem( ATTR_MERGE_FLAG );

            OSL_ENSURE( nCountY==1 || nThisStart==nThisEnd, "What's up?" );

            SCCOL nThisCol = nCol;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisEnd + nCountY - 1;

            // ApplyAttr for areas
            for (SCROW nThisRow = nThisStart; nThisRow <= nThisEnd; nThisRow++)
                rDocument.ApplyAttr( nThisCol, nThisRow, nTab, *pAttr );

            ScPatternAttr aNewPattern( rDocument.getCellAttributeHelper() );
            SfxItemSet*     pSet = &aNewPattern.GetItemSet();
            pSet->Put( *pFlagAttr );
            rDocument.ApplyPatternAreaTab( nThisCol, nThisStart, nMergeEndCol, nMergeEndRow,
                                                nTab, aNewPattern );

            Search( nThisEnd, nIndex );    // data changed
        }

        ++nIndex;
        if ( nIndex < mvData.size() )
            nThisStart = mvData[nIndex-1].nEndRow+1;
        else
            nThisStart = rDocument.MaxRow()+1;   // End
    }
}

void ScAttrArray::SetPatternAreaSafe(SCROW nStartRow, SCROW nEndRow, const CellAttributeHolder& rWantedPattern)
{
    SetDefaultIfNotInit();
    const ScMergeFlagAttr* pItem;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    while ( nThisRow <= nEndRow )
    {
        const CellAttributeHolder& rOldPattern(mvData[nIndex].getCellAttributeHolder());
        if (!CellAttributeHolder::areSame(&rOldPattern, &rWantedPattern)) // FIXME: else-branch?
        {
            if (nThisRow < nStartRow) nThisRow = nStartRow;
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            pItem = &rOldPattern.getScPatternAttr()->GetItem( ATTR_MERGE_FLAG );

            if (pItem->IsOverlapped() || pItem->HasAutoFilter())
            {
                //  default-constructing a ScPatternAttr for DeleteArea doesn't work
                //  because it would have no cell style information.
                //  Instead, the document's getCellAttributeHelper().getDefaultCellAttribute() is copied. Since it is passed as
                //  pWantedPattern, no special treatment of default is needed here anymore.
                ScPatternAttr* pNewPattern(new ScPatternAttr(*rWantedPattern.getScPatternAttr()));
                pNewPattern->GetItemSet().Put( *pItem );
                SetPatternArea( nThisRow, nAttrRow, CellAttributeHolder(pNewPattern, true) );
            }
            else
            {
                SetPatternArea(nThisRow, nAttrRow, rWantedPattern);
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
        pOldPattern = mvData[nIndex].getScPatternAttr();
        nOldValue = pOldPattern->GetItem( ATTR_MERGE_FLAG ).GetValue();
        if ( (nOldValue | nFlags) != nOldValue )
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            ScPatternAttr* pNewPattern(new ScPatternAttr(*pOldPattern));
            pNewPattern->GetItemSet().Put( ScMergeFlagAttr( nOldValue | nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, CellAttributeHolder(pNewPattern, true) );
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
        pOldPattern = mvData[nIndex].getScPatternAttr();
        nOldValue = pOldPattern->GetItem( ATTR_MERGE_FLAG ).GetValue();
        if ( (nOldValue & ~nFlags) != nOldValue )
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            ScPatternAttr* pNewPattern(new ScPatternAttr(*pOldPattern));
            pNewPattern->GetItemSet().Put( ScMergeFlagAttr( nOldValue & ~nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, CellAttributeHolder(pNewPattern, true) );
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
        const ScPatternAttr* pOldPattern = mvData[nIndex].getScPatternAttr();
        if ( pOldPattern->HasItemsSet( pWhich ) )
        {
            ScPatternAttr* pNewPattern(new ScPatternAttr(*pOldPattern));
            pNewPattern->ClearItems( pWhich );

            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );
            SetPatternArea( nThisRow, nAttrRow, CellAttributeHolder(pNewPattern, true) );
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
        const ScPatternAttr* pOldPattern = mvData[nIndex].getScPatternAttr();
        const SfxItemSet& rOldSet = pOldPattern->GetItemSet();
        const SvxHorJustifyItem* pItem;

        bool bNeedJust = !( pItem = rOldSet.GetItemIfSet( ATTR_HOR_JUSTIFY, false ) )
                           || (pItem->GetValue() != SvxCellHorJustify::Left &&
                               pItem->GetValue() != SvxCellHorJustify::Right );
        sal_uInt16 nOldValue = rOldSet.Get( ATTR_INDENT ).GetValue();
        sal_uInt16 nNewValue = nOldValue;
        // To keep Increment indent from running outside the cell1659
        tools::Long nColWidth = static_cast<tools::Long>(
            rDocument.GetColWidth(nCol == -1 ? rDocument.MaxCol() : nCol,nTab));
        if ( bIncrement )
        {
            if ( nNewValue < nColWidth-SC_INDENT_STEP )
            {
                nNewValue += SC_INDENT_STEP;
                if ( nNewValue > nColWidth-SC_INDENT_STEP )
                    nNewValue = nColWidth-SC_INDENT_STEP;
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
            ScPatternAttr* pNewPattern(new ScPatternAttr(*pOldPattern));
            pNewPattern->GetItemSet().Put( ScIndentItem( nNewValue ) );
            if ( bNeedJust )
                pNewPattern->GetItemSet().Put(
                                SvxHorJustifyItem( SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY ) );
            SetPatternArea( nThisStart, nAttrRow, CellAttributeHolder(pNewPattern, true) );

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
    tools::Long nRet = nRow;
    if (rDocument.ValidRow(nRow))
    {
        if ( mvData.empty() )
        {
            if ( bUp )
                return -1;
            else
                return rDocument.MaxRow()+1;
        }

        SCSIZE nIndex;
        Search(nRow, nIndex);
        while (mvData[nIndex].getScPatternAttr()->GetItem(ATTR_PROTECTION).GetProtection())
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
                    return rDocument.MaxRow()+1; // not found
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
        if (mvData[nPos].getScPatternAttr()->GetStyleSheet() == pStyleSheet)
        {
            rUsedRows.setTrue(nStart, nEnd);

            if (bReset)
            {
                ScPatternAttr* pNewPattern(new ScPatternAttr(*mvData[nPos].getScPatternAttr()));
                pNewPattern->SetStyleSheet( static_cast<ScStyleSheet*>(
                    rDocument.GetStyleSheetPool()->
                        Find( ScResId(STR_STYLENAME_STANDARD),
                              SfxStyleFamily::Para,
                              SfxStyleSearchBits::Auto | SfxStyleSearchBits::ScStandard ) ) );
                mvData[nPos].setScPatternAttr(pNewPattern, true);

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
        const ScStyleSheet* pStyle = rDocument.getCellAttributeHelper().getDefaultCellAttribute().GetStyleSheet();
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
        const ScStyleSheet* pStyle = mvData[nPos].getScPatternAttr()->GetStyleSheet();
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
        return mvData[0].getScPatternAttr()->isDefault();
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
    while ( nVisStart < mvData.size() && mvData[nVisStart].getScPatternAttr()->IsVisibleEqual(*mvData[nVisStart-1].getScPatternAttr()) )
        ++nVisStart;
    if ( nVisStart >= mvData.size() || mvData[nVisStart-1].nEndRow > 0 )   // more than 1 row?
        nStart = nVisStart;

    while ( nStart < mvData.size() && !bFound )
    {
        if ( mvData[nStart].getScPatternAttr()->IsVisible() )
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

bool ScAttrArray::GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData, bool bSkipEmpty ) const
{
    if ( mvData.empty() )
    {
        rLastRow = nLastData;
        return false;
    }

    //  #i30830# changed behavior:
    //  ignore all attributes starting with the first run of SC_VISATTR_STOP equal rows
    //  below the last content cell

    if ( nLastData == rDocument.MaxRow() )
    {
        rLastRow = rDocument.MaxRow();      // can't look for attributes below rDocument.MaxRow()
        return true;
    }

    // Quick check: last data row in or immediately preceding a run that is the
    // last attribution down to the end, e.g. default style or column style.
    SCSIZE nPos = mvData.size() - 1;
    SCROW nStartRow = (nPos ? mvData[nPos-1].nEndRow + 1 : 0);
    if (nStartRow <= nLastData + 1)
    {
        // Ignore here a few rows if data happens to end within
        // SC_VISATTR_STOP rows before rDocument.MaxRow().
        rLastRow = nLastData;
        return false;
    }
    // tdf#93315: If "Suppress output of empty pages" in Calc Options is not checked, show empty
    // (containing only empty data cells) page in the document
    bool bFound = false;
    if (bSkipEmpty)
    {
        Search( nLastData, nPos );
        while ( nPos < mvData.size() )
        {
            // find range of visually equal formats
            SCSIZE nEndPos = nPos;
            while ( nEndPos < mvData.size()-1 &&
                    mvData[nEndPos].getScPatternAttr()->IsVisibleEqual(*mvData[nEndPos+1].getScPatternAttr()))
                ++nEndPos;
            SCROW nAttrStartRow = ( nPos > 0 ) ? ( mvData[nPos-1].nEndRow + 1) : 0;
            if ( nAttrStartRow <= nLastData )
                nAttrStartRow = nLastData + 1;
            SCROW nAttrSize = mvData[nEndPos].nEndRow + 1 - nAttrStartRow;
            if ( nAttrSize >= SC_VISATTR_STOP )
                break;  // while, ignore this range and below
            else if ( mvData[nEndPos].getScPatternAttr()->IsVisible() )
            {
                rLastRow = mvData[nEndPos].nEndRow;
                bFound = true;
            }
            nPos = nEndPos + 1;
        }
    }
    else
    {
        if ((nPos > 0 && mvData[nPos-1].getScPatternAttr()->IsVisible())
                || (nPos > 1 && !mvData[nPos-1].getScPatternAttr()->IsVisibleEqual(*mvData[nPos-2].getScPatternAttr())))
        {
            rLastRow = mvData[nPos-1].nEndRow;
            return true;
        }
        rLastRow = nLastData;
    }
    return bFound;
}

bool ScAttrArray::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( mvData.empty() )
        return rDocument.getCellAttributeHelper().getDefaultCellAttribute().IsVisible();

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = nStartRow;
    bool bFound = false;
    while ( nIndex < mvData.size() && nThisStart <= nEndRow && !bFound )
    {
        if ( mvData[nIndex].getScPatternAttr()->IsVisible() )
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
        const ScPatternAttr* pDefPattern1(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        const ScPatternAttr* pDefPattern2(&rOther.rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        return ( ScPatternAttr::areSame(pDefPattern1, pDefPattern2) || pDefPattern1->IsVisibleEqual( *pDefPattern2 ) );
    }

    {
        const ScAttrArray* pNonDefault = nullptr;
        const ScPatternAttr* pDefPattern = nullptr;
        bool bDefNonDefCase = false;
        if ( mvData.empty() && !rOther.mvData.empty() )
        {
            pNonDefault = &rOther;
            pDefPattern = &rDocument.getCellAttributeHelper().getDefaultCellAttribute();
            bDefNonDefCase = true;
        }
        else if ( !mvData.empty() && rOther.mvData.empty() )
        {
            pNonDefault = this;
            pDefPattern = &rOther.rDocument.getCellAttributeHelper().getDefaultCellAttribute();
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
                const ScPatternAttr* pNonDefPattern = pNonDefault->mvData[nPos].getScPatternAttr();
                bEqual = ScPatternAttr::areSame(pNonDefPattern, pDefPattern) || pNonDefPattern->IsVisibleEqual( *pDefPattern );

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
        const ScPatternAttr* pThisPattern = mvData[nThisPos].getScPatternAttr();
        const ScPatternAttr* pOtherPattern = rOther.mvData[nOtherPos].getScPatternAttr();
        bEqual = ( ScPatternAttr::areSame(pThisPattern, pOtherPattern) || pThisPattern->IsVisibleEqual(*pOtherPattern) );

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
        const ScPatternAttr* pDefPattern1(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        const ScPatternAttr* pDefPattern2(&rOther.rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        return ScPatternAttr::areSame(pDefPattern1, pDefPattern2);
    }

    {
        const ScAttrArray* pNonDefault = nullptr;
        const ScPatternAttr* pDefPattern = nullptr;
        bool bDefNonDefCase = false;
        if ( mvData.empty() && !rOther.mvData.empty() )
        {
            pNonDefault = &rOther;
            pDefPattern = &rDocument.getCellAttributeHelper().getDefaultCellAttribute();
            bDefNonDefCase = true;
        }
        else if ( !mvData.empty() && rOther.mvData.empty() )
        {
            pNonDefault = this;
            pDefPattern = &rOther.rDocument.getCellAttributeHelper().getDefaultCellAttribute();
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
                const ScPatternAttr* pNonDefPattern = pNonDefault->mvData[nPos].getScPatternAttr();
                bEqual = ScPatternAttr::areSame( pNonDefPattern, pDefPattern );

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
        const ScPatternAttr* pThisPattern = mvData[nThisPos].getScPatternAttr();
        const ScPatternAttr* pOtherPattern = rOther.mvData[nOtherPos].getScPatternAttr();
        bEqual = ScPatternAttr::areSame( pThisPattern, pOtherPattern );

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
            if ( mvData[nIndex].getScPatternAttr()->GetItem(ATTR_MERGE_FLAG).IsHorOverlapped() )
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

    // rDocument.MaxRow() + 1 - nSize   = 1st row pushed out

    if ( mvData.empty() )
        return !rDocument.getCellAttributeHelper().getDefaultCellAttribute().GetItem(ATTR_MERGE_FLAG).IsVerOverlapped();

    SCSIZE nFirstLost = mvData.size()-1;
    while ( nFirstLost && mvData[nFirstLost-1].nEndRow >= sal::static_int_cast<SCROW>(rDocument.MaxRow() + 1 - nSize) )
        --nFirstLost;

    return !mvData[nFirstLost].getScPatternAttr()->GetItem(ATTR_MERGE_FLAG).IsVerOverlapped();
}

void ScAttrArray::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    SetDefaultIfNotInit();

    SCROW nSearch = nStartRow > 0 ? nStartRow - 1 : 0;  // expand predecessor
    SCSIZE nIndex;
    Search( nSearch, nIndex );

    // set ScMergeAttr may not be extended (so behind delete again)

    bool bDoMerge = mvData[nIndex].getScPatternAttr()->GetItem(ATTR_MERGE).IsMerged();

    assert( !bDoMerge || nCol != -1 );

    SCSIZE nRemove = 0;
    SCSIZE i;
    for (i = nIndex; i < mvData.size()-1; i++)
    {
        SCROW nNew = mvData[i].nEndRow + nSize;
        if ( nNew >= rDocument.MaxRow() )    // at end?
        {
            nNew = rDocument.MaxRow();
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

        const SfxPoolItem& rDef = rDocument.GetPool()->GetUserOrPoolDefaultItem( ATTR_MERGE );
        for (SCSIZE nAdd=0; nAdd<nSize; nAdd++)
            rDocument.ApplyAttr( nCol, nStartRow+nAdd, nTab, rDef );

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
                if ( ScPatternAttr::areSame( mvData[nStartIndex-1].getScPatternAttr(), mvData[nStartIndex].getScPatternAttr() ) )
                    DeleteRange( nStartIndex-1, nStartIndex-1 );
        }
    }
    for (i = 0; i < mvData.size()-1; i++)
        if (mvData[i].nEndRow >= nStartRow)
            mvData[i].nEndRow -= nSize;

    // Below does not follow the pattern to detect pressure ranges;
    // instead, only remove merge flags.
    RemoveFlags( rDocument.MaxRow()-nSize+1, rDocument.MaxRow(), ScMF::Hor | ScMF::Ver | ScMF::Auto );
}

void ScAttrArray::DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex )
{
    SetDefaultIfNotInit();
    mvData.erase(mvData.begin() + nStartIndex, mvData.begin() + nEndIndex + 1);
}

void ScAttrArray::DeleteArea(SCROW nStartRow, SCROW nEndRow)
{
    SetDefaultIfNotInit();
    if ( nCol != -1 )
        RemoveAreaMerge( nStartRow, nEndRow );  // remove from combined flags

    const CellAttributeHolder aDefHolder(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());

    if ( !HasAttrib( nStartRow, nEndRow, HasAttrFlags::Overlapped | HasAttrFlags::AutoFilter) )
        SetPatternArea( nStartRow, nEndRow, aDefHolder );
    else
        SetPatternAreaSafe( nStartRow, nEndRow, aDefHolder ); // leave merge flags
}

void ScAttrArray::DeleteHardAttr(SCROW nStartRow, SCROW nEndRow)
{
    SetDefaultIfNotInit();
    const CellAttributeHolder aDefHolder(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? mvData[nIndex-1].nEndRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = mvData[nIndex].getScPatternAttr();

        if ( pOldPattern->GetItemSet().Count() )  // hard attributes ?
        {
            nRow = mvData[nIndex].nEndRow;
            SCROW nAttrRow = std::min( nRow, nEndRow );

            ScPatternAttr* pNewPattern(new ScPatternAttr(*pOldPattern));
            SfxItemSet& rSet = pNewPattern->GetItemSet();
            for (sal_uInt16 nId = ATTR_PATTERN_START; nId <= ATTR_PATTERN_END; nId++)
                if (nId != ATTR_MERGE && nId != ATTR_MERGE_FLAG)
                    rSet.ClearItem(nId);

            if ( *pNewPattern == *aDefHolder.getScPatternAttr() )
            {
                delete pNewPattern;
                SetPatternArea( nThisRow, nAttrRow, aDefHolder );
            }
            else
            {
                SetPatternArea( nThisRow, nAttrRow, CellAttributeHolder(pNewPattern, true) );
            }

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
            rAttrArray.SetPatternArea( nStart, std::min( mvData[i].nEndRow, nEndRow ), mvData[i].getCellAttributeHolder() );
        }
        nStart = std::max( nStart, mvData[i].nEndRow + 1 );
    }
    DeleteArea(nStartRow, nEndRow);
}

/**
 * Copy between documents (Clipboard)
 */
void ScAttrArray::CopyArea(
    SCROW nStartRow, SCROW nEndRow, tools::Long nDy, ScAttrArray& rAttrArray, ScMF nStripFlags) const
{
    nStartRow -= nDy;   // Source
    nEndRow -= nDy;

    SCROW nDestStart = std::max(static_cast<tools::Long>(static_cast<tools::Long>(nStartRow) + nDy), tools::Long(0));
    SCROW nDestEnd = std::min(static_cast<tools::Long>(static_cast<tools::Long>(nEndRow) + nDy), tools::Long(rDocument.MaxRow()));
    const bool bSameCellAttributeHelper(&rDocument.getCellAttributeHelper() == &rAttrArray.rDocument.getCellAttributeHelper());

    if ( mvData.empty() )
    {
        const ScPatternAttr* pNewPattern = &rAttrArray.rDocument.getCellAttributeHelper().getDefaultCellAttribute();
        rAttrArray.SetPatternArea(nDestStart, nDestEnd, pNewPattern);
        return;
    }

    for (SCSIZE i = 0; (i < mvData.size()) && (nDestStart <= nDestEnd); i++)
    {
        if (mvData[i].nEndRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = mvData[i].getScPatternAttr();
            CellAttributeHolder aNewPattern;

            if (ScPatternAttr::areSame(&rDocument.getCellAttributeHelper().getDefaultCellAttribute(), pOldPattern ))
            {
                // default: nothing changed
                aNewPattern.setScPatternAttr(&rAttrArray.rDocument.getCellAttributeHelper().getDefaultCellAttribute());
            }
            else if ( nStripFlags != ScMF::NONE )
            {
                ScPatternAttr* pTmpPattern(new ScPatternAttr(*pOldPattern ));
                ScMF nNewFlags = ScMF::NONE;
                if ( nStripFlags != ScMF::All )
                    nNewFlags = pTmpPattern->GetItem(ATTR_MERGE_FLAG).GetValue() & ~nStripFlags;

                if ( nNewFlags != ScMF::NONE )
                    pTmpPattern->GetItemSet().Put( ScMergeFlagAttr( nNewFlags ) );
                else
                    pTmpPattern->GetItemSet().ClearItem( ATTR_MERGE_FLAG );

                if (bSameCellAttributeHelper)
                    aNewPattern.setScPatternAttr(pTmpPattern, true);
                else
                {
                    aNewPattern = pTmpPattern->MigrateToDocument( &rAttrArray.rDocument, &rDocument );
                    delete pTmpPattern;
                }
            }
            else
            {
                if (bSameCellAttributeHelper)
                    aNewPattern.setScPatternAttr(pOldPattern);
                else
                    aNewPattern = pOldPattern->MigrateToDocument( &rAttrArray.rDocument, &rDocument );
            }

            rAttrArray.SetPatternArea(nDestStart,
                std::min(static_cast<SCROW>(mvData[i].nEndRow + nDy), nDestEnd), aNewPattern.getScPatternAttr());
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = std::max(static_cast<tools::Long>(nDestStart), static_cast<tools::Long>(mvData[i].nEndRow + nDy + 1));
    }
}

/**
 * Leave flags
 * summarized with CopyArea
 */
void ScAttrArray::CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, tools::Long nDy, ScAttrArray& rAttrArray )
{
    nStartRow -= nDy;  // Source
    nEndRow -= nDy;

    SCROW nDestStart = std::max(static_cast<tools::Long>(static_cast<tools::Long>(nStartRow) + nDy), tools::Long(0));
    SCROW nDestEnd = std::min(static_cast<tools::Long>(static_cast<tools::Long>(nEndRow) + nDy), tools::Long(rDocument.MaxRow()));

    if ( !rAttrArray.HasAttrib( nDestStart, nDestEnd, HasAttrFlags::Overlapped ) )
    {
        CopyArea( nStartRow+nDy, nEndRow+nDy, nDy, rAttrArray );
        return;
    }

    const bool bSameCellAttributeHelper(&rDocument.getCellAttributeHelper() == &rAttrArray.rDocument.getCellAttributeHelper());

    if ( mvData.empty() )
    {
        CellAttributeHolder aNewPattern;
        if (bSameCellAttributeHelper)
            aNewPattern.setScPatternAttr(&rDocument.getCellAttributeHelper().getDefaultCellAttribute());
        else
            aNewPattern = rDocument.getCellAttributeHelper().getDefaultCellAttribute().MigrateToDocument( &rAttrArray.rDocument, &rDocument );

        rAttrArray.SetPatternAreaSafe(nDestStart, nDestEnd, aNewPattern);
        return;
    }


    for (SCSIZE i = 0; (i < mvData.size()) && (nDestStart <= nDestEnd); i++)
    {
        if (mvData[i].nEndRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = mvData[i].getScPatternAttr();
            CellAttributeHolder aNewPattern;

            if (bSameCellAttributeHelper)
                aNewPattern.setScPatternAttr(pOldPattern);
            else
                aNewPattern = pOldPattern->MigrateToDocument( &rAttrArray.rDocument, &rDocument );

            rAttrArray.SetPatternAreaSafe(nDestStart,
                std::min(static_cast<SCROW>(mvData[i].nEndRow + nDy), nDestEnd), aNewPattern);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = std::max(static_cast<tools::Long>(nDestStart), static_cast<tools::Long>(mvData[i].nEndRow + nDy + 1));
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
        if (!rDocument.ValidRow(nRow))
            return nRow;
    }

    if ( mvData.empty() )
    {
        if (rDocument.getCellAttributeHelper().getDefaultCellAttribute().GetStyleSheet() == pSearchStyle)
            return nRow;

        nRow = bUp ? -1 : rDocument.MaxRow() + 1;
        return nRow;
    }

    SCSIZE nIndex;
    Search(nRow, nIndex);
    const ScPatternAttr* pPattern = mvData[nIndex].getScPatternAttr();

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
                    pPattern = mvData[nIndex].getScPatternAttr();
                }
            }
            else
            {
                nRow = mvData[nIndex].nEndRow+1;
                ++nIndex;
                if (nIndex<mvData.size())
                    pPattern = mvData[nIndex].getScPatternAttr();
            }
        }
    }

    OSL_ENSURE( bFound || !rDocument.ValidRow(nRow), "Internal failure in ScAttrArray::SearchStyle" );

    return nRow;
}

bool ScAttrArray::SearchStyleRange(
    SCROW& rRow, SCROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
    const ScMarkArray* pMarkArray) const
{
    SCROW nStartRow = SearchStyle( rRow, pSearchStyle, bUp, pMarkArray );
    if (rDocument.ValidRow(nStartRow))
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
                rEndRow = rDocument.MaxRow();
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
