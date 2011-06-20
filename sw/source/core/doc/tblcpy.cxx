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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#define _ZFORLIST_DECLARE_TABLE
#include <svl/zforlist.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <tblsel.hxx>
#include <tabfrm.hxx>
#include <poolfmt.hxx>
#include <cellatr.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <hints.hxx>
#include <UndoTable.hxx>
#include <redline.hxx>
#include <fmtfsize.hxx>
#include <list>

sal_Bool _FndCntntLine( const SwTableLine*& rpLine, void* pPara );
sal_Bool _FndCntntBox( const SwTableBox*& rpBox, void* pPara );
void lcl_CpyBox( const SwTable& rCpyTbl, const SwTableBox* pCpyBox,
                    SwTable& rDstTbl, SwTableBox* pDstBox,
                    sal_Bool bDelCntnt, SwUndoTblCpyTbl* pUndo );

// The following type will be used by table copy functions to describe
// the structure of tables (or parts of tables).
// It's for new table model only.

namespace
{
    struct BoxSpanInfo
    {
        SwTableBox* mpBox;
        SwTableBox* mpCopy;
        sal_uInt16 mnColSpan;
        bool mbSelected;
    };

    typedef std::vector< BoxSpanInfo > BoxStructure;
    typedef std::vector< BoxStructure > LineStructure;
    typedef std::list< sal_uLong > ColumnStructure;

    struct SubBox
    {
        SwTableBox *mpBox;
        bool mbCovered;
    };

    typedef std::list< SubBox > SubLine;
    typedef std::list< SubLine > SubTable;

    class TableStructure
    {
    public:
        LineStructure maLines;
        ColumnStructure maCols;
        sal_uInt16 mnStartCol;
        sal_uInt16 mnAddLine;
        void addLine( sal_uInt16 &rLine, const SwTableBoxes&, const SwSelBoxes*,
                      bool bNewModel );
        void addBox( sal_uInt16 nLine, const SwSelBoxes*, SwTableBox *pBox,
                     sal_uLong &rnB, sal_uInt16 &rnC, ColumnStructure::iterator& rpCl,
                     BoxStructure::iterator& rpSel, bool &rbSel, bool bCover );
        void incColSpan( sal_uInt16 nLine, sal_uInt16 nCol );
        TableStructure( const SwTable& rTable );
        TableStructure( const SwTable& rTable, _FndBox &rFndBox,
                        const SwSelBoxes& rSelBoxes,
                        LineStructure::size_type nMinSize );
        LineStructure::size_type getLineCount() const
            { return maLines.size(); }
        void moreLines( const SwTable& rTable );
        void assignBoxes( const TableStructure &rSource );
        void copyBoxes( const SwTable& rSource, SwTable& rDstTbl,
                        SwUndoTblCpyTbl* pUndo ) const;
    };

    SubTable::iterator insertSubLine( SubTable& rSubTable, SwTableLine& rLine,
        SubTable::iterator pStartLn );

    SubTable::iterator insertSubBox( SubTable& rSubTable, SwTableBox& rBox,
        SubTable::iterator pStartLn, SubTable::iterator pEndLn )
    {
        if( rBox.GetTabLines().Count() )
        {
            SubTable::difference_type nSize = std::distance( pStartLn, pEndLn );
            if( nSize < rBox.GetTabLines().Count() )
            {
                SubLine aSubLine;
                SubLine::iterator pBox = pStartLn->begin();
                SubLine::iterator pEnd = pStartLn->end();
                while( pBox != pEnd )
                {
                    SubBox aSub;
                    aSub.mpBox = pBox->mpBox;
                    aSub.mbCovered = true;
                    aSubLine.push_back( aSub );
                    ++pBox;
                }
                do
                {
                    rSubTable.insert( pEndLn, aSubLine );
                } while( ++nSize < rBox.GetTabLines().Count() );
            }
            for( sal_uInt16 nLine = 0; nLine < rBox.GetTabLines().Count(); ++nLine )
                pStartLn = insertSubLine( rSubTable, *rBox.GetTabLines()[nLine],
                           pStartLn );
            OSL_ENSURE( pStartLn == pEndLn, "Sub line confusion" );
        }
        else
        {
            SubBox aSub;
            aSub.mpBox = &rBox;
            aSub.mbCovered = false;
            while( pStartLn != pEndLn )
            {
                pStartLn->push_back( aSub );
                aSub.mbCovered = true;
                ++pStartLn;
            }
        }
        return pStartLn;
    }

    SubTable::iterator insertSubLine( SubTable& rSubTable, SwTableLine& rLine,
        SubTable::iterator pStartLn )
    {
        SubTable::iterator pMax = pStartLn;
        ++pMax;
        SubTable::difference_type nMax = 1;
        for( sal_uInt16 nBox = 0; nBox < rLine.GetTabBoxes().Count(); ++nBox )
        {
            SubTable::iterator pTmp = insertSubBox( rSubTable,
                *rLine.GetTabBoxes()[nBox], pStartLn, pMax );
            SubTable::difference_type nTmp = std::distance( pStartLn, pTmp );
            if( nTmp > nMax )
            {
                pMax = pTmp;
                nMax = nTmp;
            }
        }
        return pMax;
    }

