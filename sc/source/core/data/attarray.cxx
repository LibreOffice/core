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

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/poolcach.hxx>
#include <editeng/fontitem.hxx>
#include <unotools/fontcvt.hxx>

#include "attarray.hxx"
#include "global.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "markarr.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "segmenttree.hxx"
#include "cell.hxx"
#include <rtl/strbuf.hxx>

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------
using ::editeng::SvxBorderLine;

ScAttrArray::ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc ) :
    nCol( nNewCol ),
    nTab( nNewTab ),
    pDocument( pDoc )
{
    nCount = nLimit = 1;
    pData = new ScAttrEntry[1];
    if (pData)
    {
        pData[0].nRow = MAXROW;
        pData[0].pPattern = pDocument->GetDefPattern(); // no put
    }
}

//------------------------------------------------------------------------

ScAttrArray::~ScAttrArray()
{
#if OSL_DEBUG_LEVEL > 1
    TestData();
#endif

    if (pData)
    {
        ScDocumentPool* pDocPool = pDocument->GetPool();
        for (SCSIZE i=0; i<nCount; i++)
            pDocPool->Remove(*pData[i].pPattern);

        delete[] pData;
    }
}

//------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
void ScAttrArray::TestData() const
{

    sal_uInt16 nErr = 0;
    if (pData)
    {
        SCSIZE nPos;
        for (nPos=0; nPos<nCount; nPos++)
        {
            if (nPos > 0)
                if (pData[nPos].pPattern == pData[nPos-1].pPattern || pData[nPos].nRow <= pData[nPos-1].nRow)
                    ++nErr;
            if (pData[nPos].pPattern->Which() != ATTR_PATTERN)
                ++nErr;
        }
        if ( nPos && pData[nPos-1].nRow != MAXROW )
            ++nErr;
    }
    if (nErr)
    {
        rtl::OStringBuffer aMsg;
        aMsg.append(static_cast<sal_Int32>(nErr));
        aMsg.append(RTL_CONSTASCII_STRINGPARAM(
            " errors in attribute array, column "));
        aMsg.append(static_cast<sal_Int32>(nCol));
        OSL_FAIL(aMsg.getStr());
    }
}
#endif

//------------------------------------------------------------------------

void ScAttrArray::Reset( const ScPatternAttr* pPattern, bool bAlloc )
{
    if (pData)
    {
        ScDocumentPool*      pDocPool = pDocument->GetPool();
        const ScPatternAttr* pOldPattern;
        ScAddress            aAdrStart( nCol, 0, nTab );
        ScAddress            aAdrEnd  ( nCol, 0, nTab );

        for (SCSIZE i=0; i<nCount; i++)
        {
            // ensure that attributing changes text width of cell
            pOldPattern = pData[i].pPattern;
            bool bNumFormatChanged;
            if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                    pPattern->GetItemSet(), pOldPattern->GetItemSet() ) )
            {
                aAdrStart.SetRow( i ? pData[i-1].nRow+1 : 0 );
                aAdrEnd  .SetRow( pData[i].nRow );
                pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
            }
            pDocPool->Remove(*pOldPattern);
        }
        delete[] pData;

        if (pDocument->IsStreamValid(nTab))
            pDocument->SetStreamValid(nTab, false);

        if (bAlloc)
        {
            nCount = nLimit = 1;
            pData = new ScAttrEntry[1];
            if (pData)
            {
                ScPatternAttr* pNewPattern = (ScPatternAttr*) &pDocPool->Put(*pPattern);
                pData[0].nRow = MAXROW;
                pData[0].pPattern = pNewPattern;
            }
        }
        else
        {
            nCount = nLimit = 0;
            pData = NULL;        // should be immediately occupied again
        }
    }
}


bool ScAttrArray::Concat(SCSIZE nPos)
{
    bool bRet = false;
    if (pData && (nPos < nCount))
    {
        if (nPos > 0)
        {
            if (pData[nPos - 1].pPattern == pData[nPos].pPattern)
            {
                pData[nPos - 1].nRow = pData[nPos].nRow;
                pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                memmove(&pData[nPos], &pData[nPos + 1], (nCount - nPos - 1) * sizeof(ScAttrEntry));
                pData[nCount - 1].pPattern = NULL;
                pData[nCount - 1].nRow = 0;
                nCount--;
                nPos--;
                bRet = true;
            }
        }
        if (nPos + 1 < nCount)
        {
            if (pData[nPos + 1].pPattern == pData[nPos].pPattern)
            {
                pData[nPos].nRow = pData[nPos + 1].nRow;
                pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                memmove(&pData[nPos + 1], &pData[nPos + 2], (nCount - nPos - 2) * sizeof(ScAttrEntry));
                pData[nCount - 1].pPattern = NULL;
                pData[nCount - 1].nRow = 0;
                nCount--;
                bRet = true;
            }
        }
    }
    return bRet;
}

//------------------------------------------------------------------------

bool ScAttrArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    long    nHi         = static_cast<long>(nCount) - 1;
    long    i           = 0;
    bool    bFound      = (nCount == 1);
    if (pData)
    {
        long nLo = 0;
        long nStartRow = 0;
        long nEndRow = 0;
        while ( !bFound && nLo <= nHi )
        {
            i = (nLo + nHi) / 2;
            if (i > 0)
                nStartRow = (long) pData[i - 1].nRow;
            else
                nStartRow = -1;
            nEndRow = (long) pData[i].nRow;
            if (nEndRow < (long) nRow)
                nLo = ++i;
            else
                if (nStartRow >= (long) nRow)
                    nHi = --i;
                else
                    bFound = true;
        }
    }
    else
        bFound = false;

    if (bFound)
        nIndex=(SCSIZE)i;
    else
        nIndex=0;
    return bFound;
}


const ScPatternAttr* ScAttrArray::GetPattern( SCROW nRow ) const
{
    SCSIZE i;
    if (Search( nRow, i ))
        return pData[i].pPattern;
    else
        return NULL;
}


const ScPatternAttr* ScAttrArray::GetPatternRange( SCROW& rStartRow,
        SCROW& rEndRow, SCROW nRow ) const
{
    SCSIZE nIndex;
    if ( Search( nRow, nIndex ) )
    {
        if ( nIndex > 0 )
            rStartRow = pData[nIndex-1].nRow + 1;
        else
            rStartRow = 0;
        rEndRow = pData[nIndex].nRow;
        return pData[nIndex].pPattern;
    }
    return NULL;
}

void ScAttrArray::AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!VALIDROW(nStartRow) || !VALIDROW(nEndRow))
        return;

    if(nEndRow < nStartRow)
        return;

    SCROW nTempStartRow = nStartRow;
    SCROW nTempEndRow = nEndRow;

    do
    {
        const ScPatternAttr* pPattern = GetPattern(nTempStartRow);

        ScPatternAttr aPattern( pDocument->GetPool() );
        if(pPattern)
        {
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            const SfxPoolItem* pItem = NULL;
            pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, true, &pItem );
            std::vector< sal_uInt32 > aCondFormatData;
            if(pItem)
                aCondFormatData = static_cast<const ScCondFormatItem*>(pItem)->GetCondFormatData();
            aCondFormatData.push_back(nIndex);

            ScCondFormatItem aItem;
            aItem.SetCondFormatData( aCondFormatData );
            aPattern.GetItemSet().Put( aItem );
        }
        else
        {
            ScCondFormatItem aItem;
            aItem.AddCondFormatData(nIndex);
            aPattern.GetItemSet().Put( aItem );
            nTempEndRow = nEndRow;
        }

        SetPatternArea( nTempStartRow, nTempEndRow, &aPattern, true );
        nTempStartRow = nTempEndRow + 1;
    }
    while(nTempEndRow < nEndRow);

}

