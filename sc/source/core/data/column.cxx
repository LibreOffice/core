/*************************************************************************
 *
 *  $RCSfile: column.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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

// INCLUDE ---------------------------------------------------------------

#include <svtools/poolcach.hxx>
#include <svtools/zforlist.hxx>
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

//#pragma optimize ( "", off )
//  nur Search ohne Optimierung!

// STATIC DATA -----------------------------------------------------------


inline BOOL CellVisible( const ScBaseCell* pCell )      //! an Zelle verschieben
{
    return ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() );
}

// -----------------------------------------------------------------------------------------


ScColumn::ScColumn() :
    nCol( 0 ),
    nCount( 0 ),
    nLimit( 0 ),
    pItems( NULL ),
    pDocument( NULL ),
    pAttrArray( NULL )
{
}


ScColumn::~ScColumn()
{
    FreeAll();
    if (pAttrArray) delete pAttrArray;
}


void ScColumn::Init(USHORT nNewCol, USHORT nNewTab, ScDocument* pDoc)
{
    nCol = nNewCol;
    nTab = nNewTab;
    pDocument = pDoc;
    pAttrArray = new ScAttrArray( nCol, nTab, pDocument );
}


short ScColumn::GetNextUnprotected( short nRow, BOOL bUp ) const
{
    return pAttrArray->GetNextUnprotected(nRow, bUp);
}


USHORT ScColumn::GetBlockMatrixEdges( USHORT nRow1, USHORT nRow2, USHORT nMask ) const
{
    // nix:0, mitte:1, unten:2, links:4, oben:8, rechts:16, offen:32
    if ( !pItems )
        return 0;
    if ( nRow1 == nRow2 )
    {
        USHORT nIndex;
        if ( Search( nRow1, nIndex ) )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA
                && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
            {
                ScAddress aOrg( (UINT32)0xFFFFFFFF );
                return ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
            }
        }
        return 0;
    }
    else
    {
        ScAddress aOrg( (UINT32)0xFFFFFFFF );
        BOOL bOpen = FALSE;
        USHORT nEdges = 0;
        USHORT nIndex;
        Search( nRow1, nIndex );
        while ( nIndex < nCount && pItems[nIndex].nRow <= nRow2 )
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA
                && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
            {
                nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
                if ( nEdges )
                {
                    if ( nEdges & 8 )
                        bOpen = TRUE;   // obere Kante oeffnet, weitersehen
                    else if ( !bOpen )
                        return nEdges | 32; // es gibt was, was nicht geoeffnet wurde
                    else if ( nEdges & 1 )
                        return nEdges;  // mittendrin
                    // (nMask & 16 und  (4 und nicht 16)) oder
                    // (nMask & 4  und (16 und nicht 4))
                    if ( ((nMask & 16) && (nEdges & 4)  && !(nEdges & 16))
                        || ((nMask & 4)  && (nEdges & 16) && !(nEdges & 4)) )
                        return nEdges;  // nur linke/rechte Kante
                    if ( nEdges & 2 )
                        bOpen = FALSE;  // untere Kante schliesst
                }
            }
            nIndex++;
        }
        if ( bOpen )
            nEdges |= 32;           // es geht noch weiter
        return nEdges;
    }
}


BOOL ScColumn::HasSelectionMatrixFragment(const ScMarkData& rMark) const
{
    if ( rMark.IsMultiMarked() )
    {
        BOOL bFound = FALSE;

        ScAddress aOrg( (UINT32)0xFFFFFFFF );
        ScAddress aCurOrg( (UINT32)0xFFFFFFFF );
        USHORT nTop, nBottom;
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while ( !bFound && aMarkIter.Next( nTop, nBottom ) )
        {
            BOOL bOpen = FALSE;
            USHORT nEdges;
            USHORT nIndex;
            Search( nTop, nIndex );
            while ( !bFound && nIndex < nCount && pItems[nIndex].nRow <= nBottom )
            {
                ScBaseCell* pCell = pItems[nIndex].pCell;
                if ( pCell->GetCellType() == CELLTYPE_FORMULA
                    && ((ScFormulaCell*)pCell)->GetMatrixFlag() )
                {
                    nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
                    if ( nEdges )
                    {
                        if ( nEdges & 8 )
                            bOpen = TRUE;   // obere Kante oeffnet, weitersehen
                        else if ( !bOpen )
                            return TRUE;    // es gibt was, was nicht geoeffnet wurde
                        else if ( nEdges & 1 )
                            bFound = TRUE;  // mittendrin, alles selektiert?
                        // (4 und nicht 16) oder (16 und nicht 4)
                        if ( (((nEdges & 4) | 16) ^ ((nEdges & 16) | 4)) )
                            bFound = TRUE;  // nur linke/rechte Kante, alles selektiert?
                        if ( nEdges & 2 )
                            bOpen = FALSE;  // untere Kante schliesst

                        if ( bFound )
                        {   // alles selektiert?
                            if ( aCurOrg != aOrg )
                            {   // neue Matrix zu pruefen?
                                aCurOrg = aOrg;
                                ScFormulaCell* pFCell;
                                if ( ((ScFormulaCell*)pCell)->GetMatrixFlag()
                                        == MM_REFERENCE )
                                    pFCell = (ScFormulaCell*) pDocument->GetCell( aOrg );
                                else
                                    pFCell = (ScFormulaCell*)pCell;
                                USHORT nC, nR;
                                pFCell->GetMatColsRows( nC, nR );
                                ScRange aRange( aOrg, ScAddress(
                                    aOrg.Col() + nC - 1, aOrg.Row() + nR - 1,
                                    aOrg.Tab() ) );
                                if ( rMark.IsAllMarked( aRange ) )
                                    bFound = FALSE;
                            }
                            else
                                bFound = FALSE;     // war schon
                        }
                    }
                }
                nIndex++;
            }
            if ( bOpen )
                return TRUE;
        }
        return bFound;
    }
    else
        return FALSE;
}


BOOL ScColumn::HasLines( USHORT nRow1, USHORT nRow2, Rectangle& rSizes,
                            BOOL bLeft, BOOL bRight ) const
{
    return pAttrArray->HasLines( nRow1, nRow2, rSizes, bLeft, bRight );
}


BOOL ScColumn::HasAttrib( USHORT nRow1, USHORT nRow2, USHORT nMask ) const
{
    return pAttrArray->HasAttrib( nRow1, nRow2, nMask );
}


BOOL ScColumn::HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const
{
    BOOL bFound = FALSE;

    USHORT nTop;
    USHORT nBottom;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while (aMarkIter.Next( nTop, nBottom ) && !bFound)
        {
            if (pAttrArray->HasAttrib( nTop, nBottom, nMask ))
                bFound = TRUE;
        }
    }

    return bFound;
}


BOOL ScColumn::ExtendMerge( USHORT nThisCol, USHORT nStartRow, USHORT nEndRow,
                            USHORT& rPaintCol, USHORT& rPaintRow,
                            BOOL bRefresh, BOOL bAttrs )
{
    return pAttrArray->ExtendMerge( nThisCol, nStartRow, nEndRow, rPaintCol, rPaintRow, bRefresh, bAttrs );
}


void ScColumn::MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const
{
    USHORT nTop;
    USHORT nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->MergePatternArea( nTop, nBottom, ppSet, bDeep );
    }
}


void ScColumn::MergePatternArea( SfxItemSet** ppSet, USHORT nRow1, USHORT nRow2, BOOL bDeep ) const
{
    pAttrArray->MergePatternArea( nRow1, nRow2, ppSet, bDeep );
}


void ScColumn::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner,
                            ScLineFlags& rFlags,
                            USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight ) const
{
    pAttrArray->MergeBlockFrame( pLineOuter, pLineInner, rFlags, nStartRow, nEndRow, bLeft, nDistRight );
}


void ScColumn::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            USHORT nStartRow, USHORT nEndRow, BOOL bLeft, USHORT nDistRight )
{
    pAttrArray->ApplyBlockFrame( pLineOuter, pLineInner, nStartRow, nEndRow, bLeft, nDistRight );
}


const ScPatternAttr* ScColumn::GetPattern( USHORT nRow ) const
{
    return pAttrArray->GetPattern( nRow );
}


const SfxPoolItem* ScColumn::GetAttr( USHORT nRow, USHORT nWhich ) const
{
    return &pAttrArray->GetPattern( nRow )->GetItemSet().Get(nWhich);
}


ULONG ScColumn::GetNumberFormat( USHORT nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetNumberFormat( pDocument->GetFormatTable() );
}


short ScColumn::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark )
{
    USHORT nTop;
    USHORT nBottom;
    BOOL bFound = FALSE;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
        {
            pAttrArray->ApplyCacheArea( nTop, nBottom, pCache );
            bFound = TRUE;
        }
    }

    if (!bFound)
        return -1;
    else if (nTop==0 && nBottom==MAXROW)
        return 0;
    else
        return (short) nBottom;
}


void ScColumn::ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark )
{
    USHORT nTop;
    USHORT nBottom;

    if ( pAttrArray && rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ChangeIndent(nTop, nBottom, bIncrement);
    }
}


void ScColumn::ClearSelectionItems( const USHORT* pWhich,const ScMarkData& rMark )
{
    USHORT nTop;
    USHORT nBottom;

    if ( pAttrArray && rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ClearItems(nTop, nBottom, pWhich);
    }
}


void ScColumn::DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark )
{
    USHORT nTop;
    USHORT nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            DeleteArea(nTop, nBottom, nDelFlag);
    }
}


void ScColumn::ApplyPattern( USHORT nRow, const ScPatternAttr& rPatAttr )
{
    const SfxItemSet* pSet = &rPatAttr.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );

    const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );

    //  TRUE = alten Eintrag behalten

    ScPatternAttr* pNewPattern = (ScPatternAttr*) &aCache.ApplyTo( *pPattern, TRUE );
    ScDocumentPool::CheckRef( *pPattern );
    ScDocumentPool::CheckRef( *pNewPattern );

    if (pNewPattern != pPattern)
      pAttrArray->SetPattern( nRow, pNewPattern );
}


void ScColumn::ApplyPatternArea( USHORT nStartRow, USHORT nEndRow, const ScPatternAttr& rPatAttr )
{
    const SfxItemSet* pSet = &rPatAttr.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );
    pAttrArray->ApplyCacheArea( nStartRow, nEndRow, &aCache );
}


void ScColumn::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScPatternAttr& rPattern, short nNewType )
{
    const SfxItemSet* pSet = &rPattern.GetItemSet();
    SfxItemPoolCache aCache( pDocument->GetPool(), pSet );
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
    USHORT nEndRow = rRange.aEnd.Row();
    for ( USHORT nRow = rRange.aStart.Row(); nRow <= nEndRow; nRow++ )
    {
        USHORT nRow1, nRow2;
        const ScPatternAttr* pPattern = pAttrArray->GetPatternRange(
            nRow1, nRow2, nRow );
        ULONG nFormat = pPattern->GetNumberFormat( pFormatter );
        short nOldType = pFormatter->GetType( nFormat );
        if ( nOldType == nNewType || pFormatter->IsCompatible( nOldType, nNewType ) )
            nRow = nRow2;
        else
        {
            USHORT nNewRow1 = Max( nRow1, nRow );
            USHORT nNewRow2 = Min( nRow2, nEndRow );
            pAttrArray->ApplyCacheArea( nNewRow1, nNewRow2, &aCache );
            nRow = nNewRow2;
        }
    }
}


void ScColumn::ApplyStyle( USHORT nRow, const ScStyleSheet& rStyle )
{
    const ScPatternAttr* pPattern = pAttrArray->GetPattern(nRow);
    ScPatternAttr* pNewPattern = new ScPatternAttr(*pPattern);
    if (pNewPattern)
    {
        pNewPattern->SetStyleSheet((ScStyleSheet*)&rStyle);
        pAttrArray->SetPattern(nRow, pNewPattern, TRUE);
        delete pNewPattern;
    }
}


void ScColumn::ApplyStyleArea( USHORT nStartRow, USHORT nEndRow, const ScStyleSheet& rStyle )
{
    pAttrArray->ApplyStyleArea(nStartRow, nEndRow, (ScStyleSheet*)&rStyle);
}


void ScColumn::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    USHORT nTop;
    USHORT nBottom;

    if ( rMark.IsMultiMarked() )
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ApplyStyleArea(nTop, nBottom, (ScStyleSheet*)&rStyle);
    }
}


void ScColumn::ApplySelectionLineStyle( const ScMarkData& rMark,
                                    const SvxBorderLine* pLine, BOOL bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    USHORT nTop;
    USHORT nBottom;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
        while (aMarkIter.Next( nTop, nBottom ))
            pAttrArray->ApplyLineStyleArea(nTop, nBottom, pLine, bColorOnly );
    }
}


const ScStyleSheet* ScColumn::GetStyle( USHORT nRow ) const
{
    return pAttrArray->GetPattern( nRow )->GetStyleSheet();
}


const ScStyleSheet* ScColumn::GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const
{
    rFound = FALSE;
    if (!rMark.IsMultiMarked())
    {
        DBG_ERROR("ScColumn::GetSelectionStyle ohne Selektion");
        return NULL;
    }

    BOOL bEqual = TRUE;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
    USHORT nTop;
    USHORT nBottom;
    while (bEqual && aMarkIter.Next( nTop, nBottom ))
    {
        ScAttrIterator aAttrIter( pAttrArray, nTop, nBottom );
        USHORT nRow;
        USHORT nDummy;
        const ScPatternAttr* pPattern;
        while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ))
        {
            pNewStyle = pPattern->GetStyleSheet();
            rFound = TRUE;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = FALSE;                                             // unterschiedliche
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : NULL;
}


const ScStyleSheet* ScColumn::GetAreaStyle( BOOL& rFound, USHORT nRow1, USHORT nRow2 ) const
{
    rFound = FALSE;

    BOOL bEqual = TRUE;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    ScAttrIterator aAttrIter( pAttrArray, nRow1, nRow2 );
    USHORT nRow;
    USHORT nDummy;
    const ScPatternAttr* pPattern;
    while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ))
    {
        pNewStyle = pPattern->GetStyleSheet();
        rFound = TRUE;
        if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
            bEqual = FALSE;                                             // unterschiedliche
        pStyle = pNewStyle;
    }

    return bEqual ? pStyle : NULL;
}


void ScColumn::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, BOOL* pUsed, BOOL bReset )
{
    pAttrArray->FindStyleSheet( pStyleSheet, pUsed, bReset );
}


BOOL ScColumn::IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const
{
    return pAttrArray->IsStyleSheetUsed( rStyle );
}


BOOL ScColumn::ApplyFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
{
    return pAttrArray->ApplyFlags( nStartRow, nEndRow, nFlags );
}


BOOL ScColumn::RemoveFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
{
    return pAttrArray->RemoveFlags( nStartRow, nEndRow, nFlags );
}


void ScColumn::SetPattern( USHORT nRow, const ScPatternAttr& rPatAttr, BOOL bPutToPool )
{
    pAttrArray->SetPattern( nRow, &rPatAttr, bPutToPool );
}


void ScColumn::SetPatternArea( USHORT nStartRow, USHORT nEndRow,
                                const ScPatternAttr& rPatAttr, BOOL bPutToPool )
{
    pAttrArray->SetPatternArea( nStartRow, nEndRow, &rPatAttr, bPutToPool );
}


void ScColumn::ApplyAttr( USHORT nRow, const SfxPoolItem& rAttr )
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
#if 0
    SfxItemPoolCache aCache( pDocument->GetPool(), &rAttr );

    const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );

    //  TRUE = alten Eintrag behalten

    ScPatternAttr* pNewPattern = (ScPatternAttr*) &aCache.ApplyTo( *pPattern, TRUE );
    ScDocumentPool::CheckRef( *pPattern );
    ScDocumentPool::CheckRef( *pNewPattern );

    if (pNewPattern != pPattern)
      pAttrArray->SetPattern( nRow, pNewPattern );
#endif
}

#pragma optimize ( "", off )


BOOL ScColumn::Search( USHORT nRow, USHORT& nIndex ) const
{
    if ( !pItems || !nCount )
    {
        nIndex = 0;
        return FALSE;
    }
    USHORT nMinRow = pItems[0].nRow;
    if ( nRow <= nMinRow )
    {
        nIndex = 0;
        return nRow == nMinRow;
    }
    USHORT nMaxRow = pItems[nCount-1].nRow;
    if ( nRow >= nMaxRow )
    {
        if ( nRow == nMaxRow )
        {
            nIndex = nCount - 1;
            return TRUE;
        }
        else
        {
            nIndex = nCount;
            return FALSE;
        }
    }

    short nOldLo, nOldHi;
    short   nLo     = nOldLo = 0;
    short   nHi     = nOldHi = Min( (short)(nCount-1), (short)nRow );
    short   i       = 0;
    BOOL    bFound  = FALSE;
    // einigermassen durchgaengige Belegung? => interpolierende Suche
    BOOL    bInterpol = (nMaxRow - nMinRow < nCount * 2);
    USHORT  nR;

    while ( !bFound && nLo <= nHi )
    {
        if ( !bInterpol || nHi - nLo < 3 )
            i = (nLo+nHi) / 2;          // kein Aufwand, kein division by zero
        else
        {   // interpolierende Suche
            long nLoRow = pItems[nLo].nRow;     // kein USHORT Unterlauf bei Subtraktion
            i = nLo + (short)((long)(nRow - nLoRow) * (nHi - nLo)
                / (pItems[nHi].nRow - nLoRow));
            if ( i < 0 || i >= nCount )
            {   // oops ...
                i = (nLo+nHi) / 2;
                bInterpol = FALSE;
            }
        }
        nR = pItems[i].nRow;
        if ( nR < nRow )
        {
            nLo = i+1;
            if ( bInterpol )
            {
                if ( nLo <= nOldLo )
                    bInterpol = FALSE;
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
                        bInterpol = FALSE;
                    else
                        nOldHi = nHi;
                }
            }
            else
                bFound = TRUE;
        }
    }
    if (bFound)
        nIndex = i;
    else
        nIndex = nLo; // hinterer Index
    return bFound;
}

#pragma optimize ( "", on )


ScBaseCell* ScColumn::GetCell( USHORT nRow ) const
{
    USHORT nIndex;
    if (Search(nRow, nIndex))
        return pItems[nIndex].pCell;
    return NULL;
}


void ScColumn::Resize( USHORT nSize )
{
    if (nSize > MAXROW+1)
        nSize = MAXROW+1;
    if (nSize < nCount)
        nSize = nCount;

    ColEntry* pNewItems;
    if (nSize)
    {
        USHORT nNewSize = nSize + COLUMN_DELTA - 1;
        nNewSize -= nNewSize % COLUMN_DELTA;
        nLimit = nNewSize;
        pNewItems = new ColEntry[nLimit];
    }
    else
    {
        nLimit = 0;
        pNewItems = NULL;
    }
    if (pItems)
    {
        if (pNewItems)
            memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
        delete[] pItems;
    }
    pItems = pNewItems;
}

//  SetNewRow gehoert zu SwapRow

void ScColumn::SetNewRow( USHORT nOldRow, USHORT nNewRow )  // nNewRow muss leer sein
{
    USHORT  nIndex;
    if (Search(nOldRow, nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        --nCount;
        memmove( &pItems[nIndex], &pItems[nIndex + 1], (nCount - nIndex) * sizeof(ColEntry) );

        ScBroadcasterList* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
            MoveListeners( *pBC, nOldRow );     // Broadcaster bleibt an alter Stelle
            pCell->SetBroadcaster(NULL);
        }

        if (pCell->GetCellType() != CELLTYPE_NOTE)      // sonst geloescht
        {
            Insert(nNewRow, pCell);

            short dy = (short)nNewRow - (short)nOldRow;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                ScFormulaCell* pFormula = (ScFormulaCell*)pCell;
                ScRange aRange( ScAddress( 0, nNewRow, nTab ),
                                ScAddress( MAXCOL, nNewRow, nTab ) );
                pFormula->aPos.SetRow( nNewRow );
                pFormula->UpdateReference(URM_MOVE, aRange, 0, -dy, 0);
            }
        }
    }
}

//  SwapRow zum Sortieren

void ScColumn::SwapRow(USHORT nRow1, USHORT nRow2)
{
    //      Zeiger vertauschen klappt nicht wegen Broadcastern
    //      (Absturz, wenn Zelle, auch indirekt, auf sich selbst broadcasted)

    ScBaseCell *pCell1, *pCell2;
    USHORT nIndex1, nIndex2;

    if ( Search( nRow1, nIndex1 ) )
        pCell1 = pItems[nIndex1].pCell;
    else
        pCell1 = NULL;
    if ( Search( nRow2, nIndex2 ) )
        pCell2 = pItems[nIndex2].pCell;
    else
        pCell2 = NULL;

    if ( !pCell1 && !pCell2 )
        return ;

    CellType eType1 = ( pCell1 ? pCell1->GetCellType() : CELLTYPE_NONE );
    CellType eType2 = ( pCell2 ? pCell2->GetCellType() : CELLTYPE_NONE );

    // Broadcaster bleiben an alter Stelle
    ScBroadcasterList *pBC1, *pBC2;

    if ( eType1 != CELLTYPE_FORMULA && eType2 != CELLTYPE_FORMULA )
    {   // simple swap, kann nichts auf sich selbst broadcasten
        if ( pCell1 )
        {
            pBC1 = pCell1->GetBroadcaster();
            if ( pBC1 )
                pCell1->ForgetBroadcaster();
        }
        else
            pBC1 = NULL;

        if ( pCell2 )
        {
            pBC2 = pCell2->GetBroadcaster();
            if ( pBC2 )
                pCell2->ForgetBroadcaster();
        }
        else
            pBC2 = NULL;

        if ( pCell1 && pCell2 )
        {
            pItems[nIndex1].pCell = pCell2;
            pItems[nIndex2].pCell = pCell1;
            if ( pBC1 )
                pCell2->SetBroadcaster( pBC1 );
            if ( pBC2 )
                pCell1->SetBroadcaster( pBC2 );
            ScAddress aPos( nCol, nRow1, nTab );
            pDocument->Broadcast( SC_HINT_DATACHANGED, aPos, pCell2 );
            aPos.SetRow( nRow2 );
            pDocument->Broadcast( SC_HINT_DATACHANGED, aPos, pCell1 );
        }
        else if ( pCell1 )
        {
            ScNoteCell* pNew = ( pBC1 ? new ScNoteCell : NULL );
            if ( pNew )
            {
                pItems[nIndex1].pCell = pNew;
                pNew->SetBroadcaster( pBC1 );
            }
            else
            {   // Loeschen
                --nCount;
                memmove( &pItems[nIndex1], &pItems[nIndex1 + 1], (nCount - nIndex1) * sizeof(ColEntry) );
                pItems[nCount].nRow = 0;
                pItems[nCount].pCell = NULL;
            }
            // Einfuegen
            Insert( nRow2, pCell1 );
            pDocument->Broadcast( SC_HINT_DATACHANGED,
                ScAddress( nCol, nRow1, nTab ), pNew );
        }
        else    // pCell2
        {
            ScNoteCell* pNew = ( pBC2 ? new ScNoteCell : NULL );
            if ( pNew )
            {
                pItems[nIndex2].pCell = pNew;
                pNew->SetBroadcaster( pBC2 );
            }
            else
            {   // Loeschen
                --nCount;
                memmove( &pItems[nIndex2], &pItems[nIndex2 + 1], (nCount - nIndex2) * sizeof(ColEntry) );
                pItems[nCount].nRow = 0;
                pItems[nCount].pCell = NULL;
            }
            // Einfuegen
            Insert( nRow1, pCell2 );
            pDocument->Broadcast( SC_HINT_DATACHANGED,
                ScAddress( nCol, nRow2, nTab ), pNew );
        }

        return ;
    }

    // ab hier ist mindestens eine Formelzelle beteiligt

    if ( eType1 == CELLTYPE_FORMULA && eType2 == CELLTYPE_FORMULA )
    {
        ScTokenArray* pCode1 = ((ScFormulaCell*)pCell1)->GetCode();
        ScTokenArray* pCode2 = ((ScFormulaCell*)pCell2)->GetCode();

        if (pCode1->GetLen() == pCode2->GetLen())       // nicht-UPN
        {
            BOOL bEqual = TRUE;
            USHORT nLen = pCode1->GetLen();
            ScToken** ppToken1 = pCode1->GetArray();
            ScToken** ppToken2 = pCode2->GetArray();
            for (USHORT i=0; i<nLen; i++)
                if ( !ppToken1[i]->TextEqual(*(ppToken2[i])) )
                {
                    bEqual = FALSE;
                    break;
                }

            if (bEqual)             // gleiche Formeln nicht vertauschen
                return;
        }
    }

    ScBaseCell *pNew1, *pNew2;

    //  hier kein UpdateReference wegen #30529# - mitsortiert werden nur noch relative Referenzen
//  short dy = (short)nRow2 - (short)nRow1;

    if (pCell1)
    {
        pBC1 = pCell1->GetBroadcaster();
        if ( pBC1 )
            pCell1->ForgetBroadcaster();
        if ( eType1 == CELLTYPE_FORMULA )
        {
            pNew2 = ((ScFormulaCell*)pCell1)->Clone( pDocument, ScAddress( nCol, nRow2, nTab ) );
//          ScRange aRange( ScAddress( 0, nRow2, nTab ), ScAddress( MAXCOL, nRow2, nTab ) );
//          ((ScFormulaCell*)pNew2)->UpdateReference(URM_MOVE, aRange, 0, dy, 0);
        }
        else
            pNew2 = pCell1->Clone( pDocument );
    }
    else
    {
        pNew2 = NULL;
        pBC1 = NULL;
    }

    if (pCell2)
    {
        pBC2 = pCell2->GetBroadcaster();
        if ( pBC2 )
            pCell2->ForgetBroadcaster();
        if ( eType2 == CELLTYPE_FORMULA )
        {
            pNew1 = ((ScFormulaCell*)pCell2)->Clone( pDocument, ScAddress( nCol, nRow1, nTab ) );
//          ScRange aRange( ScAddress( 0, nRow1, nTab ), ScAddress( MAXCOL, nRow1, nTab ) );
//          ((ScFormulaCell*)pNew1)->UpdateReference(URM_MOVE, aRange, 0, -dy, 0);
        }
        else
            pNew1 = pCell2->Clone( pDocument );
    }
    else
    {
        pNew1 = NULL;
        pBC2 = NULL;
    }

    if ( !pNew1 && pBC1 )
        pNew1 = new ScNoteCell;
    if ( !pNew2 && pBC2 )
        pNew2 = new ScNoteCell;

    //  Delete nur, wenn es keine neue Zelle gibt (Insert loescht die alte Zelle auch)
    //  Notizen muessen aber einzeln geloescht werden, weil Insert sie stehenlaesst

    if ( pCell1 && ( !pNew1 || (pCell1->GetNotePtr() && !pNew1->GetNotePtr()) ) )
        Delete( nRow1 );
    if ( pCell2 && ( !pNew2 || (pCell2->GetNotePtr() && !pNew2->GetNotePtr()) ) )
        Delete( nRow2 );

    if (pNew1)
    {
        Insert( nRow1, pNew1 );
        if ( pBC1 )
            pNew1->SetBroadcaster( pBC1 );
    }
    if (pNew2)
    {
        Insert( nRow2, pNew2 );
        if ( pBC2 )
            pNew2->SetBroadcaster( pBC2 );
    }

    //  #64122# Bei Formeln hinterher nochmal broadcasten, damit die Formel nicht in irgendwelchen
    //  FormulaTrack-Listen landet, ohne die Broadcaster beruecksichtigt zu haben
    //  (erst hier, wenn beide Zellen eingefuegt sind)
    if ( pBC1 && eType2 == CELLTYPE_FORMULA )
        pDocument->Broadcast( SC_HINT_DATACHANGED, ScAddress( nCol, nRow1, nTab ), pNew1 );
    if ( pBC2 && eType1 == CELLTYPE_FORMULA )
        pDocument->Broadcast( SC_HINT_DATACHANGED, ScAddress( nCol, nRow2, nTab ), pNew2 );
}


void ScColumn::SwapCell( USHORT nRow, ScColumn& rCol)
{
    BOOL bFound1;
    BOOL bFound2;
    USHORT nIndex1;
    USHORT nIndex2;
    bFound1 = Search(nRow, nIndex1);
    bFound2 = rCol.Search(nRow, nIndex2);
    if (bFound1 && bFound2)
    {
        // Tauschen
        ScFormulaCell* pCell1 = (ScFormulaCell*) pItems[nIndex1].pCell;
        ScFormulaCell* pCell2 = (ScFormulaCell*) rCol.pItems[nIndex2].pCell;
        pItems[nIndex1].pCell = pCell2;
        rCol.pItems[nIndex2].pCell = pCell1;
        // Referenzen aktualisieren
        short dx = (short)rCol.nCol - (short)nCol;
        if (pCell1->GetCellType() == CELLTYPE_FORMULA)
        {
            ScRange aRange( ScAddress( rCol.nCol, 0, nTab ),
                            ScAddress( rCol.nCol, MAXROW, nTab ) );
            pCell1->aPos.SetCol( rCol.nCol );
            pCell1->UpdateReference(URM_MOVE, aRange, dx, 0, 0);
        }
        if (pCell2->GetCellType() == CELLTYPE_FORMULA)
        {
            ScRange aRange( ScAddress( nCol, 0, nTab ),
                            ScAddress( nCol, MAXROW, nTab ) );
            pCell2->aPos.SetCol( nCol );
            pCell2->UpdateReference(URM_MOVE, aRange, -dx, 0, 0);
        }
    }
    else if (bFound1)
    {
        ScFormulaCell* pCell = (ScFormulaCell*) pItems[nIndex1].pCell;
        // Loeschen
        --nCount;
        memmove( &pItems[nIndex1], &pItems[nIndex1 + 1], (nCount - nIndex1) * sizeof(ColEntry) );
        pItems[nCount].nRow = 0;
        pItems[nCount].pCell = NULL;
        // Referenzen aktualisieren
        short dx = (short)rCol.nCol - (short)nCol;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScRange aRange( ScAddress( rCol.nCol, 0, nTab ),
                            ScAddress( rCol.nCol, MAXROW, nTab ) );
            pCell->aPos.SetCol( rCol.nCol );
            pCell->UpdateReference(URM_MOVE, aRange, dx, 0, 0);
        }
        // Einfuegen
        rCol.Insert(nRow, pCell);
    }
    else if (bFound2)
    {
        ScFormulaCell* pCell = (ScFormulaCell*) rCol.pItems[nIndex2].pCell;
        // Loeschen
        --(rCol.nCount);
        memmove( &rCol.pItems[nIndex2], &rCol.pItems[nIndex2 + 1], (rCol.nCount - nIndex2) * sizeof(ColEntry) );
        rCol.pItems[rCol.nCount].nRow = 0;
        rCol.pItems[rCol.nCount].pCell = NULL;
        // Referenzen aktualisieren
        short dx = (short)rCol.nCol - (short)nCol;
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScRange aRange( ScAddress( nCol, 0, nTab ),
                            ScAddress( nCol, MAXROW, nTab ) );
            pCell->aPos.SetCol( nCol );
            pCell->UpdateReference(URM_MOVE, aRange, dx, 0, 0);
        }
        // Einfuegen
        Insert(nRow, pCell);
    }
}


BOOL ScColumn::TestInsertCol( USHORT nStartRow, USHORT nEndRow) const
{
    if (!IsEmpty())
    {
        BOOL bTest = TRUE;
        if (pItems)
            for (USHORT i=0; (i<nCount) && bTest; i++)
                bTest = ((pItems[i].nRow < nStartRow) && (pItems[i].nRow > nEndRow))
                        || !CellVisible(pItems[i].pCell);

        //  AttrArray testet nur zusammengefasste

        if ((bTest) && (pAttrArray))
            bTest = pAttrArray->TestInsertCol(nStartRow, nEndRow);

        //!     rausgeschobene Attribute bei Undo beruecksichtigen

        return bTest;
    }
    else
        return TRUE;
}


BOOL ScColumn::TestInsertRow( USHORT nSize ) const
{
    //  AttrArray testet nur zusammengefasste

    if ( pItems && nCount )
        return ( pItems[nCount-1].nRow <= MAXROW-nSize && pAttrArray->TestInsertRow( nSize ) );
    else
        return pAttrArray->TestInsertRow( nSize );

    //!     rausgeschobene Attribute bei Undo beruecksichtigen

    if ( nSize > MAXROW )
        return FALSE;

    USHORT nVis = nCount;
    while ( nVis && !CellVisible(pItems[nVis-1].pCell) )
        --nVis;

    if ( nVis )
        return ( pItems[nVis-1].nRow <= MAXROW-nSize );
    else
        return TRUE;
}


void ScColumn::InsertRow( USHORT nStartRow, USHORT nSize )
{
    pAttrArray->InsertRow( nStartRow, nSize );

    //! Search

    if ( !pItems || !nCount )
        return;

    USHORT i;
    Search( nStartRow, i );
    if ( i >= nCount )
        return ;

    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden

    USHORT nNewCount = nCount;
    BOOL bCountChanged = FALSE;
    USHORT nLastBroadcast = MAXROW+1;
    ScAddress aAdr( nCol, 0, nTab );

    for ( ; i < nCount; i++)
    {
        USHORT nOldRow = pItems[i].nRow;
        // #43940# Aenderung Quelle broadcasten
        if ( nLastBroadcast != nOldRow )
        {   // direkt aufeinanderfolgende nicht doppelt broadcasten
            aAdr.SetRow( nOldRow );
            pDocument->Broadcast( SC_HINT_DATACHANGED, aAdr, NULL );    // nur Areas
        }
        USHORT nNewRow = (pItems[i].nRow += nSize);
        // #43940# Aenderung Ziel broadcasten
        aAdr.SetRow( nNewRow );
        pDocument->Broadcast( SC_HINT_DATACHANGED, aAdr, NULL );    // nur Areas
        nLastBroadcast = nNewRow;
        ScBaseCell* pCell = pItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
        if ( nNewRow > MAXROW && !bCountChanged )
        {
            nNewCount = i;
            bCountChanged = TRUE;
        }
    }

    if (bCountChanged)
    {
        USHORT nDelCount = nCount - nNewCount;
        ScBaseCell** ppDelCells = new ScBaseCell*[nDelCount];
        USHORT* pDelRows = new USHORT[nDelCount];
        for (i = 0; i < nDelCount; i++)
        {
            ppDelCells[i] = pItems[nNewCount+i].pCell;
            pDelRows[i] = pItems[nNewCount+i].nRow;
        }
        nCount = nNewCount;

        for (i = 0; i < nDelCount; i++)
        {
            ScBaseCell* pCell = ppDelCells[i];
            DBG_ASSERT( !CellVisible(pCell), "sichtbare Zelle weggeschoben" );
            ScBroadcasterList* pBC = pCell->GetBroadcaster();
            if (pBC)
            {
                MoveListeners( *pBC, pDelRows[i] - nSize );
                pCell->SetBroadcaster(NULL);
                pCell->Delete();
            }
        }

        delete pDelRows;
        delete ppDelCells;
    }

    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::CopyToClip(USHORT nRow1, USHORT nRow2, ScColumn& rColumn, BOOL bKeepScenarioFlags)
{
    pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray,
                            bKeepScenarioFlags ? (SC_MF_ALL & ~SC_MF_SCENARIO) : SC_MF_ALL );

    USHORT i;
    USHORT nBlockCount = 0;
    USHORT nStartIndex, nEndIndex;
    for (i = 0; i < nCount; i++)
        if ((pItems[i].nRow >= nRow1) && (pItems[i].nRow <= nRow2))
        {
            if (!nBlockCount)
                nStartIndex = i;
            nEndIndex = i;
            ++nBlockCount;

            //  im Clipboard muessen interpretierte Zellen stehen, um andere Formate
            //  (Text, Grafik...) erzueugen zu koennen

            if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                ScFormulaCell* pFCell = (ScFormulaCell*) pItems[i].pCell;
                if (pFCell->GetDirty() && pDocument->GetAutoCalc())
                    pFCell->Interpret();
            }
        }

    if (nBlockCount)
    {
        rColumn.Resize( rColumn.GetCellCount() + nBlockCount );
        for (i = nStartIndex; i <= nEndIndex; i++)
            rColumn.Append(pItems[i].nRow, pItems[i].pCell->Clone(rColumn.pDocument));
    }
}


void ScColumn::CopyToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData, BOOL bAsLink )
{
    if (bMarked)
    {
        USHORT nStart, nEnd;
        if (pMarkData && pMarkData->IsMultiMarked())
        {
            ScMarkArrayIter aIter( pMarkData->GetArray()+nCol );

            while ( aIter.Next( nStart, nEnd ) && nStart <= nRow2 )
            {
                if ( nEnd >= nRow1 )
                    CopyToColumn( Max(nRow1,nStart), Min(nRow2,nEnd),
                                    nFlags, FALSE, rColumn, pMarkData, bAsLink );
            }
        }
        else
            DBG_ERROR("CopyToColumn: bMarked, aber keine Markierung");
        return;
    }

    if ( (nFlags & IDF_ATTRIB) != 0 )
    {
        if ( (nFlags & IDF_STYLES) != IDF_STYLES )
        {   // StyleSheets im Zieldokument bleiben erhalten
            // z.B. DIF und RTF Clipboard-Import
            for ( USHORT nRow = nRow1; nRow <= nRow2; nRow++ )
            {
                const ScStyleSheet* pStyle =
                    rColumn.pAttrArray->GetPattern( nRow )->GetStyleSheet();
                const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
                ScPatternAttr* pNewPattern = new ScPatternAttr( *pPattern );
                pNewPattern->SetStyleSheet( (ScStyleSheet*)pStyle );
                rColumn.pAttrArray->SetPattern( nRow, pNewPattern, TRUE );
                delete pNewPattern;
            }
        }
        else
            pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray);
    }


    if ((nFlags & IDF_CONTENTS) != 0)
    {
        USHORT i;
        USHORT nBlockCount = 0;
        USHORT nStartIndex, nEndIndex;
        for (i = 0; i < nCount; i++)
            if ((pItems[i].nRow >= nRow1) && (pItems[i].nRow <= nRow2))
            {
                if (!nBlockCount)
                    nStartIndex = i;
                nEndIndex = i;
                ++nBlockCount;
            }

        if (nBlockCount)
        {
            rColumn.Resize( rColumn.GetCellCount() + nBlockCount );
            ScAddress aAdr( rColumn.nCol, 0, rColumn.nTab );
            for (i = nStartIndex; i <= nEndIndex; i++)
            {
                ScBaseCell* pOld = pItems[i].pCell;
                aAdr.SetRow( pItems[i].nRow );
                ScBaseCell* pNew;
                if (bAsLink)
                {
                    pNew = CreateRefCell( rColumn.pDocument, aAdr, i, nFlags );
                }
                else
                {
                    pNew = CloneCell( i, nFlags, rColumn.pDocument, aAdr );

                    if ( pNew && pNew->GetNotePtr() && (nFlags & IDF_NOTE) == 0 )
                        pNew->DeleteNote();
                }

                if (pNew)
                    rColumn.Insert(pItems[i].nRow, pNew);
            }
        }
    }
}


void ScColumn::UndoToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
                                ScColumn& rColumn, const ScMarkData* pMarkData )
{
    if (nRow1 > 0)
        CopyToColumn( 0, nRow1-1, IDF_FORMULA, FALSE, rColumn );

    CopyToColumn( nRow1, nRow2, nFlags, bMarked, rColumn, pMarkData );      //! bMarked ????

    if (nRow2 < MAXROW)
        CopyToColumn( nRow2+1, MAXROW, IDF_FORMULA, FALSE, rColumn );
}


void ScColumn::CopyUpdated( const ScColumn& rPosCol, ScColumn& rDestCol ) const
{
    ScDocument* pDestDoc = rDestCol.pDocument;

    USHORT nPosCount = rPosCol.nCount;
    for (USHORT nPosIndex = 0; nPosIndex < nPosCount; nPosIndex++)
    {
        USHORT nRow = rPosCol.pItems[nPosIndex].nRow;
        USHORT nThisIndex;
        if ( Search( nRow, nThisIndex ) )
        {
            ScBaseCell* pNew = pItems[nThisIndex].pCell->Clone(pDestDoc);
            rDestCol.Insert( nRow, pNew );
        }
    }

    //  Dummy:
    //  CopyToColumn( 0,MAXROW, IDF_FORMULA, FALSE, rDestCol, NULL, FALSE );
}


void ScColumn::CopyScenarioFrom( const ScColumn& rSrcCol )
{
    //  Dies ist die Szenario-Tabelle, die Daten werden hineinkopiert

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    USHORT nStart, nEnd;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            DeleteArea( nStart, nEnd, IDF_CONTENTS );
            ((ScColumn&)rSrcCol).
                CopyToColumn( nStart, nEnd, IDF_CONTENTS, FALSE, *this );

            //  UpdateUsed nicht noetig, schon in TestCopyScenario passiert

            short nDz = ((short)nTab) - (short)rSrcCol.nTab;
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
    USHORT nStart, nEnd;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            rDestCol.DeleteArea( nStart, nEnd, IDF_CONTENTS );
            ((ScColumn*)this)->
                CopyToColumn( nStart, nEnd, IDF_CONTENTS, FALSE, rDestCol );

            //  UpdateUsed nicht noetig, schon in TestCopyScenario passiert

            short nDz = ((short)rDestCol.nTab) - (short)nTab;
            rDestCol.UpdateReference(URM_COPY, rDestCol.nCol, nStart, rDestCol.nTab,
                                               rDestCol.nCol, nEnd,   rDestCol.nTab,
                                               0, 0, nDz, NULL);
            rDestCol.UpdateCompile();
        }

        //! CopyToColumn "const" machen !!!

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}


BOOL ScColumn::TestCopyScenarioTo( const ScColumn& rDestCol ) const
{
    BOOL bOk = TRUE;
    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    USHORT nStart, nEnd;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern && bOk)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
            if ( rDestCol.pAttrArray->HasAttrib( nStart, nEnd, HASATTR_PROTECTED ) )
                bOk = FALSE;

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
    return bOk;
}


void ScColumn::MarkScenarioIn( ScMarkData& rDestMark ) const
{
    ScRange aRange( nCol, 0, nTab );

    ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
    USHORT nStart, nEnd;
    const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
    while (pPattern)
    {
        if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
        {
            aRange.aStart.SetRow( nStart );
            aRange.aEnd.SetRow( nEnd );
            rDestMark.SetMultiMarkArea( aRange, TRUE );
        }

        pPattern = aAttrIter.Next( nStart, nEnd );
    }
}


void ScColumn::SwapCol(ScColumn& rCol)
{
    USHORT nTemp;

    nTemp = rCol.nCount;
    rCol.nCount  = nCount;
    nCount = nTemp;

    nTemp = rCol.nLimit;
    rCol.nLimit = nLimit;
    nLimit = nTemp;

    ColEntry* pTempItems = rCol.pItems;
    rCol.pItems = pItems;
    pItems = pTempItems;

    ScAttrArray* pTempAttr = rCol.pAttrArray;
    rCol.pAttrArray = pAttrArray;
    pAttrArray = pTempAttr;

    // #38415# AttrArray muss richtige Spaltennummer haben
    pAttrArray->SetCol(nCol);
    rCol.pAttrArray->SetCol(rCol.nCol);

    USHORT i;
    if (pItems)
        for (i = 0; i < nCount; i++)
        {
            ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
            if( pCell->GetCellType() == CELLTYPE_FORMULA)
                pCell->aPos.SetCol(nCol);
        }
    if (rCol.pItems)
        for (i = 0; i < rCol.nCount; i++)
        {
            ScFormulaCell* pCell = (ScFormulaCell*) rCol.pItems[i].pCell;
            if( pCell->GetCellType() == CELLTYPE_FORMULA)
                pCell->aPos.SetCol(rCol.nCol);
        }
}


void ScColumn::MoveTo(USHORT nStartRow, USHORT nEndRow, ScColumn& rCol)
{
    pAttrArray->MoveTo(nStartRow, nEndRow, *rCol.pAttrArray);

    if (pItems)
    {
        USHORT nStartPos;
        USHORT nMoveCount=0;
        USHORT i;
        for (i=0; i < nCount; i++)
        {
            if ((pItems[i].nRow >= nStartRow) && (pItems[i].nRow <= nEndRow))
            {
                if (nMoveCount==0)
                    nStartPos=i;
                ++nMoveCount;

                rCol.Insert(pItems[i].nRow, pItems[i].pCell);
            }
        }
        if (nMoveCount > 0)
        {
            //  Formeln benachrichtigen, dass sich etwas aendert

            ScNoteCell* pNoteCell = new ScNoteCell;     // Dummy wie in DeleteRange
            USHORT nEndPos = nStartPos+nMoveCount-1;
            for (i=nStartPos; i<=nEndPos; i++)
                pItems[i].pCell = pNoteCell;            // nicht auf die verschobenen zugreifen
            for (i=nStartPos; i<=nEndPos; i++)
                pDocument->Broadcast( SC_HINT_DYING,
                    ScAddress( nCol, pItems[i].nRow, nTab ), NULL );    // nur Areas
            delete pNoteCell;

            nCount -= nMoveCount;
            memmove( &pItems[nStartPos], &pItems[nStartPos+nMoveCount],
                        (nCount - nStartPos) * sizeof(ColEntry) );
            pItems[nCount].nRow = 0;
            pItems[nCount].pCell = NULL;
        }
    }
}


void ScColumn::UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
             USHORT nCol2, USHORT nRow2, USHORT nTab2, short nDx, short nDy, short nDz,
             ScDocument* pUndoDoc )
{
    if (pItems)
    {
        ScRange aRange( ScAddress( nCol1, nRow1, nTab1 ),
                        ScAddress( nCol2, nRow2, nTab2 ) );
        if ( eUpdateRefMode == URM_COPY && nRow1 == nRow2 )
        {   // z.B. eine einzelne Zelle aus dem Clipboard eingefuegt
            USHORT nIndex;
            if ( Search( nRow1, nIndex ) )
            {
                ScFormulaCell* pCell = (ScFormulaCell*) pItems[nIndex].pCell;
                if( pCell->GetCellType() == CELLTYPE_FORMULA)
                    pCell->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc );
            }
        }
        else
        {
            USHORT i, nRow;
            if ( eUpdateRefMode == URM_COPY )
                Search( nRow1, i );
            else
                i = 0;
            for ( ; i < nCount; i++ )
            {
                nRow = pItems[i].nRow;
                if ( eUpdateRefMode == URM_COPY && nRow > nRow2 )
                    break;
                ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
                if( pCell->GetCellType() == CELLTYPE_FORMULA)
                    pCell->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
    }
}


void ScColumn::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    if (pItems)
        for (USHORT i=0; i<nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->UpdateTranspose( rSource, rDest, pUndoDoc );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );              // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    if (pItems)
        for (USHORT i=0; i<nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->UpdateGrow( rArea, nGrowX, nGrowY );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );              // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateInsertTab( USHORT nTable)
{
    if (nTab >= nTable) nTab++;
    if( pItems )
        UpdateInsertTabOnlyCells( nTable );
}


void ScColumn::UpdateInsertTabOnlyCells( USHORT nTable)
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
            if( pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                USHORT nRow = pItems[i].nRow;
                pCell->UpdateInsertTab(nTable);
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateInsertTabAbs(USHORT nTable)
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
            if( pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                USHORT nRow = pItems[i].nRow;
                pCell->UpdateInsertTabAbs(nTable);
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScColumn* pRefUndo )
{
    if (nTab > nTable)
        pAttrArray->SetTab(--nTab);

    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
            if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                ScFormulaCell* pOld = (ScFormulaCell*)pItems[i].pCell;

                ScFormulaCell* pSave = NULL;
                if (pRefUndo)
                    pSave = (ScFormulaCell*)pOld->Clone( pDocument,
                        ScAddress( nCol, nRow, nTab ), TRUE );

                BOOL bChanged = pOld->UpdateDeleteTab(nTable, bIsMove);
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?

                if (pRefUndo)
                {
                    if (bChanged)
                        pRefUndo->Insert( nRow, pSave );
                    else
                        delete pSave;
                }
            }
}


void ScColumn::UpdateMoveTab( USHORT nOldPos, USHORT nNewPos, USHORT nTabNo )
{
    nTab = nTabNo;
    pAttrArray->SetTab( nTabNo );
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                pCell->UpdateMoveTab( nOldPos, nNewPos, nTabNo );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::UpdateCompile( BOOL bForceIfNameInUse )
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
            if( p->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                p->UpdateCompile( bForceIfNameInUse );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::SetTabNo(USHORT nNewTab)
{
    nTab = nNewTab;
    pAttrArray->SetTab( nNewTab );
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
            if( p->GetCellType() == CELLTYPE_FORMULA )
                p->aPos.SetTab( nNewTab );
        }
}


BOOL ScColumn::IsRangeNameInUse(USHORT nRow1, USHORT nRow2, USHORT nIndex) const
{
    BOOL bInUse = FALSE;
    if (pItems)
        for (USHORT i = 0; !bInUse && (i < nCount); i++)
            if ((pItems[i].nRow >= nRow1) &&
                (pItems[i].nRow <= nRow2) &&
                (pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA))
                    bInUse = ((ScFormulaCell*)pItems[i].pCell)->IsRangeNameInUse(nIndex);
    return bInUse;
}


void ScColumn::ReplaceRangeNamesInUse(USHORT nRow1, USHORT nRow2,
                                     const ScIndexMap& rMap )
{
    BOOL bInUse = FALSE;
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            if ((pItems[i].nRow >= nRow1) &&
                (pItems[i].nRow <= nRow2) &&
                (pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA))
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pItems[i].pCell)->ReplaceRangeNamesInUse( rMap );
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::SetDirtyVar()
{
    for (USHORT i=0; i<nCount; i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
        if( p->GetCellType() == CELLTYPE_FORMULA )
            p->SetDirtyVar();
    }
}


void ScColumn::SetDirty()
{
    // wird nur dokumentweit verwendet, kein FormulaTrack
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<nCount; i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
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
    if ( !pItems || !nCount )
        return ;
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    USHORT nRow2 = rRange.aEnd.Row();
    ScAddress aPos( nCol, 0, nTab );
    USHORT nRow, nIndex;
    Search( rRange.aStart.Row(), nIndex );
    while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRow2 )
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            ((ScFormulaCell*)pCell)->SetDirty();
        else
        {
            aPos.SetRow( nRow );
            pDocument->Broadcast( SC_HINT_DATACHANGED, aPos, pCell );
        }
        nIndex++;
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetDirtyAfterLoad()
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<nCount; i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
        if ( p->GetCellType() == CELLTYPE_FORMULA && p->GetDirty() )
            p->SetDirty();
            // wenn die Zelle durch CalcAfterLoad schon bDirty war, muss
            // jetzt noch FormulaTracking stattfinden
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::SetRelNameDirty()
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<nCount; i++)
    {
        ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
        if( p->GetCellType() == CELLTYPE_FORMULA && p->HasRelNameReference() )
            p->SetDirty();
    }
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScColumn::CalcAll()
{
    if (pItems)
        for (USHORT i=0; i<nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
#ifdef DEBUG
                // nach F9 ctrl-F9: ueberprueft die Berechnung per FormulaTree
                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                double nOldVal, nNewVal;
                nOldVal = pFCell->GetValue();
#endif
                ((ScFormulaCell*)pCell)->Interpret();
#ifdef DEBUG
                if ( pFCell->GetCode()->IsRecalcModeNormal() )
                    nNewVal = pFCell->GetValue();
                else
                    nNewVal = nOldVal;  // random(), jetzt() etc.
                DBG_ASSERT( nOldVal==nNewVal, "CalcAll: nOldVal != nNewVal" );
#endif
            }
        }
}


void ScColumn::CompileAll()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                // fuer unbedingtes kompilieren
                // bCompile=TRUE und pCode->nError=0
                ((ScFormulaCell*)pCell)->GetCode()->SetError( 0 );
                ((ScFormulaCell*)pCell)->SetCompile( TRUE );
                ((ScFormulaCell*)pCell)->CompileTokenArray();
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::CompileXML()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            {
                USHORT nRow = pItems[i].nRow;
                ((ScFormulaCell*)pCell)->CompileXML();
                if ( nRow != pItems[i].nRow )
                    Search( nRow, i );      // Listener geloescht/eingefuegt?
            }
        }
}


void ScColumn::CalcAfterLoad()
{
    if (pItems)
        for (USHORT i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*)pCell)->CalcAfterLoad();
        }
}


void ScColumn::ResetChanged( USHORT nStartRow, USHORT nEndRow )
{
    if (pItems)
    {
        USHORT nIndex;
        Search(nStartRow,nIndex);
        while (nIndex<nCount && pItems[nIndex].nRow <= nEndRow)
        {
            ScBaseCell* pCell = pItems[nIndex].pCell;
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
                ((ScFormulaCell*)pCell)->ResetChanged();
            ++nIndex;
        }
    }
}


BOOL ScColumn::HasEditCells(USHORT nStartRow, USHORT nEndRow, USHORT& rFirst) const
{
    USHORT nRow;
    USHORT nIndex;
    Search(nStartRow,nIndex);
    while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : FALSE )
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        if (pCell->GetCellType() == CELLTYPE_EDIT)
        {
            rFirst = nRow;
            return TRUE;
        }
        ++nIndex;
    }

    return FALSE;
}


short ScColumn::SearchStyle( short nRow, const ScStyleSheet* pSearchStyle,
                                BOOL bUp, BOOL bInSelection, const ScMarkData& rMark )
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


BOOL ScColumn::SearchStyleRange( short& rRow, short& rEndRow, const ScStyleSheet* pSearchStyle,
                                    BOOL bUp, BOOL bInSelection, const ScMarkData& rMark )
{
    if (bInSelection)
    {
        if (rMark.IsMultiMarked())
            return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp,
                                    (ScMarkArray*) rMark.GetArray()+nCol );     //! const
        else
            return FALSE;
    }
    else
        return pAttrArray->SearchStyleRange( rRow, rEndRow, pSearchStyle, bUp, NULL );
}


