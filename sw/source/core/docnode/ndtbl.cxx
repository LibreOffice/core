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


#include <com/sun/star/chart2/XChartDocument.hpp>
#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/boxitem.hxx>
#include <svl/stritem.hxx>
// #i17174#
#include <editeng/shaditem.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtfordr.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <fmtlsplt.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <cellfrm.hxx>
#include <pagefrm.hxx>
#include <tabcol.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <UndoManager.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <viscrs.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <tblsel.hxx>
#include <fldbas.hxx>
#include <poolfmt.hxx>
#include <tabfrm.hxx>
#include <UndoCore.hxx>
#include <UndoRedline.hxx>
#include <UndoDelete.hxx>
#include <UndoNumbering.hxx>
#include <UndoTable.hxx>
#include <hints.hxx>
#include <tblafmt.hxx>
#include <swcache.hxx>
#include <ddefld.hxx>
#include <frminf.hxx>
#include <cellatr.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <rolbck.hxx>
#include <tblrwcl.hxx>
#include <editsh.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <section.hxx>
#include <frmtool.hxx>
#include <node2lay.hxx>
#include <comcore.hrc>
#include "docsh.hxx"
#include <unochart.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <map>
#include <algorithm>
#include <rootfrm.hxx>
#include <fldupde.hxx>
#include <switerator.hxx>
#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

const sal_Unicode T2T_PARA = 0x0a;

extern void ClearFEShellTabCols();

// steht im gctable.cxx
extern sal_Bool lcl_GC_Line_Border( const SwTableLine*& , void* pPara );

void lcl_SetDfltBoxAttr( SwFrmFmt& rFmt, sal_uInt8 nId )
{
    sal_Bool bTop = sal_False, bBottom = sal_False, bLeft = sal_False, bRight = sal_False;
    switch ( nId )
    {
    case 0: bTop = bBottom = bLeft = sal_True;          break;
    case 1: bTop = bBottom = bLeft = bRight = sal_True; break;
    case 2: bBottom = bLeft = sal_True;                 break;
    case 3: bBottom = bLeft = bRight = sal_True;        break;
    }

    const sal_Bool bHTML = rFmt.getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE);
    Color aCol( bHTML ? COL_GRAY : COL_BLACK );
    SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
    if ( bHTML )
    {
        aLine.SetStyle( editeng::DOUBLE );
        aLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    SvxBoxItem aBox(RES_BOX); aBox.SetDistance( 55 );
    if ( bTop )
        aBox.SetLine( &aLine, BOX_LINE_TOP );
    if ( bBottom )
        aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
    if ( bLeft )
        aBox.SetLine( &aLine, BOX_LINE_LEFT );
    if ( bRight )
        aBox.SetLine( &aLine, BOX_LINE_RIGHT );
    rFmt.SetFmtAttr( aBox );
}

void lcl_SetDfltBoxAttr( SwTableBox& rBox, SvPtrarr &rBoxFmtArr, sal_uInt8 nId,
                            const SwTableAutoFmt* pAutoFmt = 0 )
{
    SvPtrarr* pArr = (SvPtrarr*)rBoxFmtArr[ nId ];
    if( !pArr )
    {
        pArr = new SvPtrarr;
        rBoxFmtArr.Replace( pArr, nId );
    }

    SwTableBoxFmt* pNewBoxFmt = 0;
    SwFrmFmt* pBoxFmt = rBox.GetFrmFmt();
    for( sal_uInt16 n = 0; n < pArr->Count(); n += 2 )
        if( pArr->GetObject( n ) == pBoxFmt )
        {
            pNewBoxFmt = (SwTableBoxFmt*)pArr->GetObject( n + 1 );
            break;
        }

    if( !pNewBoxFmt )
    {
        SwDoc* pDoc = pBoxFmt->GetDoc();
        // das Format ist also nicht vorhanden, also neu erzeugen
        pNewBoxFmt = pDoc->MakeTableBoxFmt();
        pNewBoxFmt->SetFmtAttr( pBoxFmt->GetAttrSet().Get( RES_FRM_SIZE ) );

        if( pAutoFmt )
            pAutoFmt->UpdateToSet( nId, (SfxItemSet&)pNewBoxFmt->GetAttrSet(),
                                    SwTableAutoFmt::UPDATE_BOX,
                                    pDoc->GetNumberFormatter( sal_True ) );
        else
            ::lcl_SetDfltBoxAttr( *pNewBoxFmt, nId );

        void* p = pBoxFmt;
        pArr->Insert( p, pArr->Count() );
        p = pNewBoxFmt;
        pArr->Insert( p, pArr->Count() );
    }
    rBox.ChgFrmFmt( pNewBoxFmt );
}

SwTableBoxFmt *lcl_CreateDfltBoxFmt( SwDoc &rDoc, SvPtrarr &rBoxFmtArr,
                                    sal_uInt16 nCols, sal_uInt8 nId )
{
    if ( !rBoxFmtArr[nId] )
    {
        SwTableBoxFmt* pBoxFmt = rDoc.MakeTableBoxFmt();
        if( USHRT_MAX != nCols )
            pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        ::lcl_SetDfltBoxAttr( *pBoxFmt, nId );
        rBoxFmtArr.Replace( pBoxFmt, nId );
    }
    return (SwTableBoxFmt*)rBoxFmtArr[nId];
}

SwTableBoxFmt *lcl_CreateAFmtBoxFmt( SwDoc &rDoc, SvPtrarr &rBoxFmtArr,
                                    const SwTableAutoFmt& rAutoFmt,
                                    sal_uInt16 nCols, sal_uInt8 nId )
{
    if( !rBoxFmtArr[nId] )
    {
        SwTableBoxFmt* pBoxFmt = rDoc.MakeTableBoxFmt();
        rAutoFmt.UpdateToSet( nId, (SfxItemSet&)pBoxFmt->GetAttrSet(),
                                SwTableAutoFmt::UPDATE_BOX,
                                rDoc.GetNumberFormatter( sal_True ) );
        if( USHRT_MAX != nCols )
            pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        rBoxFmtArr.Replace( pBoxFmt, nId );
    }
    return (SwTableBoxFmt*)rBoxFmtArr[nId];
}

// --------------- einfuegen einer neuen Box --------------

    // fuege in der Line, vor der InsPos eine neue Box ein.

sal_Bool SwNodes::InsBoxen( SwTableNode* pTblNd,
                        SwTableLine* pLine,
                        SwTableBoxFmt* pBoxFmt,
                        SwTxtFmtColl* pTxtColl,
                        const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos,
                        sal_uInt16 nCnt )
{
    if( !nCnt )
        return sal_False;
    OSL_ENSURE( pLine, "keine gueltige Zeile" );

    // Index hinter die letzte Box der Line
    sal_uLong nIdxPos = 0;
    SwTableBox *pPrvBox = 0, *pNxtBox = 0;
    if( pLine->GetTabBoxes().Count() )
    {
        if( nInsPos < pLine->GetTabBoxes().Count() )
        {
            if( 0 == (pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos ] )))
                pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );
        }
        else
        {
            if( 0 == (pNxtBox = pLine->FindNextBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes()[ pLine->GetTabBoxes().Count()-1 ] )))
                pNxtBox = pLine->FindNextBox( pTblNd->GetTable() );
        }
    }
    else if( 0 == ( pNxtBox = pLine->FindNextBox( pTblNd->GetTable() )))
        pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );

    if( !pPrvBox && !pNxtBox )
    {
        sal_Bool bSetIdxPos = sal_True;
        if( pTblNd->GetTable().GetTabLines().Count() && !nInsPos )
        {
            const SwTableLine* pTblLn = pLine;
            while( pTblLn->GetUpper() )
                pTblLn = pTblLn->GetUpper()->GetUpper();

            if( pTblNd->GetTable().GetTabLines()[ 0 ] == pTblLn )
            {
                // also vor die erste Box der Tabelle
                while( ( pNxtBox = pLine->GetTabBoxes()[0])->GetTabLines().Count() )
                    pLine = pNxtBox->GetTabLines()[0];
                nIdxPos = pNxtBox->GetSttIdx();
                bSetIdxPos = sal_False;
            }
        }
        if( bSetIdxPos )
            // Tabelle ohne irgendeinen Inhalt oder am Ende, also vors Ende
            nIdxPos = pTblNd->EndOfSectionIndex();
    }
    else if( pNxtBox )          // es gibt einen Nachfolger
        nIdxPos = pNxtBox->GetSttIdx();
    else                        // es gibt einen Vorgaenger
        nIdxPos = pPrvBox->GetSttNd()->EndOfSectionIndex() + 1;

    SwNodeIndex aEndIdx( *this, nIdxPos );
    for( sal_uInt16 n = 0; n < nCnt; ++n )
    {
        SwStartNode* pSttNd = new SwStartNode( aEndIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
        pSttNd->pStartOfSection = pTblNd;
        new SwEndNode( aEndIdx, *pSttNd );

        pPrvBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );

        SwTableBoxes & rTabBoxes = pLine->GetTabBoxes();
        sal_uInt16 nRealInsPos = nInsPos + n;
        if (nRealInsPos > rTabBoxes.Count())
            nRealInsPos = rTabBoxes.Count();

        rTabBoxes.C40_INSERT( SwTableBox, pPrvBox, nRealInsPos );

        if( ! pTxtColl->IsAssignedToListLevelOfOutlineStyle()//<-end,zhaojianwei
//FEATURE::CONDCOLL
            && RES_CONDTXTFMTCOLL != pTxtColl->Which()
//FEATURE::CONDCOLL
        )
            new SwTxtNode( SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                                pTxtColl, pAutoAttr );
        else
        {
            // Outline-Numerierung richtig behandeln !!!
            SwTxtNode* pTNd = new SwTxtNode(
                            SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                            (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl(),
                            pAutoAttr );
            pTNd->ChgFmtColl( pTxtColl );
        }
    }
    return sal_True;
}