void ScAttrArray::RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex )
{
    if(!VALIDROW(nStartRow) || !VALIDROW(nEndRow))
        return;

    if(nEndRow < nStartRow)
        return;

    SCROW nTempStartRow = nStartRow;
    SCROW nTempEndRow = nEndRow;

    do
    {
        const ScPatternAttr* pPattern = GetPattern(nTempStartRow);

        ScPatternAttr aPattern( pDocument->GetPool() );
        if(pPattern)
        {
            SCROW nPatternStartRow;
            SCROW nPatternEndRow;
            GetPatternRange( nPatternStartRow, nPatternEndRow, nTempStartRow );

            nTempEndRow = std::min<SCROW>( nPatternEndRow, nEndRow );
            const SfxPoolItem* pItem = NULL;
            pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, true, &pItem );
            if(pItem)
            {
                std::vector< sal_uInt32 > aCondFormatData = static_cast<const ScCondFormatItem*>(pItem)->GetCondFormatData();
                std::vector<sal_uInt32>::iterator itr = std::find(aCondFormatData.begin(), aCondFormatData.end(), nIndex);
                if(itr != aCondFormatData.end())
                {
                    ScCondFormatItem aItem;
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

//------------------------------------------------------------------------

void ScAttrArray::SetPattern( SCROW nRow, const ScPatternAttr* pPattern, bool bPutToPool )
{
    SetPatternArea( nRow, nRow, pPattern, bPutToPool );
}

void ScAttrArray::RemoveCellCharAttribs( SCROW nStartRow, SCROW nEndRow,
                                       const ScPatternAttr* pPattern, ScEditDataArray* pDataArray )
{
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        ScBaseCell* pCell;
        pDocument->GetCell(nCol, nRow, nTab, pCell);
        if (pCell && pCell->GetCellType() == CELLTYPE_EDIT)
        {
            EditTextObject* pOldData = NULL;
            ScEditCell* pEditCell = static_cast<ScEditCell*>(pCell);
            if (pDataArray)
                pOldData = pEditCell->GetData()->Clone();
            pEditCell->RemoveCharAttribs(*pPattern);
            if (pDataArray)
            {
                EditTextObject* pNewData = pEditCell->GetData()->Clone();
                pDataArray->AddItem(nTab, nCol, nRow, pOldData, pNewData);
            }
        }
    }
}

void ScAttrArray::SetPatternArea(SCROW nStartRow, SCROW nEndRow, const ScPatternAttr *pPattern,
                                 bool bPutToPool, ScEditDataArray* pDataArray )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        if (bPutToPool)
            pPattern = (const ScPatternAttr*) &pDocument->GetPool()->Put(*pPattern);

        if ((nStartRow == 0) && (nEndRow == MAXROW))
            Reset(pPattern);
        else
        {
            SCSIZE nNeeded = nCount + 2;
            if ( nLimit < nNeeded )
            {
                nLimit += SC_ATTRARRAY_DELTA;
                if ( nLimit < nNeeded )
                    nLimit = nNeeded;
                ScAttrEntry* pNewData = new ScAttrEntry[nLimit];
                memcpy( pNewData, pData, nCount*sizeof(ScAttrEntry) );
                delete[] pData;
                pData = pNewData;
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
                    ns = pData[ni-1].nRow+1;
                }
            }

            // ensure that attributing changes text width of cell
            // otherwise, conditional formats need to be reset or deleted
            while ( ns <= nEndRow )
            {
                const SfxItemSet& rNewSet = pPattern->GetItemSet();
                const SfxItemSet& rOldSet = pData[nx].pPattern->GetItemSet();

                bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rNewSet, rOldSet ) )
                {
                    aAdrStart.SetRow( Max(nStartRow,ns) );
                    aAdrEnd  .SetRow( Min(nEndRow,pData[nx].nRow) );
                    pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                }
                ns = pData[nx].nRow + 1;
                nx++;
            }

            // continue modifying data array

            SCSIZE nInsert;     // insert position (MAXROWCOUNT := no insert)
            bool bCombined = false;
            bool bSplit = false;
            if ( nStartRow > 0 )
            {
                nInsert = MAXROWCOUNT;
                if ( pData[ni].pPattern != pPattern )
                {
                    if ( ni == 0 || (pData[ni-1].nRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( pData[ni].nRow > nEndRow )
                            bSplit = true;
                        ni++;
                        nInsert = ni;
                    }
                    else if ( ni > 0 && pData[ni-1].nRow == nStartRow - 1 )
                        nInsert = ni;
                }
                if ( ni > 0 && pData[ni-1].pPattern == pPattern )
                {   // combine
                    pData[ni-1].nRow = nEndRow;
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
            }
            else
                nInsert = 0;

            SCSIZE nj = ni;     // stop position of range to replace
            while ( nj < nCount && pData[nj].nRow <= nEndRow )
                nj++;
            if ( !bSplit )
            {
                if ( nj < nCount && pData[nj].pPattern == pPattern )
                {   // combine
                    if ( ni > 0 )
                    {
                        if ( pData[ni-1].pPattern == pPattern )
                        {   // adjacent entries
                            pData[ni-1].nRow = pData[nj].nRow;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            pData[ni-1].nRow = nStartRow - 1;   // shrink
                    }
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
                else if ( ni > 0 && ni == nInsert )
                    pData[ni-1].nRow = nStartRow - 1;   // shrink
            }
            ScDocumentPool* pDocPool = pDocument->GetPool();
            if ( bSplit )
            {   // duplicate splitted entry in pool
                pDocPool->Put( *pData[ni-1].pPattern );
            }
            if ( ni < nj )
            {   // remove middle entries
                for ( SCSIZE nk=ni; nk<nj; nk++)
                {   // remove entries from pool
                    pDocPool->Remove( *pData[nk].pPattern );
                }
                if ( !bCombined )
                {   // replace one entry
                    pData[ni].nRow = nEndRow;
                    pData[ni].pPattern = pPattern;
                    ni++;
                    nInsert = MAXROWCOUNT;
                }
                if ( ni < nj )
                {   // remove entries
                    memmove( pData + ni, pData + nj, (nCount - nj) * sizeof(ScAttrEntry) );
                    nCount -= nj - ni;
                }
            }

            if ( nInsert < sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
            {   // insert or append new entry
                if ( nInsert <= nCount )
                {
                    if ( !bSplit )
                        memmove( pData + nInsert + 1, pData + nInsert,
                            (nCount - nInsert) * sizeof(ScAttrEntry) );
                    else
                    {
                        memmove( pData + nInsert + 2, pData + nInsert,
                            (nCount - nInsert) * sizeof(ScAttrEntry) );
                        pData[nInsert+1] = pData[nInsert-1];
                        nCount++;
                    }
                }
                if ( nInsert )
                    pData[nInsert-1].nRow = nStartRow - 1;
                pData[nInsert].nRow = nEndRow;
                pData[nInsert].pPattern = pPattern;

                // Remove character attributes from these cells if the pattern
                // is applied during normal session.
                if (pDataArray)
                    RemoveCellCharAttribs(nStartRow, nEndRow, pPattern, pDataArray);

                nCount++;
            }

            if (pDocument->IsStreamValid(nTab))
                pDocument->SetStreamValid(nTab, false);
        }
    }

#if OSL_DEBUG_LEVEL > 1
    TestData();
#endif
}


void ScAttrArray::ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, ScStyleSheet* pStyle )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
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
            const ScPatternAttr* pOldPattern = pData[nPos].pPattern;
            ScPatternAttr* pNewPattern = new ScPatternAttr(*pOldPattern);
            pNewPattern->SetStyleSheet(pStyle);
            SCROW nY1 = nStart;
            SCROW nY2 = pData[nPos].nRow;
            nStart = pData[nPos].nRow + 1;

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
                SetPatternArea( nY1, nY2, pNewPattern, true );
                Search( nStart, nPos );
            }
            else
            {
                // ensure attributing changes text width of cell; otherwise
                // there aren't (yet) template format changes
                const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rNewSet, rOldSet ) )
                {
                    aAdrStart.SetRow( nPos ? pData[nPos-1].nRow+1 : 0 );
                    aAdrEnd  .SetRow( pData[nPos].nRow );
                    pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                }

                pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                pData[nPos].pPattern = (const ScPatternAttr*)
                                            &pDocument->GetPool()->Put(*pNewPattern);
                if (Concat(nPos))
                    Search(nStart, nPos);
                else
                    nPos++;
            }
            delete pNewPattern;
        }
        while ((nStart <= nEndRow) && (nPos < nCount));

        if (pDocument->IsStreamValid(nTab))
            pDocument->SetStreamValid(nTab, false);
    }