    TableStructure::TableStructure( const SwTable& rTable ) :
        maLines( rTable.GetTabLines().Count() ), mnStartCol(USHRT_MAX),
        mnAddLine(0)
    {
        maCols.push_front(0);
        const SwTableLines &rLines = rTable.GetTabLines();
        sal_uInt16 nCnt = 0;
        for( sal_uInt16 nLine = 0; nLine < rLines.Count(); ++nLine )
            addLine( nCnt, rLines[nLine]->GetTabBoxes(), 0, rTable.IsNewModel() );
    }

    TableStructure::TableStructure( const SwTable& rTable,
        _FndBox &rFndBox, const SwSelBoxes& rSelBoxes,
        LineStructure::size_type nMinSize )
        : mnStartCol(USHRT_MAX), mnAddLine(0)
    {
        if( rFndBox.GetLines().Count() )
        {
            bool bNoSelection = rSelBoxes.Count() < 2;
            _FndLines &rFndLines = rFndBox.GetLines();
            maCols.push_front(0);
            const SwTableLine* pLine = rFndLines[0]->GetLine();
            sal_uInt16 nStartLn = rTable.GetTabLines().C40_GETPOS( SwTableLine, pLine );
            sal_uInt16 nEndLn = nStartLn;
            if( rFndLines.Count() > 1 )
            {
                pLine = rFndLines[ rFndLines.Count()-1 ]->GetLine();
                nEndLn = rTable.GetTabLines().C40_GETPOS( SwTableLine, pLine );
            }
            if( nStartLn < USHRT_MAX && nEndLn < USHRT_MAX )
            {
                const SwTableLines &rLines = rTable.GetTabLines();
                if( bNoSelection &&
                    (sal_uInt16)nMinSize > nEndLn - nStartLn + 1 )
                {
                    sal_uInt16 nNewEndLn = nStartLn + (sal_uInt16)nMinSize - 1;
                    if( nNewEndLn >= rLines.Count() )
                    {
                        mnAddLine = nNewEndLn - rLines.Count() + 1;
                        nNewEndLn = rLines.Count() - 1;
                    }
                    while( nEndLn < nNewEndLn )
                    {
                        SwTableLine *pLine2 = rLines[ ++nEndLn ];
                        SwTableBox *pTmpBox = pLine2->GetTabBoxes()[0];
                        _FndLine *pInsLine = new _FndLine( pLine2, &rFndBox );
                        _FndBox *pFndBox = new _FndBox( pTmpBox, pInsLine );
                        pInsLine->GetBoxes().C40_INSERT( _FndBox, pFndBox, 0 );
                        rFndLines.C40_INSERT( _FndLine, pInsLine, rFndLines.Count() );
                    }
                }
                maLines.resize( nEndLn - nStartLn + 1 );
                const SwSelBoxes* pSelBoxes = &rSelBoxes;
                sal_uInt16 nCnt = 0;
                for( sal_uInt16 nLine = nStartLn; nLine <= nEndLn; ++nLine )
                {
                    addLine( nCnt, rLines[nLine]->GetTabBoxes(),
                             pSelBoxes, rTable.IsNewModel() );
                    if( bNoSelection )
                        pSelBoxes = 0;
                }
            }
            if( bNoSelection && mnStartCol < USHRT_MAX )
            {
                BoxStructure::iterator pC = maLines[0].begin();
                BoxStructure::iterator pEnd = maLines[0].end();
                sal_uInt16 nIdx = mnStartCol;
                mnStartCol = 0;
                while( nIdx && pC != pEnd )
                {
                    mnStartCol = mnStartCol + pC->mnColSpan;
                    --nIdx;
                    ++pC;
                }
            }
            else
                mnStartCol = USHRT_MAX;
        }
    }

