/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>

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
#include <boost/foreach.hpp>

static void lcl_CpyBox( const SwTable& rCpyTbl, const SwTableBox* pCpyBox,
                    SwTable& rDstTbl, SwTableBox* pDstBox,
                    bool bDelCntnt, SwUndoTblCpyTbl* pUndo );





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
        if( !rBox.GetTabLines().empty() )
        {
            SubTable::difference_type nSize = std::distance( pStartLn, pEndLn );
            if( nSize < (sal_uInt16)rBox.GetTabLines().size() )
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
                } while( ++nSize < (sal_uInt16)rBox.GetTabLines().size() );
            }
            for( sal_uInt16 nLine = 0; nLine < rBox.GetTabLines().size(); ++nLine )
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
        for( sal_uInt16 nBox = 0; nBox < rLine.GetTabBoxes().size(); ++nBox )
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
        maLines( rTable.GetTabLines().size() ), mnStartCol(USHRT_MAX),
        mnAddLine(0)
    {
        maCols.push_front(0);
        const SwTableLines &rLines = rTable.GetTabLines();
        sal_uInt16 nCnt = 0;
        for( sal_uInt16 nLine = 0; nLine < rLines.size(); ++nLine )
            addLine( nCnt, rLines[nLine]->GetTabBoxes(), 0, rTable.IsNewModel() );
    }

    TableStructure::TableStructure( const SwTable& rTable,
        _FndBox &rFndBox, const SwSelBoxes& rSelBoxes,
        LineStructure::size_type nMinSize )
        : mnStartCol(USHRT_MAX), mnAddLine(0)
    {
        if( !rFndBox.GetLines().empty() )
        {
            bool bNoSelection = rSelBoxes.size() < 2;
            _FndLines &rFndLines = rFndBox.GetLines();
            maCols.push_front(0);
            const SwTableLine* pLine = rFndLines.front().GetLine();
            sal_uInt16 nStartLn = rTable.GetTabLines().GetPos( pLine );
            sal_uInt16 nEndLn = nStartLn;
            if( rFndLines.size() > 1 )
            {
                pLine = rFndLines.back().GetLine();
                nEndLn = rTable.GetTabLines().GetPos( pLine );
            }
            if( nStartLn < USHRT_MAX && nEndLn < USHRT_MAX )
            {
                const SwTableLines &rLines = rTable.GetTabLines();
                if( bNoSelection &&
                    (sal_uInt16)nMinSize > nEndLn - nStartLn + 1 )
                {
                    sal_uInt16 nNewEndLn = nStartLn + (sal_uInt16)nMinSize - 1;
                    if( nNewEndLn >= rLines.size() )
                    {
                        mnAddLine = nNewEndLn - rLines.size() + 1;
                        nNewEndLn = rLines.size() - 1;
                    }
                    while( nEndLn < nNewEndLn )
                    {
                        SwTableLine *pLine2 = rLines[ ++nEndLn ];
                        SwTableBox *pTmpBox = pLine2->GetTabBoxes()[0];
                        _FndLine *pInsLine = new _FndLine( pLine2, &rFndBox );
                        _FndBox *pFndBox = new _FndBox( pTmpBox, pInsLine );
                        pInsLine->GetBoxes().insert(pInsLine->GetBoxes().begin(), pFndBox);
                        rFndLines.push_back( pInsLine );
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
            for( sal_uInt16 nBox = 0; !bComplex && nBox < rBoxes.size(); ++nBox )
                bComplex = !rBoxes[nBox]->GetTabLines().empty();
        if( bComplex )
        {
            SubTable aSubTable;
            SubLine aSubLine;
            aSubTable.push_back( aSubLine );
            SubTable::iterator pStartLn = aSubTable.begin();
            SubTable::iterator pEndLn = aSubTable.end();
            for( sal_uInt16 nBox = 0; nBox < rBoxes.size(); ++nBox )
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
            maLines[rLine].reserve( rBoxes.size() );
            ColumnStructure::iterator pCol = maCols.begin();
            BoxStructure::iterator pSel = maLines[rLine].end();
            for( sal_uInt16 nBox = 0; nBox < rBoxes.size(); ++nBox )
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
            pSelBoxes->end() != pSelBoxes->find( pBox ) )
        {
            aInfo.mbSelected = true;
            if( mnStartCol == USHRT_MAX )
            {
                mnStartCol = (sal_uInt16)maLines[nLine].size();
                if( pSelBoxes->size() < 2 )
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
            sal_uInt16 nLineCount = rLines.size();
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
                                    true, pUndo );
                }
            }
        }
    }
}

/** Copy Table into this Box.
    Copy all Boxes of a Line into the corresponding Boxes. The old content
    is deleted by doing this.
    If no Box is left the remaining content goes to the Box of a "BaseLine".
    If there's no Line anymore, put it also into the last Box of a "BaseLine". */
static void lcl_CpyBox( const SwTable& rCpyTbl, const SwTableBox* pCpyBox,
                    SwTable& rDstTbl, SwTableBox* pDstBox,
                    bool bDelCntnt, SwUndoTblCpyTbl* pUndo )
{
    OSL_ENSURE( ( !pCpyBox || pCpyBox->GetSttNd() ) && pDstBox->GetSttNd(),
            "No content in this Box" );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();
    SwDoc* pDoc = rDstTbl.GetFrmFmt()->GetDoc();

    
    
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
        pCpyDoc->CopyWithFlyInFly( *pRg, 0, aInsIdx, NULL, sal_False );
    else
        pDoc->GetNodes().MakeTxtNode( aInsIdx, (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    ++aSavePos;

    SwTableLine* pLine = pDstBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    bool bReplaceColl = true;
    if( bDelCntnt && !bUndoRedline )
    {
        
        SwNodeIndex aEndNdIdx( *aInsIdx.GetNode().EndOfSectionNode() );

        
        {
            SwPosition aMvPos( aInsIdx );
            SwCntntNode* pCNd = pDoc->GetNodes().GoPrevious( &aMvPos.nNode );
            aMvPos.nContent.Assign( pCNd, pCNd->Len() );
            pDoc->CorrAbs( aInsIdx, aEndNdIdx, aMvPos, /*sal_True*/sal_False );
        }

        
        for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->size(); ++n )
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

        
        
        if( 1 < rDstTbl.GetTabLines().size() &&
            pLine == rDstTbl.GetTabLines().front() )
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
                bReplaceColl = false;
        }

        pDoc->GetNodes().Delete( aInsIdx, aEndNdIdx.GetIndex() - aInsIdx.GetIndex() );
    }

    
    if( pUndo )
        pUndo->AddBoxAfter( *pDstBox, aInsIdx, bDelCntnt );

    
    SwTxtNode *const pTxtNd = aSavePos.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        sal_uInt16 nPoolId = pTxtNd->GetTxtColl()->GetPoolFmtId();
        if( bReplaceColl &&
            (( 1 < rDstTbl.GetTabLines().size() &&
                pLine == rDstTbl.GetTabLines().front() )
                
                ? RES_POOLCOLL_TABLE == nPoolId
                : RES_POOLCOLL_TABLE_HDLN == nPoolId ) )
        {
            SwTxtFmtColl* pColl = pDoc->GetTxtCollFromPool(
                static_cast<sal_uInt16>(
                                    RES_POOLCOLL_TABLE == nPoolId
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ) );
            if( pColl )         
            {
                SwPaM aPam( aSavePos );
                aPam.SetMark();
                aPam.Move( fnMoveForward, fnGoSection );
                pDoc->SetTxtFmtColl( aPam, pColl );
            }
        }

        
        if( SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMULA ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE ) )
        {
            pDstBox->ClaimFrmFmt()->ResetFmtAttr( RES_BOXATR_FORMAT,
                                                 RES_BOXATR_VALUE );
        }

        
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
                    GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
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

    
    TableStructure aCopyStruct( rCpyTbl );

    
    _FndBox aFndBox( 0, 0 );
    {   
        _FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    TableStructure aTarget( *this, aFndBox, rSelBoxes, aCopyStruct.getLineCount() );

    bool bClear = false;
    if( aTarget.mnAddLine && IsNewModel() )
    {
        SwSelBoxes aBoxes;
        aBoxes.insert( GetTabLines().back()->GetTabBoxes().front() );
        if( pUndo )
            pUndo->InsertRow( *this, aBoxes, aTarget.mnAddLine );
        else
            InsertRow( pDoc, aBoxes, aTarget.mnAddLine, true );

        aTarget.moreLines( *this );
        bClear = true;
    }

    
    aTarget.assignBoxes( aCopyStruct );

    {
        
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    
    aFndBox.SetTableLines( *this );
    if( bClear )
        aFndBox.ClearLineBehind();
    aFndBox.DelFrms( *this );

    
    aTarget.copyBoxes( rCpyTbl, *this, pUndo );

    

    
    aFndBox.MakeFrms( *this );

    return sal_True;
}

/** Copy Table into this Box.
    Copy all Boxes of a Line into the corresponding Boxes. The old content is
    deleted by doing this.
    If no Box is left the remaining content goes to the Box of a "BaseLine".
    If there's no Line anymore, put it also into the last Box of a "BaseLine". */
sal_Bool SwTable::InsTable( const SwTable& rCpyTbl, const SwNodeIndex& rSttBox,
                        SwUndoTblCpyTbl* pUndo )
{
    SetHTMLTableLayout( 0 );    

    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    SwTableNode* pTblNd = pDoc->IsIdxInTbl( rSttBox );

    
    SwTableBox* pMyBox = (SwTableBox*)GetTblBox(
            rSttBox.GetNode().FindTableBoxStartNode()->GetIndex() );

    OSL_ENSURE( pMyBox, "Index is not in a Box in this Table" );

    
    _FndBox aFndBox( 0, 0 );
    aFndBox.DelFrms( pTblNd->GetTable() );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    {
        
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    bool bDelCntnt = true;
    const SwTableBox* pTmp;

    for( sal_uInt16 nLines = 0; nLines < rCpyTbl.GetTabLines().size(); ++nLines )
    {
        
        const SwTableBox* pCpyBox = rCpyTbl.GetTabLines()[nLines]
                                    ->GetTabBoxes().front();
        while( !pCpyBox->GetTabLines().empty() )
            pCpyBox = pCpyBox->GetTabLines().front()->GetTabBoxes().front();

        do {
            
            
            lcl_CpyBox( rCpyTbl, pCpyBox, *this, pMyBox, bDelCntnt, pUndo );

            if( 0 == (pTmp = pCpyBox->FindNextBox( rCpyTbl, pCpyBox, false )))
                break;      
            pCpyBox = pTmp;

            if( 0 == ( pTmp = pMyBox->FindNextBox( *this, pMyBox, false )))
                bDelCntnt = false;  
            else
                pMyBox = (SwTableBox*)pTmp;

        } while( true );

        
        SwTableLine* pNxtLine = pMyBox->GetUpper();
        while( pNxtLine->GetUpper() )
            pNxtLine = pNxtLine->GetUpper()->GetUpper();
        sal_uInt16 nPos = GetTabLines().GetPos( pNxtLine );
        
        if( nPos + 1 >= (sal_uInt16)GetTabLines().size() )
            bDelCntnt = false;      
        else
        {
            
            pNxtLine = GetTabLines()[ nPos+1 ];
            pMyBox = pNxtLine->GetTabBoxes().front();
            while( !pMyBox->GetTabLines().empty() )
                pMyBox = pMyBox->GetTabLines().front()->GetTabBoxes().front();
            bDelCntnt = true;
        }
    }

    aFndBox.MakeFrms( pTblNd->GetTable() );     
    return sal_True;
}

sal_Bool SwTable::InsTable( const SwTable& rCpyTbl, const SwSelBoxes& rSelBoxes,
                        SwUndoTblCpyTbl* pUndo )
{
    OSL_ENSURE( !rSelBoxes.empty(), "Missing selection" );

    SetHTMLTableLayout( 0 );    

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
    
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }

    
    
    if( 1 != rCpyTbl.GetTabSortBoxes().size() )
    {
        SwTableLine* pSttLine = pSttBox->GetUpper();
        sal_uInt16 nSttLine = GetTabLines().GetPos( pSttLine );
        _FndBox* pFndBox;

        sal_uInt16 nFndCnt = aFndBox.GetLines().size();
        if( !nFndCnt )
            return sal_False;

        
        sal_uInt16 nTstLns = 0;
        pFLine = &aFndBox.GetLines().front();
        pSttLine = pFLine->GetLine();
        nSttLine = GetTabLines().GetPos( pSttLine );
        
        if( 1 == nFndCnt )
        {
            
            if( (GetTabLines().size() - nSttLine ) <
                rCpyTbl.GetTabLines().size() )
            {
                
                
                
                if( 1 < rSelBoxes.size() )
                    return sal_False;

                sal_uInt16 nNewLns = rCpyTbl.GetTabLines().size() -
                                (GetTabLines().size() - nSttLine );

                
                SwTableLine* pLastLn = GetTabLines().back();

                pSttBox = pFLine->GetBoxes()[0].GetBox();
                sal_uInt16 nSttBox = pFLine->GetLine()->GetTabBoxes().GetPos( pSttBox );
                for( sal_uInt16 n = rCpyTbl.GetTabLines().size() - nNewLns;
                        n < rCpyTbl.GetTabLines().size(); ++n )
                {
                    SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ n ];

                    if( pLastLn->GetTabBoxes().size() < nSttBox ||
                        ( pLastLn->GetTabBoxes().size() - nSttBox ) <
                            pCpyLn->GetTabBoxes().size() )
                        return sal_False;

                    
                    for( nBx = 0; nBx < pCpyLn->GetTabBoxes().size(); ++nBx )
                        if( !( pTmpBox = pLastLn->GetTabBoxes()[ nSttBox + nBx ])
                                    ->GetSttNd() )
                            return sal_False;
                }
                
                
                SwTableBox* pInsBox = pLastLn->GetTabBoxes()[ nSttBox ];
                OSL_ENSURE( pInsBox && pInsBox->GetSttNd(),
                    "no CntntBox or it's not in this Table" );
                SwSelBoxes aBoxes;

                if( pUndo
                    ? !pUndo->InsertRow( *this, SelLineFromBox( pInsBox,
                                aBoxes, true ), nNewLns )
                    : !InsertRow( pDoc, SelLineFromBox( pInsBox,
                                aBoxes, true ), nNewLns, true ) )
                    return sal_False;
            }

            nTstLns = rCpyTbl.GetTabLines().size();        
        }
        else if( 0 == (nFndCnt % rCpyTbl.GetTabLines().size()) )
            nTstLns = nFndCnt;
        else
            return sal_False;       

        for( nLn = 0; nLn < nTstLns; ++nLn )
        {
            
            pFLine = &aFndBox.GetLines()[ nLn % nFndCnt ];
            SwTableLine* pLine = pFLine->GetLine();
            pSttBox = pFLine->GetBoxes()[0].GetBox();
            sal_uInt16 nSttBox = pLine->GetTabBoxes().GetPos( pSttBox );
            if( nLn >= nFndCnt )
            {
                
                pInsFLine = new _FndLine( GetTabLines()[ nSttLine + nLn ],
                                        &aFndBox );
                pLine = pInsFLine->GetLine();
            }
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ nLn %
                                        rCpyTbl.GetTabLines().size() ];

            
            if( pInsFLine )
            {
                
                if( pLine->GetTabBoxes().size() < nSttBox ||
                    sal::static_int_cast< sal_uInt16 >(
                        pLine->GetTabBoxes().size() - nSttBox ) <
                    pFLine->GetBoxes().size() )
                {
                    delete pInsFLine;
                    return sal_False;
                }

                
                for( nBx = 0; nBx < pFLine->GetBoxes().size(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                    {
                        delete pInsFLine;
                        return sal_False;
                    }
                    
                    pFndBox = new _FndBox( pTmpBox, pInsFLine );
                    pInsFLine->GetBoxes().insert( pInsFLine->GetBoxes().begin() + nBx, pFndBox );
                }
                aFndBox.GetLines().insert( aFndBox.GetLines().begin() + nLn, pInsFLine );
            }
            else if( pFLine->GetBoxes().size() == 1 )
            {
                if( pLine->GetTabBoxes().size() < nSttBox  ||
                    ( pLine->GetTabBoxes().size() - nSttBox ) <
                    pCpyLn->GetTabBoxes().size() )
                    return sal_False;

                
                for( nBx = 0; nBx < pCpyLn->GetTabBoxes().size(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                        return sal_False;
                    
                    if( nBx == pFLine->GetBoxes().size() )
                    {
                        pFndBox = new _FndBox( pTmpBox, pFLine );
                        pFLine->GetBoxes().insert( pFLine->GetBoxes().begin() + nBx, pFndBox );
                    }
                }
            }
            else
            {
                
                
                if( 0 != ( pFLine->GetBoxes().size() %
                            pCpyLn->GetTabBoxes().size() ))
                    return sal_False;

                
                for( nBx = 0; nBx < pFLine->GetBoxes().size(); ++nBx )
                    if (!pFLine->GetBoxes()[nBx].GetBox()->GetSttNd())
                        return sal_False;
            }
        }

        if( aFndBox.GetLines().empty() )
            return sal_False;
    }

    {
        
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    
    aFndBox.SetTableLines( *this );
    
    aFndBox.DelFrms( *this,sal_False );

    if( 1 == rCpyTbl.GetTabSortBoxes().size() )
    {
        SwTableBox *pTmpBx = rCpyTbl.GetTabSortBoxes()[0];
        for (size_t n = 0; n < rSelBoxes.size(); ++n)
        {
            lcl_CpyBox( rCpyTbl, pTmpBx, *this,
                        (SwTableBox*)rSelBoxes[n], true, pUndo );
        }
    }
    else
        for( nLn = 0; nLn < aFndBox.GetLines().size(); ++nLn )
        {
            pFLine = &aFndBox.GetLines()[ nLn ];
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[
                                nLn % rCpyTbl.GetTabLines().size() ];
            for( nBx = 0; nBx < pFLine->GetBoxes().size(); ++nBx )
            {
                
                lcl_CpyBox( rCpyTbl, pCpyLn->GetTabBoxes()[
                            nBx % pCpyLn->GetTabBoxes().size() ],
                    *this, pFLine->GetBoxes()[nBx].GetBox(), true, pUndo );
            }
        }

    aFndBox.MakeFrms( *this );
    return sal_True;
}

static void _FndCntntLine( const SwTableLine* pLine, SwSelBoxes* pPara );

static void _FndCntntBox( const SwTableBox* pBox, SwSelBoxes* pPara )
{
    if( !pBox->GetTabLines().empty() )
    {
        BOOST_FOREACH( const SwTableLine* pLine, pBox->GetTabLines() )
            _FndCntntLine( pLine, pPara );
    }
    else
        pPara->insert( (SwTableBox*)pBox );
}

static void _FndCntntLine( const SwTableLine* pLine, SwSelBoxes* pPara )
{
    BOOST_FOREACH( const SwTableBox* pBox, pLine->GetTabBoxes() )
        _FndCntntBox(pBox, pPara );
}


SwSelBoxes& SwTable::SelLineFromBox( const SwTableBox* pBox,
                                    SwSelBoxes& rBoxes, bool bToTop ) const
{
    SwTableLine* pLine = (SwTableLine*)pBox->GetUpper();
    if( bToTop )
        while( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();

    
    rBoxes.clear();
    for( SwTableBoxes::iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        _FndCntntBox(*it, &rBoxes );
    return rBoxes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