#if OSL_DEBUG_LEVEL > 1
    TestData();
#endif
}


    // const cast, otherwise it will be too inefficient/complicated
#define SET_LINECOLOR(dest,c)                    \
    if ((dest))                                  \
    {                                            \
        ((SvxBorderLine*)(dest))->SetColor((c)); \
    }

#define SET_LINE(dest,src)                             \
    if ((dest))                                        \
    {                                                  \
        SvxBorderLine* pCast = (SvxBorderLine*)(dest); \
        pCast->SetBorderLineStyle( (src)->GetBorderLineStyle() ); \
        pCast->SetWidth( (src)->GetWidth( ) );         \
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
        if (!Search( nStartRow, nPos ))
        {
            OSL_FAIL("Search failure");
            return;
        }

        do
        {
            const ScPatternAttr*    pOldPattern = pData[nPos].pPattern;
            const SfxItemSet&       rOldSet = pOldPattern->GetItemSet();
            const SfxPoolItem*      pBoxItem = 0;
            SfxItemState            eState = rOldSet.GetItemState( ATTR_BORDER, true, &pBoxItem );
            const SfxPoolItem*      pTLBRItem = 0;
            SfxItemState            eTLBRState = rOldSet.GetItemState( ATTR_BORDER_TLBR, true, &pTLBRItem );
            const SfxPoolItem*      pBLTRItem = 0;
            SfxItemState            eBLTRState = rOldSet.GetItemState( ATTR_BORDER_BLTR, true, &pBLTRItem );

            if ( (SFX_ITEM_SET == eState) || (SFX_ITEM_SET == eTLBRState) || (SFX_ITEM_SET == eBLTRState) )
            {
                ScPatternAttr*  pNewPattern = new ScPatternAttr(*pOldPattern);
                SfxItemSet&     rNewSet = pNewPattern->GetItemSet();
                SCROW           nY1 = nStart;
                SCROW           nY2 = pData[nPos].nRow;

                SvxBoxItem*     pNewBoxItem = pBoxItem ? (SvxBoxItem*)pBoxItem->Clone() : 0;
                SvxLineItem*    pNewTLBRItem = pTLBRItem ? (SvxLineItem*)pTLBRItem->Clone() : 0;
                SvxLineItem*    pNewBLTRItem = pBLTRItem ? (SvxLineItem*)pBLTRItem->Clone() : 0;

                // fetch line and update attributes with parameters

                if ( !pLine )
                {
                    if( pNewBoxItem )
                    {
                        if ( pNewBoxItem->GetTop() )    pNewBoxItem->SetLine( NULL, BOX_LINE_TOP );
                        if ( pNewBoxItem->GetBottom() ) pNewBoxItem->SetLine( NULL, BOX_LINE_BOTTOM );
                        if ( pNewBoxItem->GetLeft() )   pNewBoxItem->SetLine( NULL, BOX_LINE_LEFT );
                        if ( pNewBoxItem->GetRight() )  pNewBoxItem->SetLine( NULL, BOX_LINE_RIGHT );
                    }
                    if( pNewTLBRItem && pNewTLBRItem->GetLine() )
                        pNewTLBRItem->SetLine( 0 );
                    if( pNewBLTRItem && pNewBLTRItem->GetLine() )
                        pNewBLTRItem->SetLine( 0 );
                }
                else
                {
                    if ( bColorOnly )
                    {
                        Color aColor( pLine->GetColor() );
                        if( pNewBoxItem )
                        {
                            SET_LINECOLOR( pNewBoxItem->GetTop(),    aColor );
                            SET_LINECOLOR( pNewBoxItem->GetBottom(), aColor );
                            SET_LINECOLOR( pNewBoxItem->GetLeft(),   aColor );
                            SET_LINECOLOR( pNewBoxItem->GetRight(),   aColor );
                        }
                        if( pNewTLBRItem )
                            SET_LINECOLOR( pNewTLBRItem->GetLine(), aColor );
                        if( pNewBLTRItem )
                            SET_LINECOLOR( pNewBLTRItem->GetLine(), aColor );
                    }
                    else
                    {
                        if( pNewBoxItem )
                        {
                            SET_LINE( pNewBoxItem->GetTop(),    pLine );
                            SET_LINE( pNewBoxItem->GetBottom(), pLine );
                            SET_LINE( pNewBoxItem->GetLeft(),   pLine );
                            SET_LINE( pNewBoxItem->GetRight(),   pLine );
                        }
                        if( pNewTLBRItem )
                            SET_LINE( pNewTLBRItem->GetLine(), pLine );
                        if( pNewBLTRItem )
                            SET_LINE( pNewBLTRItem->GetLine(), pLine );
                    }
                }
                if( pNewBoxItem )   rNewSet.Put( *pNewBoxItem );
                if( pNewTLBRItem )  rNewSet.Put( *pNewTLBRItem );
                if( pNewBLTRItem )  rNewSet.Put( *pNewBLTRItem );

                nStart = pData[nPos].nRow + 1;

                if ( nY1 < nStartRow || nY2 > nEndRow )
                {
                    if (nY1 < nStartRow) nY1=nStartRow;
                    if (nY2 > nEndRow) nY2=nEndRow;
                    SetPatternArea( nY1, nY2, pNewPattern, true );
                    Search( nStart, nPos );
                }
                else
                {
                    // remove from pool ?
                    pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                    pData[nPos].pPattern = (const ScPatternAttr*)
                                &pDocument->GetPool()->Put(*pNewPattern);

                    if (Concat(nPos))
                        Search(nStart, nPos);
                    else
                        nPos++;
                }
                delete pNewBoxItem;
                delete pNewTLBRItem;
                delete pNewBLTRItem;
                delete pNewPattern;
            }
            else
            {
                nStart = pData[nPos].nRow + 1;
                nPos++;
            }
        }
        while ((nStart <= nEndRow) && (nPos < nCount));
    }
}

#undef SET_LINECOLOR
#undef SET_LINE


void ScAttrArray::ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache, ScEditDataArray* pDataArray )
{
#if OSL_DEBUG_LEVEL > 1
    TestData();
#endif

    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
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
            const ScPatternAttr* pOldPattern = pData[nPos].pPattern;
            const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pCache->ApplyTo( *pOldPattern, true );
            ScDocumentPool::CheckRef( *pOldPattern );
            ScDocumentPool::CheckRef( *pNewPattern );
            if (pNewPattern != pOldPattern)
            {
                SCROW nY1 = nStart;
                SCROW nY2 = pData[nPos].nRow;
                nStart = pData[nPos].nRow + 1;

                if ( nY1 < nStartRow || nY2 > nEndRow )
                {
                    if (nY1 < nStartRow) nY1=nStartRow;
                    if (nY2 > nEndRow) nY2=nEndRow;
                    SetPatternArea( nY1, nY2, pNewPattern, false, pDataArray );
                    Search( nStart, nPos );
                }
                else
                {
                    // ensure attributing changes text-width of cell

                    const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                    const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                    bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( nPos ? pData[nPos-1].nRow+1 : 0 );
                        aAdrEnd  .SetRow( pData[nPos].nRow );
                        pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
                    }

                    pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                    pData[nPos].pPattern = pNewPattern;
                    if (Concat(nPos))
                        Search(nStart, nPos);
                    else
                        ++nPos;
                }
            }
            else
            {
                nStart = pData[nPos].nRow + 1;
                ++nPos;
            }
        }
        while (nStart <= nEndRow);

        if (pDocument->IsStreamValid(nTab))
            pDocument->SetStreamValid(nTab, false);
    }

#if OSL_DEBUG_LEVEL > 1
    TestData();
#endif
}

bool ScAttrArray::SetAttrEntries(ScAttrEntry* pNewData, SCSIZE nSize)
{
    if (pData)
    {
        ScDocumentPool* pDocPool = pDocument->GetPool();
        for (SCSIZE i=0; i<nCount; i++)
            pDocPool->Remove(*pData[i].pPattern);

        delete[] pData;
    }

    pData = pNewData;
    nCount = nLimit = nSize;
    return true;
}