    void TableStructure::addLine( sal_uInt16 &rLine, const SwTableBoxes& rBoxes,
        const SwSelBoxes* pSelBoxes, bool bNewModel )
    {
        bool bComplex = false;
        if( !bNewModel )
            for( sal_uInt16 nBox = 0; !bComplex && nBox < rBoxes.Count(); ++nBox )
                bComplex = rBoxes[nBox]->GetTabLines().Count() > 0;
        if( bComplex )
        {
            SubTable aSubTable;
            SubLine aSubLine;
            aSubTable.push_back( aSubLine );
            SubTable::iterator pStartLn = aSubTable.begin();
            SubTable::iterator pEndLn = aSubTable.end();
            for( sal_uInt16 nBox = 0; nBox < rBoxes.Count(); ++nBox )
                insertSubBox( aSubTable, *rBoxes[nBox], pStartLn, pEndLn );
            SubTable::size_type nSize = aSubTable.size();
            if( nSize )
            {
                maLines.resize( maLines.size() + nSize - 1 );
                while( pStartLn != pEndLn )
                {
                    bool bSelected = false;
                    sal_uLong nBorder = 0;
                    sal_uInt16 nCol = 0;
                    maLines[rLine].reserve( pStartLn->size() );
                    BoxStructure::iterator pSel = maLines[rLine].end();
                    ColumnStructure::iterator pCol = maCols.begin();
                    SubLine::iterator pBox = pStartLn->begin();
                    SubLine::iterator pEnd = pStartLn->end();
                    while( pBox != pEnd )
                    {
                        addBox( rLine, pSelBoxes, pBox->mpBox, nBorder, nCol,
                            pCol, pSel, bSelected, pBox->mbCovered );
                        ++pBox;
                    }
                    ++rLine;
                    ++pStartLn;
                }
            }
        }
        else
        {
            bool bSelected = false;
            sal_uLong nBorder = 0;
            sal_uInt16 nCol = 0;
            maLines[rLine].reserve( rBoxes.Count() );
            ColumnStructure::iterator pCol = maCols.begin();
            BoxStructure::iterator pSel = maLines[rLine].end();
            for( sal_uInt16 nBox = 0; nBox < rBoxes.Count(); ++nBox )
                addBox( rLine, pSelBoxes, rBoxes[nBox], nBorder, nCol,
                        pCol, pSel, bSelected, false );
            ++rLine;
        }
    }

    void TableStructure::addBox( sal_uInt16 nLine, const SwSelBoxes* pSelBoxes,
        SwTableBox *pBox, sal_uLong &rnBorder, sal_uInt16 &rnCol,
        ColumnStructure::iterator& rpCol, BoxStructure::iterator& rpSel,
        bool &rbSelected, bool bCovered )
    {
        BoxSpanInfo aInfo;
        if( pSelBoxes &&
            USHRT_MAX != pSelBoxes->GetPos( pBox ) )
        {
            aInfo.mbSelected = true;
            if( mnStartCol == USHRT_MAX )
            {
                mnStartCol = (sal_uInt16)maLines[nLine].size();
                if( pSelBoxes->Count() < 2 )
                {
                    pSelBoxes = 0;
                    aInfo.mbSelected = false;
                }
            }
        }
        else
            aInfo.mbSelected = false;
        rnBorder += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_uInt16 nLeftCol = rnCol;
        while( rpCol != maCols.end() && *rpCol < rnBorder )
        {
            ++rnCol;
            ++rpCol;
        }
        if( rpCol == maCols.end() || *rpCol > rnBorder )
        {
            maCols.insert( rpCol, rnBorder );
            --rpCol;
            incColSpan( nLine, rnCol );
        }
        aInfo.mnColSpan = rnCol - nLeftCol;
        aInfo.mpCopy = 0;
        aInfo.mpBox = bCovered ? 0 : pBox;
        maLines[nLine].push_back( aInfo );
        if( aInfo.mbSelected )
        {
            if( rbSelected )
            {
                while( rpSel != maLines[nLine].end() )
                {
                    rpSel->mbSelected = true;
                    ++rpSel;
                }
            }
            else
            {
                rpSel = maLines[nLine].end();
                rbSelected = true;
            }
            --rpSel;
        }
    }

    void TableStructure::moreLines( const SwTable& rTable )
    {
        if( mnAddLine )
        {
            const SwTableLines &rLines = rTable.GetTabLines();
            sal_uInt16 nLineCount = rLines.Count();
            if( nLineCount < mnAddLine )
                mnAddLine = nLineCount;
            sal_uInt16 nLine = (sal_uInt16)maLines.size();
            maLines.resize( nLine + mnAddLine );
            while( mnAddLine )
            {
                SwTableLine *pLine = rLines[ nLineCount - mnAddLine ];
                addLine( nLine, pLine->GetTabBoxes(), 0, rTable.IsNewModel() );
                --mnAddLine;
            }
        }
    }

    void TableStructure::incColSpan( sal_uInt16 nLineMax, sal_uInt16 nNewCol )
    {
        for( sal_uInt16 nLine = 0; nLine < nLineMax; ++nLine )
        {
            BoxStructure::iterator pInfo = maLines[nLine].begin();
            BoxStructure::iterator pEnd = maLines[nLine].end();
            long nCol = pInfo->mnColSpan;
            while( nNewCol > nCol && ++pInfo != pEnd )
                nCol += pInfo->mnColSpan;
            if( pInfo != pEnd )
                ++(pInfo->mnColSpan);
        }
    }

