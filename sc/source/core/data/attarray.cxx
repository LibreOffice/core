/*************************************************************************
 *
 *  $RCSfile: attarray.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-12 08:49:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/shaditem.hxx>
#include <svtools/poolcach.hxx>

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


#undef DBG_INVALIDATE
#define DBGOUTPUT(s) \
    DBG_ERROR( String("Invalidate ") + String(s) + String(": ") \
               + String(nCol) + String('/') + String(aAdrStart.Row()) + String('/') + String(nTab) \
               + String(" bis ") \
               + String(nCol) + String('/') + String(aAdrEnd.Row())   + String('/') + String(nTab) \
              );

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

ScAttrArray::ScAttrArray( USHORT nNewCol, USHORT nNewTab, ScDocument* pDoc ) :
    nCol( nNewCol ),
    nTab( nNewTab ),
    pDocument( pDoc )
{
    ScDocumentPool* pDocPool = pDocument->GetPool();

    nCount = 1;
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
        for (USHORT i=0; i<nCount; i++)
            pDocPool->Remove(*pData[i].pPattern);

        delete[] pData;
    }
}

//------------------------------------------------------------------------

void ScAttrArray::TestData() const
{
#ifdef DBG_UTIL
    USHORT nErr = 0;
    if (pData)
    {
        for (USHORT nPos=0; nPos<nCount; nPos++)
        {
            if (nPos)
                if (pData[nPos].pPattern == pData[nPos-1].pPattern || pData[nPos].nRow <= pData[nPos-1].nRow)
                    ++nErr;
            if (pData[nPos].pPattern->Which() != ATTR_PATTERN)
                ++nErr;
        }
    }
    if (nErr)
    {
        ByteString aMsg = ByteString::CreateFromInt32(nErr);
        aMsg += " errors in attribute array, column ";
        aMsg += ByteString::CreateFromInt32(nCol);
        DBG_ERROR( aMsg.GetBuffer() );
    }
#endif
}

//------------------------------------------------------------------------

void ScAttrArray::Reset( const ScPatternAttr* pPattern, BOOL bAlloc )
{
    if (pData)
    {
        ScDocumentPool*      pDocPool = pDocument->GetPool();
        const ScPatternAttr* pOldPattern;
        ScAddress            aAdrStart( nCol, 0, nTab );
        ScAddress            aAdrEnd  ( nCol, 0, nTab );

        for (USHORT i=0; i<nCount; i++)
        {
            // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
            pOldPattern = pData[i].pPattern;
            BOOL bNumFormatChanged;
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

        if (bAlloc)
        {
            nCount = 1;
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
            nCount = 0;
            pData = NULL;               // muss sofort wieder belegt werden !
        }
    }
}


BOOL ScAttrArray::Concat(USHORT nPos)
{
    BOOL bRet = FALSE;
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
                bRet = TRUE;
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
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

//------------------------------------------------------------------------

BOOL ScAttrArray::Search( USHORT nRow, short& nIndex ) const
{
    short   nLo         = 0;
    short   nHi         = ((short) nCount) - 1;
    short   nStartRow   = 0;
    short   nEndRow     = 0;
    short   i           = 0;
    BOOL    bFound      = (nCount == 1);
    if (pData)
    {
        while ( !bFound && nLo <= nHi )
        {
            i = (nLo + nHi) / 2;
            if (i > 0)
                nStartRow = (short) pData[i - 1].nRow;
            else
                nStartRow = -1;
            nEndRow = (short) pData[i].nRow;
            if (nEndRow < (short) nRow)
                nLo = ++i;
            else
                if (nStartRow >= (short) nRow)
                    nHi = --i;
                else
                    bFound = TRUE;
        }
    }
    else
        bFound = FALSE;

    if (bFound)
        nIndex=i;
    else
        nIndex=0;
    return bFound;
}


const ScPatternAttr* ScAttrArray::GetPattern( USHORT nRow ) const
{
    short i;
    if (Search( nRow, i ))
        return pData[i].pPattern;
    else
        return NULL;
}


const ScPatternAttr* ScAttrArray::GetPatternRange( USHORT& rStartRow,
        USHORT& rEndRow, USHORT nRow ) const
{
    short nIndex;
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

void ScAttrArray::SetPattern( USHORT nRow, const ScPatternAttr* pPattern, BOOL bPutToPool )
{
    SetPatternArea( nRow, nRow, pPattern, bPutToPool );
}


void ScAttrArray::SetPatternArea(USHORT nStartRow, USHORT nEndRow, const ScPatternAttr *pPattern, BOOL bPutToPool )
{
    if (nStartRow >= 0 && nStartRow <= MAXROW && nEndRow >= 0 && nEndRow <= MAXROW)
    {
        if (bPutToPool)
            pPattern = (const ScPatternAttr*) &pDocument->GetPool()->Put(*pPattern);

        if ((nStartRow == 0) && (nEndRow == MAXROW))
            Reset(pPattern);
        else
        {
            USHORT nNewCount = 0;
            ScAttrEntry* pNewData = new ScAttrEntry[nCount + 2];

            if (pNewData)
            {
                ScAddress       aAdrStart( nCol, 0, nTab );
                ScAddress       aAdrEnd  ( nCol, 0, nTab );
                ScPatternAttr*  pOldPattern = NULL;

                USHORT ni = 0;
                USHORT nx = 0;
                USHORT ns = 0;
                if (nStartRow > 0)
                {
                    // Anfangsbereich kopieren
                    short nIndex;
                    Search( nStartRow, nIndex );
                    ni = nNewCount = nIndex;
                    memcpy( pNewData, pData, ni*sizeof(ScAttrEntry) );

                    if ( ni )
                    {
                        nx = ni;
                        ns = pData[ni-1].nRow+1;
                    }

                    // erzeugen des neuen Daten-Array fortsetzen

                    if ((pData[ni].pPattern != pPattern) &&
                        ((ni==0) ? TRUE : (pData[ni - 1].nRow < nStartRow - 1)))
                    {
                        // Eintrag splitten
                        pNewData[nNewCount].nRow = nStartRow - 1;
                        pNewData[nNewCount].pPattern = pData[ni].pPattern;
                        nNewCount++;
                        ni++;
                    }
                    if (ni > 0)
                        if (pData[ni-1].pPattern == pPattern)       // zusammenfassen
                        {
                            ni--;
                            nNewCount--;
                        }
                } // if StartRow > 0

                // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
                // oder bedingte Formate neu gesetzt oder geloescht werden
                while ( ns <= nEndRow )
                {
                    const SfxItemSet& rNewSet = pPattern->GetItemSet();
                    const SfxItemSet& rOldSet = pData[nx].pPattern->GetItemSet();

                    BOOL bNumFormatChanged;
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


                // Bereich setzen
                pNewData[nNewCount].nRow = nEndRow;
                pNewData[nNewCount].pPattern = pPattern;
                nNewCount++;

                USHORT nj = 0;
                if (nEndRow < MAXROW)
                {
                    // mittleren Bereich ueberspringen
                    while (pData[nj].nRow <= nEndRow) nj++;
                    if (pData[nj].pPattern == pPattern)
                    {
                        // Eintrag zusammenfassen
                        pNewData[nNewCount - 1].nRow = pData[nj].nRow;
                        nj++;
                    }
                    ScDocumentPool* pDocPool = pDocument->GetPool();
                    if (nj<ni)
                    {
                        //      gesplitteten Eintrag im Pool verdoppeln
                        pDocPool->Put(*pData[ni-1].pPattern);

                    }
                    // Eintraege aus Pool loeschen
                    for (USHORT nk=ni; nk<nj; nk++)
                    {
                        pDocPool->Remove(*pData[nk].pPattern);
                    }
                    // Den Endbereich kopieren
                    while (nj < nCount)
                    {
                        pNewData[nNewCount].nRow = pData[nj].nRow;
                        pNewData[nNewCount].pPattern = pData[nj].pPattern;
                        nNewCount++;
                        nj++;
                    }
                } // if EndRow < MaxRow
                // Zeiger umsetzen
                delete[] pData;
                nCount = nNewCount;
                pData = pNewData;
            }
        }
    }
//  InfoBox(0, String(nCount) + String(" Eintraege") ).Execute();

#ifdef DBG_UTIL
    TestData();
#endif
}


void ScAttrArray::ApplyStyleArea( USHORT nStartRow, USHORT nEndRow, ScStyleSheet* pStyle )
{
    if (nStartRow >= 0 && nStartRow <= MAXROW && nEndRow >= 0 && nEndRow <= MAXROW)
    {
        short nPos;
        USHORT nStart=0;
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
            USHORT nY1 = nStart;
            USHORT nY2 = pData[nPos].nRow;
            nStart = pData[nPos].nRow + 1;

            if ( nY1 < nStartRow || nY2 > nEndRow )
            {
                if (nY1 < nStartRow) nY1=nStartRow;
                if (nY2 > nEndRow) nY2=nEndRow;
                SetPatternArea( nY1, nY2, pNewPattern, TRUE );
                Search( nStart, nPos );
            }
            else
            {
                // ueberpruefen, ob Attributierung die Textbreite der Zelle aendert
                // bedingte Formate in Vorlagen gibt es (noch) nicht

                const SfxItemSet& rNewSet = pNewPattern->GetItemSet();
                const SfxItemSet& rOldSet = pOldPattern->GetItemSet();

                BOOL bNumFormatChanged;
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
        while ((nStart <= nEndRow) && (nPos < (short)nCount));
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

void ScAttrArray::ApplyLineStyleArea( USHORT nStartRow, USHORT nEndRow,
                                      const SvxBorderLine* pLine, BOOL bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    if (nStartRow >= 0 && nStartRow <= MAXROW && nEndRow >= 0 && nEndRow <= MAXROW)
    {
        short nPos;
        USHORT nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            DBG_ERROR("Search-Fehler");
            return;
        }

        do
        {
            const ScPatternAttr*    pOldPattern = pData[nPos].pPattern;
            const SfxPoolItem*      pItem = NULL;

            if ( SFX_ITEM_SET == pOldPattern->GetItemSet().
                                    GetItemState( ATTR_BORDER, TRUE, &pItem ) )
            {
                ScPatternAttr*  pNewPattern = new ScPatternAttr(*pOldPattern);
                SvxBoxItem      aBoxItem( *(const SvxBoxItem*)pItem );
                USHORT          nY1 = nStart;
                USHORT          nY2 = pData[nPos].nRow;

                // Linienattribute holen und mit Parametern aktualisieren

                if ( !pLine )
                {
                    if ( aBoxItem.GetTop() )    aBoxItem.SetLine( NULL, BOX_LINE_TOP );
                    if ( aBoxItem.GetBottom() ) aBoxItem.SetLine( NULL, BOX_LINE_BOTTOM );
                    if ( aBoxItem.GetLeft() )   aBoxItem.SetLine( NULL, BOX_LINE_LEFT );
                    if ( aBoxItem.GetRight() )  aBoxItem.SetLine( NULL, BOX_LINE_RIGHT );
                }
                else
                {
                    if ( bColorOnly )
                    {
                        Color aColor( pLine->GetColor() );
                        SET_LINECOLOR( aBoxItem.GetTop(),    aColor );
                        SET_LINECOLOR( aBoxItem.GetBottom(), aColor );
                        SET_LINECOLOR( aBoxItem.GetLeft(),   aColor );
                        SET_LINECOLOR( aBoxItem.GetRight(),  aColor );
                    }
                    else
                    {
                        SET_LINE( aBoxItem.GetTop(),    pLine );
                        SET_LINE( aBoxItem.GetBottom(), pLine );
                        SET_LINE( aBoxItem.GetLeft(),   pLine );
                        SET_LINE( aBoxItem.GetRight(),  pLine );
                    }
                }
                pNewPattern->GetItemSet().Put( aBoxItem );

                nStart = pData[nPos].nRow + 1;

                if ( nY1 < nStartRow || nY2 > nEndRow )
                {
                    if (nY1 < nStartRow) nY1=nStartRow;
                    if (nY2 > nEndRow) nY2=nEndRow;
                    SetPatternArea( nY1, nY2, pNewPattern, TRUE );
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
                delete pNewPattern;
            }
            else
            {
                nStart = pData[nPos].nRow + 1;
                nPos++;
            }
        }
        while ((nStart <= nEndRow) && (nPos < (short)nCount));
    }
}

#undef SET_LINECOLOR
#undef SET_LINE


void ScAttrArray::ApplyCacheArea( USHORT nStartRow, USHORT nEndRow, SfxItemPoolCache* pCache )
{
#ifdef DBG_UTIL
    TestData();
#endif

    if (nStartRow >= 0 && nStartRow <= MAXROW && nEndRow >= 0 && nEndRow <= MAXROW)
    {
        short nPos;
        USHORT nStart=0;
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
            const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pCache->ApplyTo( *pOldPattern, TRUE );
            ScDocumentPool::CheckRef( *pOldPattern );
            ScDocumentPool::CheckRef( *pNewPattern );
            if (pNewPattern != pOldPattern)
            {
                USHORT nY1 = nStart;
                USHORT nY2 = pData[nPos].nRow;
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

                    BOOL bNumFormatChanged;
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
    }

#ifdef DBG_UTIL
    TestData();
#endif
}


void lcl_MergeDeep( SfxItemSet& rMergeSet, const SfxItemSet& rSource )
{
    const SfxPoolItem* pNewItem;
    const SfxPoolItem* pOldItem;
    for (USHORT nId=ATTR_PATTERN_START; nId<=ATTR_PATTERN_END; nId++)
    {
        //  pMergeSet hat keinen Parent
        SfxItemState eOldState = rMergeSet.GetItemState( nId, FALSE, &pOldItem );

        if ( eOldState == SFX_ITEM_DEFAULT )                // Default
        {
            SfxItemState eNewState = rSource.GetItemState( nId, TRUE, &pNewItem );
            if ( eNewState == SFX_ITEM_SET )
            {
                if ( *pNewItem != rMergeSet.GetPool()->GetDefaultItem(nId) )
                    rMergeSet.InvalidateItem( nId );
            }
        }
        else if ( eOldState == SFX_ITEM_SET )               // Item gesetzt
        {
            SfxItemState eNewState = rSource.GetItemState( nId, TRUE, &pNewItem );
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


void ScAttrArray::MergePatternArea( USHORT nStartRow, USHORT nEndRow,
                                    SfxItemSet** ppSet, BOOL bDeep ) const
{
    if (nStartRow >= 0 && nStartRow <= MAXROW && nEndRow >= 0 && nEndRow <= MAXROW)
    {
        const ScPatternAttr* pOld1 = NULL;
        const ScPatternAttr* pOld2 = NULL;

        short nPos;
        USHORT nStart=0;
        if (!Search( nStartRow, nPos ))
        {
            DBG_ERROR("Search-Fehler");
            return;
        }

        do
        {
            //  gleiche Patterns muessen nicht mehrfach angesehen werden

            const ScPatternAttr* pPattern = pData[nPos].pPattern;
            if ( pPattern != pOld1 && pPattern != pOld2 )
            {
                const SfxItemSet& rThisSet = pPattern->GetItemSet();
                if (*ppSet)
                {
                    //  (*ppSet)->MergeValues( rThisSet, FALSE );
                    //  geht nicht, weil die Vorlagen nicht beruecksichtigt werden

                    if (bDeep)
                        lcl_MergeDeep( **ppSet, rThisSet );
                    else
                        (*ppSet)->MergeValues( rThisSet, FALSE );
                }
                else
                {
                    //  erstes Pattern - in Set ohne Parent kopieren
                    *ppSet = new SfxItemSet( *rThisSet.GetPool(), rThisSet.GetRanges() );
                    (*ppSet)->Set( rThisSet, bDeep );
                }

                pOld2 = pOld1;
                pOld1 = pPattern;
            }

            nStart = pData[nPos].nRow + 1;
            ++nPos;
        }
        while (nStart <= nEndRow);
    }
}



//          Umrandung zusammenbauen

BOOL lcl_TestAttr( const SvxBorderLine* pOldLine, const SvxBorderLine* pNewLine,
                            BYTE& rModified, const SvxBorderLine*& rpNew )
{
    if (rModified == SC_LINE_DONTCARE)
        return FALSE;                       // weiter geht's nicht

    if (rModified == SC_LINE_EMPTY)
    {
        rModified = SC_LINE_SET;
        rpNew = pNewLine;
        return TRUE;                        // zum ersten mal gesetzt
    }

    if (pOldLine == pNewLine)
    {
        rpNew = pOldLine;
        return FALSE;
    }

    if (pOldLine && pNewLine)
        if (*pOldLine == *pNewLine)
        {
            rpNew = pOldLine;
            return FALSE;
        }

    rModified = SC_LINE_DONTCARE;
    rpNew = NULL;
    return TRUE;                            // andere Linie -> dontcare
}


void lcl_MergeToFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                                ScLineFlags& rFlags, const ScPatternAttr* pPattern,
                                BOOL bLeft, USHORT nDistRight, BOOL bTop, USHORT nDistBottom )
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
                    USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight ) const
{
    const ScPatternAttr* pPattern;

    if (nStartRow == nEndRow)
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, TRUE, 0 );
    }
    else
    {
        pPattern = GetPattern( nStartRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, TRUE,
                            nEndRow-nStartRow );

        short nStartIndex;
        short nEndIndex;
        Search( nStartRow+1, nStartIndex );
        Search( nEndRow-1, nEndIndex );
        for (short i=nStartIndex; i<=nEndIndex; i++)
        {
            pPattern = (ScPatternAttr*) pData[i].pPattern;
            lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, FALSE,
                            nEndRow - Min( pData[i].nRow, (USHORT)(nEndRow-1) ) );
            // nDistBottom hier immer > 0
        }

        pPattern = GetPattern( nEndRow );
        lcl_MergeToFrame( pLineOuter, pLineInner, rFlags, pPattern, bLeft, nDistRight, FALSE, 0 );
    }
}

//
//  Rahmen anwenden
//

//  ApplyFrame - auf einen Eintrag im Array


BOOL ScAttrArray::ApplyFrame( const SvxBoxItem*     pBoxItem,
                              const SvxBoxInfoItem* pBoxInfoItem,
                              USHORT nStartRow, USHORT nEndRow,
                              BOOL bLeft, USHORT nDistRight, BOOL bTop, USHORT nDistBottom )
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
        return FALSE;
    }
    else
    {
        SfxItemPoolCache aCache( pDocument->GetPool(), &aNewFrame );
        ApplyCacheArea( nStartRow, nEndRow, &aCache );

/*      ScPatternAttr* pNewPattern = (ScPatternAttr*) pPattern->Clone();
        pNewPattern->GetItemSet().Put( aNewFrame );
        SetPatternArea( nStartRow, nEndRow, pNewPattern, TRUE );
*/
        return TRUE;
    }
}