static void lcl_MergeDeep( SfxItemSet& rMergeSet, const SfxItemSet& rSource )
{
    const SfxPoolItem* pNewItem;
    const SfxPoolItem* pOldItem;
    for (sal_uInt16 nId=ATTR_PATTERN_START; nId<=ATTR_PATTERN_END; nId++)
    {
        //  pMergeSet has no parent
        SfxItemState eOldState = rMergeSet.GetItemState( nId, false, &pOldItem );

        if ( eOldState == SFX_ITEM_DEFAULT )
        {
            SfxItemState eNewState = rSource.GetItemState( nId, true, &pNewItem );
            if ( eNewState == SFX_ITEM_SET )
            {
                if ( *pNewItem != rMergeSet.GetPool()->GetDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        else if ( eOldState == SFX_ITEM_SET )               // Item gesetzt
        {
            SfxItemState eNewState = rSource.GetItemState( nId, true, &pNewItem );
            if ( eNewState == SFX_ITEM_SET )
            {
                if ( pNewItem != pOldItem )                 // beide gepuhlt
                    rMergeSet.InvalidateItem( nId );
            }
            else            // Default
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
        SCSIZE nPos;
        SCROW nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            OSL_FAIL("Search failure");
            return;
        }

        do
        {
            // similar patterns must not be repeated

            const ScPatternAttr* pPattern = pData[nPos].pPattern;
            if ( pPattern != rState.pOld1 && pPattern != rState.pOld2 )
            {
                const SfxItemSet& rThisSet = pPattern->GetItemSet();
                if (rState.pItemSet)
                {
                    if (bDeep)
                        lcl_MergeDeep( *rState.pItemSet, rThisSet );
                    else
                        rState.pItemSet->MergeValues( rThisSet, false );
                }
                else
                {
                    // first pattern - copied from parent
                    rState.pItemSet = new SfxItemSet( *rThisSet.GetPool(), rThisSet.GetRanges() );
                    rState.pItemSet->Set( rThisSet, bDeep );
                }

                rState.pOld2 = rState.pOld1;
                rState.pOld1 = pPattern;
            }

            nStart = pData[nPos].nRow + 1;
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
    rpNew = NULL;
    return true;              // another line -> don't care
}


static void lcl_MergeToFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                ScLineFlags& rFlags, const ScPatternAttr* pPattern,
                                bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom )
{
    // right/bottom border set when connected together
    const ScMergeAttr& rMerge = (const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE);
    if ( rMerge.GetColMerge() == nDistRight + 1 )
        nDistRight = 0;
    if ( rMerge.GetRowMerge() == nDistBottom + 1 )
        nDistBottom = 0;

    const SvxBoxItem* pCellFrame = (SvxBoxItem*) &pPattern->GetItemSet().Get( ATTR_BORDER );
    const SvxBorderLine* pLeftAttr   = pCellFrame->GetLeft();
    const SvxBorderLine* pRightAttr  = pCellFrame->GetRight();
    const SvxBorderLine* pTopAttr    = pCellFrame->GetTop();
    const SvxBorderLine* pBottomAttr = pCellFrame->GetBottom();
    const SvxBorderLine* pNew;

    if (bTop)
    {
        if (lcl_TestAttr( pLineOuter->GetTop(), pTopAttr, rFlags.nTop, pNew ))
            pLineOuter->SetLine( pNew, BOX_LINE_TOP );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetHori(), pTopAttr, rFlags.nHori, pNew ))
            pLineInner->SetLine( pNew, BOXINFO_LINE_HORI );
    }

    if (nDistBottom == 0)
    {
        if (lcl_TestAttr( pLineOuter->GetBottom(), pBottomAttr, rFlags.nBottom, pNew ))
            pLineOuter->SetLine( pNew, BOX_LINE_BOTTOM );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetHori(), pBottomAttr, rFlags.nHori, pNew ))
            pLineInner->SetLine( pNew, BOXINFO_LINE_HORI );
    }

    if (bLeft)
    {
        if (lcl_TestAttr( pLineOuter->GetLeft(), pLeftAttr, rFlags.nLeft, pNew ))
            pLineOuter->SetLine( pNew, BOX_LINE_LEFT );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetVert(), pLeftAttr, rFlags.nVert, pNew ))
            pLineInner->SetLine( pNew, BOXINFO_LINE_VERT );
    }

    if (nDistRight == 0)
    {
        if (lcl_TestAttr( pLineOuter->GetRight(), pRightAttr, rFlags.nRight, pNew ))
            pLineOuter->SetLine( pNew, BOX_LINE_RIGHT );
    }
    else
    {
        if (lcl_TestAttr( pLineInner->GetVert(), pRightAttr, rFlags.nVert, pNew ))
            pLineInner->SetLine( pNew, BOXINFO_LINE_VERT );
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
    else
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
            pPattern = (ScPatternAttr*) pData[i].pPattern;
            lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false,
                            nEndRow - Min( pData[i].nRow, (SCROW)(nEndRow-1) ) );
            // nDistBottom here always > 0
        }

        pPattern = GetPattern( nEndRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, false, 0 );
    }
}

//
// apply border
//

// ApplyFrame - on an entry into the array

bool ScAttrArray::ApplyFrame( const SvxBoxItem*     pBoxItem,
                              const SvxBoxInfoItem* pBoxInfoItem,
                              SCROW nStartRow, SCROW nEndRow,
                              bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom )
{
    OSL_ENSURE( pBoxItem && pBoxInfoItem, "Linienattribute fehlen!" );

    const ScPatternAttr* pPattern = GetPattern( nStartRow );
    const SvxBoxItem* pOldFrame = (const SvxBoxItem*)
                                  &pPattern->GetItemSet().Get( ATTR_BORDER );

    // right/bottom border set when connected together
    const ScMergeAttr& rMerge = (const ScMergeAttr&)pPattern->GetItem(ATTR_MERGE);
    if ( rMerge.GetColMerge() == nDistRight + 1 )
        nDistRight = 0;
    if ( rMerge.GetRowMerge() == nDistBottom + 1 )
        nDistBottom = 0;

    SvxBoxItem aNewFrame( *pOldFrame );

    if ( bLeft ? pBoxInfoItem->IsValid(VALID_LEFT) : pBoxInfoItem->IsValid(VALID_VERT) )
        aNewFrame.SetLine( bLeft ? pBoxItem->GetLeft() : pBoxInfoItem->GetVert(),
            BOX_LINE_LEFT );
    if ( (nDistRight==0) ? pBoxInfoItem->IsValid(VALID_RIGHT) : pBoxInfoItem->IsValid(VALID_VERT) )
        aNewFrame.SetLine( (nDistRight==0) ? pBoxItem->GetRight() : pBoxInfoItem->GetVert(),
            BOX_LINE_RIGHT );
    if ( bTop ? pBoxInfoItem->IsValid(VALID_TOP) : pBoxInfoItem->IsValid(VALID_HORI) )
        aNewFrame.SetLine( bTop ? pBoxItem->GetTop() : pBoxInfoItem->GetHori(),
            BOX_LINE_TOP );
    if ( (nDistBottom==0) ? pBoxInfoItem->IsValid(VALID_BOTTOM) : pBoxInfoItem->IsValid(VALID_HORI) )
        aNewFrame.SetLine( (nDistBottom==0) ? pBoxItem->GetBottom() : pBoxInfoItem->GetHori(),
            BOX_LINE_BOTTOM );

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


void ScAttrArray::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight )
{
    if (nStartRow == nEndRow)
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, true, 0 );
    else
    {
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nStartRow, bLeft, nDistRight,
                        true, nEndRow-nStartRow );

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
                nTmpEnd = Min( (SCROW)(nEndRow-1), (SCROW)(pData[i].nRow) );
                bool bChanged = ApplyFrame( pLineOuter, pLineInner, nTmpStart, nTmpEnd,
                                            bLeft, nDistRight, false, nEndRow-nTmpEnd );
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

        ApplyFrame( pLineOuter, pLineInner, nEndRow, nEndRow, bLeft, nDistRight, false, 0 );
    }
}

// Test if field contains specific attribute