    void TableStructure::assignBoxes( const TableStructure &rSource )
    {
        LineStructure::const_iterator pFirstLine = rSource.maLines.begin();
        LineStructure::const_iterator pLastLine = rSource.maLines.end();
        if( pFirstLine == pLastLine )
            return;
        LineStructure::const_iterator pCurrLine = pFirstLine;
        LineStructure::size_type nLineCount = maLines.size();
        sal_uInt16 nFirstStartCol = 0;
        {
            BoxStructure::const_iterator pFirstBox = pFirstLine->begin();
            if( pFirstBox != pFirstLine->end() && pFirstBox->mpBox &&
                pFirstBox->mpBox->getDummyFlag() )
                nFirstStartCol = pFirstBox->mnColSpan;
        }
        for( LineStructure::size_type nLine = 0; nLine < nLineCount; ++nLine )
        {
            BoxStructure::const_iterator pFirstBox = pCurrLine->begin();
            BoxStructure::const_iterator pLastBox = pCurrLine->end();
            sal_uInt16 nCurrStartCol = mnStartCol;
            if( pFirstBox != pLastBox )
            {
                BoxStructure::const_iterator pTmpBox = pLastBox;
                --pTmpBox;
                if( pTmpBox->mpBox && pTmpBox->mpBox->getDummyFlag() )
                    --pLastBox;
                if( pFirstBox != pLastBox && pFirstBox->mpBox &&
                    pFirstBox->mpBox->getDummyFlag() )
                {
                    if( nCurrStartCol < USHRT_MAX )
                    {
                        if( pFirstBox->mnColSpan > nFirstStartCol )
                            nCurrStartCol = pFirstBox->mnColSpan - nFirstStartCol
                                            + nCurrStartCol;
                    }
                    ++pFirstBox;
                }
            }
            if( pFirstBox != pLastBox )
            {
                BoxStructure::const_iterator pCurrBox = pFirstBox;
                BoxStructure &rBox = maLines[nLine];
                BoxStructure::size_type nBoxCount = rBox.size();
                sal_uInt16 nCol = 0;
                for( BoxStructure::size_type nBox = 0; nBox < nBoxCount; ++nBox )
                {
                    BoxSpanInfo& rInfo = rBox[nBox];
                    nCol = nCol + rInfo.mnColSpan;
                    if( rInfo.mbSelected || nCol > nCurrStartCol )
                    {
                        rInfo.mpCopy = pCurrBox->mpBox;
                        if( rInfo.mbSelected && rInfo.mpCopy->getDummyFlag() )
                        {
                            ++pCurrBox;
                            if( pCurrBox == pLastBox )
                            {
                                pCurrBox = pFirstBox;
                                if( pCurrBox->mpBox->getDummyFlag() )
                                    ++pCurrBox;
                            }
                            rInfo.mpCopy = pCurrBox->mpBox;
                        }
                        ++pCurrBox;
                        if( pCurrBox == pLastBox )
                        {
                            if( rInfo.mbSelected )
                                pCurrBox = pFirstBox;
                            else
                            {
                                rInfo.mbSelected = rInfo.mpCopy == 0;
                                break;
                            }
                        }
                        rInfo.mbSelected = rInfo.mpCopy == 0;
                    }
                }
            }
            ++pCurrLine;
            if( pCurrLine == pLastLine )
                pCurrLine = pFirstLine;
        }
    }

    void TableStructure::copyBoxes( const SwTable& rSource, SwTable& rDstTbl,
                                    SwUndoTblCpyTbl* pUndo ) const
    {
        LineStructure::size_type nLineCount = maLines.size();
        for( LineStructure::size_type nLine = 0; nLine < nLineCount; ++nLine )
        {
            const BoxStructure &rBox = maLines[nLine];
            BoxStructure::size_type nBoxCount = rBox.size();
            for( BoxStructure::size_type nBox = 0; nBox < nBoxCount; ++nBox )
            {
                const BoxSpanInfo& rInfo = rBox[nBox];
                if( ( rInfo.mpCopy && !rInfo.mpCopy->getDummyFlag() )
                    || rInfo.mbSelected )
                {
                    SwTableBox *pBox = rInfo.mpBox;
                    if( pBox && pBox->getRowSpan() > 0 )
                        lcl_CpyBox( rSource, rInfo.mpCopy, rDstTbl, pBox,
                                    sal_True, pUndo );
                }
            }
        }
    }
}

// ---------------------------------------------------------------

// kopiere die Tabelle in diese.
//  Kopiere alle Boxen einer Line in entsprechenden Boxen. Der alte Inhalt
//  wird dabei geloescht.
//  Ist keine mehr vorhanden, kommt der restliche Inhalt in die letzte
//  Box einer "GrundLine".
//  Ist auch keine Line mehr vorhanden, -> auch in die letzte Box
//  einer "GrundLine"


