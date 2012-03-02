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


#include "hintids.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <tabcol.hxx>
#include <frmatr.hxx>
#include <cellfrm.hxx>
#include <tabfrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include "pam.hxx"
#include "swcrsr.hxx"
#include "viscrs.hxx"
#include "swtable.hxx"
#include "htmltbl.hxx"
#include "tblsel.hxx"
#include "swtblfmt.hxx"
#include "docary.hxx"
#include "ndindex.hxx"
#include "undobj.hxx"
#include "switerator.hxx"
#include <UndoTable.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;


extern void ClearFEShellTabCols();

//siehe auch swtable.cxx
#define COLFUZZY 20L

inline sal_Bool IsSame( long nA, long nB ) { return  Abs(nA-nB) <= COLFUZZY; }

class SwTblFmtCmp
{
public:
    SwFrmFmt *pOld,
             *pNew;
    sal_Int16     nType;

    SwTblFmtCmp( SwFrmFmt *pOld, SwFrmFmt *pNew, sal_Int16 nType );

    static SwFrmFmt *FindNewFmt( SvPtrarr &rArr, SwFrmFmt*pOld, sal_Int16 nType );
    static void Delete( SvPtrarr &rArr );
};


SwTblFmtCmp::SwTblFmtCmp( SwFrmFmt *pO, SwFrmFmt *pN, sal_Int16 nT )
    : pOld ( pO ), pNew ( pN ), nType( nT )
{
}

SwFrmFmt *SwTblFmtCmp::FindNewFmt( SvPtrarr &rArr, SwFrmFmt *pOld, sal_Int16 nType )
{
    for ( sal_uInt16 i = 0; i < rArr.Count(); ++i )
    {
        SwTblFmtCmp *pCmp = (SwTblFmtCmp*)rArr[i];
        if ( pCmp->pOld == pOld && pCmp->nType == nType )
            return pCmp->pNew;
    }
    return 0;
}

void SwTblFmtCmp::Delete( SvPtrarr &rArr )
{
    for ( sal_uInt16 i = 0; i < rArr.Count(); ++i )
        delete (SwTblFmtCmp*)rArr[i];
}

void lcl_GetStartEndCell( const SwCursor& rCrsr,
                        SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
{
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( sal_False ),
            "Tabselection nicht auf Cnt." );

    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }

    // robust:
    SwCntntNode* pPointNd = rCrsr.GetCntntNode();
    SwCntntNode* pMarkNd  = rCrsr.GetCntntNode(sal_False);

    SwFrm* pPointFrm = pPointNd ? pPointNd->getLayoutFrm( pPointNd->GetDoc()->GetCurrentLayout(), &aPtPos ) : 0;
    SwFrm* pMarkFrm  = pMarkNd  ? pMarkNd->getLayoutFrm( pMarkNd->GetDoc()->GetCurrentLayout(), &aMkPos )  : 0;

    prStart = pPointFrm ? pPointFrm->GetUpper() : 0;
    prEnd   = pMarkFrm  ? pMarkFrm->GetUpper() : 0;
}