// --------------- einfuegen einer neuen Tabelle --------------

SwTableNode* SwNodes::InsertTable( const SwNodeIndex& rNdIdx,
                                   sal_uInt16 nBoxes,
                                   SwTxtFmtColl* pCntntTxtColl,
                                   sal_uInt16 nLines,
                                   sal_uInt16 nRepeat,
                                   SwTxtFmtColl* pHeadlineTxtColl,
                                   const SwAttrSet * pAttrSet)
{
    if( !nBoxes )
        return 0;

    // wenn Lines angegeben, erzeuge die Matrix aus Lines & Boxen
    if( !pHeadlineTxtColl || !nLines )
        pHeadlineTxtColl = pCntntTxtColl;

    SwTableNode * pTblNd = new SwTableNode( rNdIdx );
    SwEndNode* pEndNd = new SwEndNode( rNdIdx, *pTblNd );

    if( !nLines )       // fuer die FOR-Schleife
        ++nLines;

    SwNodeIndex aIdx( *pEndNd );
    SwTxtFmtColl* pTxtColl = pHeadlineTxtColl;
    for( sal_uInt16 nL = 0; nL < nLines; ++nL )
    {
        for( sal_uInt16 nB = 0; nB < nBoxes; ++nB )
        {
            SwStartNode* pSttNd = new SwStartNode( aIdx, ND_STARTNODE,
                                                    SwTableBoxStartNode );
            pSttNd->pStartOfSection = pTblNd;

            SwTxtNode * pTmpNd = new SwTxtNode( aIdx, pTxtColl );

            // #i60422# Propagate some more attributes.
            const SfxPoolItem* pItem = NULL;
            if ( NULL != pAttrSet )
            {
                static const sal_uInt16 aPropagateItems[] = {
                    RES_PARATR_ADJUST,
                    RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                    RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
                    RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE, 0 };

                const sal_uInt16* pIdx = aPropagateItems;
                while ( *pIdx != 0 )
                {
                    if ( SFX_ITEM_SET != pTmpNd->GetSwAttrSet().GetItemState( *pIdx ) &&
                         SFX_ITEM_SET == pAttrSet->GetItemState( *pIdx, sal_True, &pItem ) )
                        static_cast<SwCntntNode *>(pTmpNd)->SetAttr(*pItem);
                    ++pIdx;
                }
            }

            new SwEndNode( aIdx, *pSttNd );
        }
        if ( nL + 1 >= nRepeat )
            pTxtColl = pCntntTxtColl;
    }
    return pTblNd;
}


//---------------- Text -> Tabelle -----------------------


SwTableNode* SwNodes::TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                    SwTableFmt* pTblFmt,
                                    SwTableLineFmt* pLineFmt,
                                    SwTableBoxFmt* pBoxFmt,
                                    SwTxtFmtColl* pTxtColl,
                                    SwUndoTxtToTbl* pUndo )
{
    if( rRange.aStart >= rRange.aEnd )
        return 0;

    SwTableNode * pTblNd = new SwTableNode( rRange.aStart );
    new SwEndNode( rRange.aEnd, *pTblNd );

    SwDoc* pDoc = GetDoc();
    std::vector<sal_uInt16> aPosArr;
    SwTable * pTable = &pTblNd->GetTable();
    SwTableLine* pLine;
    SwTableBox* pBox;
    sal_uInt16 nBoxes, nLines, nMaxBoxes = 0;

    SwNodeIndex aSttIdx( *pTblNd, 1 );
    SwNodeIndex aEndIdx( rRange.aEnd, -1 );
    for( nLines = 0, nBoxes = 0;
        aSttIdx.GetIndex() < aEndIdx.GetIndex();
        aSttIdx += 2, nLines++, nBoxes = 0 )
    {
        SwTxtNode* pTxtNd = aSttIdx.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNd, "nur TextNodes in der Tabelle aufnehmen" );

        if( !nLines && 0x0b == cCh )
        {
            cCh = 0x09;

            // JP 28.10.96: vom 1. Node die Positionen des Trenners besorgen,
            //              damit die Boxen entsprechend eingestellt werden
            SwTxtFrmInfo aFInfo( (SwTxtFrm*)pTxtNd->getLayoutFrm( pTxtNd->GetDoc()->GetCurrentLayout() ) );
            if( aFInfo.IsOneLine() )        // nur dann sinnvoll!
            {
                const sal_Unicode* pTxt = pTxtNd->GetTxt().GetBuffer();
                for( xub_StrLen nChPos = 0; *pTxt; ++nChPos, ++pTxt )
                {
                    if( *pTxt == cCh )
                    {
                        aPosArr.push_back( static_cast<sal_uInt16>(
                                        aFInfo.GetCharPos( nChPos+1, sal_False )) );
                    }
                }

                aPosArr.push_back(
                                static_cast<sal_uInt16>(aFInfo.GetFrm()->IsVertical() ?
                                aFInfo.GetFrm()->Prt().Bottom() :
                                aFInfo.GetFrm()->Prt().Right()) );

            }
        }

        // die alten Frames loeschen, es werden neue erzeugt
        pTxtNd->DelFrms();

        // PageBreaks/PageDesc/ColBreak rausschmeissen.
        const SfxItemSet* pSet = pTxtNd->GetpSwAttrSet();
        if( pSet )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pItem ) )
            {
                if( !nLines )
                    pTblFmt->SetFmtAttr( *pItem );
                pTxtNd->ResetAttr( RES_BREAK );
                pSet = pTxtNd->GetpSwAttrSet();
            }

            if( pSet && SFX_ITEM_SET == pSet->GetItemState(
                RES_PAGEDESC, sal_False, &pItem ) &&
                ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                if( !nLines )
                    pTblFmt->SetFmtAttr( *pItem );
                pTxtNd->ResetAttr( RES_PAGEDESC );
            }
        }

        // setze den bei allen TextNode in der Tabelle den TableNode
        // als StartNode
        pTxtNd->pStartOfSection = pTblNd;

        pLine = new SwTableLine( pLineFmt, 1, 0 );
        pTable->GetTabLines().C40_INSERT( SwTableLine, pLine, nLines );

        SwStartNode* pSttNd;
        SwPosition aCntPos( aSttIdx, SwIndex( pTxtNd ));

        std::vector<sal_uLong> aBkmkArr;
        _SaveCntntIdx( pDoc, aSttIdx.GetIndex(), pTxtNd->GetTxt().Len(), aBkmkArr );

        const sal_Unicode* pTxt = pTxtNd->GetTxt().GetBuffer();

        if( T2T_PARA != cCh )
            for( xub_StrLen nChPos = 0; *pTxt; ++nChPos, ++pTxt )
                if( *pTxt == cCh )
                {
                    aCntPos.nContent = nChPos;
                    SwCntntNode* pNewNd = pTxtNd->SplitCntntNode( aCntPos );

                    if( !aBkmkArr.empty() )
                        _RestoreCntntIdx( aBkmkArr, *pNewNd, nChPos,
                                            nChPos + 1 );

                    // Trennzeichen loeschen und SuchString korrigieren
                    pTxtNd->EraseText( aCntPos.nContent, 1 );
                    pTxt = pTxtNd->GetTxt().GetBuffer();
                    nChPos = 0;
                    --nChPos, --pTxt;           // for the ++ in the for loop !!!

                    // setze bei allen TextNodes in der Tabelle den TableNode
                    // als StartNode
                    const SwNodeIndex aTmpIdx( aCntPos.nNode, -1 );
                    pSttNd = new SwStartNode( aTmpIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
                    new SwEndNode( aCntPos.nNode, *pSttNd );
                    pNewNd->pStartOfSection = pSttNd;

                    // Section der Box zuweisen
                    pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
                    pLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, nBoxes++ );
                }

        // und jetzt den letzten Teil-String
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( aBkmkArr, *pTxtNd, pTxtNd->GetTxt().Len(),
                                pTxtNd->GetTxt().Len()+1 );

        pSttNd = new SwStartNode( aCntPos.nNode, ND_STARTNODE, SwTableBoxStartNode );
        const SwNodeIndex aTmpIdx( aCntPos.nNode, 1 );
        new SwEndNode( aTmpIdx, *pSttNd  );
        pTxtNd->pStartOfSection = pSttNd;

        pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
        pLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, nBoxes++ );
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    // die Tabelle ausgleichen, leere Sections einfuegen
    sal_uInt16 n;

    for( n = 0; n < pTable->GetTabLines().Count(); ++n )
    {
        SwTableLine* pCurrLine = pTable->GetTabLines()[ n ];
        if( nMaxBoxes != ( nBoxes = pCurrLine->GetTabBoxes().Count() ))
        {
            InsBoxen( pTblNd, pCurrLine, pBoxFmt, pTxtColl, 0,
                        nBoxes, nMaxBoxes - nBoxes );

            if( pUndo )
                for( sal_uInt16 i = nBoxes; i < nMaxBoxes; ++i )
                    pUndo->AddFillBox( *pCurrLine->GetTabBoxes()[ i ] );

            // fehlen der 1. Line Boxen, dann kann man das Breiten Array
            // vergessen!
            if( !n )
                aPosArr.clear();
        }
    }

    if( !aPosArr.empty() )
    {
        SwTableLines& rLns = pTable->GetTabLines();
        sal_uInt16 nLastPos = 0;
        for( n = 0; n < aPosArr.size(); ++n )
        {
            SwTableBoxFmt *pNewFmt = pDoc->MakeTableBoxFmt();
            pNewFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                aPosArr[ n ] - nLastPos ));
            for( sal_uInt16 nTmpLine = 0; nTmpLine < rLns.Count(); ++nTmpLine )
                //JP 24.06.98: hier muss ein Add erfolgen, da das BoxFormat
                //              von der rufenden Methode noch gebraucht wird!
                pNewFmt->Add( rLns[ nTmpLine ]->GetTabBoxes()[ n ] );

            nLastPos = aPosArr[ n ];
        }

        // damit die Tabelle die richtige Groesse bekommt, im BoxFormat die
        // Groesse nach "oben" transportieren.
        OSL_ENSURE( !pBoxFmt->GetDepends(), "wer ist in dem Format noch angemeldet" );
        pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nLastPos ));
    }
    else
        pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX / nMaxBoxes ));

    // das wars doch wohl ??
    return pTblNd;
}