void lcl_CpyBox( const SwTable& rCpyTbl, const SwTableBox* pCpyBox,
                    SwTable& rDstTbl, SwTableBox* pDstBox,
                    sal_Bool bDelCntnt, SwUndoTblCpyTbl* pUndo )
{
    OSL_ENSURE( ( !pCpyBox || pCpyBox->GetSttNd() ) && pDstBox->GetSttNd(),
            "Keine inhaltstragende Box" );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();
    SwDoc* pDoc = rDstTbl.GetFrmFmt()->GetDoc();

    // kopiere erst den neuen und loeschen dann den alten Inhalt
    // (keine leeren Section erzeugen; werden sonst geloescht!)
    std::auto_ptr< SwNodeRange > pRg( pCpyBox ?
        new SwNodeRange ( *pCpyBox->GetSttNd(), 1,
        *pCpyBox->GetSttNd()->EndOfSectionNode() ) : 0 );

    SwNodeIndex aInsIdx( *pDstBox->GetSttNd(), bDelCntnt ? 1 :
                        pDstBox->GetSttNd()->EndOfSectionIndex() -
                        pDstBox->GetSttIdx() );

    if( pUndo )
        pUndo->AddBoxBefore( *pDstBox, bDelCntnt );

    bool bUndoRedline = pUndo && pDoc->IsRedlineOn();
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwNodeIndex aSavePos( aInsIdx, -1 );
    if( pRg.get() )
        pCpyDoc->CopyWithFlyInFly( *pRg, 0, aInsIdx, sal_False );
    else
        pDoc->GetNodes().MakeTxtNode( aInsIdx, (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    aSavePos++;

    SwTableLine* pLine = pDstBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    sal_Bool bReplaceColl = sal_True;
    if( bDelCntnt && !bUndoRedline )
    {
        // zuerst die Fly loeschen, dann die entsprechenden Nodes
        SwNodeIndex aEndNdIdx( *aInsIdx.GetNode().EndOfSectionNode() );

            // Bookmarks usw. verschieben
        {
            SwPosition aMvPos( aInsIdx );
            SwCntntNode* pCNd = pDoc->GetNodes().GoPrevious( &aMvPos.nNode );
            aMvPos.nContent.Assign( pCNd, pCNd->Len() );
            pDoc->CorrAbs( aInsIdx, aEndNdIdx, aMvPos, /*sal_True*/sal_False );
        }

        // stehen noch FlyFrames rum, loesche auch diese
        for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
        {
            SwFrmFmt *const pFly = (*pDoc->GetSpzFrmFmts())[n];
            SwFmtAnchor const*const pAnchor = &pFly->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                 (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                aInsIdx <= pAPos->nNode && pAPos->nNode <= aEndNdIdx )
            {
                pDoc->DelLayoutFmt( pFly );
            }
        }

        // ist DestBox eine Headline-Box und hat Tabellen-Vorlage gesetzt,
        // dann NICHT die TabellenHeadline-Vorlage automatisch setzen
        if( 1 < rDstTbl.GetTabLines().Count() &&
            pLine == rDstTbl.GetTabLines()[0] )
        {
            SwCntntNode* pCNd = aInsIdx.GetNode().GetCntntNode();
            if( !pCNd )
            {
                SwNodeIndex aTmp( aInsIdx );
                pCNd = pDoc->GetNodes().GoNext( &aTmp );
            }

            if( pCNd &&
                RES_POOLCOLL_TABLE_HDLN !=
                    pCNd->GetFmtColl()->GetPoolFmtId() )
                bReplaceColl = sal_False;
        }

        pDoc->GetNodes().Delete( aInsIdx, aEndNdIdx.GetIndex() - aInsIdx.GetIndex() );
    }

    //b6341295: Table copy redlining will be managed by AddBoxAfter()
    if( pUndo )
        pUndo->AddBoxAfter( *pDstBox, aInsIdx, bDelCntnt );

    // heading
    SwTxtNode *const pTxtNd = aSavePos.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        sal_uInt16 nPoolId = pTxtNd->GetTxtColl()->GetPoolFmtId();
        if( bReplaceColl &&
            (( 1 < rDstTbl.GetTabLines().Count() &&
                pLine == rDstTbl.GetTabLines()[0] )
                // gilt noch die Tabellen-Inhalt ??
                ? RES_POOLCOLL_TABLE == nPoolId
                : RES_POOLCOLL_TABLE_HDLN == nPoolId ) )
        {
            SwTxtFmtColl* pColl = pDoc->GetTxtCollFromPool(
                static_cast<sal_uInt16>(
                                    RES_POOLCOLL_TABLE == nPoolId
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ) );
            if( pColl )         // Vorlage umsetzen
            {
                SwPaM aPam( aSavePos );
                aPam.SetMark();
                aPam.Move( fnMoveForward, fnGoSection );
                pDoc->SetTxtFmtColl( aPam, pColl );
            }
        }

        // loesche die akt. Formel/Format/Value Werte
        if( SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMULA ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE ) )
        {
            pDstBox->ClaimFrmFmt()->ResetFmtAttr( RES_BOXATR_FORMAT,
                                                 RES_BOXATR_VALUE );
        }

        // kopiere die TabellenBoxAttribute - Formel/Format/Value
        if( pCpyBox )
        {
            SfxItemSet aBoxAttrSet( pCpyDoc->GetAttrPool(), RES_BOXATR_FORMAT,
                                                            RES_BOXATR_VALUE );
            aBoxAttrSet.Put( pCpyBox->GetFrmFmt()->GetAttrSet() );
            if( aBoxAttrSet.Count() )
            {
                const SfxPoolItem* pItem;
                SvNumberFormatter* pN = pDoc->GetNumberFormatter( sal_False );
                if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == aBoxAttrSet.
                    GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem ) )
                {
                    sal_uLong nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                    sal_uLong nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                    if( nNewIdx != nOldIdx )
                        aBoxAttrSet.Put( SwTblBoxNumFormat( nNewIdx ));
                }
                pDstBox->ClaimFrmFmt()->SetFmtAttr( aBoxAttrSet );
            }
        }
    }
}