void ScAttrArray::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight )
{
    if (nStartRow == nEndRow)
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight, TRUE, 0 );
    else
    {
        ApplyFrame( pLineOuter, pLineInner, nStartRow, nStartRow, bLeft, nDistRight,
                        TRUE, nEndRow-nStartRow );

        if ( nEndRow > nStartRow+1 )                // innerer Teil vorhanden?
        {
            short nStartIndex;
            short nEndIndex;
            Search( nStartRow+1, nStartIndex );
            Search( nEndRow-1, nEndIndex );
            USHORT nTmpStart = nStartRow+1;
            USHORT nTmpEnd;
            for (short i=nStartIndex; i<=nEndIndex;)
            {
                nTmpEnd = Min( (USHORT)(nEndRow-1), (USHORT)(pData[i].nRow) );
                BOOL bChanged = ApplyFrame( pLineOuter, pLineInner, nTmpStart, nTmpEnd,
                                            bLeft, nDistRight, FALSE, nEndRow-nTmpEnd );
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

        ApplyFrame( pLineOuter, pLineInner, nEndRow, nEndRow, bLeft, nDistRight, FALSE, 0 );
    }
}


long lcl_LineSize( const SvxBorderLine& rLine )
{
    //  nur eine Linie -> halbe Breite, min. 20
    //  doppelte Linie -> halber Abstand + eine Linie (je min. 20)

    long nTotal = 0;
    USHORT nWidth = Max( rLine.GetOutWidth(), rLine.GetInWidth() );
    USHORT nDist = rLine.GetDistance();
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


BOOL ScAttrArray::HasLines( USHORT nRow1, USHORT nRow2, Rectangle& rSizes,
                                BOOL bLeft, BOOL bRight ) const
{
    short nStartIndex;
    short nEndIndex;
    Search( nRow1, nStartIndex );
    Search( nRow2, nEndIndex );
    BOOL bFound = FALSE;

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
        bFound = TRUE;
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
        bFound = TRUE;
    }

    if ( bLeft || bRight )
        for ( USHORT i=nStartIndex; i<=nEndIndex; i++)
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
                    bFound = TRUE;
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
                    bFound = TRUE;
                }
            }
        }

    return bFound;
}