SwNodeRange * SwNodes::ExpandRangeForTableBox(const SwNodeRange & rRange)
{
    SwNodeRange * pResult = NULL;
    bool bChanged = false;

    SwNodeIndex aNewStart = rRange.aStart;
    SwNodeIndex aNewEnd = rRange.aEnd;

    SwNodeIndex aEndIndex = rRange.aEnd;
    SwNodeIndex aIndex = rRange.aStart;

    while (aIndex < aEndIndex)
    {
        SwNode& rNode = aIndex.GetNode();

        if (rNode.IsStartNode())
        {
            // advance aIndex to the end node of this start node
            SwNode * pEndNode = rNode.EndOfSectionNode();
            aIndex = *pEndNode;

            if (aIndex > aNewEnd)
            {
                aNewEnd = aIndex;
                bChanged = true;
            }
        }
        else if (rNode.IsEndNode())
        {
            SwNode * pStartNode = rNode.StartOfSectionNode();
            SwNodeIndex aStartIndex = *pStartNode;

            if (aStartIndex < aNewStart)
            {
                aNewStart = aStartIndex;
                bChanged = true;
            }
        }

        if (aIndex < aEndIndex)
            ++aIndex;
    }

    SwNode * pNode = &aIndex.GetNode();
    while (pNode->IsEndNode())
    {
        SwNode * pStartNode = pNode->StartOfSectionNode();
        SwNodeIndex aStartIndex(*pStartNode);
        aNewStart = aStartIndex;
        aNewEnd = aIndex;
        bChanged = true;

        ++aIndex;
        pNode = &aIndex.GetNode();
    }

    if (bChanged)
        pResult = new SwNodeRange(aNewStart, aNewEnd);

    return pResult;
}