bool ScAttrArray::HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nRow1, nStartIndex );
    Search( nRow2, nEndIndex );
    bool bFound = false;

    for (SCSIZE i=nStartIndex; i<=nEndIndex && !bFound; i++)
    {
        const ScPatternAttr* pPattern = pData[i].pPattern;
        if ( nMask & HASATTR_MERGED )
        {
            const ScMergeAttr* pMerge =
                    (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
            if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
                bFound = true;
        }
        if ( nMask & ( HASATTR_OVERLAPPED | HASATTR_NOTOVERLAPPED | HASATTR_AUTOFILTER ) )
        {
            const ScMergeFlagAttr* pMergeFlag =
                    (const ScMergeFlagAttr*) &pPattern->GetItem( ATTR_MERGE_FLAG );
            if ( (nMask & HASATTR_OVERLAPPED) && pMergeFlag->IsOverlapped() )
                bFound = true;
            if ( (nMask & HASATTR_NOTOVERLAPPED) && !pMergeFlag->IsOverlapped() )
                bFound = true;
            if ( (nMask & HASATTR_AUTOFILTER) && pMergeFlag->HasAutoFilter() )
                bFound = true;
        }
        if ( nMask & HASATTR_LINES )
        {
            const SvxBoxItem* pBox =
                    (const SvxBoxItem*) &pPattern->GetItem( ATTR_BORDER );
            if ( pBox->GetLeft() || pBox->GetRight() || pBox->GetTop() || pBox->GetBottom() )
                bFound = true;
        }
        if ( nMask & HASATTR_SHADOW )
        {
            const SvxShadowItem* pShadow =
                    (const SvxShadowItem*) &pPattern->GetItem( ATTR_SHADOW );
            if ( pShadow->GetLocation() != SVX_SHADOW_NONE )
                bFound = true;
        }
        if ( nMask & HASATTR_CONDITIONAL )
        {
            bool bContainsCondFormat =
                    !static_cast<const ScCondFormatItem&>(pPattern->GetItem( ATTR_CONDITIONAL )).GetCondFormatData().empty();
            if ( bContainsCondFormat )
                bFound = true;
        }
        if ( nMask & HASATTR_PROTECTED )
        {
            const ScProtectionAttr* pProtect =
                    (const ScProtectionAttr*) &pPattern->GetItem( ATTR_PROTECTION );
            bool bFoundTemp = false;
            if ( pProtect->GetProtection() || pProtect->GetHideCell() )
                bFoundTemp = true;

            bool bContainsCondFormat =
                    !static_cast<const ScCondFormatItem&>(pPattern->GetItem( ATTR_CONDITIONAL )).GetCondFormatData().empty();
            if ( bContainsCondFormat )
            {
                SCROW nRowStartCond = std::max<SCROW>( nRow1, i ? pData[i-1].nRow + 1: 0 );
                SCROW nRowEndCond = std::min<SCROW>( nRow2, pData[i].nRow );
                bool bFoundCond = false;
                for(SCROW nRowCond = nRowStartCond; nRowCond <= nRowEndCond && !bFoundCond; ++nRowCond)
                {
                    const SfxItemSet* pSet = pDocument->GetCondResult( nCol, nRowCond, nTab );

                    const SfxPoolItem* pItem;
                    if( pSet && pSet->GetItemState( ATTR_PROTECTION, true, &pItem ) == SFX_ITEM_SET )
                    {
                        const ScProtectionAttr* pCondProtect = static_cast<const ScProtectionAttr*>(pItem);
                        if( pCondProtect->GetProtection() || pProtect->GetHideCell() )
                            bFoundCond = true;
                    }
                    else
                    {
                        // well it is not true that we found one
                        // but existing one + cell where conditional
                        // formatting does not remove it
                        // => we have a protected cell
                        bFoundCond = true;
                    }
                }
                bFoundTemp = bFoundCond;
            }

            if(bFoundTemp)
                bFound = true;
        }
        if ( nMask & HASATTR_ROTATE )
        {
            const SfxInt32Item* pRotate =
                    (const SfxInt32Item*) &pPattern->GetItem( ATTR_ROTATE_VALUE );
            // 90 or 270 degrees is former SvxOrientationItem - only look for other values
            // (see ScPatternAttr::GetCellOrientation)
            sal_Int32 nAngle = pRotate->GetValue();
            if ( nAngle != 0 && nAngle != 9000 && nAngle != 27000 )
                bFound = true;
        }
        if ( nMask & HASATTR_NEEDHEIGHT )
        {
            if (pPattern->GetCellOrientation() != SVX_ORIENTATION_STANDARD)
                bFound = true;
            else if (((const SfxBoolItem&)pPattern->GetItem( ATTR_LINEBREAK )).GetValue())
                bFound = true;
            else if ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                        GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_BLOCK)
                bFound = true;
            else if (((const SfxUInt32Item&)pPattern->GetItem( ATTR_CONDITIONAL )).GetValue())
                bFound = true;
            else if (((const SfxInt32Item&)pPattern->GetItem( ATTR_ROTATE_VALUE )).GetValue())
                bFound = true;
        }
        if ( nMask & ( HASATTR_SHADOW_RIGHT | HASATTR_SHADOW_DOWN ) )
        {
            const SvxShadowItem* pShadow =
                    (const SvxShadowItem*) &pPattern->GetItem( ATTR_SHADOW );
            SvxShadowLocation eLoc = pShadow->GetLocation();
            if ( nMask & HASATTR_SHADOW_RIGHT )
                if ( eLoc == SVX_SHADOW_TOPRIGHT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    bFound = true;
            if ( nMask & HASATTR_SHADOW_DOWN )
                if ( eLoc == SVX_SHADOW_BOTTOMLEFT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    bFound = true;
        }
        if ( nMask & HASATTR_RTL )
        {
            const SvxFrameDirectionItem& rDirection =
                    (const SvxFrameDirectionItem&) pPattern->GetItem( ATTR_WRITINGDIR );
            if ( rDirection.GetValue() == FRMDIR_HORI_RIGHT_TOP )
                bFound = true;
        }
        if ( nMask & HASATTR_RIGHTORCENTER )
        {
            //  called only if the sheet is LTR, so physical=logical alignment can be assumed
            SvxCellHorJustify eHorJust = (SvxCellHorJustify)
                    ((const SvxHorJustifyItem&) pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();
            if ( eHorJust == SVX_HOR_JUSTIFY_RIGHT || eHorJust == SVX_HOR_JUSTIFY_CENTER )
                bFound = true;
        }
    }

    return bFound;
}

// Area around any given summaries expand and adapt any MergeFlag (bRefresh)
bool ScAttrArray::ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh )
{
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nStartRow, nStartIndex );
    Search( nEndRow, nEndIndex );
    bool bFound = false;

    for (SCSIZE i=nStartIndex; i<=nEndIndex; i++)
    {
        pPattern = pData[i].pPattern;
        pItem = (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
        SCsCOL  nCountX = pItem->GetColMerge();
        SCsROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            SCROW nThisRow = (i>0) ? pData[i-1].nRow+1 : 0;
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
                    pDocument->ApplyFlagsTab( nThisCol+1, nThisRow, nMergeEndCol, pData[i].nRow,
                                nTab, SC_MF_HOR );
                if ( nMergeEndRow > nThisRow )
                    pDocument->ApplyFlagsTab( nThisCol, nThisRow+1, nThisCol, nMergeEndRow,
                                nTab, SC_MF_VER );
                if ( nMergeEndCol > nThisCol && nMergeEndRow > nThisRow )
                    pDocument->ApplyFlagsTab( nThisCol+1, nThisRow+1, nMergeEndCol, nMergeEndRow,
                                nTab, SC_MF_HOR | SC_MF_VER );

                Search( nThisRow, i );    // Data changed
                Search( nStartRow, nStartIndex );
                Search( nEndRow, nEndIndex );
            }
        }
    }

    return bFound;
}


