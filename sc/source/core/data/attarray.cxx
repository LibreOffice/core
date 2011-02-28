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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
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

#undef DBG_INVALIDATE
#define DBGOUTPUT(s) \
    DBG_ERROR( String("Invalidate ") + String(s) + String(": ") \
               + String(nCol) + String('/') + String(aAdrStart.Row()) + String('/') + String(nTab) \
               + String(" bis ") \
               + String(nCol) + String('/') + String(aAdrEnd.Row())   + String('/') + String(nTab) \
              );

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

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
        pData[0].pPattern = pDocument->GetDefPattern();     // ohne Put !!!
    }
}

//------------------------------------------------------------------------

ScAttrArray::~ScAttrArray()
{
#ifdef DBG_UTIL
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
#ifdef DBG_UTIL
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
        ByteString aMsg = ByteString::CreateFromInt32(nErr);
        aMsg += " errors in attribute array, column ";
        aMsg += ByteString::CreateFromInt32(nCol);
        DBG_ERROR( aMsg.GetBuffer() );
    }
}
#endif

//------------------------------------------------------------------------

void ScAttrArray::Reset( const ScPatternAttr* pPattern, sal_Bool bAlloc )
{
    if (pData)
    {
        ScDocumentPool*      pDocPool = pDocument->GetPool();
        const ScPatternAttr* pOldPattern;
        ScAddress            aAdrStart( nCol, 0, nTab );
        ScAddress            aAdrEnd  ( nCol, 0, nTab );

        for (SCSIZE i=0; i<nCount; i++)
        {
            // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
            pOldPattern = pData[i].pPattern;
            sal_Bool bNumFormatChanged;
            if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                    pPattern->GetItemSet(), pOldPattern->GetItemSet() ) )
            {
                aAdrStart.SetRow( i ? pData[i-1].nRow+1 : 0 );
                aAdrEnd  .SetRow( pData[i].nRow );
                pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
#ifdef DBG_INVALIDATE
                DBGOUTPUT("Reset");
#endif
            }
            // bedingtes Format gesetzt oder geloescht?
            if ( &pPattern->GetItem(ATTR_CONDITIONAL) != &pOldPattern->GetItem(ATTR_CONDITIONAL) )
            {
                pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                pOldPattern->GetItem(ATTR_CONDITIONAL)).GetValue() );
                pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() );
            }
            pDocPool->Remove(*pOldPattern);
        }
        delete[] pData;

        if (pDocument->IsStreamValid(nTab))
            pDocument->SetStreamValid(nTab, sal_False);

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
            pData = NULL;               // muss sofort wieder belegt werden !
        }
    }
}


sal_Bool ScAttrArray::Concat(SCSIZE nPos)
{
    sal_Bool bRet = sal_False;
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
                bRet = sal_True;
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
                bRet = sal_True;
            }
        }
    }
    return bRet;
}

//------------------------------------------------------------------------

sal_Bool ScAttrArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    long    nLo         = 0;
    long    nHi         = static_cast<long>(nCount) - 1;
    long    nStartRow   = 0;
    long    nEndRow     = 0;
    long    i           = 0;
    sal_Bool    bFound      = (nCount == 1);
    if (pData)
    {
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
                    bFound = sal_True;
        }
    }
    else
        bFound = sal_False;

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

//------------------------------------------------------------------------

void ScAttrArray::SetPattern( SCROW nRow, const ScPatternAttr* pPattern, sal_Bool bPutToPool )
{
    SetPatternArea( nRow, nRow, pPattern, bPutToPool );
}


void ScAttrArray::SetPatternArea(SCROW nStartRow, SCROW nEndRow, const ScPatternAttr *pPattern, sal_Bool bPutToPool )
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

            // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
            // oder bedingte Formate neu gesetzt oder geloescht werden
            while ( ns <= nEndRow )
            {
                const SfxItemSet& rNewSet = pPattern->GetItemSet();
                const SfxItemSet& rOldSet = pData[nx].pPattern->GetItemSet();

                sal_Bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rNewSet, rOldSet ) )
                {
                    aAdrStart.SetRow( Max(nStartRow,ns) );
                    aAdrEnd  .SetRow( Min(nEndRow,pData[nx].nRow) );
                    pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
#ifdef DBG_INVALIDATE
                    DBGOUTPUT("SetPatternArea");
#endif
                }
                if ( &rNewSet.Get(ATTR_CONDITIONAL) != &rOldSet.Get(ATTR_CONDITIONAL) )
                {
                    pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                    rOldSet.Get(ATTR_CONDITIONAL)).GetValue() );
                    pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                    rNewSet.Get(ATTR_CONDITIONAL)).GetValue() );
                }
                ns = pData[nx].nRow + 1;
                nx++;
            }

            // continue modifying data array

            SCSIZE nInsert;     // insert position (MAXROWCOUNT := no insert)
            sal_Bool bCombined = sal_False;
            sal_Bool bSplit = sal_False;
            if ( nStartRow > 0 )
            {
                nInsert = MAXROWCOUNT;
                if ( pData[ni].pPattern != pPattern )
                {
                    if ( ni == 0 || (pData[ni-1].nRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( pData[ni].nRow > nEndRow )
                            bSplit = sal_True;
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
                    bCombined = sal_True;
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
                    bCombined = sal_True;
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
                nCount++;
            }

            if (pDocument->IsStreamValid(nTab))
                pDocument->SetStreamValid(nTab, sal_False);
        }
    }
//  InfoBox(0, String(nCount) + String(" Eintraege") ).Execute();

#ifdef DBG_UTIL
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
            DBG_ERROR("Search-Fehler");
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
                SetPatternArea( nY1, nY2, pNewPattern, sal_True );
                Search( nStart, nPos );
            }
            else
            {
                // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
                // bedingte Formate in Vorlagen gibt es (noch) nicht

                const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                sal_Bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rNewSet, rOldSet ) )
                {
                    aAdrStart.SetRow( nPos ? pData[nPos-1].nRow+1 : 0 );
                    aAdrEnd  .SetRow( pData[nPos].nRow );
                    pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
#ifdef DBG_INVALIDATE
                    DBGOUTPUT("ApplyStyleArea");
#endif
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
            pDocument->SetStreamValid(nTab, sal_False);
    }