SwTableNode* SwNodes::TextToTable( const SwNodes::TableRanges_t & rTableNodes,
                                    SwTableFmt* pTblFmt,
                                    SwTableLineFmt* pLineFmt,
                                    SwTableBoxFmt* pBoxFmt,
                                    SwTxtFmtColl* /*pTxtColl*/  /*, SwUndo... pUndo*/  )
{
    if( !rTableNodes.size() )
        return 0;

    SwTableNode * pTblNd = new SwTableNode( rTableNodes.begin()->begin()->aStart );
    //insert the end node after the last text node
   SwNodeIndex aInsertIndex( rTableNodes.rbegin()->rbegin()->aEnd );
   ++aInsertIndex;

   //!! owner ship will be transferred in c-tor to SwNodes array.
   //!! Thus no real problem here...
   new SwEndNode( aInsertIndex, *pTblNd );

#if OSL_DEBUG_LEVEL > 1
    /**debug**/
    const SwNodeRange& rStartRange = *rTableNodes.begin()->begin();
    const SwNodeRange& rEndRange = *rTableNodes.rbegin()->rbegin();
    (void) rStartRange;
    (void) rEndRange;
    /**debug**/
#endif

    SwDoc* pDoc = GetDoc();
    std::vector<sal_uInt16> aPosArr;
    SwTable * pTable = &pTblNd->GetTable();
    SwTableLine* pLine;
    SwTableBox* pBox;
    sal_uInt16 nBoxes, nLines, nMaxBoxes = 0;

    SwNodeIndex aNodeIndex = rTableNodes.begin()->begin()->aStart;
    // delete frames of all contained content nodes
    for( nLines = 0; aNodeIndex <= rTableNodes.rbegin()->rbegin()->aEnd; ++aNodeIndex,++nLines )
    {
        SwNode& rNode = aNodeIndex.GetNode();
        if( rNode.IsCntntNode() )
        {
            static_cast<SwCntntNode&>(rNode).DelFrms();
            if(rNode.IsTxtNode())
            {
                SwTxtNode& rTxtNode = static_cast<SwTxtNode&>(rNode);
                // setze den bei allen TextNode in der Tabelle den TableNode
                // als StartNode

                // remove PageBreaks/PageDesc/ColBreak
                const SwAttrSet* pSet = rTxtNode.GetpSwAttrSet();
                if( pSet )
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pItem ) )
                    {
                        if( !nLines )
                            pTblFmt->SetFmtAttr( *pItem );
                        rTxtNode.ResetAttr( RES_BREAK );
                        pSet = rTxtNode.GetpSwAttrSet();
                    }

                    if( pSet && SFX_ITEM_SET == pSet->GetItemState(
                        RES_PAGEDESC, sal_False, &pItem ) &&
                        ((SwFmtPageDesc*)pItem)->GetPageDesc() )
                    {
                        if( !nLines )
                            pTblFmt->SetFmtAttr( *pItem );
                        rTxtNode.ResetAttr( RES_PAGEDESC );
                    }
                }
            }
        }
    }

    std::vector<std::vector < SwNodeRange > >::const_iterator aRowIter = rTableNodes.begin();
    for( nLines = 0, nBoxes = 0;
        aRowIter != rTableNodes.end();
        ++aRowIter, nLines++, nBoxes = 0 )
    {
        pLine = new SwTableLine( pLineFmt, 1, 0 );
        pTable->GetTabLines().C40_INSERT( SwTableLine, pLine, nLines );

        std::vector< SwNodeRange >::const_iterator aCellIter = aRowIter->begin();

        for( ; aCellIter != aRowIter->end(); ++aCellIter )
        {
                const SwNodeIndex aTmpIdx( aCellIter->aStart, 0 );

               SwNodeIndex aCellEndIdx(aCellIter->aEnd);
               ++aCellEndIdx;
               SwStartNode* pSttNd = new SwStartNode( aTmpIdx, ND_STARTNODE,
                                            SwTableBoxStartNode );
                new SwEndNode( aCellEndIdx, *pSttNd );
                //set the start node on all node of the current cell
                SwNodeIndex aCellNodeIdx = aCellIter->aStart;
                for(;aCellNodeIdx <= aCellIter->aEnd; ++aCellNodeIdx )
                {
                    aCellNodeIdx.GetNode().pStartOfSection = pSttNd;
                    //skip start/end node pairs
                    if( aCellNodeIdx.GetNode().IsStartNode() )
                        aCellNodeIdx = SwNodeIndex( *aCellNodeIdx.GetNode().EndOfSectionNode() );
                }

                // Section der Box zuweisen
                pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
                pLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, nBoxes++ );
        }
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    // die Tabelle ausgleichen, leere Sections einfuegen
    sal_uInt16 n;

    if( !aPosArr.empty() )
    {
        SwTableLines& rLns = pTable->GetTabLines();
        sal_uInt16 nLastPos = 0;
        for( n = 0; n < aPosArr.size(); ++n )
        {
            SwTableBoxFmt *pNewFmt = pDoc->MakeTableBoxFmt();
            pNewFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                aPosArr[ n ] - nLastPos ));
            for( sal_uInt16 nLines2 = 0; nLines2 < rLns.Count(); ++nLines2 )
                //JP 24.06.98: hier muss ein Add erfolgen, da das BoxFormat
                //              von der rufenden Methode noch gebraucht wird!
                pNewFmt->Add( rLns[ nLines2 ]->GetTabBoxes()[ n ] );

            nLastPos = aPosArr[ n ];
        }

        // damit die Tabelle die richtige Groesse bekommt, im BoxFormat die
        // Groesse nach "oben" transportieren.
        OSL_ENSURE( !pBoxFmt->GetDepends(), "wer ist in dem Format noch angemeldet" );
        pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nLastPos ));
    }
    else
        pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX / nMaxBoxes ));

    // das wars doch wohl ??
    return pTblNd;
}


//---------------- Tabelle -> Text -----------------------


// -- benutze die ForEach Methode vom PtrArray um aus einer Tabelle wieder
//      Text zuerzeugen. (Die Boxen koennen auch noch Lines enthalten !!)
struct _DelTabPara
{
    SwTxtNode* pLastNd;
    SwNodes& rNds;
    SwUndoTblToTxt* pUndo;
    sal_Unicode cCh;

    _DelTabPara( SwNodes& rNodes, sal_Unicode cChar, SwUndoTblToTxt* pU ) :
        pLastNd(0), rNds( rNodes ), pUndo( pU ), cCh( cChar ) {}
    _DelTabPara( const _DelTabPara& rPara ) :
        pLastNd(rPara.pLastNd), rNds( rPara.rNds ),
        pUndo( rPara.pUndo ), cCh( rPara.cCh ) {}
};

// forward deklarieren damit sich die Lines und Boxen rekursiv aufrufen
// koennen.
sal_Bool lcl_DelBox( const SwTableBox*&, void *pPara );

sal_Bool lcl_DelLine( const SwTableLine*& rpLine, void* pPara )
{
    OSL_ENSURE( pPara, "die Parameter fehlen" );
    _DelTabPara aPara( *(_DelTabPara*)pPara );
    ((SwTableLine*&)rpLine)->GetTabBoxes().ForEach( &lcl_DelBox, &aPara );
    if( rpLine->GetUpper() )        // gibt es noch eine uebergeordnete Box ??
        // dann gebe den letzten TextNode zurueck
        ((_DelTabPara*)pPara)->pLastNd = aPara.pLastNd;
    return sal_True;
}


sal_Bool lcl_DelBox( const SwTableBox*& rpBox, void* pPara )
{
    OSL_ENSURE( pPara, "die Parameter fehlen" );

    // loesche erstmal die Lines der Box
    _DelTabPara* pDelPara = (_DelTabPara*)pPara;
    if( rpBox->GetTabLines().Count() )
        ((SwTableBox*&)rpBox)->GetTabLines().ForEach( &lcl_DelLine, pDelPara );
    else
    {
        SwDoc* pDoc = pDelPara->rNds.GetDoc();
        SwNodeRange aDelRg( *rpBox->GetSttNd(), 0,
                            *rpBox->GetSttNd()->EndOfSectionNode() );
        // loesche die Section
        pDelPara->rNds.SectionUp( &aDelRg );
        const SwTxtNode* pCurTxtNd;
        if( T2T_PARA != pDelPara->cCh && pDelPara->pLastNd &&
            0 != ( pCurTxtNd = aDelRg.aStart.GetNode().GetTxtNode() ))
        {
            // Join the current text node with the last from the previous box if possible
            sal_uLong nNdIdx = aDelRg.aStart.GetIndex();
            aDelRg.aStart--;
            if( pDelPara->pLastNd == &aDelRg.aStart.GetNode() )
            {
                // Inserting the seperator
                SwIndex aCntIdx( pDelPara->pLastNd, pDelPara->pLastNd->GetTxt().Len());
                pDelPara->pLastNd->InsertText( pDelPara->cCh, aCntIdx,
                    IDocumentContentOperations::INS_EMPTYEXPAND );
                if( pDelPara->pUndo )
                    pDelPara->pUndo->AddBoxPos( *pDoc, nNdIdx, aDelRg.aEnd.GetIndex(),
                                                aCntIdx.GetIndex() );

                std::vector<sal_uLong> aBkmkArr;
                xub_StrLen nOldTxtLen = aCntIdx.GetIndex();
                _SaveCntntIdx( pDoc, nNdIdx, pCurTxtNd->GetTxt().Len(),
                                aBkmkArr );

                pDelPara->pLastNd->JoinNext();

                if( !aBkmkArr.empty() )
                    _RestoreCntntIdx( pDoc, aBkmkArr,
                                        pDelPara->pLastNd->GetIndex(),
                                        nOldTxtLen );
            }
            else if( pDelPara->pUndo )
            {
                aDelRg.aStart++;
                pDelPara->pUndo->AddBoxPos( *pDoc, nNdIdx, aDelRg.aEnd.GetIndex() );
            }
        }
        else if( pDelPara->pUndo )
            pDelPara->pUndo->AddBoxPos( *pDoc, aDelRg.aStart.GetIndex(), aDelRg.aEnd.GetIndex() );
        aDelRg.aEnd--;
        pDelPara->pLastNd = aDelRg.aEnd.GetNode().GetTxtNode();

        //JP 03.04.97: die Ausrichtung der ZahlenFormatierung auf
        //              keinen Fall uebernehmen
        if( pDelPara->pLastNd && pDelPara->pLastNd->HasSwAttrSet() )
            pDelPara->pLastNd->ResetAttr( RES_PARATR_ADJUST );
    }
    return sal_True;
}