bool ScAttrArray::RemoveAreaMerge(SCROW nStartRow, SCROW nEndRow)
{
    bool bFound = false;
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    SCSIZE nIndex;

    Search( nStartRow, nIndex );
    SCROW nThisStart = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisStart < nStartRow)
        nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        SCROW nThisEnd = pData[nIndex].nRow;
        if (nThisEnd > nEndRow)
            nThisEnd = nEndRow;

        pPattern = pData[nIndex].pPattern;
        pItem = (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
        SCsCOL  nCountX = pItem->GetColMerge();
        SCsROW  nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            const ScMergeAttr* pAttr = (const ScMergeAttr*)
                                            &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
            const ScMergeFlagAttr* pFlagAttr = (const ScMergeFlagAttr*)
                                            &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE_FLAG );

            OSL_ENSURE( nCountY==1 || nThisStart==nThisEnd, "What's up?" );

            SCCOL nThisCol = nCol;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisEnd + nCountY - 1;

            // ApplyAttr for areas

            for (SCROW nThisRow = nThisStart; nThisRow <= nThisEnd; nThisRow++)
                pDocument->ApplyAttr( nThisCol, nThisRow, nTab, *pAttr );

            ScPatternAttr*  pNewPattern = new ScPatternAttr( pDocument->GetPool() );
            SfxItemSet*     pSet = &pNewPattern->GetItemSet();
            pSet->Put( *pFlagAttr );
            pDocument->ApplyPatternAreaTab( nThisCol, nThisStart, nMergeEndCol, nMergeEndRow,
                                                nTab, *pNewPattern );
            delete pNewPattern;

            Search( nThisEnd, nIndex );    // data changed
        }

        ++nIndex;
        if ( nIndex < nCount )
            nThisStart = pData[nIndex-1].nRow+1;
        else
            nThisStart = MAXROW+1;   // End
    }

    return bFound;
}

// Remove field, but leave MergeFlags

void ScAttrArray::DeleteAreaSafe(SCROW nStartRow, SCROW nEndRow)
{
    SetPatternAreaSafe( nStartRow, nEndRow, pDocument->GetDefPattern(), true );
}


void ScAttrArray::SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                        const ScPatternAttr* pWantedPattern, bool bDefault )
{
    const ScPatternAttr*    pOldPattern;
    const ScMergeFlagAttr*  pItem;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bFirstUse = true;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;
        if (pOldPattern != pWantedPattern)                          //! else-Zweig ?
        {
            if (nThisRow < nStartRow) nThisRow = nStartRow;
            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );
            pItem = (const ScMergeFlagAttr*) &pOldPattern->GetItem( ATTR_MERGE_FLAG );

            if (pItem->IsOverlapped() || pItem->HasAutoFilter())
            {
                //  default-constructing a ScPatternAttr for DeleteArea doesn't work
                //  because it would have no cell style information.
                //  Instead, the document's GetDefPattern is copied. Since it is passed as
                //  pWantedPattern, no special treatment of default is needed here anymore.
                ScPatternAttr*  pNewPattern = new ScPatternAttr( *pWantedPattern );
                SfxItemSet*     pSet = &pNewPattern->GetItemSet();
                pSet->Put( *pItem );
                SetPatternArea( nThisRow, nAttrRow, pNewPattern, true );
                delete pNewPattern;
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
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


bool ScAttrArray::ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    sal_Int16   nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bChanged = false;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;
        nOldValue = ((const ScMergeFlagAttr*) &pOldPattern->GetItem( ATTR_MERGE_FLAG ))->GetValue();
        if ( (nOldValue | nFlags) != nOldValue )
        {
            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue | nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
            bChanged = true;
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }

    return bChanged;
}


bool ScAttrArray::RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    sal_Int16   nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    bool    bChanged = false;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;
        nOldValue = ((const ScMergeFlagAttr*) &pOldPattern->GetItem( ATTR_MERGE_FLAG ))->GetValue();
        if ( (nOldValue & ~nFlags) != nOldValue )
        {
            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue & ~nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
            bChanged = true;
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }

    return bChanged;
}


void ScAttrArray::ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich )
{
    const ScPatternAttr* pOldPattern;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;
        if ( pOldPattern->HasItemsSet( pWhich ) )
        {
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.ClearItems( pWhich );

            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );
            Search( nThisRow, nIndex );  // data changed
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


void ScAttrArray::ChangeIndent( SCROW nStartRow, SCROW nEndRow, bool bIncrement )
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisStart < nStartRow) nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = pData[nIndex].pPattern;
        const SfxItemSet& rOldSet = pOldPattern->GetItemSet();
        const SfxPoolItem* pItem;

        bool bNeedJust = ( rOldSet.GetItemState( ATTR_HOR_JUSTIFY, false, &pItem ) != SFX_ITEM_SET
                        || ((const SvxHorJustifyItem*)pItem)->GetValue() != SVX_HOR_JUSTIFY_LEFT );
        sal_uInt16 nOldValue = ((const SfxUInt16Item&)rOldSet.Get( ATTR_INDENT )).GetValue();
        sal_uInt16 nNewValue = nOldValue;
        if ( bIncrement )
        {
            if ( nNewValue < SC_MAX_INDENT )
            {
                nNewValue += SC_INDENT_STEP;
                if ( nNewValue > SC_MAX_INDENT ) nNewValue = SC_MAX_INDENT;
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
            SCROW nThisEnd = pData[nIndex].nRow;
            SCROW nAttrRow = Min( nThisEnd, nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, nNewValue ) );
            if ( bNeedJust )
                aNewPattern.GetItemSet().Put(
                                SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
            SetPatternArea( nThisStart, nAttrRow, &aNewPattern, true );

            nThisStart = nThisEnd + 1;
            Search( nThisStart, nIndex ); // data changed
        }
        else
        {
            nThisStart = pData[nIndex].nRow + 1;
            ++nIndex;
        }
    }
}


SCsROW ScAttrArray::GetNextUnprotected( SCsROW nRow, bool bUp ) const
{
    long nRet = nRow;
    if (VALIDROW(nRow))
    {
        SCSIZE nIndex;
        Search(nRow, nIndex);
        while (((const ScProtectionAttr&)pData[nIndex].pPattern->
                GetItem(ATTR_PROTECTION)).GetProtection())
        {
            if (bUp)
            {
                if (nIndex==0)
                    return -1;   // not found
                --nIndex;
                nRet = pData[nIndex].nRow;
            }
            else
            {
                nRet = pData[nIndex].nRow+1;
                ++nIndex;
                if (nIndex>=nCount)
                    return MAXROW+1; // not found
            }
        }
    }
    return nRet;
}