#ifdef DBG_UTIL
    TestData();
#endif
}


    // const wird weggecastet, weil es sonst
    // zu ineffizient/kompliziert wird!
#define SET_LINECOLOR(dest,c)                       \
    if ((dest))                                     \
    {                                               \
        ((SvxBorderLine*)(dest))->SetColor((c));    \
    }

#define SET_LINE(dest,src)                              \
    if ((dest))                                         \
    {                                                   \
        SvxBorderLine* pCast = (SvxBorderLine*)(dest);  \
        pCast->SetOutWidth((src)->GetOutWidth());       \
        pCast->SetInWidth ((src)->GetInWidth());        \
        pCast->SetDistance((src)->GetDistance());       \
    }

void ScAttrArray::ApplyLineStyleArea( SCROW nStartRow, SCROW nEndRow,
                                      const SvxBorderLine* pLine, sal_Bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos;
        SCROW nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            DBG_ERROR("Search-Fehler");
            return;
        }

        do
        {
            const ScPatternAttr*    pOldPattern = pData[nPos].pPattern;
            const SfxItemSet&       rOldSet = pOldPattern->GetItemSet();
            const SfxPoolItem*      pBoxItem = 0;
            SfxItemState            eState = rOldSet.GetItemState( ATTR_BORDER, sal_True, &pBoxItem );
            const SfxPoolItem*      pTLBRItem = 0;
            SfxItemState            eTLBRState = rOldSet.GetItemState( ATTR_BORDER_TLBR, sal_True, &pTLBRItem );
            const SfxPoolItem*      pBLTRItem = 0;
            SfxItemState            eBLTRState = rOldSet.GetItemState( ATTR_BORDER_BLTR, sal_True, &pBLTRItem );

            if ( (SFX_ITEM_SET == eState) || (SFX_ITEM_SET == eTLBRState) || (SFX_ITEM_SET == eBLTRState) )
            {
                ScPatternAttr*  pNewPattern = new ScPatternAttr(*pOldPattern);
                SfxItemSet&     rNewSet = pNewPattern->GetItemSet();
                SCROW           nY1 = nStart;
                SCROW           nY2 = pData[nPos].nRow;

                SvxBoxItem*     pNewBoxItem = pBoxItem ? (SvxBoxItem*)pBoxItem->Clone() : 0;
                SvxLineItem*    pNewTLBRItem = pTLBRItem ? (SvxLineItem*)pTLBRItem->Clone() : 0;
                SvxLineItem*    pNewBLTRItem = pBLTRItem ? (SvxLineItem*)pBLTRItem->Clone() : 0;

                // Linienattribute holen und mit Parametern aktualisieren

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
                    SetPatternArea( nY1, nY2, pNewPattern, sal_True );
                    Search( nStart, nPos );
                }
                else
                {
                        //! aus Pool loeschen?
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


void ScAttrArray::ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache )
{
#ifdef DBG_UTIL
    TestData();
#endif

    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos;
        SCROW nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            DBG_ERROR("Search-Fehler");
            return;
        }

        ScAddress aAdrStart( nCol, 0, nTab );
        ScAddress aAdrEnd  ( nCol, 0, nTab );

        do
        {
            const ScPatternAttr* pOldPattern = pData[nPos].pPattern;
            const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pCache->ApplyTo( *pOldPattern, sal_True );
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
                    SetPatternArea( nY1, nY2, pNewPattern );
                    Search( nStart, nPos );
                }
                else
                {
                    // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert

                    const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                    const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                    sal_Bool bNumFormatChanged;
                    if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                            rNewSet, rOldSet ) )
                    {
                        aAdrStart.SetRow( nPos ? pData[nPos-1].nRow+1 : 0 );
                        aAdrEnd  .SetRow( pData[nPos].nRow );
                        pDocument->InvalidateTextWidth( &aAdrStart, &aAdrEnd, bNumFormatChanged );
#ifdef DBG_INVALIDATE
                        DBGOUTPUT("ApplyCacheArea");
#endif
                    }

                    // bedingte Formate neu gesetzt oder geloescht ?

                    if ( &rNewSet.Get(ATTR_CONDITIONAL) != &rOldSet.Get(ATTR_CONDITIONAL) )
                    {
                        pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                        rOldSet.Get(ATTR_CONDITIONAL)).GetValue() );
                        pDocument->ConditionalChanged( ((const SfxUInt32Item&)
                                        rNewSet.Get(ATTR_CONDITIONAL)).GetValue() );
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
//!!!!!!!!!!!!!!!!!! mit diesem Remove gibt es Abstuerze (Calc1 Import)
//!             pDocument->GetPool()->Remove(*pNewPattern);
                nStart = pData[nPos].nRow + 1;
                ++nPos;
            }
        }
        while (nStart <= nEndRow);

        if (pDocument->IsStreamValid(nTab))
            pDocument->SetStreamValid(nTab, sal_False);
    }

#ifdef DBG_UTIL
    TestData();
#endif
}