sal_Bool SwNodes::TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                            SwUndoTblToTxt* pUndo )
{
    // ist eine Tabelle selektiert ?
    SwTableNode* pTblNd;
    if( rRange.aStart.GetIndex() >= rRange.aEnd.GetIndex() ||
        0 == ( pTblNd = rRange.aStart.GetNode().GetTableNode()) ||
        &rRange.aEnd.GetNode() != pTblNd->EndOfSectionNode() )
        return sal_False;

    // stand die Tabelle ganz alleine in einer Section ?
    // dann ueber den Upper der Tabelle die Frames anlegen
    SwNode2Layout* pNode2Layout = 0;
    SwNodeIndex aFrmIdx( rRange.aStart );
    SwNode* pFrmNd = FindPrvNxtFrmNode( aFrmIdx, &rRange.aEnd.GetNode() );
    if( !pFrmNd )
        // dann sammel mal alle Uppers ein
        pNode2Layout = new SwNode2Layout( *pTblNd );

    // loesche schon mal die Frames
    pTblNd->DelFrms();

    // dann "loeschen" die Tabellen und fasse alle Lines/Boxen zusammen
    _DelTabPara aDelPara( *this, cCh, pUndo );
    pTblNd->pTable->GetTabLines().ForEach( &lcl_DelLine, &aDelPara );

    // jetzt ist aus jeder TableLine ein TextNode mit dem entsprechenden
    // Trenner erzeugt worden. Es braucht nur noch die Table-Section
    // geloescht und fuer die neuen TextNode die Frames erzeugt werden.
    SwNodeRange aDelRg( rRange.aStart, rRange.aEnd );

    // JP 14.01.97: hat die Tabelle PageDesc-/Break-Attribute? Dann in den
    //              ersten TextNode uebernehmen
    {
// was ist mit UNDO???
        const SfxItemSet& rTblSet = pTblNd->pTable->GetFrmFmt()->GetAttrSet();
        const SfxPoolItem *pBreak, *pDesc;
        if( SFX_ITEM_SET != rTblSet.GetItemState( RES_PAGEDESC, sal_False, &pDesc ))
            pDesc = 0;
        if( SFX_ITEM_SET != rTblSet.GetItemState( RES_BREAK, sal_False, &pBreak ))
            pBreak = 0;

        if( pBreak || pDesc )
        {
            SwNodeIndex aIdx( *pTblNd  );
            SwCntntNode* pCNd = GoNext( &aIdx );
            if( pBreak )
                pCNd->SetAttr( *pBreak );
            if( pDesc )
                pCNd->SetAttr( *pDesc );
        }
    }

    SectionUp( &aDelRg );       // loesche die Section und damit die Tabelle
    // #i28006#
    sal_uLong nStt = aDelRg.aStart.GetIndex(), nEnd = aDelRg.aEnd.GetIndex();
    if( !pFrmNd )
    {
        pNode2Layout->RestoreUpperFrms( *this,
                        aDelRg.aStart.GetIndex(), aDelRg.aEnd.GetIndex() );
        delete pNode2Layout;
    }
    else
    {
        SwCntntNode *pCNd;
        SwSectionNode *pSNd;
        while( aDelRg.aStart.GetIndex() < nEnd )
        {
            if( 0 != ( pCNd = aDelRg.aStart.GetNode().GetCntntNode()))
            {
                if( pFrmNd->IsCntntNode() )
                    ((SwCntntNode*)pFrmNd)->MakeFrms( *pCNd );
                else if( pFrmNd->IsTableNode() )
                    ((SwTableNode*)pFrmNd)->MakeFrms( aDelRg.aStart );
                else if( pFrmNd->IsSectionNode() )
                    ((SwSectionNode*)pFrmNd)->MakeFrms( aDelRg.aStart );
                pFrmNd = pCNd;
            }
            else if( 0 != ( pSNd = aDelRg.aStart.GetNode().GetSectionNode()))
            {
                if( !pSNd->GetSection().IsHidden() && !pSNd->IsCntntHidden() )
                {
                    pSNd->MakeFrms( &aFrmIdx, &aDelRg.aEnd );
                    pFrmNd = pSNd;
                    break;
                }
                aDelRg.aStart = *pSNd->EndOfSectionNode();
            }
            aDelRg.aStart++;
        }
    }

    // #i28006# Fly frames have to be restored even if the table was
    // #alone in the section
    const SwSpzFrmFmts& rFlyArr = *GetDoc()->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFlyArr.Count(); ++n )
    {
        SwFrmFmt *const pFmt = (SwFrmFmt*)rFlyArr[n];
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
            nStt <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nEnd )
        {
            pFmt->MakeFrms();
        }
    }

    return sal_True;
}


// -------------------------------------------------------

//---------
// SwTableNode
//---------

SwTableNode::SwTableNode( const SwNodeIndex& rIdx )
    : SwStartNode( rIdx, ND_TABLENODE )
{
    pTable = new SwTable( 0 );
}

SwTableNode::~SwTableNode()
{
    //don't forget to notify uno wrappers
    SwFrmFmt* pTblFmt = GetTable().GetFrmFmt();
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
                                pTblFmt );
    pTblFmt->ModifyNotification( &aMsgHint, &aMsgHint );
    DelFrms();
    delete pTable;
}

SwTabFrm *SwTableNode::MakeFrm( SwFrm* pSib )
{
    return new SwTabFrm( *pTable, pSib );
}

//Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
//Dokument. Die erzeugten Contentframes werden in das entsprechende
//Layout gehaengt.
void SwTableNode::MakeFrms(const SwNodeIndex & rIdx )
{
    if( !GetTable().GetFrmFmt()->GetDepends())//gibt es ueberhaupt Frames ??
        return;

    SwFrm *pFrm, *pNew;
    SwCntntNode * pNode = rIdx.GetNode().GetCntntNode();

    OSL_ENSURE( pNode, "Kein Contentnode oder Copy-Node und neuer Node identisch.");

    sal_Bool bBefore = rIdx < GetIndex();

    SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );

    while( 0 != (pFrm = aNode2Layout.NextFrm()) )
    {
        pNew = pNode->MakeFrm( pFrm );
        // wird ein Node vorher oder nachher mit Frames versehen
        if ( bBefore )
            // der neue liegt vor mir
            pNew->Paste( pFrm->GetUpper(), pFrm );
        else
            // der neue liegt hinter mir
            pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
    }
}

//Fuer jede Shell einen TblFrm anlegen und vor den entsprechenden
//CntntFrm pasten.

void SwTableNode::MakeFrms( SwNodeIndex* pIdxBehind )
{
    OSL_ENSURE( pIdxBehind, "kein Index" );
    *pIdxBehind = *this;
    SwNode *pNd = GetNodes().FindPrvNxtFrmNode( *pIdxBehind, EndOfSectionNode() );
    if( !pNd )
        return ;

    SwFrm *pFrm( 0L );
    SwLayoutFrm *pUpper( 0L );
    SwNode2Layout aNode2Layout( *pNd, GetIndex() );
    while( 0 != (pUpper = aNode2Layout.UpperFrm( pFrm, *this )) )
    {
        SwTabFrm* pNew = MakeFrm( pUpper );
        pNew->Paste( pUpper, pFrm );
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
        {
            ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
            }
        }
        ((SwTabFrm*)pNew)->RegistFlys();
    }
}

void SwTableNode::DelFrms()
{
    //Erstmal die TabFrms ausschneiden und deleten, die Columns und Rows
    //nehmen sie mit in's Grab.
    //Die TabFrms haengen am FrmFmt des SwTable.
    //Sie muessen etwas umstaendlich zerstort werden, damit die Master
    //die Follows mit in's Grab nehmen.

    SwIterator<SwTabFrm,SwFmt> aIter( *(pTable->GetFrmFmt()) );
    SwTabFrm *pFrm = aIter.First();
    while ( pFrm )
    {
        sal_Bool bAgain = sal_False;
        {
            if ( !pFrm->IsFollow() )
            {
                while ( pFrm->HasFollow() )
                    pFrm->JoinAndDelFollows();
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for current next paragraph will change
                // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
                {
                    ViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pFrm->FindPrevCnt( true )) );
                    }
                }
                pFrm->Cut();
                delete pFrm;
                bAgain = sal_True;
            }
        }
        pFrm = bAgain ? aIter.First() : aIter.Next();
    }
}