//  Testen, ob Bereich bestimmtes Attribut enthaelt

BOOL ScAttrArray::HasAttrib( USHORT nRow1, USHORT nRow2, USHORT nMask ) const
{
    short nStartIndex;
    short nEndIndex;
    Search( nRow1, nStartIndex );
    Search( nRow2, nEndIndex );
    BOOL bFound = FALSE;

    for (short i=nStartIndex; i<=nEndIndex && !bFound; i++)
    {
        const ScPatternAttr* pPattern = pData[i].pPattern;
        if ( nMask & HASATTR_MERGED )
        {
            const ScMergeAttr* pMerge =
                    (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
            if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
                bFound = TRUE;
        }
        if ( nMask & ( HASATTR_OVERLAPPED | HASATTR_NOTOVERLAPPED | HASATTR_AUTOFILTER ) )
        {
            const ScMergeFlagAttr* pMergeFlag =
                    (const ScMergeFlagAttr*) &pPattern->GetItem( ATTR_MERGE_FLAG );
            if ( (nMask & HASATTR_OVERLAPPED) && pMergeFlag->IsOverlapped() )
                bFound = TRUE;
            if ( (nMask & HASATTR_NOTOVERLAPPED) && !pMergeFlag->IsOverlapped() )
                bFound = TRUE;
            if ( (nMask & HASATTR_AUTOFILTER) && pMergeFlag->HasAutoFilter() )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_LINES )
        {
            const SvxBoxItem* pBox =
                    (const SvxBoxItem*) &pPattern->GetItem( ATTR_BORDER );
            if ( pBox->GetLeft() || pBox->GetRight() || pBox->GetTop() || pBox->GetBottom() )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_SHADOW )
        {
            const SvxShadowItem* pShadow =
                    (const SvxShadowItem*) &pPattern->GetItem( ATTR_SHADOW );
            if ( pShadow->GetLocation() != SVX_SHADOW_NONE )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_CONDITIONAL )
        {
            const SfxUInt32Item* pConditional =
                    (const SfxUInt32Item*) &pPattern->GetItem( ATTR_CONDITIONAL );
            if ( pConditional->GetValue() != 0 )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_PROTECTED )
        {
            const ScProtectionAttr* pProtect =
                    (const ScProtectionAttr*) &pPattern->GetItem( ATTR_PROTECTION );
            if ( pProtect->GetProtection() || pProtect->GetHideCell() )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_ROTATE )
        {
            const SfxInt32Item* pRotate =
                    (const SfxInt32Item*) &pPattern->GetItem( ATTR_ROTATE_VALUE );
            if ( pRotate->GetValue() != 0 )
                bFound = TRUE;
        }
        if ( nMask & HASATTR_NEEDHEIGHT )
        {
            SvxCellOrientation eOrient = (SvxCellOrientation)
                ((const SvxOrientationItem&)pPattern->GetItem( ATTR_ORIENTATION )).GetValue();
            if (eOrient != SVX_ORIENTATION_STANDARD)
                bFound = TRUE;
            else if (((const SfxBoolItem&)pPattern->GetItem( ATTR_LINEBREAK )).GetValue())
                bFound = TRUE;
            else if ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                        GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_BLOCK)
                bFound = TRUE;
            else if (((const SfxUInt32Item&)pPattern->GetItem( ATTR_CONDITIONAL )).GetValue())
                bFound = TRUE;
            else if (((const SfxInt32Item&)pPattern->GetItem( ATTR_ROTATE_VALUE )).GetValue())
                bFound = TRUE;
        }
        if ( nMask & ( HASATTR_SHADOW_RIGHT | HASATTR_SHADOW_DOWN ) )
        {
            const SvxShadowItem* pShadow =
                    (const SvxShadowItem*) &pPattern->GetItem( ATTR_SHADOW );
            SvxShadowLocation eLoc = pShadow->GetLocation();
            if ( nMask & HASATTR_SHADOW_RIGHT )
                if ( eLoc == SVX_SHADOW_TOPRIGHT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    bFound = TRUE;
            if ( nMask & HASATTR_SHADOW_DOWN )
                if ( eLoc == SVX_SHADOW_BOTTOMLEFT || eLoc == SVX_SHADOW_BOTTOMRIGHT )
                    bFound = TRUE;
        }
    }

    return bFound;
}

//  Bereich um evtl. enthaltene Zusammenfassungen erweitern
//  und evtl. MergeFlag anpassen (bRefresh)

BOOL ScAttrArray::ExtendMerge( USHORT nThisCol, USHORT nStartRow, USHORT nEndRow,
                                USHORT& rPaintCol, USHORT& rPaintRow,
                                BOOL bRefresh, BOOL bAttrs )
{
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    short nStartIndex;
    short nEndIndex;
    Search( nStartRow, nStartIndex );
    Search( nEndRow, nEndIndex );
    BOOL bFound = FALSE;

    for (short i=nStartIndex; i<=nEndIndex; i++)
    {
        pPattern = pData[i].pPattern;
        pItem = (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
        INT16   nCountX = pItem->GetColMerge();
        INT16   nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            USHORT nThisRow = (i>0) ? pData[i-1].nRow+1 : 0;
            USHORT nMergeEndCol = nThisCol + nCountX - 1;
            USHORT nMergeEndRow = nThisRow + nCountY - 1;
            if (nMergeEndCol > rPaintCol && nMergeEndCol <= MAXCOL)
                rPaintCol = nMergeEndCol;
            if (nMergeEndRow > rPaintRow && nMergeEndRow <= MAXROW)
                rPaintRow = nMergeEndRow;
            bFound = TRUE;

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


BOOL ScAttrArray::RemoveAreaMerge(USHORT nStartRow, USHORT nEndRow)
{
    BOOL bFound = FALSE;
    const ScPatternAttr* pPattern;
    const ScMergeAttr* pItem;
    short nIndex;

    Search( nStartRow, nIndex );
    USHORT nThisStart = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisStart < nStartRow)
        nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        USHORT nThisEnd = pData[nIndex].nRow;
        if (nThisEnd > nEndRow)
            nThisEnd = nEndRow;

        pPattern = pData[nIndex].pPattern;
        pItem = (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
        INT16   nCountX = pItem->GetColMerge();
        INT16   nCountY = pItem->GetRowMerge();
        if (nCountX>1 || nCountY>1)
        {
            const ScMergeAttr* pAttr = (const ScMergeAttr*)
                                            &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE );
            const ScMergeFlagAttr* pFlagAttr = (const ScMergeFlagAttr*)
                                            &pDocument->GetPool()->GetDefaultItem( ATTR_MERGE_FLAG );

            DBG_ASSERT( nCountY==1 || nThisStart==nThisEnd, "was'n hier los?" );

            USHORT nThisCol = nCol;
            USHORT nMergeEndCol = nThisCol + nCountX - 1;
            USHORT nMergeEndRow = nThisEnd + nCountY - 1;

            //! ApplyAttr fuer Bereiche !!!

            for (USHORT nThisRow = nThisStart; nThisRow <= nThisEnd; nThisRow++)
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
        if ( nIndex < (short) nCount )
            nThisStart = pData[nIndex-1].nRow+1;
        else
            nThisStart = MAXROW+1;      // Ende
    }

    return bFound;
}

            //      Bereich loeschen, aber Merge-Flags stehenlassen

void ScAttrArray::DeleteAreaSafe(USHORT nStartRow, USHORT nEndRow)
{
    SetPatternAreaSafe( nStartRow, nEndRow, pDocument->GetDefPattern(), TRUE );
}


void ScAttrArray::SetPatternAreaSafe( USHORT nStartRow, USHORT nEndRow,
                        const ScPatternAttr* pWantedPattern, BOOL bDefault )
{
    const ScPatternAttr*    pOldPattern;
    const ScMergeFlagAttr*  pItem;

    short   nIndex;
    USHORT  nRow;
    USHORT  nThisRow;
    BOOL    bFirstUse = TRUE;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;
        if (pOldPattern != pWantedPattern)                          //! else-Zweig ?
        {
            if (nThisRow < nStartRow) nThisRow = nStartRow;
            nRow = pData[nIndex].nRow;
            USHORT nAttrRow = Min( (USHORT)nRow, (USHORT)nEndRow );
            pItem = (const ScMergeFlagAttr*) &pOldPattern->GetItem( ATTR_MERGE_FLAG );

            if (pItem->IsOverlapped() || pItem->HasAutoFilter())
            {
                ScPatternAttr*  pNewPattern;
                if (bDefault)
                    pNewPattern = new ScPatternAttr( pDocument->GetPool() );
                else
                    pNewPattern = new ScPatternAttr( *pWantedPattern );
                SfxItemSet*     pSet = &pNewPattern->GetItemSet();
                pSet->Put( *pItem );
                SetPatternArea( nThisRow, nAttrRow, pNewPattern, TRUE );
                delete pNewPattern;
            }
            else
            {
                if ( !bDefault )
                {
                    if (bFirstUse)
                        bFirstUse = FALSE;
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


BOOL ScAttrArray::ApplyFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    INT16   nOldValue;
    short   nIndex;
    USHORT  nRow;
    USHORT  nThisRow;
    BOOL    bChanged = FALSE;

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
            USHORT nAttrRow = Min( (USHORT)nRow, (USHORT)nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue | nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, TRUE );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
            bChanged = TRUE;
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }

    return bChanged;
}


BOOL ScAttrArray::RemoveFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
{
    const ScPatternAttr* pOldPattern;

    INT16   nOldValue;
    short   nIndex;
    USHORT  nRow;
    USHORT  nThisRow;
    BOOL    bChanged = FALSE;

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
            USHORT nAttrRow = Min( (USHORT)nRow, (USHORT)nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( ScMergeFlagAttr( nOldValue & ~nFlags ) );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, TRUE );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
            bChanged = TRUE;
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }

    return bChanged;
}


void ScAttrArray::ClearItems( USHORT nStartRow, USHORT nEndRow, const USHORT* pWhich )
{
    const ScPatternAttr* pOldPattern;

    short   nIndex;
    USHORT  nRow;
    USHORT  nThisRow;

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
            USHORT nAttrRow = Min( (USHORT)nRow, (USHORT)nEndRow );
            SetPatternArea( nThisRow, nAttrRow, &aNewPattern, TRUE );
            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}


void ScAttrArray::ChangeIndent( USHORT nStartRow, USHORT nEndRow, BOOL bIncrement )
{
    short nIndex;
    Search( nStartRow, nIndex );
    USHORT nThisStart = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisStart < nStartRow) nThisStart = nStartRow;

    while ( nThisStart <= nEndRow )
    {
        const ScPatternAttr* pOldPattern = pData[nIndex].pPattern;
        const SfxItemSet& rOldSet = pOldPattern->GetItemSet();
        const SfxPoolItem* pItem;

        BOOL bNeedJust = ( rOldSet.GetItemState( ATTR_HOR_JUSTIFY, FALSE, &pItem ) != SFX_ITEM_SET
                        || ((const SvxHorJustifyItem*)pItem)->GetValue() != SVX_HOR_JUSTIFY_LEFT );
        USHORT nOldValue = ((const SfxUInt16Item&)rOldSet.Get( ATTR_INDENT )).GetValue();
        USHORT nNewValue = nOldValue;
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
            USHORT nThisEnd = pData[nIndex].nRow;
            USHORT nAttrRow = Min( nThisEnd, nEndRow );
            ScPatternAttr aNewPattern(*pOldPattern);
            aNewPattern.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, nNewValue ) );
            if ( bNeedJust )
                aNewPattern.GetItemSet().Put(
                                SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
            SetPatternArea( nThisStart, nAttrRow, &aNewPattern, TRUE );

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


short ScAttrArray::GetNextUnprotected( short nRow, BOOL bUp ) const
{
    short nRet = nRow;
    if (VALIDROW(nRow))
    {
        short nIndex;
        Search(nRow, nIndex);
        while (((const ScProtectionAttr&)pData[nIndex].pPattern->
                GetItem(ATTR_PROTECTION)).GetProtection())
        {
            if (bUp)
            {
                --nIndex;
                if (nIndex<0)
                    return -1;                  // nichts gefunden
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


void ScAttrArray::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, BOOL* pUsed, BOOL bReset )
{
    USHORT nStart = 0;
    short nPos = 0;
    while (nPos < (short) nCount)
    {
        USHORT nEnd = pData[nPos].nRow;
        if (pData[nPos].pPattern->GetStyleSheet() == pStyleSheet)
        {
//          for (USHORT nRow = nStart; nRow <= nEnd; nRow++)
//              pUsed[nRow] = TRUE;

            memset( &pUsed[nStart], TRUE, nEnd-nStart+1 );

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


BOOL ScAttrArray::IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const
{
    BOOL    bIsUsed = FALSE;
    short   nPos    = 0;

    while ( !bIsUsed && ( nPos < (short)nCount ) )
    {
        bIsUsed = ( pData[nPos].pPattern->GetStyleSheet() == &rStyle );
        nPos++;
    }

    return bIsUsed;
}


BOOL ScAttrArray::IsEmpty() const
{
    if (nCount == 1)
    {
        if ( pData[0].pPattern != pDocument->GetDefPattern() )
            return FALSE;
        else
            return TRUE;
    }
    else
        return FALSE;
}


USHORT ScAttrArray::GetFirstEntryPos() const
{
    DBG_ASSERT( nCount, "nCount = 0" );

    if ( pData[0].pPattern != pDocument->GetDefPattern() )
        return 0;
    else
    {
        if (nCount==1)
            return 0;                               // leer
        else
            return pData[0].nRow + 1;
    }
}


USHORT ScAttrArray::GetLastEntryPos( BOOL bIncludeBottom ) const
{
    DBG_ASSERT( nCount, "nCount == 0" );

    if (bIncludeBottom)
        bIncludeBottom = ( pData[nCount-1].pPattern != pDocument->GetDefPattern() );

    if (bIncludeBottom)
        return MAXROW;
    else
    {
        if (nCount<=1)
            return 0;                               // leer
        else
            return pData[nCount-2].nRow;
    }
}


BOOL ScAttrArray::HasVisibleAttr( USHORT& rFirstRow, USHORT& rLastRow, BOOL bSkipFirst ) const
{
    DBG_ASSERT( nCount, "nCount == 0" );

    BOOL bFound = FALSE;
    USHORT nStart = 0;

    if ( bSkipFirst )                           // Anfang ueberspringen, wenn >1 Zeile
    {
        USHORT nVisStart = 1;
        while ( nVisStart < nCount &&
                pData[nVisStart].pPattern->IsVisibleEqual(*pData[nVisStart-1].pPattern) )
            ++nVisStart;
        if ( nVisStart >= nCount || pData[nVisStart-1].nRow > 0 )   // mehr als 1 Zeile?
            nStart = nVisStart;
    }

    USHORT nVisCount = nCount-1;                // am Ende zusammengehoerende weglassen
    while ( nVisCount > nStart &&
            pData[nVisCount].pPattern->IsVisibleEqual(*pData[nVisCount-1].pPattern) )
        --nVisCount;

    while ( nStart < nVisCount && !bFound )
    {
        if ( pData[nStart].pPattern->IsVisible() )
        {
            rFirstRow = nStart ? ( pData[nStart-1].nRow + 1 ) : 0;
            rLastRow = pData[nStart].nRow;
            bFound = TRUE;
        }
        else
            ++nStart;
    }

    if (!bFound)
        return FALSE;

    BOOL bEnd = FALSE;
    USHORT nPos = nVisCount;
    while ( nPos > nStart && !bEnd )
    {
        --nPos;
        if ( pData[nPos].pPattern->IsVisible() )
        {
            rLastRow = pData[nPos].nRow;
            bEnd = TRUE;
        }
    }

    return TRUE;
}


BOOL ScAttrArray::HasVisibleAttrIn( USHORT nStartRow, USHORT nEndRow ) const
{
    short nIndex;
    Search( nStartRow, nIndex );
    USHORT nThisStart = nStartRow;
    BOOL bFound = FALSE;
    while ( nIndex < nCount && nThisStart <= nEndRow && !bFound )
    {
        if ( pData[nIndex].pPattern->IsVisible() )
            bFound = TRUE;

        nThisStart = pData[nIndex].nRow + 1;
        ++nIndex;
    }

    return bFound;
}


BOOL ScAttrArray::IsVisibleEqual( const ScAttrArray& rOther,
                                    USHORT nStartRow, USHORT nEndRow ) const
{
    BOOL bEqual = TRUE;
    short nThisPos = 0;
    short nOtherPos = 0;
    if ( nStartRow )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<nCount && nOtherPos<rOther.nCount && bEqual )
    {
        USHORT nThisRow = pData[nThisPos].nRow;
        USHORT nOtherRow = rOther.pData[nOtherPos].nRow;
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


BOOL ScAttrArray::IsAllEqual( const ScAttrArray& rOther, USHORT nStartRow, USHORT nEndRow ) const
{
    //! mit IsVisibleEqual zusammenfassen?

    BOOL bEqual = TRUE;
    short nThisPos = 0;
    short nOtherPos = 0;
    if ( nStartRow )
    {
        Search( nStartRow, nThisPos );
        rOther.Search( nStartRow, nOtherPos );
    }

    while ( nThisPos<nCount && nOtherPos<rOther.nCount && bEqual )
    {
        USHORT nThisRow = pData[nThisPos].nRow;
        USHORT nOtherRow = rOther.pData[nOtherPos].nRow;
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


BOOL ScAttrArray::TestInsertCol( USHORT nStartRow, USHORT nEndRow) const
{
    //  horizontal zusammengefasste duerfen nicht herausgeschoben werden
    //  (ob die ganze Zusammenfassung betroffen ist, ist hier nicht zu erkennen)

    BOOL bTest = TRUE;
    if (!IsEmpty())
    {
        short nIndex = 0;
        if ( nStartRow )
            Search( nStartRow, nIndex );

        for ( ; nIndex < nCount; nIndex++ )
        {
            if ( ((const ScMergeFlagAttr&)pData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped() )
            {
                bTest = FALSE;                      // darf nicht herausgeschoben werden
                break;
            }
            if ( pData[nIndex].nRow >= nEndRow )    // Ende des Bereichs
                break;
        }
    }
    return bTest;
}


BOOL ScAttrArray::TestInsertRow( USHORT nSize ) const
{
    //  wenn die erste herausgeschobene Zeile vertikal ueberlappt ist,
    //  wuerde eine kaputte Zusammenfassung uebrigbleiben

    if (pData)
    {
        //  MAXROW + 1 - nSize  = erste herausgeschobene Zeile

        USHORT nFirstLost = nCount-1;
        while ( nFirstLost && pData[nFirstLost-1].nRow >= MAXROW + 1 - nSize )
            --nFirstLost;

        if ( ((const ScMergeFlagAttr&)pData[nFirstLost].pPattern->
                            GetItem(ATTR_MERGE_FLAG)).IsVerOverlapped() )
            return FALSE;
    }

    return TRUE;
}


void ScAttrArray::InsertRow( USHORT nStartRow, USHORT nSize )
{
    if (!pData)
        return;

    USHORT nSearch = nStartRow ? nStartRow - 1 : 0;     // Vorgaenger erweitern
    short nIndex;
    Search( nSearch, nIndex );

    //  ein gesetztes ScMergeAttr darf nicht ausgedehnt werden
    //  (darum hinterher wieder loeschen)

    BOOL bDoMerge = ((const ScMergeAttr&) pData[nIndex].pPattern->GetItem(ATTR_MERGE)).IsMerged();

    USHORT nRemove = 0;
    USHORT i;
    for (i = nIndex; i < nCount-1; i++)
    {
        USHORT nNew = pData[i].nRow + nSize;
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
        for (i=0; i<nSize; i++)
            pDocument->ApplyAttr( nCol, nStartRow+i, nTab, rDef );

        //  im eingefuegten Bereich ist nichts zusammengefasst
    }

    //  Flags nicht duplizieren
    //! direkt am Pattern testen ??
    RemoveFlags( nStartRow, nStartRow+nSize-1, SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
}


void ScAttrArray::DeleteRow( USHORT nStartRow, USHORT nSize )
{
    if (pData)
    {
        BOOL bFirst=TRUE;
        USHORT nStartIndex;
        USHORT nEndIndex;
        for (USHORT i = 0; i < nCount-1; i++)
            if (pData[i].nRow >= nStartRow && pData[i].nRow <= nStartRow+nSize-1)
            {
                if (bFirst)
                {
                    nStartIndex = i;
                    bFirst = FALSE;
                }
                nEndIndex = i;
            }
        if (!bFirst)
        {
            USHORT nStart;
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


void ScAttrArray::DeleteRange( USHORT nStartIndex, USHORT nEndIndex )
{
    ScDocumentPool* pDocPool = pDocument->GetPool();
    for (USHORT i = nStartIndex; i <= nEndIndex; i++)
        pDocPool->Remove(*pData[i].pPattern);

    memmove( &pData[nStartIndex], &pData[nEndIndex + 1], (nCount - nEndIndex - 1) * sizeof(ScAttrEntry) );
    nCount -= nEndIndex-nStartIndex+1;
}


void ScAttrArray::DeleteArea(USHORT nStartRow, USHORT nEndRow)
{
    RemoveAreaMerge( nStartRow, nEndRow );          // von zusammengefassten auch die Flags loeschen

    if ( !HasAttrib( nStartRow, nEndRow, HASATTR_OVERLAPPED | HASATTR_AUTOFILTER) )
        SetPatternArea( nStartRow, nEndRow, pDocument->GetDefPattern() );
    else
        DeleteAreaSafe( nStartRow, nEndRow );       // Merge-Flags stehenlassen
}


void ScAttrArray::DeleteHardAttr(USHORT nStartRow, USHORT nEndRow)
{
    const ScPatternAttr* pDefPattern = pDocument->GetDefPattern();
    const ScPatternAttr* pOldPattern;

    short   nIndex;
    USHORT  nRow;
    USHORT  nThisRow;

    Search( nStartRow, nIndex );
    nThisRow = (nIndex>0) ? pData[nIndex-1].nRow+1 : 0;
    if (nThisRow < nStartRow) nThisRow = nStartRow;

    while ( nThisRow <= nEndRow )
    {
        pOldPattern = pData[nIndex].pPattern;

        if ( pOldPattern->GetItemSet().Count() )        // harte Attribute ?
        {
            nRow = pData[nIndex].nRow;
            USHORT nAttrRow = Min( (USHORT)nRow, (USHORT)nEndRow );

            ScPatternAttr aNewPattern(*pOldPattern);
            SfxItemSet& rSet = aNewPattern.GetItemSet();
            for (USHORT nId = ATTR_PATTERN_START; nId <= ATTR_PATTERN_END; nId++)
                if (nId != ATTR_MERGE && nId != ATTR_MERGE_FLAG)
                    rSet.ClearItem(nId);

            if ( aNewPattern == *pDefPattern )
                SetPatternArea( nThisRow, nAttrRow, pDefPattern, FALSE );
            else
                SetPatternArea( nThisRow, nAttrRow, &aNewPattern, TRUE );

            Search( nThisRow, nIndex );                                 // Daten wurden veraendert !!!
        }

        ++nIndex;
        nThisRow = pData[nIndex-1].nRow+1;
    }
}

        // Verschieben innerhalb eines Dokuments

void ScAttrArray::MoveTo(USHORT nStartRow, USHORT nEndRow, ScAttrArray& rAttrArray)
{
    USHORT nStart = nStartRow;
    for (USHORT i = 0; i < nCount; i++)
    {
        if ((pData[i].nRow >= nStartRow) && ((i==0) ? TRUE : pData[i-1].nRow < nEndRow))
        {
            //  Kopieren (bPutToPool=TRUE)
            rAttrArray.SetPatternArea( nStart, Min( (USHORT)pData[i].nRow, (USHORT)nEndRow ),
                                        pData[i].pPattern, TRUE );
        }
        nStart = Max( (USHORT)nStart, (USHORT)(pData[i].nRow + 1) );
    }
    DeleteArea(nStartRow, nEndRow);
}


        // Kopieren zwischen Dokumenten (Clipboard)

void ScAttrArray::CopyArea( USHORT nStartRow, USHORT nEndRow, short nDy, ScAttrArray& rAttrArray,
                                INT16 nStripFlags )
{
    nStartRow -= nDy;       // Source
    nEndRow -= nDy;

    USHORT nDestStart = Max((short)((short)nStartRow + nDy), (short) 0);
    USHORT nDestEnd = Min((short)((short)nEndRow + nDy), (short) MAXROW);

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    BOOL bSamePool = (pSourceDocPool==pDestDocPool);

    for (USHORT i = 0; (i < nCount) && (nDestStart <= nDestEnd); i++)
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
                INT16 nNewFlags = 0;
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
                            Min((USHORT)(pData[i].nRow + nDy), nDestEnd), pNewPattern);
        }
        nDestStart = Max(nDestStart, (USHORT)(pData[i].nRow + nDy + 1));
    }
}

        // Flags stehenlassen
        //! mit CopyArea zusammenfassen !!!

void ScAttrArray::CopyAreaSafe( USHORT nStartRow, USHORT nEndRow, short nDy, ScAttrArray& rAttrArray )
{
    nStartRow -= nDy;       // Source
    nEndRow -= nDy;

    USHORT nDestStart = Max((short)((short)nStartRow + nDy), (short) 0);
    USHORT nDestEnd = Min((short)((short)nEndRow + nDy), (short) MAXROW);

    if ( !rAttrArray.HasAttrib( nDestStart, nDestEnd, HASATTR_OVERLAPPED ) )
    {
        CopyArea( nStartRow+nDy, nEndRow+nDy, nDy, rAttrArray );
        return;
    }

    ScDocumentPool* pSourceDocPool = pDocument->GetPool();
    ScDocumentPool* pDestDocPool = rAttrArray.pDocument->GetPool();
    BOOL bSamePool = (pSourceDocPool==pDestDocPool);

    for (USHORT i = 0; (i < nCount) && (nDestStart <= nDestEnd); i++)
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
                            Min((USHORT)(pData[i].nRow + nDy), nDestEnd), pNewPattern, FALSE);
        }
        nDestStart = Max(nDestStart, (USHORT)(pData[i].nRow + nDy + 1));
    }
}


short ScAttrArray::SearchStyle( short nRow, const ScStyleSheet* pSearchStyle,
                                    BOOL bUp, ScMarkArray* pMarkArray )
{
    BOOL bFound = FALSE;

    if (pMarkArray)
    {
        nRow = pMarkArray->GetNextMarked( nRow, bUp );
        if (nRow<0 || nRow>MAXROW)
            return nRow;
    }

    short nIndex;
    Search(nRow, nIndex);
    const ScPatternAttr* pPattern = pData[nIndex].pPattern;

    while (nIndex>=0 && nIndex<(short)nCount && !bFound)
    {
        if (pPattern->GetStyleSheet() == pSearchStyle)
        {
            if (pMarkArray)
            {
                nRow = pMarkArray->GetNextMarked( nRow, bUp );
                short nStart = nIndex ? pData[nIndex-1].nRow+1 : 0;
                if (nRow >= nStart && nRow <= (short) pData[nIndex].nRow)
                    bFound = TRUE;
            }
            else
                bFound = TRUE;
        }

        if (!bFound)
        {
            if (bUp)
            {
                --nIndex;
                if (nIndex>=0)
                {
                    nRow = pData[nIndex].nRow;
                    pPattern = pData[nIndex].pPattern;
                }
                else
                    nRow = -1;
            }
            else
            {
                nRow = pData[nIndex].nRow+1;
                ++nIndex;
                if (nIndex<(short)nCount)
                    pPattern = pData[nIndex].pPattern;
            }
        }
    }

    DBG_ASSERT( bFound || nRow<0 || nRow>MAXROW, "interner Fehler in ScAttrArray::SearchStyle" );

    return nRow;
}


BOOL ScAttrArray::SearchStyleRange( short& rRow, short& rEndRow,
                        const ScStyleSheet* pSearchStyle, BOOL bUp, ScMarkArray* pMarkArray )
{
    short nStartRow = SearchStyle( rRow, pSearchStyle, bUp, pMarkArray );
    if (VALIDROW(nStartRow))
    {
        short nIndex;
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
                short nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, TRUE );
                if (nMarkEnd>rEndRow)
                    rEndRow = nMarkEnd;
            }
        }
        else
        {
            rEndRow = pData[nIndex].nRow;
            if (pMarkArray)
            {
                short nMarkEnd = pMarkArray->GetMarkEnd( nStartRow, FALSE );
                if (nMarkEnd<rEndRow)
                    rEndRow = nMarkEnd;
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

//------------------------------------------------------------------------
//
//                          Laden / Speichern
//


void ScAttrArray::Save( SvStream& rStream ) const
{
    ScWriteHeader aHdr( rStream, 8 );

    ScDocumentPool* pDocPool = pDocument->GetPool();

    USHORT nSaveCount = nCount;
    USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
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
    for (USHORT i=0; i<nSaveCount; i++)
    {
        rStream << Min( pData[i].nRow, nSaveMaxRow );

        const ScPatternAttr* pPattern = pData[i].pPattern;
        pDocPool->StoreSurrogate( rStream, pPattern );

        //  FALSE, weil ATTR_CONDITIONAL (noch) nicht in Vorlagen:
        if (pPattern->GetItemSet().GetItemState(ATTR_CONDITIONAL,FALSE,&pItem) == SFX_ITEM_SET)
            pDocument->SetConditionalUsed( ((const SfxUInt32Item*)pItem)->GetValue() );

        if (pPattern->GetItemSet().GetItemState(ATTR_VALIDDATA,FALSE,&pItem) == SFX_ITEM_SET)
            pDocument->SetValidationUsed( ((const SfxUInt32Item*)pItem)->GetValue() );
    }
}


void ScAttrArray::Load( SvStream& rStream )
{
    ScDocumentPool* pDocPool = pDocument->GetPool();

    ScReadHeader aHdr( rStream );

    USHORT nNewCount;
    rStream >> nNewCount;
    if ( nNewCount > MAXROW+1 )                     // wuerde das Array zu gross?
    {
        pDocument->SetLostData();
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }

    Reset( pDocument->GetDefPattern(), FALSE );     // loeschen
    pData = new ScAttrEntry[nNewCount];             // neu anlegen
    for (USHORT i=0; i<nNewCount; i++)
    {
        rStream >> pData[i].nRow;

        USHORT nWhich = ATTR_PATTERN;
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
    nCount = nNewCount;

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
}