void lcl_MergeDeep( SfxItemSet& rMergeSet, const SfxItemSet& rSource )
{
    const SfxPoolItem* pNewItem;
    const SfxPoolItem* pOldItem;
    for (sal_uInt16 nId=ATTR_PATTERN_START; nId<=ATTR_PATTERN_END; nId++)
    {
        //  pMergeSet hat keinen Parent
        SfxItemState eOldState = rMergeSet.GetItemState( nId, sal_False, &pOldItem );

        if ( eOldState == SFX_ITEM_DEFAULT )                // Default
        {
            SfxItemState eNewState = rSource.GetItemState( nId, sal_True, &pNewItem );
            if ( eNewState == SFX_ITEM_SET )
            {
                if ( *pNewItem != rMergeSet.GetPool()->GetDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        else if ( eOldState == SFX_ITEM_SET )               // Item gesetzt
        {
            SfxItemState eNewState = rSource.GetItemState( nId, sal_True, &pNewItem );
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
        //  Dontcare bleibt Dontcare
    }
}


void ScAttrArray::MergePatternArea( SCROW nStartRow, SCROW nEndRow,
                                    ScMergePatternState& rState, sal_Bool bDeep ) const
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        SCSIZE nPos;
        SCROW nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            DBG_ERROR("Search-Fehler");
            return;
        }

        do
        {
            //  gleiche Patterns muessen nicht mehrfach angesehen werden

            const ScPatternAttr* pPattern = pData[nPos].pPattern;
            if ( pPattern != rState.pOld1 && pPattern != rState.pOld2 )
            {
                const SfxItemSet& rThisSet = pPattern->GetItemSet();
                if (rState.pItemSet)
                {
                    //  (*ppSet)->MergeValues( rThisSet, sal_False );
                    //  geht nicht, weil die Vorlagen nicht beruecksichtigt werden

                    if (bDeep)
                        lcl_MergeDeep( *rState.pItemSet, rThisSet );
                    else
                        rState.pItemSet->MergeValues( rThisSet, sal_False );
                }
                else
                {
                    //  erstes Pattern - in Set ohne Parent kopieren
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



//          Umrandung zusammenbauen

sal_Bool lcl_TestAttr( const SvxBorderLine* pOldLine, const SvxBorderLine* pNewLine,
                            sal_uInt8& rModified, const SvxBorderLine*& rpNew )
{
    if (rModified == SC_LINE_DONTCARE)
        return sal_False;                       // weiter geht's nicht

    if (rModified == SC_LINE_EMPTY)
    {
        rModified = SC_LINE_SET;
        rpNew = pNewLine;
        return sal_True;                        // zum ersten mal gesetzt
    }

    if (pOldLine == pNewLine)
    {
        rpNew = pOldLine;
        return sal_False;
    }

    if (pOldLine && pNewLine)
        if (*pOldLine == *pNewLine)
        {
            rpNew = pOldLine;
            return sal_False;
        }

    rModified = SC_LINE_DONTCARE;
    rpNew = NULL;
    return sal_True;                            // andere Linie -> dontcare
}


void lcl_MergeToFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                ScLineFlags& rFlags, const ScPatternAttr* pPattern,
                                sal_Bool bLeft, SCCOL nDistRight, sal_Bool bTop, SCROW nDistBottom )
{
    //  rechten/unteren Rahmen setzen, wenn Zelle bis zum Ende zusammengefasst:
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
                    SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight ) const
{
    const ScPatternAttr* pPattern;

    if (nStartRow == nEndRow)
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, sal_True, 0 );
    }
    else
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, sal_True,
                            nEndRow-nStartRow );

        SCSIZE nStartIndex;
        SCSIZE nEndIndex;
        Search( nStartRow+1, nStartIndex );
        Search( nEndRow-1, nEndIndex );
        for (SCSIZE i=nStartIndex; i<=nEndIndex; i++)
        {
            pPattern = (ScPatternAttr*) pData[i].pPattern;
            lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, sal_False,
                            nEndRow - Min( pData[i].nRow, (SCROW)(nEndRow-1) ) );
            // nDistBottom hier immer > 0
        }

        pPattern = GetPattern( nEndRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, sal_False, 0 );
    }
}

//
//  Rahmen anwenden
//

//  ApplyFrame - auf einen Eintrag im Array


sal_Bool ScAttrArray::ApplyFrame( const SvxBoxItem*     pBoxItem,
                              const SvxBoxInfoItem* pBoxInfoItem,
                              SCROW nStartRow, SCROW nEndRow,
                              sal_Bool bLeft, SCCOL nDistRight, sal_Bool bTop, SCROW nDistBottom )
{
    DBG_ASSERT( pBoxItem && pBoxInfoItem, "Linienattribute fehlen!" );

    const ScPatternAttr* pPattern = GetPattern( nStartRow );
    const SvxBoxItem* pOldFrame = (const SvxBoxItem*)
                                  &pPattern->GetItemSet().Get( ATTR_BORDER );

    //  rechten/unteren Rahmen setzen, wenn Zelle bis zum Ende zusammengefasst:
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
        return sal_False;
    }
    else
    {
        SfxItemPoolCache aCache( pDocument->GetPool(), &aNewFrame );
        ApplyCacheArea( nStartRow, nEndRow, &aCache );

/*      ScPatternAttr* pNewPattern = (ScPatternAttr*) pPattern->Clone();
        pNewPattern->GetItemSet().Put( aNewFrame );
        SetPatternArea( nStartRow, nEndRow, pNewPattern, sal_True );
*/
        return sal_True;
    }
}


void ScAttrArray::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight )
{
    if (nStartRow == nEndRow)
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, sal_True, 0 );
    else
    {
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nStartRow, bLeft, nDistRight,
                        sal_True, nEndRow-nStartRow );

        if ( nEndRow > nStartRow+1 )                // innerer Teil vorhanden?
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
                sal_Bool bChanged = ApplyFrame( pLineOuter, pLineInner, nTmpStart, nTmpEnd,
                                            bLeft, nDistRight, sal_False, nEndRow-nTmpEnd );
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

        ApplyFrame( pLineOuter, pLineInner, nEndRow, nEndRow, bLeft, nDistRight, sal_False, 0 );
    }
}