void SwTableNode::SetNewTable( SwTable* pNewTable, sal_Bool bNewFrames )
{
    DelFrms();
    delete pTable;
    pTable = pNewTable;
    if( bNewFrames )
    {
        SwNodeIndex aIdx( *EndOfSectionNode());
        GetNodes().GoNext( &aIdx );
        MakeFrms( &aIdx );
    }
}


#define ROWFUZZY 25

struct FuzzyCompare
{
    bool operator() ( long s1, long s2 ) const;
};

bool FuzzyCompare::operator() ( long s1, long s2 ) const
{
    return ( s1 < s2 && abs( s1 - s2 ) > ROWFUZZY );
}

bool lcl_IsFrmInColumn( const SwCellFrm& rFrm, SwSelBoxes& rBoxes )
{
    return 0 != rBoxes.count(rFrm.GetTabBox());
}

// Splittet eine Tabelle in der Grund-Zeile, in der der Index steht.
// Alle GrundZeilen dahinter wandern in eine neue Tabelle/-Node.
// Ist das Flag bCalcNewSize auf sal_True, wird fuer beide neuen Tabellen
// die neue Size aus dem Max der Boxen errechnet; vorrausgesetzt,
// die Size ist "absolut" gesetzt (USHRT_MAX)

void SwCollectTblLineBoxes::AddToUndoHistory( const SwCntntNode& rNd )
{
    if( pHst )
        pHst->Add( rNd.GetFmtColl(), rNd.GetIndex(), ND_TEXTNODE );
}

void SwCollectTblLineBoxes::AddBox( const SwTableBox& rBox )
{
    aPosArr.push_back(nWidth);
    aBoxes.push_back( &rBox );
    nWidth = nWidth + (sal_uInt16)rBox.GetFrmFmt()->GetFrmSize().GetWidth();
}

const SwTableBox* SwCollectTblLineBoxes::GetBoxOfPos( const SwTableBox& rBox )
{
    const SwTableBox* pRet = 0;
    sal_uInt16 n;

    if( !aPosArr.empty() )
    {
        for( n = 0; n < aPosArr.size(); ++n )
            if( aPosArr[ n ] == nWidth )
                break;
            else if( aPosArr[ n ] > nWidth )
            {
                if( n )
                    --n;
                break;
            }

        if( n >= aPosArr.size() )
            --n;

        nWidth = nWidth + (sal_uInt16)rBox.GetFrmFmt()->GetFrmSize().GetWidth();
        pRet = aBoxes[ n ];
    }
    return pRet;
}

sal_Bool SwCollectTblLineBoxes::Resize( sal_uInt16 nOffset, sal_uInt16 nOldWidth )
{
    if( !aPosArr.empty() )
    {
        sal_uInt16 n;

        for( n = 0; n < aPosArr.size(); ++n )
        {
            if( aPosArr[ n ] == nOffset )
                break;
            else if( aPosArr[ n ] > nOffset )
            {
                if( n )
                    --n;
                break;
            }
        }

        aPosArr.erase(aPosArr.begin(), aPosArr.begin()+n);
        aBoxes.erase(aBoxes.begin(), aBoxes.begin()+n);

        // dann die Positionen der neuen Size anpassen
        for( n = 0; n < aPosArr.size(); ++n )
        {
            sal_uLong nSize = nWidth;
            nSize *= ( aPosArr[ n ] - nOffset );
            nSize /= nOldWidth;
            aPosArr[ n ] = sal_uInt16( nSize );
        }
    }
    return 0 != aPosArr.size();
}

sal_Bool lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    if( pSplPara->IsGetValues() )
        ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_Box_CollectBox, pPara );
    else
        ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_BoxSetSplitBoxFmts, pPara );
    return sal_True;
}

sal_Bool lcl_Box_CollectBox( const SwTableBox*& rpBox, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    sal_uInt16 nLen = rpBox->GetTabLines().Count();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = rpBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pPara );
    }
    else
        pSplPara->AddBox( *rpBox );
    return sal_True;
}

sal_Bool lcl_BoxSetSplitBoxFmts( const SwTableBox*& rpBox, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    sal_uInt16 nLen = rpBox->GetTabLines().Count();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = rpBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pPara );
    }
    else
    {
        const SwTableBox* pSrcBox = pSplPara->GetBoxOfPos( *rpBox );
        SwFrmFmt* pFmt = pSrcBox->GetFrmFmt();
        SwTableBox* pBox = (SwTableBox*)rpBox;

        if( HEADLINE_BORDERCOPY == pSplPara->GetMode() )
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();
            if( !rBoxItem.GetTop() )
            {
                SvxBoxItem aNew( rBoxItem );
                aNew.SetLine( pFmt->GetBox().GetBottom(), BOX_LINE_TOP );
                if( aNew != rBoxItem )
                    pBox->ClaimFrmFmt()->SetFmtAttr( aNew );
            }
        }
        else
        {
sal_uInt16 aTableSplitBoxSetRange[] = {
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    0 };
            SfxItemSet aTmpSet( pFmt->GetDoc()->GetAttrPool(),
                                aTableSplitBoxSetRange );
            aTmpSet.Put( pFmt->GetAttrSet() );
            if( aTmpSet.Count() )
                pBox->ClaimFrmFmt()->SetFmtAttr( aTmpSet );

            if( HEADLINE_BOXATRCOLLCOPY == pSplPara->GetMode() )
            {
                SwNodeIndex aIdx( *pSrcBox->GetSttNd(), 1 );
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( !pCNd )
                    pCNd = aIdx.GetNodes().GoNext( &aIdx );
                aIdx = *pBox->GetSttNd();
                SwCntntNode* pDNd = aIdx.GetNodes().GoNext( &aIdx );

                // nur wenn der Node alleine in der Section steht
                if( 2 == pDNd->EndOfSectionIndex() -
                        pDNd->StartOfSectionIndex() )
                {
                    pSplPara->AddToUndoHistory( *pDNd );
                    pDNd->ChgFmtColl( pCNd->GetFmtColl() );
                }
            }

            // bedingte Vorlage beachten
            pBox->GetSttNd()->CheckSectionCondColl();
        }
    }
    return sal_True;
}


sal_Bool lcl_ChgTblSize( SwTable& rTbl )
{
    // das Attribut darf nicht ueber das Modify an der
    // Tabelle gesetzt werden, denn sonst werden alle
    // Boxen wieder auf 0 zurueck gesetzt. Also locke das Format
    SwFrmFmt* pFmt = rTbl.GetFrmFmt();
    SwFmtFrmSize aTblMaxSz( pFmt->GetFrmSize() );

    if( USHRT_MAX == aTblMaxSz.GetWidth() )
        return sal_False;

    sal_Bool bLocked = pFmt->IsModifyLocked();
    pFmt->LockModify();

    aTblMaxSz.SetWidth( 0 );

    SwTableLines& rLns = rTbl.GetTabLines();
    for( sal_uInt16 nLns = 0; nLns < rLns.Count(); ++nLns )
    {
        SwTwips nMaxLnWidth = 0;
        SwTableBoxes& rBoxes = rLns[ nLns ]->GetTabBoxes();
        for( sal_uInt16 nBox = 0; nBox < rBoxes.Count(); ++nBox )
            nMaxLnWidth += rBoxes[nBox]->GetFrmFmt()->GetFrmSize().GetWidth();

        if( nMaxLnWidth > aTblMaxSz.GetWidth() )
            aTblMaxSz.SetWidth( nMaxLnWidth );
    }
    pFmt->SetFmtAttr( aTblMaxSz );
    if( !bLocked )          // und gegebenenfalls Lock wieder freigeben
        pFmt->UnlockModify();

    return sal_True;
}

class _SplitTable_Para
{
    SvPtrarr aSrc, aDest;
    SwTableNode* pNewTblNd;
    SwTable& rOldTbl;

public:
    _SplitTable_Para( SwTableNode* pNew, SwTable& rOld )
        : aSrc( 16 ), aDest( 16 ), pNewTblNd( pNew ), rOldTbl( rOld )
    {}
    sal_uInt16 SrcFmt_GetPos( void* pFmt ) const
            { return aSrc.GetPos( pFmt ); }