sal_Bool lcl_GetBoxSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                    sal_Bool bAllCrsr = sal_False )
{
    const SwTableCursor* pTblCrsr =
        dynamic_cast<const SwTableCursor*>(&rCursor);
    if( pTblCrsr )
        ::GetTblSelCrs( *pTblCrsr, rBoxes );
    else
    {
        const SwPaM *pCurPam = &rCursor, *pSttPam = pCurPam;
        do {
            const SwNode* pNd = pCurPam->GetNode()->FindTableBoxStartNode();
            if( pNd )
            {
                SwTableBox* pBox = (SwTableBox*)pNd->FindTableNode()->GetTable().
                                            GetTblBox( pNd->GetIndex() );
                rBoxes.insert( pBox );
            }
        } while( bAllCrsr &&
                pSttPam != ( pCurPam = (SwPaM*)pCurPam->GetNext()) );
    }
    return !rBoxes.empty();
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetRowHeight(), GetRowHeight()
#***********************************************************************/
//Die Zeilenhoehe wird ausgehend von der Selektion ermittelt/gesetzt.
//Ausgehend von jeder Zelle innerhalb der Selektion werden nach oben alle
//Zeilen abgeklappert, die oberste Zeile erhaelt den gewuenschten Wert alle
//tieferliegenden Zeilen einen entsprechenden Wert der sich aus der
//Relation der alten und neuen Groesse der obersten Zeile und ihrer
//eigenen Groesse ergiebt.
//Alle veraenderten Zeilen erhalten ggf. ein eigenes FrmFmt.
//Natuerlich darf jede Zeile nur einmal angefasst werden.

inline void InsertLine( SvPtrarr& rLineArr, SwTableLine* pLine )
{
    if( USHRT_MAX == rLineArr.GetPos( pLine ) )
        rLineArr.Insert( pLine, rLineArr.Count() );
}

//-----------------------------------------------------------------------------

sal_Bool lcl_IsAnLower( const SwTableLine *pLine, const SwTableLine *pAssumed )
{
    const SwTableLine *pTmp = pAssumed->GetUpper() ?
                                    pAssumed->GetUpper()->GetUpper() : 0;
    while ( pTmp )
    {
        if ( pTmp == pLine )
            return sal_True;
        pTmp = pTmp->GetUpper() ? pTmp->GetUpper()->GetUpper() : 0;
    }
    return sal_False;
}
//-----------------------------------------------------------------------------

struct LinesAndTable
{
          SvPtrarr &rLines;
    const SwTable  &rTable;
          sal_Bool      bInsertLines;

    LinesAndTable( SvPtrarr &rL, const SwTable &rTbl ) :
          rLines( rL ), rTable( rTbl ), bInsertLines( sal_True ) {}
};


sal_Bool _FindLine( const _FndLine*& rpLine, void* pPara );

sal_Bool _FindBox( const _FndBox*& rpBox, void* pPara )
{
    if ( rpBox->GetLines().Count() )
    {
        ((LinesAndTable*)pPara)->bInsertLines = sal_True;
        ((_FndBox*)rpBox)->GetLines().ForEach( _FindLine, pPara );
        if ( ((LinesAndTable*)pPara)->bInsertLines )
        {
            const SwTableLines &rLines = rpBox->GetBox()
                                    ? rpBox->GetBox()->GetTabLines()
                                    : ((LinesAndTable*)pPara)->rTable.GetTabLines();
            if ( rpBox->GetLines().Count() == rLines.Count() )
            {
                for ( sal_uInt16 i = 0; i < rLines.Count(); ++i )
                    ::InsertLine( ((LinesAndTable*)pPara)->rLines,
                                  (SwTableLine*)rLines[i] );
            }
            else
                ((LinesAndTable*)pPara)->bInsertLines = sal_False;
        }
    }
    else if ( rpBox->GetBox() )
        ::InsertLine( ((LinesAndTable*)pPara)->rLines,
                      (SwTableLine*)rpBox->GetBox()->GetUpper() );
    return sal_True;
}

sal_Bool _FindLine( const _FndLine*& rpLine, void* pPara )
{
    ((_FndLine*)rpLine)->GetBoxes().ForEach( _FindBox, pPara );
    return sal_True;
}

void lcl_CollectLines( SvPtrarr &rArr, const SwCursor& rCursor, bool bRemoveLines )
{
    //Zuerst die selektierten Boxen einsammeln.
    SwSelBoxes aBoxes;
    if( !::lcl_GetBoxSel( rCursor, aBoxes ))
        return ;

    //Die selektierte Struktur kopieren.
    const SwTable &rTable = aBoxes.begin()->second->GetSttNd()->FindTableNode()->GetTable();
    LinesAndTable aPara( rArr, rTable );
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aTmpPara( aBoxes, &aFndBox );
        ((SwTableLines&)rTable.GetTabLines()).ForEach( &_FndLineCopyCol, &aTmpPara );
    }

    //Diejenigen Lines einsammeln, die nur selektierte Boxen enthalten.
    const _FndBox *pTmp = &aFndBox;
    ::_FindBox( pTmp, &aPara );

    // Remove lines, that have a common superordinate row.
    // (Not for row split)
    if ( bRemoveLines )
    {
        for ( sal_uInt16 i = 0; i < rArr.Count(); ++i )
        {
            SwTableLine *pUpLine = (SwTableLine*)rArr[i];
            for ( sal_uInt16 k = 0; k < rArr.Count(); ++k )
            {
                if ( k != i && ::lcl_IsAnLower( pUpLine, (SwTableLine*)rArr[k] ) )
                {
                    rArr.Remove( k );
                    if ( k <= i )
                        --i;
                    --k;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessRowAttr( SvPtrarr& rFmtCmp, SwTableLine* pLine, const SfxPoolItem& rNew )
{
    SwFrmFmt *pNewFmt;
    if ( 0 != (pNewFmt = SwTblFmtCmp::FindNewFmt( rFmtCmp, pLine->GetFrmFmt(), 0 )))
        pLine->ChgFrmFmt( (SwTableLineFmt*)pNewFmt );
    else
    {
        SwFrmFmt *pOld = pLine->GetFrmFmt();
        SwFrmFmt *pNew = pLine->ClaimFrmFmt();
        pNew->SetFmtAttr( rNew );
        rFmtCmp.Insert( new SwTblFmtCmp( pOld, pNew, 0 ), rFmtCmp.Count());
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( SvPtrarr &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew );

void lcl_ProcessRowSize( SvPtrarr &rFmtCmp, SwTableLine *pLine, const SwFmtFrmSize &rNew )
{
    lcl_ProcessRowAttr( rFmtCmp, pLine, rNew );
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( sal_uInt16 i = 0; i < rBoxes.Count(); ++i )
        ::lcl_ProcessBoxSize( rFmtCmp, rBoxes[i], rNew );
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( SvPtrarr &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew )
{
    SwTableLines &rLines = pBox->GetTabLines();
    if ( rLines.Count() )
    {
        SwFmtFrmSize aSz( rNew );
        aSz.SetHeight( rNew.GetHeight() ? rNew.GetHeight() / rLines.Count() : 0 );
        for ( sal_uInt16 i = 0; i < rLines.Count(); ++i )
            ::lcl_ProcessRowSize( rFmtCmp, rLines[i], aSz );
    }
}
/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetTabBorders(), GetTabBorders()
#***********************************************************************/
inline void InsertCell( SvPtrarr& rCellArr, SwCellFrm* pCellFrm )
{
    if( USHRT_MAX == rCellArr.GetPos( pCellFrm ) )
        rCellArr.Insert( pCellFrm, rCellArr.Count() );
}

//-----------------------------------------------------------------------------
void lcl_CollectCells( SvPtrarr &rArr, const SwRect &rUnion,
                          SwTabFrm *pTab )
{
    SwLayoutFrm *pCell = pTab->FirstCell();
    do
    {
        // Wenn in der Zelle ein spaltiger Bereich sitzt, muessen wir
        // uns erst wieder zur Zelle hochhangeln
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell, "Frame ist keine Zelle." );
        if ( rUnion.IsOver( pCell->Frm() ) )
            ::InsertCell( rArr, (SwCellFrm*)pCell );
        //Dafuer sorgen, dass die Zelle auch verlassen wird (Bereiche)
        SwLayoutFrm *pTmp = pCell;
        do
        {   pTmp = pTmp->GetNextLayoutLeaf();
        } while ( pCell->IsAnLower( pTmp ) );
        pCell = pTmp;
    } while( pCell && pTab->IsAnLower( pCell ) );
}

void lcl_SetLineStyle( SvxBorderLine *pToSet,
                          const Color *pColor, const SvxBorderLine *pBorderLine)
{
    if ( pBorderLine )
    {
        if ( !pColor )
        {
            Color aTmp( pToSet->GetColor() );
            *pToSet = *pBorderLine;
            pToSet->SetColor( aTmp );
        }
        else
            *pToSet = *pBorderLine;
    }
    if ( pColor )
        pToSet->SetColor( *pColor );
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  AdjustCellWidth()
#***********************************************************************/
sal_uInt16 lcl_CalcCellFit( const SwLayoutFrm *pCell )
{
    SwTwips nRet = 0;
    const SwFrm *pFrm = pCell->Lower(); //Die ganze Zelle.
    SWRECTFN( pCell )
    while ( pFrm )
    {
        const SwTwips nAdd = (pFrm->Frm().*fnRect->fnGetWidth)() -
                             (pFrm->Prt().*fnRect->fnGetWidth)();

        // pFrm does not necessarily have to be a SwTxtFrm!
        const SwTwips nCalcFitToContent = pFrm->IsTxtFrm() ?
                                          ((SwTxtFrm*)pFrm)->CalcFitToContent() :
                                          (pFrm->Prt().*fnRect->fnGetWidth)();

        nRet = Max( nRet, nCalcFitToContent + nAdd );
        pFrm = pFrm->GetNext();
    }
    //Umrandung und linker/rechter Rand wollen mit kalkuliert werden.
    nRet += (pCell->Frm().*fnRect->fnGetWidth)() -
            (pCell->Prt().*fnRect->fnGetWidth)();

    //Um Rechenungenauikeiten, die spaeter bei SwTable::SetTabCols enstehen,
    //auszugleichen, addieren wir noch ein bischen.
    nRet += COLFUZZY;
    return (sal_uInt16)Max( long(MINLAY), nRet );
}

/*Die Zelle ist in der Selektion, wird aber nicht von den TabCols beschrieben.
 *Das bedeutet, dass die Zelle aufgrund der zweidimensionalen Darstellung von
 *anderen Zellen "geteilt" wurde. Wir muessen also den Wunsch- bzw. Minimalwert
 *der Zelle auf die Spalten, durch die sie geteilt wurde verteilen.
 *
 *Dazu sammeln wir zuerst die Spalten - nicht die Spaltentrenner! - ein, die
 *sich mit der Zelle ueberschneiden. Den Wunschwert der Zelle verteilen wir
 *dann anhand des Betrages der Ueberschneidung auf die Zellen.
 *Wenn eine Zelle bereits einen groesseren Wunschwert angemeldet hat, so bleibt
 *dieser erhalten, kleinere Wuensche werden ueberschrieben.
 */

void lcl_CalcSubColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                              const SwLayoutFrm *pCell, const SwLayoutFrm *pTab,
                              sal_Bool bWishValues )
{
    const sal_uInt16 nWish = bWishValues ?
                    ::lcl_CalcCellFit( pCell ) :
                    MINLAY + sal_uInt16(pCell->Frm().Width() - pCell->Prt().Width());

    SWRECTFN( pTab )

    for ( sal_uInt16 i = 0 ; i <= rCols.Count(); ++i )
    {
        long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
        long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];
        nColLeft  += rCols.GetLeftMin();
        nColRight += rCols.GetLeftMin();

        //Werte auf die Verhaeltnisse der Tabelle (Follows) anpassen.
        if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
        {
            const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
            nColLeft  += nDiff;
            nColRight += nDiff;
        }
        const long nCellLeft  = (pCell->Frm().*fnRect->fnGetLeft)();
        const long nCellRight = (pCell->Frm().*fnRect->fnGetRight)();

        //Ueberschneidungsbetrag ermitteln.
        long nWidth = 0;
        if ( nColLeft <= nCellLeft && nColRight >= (nCellLeft+COLFUZZY) )
            nWidth = nColRight - nCellLeft;
        else if ( nColLeft <= (nCellRight-COLFUZZY) && nColRight >= nCellRight )
            nWidth = nCellRight - nColLeft;
        else if ( nColLeft >= nCellLeft && nColRight <= nCellRight )
            nWidth = nColRight - nColLeft;
        if ( nWidth && pCell->Frm().Width() )
        {
            long nTmp = nWidth * nWish / pCell->Frm().Width();
            if ( sal_uInt16(nTmp) > rToFill[i] )
                rToFill[i] = sal_uInt16(nTmp);
        }
    }
}

/*Besorgt neue Werte zu Einstellung der TabCols.
 *Es wird nicht ueber die Eintrage in den TabCols itereriert, sondern
 *quasi ueber die Zwischenraeume, die ja die Zellen beschreiben.
 *
 *bWishValues == sal_True:  Es werden zur aktuellen Selektion bzw. zur aktuellen
 *                      Zelle die Wunschwerte aller betroffen Zellen ermittelt.
 *                      Sind mehrere Zellen in einer Spalte, so wird der
 *                      groesste Wunschwert als Ergebnis geliefert.
 *                      Fuer die TabCol-Eintraege, zu denen keine Zellen
 *                      ermittelt wurden, werden 0-en eingetragen.
 *
 *bWishValues == sal_False: Die Selektion wird senkrecht ausgedehnt. Zu jeder
 *                      Spalte in den TabCols, die sich mit der Selektion
 *                      schneidet wird der Minimalwert ermittelt.
 */

void lcl_CalcColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                           const SwLayoutFrm *pStart, const SwLayoutFrm *pEnd,
                           sal_Bool bWishValues )
{
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd,
                    bWishValues ? nsSwTblSearchType::TBLSEARCH_NONE : nsSwTblSearchType::TBLSEARCH_COL );

    for ( sal_uInt16 i2 = 0; i2 < aUnions.Count(); ++i2 )
    {
        SwSelUnion *pSelUnion = aUnions[i2];
        const SwTabFrm *pTab = pSelUnion->GetTable();
        const SwRect &rUnion = pSelUnion->GetUnion();

        SWRECTFN( pTab )
        sal_Bool bRTL = pTab->IsRightToLeft();

        const SwLayoutFrm *pCell = pTab->FirstCell();
        do
        {
            if ( pCell->IsCellFrm() && pCell->FindTabFrm() == pTab && ::IsFrmInTblSel( rUnion, pCell ) )
            {
                const long nCLeft  = (pCell->Frm().*fnRect->fnGetLeft)();
                const long nCRight = (pCell->Frm().*fnRect->fnGetRight)();

                sal_Bool bNotInCols = sal_True;

                for ( sal_uInt16 i = 0; i <= rCols.Count(); ++i )
                {
                    sal_uInt16 nFit = rToFill[i];
                    long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
                    long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];

                    if ( bRTL )
                    {
                        long nTmpRight = nColRight;
                        nColRight = rCols.GetRight() - nColLeft;
                        nColLeft = rCols.GetRight() - nTmpRight;
                    }

                    nColLeft  += rCols.GetLeftMin();
                    nColRight += rCols.GetLeftMin();

                    //Werte auf die Verhaeltnisse der Tabelle (Follows) anpassen.
                    long nLeftA  = nColLeft;
                    long nRightA = nColRight;
                    if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
                    {
                        const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
                        nLeftA  += nDiff;
                        nRightA += nDiff;
                    }

                    //Wir wollen nicht allzu genau hinsehen.
                    if ( ::IsSame(nCLeft, nLeftA) && ::IsSame(nCRight, nRightA))
                    {
                        bNotInCols = sal_False;
                        if ( bWishValues )
                        {
                            const sal_uInt16 nWish = ::lcl_CalcCellFit( pCell );
                            if ( nWish > nFit )
                                nFit = nWish;
                        }
                        else
                        {   const sal_uInt16 nMin = MINLAY + sal_uInt16(pCell->Frm().Width() -
                                                                pCell->Prt().Width());
                            if ( !nFit || nMin < nFit )
                                nFit = nMin;
                        }
                        if ( rToFill[i] < nFit )
                            rToFill[i] = nFit;
                    }
                }
                if ( bNotInCols )
                    ::lcl_CalcSubColValues( rToFill, rCols, pCell, pTab, bWishValues );
            }
            do {
                pCell = pCell->GetNextLayoutLeaf();
            }while( pCell && pCell->Frm().Width() == 0 );
        } while ( pCell && pTab->IsAnLower( pCell ) );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