long lcl_LineSize( const SvxBorderLine& rLine )
{
    //  nur eine Linie -> halbe Breite, min. 20
    //  doppelte Linie -> halber Abstand + eine Linie (je min. 20)

    long nTotal = 0;
    sal_uInt16 nWidth = Max( rLine.GetOutWidth(), rLine.GetInWidth() );
    sal_uInt16 nDist = rLine.GetDistance();
    if (nDist)
    {
        DBG_ASSERT( rLine.GetOutWidth() && rLine.GetInWidth(),
                        "Linie hat Abstand, aber nur eine Breite ???" );

//      nTotal += ( nDist > 40 ) ? ( nDist / 2 ) : 20;
        nTotal += ( nDist > 20 ) ? nDist : 20;
        nTotal += ( nWidth > 20 ) ? nWidth : 20;
    }
    else if (nWidth)
//      nTotal += ( nWidth > 40 ) ? ( nWidth / 2 ) : 20;
        nTotal += ( nWidth > 20 ) ? nWidth  : 20;

        //! auch halbieren ???

    return nTotal;
}


sal_Bool ScAttrArray::HasLines( SCROW nRow1, SCROW nRow2, Rectangle& rSizes,
                                sal_Bool bLeft, sal_Bool bRight ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nRow1, nStartIndex );
    Search( nRow2, nEndIndex );
    sal_Bool bFound = sal_False;

    const SvxBoxItem* pItem = 0;
    const SvxBorderLine* pLine = 0;
    long nCmp;

    //  oben

    pItem = (const SvxBoxItem*) &pData[nStartIndex].pPattern->GetItem(ATTR_BORDER);
    pLine = pItem->GetTop();
    if (pLine)
    {
        nCmp = lcl_LineSize(*pLine);
        if ( nCmp > rSizes.Top() )
            rSizes.Top() = nCmp;
        bFound = sal_True;
    }

    //  unten

    if ( nEndIndex != nStartIndex )
        pItem = (const SvxBoxItem*) &pData[nEndIndex].pPattern->GetItem(ATTR_BORDER);
    pLine = pItem->GetBottom();
    if (pLine)
    {
        nCmp = lcl_LineSize(*pLine);
        if ( nCmp > rSizes.Bottom() )
            rSizes.Bottom() = nCmp;
        bFound = sal_True;
    }

    if ( bLeft || bRight )
        for ( SCSIZE i=nStartIndex; i<=nEndIndex; i++)
        {
            pItem = (const SvxBoxItem*) &pData[i].pPattern->GetItem(ATTR_BORDER);

            //  links

            if (bLeft)
            {
                pLine = pItem->GetLeft();
                if (pLine)
                {
                    nCmp = lcl_LineSize(*pLine);
                    if ( nCmp > rSizes.Left() )
                        rSizes.Left() = nCmp;
                    bFound = sal_True;
                }
            }

            //  rechts

            if (bRight)
            {
                pLine = pItem->GetRight();
                if (pLine)
                {
                    nCmp = lcl_LineSize(*pLine);
                    if ( nCmp > rSizes.Right() )
                        rSizes.Right() = nCmp;
                    bFound = sal_True;
                }
            }
        }

    return bFound;
}

//  Testen, ob Bereich bestimmtes Attribut enthaelt

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
            const SfxUInt32Item* pConditional =
                    (const SfxUInt32Item*) &pPattern->GetItem( ATTR_CONDITIONAL );
            if ( pConditional->GetValue() != 0 )
                bFound = true;
        }
        if ( nMask & HASATTR_PROTECTED )
        {
            const ScProtectionAttr* pProtect =
                    (const ScProtectionAttr*) &pPattern->GetItem( ATTR_PROTECTION );
            if ( pProtect->GetProtection() || pProtect->GetHideCell() )
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

//  Bereich um evtl. enthaltene Zusammenfassungen erweitern
//  und evtl. MergeFlag anpassen (bRefresh)

sal_Bool ScAttrArray::ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                sal_Bool bRefresh, sal_Bool bAttrs )
{
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;
    Search( nStartRow, nStartIndex );
    Search( nEndRow, nEndIndex );
    sal_Bool bFound = sal_False;

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
            bFound = sal_True;

            if (bAttrs)
            {
                const SvxShadowItem* pShadow =
                        (const SvxShadowItem*) &pPattern->GetItem( ATTR_SHADOW );
                SvxShadowLocation eLoc = pShadow->GetLocation();
                if ( eLoc == SVX_SHADOW_TOPRIGHT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    if ( nMergeEndCol+1 > rPaintCol && nMergeEndCol < MAXCOL )
                        rPaintCol = nMergeEndCol+1;
                if ( eLoc == SVX_SHADOW_BOTTOMLEFT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    if ( nMergeEndRow+1 > rPaintRow && nMergeEndRow < MAXROW )
                        rPaintRow = nMergeEndRow+1;
            }

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

                Search( nThisRow, i );                  // Daten wurden veraendert
                Search( nStartRow, nStartIndex );
                Search( nEndRow, nEndIndex );
            }
        }
    }

    return bFound;
}


sal_Bool ScAttrArray::RemoveAreaMerge(SCROW nStartRow, SCROW nEndRow)
{
    sal_Bool bFound = sal_False;
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

            DBG_ASSERT( nCountY==1 || nThisStart==nThisEnd, "was'n hier los?" );

            SCCOL nThisCol = nCol;
            SCCOL nMergeEndCol = nThisCol + nCountX - 1;
            SCROW nMergeEndRow = nThisEnd + nCountY - 1;

            //! ApplyAttr fuer Bereiche !!!

            for (SCROW nThisRow = nThisStart; nThisRow <= nThisEnd; nThisRow++)
                pDocument->ApplyAttr( nThisCol, nThisRow, nTab, *pAttr );

            ScPatternAttr*  pNewPattern = new ScPatternAttr( pDocument->GetPool() );
            SfxItemSet*     pSet = &pNewPattern->GetItemSet();
            pSet->Put( *pFlagAttr );
            pDocument->ApplyPatternAreaTab( nThisCol, nThisStart, nMergeEndCol, nMergeEndRow,
                                                nTab, *pNewPattern );
            delete pNewPattern;

            Search( nThisEnd, nIndex );                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        if ( nIndex < nCount )
            nThisStart = pData[nIndex-1].nRow+1;
        else
            nThisStart = MAXROW+1;      // Ende
    }

    return bFound;
}

            //      Bereich loeschen, aber Merge-Flags stehenlassen