    void DestFmt_Insert( void* pFmt )
            { aDest.Insert( pFmt, aDest.Count() ); }

    void SrcFmt_Insert( void* pFmt )
            { aSrc.Insert( pFmt, aSrc.Count() ); }

    SwFrmFmt* DestFmt_Get( sal_uInt16 nPos ) const
            { return (SwFrmFmt*)aDest[ nPos ]; }

    void ChgBox( SwTableBox* pBox )
    {
        rOldTbl.GetTabSortBoxes().Remove( pBox );
        pNewTblNd->GetTable().GetTabSortBoxes().Insert( pBox );
    }
};


sal_Bool lcl_SplitTable_CpyBox( const SwTableBox*& rpBox, void* pPara );

sal_Bool lcl_SplitTable_CpyLine( const SwTableLine*& rpLine, void* pPara )
{
    SwTableLine* pLn = (SwTableLine*)rpLine;
    _SplitTable_Para& rPara = *(_SplitTable_Para*)pPara;

    SwFrmFmt *pSrcFmt = pLn->GetFrmFmt();
    sal_uInt16 nPos = rPara.SrcFmt_GetPos( pSrcFmt );
    if( USHRT_MAX == nPos )
    {
        rPara.DestFmt_Insert( pLn->ClaimFrmFmt() );
        rPara.SrcFmt_Insert( pSrcFmt );
    }
    else
        pLn->ChgFrmFmt( (SwTableLineFmt*)rPara.DestFmt_Get( nPos ) );

    pLn->GetTabBoxes().ForEach( &lcl_SplitTable_CpyBox, pPara );
    return sal_True;
}

sal_Bool lcl_SplitTable_CpyBox( const SwTableBox*& rpBox, void* pPara )
{
    SwTableBox* pBox = (SwTableBox*)rpBox;
    _SplitTable_Para& rPara = *(_SplitTable_Para*)pPara;

    SwFrmFmt *pSrcFmt = pBox->GetFrmFmt();
    sal_uInt16 nPos = rPara.SrcFmt_GetPos( pSrcFmt );
    if( USHRT_MAX == nPos )
    {
        rPara.DestFmt_Insert( pBox->ClaimFrmFmt() );
        rPara.SrcFmt_Insert( pSrcFmt );
    }
    else
        pBox->ChgFrmFmt( (SwTableBoxFmt*)rPara.DestFmt_Get( nPos ) );

    if( pBox->GetSttNd() )
        rPara.ChgBox( pBox );
    else
        pBox->GetTabLines().ForEach( &lcl_SplitTable_CpyLine, pPara );
    return sal_True;
}

SwTableNode* SwNodes::SplitTable( const SwNodeIndex& rPos, sal_Bool bAfter,
                                    sal_Bool bCalcNewSize )
{
    SwNode* pNd = &rPos.GetNode();
    SwTableNode* pTNd = pNd->FindTableNode();
    if( !pTNd || pNd->IsTableNode() )
        return 0;

    sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

    // Suche die Grund-Line dieser Box:
    SwTable& rTbl = pTNd->GetTable();
    SwTableBox* pBox = rTbl.GetTblBox( nSttIdx );
    if( !pBox )
        return 0;

    SwTableLine* pLine = pBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    // in pLine steht jetzt die GrundLine.
    sal_uInt16 nLinePos = rTbl.GetTabLines().C40_GETPOS( SwTableLine, pLine );
    if( USHRT_MAX == nLinePos ||
        ( bAfter ? ++nLinePos >= rTbl.GetTabLines().Count() : !nLinePos ))
        return 0;       // nicht gefunden oder letze Line !!

    // Suche jetzt die 1. Box der nachfolgenden Line
    SwTableLine* pNextLine = rTbl.GetTabLines()[ nLinePos ];
    pBox = pNextLine->GetTabBoxes()[0];
    while( !pBox->GetSttNd() )
        pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];

    // dann fuege mal einen End- und TabelleNode ins Nodes-Array ein.
    SwTableNode * pNewTblNd;
    {
        SwEndNode* pOldTblEndNd = (SwEndNode*)pTNd->EndOfSectionNode()->GetEndNode();
        OSL_ENSURE( pOldTblEndNd, "wo ist der EndNode?" );

        SwNodeIndex aIdx( *pBox->GetSttNd() );
        new SwEndNode( aIdx, *pTNd );
        pNewTblNd = new SwTableNode( aIdx );
        pNewTblNd->GetTable().SetTableModel( rTbl.IsNewModel() );

        pOldTblEndNd->pStartOfSection = pNewTblNd;
        pNewTblNd->pEndOfSection = pOldTblEndNd;

        SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
        do {
            OSL_ENSURE( pBoxNd->IsStartNode(), "das muss ein StartNode sein!" );
            pBoxNd->pStartOfSection = pNewTblNd;
            pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
        } while( pBoxNd != pOldTblEndNd );
    }

    {
        // die Lines ruebermoven...
        SwTable& rNewTbl = pNewTblNd->GetTable();
        rNewTbl.GetTabLines().Insert( &rTbl.GetTabLines(), 0, nLinePos );
        //
        // von hinten (unten-rechts) nach vorn (oben-links) alle Boxen
        // beim chart data provider austragen (das modified event wird dann
        // in der aufrufenden Funktion getriggert.
        // TL_CHART2:
        SwChartDataProvider *pPCD = rTbl.GetFrmFmt()->getIDocumentChartDataProviderAccess()->GetChartDataProvider();
        if( pPCD )
        {
            for (sal_uInt16 k = nLinePos;  k < rTbl.GetTabLines().Count();  ++k)
            {
                sal_uInt16 nLineIdx = (rTbl.GetTabLines().Count() - 1) - k + nLinePos;
                sal_uInt16 nBoxCnt = rTbl.GetTabLines()[ nLineIdx ]->GetTabBoxes().Count();
                for (sal_uInt16 j = 0;  j < nBoxCnt;  ++j)
                {
                    sal_uInt16 nIdx = nBoxCnt - 1 - j;
                    pPCD->DeleteBox( &rTbl, *rTbl.GetTabLines()[ nLineIdx ]->GetTabBoxes()[nIdx] );
                }
            }
        }
        //
        // ...und loeschen
        sal_uInt16 nDeleted = rTbl.GetTabLines().Count() - nLinePos;
        rTbl.GetTabLines().Remove( nLinePos, nDeleted );

        // und die betr. Boxen verschieben. Dabei die Formate eindeutig
        // machen und die StartNodes korrigieren
        _SplitTable_Para aPara( pNewTblNd, rTbl );
        rNewTbl.GetTabLines().ForEach( &lcl_SplitTable_CpyLine, &aPara );
        rTbl.CleanUpBottomRowSpan( nDeleted );
    }

    {
        // Das Tabellen-FrmFormat kopieren
        SwFrmFmt* pOldTblFmt = rTbl.GetFrmFmt();
        SwFrmFmt* pNewTblFmt = pOldTblFmt->GetDoc()->MakeTblFrmFmt(
                                pOldTblFmt->GetDoc()->GetUniqueTblName(),
                                pOldTblFmt->GetDoc()->GetDfltFrmFmt() );

        *pNewTblFmt = *pOldTblFmt;
        pNewTblNd->GetTable().RegisterToFormat( *pNewTblFmt );

        // neue Size errechnen ? (lcl_ChgTblSize nur das 2. aufrufen, wenn es
        // beim 1. schon geklappt hat; also absolute Groesse hat)
        if( bCalcNewSize && lcl_ChgTblSize( rTbl ) )
            lcl_ChgTblSize( pNewTblNd->GetTable() );
    }

    // TL_CHART2: need to inform chart of probably changed cell names
    rTbl.UpdateCharts();

    return pNewTblNd;       // das wars
}

// und die Umkehrung davon. rPos muss in der Tabelle stehen, die bestehen
// bleibt. Das Flag besagt ob die aktuelle mit der davor oder dahinter
// stehenden vereint wird.