sal_Bool SwTable::InsNewTable( const SwTable& rCpyTbl, const SwSelBoxes& rSelBoxes,
                        SwUndoTblCpyTbl* pUndo )
{
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    // analyse source structure
    TableStructure aCopyStruct( rCpyTbl );

    // analyse target structure (from start box) and selected substructure
    _FndBox aFndBox( 0, 0 );
    {   // get all boxes/lines
        _FndPara aPara( rSelBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    TableStructure aTarget( *this, aFndBox, rSelBoxes, aCopyStruct.getLineCount() );

    bool bClear = false;
    if( aTarget.mnAddLine && IsNewModel() )
    {
        SwSelBoxes aBoxes;
        aBoxes.Insert( GetTabLines()[ GetTabLines().Count()-1 ]->GetTabBoxes()[0] );
        if( pUndo )
            pUndo->InsertRow( *this, aBoxes, aTarget.mnAddLine );
        else
            InsertRow( pDoc, aBoxes, aTarget.mnAddLine, sal_True );

        aTarget.moreLines( *this );
        bClear = true;
    }

    // find mapping, if needed extend target table and/or selection
    aTarget.assignBoxes( aCopyStruct );

    {
        // Change table formulas into relative representation
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    // delete frames
    aFndBox.SetTableLines( *this );
    if( bClear )
        aFndBox.ClearLineBehind();
    aFndBox.DelFrms( *this );

    // copy boxes
    aTarget.copyBoxes( rCpyTbl, *this, pUndo );

    // adjust row span attributes accordingly

    // make frames
    aFndBox.MakeFrms( *this );

    return sal_True;
}

// kopiere die Tabelle in diese.
//  Kopiere alle Boxen einer Line in entsprechenden Boxen. Der alte Inhalt
//  wird dabei geloescht.
//  Ist keine mehr vorhanden, kommt der restliche Inhalt in die letzte
//  Box einer "GrundLine".
//  Ist auch keine Line mehr vorhanden, -> auch in die letzte Box
//  einer "GrundLine"
sal_Bool SwTable::InsTable( const SwTable& rCpyTbl, const SwNodeIndex& rSttBox,
                        SwUndoTblCpyTbl* pUndo )
{
    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    SwTableNode* pTblNd = pDoc->IsIdxInTbl( rSttBox );

    // suche erstmal die Box, in die kopiert werden soll:
    SwTableBox* pMyBox = (SwTableBox*)GetTblBox(
            rSttBox.GetNode().FindTableBoxStartNode()->GetIndex() );

    OSL_ENSURE( pMyBox, "Index steht nicht in dieser Tabelle in einer Box" );

    // loesche erstmal die Frames der Tabelle
    _FndBox aFndBox( 0, 0 );
    aFndBox.DelFrms( pTblNd->GetTable() );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    sal_Bool bDelCntnt = sal_True;
    const SwTableBox* pTmp;

    for( sal_uInt16 nLines = 0; nLines < rCpyTbl.GetTabLines().Count(); ++nLines )
    {
        // hole die erste Box von der Copy-Line
        const SwTableBox* pCpyBox = rCpyTbl.GetTabLines()[nLines]
                                    ->GetTabBoxes()[0];
        while( pCpyBox->GetTabLines().Count() )
            pCpyBox = pCpyBox->GetTabLines()[0]->GetTabBoxes()[0];

        do {
            // kopiere erst den neuen und loeschen dann den alten Inhalt
            // (keine leeren Section erzeugen, werden sonst geloescht!)
            lcl_CpyBox( rCpyTbl, pCpyBox, *this, pMyBox, bDelCntnt, pUndo );

            if( 0 == (pTmp = pCpyBox->FindNextBox( rCpyTbl, pCpyBox, sal_False )))
                break;      // es folgt keine weitere Box mehr
            pCpyBox = pTmp;

            if( 0 == ( pTmp = pMyBox->FindNextBox( *this, pMyBox, sal_False )))
                bDelCntnt = sal_False;  // kein Platz mehr ??
            else
                pMyBox = (SwTableBox*)pTmp;

        } while( sal_True );

        // suche die oberste Line
        SwTableLine* pNxtLine = pMyBox->GetUpper();
        while( pNxtLine->GetUpper() )
            pNxtLine = pNxtLine->GetUpper()->GetUpper();
        sal_uInt16 nPos = GetTabLines().C40_GETPOS( SwTableLine, pNxtLine );
        // gibt es eine naechste ??
        if( nPos + 1 >= GetTabLines().Count() )
            bDelCntnt = sal_False;      // es gibt keine, alles in die letzte Box
        else
        {
            // suche die naechste "Inhaltstragende Box"
            pNxtLine = GetTabLines()[ nPos+1 ];
            pMyBox = pNxtLine->GetTabBoxes()[0];
            while( pMyBox->GetTabLines().Count() )
                pMyBox = pMyBox->GetTabLines()[0]->GetTabBoxes()[0];
            bDelCntnt = sal_True;
        }
    }

    aFndBox.MakeFrms( pTblNd->GetTable() );     // erzeuge die Frames neu
    return sal_True;
}

sal_Bool SwTable::InsTable( const SwTable& rCpyTbl, const SwSelBoxes& rSelBoxes,
                        SwUndoTblCpyTbl* pUndo )
{
    OSL_ENSURE( rSelBoxes.Count(), "Missing selection" );

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    if( IsNewModel() || rCpyTbl.IsNewModel() )
        return InsNewTable( rCpyTbl, rSelBoxes, pUndo );

    OSL_ENSURE( !rCpyTbl.IsTblComplex(), "Table too complex" );

            SwDoc* pDoc = GetFrmFmt()->GetDoc();
    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    SwTableBox *pTmpBox, *pSttBox = (SwTableBox*)rSelBoxes[0];

    sal_uInt16 nLn, nBx;
    _FndLine *pFLine, *pInsFLine = 0;
    _FndBox aFndBox( 0, 0 );
    // suche alle Boxen / Lines
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }

    // JP 06.09.96: Sonderfall - eine Box in der Tabelle -> in alle
    //              selektierten Boxen kopieren!
    if( 1 != rCpyTbl.GetTabSortBoxes().Count() )
    {
        SwTableLine* pSttLine = pSttBox->GetUpper();
        sal_uInt16 nSttBox = pSttLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
        sal_uInt16 nSttLine = GetTabLines().C40_GETPOS( SwTableLine, pSttLine );
        _FndBox* pFndBox;

        sal_uInt16 nFndCnt = aFndBox.GetLines().Count();
        if( !nFndCnt )
            return sal_False;

        // teste ob genug Platz fuer die einzelnen Lines und Boxen ist:
        sal_uInt16 nTstLns = 0;
        pFLine = aFndBox.GetLines()[ 0 ];
        pSttLine = pFLine->GetLine();
        nSttLine = GetTabLines().C40_GETPOS( SwTableLine, pSttLine );
        // sind ueberhaupt soviele Zeilen vorhanden
        if( 1 == nFndCnt )
        {
            // in der Tabelle noch genug Platz ??
            if( (GetTabLines().Count() - nSttLine ) <
                rCpyTbl.GetTabLines().Count() )
            {
                // sollte nicht mehr soviele Lines vorhanden sein, dann
                // teste, ob man durch einfuegen neuer zum Ziel kommt. Aber
                // nur wenn die SSelection eine Box umfasst !!
                if( 1 < rSelBoxes.Count() )
                    return sal_False;

                sal_uInt16 nNewLns = rCpyTbl.GetTabLines().Count() -
                                (GetTabLines().Count() - nSttLine );

                // Dann teste mal ob die Anzahl der Boxen fuer die Lines reicht
                SwTableLine* pLastLn = GetTabLines()[ GetTabLines().Count()-1 ];

                pSttBox = pFLine->GetBoxes()[0]->GetBox();
                nSttBox = pFLine->GetLine()->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
                for( sal_uInt16 n = rCpyTbl.GetTabLines().Count() - nNewLns;
                        n < rCpyTbl.GetTabLines().Count(); ++n )
                {
                    SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ n ];

                    if( pLastLn->GetTabBoxes().Count() < nSttBox ||
                        ( pLastLn->GetTabBoxes().Count() - nSttBox ) <
                            pCpyLn->GetTabBoxes().Count() )
                        return sal_False;

                    // Test auf Verschachtelungen
                    for( nBx = 0; nBx < pCpyLn->GetTabBoxes().Count(); ++nBx )
                        if( !( pTmpBox = pLastLn->GetTabBoxes()[ nSttBox + nBx ])
                                    ->GetSttNd() )
                            return sal_False;
                }
                // es ist also Platz fuer das zu kopierende vorhanden, also
                // fuege entsprechend neue Zeilen ein.
                SwTableBox* pInsBox = pLastLn->GetTabBoxes()[ nSttBox ];
                OSL_ENSURE( pInsBox && pInsBox->GetSttNd(),
                    "kein CntntBox oder steht nicht in dieser Tabelle" );
                SwSelBoxes aBoxes;

                if( pUndo
                    ? !pUndo->InsertRow( *this, SelLineFromBox( pInsBox,
                                aBoxes, sal_True ), nNewLns )
                    : !InsertRow( pDoc, SelLineFromBox( pInsBox,
                                aBoxes, sal_True ), nNewLns, sal_True ) )
                    return sal_False;
            }

            nTstLns = rCpyTbl.GetTabLines().Count();        // soviele Kopieren
        }
        else if( 0 == (nFndCnt % rCpyTbl.GetTabLines().Count()) )
            nTstLns = nFndCnt;
        else
            return sal_False;       // kein Platz fuer die Zeilen

        for( nLn = 0; nLn < nTstLns; ++nLn )
        {
            // Zeilen sind genug vorhanden, dann ueberpruefe die Boxen
            // je Zeile
            pFLine = aFndBox.GetLines()[ nLn % nFndCnt ];
            SwTableLine* pLine = pFLine->GetLine();
            pSttBox = pFLine->GetBoxes()[0]->GetBox();
            nSttBox = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
            if( nLn >= nFndCnt )
            {
                // es sind im ClipBoard mehr Zeilen als selectiert wurden
                pInsFLine = new _FndLine( GetTabLines()[ nSttLine + nLn ],
                                        &aFndBox );
                pLine = pInsFLine->GetLine();
            }
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ nLn %
                                        rCpyTbl.GetTabLines().Count() ];

            // zu wenig Zeilen selektiert ?
            if( pInsFLine )
            {
                // eine neue Zeile wird in die FndBox eingefuegt,
                if( pLine->GetTabBoxes().Count() < nSttBox ||
                    ( pLine->GetTabBoxes().Count() - nSttBox ) <
                    pFLine->GetBoxes().Count() )
                    return sal_False;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                        return sal_False;
                    // wenn Ok, fuege die Box in die FndLine zu
                    pFndBox = new _FndBox( pTmpBox, pInsFLine );
                    pInsFLine->GetBoxes().C40_INSERT( _FndBox, pFndBox, nBx );
                }
                aFndBox.GetLines().C40_INSERT( _FndLine, pInsFLine, nLn );
            }
            else if( pFLine->GetBoxes().Count() == 1 )
            {
                if( pLine->GetTabBoxes().Count() < nSttBox  ||
                    ( pLine->GetTabBoxes().Count() - nSttBox ) <
                    pCpyLn->GetTabBoxes().Count() )
                    return sal_False;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pCpyLn->GetTabBoxes().Count(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                        return sal_False;
                    // wenn Ok, fuege die Box in die FndLine zu
                    if( nBx == pFLine->GetBoxes().Count() )
                    {
                        pFndBox = new _FndBox( pTmpBox, pFLine );
                        pFLine->GetBoxes().C40_INSERT( _FndBox, pFndBox, nBx );
                    }
                }
            }
            else
            {
                // ueberpruefe die selektierten Boxen mit denen im Clipboard
                // (n-Fach)
                if( 0 != ( pFLine->GetBoxes().Count() %
                            pCpyLn->GetTabBoxes().Count() ))
                    return sal_False;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
                    if( !pFLine->GetBoxes()[ nBx ]->GetBox()->GetSttNd() )
                        return sal_False;
            }
        }

        if( !aFndBox.GetLines().Count() )
            return sal_False;
    }

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    // loesche die Frames
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );

    if( 1 == rCpyTbl.GetTabSortBoxes().Count() )
    {
        SwTableBox *pTmpBx = rCpyTbl.GetTabSortBoxes()[0];
        for( sal_uInt16 n = 0; n < rSelBoxes.Count(); ++n )
            lcl_CpyBox( rCpyTbl, pTmpBx, *this,
                        (SwTableBox*)rSelBoxes[n], sal_True, pUndo );
    }
    else
        for( nLn = 0; nLn < aFndBox.GetLines().Count(); ++nLn )
        {
            pFLine = aFndBox.GetLines()[ nLn ];
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[
                                nLn % rCpyTbl.GetTabLines().Count() ];
            for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
            {
                // Kopiere in pMyBox die pCpyBox
                lcl_CpyBox( rCpyTbl, pCpyLn->GetTabBoxes()[
                            nBx % pCpyLn->GetTabBoxes().Count() ],
                    *this, pFLine->GetBoxes()[ nBx ]->GetBox(), sal_True, pUndo );
            }
        }

    aFndBox.MakeFrms( *this );
    return sal_True;
}

sal_Bool _FndCntntBox( const SwTableBox*& rpBox, void* pPara )
{
    SwTableBox* pBox = (SwTableBox*)rpBox;
    if( rpBox->GetTabLines().Count() )
        pBox->GetTabLines().ForEach( &_FndCntntLine, pPara );
    else
        ((SwSelBoxes*)pPara)->Insert( pBox );
    return sal_True;
}

sal_Bool _FndCntntLine( const SwTableLine*& rpLine, void* pPara )
{
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &_FndCntntBox, pPara );
    return sal_True;
}

// suche alle Inhaltstragenden-Boxen dieser Box
SwSelBoxes& SwTable::SelLineFromBox( const SwTableBox* pBox,
                                    SwSelBoxes& rBoxes, sal_Bool bToTop ) const
{
    SwTableLine* pLine = (SwTableLine*)pBox->GetUpper();
    if( bToTop )
        while( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();

    // alle alten loeschen
    rBoxes.Remove( sal_uInt16(0), rBoxes.Count() );
    pLine->GetTabBoxes().ForEach( &_FndCntntBox, &rBoxes );
    return rBoxes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