void ScAttrArray::DeleteAreaSafe(SCROW nStartRow, SCROW nEndRow)
{
    SetPatternAreaSafe( nStartRow, nEndRow, pDocument->GetDefPattern(), sal_True );
}


void ScAttrArray::SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                        const ScPatternAttr* pWantedPattern, sal_Bool bDefault )
{
    const ScPatternAttr*    pOldPattern;
    const ScMergeFlagAttr*  pItem;

    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    sal_Bool    bFirstUse = sal_True;

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
                //  #108045# default-constructing a ScPatternAttr for DeleteArea doesn't work
                //  because it would have no cell style information.
                //  Instead, the document's GetDefPattern is copied. Since it is passed as
                //  pWantedPattern, no special treatment of default is needed here anymore.
                ScPatternAttr*  pNewPattern = new ScPatternAttr( *pWantedPattern );
                SfxItemSet*     pSet = &pNewPattern->GetItemSet();
                pSet->Put( *pItem );
                SetPatternArea( nThisRow, nAttrRow, pNewPattern, sal_True );
                delete pNewPattern;
            }
            else
            {
                if ( !bDefault )
                {
                    if (bFirstUse)
                        bFirstUse = sal_False;
                    else
                        pDocument->GetPool()->Put( *pWantedPattern );       // im Pool ist es schon!
                }
                SetPatternArea( nThisRow, nAttrRow, pWantedPattern );
            }

            Search( nThisRow, nIndex );                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


sal_Bool ScAttrArray::ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    sal_Int16   nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    sal_Bool    bChanged = sal_False;

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
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, sal_True );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
            bChanged = sal_True;
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }

    return bChanged;
}


sal_Bool ScAttrArray::RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    sal_Int16   nOldValue;
    SCSIZE  nIndex;
    SCROW   nRow;
    SCROW   nThisRow;
    sal_Bool    bChanged = sal_False;

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
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, sal_True );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
            bChanged = sal_True;
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
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, sal_True );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


void ScAttrArray::ChangeIndent( SCROW nStartRow, SCROW nEndRow, sal_Bool bIncrement )
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

        sal_Bool bNeedJust = ( rOldSet.GetItemState( ATTR_HOR_JUSTIFY, sal_False, &pItem ) != SFX_ITEM_SET
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
            SetPatternArea( nThisStart, nAttrRow, &aNewPattern, sal_True );

            nThisStart = nThisEnd + 1;
            Search( nThisStart, nIndex );               // Daten wurden veraendert !!!
        }
        else
        {
            nThisStart = pData[nIndex].nRow + 1;        // weiterzaehlen...
            ++nIndex;
        }
    }
}


SCsROW ScAttrArray::GetNextUnprotected( SCsROW nRow, sal_Bool bUp ) const
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
                    return -1;                  // nichts gefunden
                --nIndex;
                nRet = pData[nIndex].nRow;
            }
            else
            {
                nRet = pData[nIndex].nRow+1;
                ++nIndex;
                if (nIndex>=nCount)
                    return MAXROW+1;            // nichts gefunden
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
//          for (SCROW nRow = nStart; nRow <= nEnd; nRow++)
//              pUsed[nRow] = sal_True;

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
                    --nPos;                         // wegen ++ am Ende
                }
            }
        }
        nStart = nEnd + 1;
        ++nPos;
    }
}


sal_Bool ScAttrArray::IsStyleSheetUsed( const ScStyleSheet& rStyle,
        sal_Bool bGatherAllStyles ) const
{
    sal_Bool    bIsUsed = sal_False;
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
                    return sal_True;
                bIsUsed = sal_True;
            }
        }
        nPos++;
    }

    return bIsUsed;
}


sal_Bool ScAttrArray::IsEmpty() const
{
    if (nCount == 1)
    {
        if ( pData[0].pPattern != pDocument->GetDefPattern() )
            return sal_False;
        else
            return sal_True;
    }
    else
        return sal_False;
}


//UNUSED2008-05  SCROW ScAttrArray::GetFirstEntryPos() const
//UNUSED2008-05  {
//UNUSED2008-05      DBG_ASSERT( nCount, "nCount = 0" );
//UNUSED2008-05
//UNUSED2008-05      if ( pData[0].pPattern != pDocument->GetDefPattern() )
//UNUSED2008-05          return 0;
//UNUSED2008-05      else
//UNUSED2008-05      {
//UNUSED2008-05          if (nCount==1)
//UNUSED2008-05              return 0;                               // leer
//UNUSED2008-05          else
//UNUSED2008-05              return pData[0].nRow + 1;
//UNUSED2008-05      }
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  SCROW ScAttrArray::GetLastEntryPos( sal_Bool bIncludeBottom ) const
//UNUSED2008-05  {
//UNUSED2008-05      DBG_ASSERT( nCount, "nCount == 0" );
//UNUSED2008-05
//UNUSED2008-05      if (bIncludeBottom)
//UNUSED2008-05          bIncludeBottom = ( pData[nCount-1].pPattern != pDocument->GetDefPattern() );
//UNUSED2008-05
//UNUSED2008-05      if (bIncludeBottom)
//UNUSED2008-05          return MAXROW;
//UNUSED2008-05      else
//UNUSED2008-05      {
//UNUSED2008-05          if (nCount<=1)
//UNUSED2008-05              return 0;                               // leer
//UNUSED2008-05          else
//UNUSED2008-05              return pData[nCount-2].nRow;
//UNUSED2008-05      }
//UNUSED2008-05  }


sal_Bool ScAttrArray::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    DBG_ASSERT( nCount, "nCount == 0" );

    sal_Bool bFound = sal_False;
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
            bFound = sal_True;
        }
        else
            ++nStart;
    }

    return bFound;
}