sal_Bool SwNodes::MergeTable( const SwNodeIndex& rPos, sal_Bool bWithPrev,
                            sal_uInt16 nMode, SwHistory* )
{
    SwTableNode* pDelTblNd = rPos.GetNode().GetTableNode();
    OSL_ENSURE( pDelTblNd, "wo ist der TableNode geblieben?" );

    SwTableNode* pTblNd = (*this)[ rPos.GetIndex() - 1]->FindTableNode();
    OSL_ENSURE( pTblNd, "wo ist der TableNode geblieben?" );

    if( !pDelTblNd || !pTblNd )
        return sal_False;

    pDelTblNd->DelFrms();

    SwTable& rDelTbl = pDelTblNd->GetTable();
    SwTable& rTbl = pTblNd->GetTable();

    //Lines fuer das Layout-Update herausuchen.
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rTbl );
    aFndBox.DelFrms( rTbl );

    // TL_CHART2: since chart currently does not want to get informed about
    // additional rows/cols there is no need for a modified event in the
    // remaining first table. Also, if it is required it  should be done
    // after the merging and not here...
    // pDoc->UpdateCharts( rTbl.GetFrmFmt()->GetName() );


    // TL_CHART2:
    // tell the charts about the table to be deleted and have them use their own data
    GetDoc()->CreateChartInternalDataProviders( &rDelTbl );

    // die Breite der TabellenFormate abgleichen:
    {
        const SwFmtFrmSize& rTblSz = rTbl.GetFrmFmt()->GetFrmSize();
        const SwFmtFrmSize& rDelTblSz = rDelTbl.GetFrmFmt()->GetFrmSize();
        if( rTblSz != rDelTblSz )
        {
            // dann sollten die mal schleunigst korrigiert werden
            if( bWithPrev )
                rDelTbl.GetFrmFmt()->SetFmtAttr( rTblSz );
            else
                rTbl.GetFrmFmt()->SetFmtAttr( rDelTblSz );
        }
    }

    if( !bWithPrev )
    {
        // dann mussen alle Attruibute der hinteren Tabelle auf die
        // vordere uebertragen werden, weil die hintere ueber das loeschen
        // des Node geloescht wird.
        rTbl.SetRowsToRepeat( rDelTbl.GetRowsToRepeat() );
        rTbl.SetTblChgMode( rDelTbl.GetTblChgMode() );

        rTbl.GetFrmFmt()->LockModify();
        *rTbl.GetFrmFmt() = *rDelTbl.GetFrmFmt();
        // auch den Namen umsetzen!
        rTbl.GetFrmFmt()->SetName( rDelTbl.GetFrmFmt()->GetName() );
        rTbl.GetFrmFmt()->UnlockModify();
    }

    // die Lines und Boxen ruebermoven
    sal_uInt16 nOldSize = rTbl.GetTabLines().Count();
    rTbl.GetTabLines().Insert( &rDelTbl.GetTabLines(), nOldSize );
    rDelTbl.GetTabLines().Remove( 0, rDelTbl.GetTabLines().Count() );

    rTbl.GetTabSortBoxes().Insert( &rDelTbl.GetTabSortBoxes() );
    rDelTbl.GetTabSortBoxes().Remove( (sal_uInt16)0, rDelTbl.GetTabSortBoxes().Count() );

    // die vordere Tabelle bleibt immer stehen, die hintere wird geloescht
    SwEndNode* pTblEndNd = pDelTblNd->EndOfSectionNode();
    pTblNd->pEndOfSection = pTblEndNd;

    SwNodeIndex aIdx( *pDelTblNd, 1 );

    SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
    do {
        OSL_ENSURE( pBoxNd->IsStartNode(), "das muss ein StartNode sein!" );
        pBoxNd->pStartOfSection = pTblNd;
        pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
    } while( pBoxNd != pTblEndNd );
    pBoxNd->pStartOfSection = pTblNd;

    aIdx -= 2;
    DelNodes( aIdx, 2 );

    // jetzt an der 1. eingefuegten Line die bedingten Vorlagen umschubsen
    const SwTableLine* pFirstLn = rTbl.GetTabLines()[ nOldSize ];
    if( 1 == nMode )        //
    {
        // Header-Vorlagen in der Zeile setzen
        // und ggfs. in der History speichern fuers Undo!!!
    }
    lcl_LineSetHeadCondColl( pFirstLn, 0 );

    // und die Borders "aufrauemen"
    if( nOldSize )
    {
        _SwGCLineBorder aPara( rTbl );
        aPara.nLinePos = --nOldSize;
        pFirstLn = rTbl.GetTabLines()[ nOldSize ];
        lcl_GC_Line_Border( pFirstLn, &aPara );
    }

    //Layout updaten
    aFndBox.MakeFrms( rTbl );

    return sal_True;
}

// -------------------------------------------------------------------


// -- benutze die ForEach Methode vom PtrArray
struct _SetAFmtTabPara
{
    SwTableAutoFmt& rTblFmt;
    SwUndoTblAutoFmt* pUndo;
    sal_uInt16 nEndBox, nCurBox;
    sal_uInt8 nAFmtLine, nAFmtBox;

    _SetAFmtTabPara( const SwTableAutoFmt& rNew )
        : rTblFmt( (SwTableAutoFmt&)rNew ), pUndo( 0 ),
        nEndBox( 0 ), nCurBox( 0 ), nAFmtLine( 0 ), nAFmtBox( 0 )
    {}
};

// forward deklarieren damit sich die Lines und Boxen rekursiv aufrufen
// koennen.
sal_Bool lcl_SetAFmtBox( const _FndBox*&, void *pPara );
sal_Bool lcl_SetAFmtLine( const _FndLine*&, void *pPara );

sal_Bool lcl_SetAFmtLine( const _FndLine*& rpLine, void *pPara )
{
    ((_FndLine*&)rpLine)->GetBoxes().ForEach( &lcl_SetAFmtBox, pPara );
    return sal_True;
}

sal_Bool lcl_SetAFmtBox( const _FndBox*& rpBox, void *pPara )
{
    _SetAFmtTabPara* pSetPara = (_SetAFmtTabPara*)pPara;

    if( !rpBox->GetUpper()->GetUpper() )    // Box auf 1. Ebene ?
    {
        if( !pSetPara->nCurBox )
            pSetPara->nAFmtBox = 0;
        else if( pSetPara->nCurBox == pSetPara->nEndBox )
            pSetPara->nAFmtBox = 3;
        else
            pSetPara->nAFmtBox = (sal_uInt8)(1 + ((pSetPara->nCurBox-1) & 1));
    }

    if( rpBox->GetBox()->GetSttNd() )
    {
        SwTableBox* pSetBox = (SwTableBox*)rpBox->GetBox();
        SwDoc* pDoc = pSetBox->GetFrmFmt()->GetDoc();
        SfxItemSet aCharSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1 );
        SfxItemSet aBoxSet( pDoc->GetAttrPool(), aTableBoxSetRange );
        sal_uInt8 nPos = pSetPara->nAFmtLine * 4 + pSetPara->nAFmtBox;
        pSetPara->rTblFmt.UpdateToSet( nPos, aCharSet,
                                        SwTableAutoFmt::UPDATE_CHAR, 0 );
        pSetPara->rTblFmt.UpdateToSet( nPos, aBoxSet,
                                        SwTableAutoFmt::UPDATE_BOX,
                                        pDoc->GetNumberFormatter( sal_True ) );
        if( aCharSet.Count() )
        {
            sal_uLong nSttNd = pSetBox->GetSttIdx()+1;
            sal_uLong nEndNd = pSetBox->GetSttNd()->EndOfSectionIndex();
            for( ; nSttNd < nEndNd; ++nSttNd )
            {
                SwCntntNode* pNd = pDoc->GetNodes()[ nSttNd ]->GetCntntNode();
                if( pNd )
                    pNd->SetAttr( aCharSet );
            }
        }

        if( aBoxSet.Count() )
        {
            if( pSetPara->pUndo &&
                SFX_ITEM_SET == aBoxSet.GetItemState( RES_BOXATR_FORMAT ))
                pSetPara->pUndo->SaveBoxCntnt( *pSetBox );

            pSetBox->ClaimFrmFmt()->SetFmtAttr( aBoxSet );
        }
    }
    else
        ((_FndBox*&)rpBox)->GetLines().ForEach( &lcl_SetAFmtLine, pPara );

    if( !rpBox->GetUpper()->GetUpper() )        // eine BaseLine
        ++pSetPara->nCurBox;
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