void ScAttrArray::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset )
{
    SCROW nStart = 0;
    SCSIZE nPos = 0;
    while (nPos < nCount)
    {
        SCROW nEnd = pData[nPos].nRow;
        if (pData[nPos].pPattern->GetStyleSheet() == pStyleSheet)
        {
            rUsedRows.setTrue(nStart, nEnd);

            if (bReset)
            {
                ScPatternAttr* pNewPattern = new ScPatternAttr(*pData[nPos].pPattern);
                pDocument->GetPool()->Remove(*pData[nPos].pPattern);
                pNewPattern->SetStyleSheet( (ScStyleSheet*)
                    pDocument->GetStyleSheetPool()->
                        Find( ScGlobal::GetRscString(STR_STYLENAME_STANDARD),
                              SFX_STYLE_FAMILY_PARA,
                              SFXSTYLEBIT_AUTO | SCSTYLEBIT_STANDARD ) );
                pData[nPos].pPattern = (const ScPatternAttr*)
                                            &pDocument->GetPool()->Put(*pNewPattern);
                delete pNewPattern;

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


bool ScAttrArray::IsStyleSheetUsed( const ScStyleSheet& rStyle,
        bool bGatherAllStyles ) const
{
    bool    bIsUsed = false;
    SCSIZE  nPos    = 0;

    while ( nPos < nCount )
    {
        const ScStyleSheet* pStyle = pData[nPos].pPattern->GetStyleSheet();
        if ( pStyle )
        {
            pStyle->SetUsage( ScStyleSheet::USED );
            if ( pStyle == &rStyle )
            {
                if ( !bGatherAllStyles )
                    return true;
                bIsUsed = true;
            }
        }
        nPos++;
    }

    return bIsUsed;
}


bool ScAttrArray::IsEmpty() const
{
    if (nCount == 1)
    {
        if ( pData[0].pPattern != pDocument->GetDefPattern() )
            return false;
        else
            return true;
    }
    else
        return false;
}


bool ScAttrArray::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    OSL_ENSURE( nCount, "nCount == 0" );

    bool bFound = false;
    SCSIZE nStart = 0;

    // Skip first entry if more than 1 row.
    // Entries at the end are not skipped, GetFirstVisibleAttr may be larger than GetLastVisibleAttr.

    SCSIZE nVisStart = 1;
    while ( nVisStart < nCount && pData[nVisStart].pPattern->IsVisibleEqual(*pData[nVisStart-1].pPattern) )
        ++nVisStart;
    if ( nVisStart >= nCount || pData[nVisStart-1].nRow > 0 )   // more than 1 row?
        nStart = nVisStart;

    while ( nStart < nCount && !bFound )
    {
        if ( pData[nStart].pPattern->IsVisible() )
        {
            rFirstRow = nStart ? ( pData[nStart-1].nRow + 1 ) : 0;
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

bool ScAttrArray::GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData, bool bFullFormattedArea ) const
{
    OSL_ENSURE( nCount, "nCount == 0" );

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
    SCSIZE nPos = nCount - 1;
    SCROW nStartRow = (nPos ? pData[nPos-1].nRow + 1 : 0);
    if (nStartRow <= nLastData + 1)
    {
        if (bFullFormattedArea && pData[nPos].pPattern->IsVisible())
        {
            rLastRow = pData[nPos].nRow;
            return true;
        }
        else
        {
            // Ignore here a few rows if data happens to end within
            // SC_VISATTR_STOP rows before MAXROW.
            rLastRow = nLastData;
            return false;
        }
    }

    // Find a run below last data row.
    bool bFound = false;
    Search( nLastData, nPos );
    while ( nPos < nCount )
    {
        // find range of visually equal formats
        SCSIZE nEndPos = nPos;
        while ( nEndPos < nCount-1 &&
                pData[nEndPos].pPattern->IsVisibleEqual( *pData[nEndPos+1].pPattern))
            ++nEndPos;
        SCROW nAttrStartRow = ( nPos > 0 ) ? ( pData[nPos-1].nRow + 1 ) : 0;
        if ( nAttrStartRow <= nLastData )
            nAttrStartRow = nLastData + 1;
        SCROW nAttrSize = pData[nEndPos].nRow + 1 - nAttrStartRow;
        if ( nAttrSize >= SC_VISATTR_STOP && !bFullFormattedArea )
            break;  // while, ignore this range and below
        else if ( pData[nEndPos].pPattern->IsVisible() )
        {
            rLastRow = pData[nEndPos].nRow;
            bFound = true;
        }
        nPos = nEndPos + 1;
    }

    return bFound;
}


bool ScAttrArray::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = nStartRow;
    bool bFound = false;
    while ( nIndex < nCount && nThisStart <= nEndRow && !bFound )
    {
        if ( pData[nIndex].pPattern->IsVisible() )
            bFound = true;

        nThisStart = pData[nIndex].nRow + 1;
        ++nIndex;
    }

    return bFound;
}


bool ScAttrArray::IsVisibleEqual( const ScAttrArray& rOther,
                                    SCROW nStartRow, SCROW nEndRow ) const
{
    bool bEqual = true;
    SCSIZE nThisPos = 0;
    SCSIZE nOtherPos = 0;
    if ( nStartRow > 0 )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<nCount && nOtherPos<rOther.nCount && bEqual )
    {
        SCROW nThisRow = pData[nThisPos].nRow;
        SCROW nOtherRow = rOther.pData[nOtherPos].nRow;
        const ScPatternAttr* pThisPattern = pData[nThisPos].pPattern;
        const ScPatternAttr* pOtherPattern = rOther.pData[nOtherPos].pPattern;
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

    bool bEqual = true;
    SCSIZE nThisPos = 0;
    SCSIZE nOtherPos = 0;
    if ( nStartRow > 0 )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<nCount && nOtherPos<rOther.nCount && bEqual )
    {
        SCROW nThisRow = pData[nThisPos].nRow;
        SCROW nOtherRow = rOther.pData[nOtherPos].nRow;
        const ScPatternAttr* pThisPattern = pData[nThisPos].pPattern;
        const ScPatternAttr* pOtherPattern = rOther.pData[nOtherPos].pPattern;
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

        for ( ; nIndex < nCount; nIndex++ )
        {
            if ( ((const ScMergeFlagAttr&)pData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped() )
            {
                bTest = false;  // may not be pushed out
                break;
            }
            if ( pData[nIndex].nRow >= nEndRow ) // end of range
                break;
        }
    }
    return bTest;
}


bool ScAttrArray::TestInsertRow( SCSIZE nSize ) const
{
    // if 1st row pushed out is vertically overlapped, summary would be broken

    if (pData)
    {
        // MAXROW + 1 - nSize   = 1st row pushed out

        SCSIZE nFirstLost = nCount-1;
        while ( nFirstLost && pData[nFirstLost-1].nRow >= sal::static_int_cast<SCROW>(MAXROW + 1 - nSize) )
            --nFirstLost;

        if ( ((const ScMergeFlagAttr&)pData[nFirstLost].pPattern->
                            GetItem(ATTR_MERGE_FLAG)).IsVerOverlapped() )
            return false;
    }

    return true;
}


void ScAttrArray::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    if (!pData)
        return;

    SCROW nSearch = nStartRow > 0 ? nStartRow - 1 : 0;  // expand predecessor
    SCSIZE nIndex;
    Search( nSearch, nIndex );

    // set ScMergeAttr may not be extended (so behind delete again)

    bool bDoMerge = ((const ScMergeAttr&) pData[nIndex].pPattern->GetItem(ATTR_MERGE)).IsMerged();

    SCSIZE nRemove = 0;
    SCSIZE i;
    for (i = nIndex; i < nCount-1; i++)
    {
        SCROW nNew = pData[i].nRow + nSize;
        if ( nNew >= MAXROW )    // at end?
        {
            nNew = MAXROW;
            if (!nRemove)
                nRemove = i+1;  // remove the following?
        }
        pData[i].nRow = nNew;
    }

    // Remove entries at end ?

    if (nRemove && nRemove < nCount)
        DeleteRange( nRemove, nCount-1 );

    if (bDoMerge)   // extensively repair (again) ScMergeAttr
    {
            //  ApplyAttr for areas

        const SfxPoolItem& rDef = pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
        for (SCSIZE nAdd=0; nAdd<nSize; nAdd++)
            pDocument->ApplyAttr( nCol, nStartRow+nAdd, nTab, rDef );

        // reply inserts in this area not summarized
    }

    // Don't duplicate the merge flags in the inserted row.
    // #i108488# SC_MF_SCENARIO has to be allowed.
    RemoveFlags( nStartRow, nStartRow+nSize-1, SC_MF_HOR | SC_MF_VER | SC_MF_AUTO | SC_MF_BUTTON );
}


void ScAttrArray::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    if (pData)
    {
        bool bFirst=true;
        SCSIZE nStartIndex = 0;
        SCSIZE nEndIndex = 0;
        SCSIZE i;

        for ( i = 0; i < nCount-1; i++)
            if (pData[i].nRow >= nStartRow && pData[i].nRow <= sal::static_int_cast<SCROW>(nStartRow+nSize-1))
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
                nStart = pData[nStartIndex-1].nRow + 1;

            if (nStart < nStartRow)
            {
                pData[nStartIndex].nRow = nStartRow - 1;
                ++nStartIndex;
            }
            if (nEndIndex >= nStartIndex)
            {
                DeleteRange( nStartIndex, nEndIndex );
                if (nStartIndex > 0)
                    if ( pData[nStartIndex-1].pPattern == pData[nStartIndex].pPattern )
                        DeleteRange( nStartIndex-1, nStartIndex-1 );
            }
        }
        for (i = 0; i < nCount-1; i++)
            if (pData[i].nRow >= nStartRow)
                pData[i].nRow -= nSize;

        // Below does not follow the pattern to detect pressure ranges;
        // instead, only remove merge flags.
        RemoveFlags( MAXROW-nSize+1, MAXROW, SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
    }
}


void ScAttrArray::DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex )
{
    ScDocumentPool* pDocPool = pDocument->GetPool();
    for (SCSIZE i = nStartIndex; i <= nEndIndex; i++)
        pDocPool->Remove(*pData[i].pPattern);

    memmove( &pData[nStartIndex], &pData[nEndIndex + 1], (nCount - nEndIndex - 1) * sizeof(ScAttrEntry) );
    nCount -= nEndIndex-nStartIndex+1;
}


void ScAttrArray::DeleteArea(SCROW nStartRow, SCROW nEndRow)
{
    RemoveAreaMerge( nStartRow, nEndRow );  // remove from combined flags

    if ( !HasAttrib( nStartRow, nEndRow, HASATTR_OVERLAPPED | HASATTR_AUTOFILTER) )
        SetPatternArea( nStartRow, nEndRow, pDocument->GetDefPattern() );
    else
        DeleteAreaSafe( nStartRow, nEndRow );  // leave merge flags
}


void ScAttrArray::DeleteHardAttr(SCROW nStartRow, SCROW nEndRow)
{
    const ScPatternAttr* pDefPattern = pDocument->GetDefPattern();
    const ScPatternAttr* pOldPattern;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;

        if ( pOldPattern->GetItemSet().Count() )  // hard attributes ?
        {
            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );

            ScPatternAttr aNewPattern(*pOldPattern);
            SfxItemSet& rSet = aNewPattern.GetItemSet();
            for (sal_uInt16 nId = ATTR_PATTERN_START; nId <= ATTR_PATTERN_END; nId++)
                if (nId != ATTR_MERGE && nId != ATTR_MERGE_FLAG)
                    rSet.ClearItem(nId);

            if ( aNewPattern == *pDefPattern )
                SetPatternArea( nThisRow, nAttrRow, pDefPattern, false );
            else
                SetPatternArea( nThisRow, nAttrRow, &aNewPattern, true );

            Search( nThisRow, nIndex );  // data changed
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


// move within a document

void ScAttrArray::MoveTo(SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray)
{
    SCROW nStart = nStartRow;
    for (SCSIZE i = 0; i < nCount; i++)
    {
        if ((pData[i].nRow >= nStartRow) && ((i==0) ? true : pData[i-1].nRow < nEndRow))
        {
            // copy (bPutToPool=TRUE)
            rAttrArray.SetPatternArea( nStart, Min( (SCROW)pData[i].nRow, (SCROW)nEndRow ),
                                        pData[i].pPattern, true );
        }
        nStart = Max( (SCROW)nStart, (SCROW)(pData[i].nRow + 1) );
    }
    DeleteArea(nStartRow, nEndRow);
}


// copy between documents (Clipboard)

void ScAttrArray::CopyArea( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray,
                                sal_Int16 nStripFlags )
{
    nStartRow -= nDy;   // Source
    nEndRow -= nDy;

    SCROW nDestStart = Max((long)((long)nStartRow + nDy), (long) 0);
    SCROW nDestEnd = Min((long)((long)nEndRow + nDy), (long) MAXROW);

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    bool bSamePool = (pSourceDocPool==pDestDocPool);

    for (SCSIZE i = 0; (i < nCount) && (nDestStart <= nDestEnd); i++)
    {
        if (pData[i].nRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = pData[i].pPattern;
            const ScPatternAttr* pNewPattern;

            if (IsDefaultItem( pOldPattern ))
            {
                // default: nothing changed

                pNewPattern = (const ScPatternAttr*)
                                &pDestDocPool->GetDefaultItem( ATTR_PATTERN );
            }
            else if ( nStripFlags )
            {
                ScPatternAttr* pTmpPattern = new ScPatternAttr( *pOldPattern );
                sal_Int16 nNewFlags = 0;
                if ( nStripFlags != SC_MF_ALL )
                    nNewFlags = ((const ScMergeFlagAttr&)pTmpPattern->GetItem(ATTR_MERGE_FLAG)).
                                GetValue() & ~nStripFlags;

                if ( nNewFlags )
                    pTmpPattern->GetItemSet().Put( ScMergeFlagAttr( nNewFlags ) );
                else
                    pTmpPattern->GetItemSet().ClearItem( ATTR_MERGE_FLAG );

                if (bSamePool)
                    pNewPattern = (ScPatternAttr*) &pDestDocPool->Put(*pTmpPattern);
                else
                    pNewPattern = pTmpPattern->PutInPool( rAttrArray.pDocument, pDocument );
                delete pTmpPattern;
            }
            else
            {
                if (bSamePool)
                    pNewPattern = (ScPatternAttr*) &pDestDocPool->Put(*pOldPattern);
                else
                    pNewPattern = pOldPattern->PutInPool( rAttrArray.pDocument, pDocument );
            }

            rAttrArray.SetPatternArea(nDestStart,
                            Min((SCROW)(pData[i].nRow + nDy), nDestEnd), pNewPattern);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = Max((long)nDestStart, (long)(pData[i].nRow + nDy + 1));
    }
}


// leave flags
// summarized with CopyArea

void ScAttrArray::CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray )
{
    nStartRow -= nDy;  // Source
    nEndRow -= nDy;

    SCROW nDestStart = Max((long)((long)nStartRow + nDy), (long) 0);
    SCROW nDestEnd = Min((long)((long)nEndRow + nDy), (long) MAXROW);

    if ( !rAttrArray.HasAttrib( nDestStart, nDestEnd, HASATTR_OVERLAPPED ) )
    {
        CopyArea( nStartRow+nDy, nEndRow+nDy, nDy, rAttrArray );
        return;
    }

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    bool bSamePool = (pSourceDocPool==pDestDocPool);

    for (SCSIZE i = 0; (i < nCount) && (nDestStart <= nDestEnd); i++)
    {
        if (pData[i].nRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = pData[i].pPattern;
            const ScPatternAttr* pNewPattern;

            if (bSamePool)
                pNewPattern = (ScPatternAttr*) &pDestDocPool->Put(*pOldPattern);
            else
                pNewPattern = pOldPattern->PutInPool( rAttrArray.pDocument, pDocument );

            rAttrArray.SetPatternAreaSafe(nDestStart,
                            Min((SCROW)(pData[i].nRow + nDy), nDestEnd), pNewPattern, false);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted
        // end position can be negative
        nDestStart = Max((long)nDestStart, (long)(pData[i].nRow + nDy + 1));
    }
}


SCsROW ScAttrArray::SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                                    bool bUp, ScMarkArray* pMarkArray )
{
    bool bFound = false;

    if (pMarkArray)
    {
        nRow = pMarkArray->GetNextMarked( nRow, bUp );
        if (!VALIDROW(nRow))
            return nRow;
    }

    SCSIZE nIndex;
    Search(nRow, nIndex);
    const ScPatternAttr* pPattern = pData[nIndex].pPattern;

    while (nIndex < nCount && !bFound)
    {
        if (pPattern->GetStyleSheet() == pSearchStyle)
        {
            if (pMarkArray)
            {
                nRow = pMarkArray->GetNextMarked( nRow, bUp );
                SCROW nStart = nIndex ? pData[nIndex-1].nRow+1 : 0;
                if (nRow >= nStart && nRow <= pData[nIndex].nRow)
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
                    nIndex = nCount;
                    nRow = -1;
                }
                else
                {
                    --nIndex;
                    nRow = pData[nIndex].nRow;
                    pPattern = pData[nIndex].pPattern;
                }
            }
            else
            {
                nRow = pData[nIndex].nRow+1;
                ++nIndex;
                if (nIndex<nCount)
                    pPattern = pData[nIndex].pPattern;
            }
        }
    }

    OSL_ENSURE( bFound || !ValidRow(nRow), "Internal failure in in ScAttrArray::SearchStyle" );

    return nRow;
}


bool ScAttrArray::SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow,
                        const ScStyleSheet* pSearchStyle, bool bUp, ScMarkArray* pMarkArray )
{
    SCsROW nStartRow = SearchStyle( rRow, pSearchStyle, bUp, pMarkArray );
    if (VALIDROW(nStartRow))
    {
        SCSIZE nIndex;
        Search(nStartRow,nIndex);

        rRow = nStartRow;
        if (bUp)
        {
            if (nIndex>0)
                rEndRow = pData[nIndex-1].nRow + 1;
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
            rEndRow = pData[nIndex].nRow;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