// size (rows) of a range of attributes after cell content where the search is stopped
// (more than a default page size, 2*42 because it's as good as any number)

const SCROW SC_VISATTR_STOP = 84;

sal_Bool ScAttrArray::GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData ) const
{
    //  #i30830# changed behavior:
    //  ignore all attributes starting with the first run of SC_VISATTR_STOP equal rows
    //  below the last content cell

    if ( nLastData == MAXROW )
    {
        rLastRow = MAXROW;      // can't look for attributes below MAXROW
        return sal_True;
    }

    sal_Bool bFound = sal_False;

    //  loop backwards from the end instead of using Search, assuming that
    //  there usually aren't many attributes below the last cell

    SCSIZE nPos = nCount;
    while ( nPos > 0 && pData[nPos-1].nRow > nLastData )
    {
        SCSIZE nEndPos = nPos - 1;
        SCSIZE nStartPos = nEndPos;         // find range of visually equal formats
        while ( nStartPos > 0 &&
                pData[nStartPos-1].nRow > nLastData &&
                pData[nStartPos-1].pPattern->IsVisibleEqual(*pData[nStartPos].pPattern) )
            --nStartPos;

        SCROW nAttrStartRow = ( nStartPos > 0 ) ? ( pData[nStartPos-1].nRow + 1 ) : 0;
        if ( nAttrStartRow <= nLastData )
            nAttrStartRow = nLastData + 1;
        SCROW nAttrSize = pData[nEndPos].nRow + 1 - nAttrStartRow;
        if ( nAttrSize >= SC_VISATTR_STOP )
        {
            bFound = sal_False;        // ignore this range and below
        }
        else if ( !bFound && pData[nEndPos].pPattern->IsVisible() )
        {
            rLastRow = pData[nEndPos].nRow;
            bFound = sal_True;
        }

        nPos = nStartPos;           // look further from the top of the range
    }

    return bFound;
}


sal_Bool ScAttrArray::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    SCROW nThisStart = nStartRow;
    sal_Bool bFound = sal_False;
    while ( nIndex < nCount && nThisStart <= nEndRow && !bFound )
    {
        if ( pData[nIndex].pPattern->IsVisible() )
            bFound = sal_True;

        nThisStart = pData[nIndex].nRow + 1;
        ++nIndex;
    }

    return bFound;
}


sal_Bool ScAttrArray::IsVisibleEqual( const ScAttrArray& rOther,
                                    SCROW nStartRow, SCROW nEndRow ) const
{
    sal_Bool bEqual = sal_True;
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


sal_Bool ScAttrArray::IsAllEqual( const ScAttrArray& rOther, SCROW nStartRow, SCROW nEndRow ) const
{
    //! mit IsVisibleEqual zusammenfassen?

    sal_Bool bEqual = sal_True;
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


sal_Bool ScAttrArray::TestInsertCol( SCROW nStartRow, SCROW nEndRow) const
{
    //  horizontal zusammengefasste duerfen nicht herausgeschoben werden
    //  (ob die ganze Zusammenfassung betroffen ist, ist hier nicht zu erkennen)

    sal_Bool bTest = sal_True;
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
                bTest = sal_False;                      // darf nicht herausgeschoben werden
                break;
            }
            if ( pData[nIndex].nRow >= nEndRow )    // Ende des Bereichs
                break;
        }
    }
    return bTest;
}


sal_Bool ScAttrArray::TestInsertRow( SCSIZE nSize ) const
{
    //  wenn die erste herausgeschobene Zeile vertikal ueberlappt ist,
    //  wuerde eine kaputte Zusammenfassung uebrigbleiben

    if (pData)
    {
        //  MAXROW + 1 - nSize  = erste herausgeschobene Zeile

        SCSIZE nFirstLost = nCount-1;
        while ( nFirstLost && pData[nFirstLost-1].nRow >= sal::static_int_cast<SCROW>(MAXROW + 1 - nSize) )
            --nFirstLost;

        if ( ((const ScMergeFlagAttr&)pData[nFirstLost].pPattern->
                            GetItem(ATTR_MERGE_FLAG)).IsVerOverlapped() )
            return sal_False;
    }

    return sal_True;
}


void ScAttrArray::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    if (!pData)
        return;

    SCROW nSearch = nStartRow > 0 ? nStartRow - 1 : 0;      // Vorgaenger erweitern
    SCSIZE nIndex;
    Search( nSearch, nIndex );

    //  ein gesetztes ScMergeAttr darf nicht ausgedehnt werden
    //  (darum hinterher wieder loeschen)

    sal_Bool bDoMerge = ((const ScMergeAttr&) pData[nIndex].pPattern->GetItem(ATTR_MERGE)).IsMerged();

    SCSIZE nRemove = 0;
    SCSIZE i;
    for (i = nIndex; i < nCount-1; i++)
    {
        SCROW nNew = pData[i].nRow + nSize;
        if ( nNew >= MAXROW )                   // Ende erreicht ?
        {
            nNew = MAXROW;
            if (!nRemove)
                nRemove = i+1;                  // folgende loeschen
        }
        pData[i].nRow = nNew;
    }

    //  muessen Eintraege am Ende geloescht werden?

    if (nRemove && nRemove < nCount)
        DeleteRange( nRemove, nCount-1 );

    if (bDoMerge)           // ausgedehntes ScMergeAttr wieder reparieren
    {
            //! ApplyAttr fuer Bereiche !!!

        const SfxPoolItem& rDef = pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
        for (SCSIZE nAdd=0; nAdd<nSize; nAdd++)
            pDocument->ApplyAttr( nCol, nStartRow+nAdd, nTab, rDef );

        //  im eingefuegten Bereich ist nichts zusammengefasst
    }

    // Don't duplicate the merge flags in the inserted row.
    // #i108488# SC_MF_SCENARIO has to be allowed.
    RemoveFlags( nStartRow, nStartRow+nSize-1, SC_MF_HOR | SC_MF_VER | SC_MF_AUTO | SC_MF_BUTTON );
}


void ScAttrArray::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    if (pData)
    {
        sal_Bool bFirst=sal_True;
        SCSIZE nStartIndex = 0;
        SCSIZE nEndIndex = 0;
        SCSIZE i;

        for ( i = 0; i < nCount-1; i++)
            if (pData[i].nRow >= nStartRow && pData[i].nRow <= sal::static_int_cast<SCROW>(nStartRow+nSize-1))
            {
                if (bFirst)
                {
                    nStartIndex = i;
                    bFirst = sal_False;
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

//      unten nicht Default-Pattern nachschieben, um Druckbereiche erkennen zu koennen
//      stattdessen nur Merge-Flags loeschen

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
    RemoveAreaMerge( nStartRow, nEndRow );          // von zusammengefassten auch die Flags loeschen

    if ( !HasAttrib( nStartRow, nEndRow, HASATTR_OVERLAPPED | HASATTR_AUTOFILTER) )
        SetPatternArea( nStartRow, nEndRow, pDocument->GetDefPattern() );
    else
        DeleteAreaSafe( nStartRow, nEndRow );       // Merge-Flags stehenlassen
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

        if ( pOldPattern->GetItemSet().Count() )        // harte Attribute ?
        {
            nRow = pData[nIndex].nRow;
            SCROW nAttrRow = Min( (SCROW)nRow, (SCROW)nEndRow );

            ScPatternAttr aNewPattern(*pOldPattern);
            SfxItemSet& rSet = aNewPattern.GetItemSet();
            for (sal_uInt16 nId = ATTR_PATTERN_START; nId <= ATTR_PATTERN_END; nId++)
                if (nId != ATTR_MERGE && nId != ATTR_MERGE_FLAG)
                    rSet.ClearItem(nId);

            if ( aNewPattern == *pDefPattern )
                SetPatternArea( nThisRow, nAttrRow, pDefPattern, sal_False );
            else
                SetPatternArea( nThisRow, nAttrRow, &aNewPattern, sal_True );

            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}

        // Verschieben innerhalb eines Dokuments

void ScAttrArray::MoveTo(SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray)
{
    SCROW nStart = nStartRow;
    for (SCSIZE i = 0; i < nCount; i++)
    {
        if ((pData[i].nRow >= nStartRow) && ((i==0) ? sal_True : pData[i-1].nRow < nEndRow))
        {
            //  Kopieren (bPutToPool=sal_True)
            rAttrArray.SetPatternArea( nStart, Min( (SCROW)pData[i].nRow, (SCROW)nEndRow ),
                                        pData[i].pPattern, sal_True );
        }
        nStart = Max( (SCROW)nStart, (SCROW)(pData[i].nRow + 1) );
    }
    DeleteArea(nStartRow, nEndRow);
}


        // Kopieren zwischen Dokumenten (Clipboard)

void ScAttrArray::CopyArea( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray,
                                sal_Int16 nStripFlags )
{
    nStartRow -= nDy;       // Source
    nEndRow -= nDy;

    SCROW nDestStart = Max((long)((long)nStartRow + nDy), (long) 0);
    SCROW nDestEnd = Min((long)((long)nEndRow + nDy), (long) MAXROW);

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    sal_Bool bSamePool = (pSourceDocPool==pDestDocPool);

    for (SCSIZE i = 0; (i < nCount) && (nDestStart <= nDestEnd); i++)
    {
        if (pData[i].nRow >= nStartRow)
        {
            const ScPatternAttr* pOldPattern = pData[i].pPattern;
            const ScPatternAttr* pNewPattern;

            if (IsDefaultItem( pOldPattern ))
            {
                //  am Default muss nichts veraendert werden

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

        // when pasting from clipboard and skipping filtered rows, the adjusted end position
        // can be negative
        nDestStart = Max((long)nDestStart, (long)(pData[i].nRow + nDy + 1));
    }
}

        // Flags stehenlassen
        //! mit CopyArea zusammenfassen !!!

void ScAttrArray::CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray )
{
    nStartRow -= nDy;       // Source
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
    sal_Bool bSamePool = (pSourceDocPool==pDestDocPool);

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
                            Min((SCROW)(pData[i].nRow + nDy), nDestEnd), pNewPattern, sal_False);
        }

        // when pasting from clipboard and skipping filtered rows, the adjusted end position
        // can be negative
        nDestStart = Max((long)nDestStart, (long)(pData[i].nRow + nDy + 1));
    }
}


SCsROW ScAttrArray::SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                                    sal_Bool bUp, ScMarkArray* pMarkArray )
{
    sal_Bool bFound = sal_False;

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
                    bFound = sal_True;
            }
            else
                bFound = sal_True;
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

    DBG_ASSERT( bFound || !ValidRow(nRow), "interner Fehler in ScAttrArray::SearchStyle" );

    return nRow;
}


sal_Bool ScAttrArray::SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow,
                        const ScStyleSheet* pSearchStyle, sal_Bool bUp, ScMarkArray* pMarkArray )
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
                SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, sal_True );
                if (nMarkEnd>rEndRow)
                    rEndRow = nMarkEnd;
            }
        }
        else
        {
            rEndRow = pData[nIndex].nRow;
            if (pMarkArray)
            {
                SCROW nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, sal_False );
                if (nMarkEnd<rEndRow)
                    rEndRow = nMarkEnd;
            }
        }

        return sal_True;
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------
//
//                          Laden / Speichern
//


#if 0
void ScAttrArray::Save( SvStream& /* rStream */ ) const
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    ScWriteHeader aHdr( rStream, 8 );

    ScDocumentPool* pDocPool = pDocument->GetPool();

    sal_uInt16 nSaveCount = nCount;
    SCROW nSaveMaxRow = pDocument->GetSrcMaxRow();
    if ( nSaveMaxRow != MAXROW )
    {
        if ( nSaveCount > 1 && pData[nSaveCount-2].nRow >= nSaveMaxRow )
        {
            pDocument->SetLostData();           // Warnung ausgeben
            do
                --nSaveCount;
            while ( nSaveCount > 1 && pData[nSaveCount-2].nRow >= nSaveMaxRow );
        }
    }

    rStream << nSaveCount;

    const SfxPoolItem* pItem;
    for (SCSIZE i=0; i<nSaveCount; i++)
    {
        rStream << Min( pData[i].nRow, nSaveMaxRow );

        const ScPatternAttr* pPattern = pData[i].pPattern;
        pDocPool->StoreSurrogate( rStream, pPattern );

        //  sal_False, weil ATTR_CONDITIONAL (noch) nicht in Vorlagen:
        if (pPattern->GetItemSet().GetItemState(ATTR_CONDITIONAL,sal_False,&pItem) == SFX_ITEM_SET)
            pDocument->SetConditionalUsed( ((const SfxUInt32Item*)pItem)->GetValue() );

        if (pPattern->GetItemSet().GetItemState(ATTR_VALIDDATA,sal_False,&pItem) == SFX_ITEM_SET)
            pDocument->SetValidationUsed( ((const SfxUInt32Item*)pItem)->GetValue() );
    }
#endif // SC_ROWLIMIT_STREAM_ACCESS
}


void ScAttrArray::Load( SvStream& /* rStream */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    ScDocumentPool* pDocPool = pDocument->GetPool();

    ScReadHeader aHdr( rStream );

    sal_uInt16 nNewCount;
    rStream >> nNewCount;
    if ( nNewCount > MAXROW+1 )                     // wuerde das Array zu gross?
    {
        pDocument->SetLostData();
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }

    Reset( pDocument->GetDefPattern(), sal_False );     // loeschen
    pData = new ScAttrEntry[nNewCount];             // neu anlegen
    for (SCSIZE i=0; i<nNewCount; i++)
    {
        rStream >> pData[i].nRow;

        sal_uInt16 nWhich = ATTR_PATTERN;
        const ScPatternAttr* pNewPattern = (const ScPatternAttr*)
                                           pDocPool->LoadSurrogate( rStream, nWhich, ATTR_PATTERN );
        if (!pNewPattern)
        {
            // da is was schiefgelaufen
            DBG_ERROR("ScAttrArray::Load: Surrogat nicht im Pool");
            pNewPattern = pDocument->GetDefPattern();
        }
        ScDocumentPool::CheckRef( *pNewPattern );
        pData[i].pPattern = pNewPattern;

        // LoadSurrogate erhoeht auch die Ref
    }
    nCount = nLimit = nNewCount;

    if ( nCount > 1 && pData[nCount-2].nRow >= MAXROW ) // faengt ein Attribut hinter MAXROW an?
    {
        pDocument->SetLostData();
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }

    if ( pDocument->GetSrcMaxRow() != MAXROW )          // Ende anpassen?
    {
        //  Ende immer auf MAXROW umsetzen (nur auf 32 Bit)

        DBG_ASSERT( pData[nCount-1].nRow == pDocument->GetSrcMaxRow(), "Attribut-Ende ?!?" );
        pData[nCount-1].nRow = MAXROW;
    }
#endif // SC_ROWLIMIT_STREAM_ACCESS
}
#endif


//UNUSED2008-05  void ScAttrArray::ConvertFontsAfterLoad()
//UNUSED2008-05  {
//UNUSED2008-05      ScFontToSubsFontConverter_AutoPtr xFontConverter;
//UNUSED2008-05      const sal_uLong nFlags = FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
//UNUSED2008-05      SCSIZE   nIndex = 0;
//UNUSED2008-05      SCROW  nThisRow = 0;
//UNUSED2008-05
//UNUSED2008-05      while ( nThisRow <= MAXROW )
//UNUSED2008-05      {
//UNUSED2008-05          const ScPatternAttr* pOldPattern = pData[nIndex].pPattern;
//UNUSED2008-05          const SfxPoolItem* pItem;
//UNUSED2008-05          if( pOldPattern->GetItemSet().GetItemState( ATTR_FONT, sal_False, &pItem ) == SFX_ITEM_SET )
//UNUSED2008-05          {
//UNUSED2008-05              const SvxFontItem* pFontItem = (const SvxFontItem*) pItem;
//UNUSED2008-05              const String& rOldName = pFontItem->GetFamilyName();
//UNUSED2008-05              xFontConverter = CreateFontToSubsFontConverter( rOldName, nFlags );
//UNUSED2008-05              if ( xFontConverter )
//UNUSED2008-05              {
//UNUSED2008-05                  String aNewName( GetFontToSubsFontName( xFontConverter ) );
//UNUSED2008-05                  if ( aNewName != rOldName )
//UNUSED2008-05                  {
//UNUSED2008-05                      SCROW nAttrRow = pData[nIndex].nRow;
//UNUSED2008-05                      SvxFontItem aNewItem( pFontItem->GetFamily(), aNewName,
//UNUSED2008-05                          pFontItem->GetStyleName(), pFontItem->GetPitch(),
//UNUSED2008-05                          RTL_TEXTENCODING_DONTKNOW, ATTR_FONT );
//UNUSED2008-05                      ScPatternAttr aNewPattern( *pOldPattern );
//UNUSED2008-05                      aNewPattern.GetItemSet().Put( aNewItem );
//UNUSED2008-05                      SetPatternArea( nThisRow, nAttrRow, &aNewPattern, sal_True );
//UNUSED2008-05                      Search( nThisRow, nIndex );     //! data changed
//UNUSED2008-05                  }
//UNUSED2008-05              }
//UNUSED2008-05          }
//UNUSED2008-05          ++nIndex;
//UNUSED2008-05          nThisRow = pData[nIndex-1].nRow+1;
//UNUSED2008-05      }
//UNUSED2008-05  }

