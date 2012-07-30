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
#include <boost/foreach.hpp>

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
        aLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
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

typedef std::map<SwFrmFmt *, SwTableBoxFmt *> DfltBoxAttrMap_t;
typedef std::vector<DfltBoxAttrMap_t *> DfltBoxAttrList_t;

static void
lcl_SetDfltBoxAttr(SwTableBox& rBox, DfltBoxAttrList_t & rBoxFmtArr,
        sal_uInt8 const nId, SwTableAutoFmt const*const pAutoFmt = 0)
{
    DfltBoxAttrMap_t * pMap = rBoxFmtArr[ nId ];
    if (!pMap)
    {
        pMap = new DfltBoxAttrMap_t;
        rBoxFmtArr[ nId ] = pMap;
    }

    SwTableBoxFmt* pNewTableBoxFmt = 0;
    SwFrmFmt* pBoxFrmFmt = rBox.GetFrmFmt();
    DfltBoxAttrMap_t::iterator const iter(pMap->find(pBoxFrmFmt));
    if (pMap->end() != iter)
    {
        pNewTableBoxFmt = iter->second;
    }
    else
    {
        SwDoc* pDoc = pBoxFrmFmt->GetDoc();
        // format does not exist, so create it
        pNewTableBoxFmt = pDoc->MakeTableBoxFmt();
        pNewTableBoxFmt->SetFmtAttr( pBoxFrmFmt->GetAttrSet().Get( RES_FRM_SIZE ) );

        if( pAutoFmt )
            pAutoFmt->UpdateToSet( nId, (SfxItemSet&)pNewTableBoxFmt->GetAttrSet(),
                                    SwTableAutoFmt::UPDATE_BOX,
                                    pDoc->GetNumberFormatter( sal_True ) );
        else
            ::lcl_SetDfltBoxAttr( *pNewTableBoxFmt, nId );

        (*pMap)[pBoxFrmFmt] = pNewTableBoxFmt;
    }
    rBox.ChgFrmFmt( pNewTableBoxFmt );
}

SwTableBoxFmt *lcl_CreateDfltBoxFmt( SwDoc &rDoc, std::vector<SwTableBoxFmt*> &rBoxFmtArr,
                                    sal_uInt16 nCols, sal_uInt8 nId )
{
    if ( !rBoxFmtArr[nId] )
    {
        SwTableBoxFmt* pBoxFmt = rDoc.MakeTableBoxFmt();
        if( USHRT_MAX != nCols )
            pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        ::lcl_SetDfltBoxAttr( *pBoxFmt, nId );
        rBoxFmtArr[ nId ] = pBoxFmt;
    }
    return rBoxFmtArr[nId];
}

SwTableBoxFmt *lcl_CreateAFmtBoxFmt( SwDoc &rDoc, std::vector<SwTableBoxFmt*> &rBoxFmtArr,
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
        rBoxFmtArr[ nId ] = pBoxFmt;
    }
    return rBoxFmtArr[nId];
}

SwTableNode* SwDoc::IsIdxInTbl(const SwNodeIndex& rIdx)
{
    SwTableNode* pTableNd = 0;
    sal_uLong nIndex = rIdx.GetIndex();
    do {
        SwNode* pNd = (SwNode*)GetNodes()[ nIndex ]->StartOfSectionNode();
        if( 0 != ( pTableNd = pNd->GetTableNode() ) )
            break;

        nIndex = pNd->GetIndex();
    } while ( nIndex );
    return pTableNd;
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
    if( !pLine->GetTabBoxes().empty() )
    {
        if( nInsPos < pLine->GetTabBoxes().size() )
        {
            if( 0 == (pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos ] )))
                pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );
        }
        else
        {
            if( 0 == (pNxtBox = pLine->FindNextBox( pTblNd->GetTable(),
                            pLine->GetTabBoxes().back() )))
                pNxtBox = pLine->FindNextBox( pTblNd->GetTable() );
        }
    }
    else if( 0 == ( pNxtBox = pLine->FindNextBox( pTblNd->GetTable() )))
        pPrvBox = pLine->FindPreviousBox( pTblNd->GetTable() );

    if( !pPrvBox && !pNxtBox )
    {
        sal_Bool bSetIdxPos = sal_True;
        if( pTblNd->GetTable().GetTabLines().size() && !nInsPos )
        {
            const SwTableLine* pTblLn = pLine;
            while( pTblLn->GetUpper() )
                pTblLn = pTblLn->GetUpper()->GetUpper();

            if( pTblNd->GetTable().GetTabLines()[ 0 ] == pTblLn )
            {
                // also vor die erste Box der Tabelle
                while( ( pNxtBox = pLine->GetTabBoxes()[0])->GetTabLines().size() )
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
        if (nRealInsPos > rTabBoxes.size())
            nRealInsPos = rTabBoxes.size();

        rTabBoxes.insert( rTabBoxes.begin() + nRealInsPos, pPrvBox );

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

const SwTable* SwDoc::InsertTable( const SwInsertTableOptions& rInsTblOpts,
                                   const SwPosition& rPos, sal_uInt16 nRows,
                                   sal_uInt16 nCols, sal_Int16 eAdjust,
                                   const SwTableAutoFmt* pTAFmt,
                                   const std::vector<sal_uInt16> *pColArr,
                                   sal_Bool bCalledFromShell,
                                   sal_Bool bNewModel )
{
    OSL_ENSURE( nRows, "Tabelle ohne Zeile?" );
    OSL_ENSURE( nCols, "Tabelle ohne Spalten?" );

    {
        // nicht in Fussnoten kopieren !!
        if( rPos.nNode < GetNodes().GetEndOfInserts().GetIndex() &&
            rPos.nNode >= GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;

        // sollte das ColumnArray die falsche Anzahl haben wird es ignoriert!
        if( pColArr &&
            (size_t)(nCols + ( text::HoriOrientation::NONE == eAdjust ? 2 : 1 )) != pColArr->size() )
            pColArr = 0;
    }

    String aTblName = GetUniqueTblName();

    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsTbl( rPos, nCols, nRows, static_cast<sal_uInt16>(eAdjust),
                                      rInsTblOpts, pTAFmt, pColArr,
                                      aTblName));
    }

    // fuege erstmal die Nodes ein
    // hole das Auto-Format fuer die Tabelle
    SwTxtFmtColl *pBodyColl = GetTxtCollFromPool( RES_POOLCOLL_TABLE ),
                 *pHeadColl = pBodyColl;

    sal_Bool bDfltBorders = 0 != ( rInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER );

    if( (rInsTblOpts.mnInsMode & tabopts::HEADLINE) && (1 != nRows || !bDfltBorders) )
        pHeadColl = GetTxtCollFromPool( RES_POOLCOLL_TABLE_HDLN );

    const sal_uInt16 nRowsToRepeat =
            tabopts::HEADLINE == (rInsTblOpts.mnInsMode & tabopts::HEADLINE) ?
            rInsTblOpts.mnRowsToRepeat :
            0;

    /* Save content node to extract FRAMEDIR from. */
    const SwCntntNode * pCntntNd = rPos.nNode.GetNode().GetCntntNode();

    /* If we are called from a shell pass the attrset from
        pCntntNd (aka the node the table is inserted at) thus causing
        SwNodes::InsertTable to propagate an adjust item if
        necessary. */
    SwTableNode *pTblNd = GetNodes().InsertTable(
        rPos.nNode,
        nCols,
        pBodyColl,
        nRows,
        nRowsToRepeat,
        pHeadColl,
        bCalledFromShell ? &pCntntNd->GetSwAttrSet() : 0 );

    // dann erstelle die Box/Line/Table-Struktur
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( aTblName, GetDfltFrmFmt() );

    /* If the node to insert the table at is a context node and has a
       non-default FRAMEDIR propagate it to the table. */
    if (pCntntNd)
    {
        const SwAttrSet & aNdSet = pCntntNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    //Orientation am Fmt der Table setzen
    pTableFmt->SetFmtAttr( SwFmtHoriOrient( 0, eAdjust ) );
    // alle Zeilen haben die Fill-Order von links nach rechts !
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));

    // die Tabelle bekommt USHRT_MAX als default SSize
    SwTwips nWidth = USHRT_MAX;
    if( pColArr )
    {
        sal_uInt16 nSttPos = pColArr->front();
        sal_uInt16 nLastPos = pColArr->back();
        if( text::HoriOrientation::NONE == eAdjust )
        {
            sal_uInt16 nFrmWidth = nLastPos;
            nLastPos = (*pColArr)[ pColArr->size()-2 ];
            pTableFmt->SetFmtAttr( SvxLRSpaceItem( nSttPos, nFrmWidth - nLastPos, 0, 0, RES_LR_SPACE ) );
        }
        nWidth = nLastPos - nSttPos;
    }
    else if( nCols )
    {
        nWidth /= nCols;
        nWidth *= nCols; // to avoid rounding problems
    }
    pTableFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth ));
    if( !(rInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT) )
        pTableFmt->SetFmtAttr( SwFmtLayoutSplit( sal_False ));

    // verschiebe ggfs. die harten PageDesc/PageBreak Attribute:
    SwCntntNode* pNextNd = GetNodes()[ pTblNd->EndOfSectionIndex()+1 ]
                            ->GetCntntNode();
    if( pNextNd && pNextNd->HasSwAttrSet() )
    {
        const SfxItemSet* pNdSet = pNextNd->GetpSwAttrSet();
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, sal_False,
            &pItem ) )
        {
            pTableFmt->SetFmtAttr( *pItem );
            pNextNd->ResetAttr( RES_PAGEDESC );
            pNdSet = pNextNd->GetpSwAttrSet();
        }
        if( pNdSet && SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, sal_False,
             &pItem ) )
        {
            pTableFmt->SetFmtAttr( *pItem );
            pNextNd->ResetAttr( RES_BREAK );
        }
    }

    SwTable * pNdTbl = &pTblNd->GetTable();
    pNdTbl->RegisterToFormat( *pTableFmt );

    pNdTbl->SetRowsToRepeat( nRowsToRepeat );
    pNdTbl->SetTableModel( bNewModel );

    std::vector<SwTableBoxFmt*> aBoxFmtArr;
    SwTableBoxFmt* pBoxFmt = 0;
    if( !bDfltBorders && !pTAFmt )
    {
        pBoxFmt = MakeTableBoxFmt();
        pBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX / nCols, 0 ));
    }
    else
    {
        const sal_uInt16 nBoxArrLen = pTAFmt ? 16 : 4;
        aBoxFmtArr.resize( nBoxArrLen, NULL );
    }
    SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1 );

    SwNodeIndex aNdIdx( *pTblNd, 1 );   // auf den ersten Box-StartNode
    SwTableLines& rLines = pNdTbl->GetTabLines();
    for( sal_uInt16 n = 0; n < nRows; ++n )
    {
        SwTableLine* pLine = new SwTableLine( pLineFmt, nCols, 0 );
        rLines.insert( rLines.begin() + n, pLine );
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 i = 0; i < nCols; ++i )
        {
            SwTableBoxFmt *pBoxF;
            if( pTAFmt )
            {
                sal_uInt8 nId = static_cast<sal_uInt8>(!n ? 0 : (( n+1 == nRows )
                                        ? 12 : (4 * (1 + ((n-1) & 1 )))));
                nId = nId + static_cast<sal_uInt8>( !i ? 0 :
                            ( i+1 == nCols ? 3 : (1 + ((i-1) & 1))));
                pBoxF = ::lcl_CreateAFmtBoxFmt( *this, aBoxFmtArr, *pTAFmt,
                                                nCols, nId );

                // ggfs. noch die Absatz/ZeichenAttribute setzen
                if( pTAFmt->IsFont() || pTAFmt->IsJustify() )
                {
                    aCharSet.ClearItem();
                    pTAFmt->UpdateToSet( nId, aCharSet,
                                        SwTableAutoFmt::UPDATE_CHAR, 0 );
                    if( aCharSet.Count() )
                        GetNodes()[ aNdIdx.GetIndex()+1 ]->GetCntntNode()->
                            SetAttr( aCharSet );
                }
            }
            else if( bDfltBorders )
            {
                sal_uInt8 nBoxId = (i < nCols - 1 ? 0 : 1) + (n ? 2 : 0 );
                pBoxF = ::lcl_CreateDfltBoxFmt( *this, aBoxFmtArr, nCols, nBoxId);
            }
            else
                pBoxF = pBoxFmt;

            // fuer AutoFormat bei der Eingabe: beim Einfuegen der Tabelle
            // werden gleich die Spalten gesetzt. Im Array stehen die
            // Positionen der Spalten!! (nicht deren Breite!)
            if( pColArr )
            {
                nWidth = (*pColArr)[ i + 1 ] - (*pColArr)[ i ];
                if( pBoxF->GetFrmSize().GetWidth() != nWidth )
                {
                    if( pBoxF->GetDepends() )       // neues Format erzeugen!
                    {
                        SwTableBoxFmt *pNewFmt = MakeTableBoxFmt();
                        *pNewFmt = *pBoxF;
                        pBoxF = pNewFmt;
                    }
                    pBoxF->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth ));
                }
            }

            SwTableBox *pBox = new SwTableBox( pBoxF, aNdIdx, pLine);
            rBoxes.insert( rBoxes.begin() + i, pBox );
            aNdIdx += 3;        // StartNode, TextNode, EndNode  == 3 Nodes
        }
    }
    // und Frms einfuegen.
    GetNodes().GoNext( &aNdIdx );      // zum naechsten ContentNode
    pTblNd->MakeFrms( &aNdIdx );

    if( IsRedlineOn() || (!IsIgnoreRedline() && !pRedlineTbl->empty() ))
    {
        SwPaM aPam( *pTblNd->EndOfSectionNode(), *pTblNd, 1 );
        if( IsRedlineOn() )
            AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        else
            SplitRedline( aPam );
    }

    SetModified();
    CHECK_TABLE( *pNdTbl );
    return pNdTbl;
}

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

const SwTable* SwDoc::TextToTable( const SwInsertTableOptions& rInsTblOpts,
                                   const SwPaM& rRange, sal_Unicode cCh,
                                   sal_Int16 eAdjust,
                                   const SwTableAutoFmt* pTAFmt )
{
    // pruefe ob in der Selection eine Tabelle liegt
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    {
        sal_uLong nCnt = pStt->nNode.GetIndex();
        for( ; nCnt <= pEnd->nNode.GetIndex(); ++nCnt )
            if( !GetNodes()[ nCnt ]->IsTxtNode() )
                return 0;
    }

    /* Save first node in the selection if it is a context node. */
    SwCntntNode * pSttCntntNd = pStt->nNode.GetNode().GetCntntNode();

    SwPaM aOriginal( *pStt, *pEnd );
    pStt = aOriginal.GetMark();
    pEnd = aOriginal.GetPoint();

    SwUndoTxtToTbl* pUndo = 0;
    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_TEXTTOTABLE, NULL );
        pUndo = new SwUndoTxtToTbl( aOriginal, rInsTblOpts, cCh,
                    static_cast<sal_uInt16>(eAdjust), pTAFmt );
        GetIDocumentUndoRedo().AppendUndo( pUndo );

        // das Splitten vom TextNode nicht in die Undohistory aufnehmen
        GetIDocumentUndoRedo().DoUndo( false );
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    // sorge dafuer, das der Bereich auf Node-Grenzen liegt
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        SplitNode( *pStt, false );

    sal_Bool bEndCntnt = 0 != pEnd->nContent.GetIndex();
    // nicht splitten am Ende der Zeile (aber am Ende vom Doc!!)
    if( bEndCntnt )
    {
        if( pEnd->nNode.GetNode().GetCntntNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            SplitNode( *pEnd, false );
            ((SwNodeIndex&)pEnd->nNode)--;
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetCntntNode(), 0 );
            // ein Node und am Ende ??
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                aRg.aStart--;
        }
        else
            aRg.aEnd++;
    }


    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "Kein Bereich" );
        aRg.aEnd++;
    }

    // Wir gehen jetzt immer ueber die Upper, um die Tabelle einzufuegen:
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo( 0 != pUndo );

    // dann erstelle die Box/Line/Table-Struktur
    SwTableBoxFmt* pBoxFmt = MakeTableBoxFmt();
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( GetUniqueTblName(), GetDfltFrmFmt() );

    // alle Zeilen haben die Fill-Order von links nach rechts !
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));
    // die Tabelle bekommt USHRT_MAX als default SSize
    pTableFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX ));
    if( !(rInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT) )
        pTableFmt->SetFmtAttr( SwFmtLayoutSplit( sal_False ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttCntntNd)
    {
        const SwAttrSet & aNdSet = pSttCntntNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    SwTableNode* pTblNd = GetNodes().TextToTable(
            aRg, cCh, pTableFmt, pLineFmt, pBoxFmt,
            GetTxtCollFromPool( RES_POOLCOLL_STANDARD ), pUndo );

    SwTable * pNdTbl = &pTblNd->GetTable();
    OSL_ENSURE( pNdTbl, "kein Tabellen-Node angelegt."  );

    const sal_uInt16 nRowsToRepeat =
            tabopts::HEADLINE == (rInsTblOpts.mnInsMode & tabopts::HEADLINE) ?
            rInsTblOpts.mnRowsToRepeat :
            0;
    pNdTbl->SetRowsToRepeat( nRowsToRepeat );

    sal_Bool bUseBoxFmt = sal_False;
    if( !pBoxFmt->GetDepends() )
    {
        // die Formate an den Boxen haben schon die richtige Size, es darf
        // also nur noch die richtige Umrandung/AutoFmt gesetzt werden.
        bUseBoxFmt = sal_True;
        pTableFmt->SetFmtAttr( pBoxFmt->GetFrmSize() );
        delete pBoxFmt;
        eAdjust = text::HoriOrientation::NONE;
    }

    //Orientation am Fmt der Table setzen
    pTableFmt->SetFmtAttr( SwFmtHoriOrient( 0, eAdjust ) );
    pNdTbl->RegisterToFormat( *pTableFmt );

    if( pTAFmt || ( rInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER) )
    {
        sal_uInt8 nBoxArrLen = pTAFmt ? 16 : 4;
        boost::scoped_ptr< DfltBoxAttrList_t > aBoxFmtArr1;
        boost::scoped_ptr< std::vector<SwTableBoxFmt*> > aBoxFmtArr2;
        if( bUseBoxFmt )
        {
            aBoxFmtArr1.reset(new DfltBoxAttrList_t( nBoxArrLen, NULL ));
        }
        else
        {
            aBoxFmtArr2.reset(new std::vector<SwTableBoxFmt*>( nBoxArrLen, NULL ));
        }


        SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1 );

        SwHistory* pHistory = pUndo ? &pUndo->GetHistory() : 0;

        SwTableBoxFmt *pBoxF = 0;
        SwTableLines& rLines = pNdTbl->GetTabLines();
        sal_uInt16 nRows = rLines.size();
        for( sal_uInt16 n = 0; n < nRows; ++n )
        {
            SwTableBoxes& rBoxes = rLines[ n ]->GetTabBoxes();
            sal_uInt16 nCols = rBoxes.size();
            for( sal_uInt16 i = 0; i < nCols; ++i )
            {
                SwTableBox* pBox = rBoxes[ i ];
                sal_Bool bChgSz = sal_False;

                if( pTAFmt )
                {
                    sal_uInt8 nId = static_cast<sal_uInt8>(!n ? 0 : (( n+1 == nRows )
                                            ? 12 : (4 * (1 + ((n-1) & 1 )))));
                    nId = nId + static_cast<sal_uInt8>(!i ? 0 :
                                ( i+1 == nCols ? 3 : (1 + ((i-1) & 1))));
                    if( bUseBoxFmt )
                        ::lcl_SetDfltBoxAttr( *pBox, *aBoxFmtArr1, nId, pTAFmt );
                    else
                    {
                        bChgSz = 0 == (*aBoxFmtArr2)[ nId ];
                        pBoxF = ::lcl_CreateAFmtBoxFmt( *this, *aBoxFmtArr2,
                                                *pTAFmt, USHRT_MAX, nId );
                    }

                    // ggfs. noch die Absatz/ZeichenAttribute setzen
                    if( pTAFmt->IsFont() || pTAFmt->IsJustify() )
                    {
                        aCharSet.ClearItem();
                        pTAFmt->UpdateToSet( nId, aCharSet,
                                            SwTableAutoFmt::UPDATE_CHAR, 0 );
                        if( aCharSet.Count() )
                        {
                            sal_uLong nSttNd = pBox->GetSttIdx()+1;
                            sal_uLong nEndNd = pBox->GetSttNd()->EndOfSectionIndex();
                            for( ; nSttNd < nEndNd; ++nSttNd )
                            {
                                SwCntntNode* pNd = GetNodes()[ nSttNd ]->GetCntntNode();
                                if( pNd )
                                {
                                    if( pHistory )
                                    {
                                        SwRegHistory aReg( pNd, *pNd, pHistory );
                                        pNd->SetAttr( aCharSet );
                                    }
                                    else
                                        pNd->SetAttr( aCharSet );
                                }
                            }
                        }
                    }
                }
                else
                {
                    sal_uInt8 nId = (i < nCols - 1 ? 0 : 1) + (n ? 2 : 0 );
                    if( bUseBoxFmt )
                        ::lcl_SetDfltBoxAttr( *pBox, *aBoxFmtArr1, nId );
                    else
                    {
                        bChgSz = 0 == (*aBoxFmtArr2)[ nId ];
                        pBoxF = ::lcl_CreateDfltBoxFmt( *this, *aBoxFmtArr2,
                                                        USHRT_MAX, nId );
                    }
                }

                if( !bUseBoxFmt )
                {
                    if( bChgSz )
                        pBoxF->SetFmtAttr( pBox->GetFrmFmt()->GetFrmSize() );
                    pBox->ChgFrmFmt( pBoxF );
                }
            }
        }

        if( bUseBoxFmt )
        {
            for( sal_uInt8 i = 0; i < nBoxArrLen; ++i )
            {
                delete (*aBoxFmtArr1)[ i ];
            }
        }
    }

    // JP 03.04.97: Inhalt der Boxen auf Zahlen abpruefen
    if( IsInsTblFormatNum() )
    {
        for( sal_uInt16 nBoxes = pNdTbl->GetTabSortBoxes().size(); nBoxes; )
            ChkBoxNumFmt( *pNdTbl->GetTabSortBoxes()[ --nBoxes ], sal_False );
    }

    sal_uLong nIdx = pTblNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    {
        SwPaM& rTmp = (SwPaM&)rRange;   // Point immer an den Anfang
        rTmp.DeleteMark();
        rTmp.GetPoint()->nNode = *pTblNd;
        SwCntntNode* pCNd = GetNodes().GoNext( &rTmp.GetPoint()->nNode );
        rTmp.GetPoint()->nContent.Assign( pCNd, 0 );
    }

    if( pUndo )
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_TEXTTOTABLE, NULL );
    }

    SetModified();
    SetFieldsDirty(true, NULL, 0);
    return pNdTbl;
}

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
        pTable->GetTabLines().insert( pTable->GetTabLines().begin() + nLines, pLine );

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
                    pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
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
        pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    // die Tabelle ausgleichen, leere Sections einfuegen
    sal_uInt16 n;

    for( n = 0; n < pTable->GetTabLines().size(); ++n )
    {
        SwTableLine* pCurrLine = pTable->GetTabLines()[ n ];
        if( nMaxBoxes != ( nBoxes = pCurrLine->GetTabBoxes().size() ))
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
            for( sal_uInt16 nTmpLine = 0; nTmpLine < rLns.size(); ++nTmpLine )
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

const SwTable* SwDoc::TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes )
{
    if (rTableNodes.empty())
        return NULL;

    std::vector<SwNodeRange> rFirstRange = *rTableNodes.begin();

    if (rFirstRange.empty())
        return NULL;

    /* Save first node in the selection if it is a content node. */
    SwCntntNode * pSttCntntNd = rFirstRange.begin()->aStart.GetNode().GetCntntNode();

    /**debug**/
#if OSL_DEBUG_LEVEL > 1
    const SwNodeRange& rStartRange = *rTableNodes.begin()->begin();
    const SwNodeRange& rEndRange = *rTableNodes.rbegin()->rbegin();
    (void) rStartRange;
    (void) rEndRange;
#endif
    /**debug**/

    //!!! not necessarily TextNodes !!!
    SwPaM aOriginal( rTableNodes.begin()->begin()->aStart, rTableNodes.rbegin()->rbegin()->aEnd );
    const SwPosition *pStt = aOriginal.GetMark();
    const SwPosition *pEnd = aOriginal.GetPoint();

//    SwUndoTxtToTbl* pUndo = 0;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        // das Splitten vom TextNode nicht in die Undohistory aufnehmen
        GetIDocumentUndoRedo().DoUndo(false);
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    // sorge dafuer, das der Bereich auf Node-Grenzen liegt
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        SplitNode( *pStt, false );

    sal_Bool bEndCntnt = 0 != pEnd->nContent.GetIndex();
    // nicht splitten am Ende der Zeile (aber am Ende vom Doc!!)
    if( bEndCntnt )
    {
        if( pEnd->nNode.GetNode().GetCntntNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            SplitNode( *pEnd, false );
            ((SwNodeIndex&)pEnd->nNode)--;
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetCntntNode(), 0 );
            // ein Node und am Ende ??
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                aRg.aStart--;
        }
        else
            aRg.aEnd++;
    }


    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "Kein Bereich" );
        aRg.aEnd++;
    }

    // Wir gehen jetzt immer ueber die Upper, um die Tabelle einzufuegen:
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo(bUndo);

    // dann erstelle die Box/Line/Table-Struktur
    SwTableBoxFmt* pBoxFmt = MakeTableBoxFmt();
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( GetUniqueTblName(), GetDfltFrmFmt() );

    // alle Zeilen haben die Fill-Order von links nach rechts !
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));
    // die Tabelle bekommt USHRT_MAX als default SSize
    pTableFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttCntntNd)
    {
        const SwAttrSet & aNdSet = pSttCntntNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    SwTableNode* pTblNd = GetNodes().TextToTable(
            rTableNodes, pTableFmt, pLineFmt, pBoxFmt,
            GetTxtCollFromPool( RES_POOLCOLL_STANDARD )/*, pUndo*/ );

    SwTable * pNdTbl = &pTblNd->GetTable();
    OSL_ENSURE( pNdTbl, "kein Tabellen-Node angelegt."  );
    pNdTbl->RegisterToFormat( *pTableFmt );

    if( !pBoxFmt->GetDepends() )
    {
        // die Formate an den Boxen haben schon die richtige Size, es darf
        // also nur noch die richtige Umrandung/AutoFmt gesetzt werden.
        pTableFmt->SetFmtAttr( pBoxFmt->GetFrmSize() );
        delete pBoxFmt;
    }

    sal_uLong nIdx = pTblNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    SetModified();
    SetFieldsDirty( true, NULL, 0 );
    return pNdTbl;
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
        pTable->GetTabLines().insert( pTable->GetTabLines().begin() + nLines, pLine );

        std::vector< SwNodeRange >::const_iterator aCellIter = aRowIter->begin();

        for( ; aCellIter != aRowIter->end(); ++aCellIter )
        {
                const SwNodeIndex aTmpIdx( aCellIter->aStart, 0 );

               SwNodeIndex aCellEndIdx(aCellIter->aEnd);
               ++aCellEndIdx;
               SwStartNode* pSttNd = new SwStartNode( aTmpIdx, ND_STARTNODE,
                                            SwTableBoxStartNode );

                // Quotation of http://nabble.documentfoundation.org/Some-strange-lines-by-taking-a-look-at-the-bt-of-fdo-51916-tp3994561p3994639.html
                // SwNode's constructor adds itself to the same SwNodes array as the other node (pSttNd).
                // So this statement is only executed for the side-effect.
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
                pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
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
            for( sal_uInt16 nLines2 = 0; nLines2 < rLns.size(); ++nLines2 )
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


sal_Bool SwDoc::TableToText( const SwTableNode* pTblNd, sal_Unicode cCh )
{
    if( !pTblNd )
        return sal_False;

    // #i34471#
    // If this is trigged by SwUndoTblToTxt::Repeat() nobody ever deleted
    // the table cursor.
    SwEditShell* pESh = GetEditShell();
    if( pESh && pESh->IsTableMode() )
        pESh->ClearMark();

    SwNodeRange aRg( *pTblNd, 0, *pTblNd->EndOfSectionNode() );
    SwUndoTblToTxt* pUndo = 0;
    SwNodeRange* pUndoRg = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoRg = new SwNodeRange( aRg.aStart, -1, aRg.aEnd, +1 );
        pUndo = new SwUndoTblToTxt( pTblNd->GetTable(), cCh );
    }

    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_BOXNAME;
    UpdateTblFlds( &aMsgHnt );

    sal_Bool bRet = GetNodes().TableToText( aRg, cCh, pUndo );
    if( pUndoRg )
    {
        pUndoRg->aStart++;
        pUndoRg->aEnd--;
        pUndo->SetRange( *pUndoRg );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        delete pUndoRg;
    }

    if( bRet )
        SetModified();

    return bRet;
}

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
static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara );

static void lcl_DelLine( SwTableLine* pLine, _DelTabPara* pPara )
{
    OSL_ENSURE( pPara, "die Parameter fehlen" );
    _DelTabPara aPara( *pPara );
    for( SwTableBoxes::iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        lcl_DelBox(*it, &aPara );
    if( pLine->GetUpper() )        // gibt es noch eine uebergeordnete Box ??
        // dann gebe den letzten TextNode zurueck
        pPara->pLastNd = aPara.pLastNd;
}


static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara )
{
    OSL_ENSURE( pDelPara, "die Parameter fehlen" );

    // loesche erstmal die Lines der Box
    if( !pBox->GetTabLines().empty() )
        BOOST_FOREACH( SwTableLine* pLine, pBox->GetTabLines() )
            lcl_DelLine( pLine, pDelPara );
    else
    {
        SwDoc* pDoc = pDelPara->rNds.GetDoc();
        SwNodeRange aDelRg( *pBox->GetSttNd(), 0,
                            *pBox->GetSttNd()->EndOfSectionNode() );
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
                pDelPara->pLastNd->InsertText( rtl::OUString(pDelPara->cCh), aCntIdx,
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
    BOOST_FOREACH( SwTableLine *pLine, pTblNd->pTable->GetTabLines() )
        lcl_DelLine( pLine, &aDelPara );

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
    const SwFrmFmts& rFlyArr = *GetDoc()->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFlyArr.size(); ++n )
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


// ----- einfuegen von Spalten/Zeilen ------------------------

sal_Bool SwDoc::InsertCol( const SwCursor& rCursor, sal_uInt16 nCnt, sal_Bool bBehind )
{
    if( !::CheckSplitCells( rCursor, nCnt + 1, nsSwTblSearchType::TBLSEARCH_COL ) )
        return sal_False;

    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    ::GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );

    sal_Bool bRet = sal_False;
    if( !aBoxes.empty() )
        bRet = InsertCol( aBoxes, nCnt, bBehind );
    return bRet;
}

sal_Bool SwDoc::InsertCol( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, sal_Bool bBehind )
{
    // uebers SwDoc fuer Undo !!
    OSL_ENSURE( !rBoxes.empty(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    SwTable& rTbl = pTblNd->GetTable();
    if( rTbl.ISA( SwDDETable ))
        return sal_False;

    SwTableSortBoxes aTmpLst;
    SwUndoTblNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTblNdsChg( UNDO_TABLE_INSCOL, rBoxes, *pTblNd,
                                     0, 0, nCnt, bBehind, sal_False );
        aTmpLst.insert( rTbl.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFmlUpdate aMsgHnt( &rTbl );
        aMsgHnt.eFlags = TBL_BOXPTR;
        UpdateTblFlds( &aMsgHnt );

        bRet = rTbl.InsertCol( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            SetModified();
            ::ClearFEShellTabCols();
            SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            pUndo->SaveNewBoxes( *pTblNd, aTmpLst );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }
    return bRet;
}

sal_Bool SwDoc::InsertRow( const SwCursor& rCursor, sal_uInt16 nCnt, sal_Bool bBehind )
{
    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );

    sal_Bool bRet = sal_False;
    if( !aBoxes.empty() )
        bRet = InsertRow( aBoxes, nCnt, bBehind );
    return bRet;
}

sal_Bool SwDoc::InsertRow( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, sal_Bool bBehind )
{
    // uebers SwDoc fuer Undo !!
    OSL_ENSURE( !rBoxes.empty(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    SwTable& rTbl = pTblNd->GetTable();
    if( rTbl.ISA( SwDDETable ))
        return sal_False;

    SwTableSortBoxes aTmpLst;
    SwUndoTblNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTblNdsChg( UNDO_TABLE_INSROW,rBoxes, *pTblNd,
                                     0, 0, nCnt, bBehind, sal_False );
        aTmpLst.insert( rTbl.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFmlUpdate aMsgHnt( &rTbl );
        aMsgHnt.eFlags = TBL_BOXPTR;
        UpdateTblFlds( &aMsgHnt );

        bRet = rTbl.InsertRow( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            SetModified();
            ::ClearFEShellTabCols();
            SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            pUndo->SaveNewBoxes( *pTblNd, aTmpLst );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }
    return bRet;

}

// ----- loeschen von Spalten/Zeilen ------------------------

sal_Bool SwDoc::DeleteRow( const SwCursor& rCursor )
{
    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );
    if( ::HasProtectedCells( aBoxes ))
        return sal_False;

    // die Crsr aus dem Loeschbereich entfernen.
    // Der Cursor steht danach:
    //  - es folgt noch eine Zeile, in dieser
    //  - vorher steht noch eine Zeile, in dieser
    //  - sonst immer dahinter
    {
        SwTableNode* pTblNd = rCursor.GetNode()->FindTableNode();

        if( pTblNd->GetTable().ISA( SwDDETable ))
            return sal_False;

        // suche alle Boxen / Lines
        _FndBox aFndBox( 0, 0 );
        {
            _FndPara aPara( aBoxes, &aFndBox );
            ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
        }

        if( !aFndBox.GetLines().size() )
            return sal_False;

        SwEditShell* pESh = GetEditShell();
        if( pESh )
        {
            pESh->KillPams();
            // JP: eigentlich sollte man ueber alle Shells iterieren!!
        }

        _FndBox* pFndBox = &aFndBox;
        while( 1 == pFndBox->GetLines().size() &&
                1 == pFndBox->GetLines().front().GetBoxes().size() )
        {
            _FndBox *const pTmp = & pFndBox->GetLines().front().GetBoxes()[0];
            if( pTmp->GetBox()->GetSttNd() )
                break;      // das ist sonst zu weit
            pFndBox = pTmp;
        }

        SwTableLine* pDelLine = pFndBox->GetLines().back().GetLine();
        SwTableBox* pDelBox = pDelLine->GetTabBoxes().back();
        while( !pDelBox->GetSttNd() )
        {
            SwTableLine* pLn = pDelBox->GetTabLines()[
                        pDelBox->GetTabLines().size()-1 ];
            pDelBox = pLn->GetTabBoxes().back();
        }
        SwTableBox* pNextBox = pDelLine->FindNextBox( pTblNd->GetTable(),
                                                        pDelBox, sal_True );
        while( pNextBox &&
                pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
            pNextBox = pNextBox->FindNextBox( pTblNd->GetTable(), pNextBox );

        if( !pNextBox )         // keine nachfolgende? dann die vorhergehende
        {
            pDelLine = pFndBox->GetLines().front().GetLine();
            pDelBox = pDelLine->GetTabBoxes()[ 0 ];
            while( !pDelBox->GetSttNd() )
                pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
            pNextBox = pDelLine->FindPreviousBox( pTblNd->GetTable(),
                                                        pDelBox, sal_True );
            while( pNextBox &&
                    pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                pNextBox = pNextBox->FindPreviousBox( pTblNd->GetTable(), pNextBox );
        }

        sal_uLong nIdx;
        if( pNextBox )      // dann den Cursor hier hinein
            nIdx = pNextBox->GetSttIdx() + 1;
        else                // ansonsten hinter die Tabelle
            nIdx = pTblNd->EndOfSectionIndex() + 1;

        SwNodeIndex aIdx( GetNodes(), nIdx );
        SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
        if( !pCNd )
            pCNd = GetNodes().GoNext( &aIdx );

        if( pCNd )
        {
            // die Cursor von der Shell oder den uebergebenen Cursor aendern?
            SwPaM* pPam = (SwPaM*)&rCursor;
            pPam->GetPoint()->nNode = aIdx;
            pPam->GetPoint()->nContent.Assign( pCNd, 0 );
            pPam->SetMark();            // beide wollen etwas davon haben
            pPam->DeleteMark();
        }
    }

    // dann loesche doch die Zeilen

    GetIDocumentUndoRedo().StartUndo(UNDO_ROW_DELETE, NULL);
    sal_Bool bResult = DeleteRowCol( aBoxes );
    GetIDocumentUndoRedo().EndUndo(UNDO_ROW_DELETE, NULL);

    return bResult;
}

sal_Bool SwDoc::DeleteCol( const SwCursor& rCursor )
{
    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
    if( ::HasProtectedCells( aBoxes ))
        return sal_False;

    // die Crsr muessen noch aus dem Loesch Bereich entfernt
    // werden. Setze sie immer hinter/auf die Tabelle; ueber die
    // Dokument-Position werden sie dann immer an die alte Position gesetzt.
    SwEditShell* pESh = GetEditShell();
    if( pESh )
    {
        const SwNode* pNd = rCursor.GetNode()->FindTableBoxStartNode();
        pESh->ParkCrsr( SwNodeIndex( *pNd ) );
    }

    // dann loesche doch die Spalten
    GetIDocumentUndoRedo().StartUndo(UNDO_COL_DELETE, NULL);
    sal_Bool bResult = DeleteRowCol( aBoxes, true );
    GetIDocumentUndoRedo().EndUndo(UNDO_COL_DELETE, NULL);

    return bResult;
}

sal_Bool SwDoc::DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn )
{
    if( ::HasProtectedCells( rBoxes ))
        return sal_False;

    // uebers SwDoc fuer Undo !!
    OSL_ENSURE( !rBoxes.empty(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    if( pTblNd->GetTable().ISA( SwDDETable ))
        return sal_False;

    ::ClearFEShellTabCols();
    SwSelBoxes aSelBoxes( rBoxes );
    SwTable &rTable = pTblNd->GetTable();
    long nMin = 0;
    long nMax = 0;
    if( rTable.IsNewModel() )
    {
        if( bColumn )
            rTable.ExpandColumnSelection( aSelBoxes, nMin, nMax );
        else
            rTable.FindSuperfluousRows( aSelBoxes );
    }

    // soll die gesamte Tabelle geloescht werden ??
    const sal_uLong nTmpIdx1 = pTblNd->GetIndex();
    const sal_uLong nTmpIdx2 = aSelBoxes.back()->GetSttNd()->EndOfSectionIndex() + 1;
    if( pTblNd->GetTable().GetTabSortBoxes().size() == aSelBoxes.size() &&
        aSelBoxes[0]->GetSttIdx()-1 == nTmpIdx1 &&
        nTmpIdx2 == pTblNd->EndOfSectionIndex() )
    {
        sal_Bool bNewTxtNd = sal_False;
        // steht diese auch noch alleine in einem FlyFrame ?
        SwNodeIndex aIdx( *pTblNd, -1 );
        const SwStartNode* pSttNd = aIdx.GetNode().GetStartNode();
        if( pSttNd )
        {
            const sal_uLong nTblEnd = pTblNd->EndOfSectionIndex() + 1;
            const sal_uLong nSectEnd = pSttNd->EndOfSectionIndex();
            if( nTblEnd == nSectEnd )
            {
                if( SwFlyStartNode == pSttNd->GetStartNodeType() )
                {
                    SwFrmFmt* pFmt = pSttNd->GetFlyFmt();
                    if( pFmt )
                    {
                        // Ok, das ist das gesuchte FlyFormat
                        DelLayoutFmt( pFmt );
                        return sal_True;
                    }
                }
                // kein Fly ?? also Kopf- oder Fusszeile: dann immer einen
                // TextNode ueberig lassen.
                // Undo koennen wir dann vergessen !!
                bNewTxtNd = sal_True;
            }
        }

        // kein Fly ?? also Kopf- oder Fusszeile: dann immer einen
        // TextNode ueberig lassen.
        aIdx++;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().ClearRedo();
            SwPaM aPaM( *pTblNd->EndOfSectionNode(), aIdx.GetNode() );

            if( bNewTxtNd )
            {
                const SwNodeIndex aTmpIdx( *pTblNd->EndOfSectionNode(), 1 );
                GetNodes().MakeTxtNode( aTmpIdx,
                            GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            // save the cursors (UNO and otherwise)
            SwPaM aSavePaM( SwNodeIndex( *pTblNd->EndOfSectionNode() ) );
            if( ! aSavePaM.Move( fnMoveForward, fnGoNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTblNd );
                aSavePaM.Move( fnMoveBackward, fnGoNode );
            }
            {
                SwPaM const tmpPaM(*pTblNd, *pTblNd->EndOfSectionNode());
                ::PaMCorrAbs(tmpPaM, *aSavePaM.GetMark());
            }

            // harte SeitenUmbrueche am nachfolgenden Node verschieben
            sal_Bool bSavePageBreak = sal_False, bSavePageDesc = sal_False;
            sal_uLong nNextNd = pTblNd->EndOfSectionIndex()+1;
            SwCntntNode* pNextNd = GetNodes()[ nNextNd ]->GetCntntNode();
            if( pNextNd )
            {
                {
                    SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
                    const SfxPoolItem *pItem;
                    if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
                        sal_False, &pItem ) )
                    {
                        pNextNd->SetAttr( *pItem );
                        bSavePageDesc = sal_True;
                    }

                    if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
                        sal_False, &pItem ) )
                    {
                        pNextNd->SetAttr( *pItem );
                        bSavePageBreak = sal_True;
                    }
                }
            }
            SwUndoDelete* pUndo = new SwUndoDelete( aPaM );
            if( bNewTxtNd )
                pUndo->SetTblDelLastNd();
            pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
            pUndo->SetTableName(pTblNd->GetTable().GetFrmFmt()->GetName());
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
        {
            if( bNewTxtNd )
            {
                const SwNodeIndex aTmpIdx( *pTblNd->EndOfSectionNode(), 1 );
                GetNodes().MakeTxtNode( aTmpIdx,
                            GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            // save the cursors (UNO and otherwise)
            SwPaM aSavePaM( SwNodeIndex( *pTblNd->EndOfSectionNode() ) );
            if( ! aSavePaM.Move( fnMoveForward, fnGoNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTblNd );
                aSavePaM.Move( fnMoveBackward, fnGoNode );
            }
            {
                SwPaM const tmpPaM(*pTblNd, *pTblNd->EndOfSectionNode());
                ::PaMCorrAbs(tmpPaM, *aSavePaM.GetMark());
            }

            // harte SeitenUmbrueche am nachfolgenden Node verschieben
            SwCntntNode* pNextNd = GetNodes()[ pTblNd->EndOfSectionIndex()+1 ]->GetCntntNode();
            if( pNextNd )
            {
                SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
                const SfxPoolItem *pItem;
                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
                    sal_False, &pItem ) )
                    pNextNd->SetAttr( *pItem );

                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
                    sal_False, &pItem ) )
                    pNextNd->SetAttr( *pItem );
            }

            pTblNd->DelFrms();
            DeleteSection( pTblNd );
        }
        SetModified();
        SetFieldsDirty( true, NULL, 0 );
        return sal_True;
    }

    SwUndoTblNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTblNdsChg( UNDO_TABLE_DELBOX, aSelBoxes, *pTblNd,
                                     nMin, nMax, 0, sal_False, sal_False );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
        aMsgHnt.eFlags = TBL_BOXPTR;
        UpdateTblFlds( &aMsgHnt );

        if (rTable.IsNewModel())
        {
            if (bColumn)
                rTable.PrepareDeleteCol( nMin, nMax );
            rTable.FindSuperfluousRows( aSelBoxes );
            if (pUndo)
                pUndo->ReNewBoxes( aSelBoxes );
        }
        bRet = rTable.DeleteSel( this, aSelBoxes, 0, pUndo, sal_True, sal_True );
        if (bRet)
        {
            SetModified();
            SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }

    return bRet;
}


// ---------- teilen / zusammenfassen von Boxen in der Tabelle --------

sal_Bool SwDoc::SplitTbl( const SwSelBoxes& rBoxes, sal_Bool bVert, sal_uInt16 nCnt,
                      sal_Bool bSameHeight )
{
    // uebers SwDoc fuer Undo !!
    OSL_ENSURE( !rBoxes.empty() && nCnt, "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    SwTable& rTbl = pTblNd->GetTable();
    if( rTbl.ISA( SwDDETable ))
        return sal_False;

    std::vector<sal_uLong> aNdsCnts;
    SwTableSortBoxes aTmpLst;
    SwUndoTblNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTblNdsChg( UNDO_TABLE_SPLIT, rBoxes, *pTblNd, 0, 0,
                                     nCnt, bVert, bSameHeight );

        aTmpLst.insert( rTbl.GetTabSortBoxes() );
        if( !bVert )
        {
            for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
            {
                const SwStartNode* pSttNd = rBoxes[ n ]->GetSttNd();
                aNdsCnts.push_back( pSttNd->EndOfSectionIndex() -
                                    pSttNd->GetIndex() );
            }
        }
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFmlUpdate aMsgHnt( &rTbl );
        aMsgHnt.eFlags = TBL_BOXPTR;
        UpdateTblFlds( &aMsgHnt );

        if (bVert)
            bRet = rTbl.SplitCol( this, rBoxes, nCnt );
        else
            bRet = rTbl.SplitRow( this, rBoxes, nCnt, bSameHeight );

        if (bRet)
        {
            SetModified();
            SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            if( bVert )
                pUndo->SaveNewBoxes( *pTblNd, aTmpLst );
            else
                pUndo->SaveNewBoxes( *pTblNd, aTmpLst, rBoxes, aNdsCnts );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }

    return bRet;
}


sal_uInt16 SwDoc::MergeTbl( SwPaM& rPam )
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwTableNode* pTblNd = rPam.GetNode()->FindTableNode();
    if( !pTblNd )
        return TBLMERGE_NOSELECTION;
    SwTable& rTable = pTblNd->GetTable();
    if( rTable.ISA(SwDDETable) )
        return TBLMERGE_NOSELECTION;
    sal_uInt16 nRet = TBLMERGE_NOSELECTION;
    if( !rTable.IsNewModel() )
    {
        nRet =::CheckMergeSel( rPam );
        if( TBLMERGE_OK != nRet )
            return nRet;
        nRet = TBLMERGE_NOSELECTION;
    }

    // #i33394#
    GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_MERGE, NULL );

    RedlineMode_t eOld = GetRedlineMode();
    SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

    SwUndoTblMerge *const pUndo( (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTblMerge( rPam )
        :   0 );

    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    SwSelBoxes aMerged;
    SwTableBox* pMergeBox;

    if( !rTable.PrepareMerge( rPam, aBoxes, aMerged, &pMergeBox, pUndo ) )
    {   // no cells found to merge
        SetRedlineMode_intern( eOld );
        if( pUndo )
        {
            delete pUndo;
            SwUndoId nLastUndoId(UNDO_EMPTY);
            if (GetIDocumentUndoRedo().GetLastUndoInfo(0, & nLastUndoId)
                && (UNDO_REDLINE == nLastUndoId))
            {
                // FIXME: why is this horrible cleanup necessary?
                SwUndoRedline *const pU = dynamic_cast<SwUndoRedline*>(
                        GetUndoManager().RemoveLastUndo());
                if( pU->GetRedlSaveCount() )
                {
                    SwEditShell *const pEditShell(GetEditShell(0));
                    OSL_ASSERT(pEditShell);
                    ::sw::UndoRedoContext context(*this, *pEditShell);
                    static_cast<SfxUndoAction *>(pU)->UndoWithContext(context);
                }
                delete pU;
            }
        }
    }
    else
    {
        // die PaMs muessen noch aus dem Loesch Bereich entfernt
        // werden. Setze sie immer hinter/auf die Tabelle; ueber die
        // Dokument-Position werden sie dann immer an die alte Position gesetzt.
        // Erstmal einen Index auf die Parkposition merken, denn nach GetMergeSel
        // komme ich nicht mehr dran.
        {
            rPam.DeleteMark();
            rPam.GetPoint()->nNode = *pMergeBox->GetSttNd();
            rPam.GetPoint()->nContent.Assign( 0, 0 );
            rPam.SetMark();
            rPam.DeleteMark();

            SwPaM* pTmp = &rPam;
            while( &rPam != ( pTmp = (SwPaM*)pTmp->GetNext() ))
                for( int i = 0; i < 2; ++i )
                    pTmp->GetBound( (sal_Bool)i ) = *rPam.GetPoint();
        }

        // dann fuege sie zusammen
        SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
        aMsgHnt.eFlags = TBL_BOXPTR;
        UpdateTblFlds( &aMsgHnt );

        if( pTblNd->GetTable().Merge( this, aBoxes, aMerged, pMergeBox, pUndo ))
        {
            nRet = TBLMERGE_OK;
            SetModified();
            SetFieldsDirty( true, NULL, 0 );
            if( pUndo )
            {
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
        }
        else
            delete pUndo;

        rPam.GetPoint()->nNode = *pMergeBox->GetSttNd();
        rPam.Move();

        ::ClearFEShellTabCols();
        SetRedlineMode_intern( eOld );
    }
    GetIDocumentUndoRedo().EndUndo( UNDO_TABLE_MERGE, NULL );
    return nRet;
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

void SwDoc::GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                        const SwCellFrm* pBoxFrm ) const
{
    const SwTableBox* pBox = 0;
    SwTabFrm *pTab = 0;

    if( pBoxFrm )
    {
        pTab = ((SwFrm*)pBoxFrm)->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( pCrsr )
    {
        const SwCntntNode* pCNd = pCrsr->GetCntntNode();
        if( !pCNd )
            return ;

        Point aPt;
        const SwShellCrsr *pShCrsr = dynamic_cast<const SwShellCrsr*>(pCrsr);
        if( pShCrsr )
            aPt = pShCrsr->GetPtPos();

        const SwFrm* pTmpFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );
        do {
            pTmpFrm = pTmpFrm->GetUpper();
        } while ( !pTmpFrm->IsCellFrm() );

        pBoxFrm = (SwCellFrm*)pTmpFrm;
        pTab = ((SwFrm*)pBoxFrm)->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( !pCrsr && !pBoxFrm )
    {
        OSL_ENSURE( !this, "einer von beiden muss angegeben werden!" );
        return ;
    }

    //Fix-Punkte setzen, LeftMin in Dokumentkoordinaten die anderen relativ.
    SWRECTFN( pTab )
    const SwPageFrm* pPage = pTab->FindPageFrm();
    const sal_uLong nLeftMin = (pTab->Frm().*fnRect->fnGetLeft)() -
                           (pPage->Frm().*fnRect->fnGetLeft)();
    const sal_uLong nRightMax = (pTab->Frm().*fnRect->fnGetRight)() -
                            (pPage->Frm().*fnRect->fnGetLeft)();

    rFill.SetLeftMin ( nLeftMin );
    rFill.SetLeft    ( (pTab->Prt().*fnRect->fnGetLeft)() );
    rFill.SetRight   ( (pTab->Prt().*fnRect->fnGetRight)());
    rFill.SetRightMax( nRightMax - nLeftMin );

    pTab->GetTable()->GetTabCols( rFill, pBox );
}

//
// Here are some little helpers used in SwDoc::GetTabRows
//

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
    for( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
    {
        if ( rFrm.GetTabBox() == rBoxes[ i ] )
            return true;
    }

    return false;
}

//
// SwDoc::GetTabRows()
//

void SwDoc::GetTabRows( SwTabCols &rFill, const SwCursor* ,
                        const SwCellFrm* pBoxFrm ) const
{
    OSL_ENSURE( pBoxFrm, "GetTabRows called without pBoxFrm" );

    // Make code robust:
    if ( !pBoxFrm )
        return;

    // #i39552# Collection of the boxes of the current
    // column has to be done at the beginning of this function, because
    // the table may be formatted in ::GetTblSel.
    SwDeletionChecker aDelCheck( pBoxFrm );

    SwSelBoxes aBoxes;
    const SwCntntFrm* pCntnt = ::GetCellCntnt( *pBoxFrm );
    if ( pCntnt && pCntnt->IsTxtFrm() )
    {
        const SwPosition aPos( *((SwTxtFrm*)pCntnt)->GetTxtNode() );
        const SwCursor aTmpCrsr( aPos, 0, false );
        ::GetTblSel( aTmpCrsr, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
    }

    // Make code robust:
    if ( aDelCheck.HasBeenDeleted() )
    {
        OSL_FAIL( "Current box has been deleted during GetTabRows()" );
        return;
    }

    // Make code robust:
    const SwTabFrm* pTab = pBoxFrm->FindTabFrm();
    OSL_ENSURE( pTab, "GetTabRows called without a table" );
    if ( !pTab )
        return;

    const SwFrm* pFrm = pTab->GetNextLayoutLeaf();

    //Fix-Punkte setzen, LeftMin in Dokumentkoordinaten die anderen relativ.
    SWRECTFN( pTab )
    const SwPageFrm* pPage = pTab->FindPageFrm();
    const long nLeftMin  = ( bVert ?
                             pTab->GetPrtLeft() - pPage->Frm().Left() :
                             pTab->GetPrtTop() - pPage->Frm().Top() );
    const long nLeft     = bVert ? LONG_MAX : 0;
    const long nRight    = (pTab->Prt().*fnRect->fnGetHeight)();
    const long nRightMax = bVert ? nRight : LONG_MAX;

    rFill.SetLeftMin( nLeftMin );
    rFill.SetLeft( nLeft );
    rFill.SetRight( nRight );
    rFill.SetRightMax( nRightMax );

    typedef std::map< long, std::pair< long, long >, FuzzyCompare > BoundaryMap;
    BoundaryMap aBoundaries;
    BoundaryMap::iterator aIter;
    std::pair< long, long > aPair;

    typedef std::map< long, bool > HiddenMap;
    HiddenMap aHidden;
    HiddenMap::iterator aHiddenIter;

    while ( pFrm && pTab->IsAnLower( pFrm ) )
    {
        if ( pFrm->IsCellFrm() && pFrm->FindTabFrm() == pTab )
        {
            // upper and lower borders of current cell frame:
            long nUpperBorder = (pFrm->Frm().*fnRect->fnGetTop)();
            long nLowerBorder = (pFrm->Frm().*fnRect->fnGetBottom)();

            // get boundaries for nUpperBorder:
            aIter = aBoundaries.find( nUpperBorder );
            if ( aIter == aBoundaries.end() )
            {
                aPair.first = nUpperBorder; aPair.second = LONG_MAX;
                aBoundaries[ nUpperBorder ] = aPair;
            }

            // get boundaries for nLowerBorder:
            aIter = aBoundaries.find( nLowerBorder );
            if ( aIter == aBoundaries.end() )
            {
                aPair.first = nUpperBorder; aPair.second = LONG_MAX;
            }
            else
            {
                nLowerBorder = (*aIter).first;
                long nNewLowerBorderUpperBoundary = Max( (*aIter).second.first, nUpperBorder );
                aPair.first = nNewLowerBorderUpperBoundary; aPair.second = LONG_MAX;
            }
            aBoundaries[ nLowerBorder ] = aPair;

            // calculate hidden flags for entry nUpperBorder/nLowerBorder:
            long nTmpVal = nUpperBorder;
            for ( sal_uInt8 i = 0; i < 2; ++i )
            {
                aHiddenIter = aHidden.find( nTmpVal );
                if ( aHiddenIter == aHidden.end() )
                    aHidden[ nTmpVal ] = !lcl_IsFrmInColumn( *((SwCellFrm*)pFrm), aBoxes );
                else
                {
                    if ( aHidden[ nTmpVal ] &&
                         lcl_IsFrmInColumn( *((SwCellFrm*)pFrm), aBoxes ) )
                        aHidden[ nTmpVal ] = false;
                }
                nTmpVal = nLowerBorder;
            }
        }

        pFrm = pFrm->GetNextLayoutLeaf();
    }

    // transfer calculated values from BoundaryMap and HiddenMap into rFill:
    sal_uInt16 nIdx = 0;
    for ( aIter = aBoundaries.begin(); aIter != aBoundaries.end(); ++aIter )
    {
        const long nTabTop = (pTab->*fnRect->fnGetPrtTop)();
        const long nKey = (*fnRect->fnYDiff)( (*aIter).first, nTabTop );
        const std::pair< long, long > aTmpPair = (*aIter).second;
        const long nFirst = (*fnRect->fnYDiff)( aTmpPair.first, nTabTop );
        const long nSecond = aTmpPair.second;

        aHiddenIter = aHidden.find( (*aIter).first );
        const bool bHidden = aHiddenIter != aHidden.end() && (*aHiddenIter).second;
        rFill.Insert( nKey, nFirst, nSecond, bHidden, nIdx++ );
    }

    // delete first and last entry
    OSL_ENSURE( rFill.Count(), "Deleting from empty vector. Fasten your seatbelts!" );
    // #i60818# There may be only one entry in rFill. Make
    // code robust by checking count of rFill.
    if ( rFill.Count() ) rFill.Remove( 0, 1 );
    if ( rFill.Count() ) rFill.Remove( rFill.Count() - 1 , 1 );
    rFill.SetLastRowAllowedToChange( !pTab->HasFollowFlowLine() );
}

void SwDoc::SetTabCols( const SwTabCols &rNew, sal_Bool bCurRowOnly,
                        const SwCursor* pCrsr, const SwCellFrm* pBoxFrm )
{
    const SwTableBox* pBox = 0;
    SwTabFrm *pTab = 0;

    if( pBoxFrm )
    {
        pTab = ((SwFrm*)pBoxFrm)->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( pCrsr )
    {
        const SwCntntNode* pCNd = pCrsr->GetCntntNode();
        if( !pCNd )
            return ;

        Point aPt;
        const SwShellCrsr *pShCrsr = dynamic_cast<const SwShellCrsr*>(pCrsr);
        if( pShCrsr )
            aPt = pShCrsr->GetPtPos();

        const SwFrm* pTmpFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );
        do {
            pTmpFrm = pTmpFrm->GetUpper();
        } while ( !pTmpFrm->IsCellFrm() );

        pBoxFrm = (SwCellFrm*)pTmpFrm;
        pTab = ((SwFrm*)pBoxFrm)->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( !pCrsr && !pBoxFrm )
    {
        OSL_ENSURE( !this, "einer von beiden muss angegeben werden!" );
        return ;
    }

    // sollte die Tabelle noch auf relativen Werten (USHRT_MAX) stehen
    // dann muss es jetzt auf absolute umgerechnet werden.
    SwTable& rTab = *pTab->GetTable();
    const SwFmtFrmSize& rTblFrmSz = rTab.GetFrmFmt()->GetFrmSize();
    SWRECTFN( pTab )
    // #i17174# - With fix for #i9040# the shadow size is taken
    // from the table width. Thus, add its left and right size to current table
    // printing area width in order to get the correct table size attribute.
    SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
    {
        SvxShadowItem aShadow( rTab.GetFrmFmt()->GetShadow() );
        nPrtWidth += aShadow.CalcShadowSpace( SHADOW_LEFT ) +
                     aShadow.CalcShadowSpace( SHADOW_RIGHT );
    }
    if( nPrtWidth != rTblFrmSz.GetWidth() )
    {
        SwFmtFrmSize aSz( rTblFrmSz );
        aSz.SetWidth( nPrtWidth );
        rTab.GetFrmFmt()->SetFmtAttr( aSz );
    }

    SwTabCols aOld( rNew.Count() );

    const SwPageFrm* pPage = pTab->FindPageFrm();
    const sal_uLong nLeftMin = (pTab->Frm().*fnRect->fnGetLeft)() -
                           (pPage->Frm().*fnRect->fnGetLeft)();
    const sal_uLong nRightMax = (pTab->Frm().*fnRect->fnGetRight)() -
                            (pPage->Frm().*fnRect->fnGetLeft)();

    //Fix-Punkte setzen, LeftMin in Dokumentkoordinaten die anderen relativ.
    aOld.SetLeftMin ( nLeftMin );
    aOld.SetLeft    ( (pTab->Prt().*fnRect->fnGetLeft)() );
    aOld.SetRight   ( (pTab->Prt().*fnRect->fnGetRight)());
    aOld.SetRightMax( nRightMax - nLeftMin );

    rTab.GetTabCols( aOld, pBox );
    SetTabCols(rTab, rNew, aOld, pBox, bCurRowOnly );
}

void SwDoc::SetTabRows( const SwTabCols &rNew, sal_Bool bCurColOnly, const SwCursor*,
                        const SwCellFrm* pBoxFrm )
{
    const SwTableBox* pBox;
    SwTabFrm *pTab;

    OSL_ENSURE( pBoxFrm, "SetTabRows called without pBoxFrm" );

    pTab = ((SwFrm*)pBoxFrm)->ImplFindTabFrm();
    pBox = pBoxFrm->GetTabBox();

    // sollte die Tabelle noch auf relativen Werten (USHRT_MAX) stehen
    // dann muss es jetzt auf absolute umgerechnet werden.
    SWRECTFN( pTab )
    SwTabCols aOld( rNew.Count() );

    //Fix-Punkte setzen, LeftMin in Dokumentkoordinaten die anderen relativ.
    const SwPageFrm* pPage = pTab->FindPageFrm();

    aOld.SetRight( (pTab->Prt().*fnRect->fnGetHeight)() );
    long nLeftMin;
    if ( bVert )
    {
        nLeftMin = pTab->GetPrtLeft() - pPage->Frm().Left();
        aOld.SetLeft    ( LONG_MAX );
        aOld.SetRightMax( aOld.GetRight() );

    }
    else
    {
        nLeftMin = pTab->GetPrtTop() - pPage->Frm().Top();
        aOld.SetLeft    ( 0 );
        aOld.SetRightMax( LONG_MAX );
    }
    aOld.SetLeftMin ( nLeftMin );

    GetTabRows( aOld, 0, pBoxFrm );

    GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_ATTR, NULL );

    // check for differences between aOld and rNew:
    const sal_uInt16 nCount = rNew.Count();
    const SwTable* pTable = pTab->GetTable();
    OSL_ENSURE( pTable, "My colleague told me, this couldn't happen" );

    for ( sal_uInt16 i = 0; i <= nCount; ++i )
    {
        const sal_uInt16 nIdxStt = bVert ? nCount - i : i - 1;
        const sal_uInt16 nIdxEnd = bVert ? nCount - i - 1 : i;

        const long nOldRowStart = i == 0  ? 0 : aOld[ nIdxStt ];
        const long nOldRowEnd =   i == nCount ? aOld.GetRight() : aOld[ nIdxEnd ];
        const long nOldRowHeight = nOldRowEnd - nOldRowStart;

        const long nNewRowStart = i == 0  ? 0 : rNew[ nIdxStt ];
        const long nNewRowEnd =   i == nCount ? rNew.GetRight() : rNew[ nIdxEnd ];
        const long nNewRowHeight = nNewRowEnd - nNewRowStart;

        const long nDiff = nNewRowHeight - nOldRowHeight;
        if ( abs( nDiff ) >= ROWFUZZY )
        {
            // For the old table model pTxtFrm and pLine will be set for every box.
            // For the new table model pTxtFrm will be set if the box is not covered,
            // but the pLine will be set if the box is not an overlapping box
            // In the new table model the row height can be adjusted,
            // when both variables are set.
            SwTxtFrm* pTxtFrm = 0;
            const SwTableLine* pLine = 0;

            // Iterate over all SwCellFrms with Bottom = nOldPos
            const SwFrm* pFrm = pTab->GetNextLayoutLeaf();
            while ( pFrm && pTab->IsAnLower( pFrm ) )
            {
                if ( pFrm->IsCellFrm() && pFrm->FindTabFrm() == pTab )
                {
                    const long nLowerBorder = (pFrm->Frm().*fnRect->fnGetBottom)();
                    const sal_uLong nTabTop = (pTab->*fnRect->fnGetPrtTop)();
                    if ( abs( (*fnRect->fnYInc)( nTabTop, nOldRowEnd ) - nLowerBorder ) <= ROWFUZZY )
                    {
                        if ( !bCurColOnly || pFrm == pBoxFrm )
                        {
                            const SwFrm* pCntnt = ::GetCellCntnt( static_cast<const SwCellFrm&>(*pFrm) );

                            if ( pCntnt && pCntnt->IsTxtFrm() )
                            {
                                pBox = ((SwCellFrm*)pFrm)->GetTabBox();
                                const long nRowSpan = pBox->getRowSpan();
                                if( nRowSpan > 0 ) // Not overlapped
                                    pTxtFrm = (SwTxtFrm*)pCntnt;
                                if( nRowSpan < 2 ) // Not overlapping for row height
                                    pLine = pBox->GetUpper();
                                if( pLine && pTxtFrm ) // always for old table model
                                {
                                    // The new row height must not to be calculated from a overlapping box
                                    SwFmtFrmSize aNew( pLine->GetFrmFmt()->GetFrmSize() );
                                    const long nNewSize = (pFrm->Frm().*fnRect->fnGetHeight)() + nDiff;
                                    if( nNewSize != aNew.GetHeight() )
                                    {
                                        aNew.SetHeight( nNewSize );
                                        if ( ATT_VAR_SIZE == aNew.GetHeightSizeType() )
                                            aNew.SetHeightSizeType( ATT_MIN_SIZE );
                                        // This position must not be in an overlapped box
                                        const SwPosition aPos( *((SwTxtFrm*)pCntnt)->GetTxtNode() );
                                        const SwCursor aTmpCrsr( aPos, 0, false );
                                        SetRowHeight( aTmpCrsr, aNew );
                                        // For the new table model we're done, for the old one
                                        // there might be another (sub)row to adjust...
                                        if( pTable->IsNewModel() )
                                            break;
                                    }
                                    pLine = 0;
                                }
                            }
                        }
                    }
                }
                pFrm = pFrm->GetNextLayoutLeaf();
            }
        }
    }

    GetIDocumentUndoRedo().EndUndo( UNDO_TABLE_ATTR, NULL );

    ::ClearFEShellTabCols();
}

/* --------------------------------------------------
 *  Direktzugriff fuer UNO
 * --------------------------------------------------*/
void SwDoc::SetTabCols(SwTable& rTab, const SwTabCols &rNew, const SwTabCols &rOld,
                                const SwTableBox *pStart, sal_Bool bCurRowOnly )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoAttrTbl( *rTab.GetTableNode(), sal_True ));
    }
    rTab.SetTabCols( rNew, rOld, pStart, bCurRowOnly );
      ::ClearFEShellTabCols();
    SetModified();
}

void SwDoc::SetRowsToRepeat( SwTable &rTable, sal_uInt16 nSet )
{
    if( nSet == rTable.GetRowsToRepeat() )
        return;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoTblHeadline(rTable, rTable.GetRowsToRepeat(), nSet) );
    }

    SwMsgPoolItem aChg( RES_TBLHEADLINECHG );
    rTable.SetRowsToRepeat( nSet );
    rTable.GetFrmFmt()->ModifyNotification( &aChg, &aChg );
    SetModified();
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
    SwTableBox* p = (SwTableBox*)&rBox;
    m_Boxes.push_back(p);
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
        pRet = m_Boxes[ n ];
    }
    return pRet;
}

sal_Bool SwCollectTblLineBoxes::Resize( sal_uInt16 nOffset, sal_uInt16 nOldWidth )
{
    sal_uInt16 n;

    if( !aPosArr.empty() )
    {
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

        aPosArr.erase( aPosArr.begin(), aPosArr.begin() + n );
        m_Boxes.erase(m_Boxes.begin(), m_Boxes.begin() + n);

        // dann die Positionen der neuen Size anpassen
        for( n = 0; n < aPosArr.size(); ++n )
        {
            sal_uLong nSize = nWidth;
            nSize *= ( aPosArr[ n ] - nOffset );
            nSize /= nOldWidth;
            aPosArr[ n ] = sal_uInt16( nSize );
        }
    }
    return !aPosArr.empty();
}

sal_Bool lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    if( pSplPara->IsGetValues() )
        for( SwTableBoxes::iterator it = ((SwTableLine*)rpLine)->GetTabBoxes().begin();
                 it != ((SwTableLine*)rpLine)->GetTabBoxes().end(); ++it)
            lcl_Box_CollectBox(*it, pSplPara );
    else
        for( SwTableBoxes::iterator it = ((SwTableLine*)rpLine)->GetTabBoxes().begin();
                 it != ((SwTableLine*)rpLine)->GetTabBoxes().end(); ++it)
            lcl_BoxSetSplitBoxFmts(*it, pSplPara );
    return sal_True;
}

void lcl_Box_CollectBox( const SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara )
{
    sal_uInt16 nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = pBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pSplPara );
    }
    else
        pSplPara->AddBox( *pBox );
}

void lcl_BoxSetSplitBoxFmts( SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara )
{
    sal_uInt16 nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        // dann mit der richtigen Line weitermachen
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = pBox->GetTabLines()[ nLen ];
        lcl_Line_CollectBox( pLn, pSplPara );
    }
    else
    {
        const SwTableBox* pSrcBox = pSplPara->GetBoxOfPos( *pBox );
        SwFrmFmt* pFmt = pSrcBox->GetFrmFmt();

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
}


sal_Bool SwDoc::SplitTable( const SwPosition& rPos, sal_uInt16 eHdlnMode,
                        sal_Bool bCalcNewSize )
{
    SwNode* pNd = &rPos.nNode.GetNode();
    SwTableNode* pTNd = pNd->FindTableNode();
    if( !pTNd || pNd->IsTableNode() )
        return 0;

    if( pTNd->GetTable().ISA( SwDDETable ))
        return sal_False;

    SwTable& rTbl = pTNd->GetTable();
    rTbl.SetHTMLTableLayout( 0 );   // MIB 9.7.97: HTML-Layout loeschen

    SwTableFmlUpdate aMsgHnt( &rTbl );

    SwHistory aHistory;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        aMsgHnt.pHistory = &aHistory;
    }

    {
        sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

        // Suche die Grund-Line dieser Box:
        SwTableBox* pBox = rTbl.GetTblBox( nSttIdx );
        if( pBox )
        {
            SwTableLine* pLine = pBox->GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            // in pLine steht jetzt die GrundLine.
            aMsgHnt.nSplitLine = rTbl.GetTabLines().GetPos( pLine );
        }

        String sNewTblNm( GetUniqueTblName() );
        aMsgHnt.DATA.pNewTblNm = &sNewTblNm;
        aMsgHnt.eFlags = TBL_SPLITTBL;
        UpdateTblFlds( &aMsgHnt );
    }

    //Lines fuer das Layout-Update heraussuchen.
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rTbl );
    aFndBox.DelFrms( rTbl );

    SwTableNode* pNew = GetNodes().SplitTable( rPos.nNode, sal_False, bCalcNewSize );

    if( pNew )
    {
        SwSaveRowSpan* pSaveRowSp = pNew->GetTable().CleanUpTopRowSpan( rTbl.GetTabLines().size() );
        SwUndoSplitTbl* pUndo = 0;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo = new SwUndoSplitTbl(
                        *pNew, pSaveRowSp, eHdlnMode, bCalcNewSize);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
            if( aHistory.Count() )
                pUndo->SaveFormula( aHistory );
        }

        switch( eHdlnMode )
        {
            // setze die untere Border der vorherige Line,
            // an der aktuellen als obere
        case HEADLINE_BORDERCOPY:
            {
                SwCollectTblLineBoxes aPara( sal_False, eHdlnMode );
                SwTableLine* pLn = rTbl.GetTabLines()[
                            rTbl.GetTabLines().size() - 1 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    lcl_Box_CollectBox(*it, &aPara );

                aPara.SetValues( sal_True );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    lcl_BoxSetSplitBoxFmts(*it, &aPara );

                // Kopfzeile wiederholen abschalten
                pNew->GetTable().SetRowsToRepeat( 0 );
            }
            break;

            // setze die Attributierung der ersten Line an der neuen ersten
        case HEADLINE_BOXATTRCOPY:
        case HEADLINE_BOXATRCOLLCOPY:
            {
                SwHistory* pHst = 0;
                if( HEADLINE_BOXATRCOLLCOPY == eHdlnMode && pUndo )
                    pHst = pUndo->GetHistory();

                SwCollectTblLineBoxes aPara( sal_True, eHdlnMode, pHst );
                SwTableLine* pLn = rTbl.GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    lcl_Box_CollectBox(*it, &aPara );

                aPara.SetValues( sal_True );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    lcl_BoxSetSplitBoxFmts(*it, &aPara );
            }
            break;

        case HEADLINE_CNTNTCOPY:
            rTbl.CopyHeadlineIntoTable( *pNew );
            if( pUndo )
                pUndo->SetTblNodeOffset( pNew->GetIndex() );
            break;

        case HEADLINE_NONE:
            // Kopfzeile wiederholen abschalten
            pNew->GetTable().SetRowsToRepeat( 0 );
            break;
        }

        // und Frms einfuegen.
        SwNodeIndex aNdIdx( *pNew->EndOfSectionNode() );
        GetNodes().GoNext( &aNdIdx );      // zum naechsten ContentNode
        pNew->MakeFrms( &aNdIdx );

        //Zwischen die Tabellen wird ein Absatz geschoben
        GetNodes().MakeTxtNode( SwNodeIndex( *pNew ),
                                GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );
    }

    //Layout updaten
    aFndBox.MakeFrms( rTbl );

    // TL_CHART2: need to inform chart of probably changed cell names
    UpdateCharts( rTbl.GetFrmFmt()->GetName() );

    SetFieldsDirty( true, NULL, 0 );

    return 0 != pNew;
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
    for( sal_uInt16 nLns = 0; nLns < rLns.size(); ++nLns )
    {
        SwTwips nMaxLnWidth = 0;
        SwTableBoxes& rBoxes = rLns[ nLns ]->GetTabBoxes();
        for( sal_uInt16 nBox = 0; nBox < rBoxes.size(); ++nBox )
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
    std::map<SwFrmFmt*, SwFrmFmt*> aSrcDestMap;
    SwTableNode* pNewTblNd;
    SwTable& rOldTbl;

public:
    _SplitTable_Para( SwTableNode* pNew, SwTable& rOld )
        : aSrcDestMap(), pNewTblNd( pNew ), rOldTbl( rOld )
    {}
    SwFrmFmt* GetDestFmt( SwFrmFmt* pSrcFmt ) const
    {
        std::map<SwFrmFmt*, SwFrmFmt*>::const_iterator it = aSrcDestMap.find( pSrcFmt );
        return it == aSrcDestMap.end() ? NULL : it->second;
    }

    void InsertSrcDest( SwFrmFmt* pSrcFmt, SwFrmFmt* pDestFmt )
            { aSrcDestMap[ pSrcFmt ] = pDestFmt; }

    void ChgBox( SwTableBox* pBox )
    {
        rOldTbl.GetTabSortBoxes().erase( pBox );
        pNewTblNd->GetTable().GetTabSortBoxes().insert( pBox );
    }
};


static void lcl_SplitTable_CpyBox( SwTableBox* pBox, _SplitTable_Para* pPara );

static void lcl_SplitTable_CpyLine( SwTableLine* pLn, _SplitTable_Para* pPara )
{
    SwFrmFmt *pSrcFmt = pLn->GetFrmFmt();
    SwTableLineFmt* pDestFmt = (SwTableLineFmt*) pPara->GetDestFmt( pSrcFmt );
    if( pDestFmt == NULL )
    {
        pPara->InsertSrcDest( pSrcFmt, pLn->ClaimFrmFmt() );
    }
    else
        pLn->ChgFrmFmt( pDestFmt );

    for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
             it != pLn->GetTabBoxes().end(); ++it)
        lcl_SplitTable_CpyBox(*it, pPara );
}

static void lcl_SplitTable_CpyBox( SwTableBox* pBox, _SplitTable_Para* pPara )
{
    SwFrmFmt *pSrcFmt = pBox->GetFrmFmt();
    SwTableBoxFmt* pDestFmt = (SwTableBoxFmt*)pPara->GetDestFmt( pSrcFmt );
    if( pDestFmt == NULL )
    {
        pPara->InsertSrcDest( pSrcFmt, pBox->ClaimFrmFmt() );
    }
    else
        pBox->ChgFrmFmt( pDestFmt );

    if( pBox->GetSttNd() )
        pPara->ChgBox( pBox );
    else
        BOOST_FOREACH( SwTableLine* pLine, pBox->GetTabLines() )
            lcl_SplitTable_CpyLine( pLine, pPara );
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
    sal_uInt16 nLinePos = rTbl.GetTabLines().GetPos( pLine );
    if( USHRT_MAX == nLinePos ||
        ( bAfter ? ++nLinePos >= rTbl.GetTabLines().size() : !nLinePos ))
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
        rNewTbl.GetTabLines().insert( rNewTbl.GetTabLines().begin(),
                      rTbl.GetTabLines().begin() + nLinePos, rTbl.GetTabLines().end() );
        //
        // von hinten (unten-rechts) nach vorn (oben-links) alle Boxen
        // beim chart data provider austragen (das modified event wird dann
        // in der aufrufenden Funktion getriggert.
        // TL_CHART2:
        SwChartDataProvider *pPCD = rTbl.GetFrmFmt()->getIDocumentChartDataProviderAccess()->GetChartDataProvider();
        if( pPCD )
        {
            for (sal_uInt16 k = nLinePos;  k < rTbl.GetTabLines().size();  ++k)
            {
                sal_uInt16 nLineIdx = (rTbl.GetTabLines().size() - 1) - k + nLinePos;
                sal_uInt16 nBoxCnt = rTbl.GetTabLines()[ nLineIdx ]->GetTabBoxes().size();
                for (sal_uInt16 j = 0;  j < nBoxCnt;  ++j)
                {
                    sal_uInt16 nIdx = nBoxCnt - 1 - j;
                    pPCD->DeleteBox( &rTbl, *rTbl.GetTabLines()[ nLineIdx ]->GetTabBoxes()[nIdx] );
                }
            }
        }
        //
        // ...und loeschen
        sal_uInt16 nDeleted = rTbl.GetTabLines().size() - nLinePos;
        rTbl.GetTabLines().erase( rTbl.GetTabLines().begin() + nLinePos, rTbl.GetTabLines().end() );

        // und die betr. Boxen verschieben. Dabei die Formate eindeutig
        // machen und die StartNodes korrigieren
        _SplitTable_Para aPara( pNewTblNd, rTbl );
        BOOST_FOREACH( SwTableLine* pNewLine, rNewTbl.GetTabLines() )
            lcl_SplitTable_CpyLine( pNewLine, &aPara );
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
sal_Bool SwDoc::MergeTable( const SwPosition& rPos, sal_Bool bWithPrev, sal_uInt16 nMode )
{
    SwTableNode* pTblNd = rPos.nNode.GetNode().FindTableNode(), *pDelTblNd;
    if( !pTblNd )
        return sal_False;

    SwNodes& rNds = GetNodes();
    if( bWithPrev )
        pDelTblNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
    else
        pDelTblNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();
    if( !pDelTblNd )
        return sal_False;

    if( pTblNd->GetTable().ISA( SwDDETable ) ||
        pDelTblNd->GetTable().ISA( SwDDETable ))
        return sal_False;

    // MIB 9.7.97: HTML-Layout loeschen
    pTblNd->GetTable().SetHTMLTableLayout( 0 );
    pDelTblNd->GetTable().SetHTMLTableLayout( 0 );

    // beide Tabellen vorhanden, also kanns losgehen
    SwUndoMergeTbl* pUndo = 0;
    SwHistory* pHistory = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMergeTbl( *pTblNd, *pDelTblNd, bWithPrev, nMode );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        pHistory = new SwHistory;
    }

    // alle "Tabellenformeln" anpassen
    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.DATA.pDelTbl = &pDelTblNd->GetTable();
    aMsgHnt.eFlags = TBL_MERGETBL;
    aMsgHnt.pHistory = pHistory;
    UpdateTblFlds( &aMsgHnt );

    // das eigentliche Mergen
    SwNodeIndex aIdx( bWithPrev ? *pTblNd : *pDelTblNd );
    sal_Bool bRet = rNds.MergeTable( aIdx, !bWithPrev, nMode, pHistory );

    if( pHistory )
    {
        if( pHistory->Count() )
            pUndo->SaveFormula( *pHistory );
        delete pHistory;
    }
    if( bRet )
    {
        SetModified();
        SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}

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
    sal_uInt16 nOldSize = rTbl.GetTabLines().size();
    rTbl.GetTabLines().insert( rTbl.GetTabLines().begin() + nOldSize,
                               rDelTbl.GetTabLines().begin(), rDelTbl.GetTabLines().end() );
    rDelTbl.GetTabLines().clear();

    rTbl.GetTabSortBoxes().insert( rDelTbl.GetTabSortBoxes() );
    rDelTbl.GetTabSortBoxes().clear();

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
    lcl_LineSetHeadCondColl( pFirstLn );

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
sal_Bool lcl_SetAFmtBox(_FndBox &, _SetAFmtTabPara *pSetPara);
sal_Bool lcl_SetAFmtLine(_FndLine &, _SetAFmtTabPara *pPara);

sal_Bool lcl_SetAFmtLine(_FndLine & rLine, _SetAFmtTabPara *pPara)
{
    for (_FndBoxes::iterator it = rLine.GetBoxes().begin();
         it != rLine.GetBoxes().end(); ++it)
    {
        lcl_SetAFmtBox(*it, pPara);
    }
    return sal_True;
}

sal_Bool lcl_SetAFmtBox( _FndBox & rBox, _SetAFmtTabPara *pSetPara )
{
    if (!rBox.GetUpper()->GetUpper())     // Box on first level?
    {
        if( !pSetPara->nCurBox )
            pSetPara->nAFmtBox = 0;
        else if( pSetPara->nCurBox == pSetPara->nEndBox )
            pSetPara->nAFmtBox = 3;
        else
            pSetPara->nAFmtBox = (sal_uInt8)(1 + ((pSetPara->nCurBox-1) & 1));
    }

    if (rBox.GetBox()->GetSttNd())
    {
        SwTableBox* pSetBox = static_cast<SwTableBox*>(rBox.GetBox());
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
        BOOST_FOREACH( _FndLine& rFndLine, rBox.GetLines() )
            lcl_SetAFmtLine( rFndLine, pSetPara );

    if (!rBox.GetUpper()->GetUpper())     // a BaseLine
        ++pSetPara->nCurBox;
    return sal_True;
}


        // AutoFormat fuer die Tabelle/TabellenSelection
sal_Bool SwDoc::SetTableAutoFmt( const SwSelBoxes& rBoxes, const SwTableAutoFmt& rNew )
{
    OSL_ENSURE( !rBoxes.empty(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return sal_False;

    SwTable &table = pTblNd->GetTable();
    table.SetHTMLTableLayout( 0 );

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front().GetBoxes().size() )
    {
        pFndBox = &pFndBox->GetLines().front().GetBoxes()[0];
    }

    if( pFndBox->GetLines().empty() )      // eine zu weit? (nur 1 sel.Box)
        pFndBox = pFndBox->GetUpper()->GetUpper();


    // Undo abschalten, Attribute werden sich vorher gemerkt
    SwUndoTblAutoFmt* pUndo = 0;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndo = new SwUndoTblAutoFmt( *pTblNd, rNew );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        GetIDocumentUndoRedo().DoUndo(false);
    }

    rNew.RestoreTableProperties(table);

    _SetAFmtTabPara aPara( rNew );
    _FndLines& rFLns = pFndBox->GetLines();
    _FndLine* pLine;

    for( sal_uInt16 n = 0; n < rFLns.size(); ++n )
    {
        pLine = &rFLns[n];

        // Upper auf 0 setzen (Base-Line simulieren!)
        _FndBox* pSaveBox = pLine->GetUpper();
        pLine->SetUpper( 0 );

        if( !n )
            aPara.nAFmtLine = 0;
        else if (static_cast<size_t>(n+1) == rFLns.size())
            aPara.nAFmtLine = 3;
        else
            aPara.nAFmtLine = (sal_uInt8)(1 + ((n-1) & 1 ));

        aPara.nAFmtBox = 0;
        aPara.nCurBox = 0;
        aPara.nEndBox = pLine->GetBoxes().size()-1;
        aPara.pUndo = pUndo;
        for (_FndBoxes::iterator it = pLine->GetBoxes().begin();
             it != pLine->GetBoxes().end(); ++it)
        {
            lcl_SetAFmtBox(*it, &aPara);
        }

        pLine->SetUpper( pSaveBox );
    }

    if( pUndo )
    {
        GetIDocumentUndoRedo().DoUndo(bUndo);
    }

    SetModified();
    SetFieldsDirty( true, NULL, 0 );

    return sal_True;
}


        // Erfrage wie attributiert ist
sal_Bool SwDoc::GetTableAutoFmt( const SwSelBoxes& rBoxes, SwTableAutoFmt& rGet )
{
    OSL_ENSURE( !rBoxes.empty(), "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return sal_False;

    // Store table properties
    SwTable &table = pTblNd->GetTable();
    rGet.StoreTableProperties(table);

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front().GetBoxes().size() )
    {
        pFndBox = &pFndBox->GetLines().front().GetBoxes()[0];
    }

    if( pFndBox->GetLines().empty() )      // eine zu weit? (nur 1 sel.Box)
        pFndBox = pFndBox->GetUpper()->GetUpper();

    _FndLines& rFLns = pFndBox->GetLines();

    sal_uInt16 aLnArr[4];
    aLnArr[0] = 0;
    aLnArr[1] = 1 < rFLns.size() ? 1 : 0;
    aLnArr[2] = 2 < rFLns.size() ? 2 : aLnArr[1];
    aLnArr[3] = rFLns.size() - 1;

    for( sal_uInt8 nLine = 0; nLine < 4; ++nLine )
    {
        _FndLine& rLine = rFLns[ aLnArr[ nLine ] ];

        sal_uInt16 aBoxArr[4];
        aBoxArr[0] = 0;
        aBoxArr[1] = 1 < rLine.GetBoxes().size() ? 1 : 0;
        aBoxArr[2] = 2 < rLine.GetBoxes().size() ? 2 : aBoxArr[1];
        aBoxArr[3] = rLine.GetBoxes().size() - 1;

        for( sal_uInt8 nBox = 0; nBox < 4; ++nBox )
        {
            SwTableBox* pFBox = rLine.GetBoxes()[ aBoxArr[ nBox ] ].GetBox();
            // immer auf die 1. runterfallen
            while( !pFBox->GetSttNd() )
                pFBox = pFBox->GetTabLines()[0]->GetTabBoxes()[0];

            sal_uInt8 nPos = nLine * 4 + nBox;
            SwNodeIndex aIdx( *pFBox->GetSttNd(), 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetNodes().GoNext( &aIdx );

            if( pCNd )
                rGet.UpdateFromSet( nPos, pCNd->GetSwAttrSet(),
                                    SwTableAutoFmt::UPDATE_CHAR, 0 );
            rGet.UpdateFromSet( nPos, pFBox->GetFrmFmt()->GetAttrSet(),
                                SwTableAutoFmt::UPDATE_BOX,
                                GetNumberFormatter( sal_True ) );
        }
    }

    return sal_True;
}

String SwDoc::GetUniqueTblName() const
{
    ResId aId( STR_TABLE_DEFNAME, *pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    sal_uInt16 nNum, nTmp, nFlagSize = ( pTblFrmFmtTbl->size() / 8 ) +2;
    sal_uInt16 n;

    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( n = 0; n < pTblFrmFmtTbl->size(); ++n )
    {
        const SwFrmFmt* pFmt = (*pTblFrmFmtTbl)[ n ];
        if( !pFmt->IsDefault() && IsUsed( *pFmt )  &&
            pFmt->GetName().Match( aName ) == nNmLen )
        {
            // Nummer bestimmen und das Flag setzen
            nNum = static_cast<sal_uInt16>(pFmt->GetName().Copy( nNmLen ).ToInt32());
            if( nNum-- && nNum < pTblFrmFmtTbl->size() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
    nNum = pTblFrmFmtTbl->size();
    for( n = 0; n < nFlagSize; ++n )
        if( 0xff != ( nTmp = pSetFlags[ n ] ))
        {
            // also die Nummer bestimmen
            nNum = n * 8;
            while( nTmp & 1 )
                ++nNum, nTmp >>= 1;
            break;
        }

    delete [] pSetFlags;
    return aName += String::CreateFromInt32( ++nNum );
}

SwTableFmt* SwDoc::FindTblFmtByName( const String& rName, sal_Bool bAll ) const
{
    const SwFmt* pRet = 0;
    if( bAll )
        pRet = FindFmtByName( *pTblFrmFmtTbl, rName );
    else
    {
        // dann nur die, die im Doc gesetzt sind
        for( sal_uInt16 n = 0; n < pTblFrmFmtTbl->size(); ++n )
        {
            const SwFrmFmt* pFmt = (*pTblFrmFmtTbl)[ n ];
            if( !pFmt->IsDefault() && IsUsed( *pFmt ) &&
                pFmt->GetName() == rName )
            {
                pRet = pFmt;
                break;
            }
        }
    }
    return (SwTableFmt*)pRet;
}

sal_Bool SwDoc::SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                    SwTwips nAbsDiff, SwTwips nRelDiff )
{
    SwTableNode* pTblNd = (SwTableNode*)rAktBox.GetSttNd()->FindTableNode();
    SwUndo* pUndo = 0;

    if( nsTblChgWidthHeightType::WH_FLAG_INSDEL & eType && pTblNd->GetTable().ISA( SwDDETable ))
        return sal_False;

    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_BOXPTR;
    UpdateTblFlds( &aMsgHnt );

    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    sal_Bool bRet = sal_False;
    switch( eType & 0xff )
    {
    case nsTblChgWidthHeightType::WH_COL_LEFT:
    case nsTblChgWidthHeightType::WH_COL_RIGHT:
    case nsTblChgWidthHeightType::WH_CELL_LEFT:
    case nsTblChgWidthHeightType::WH_CELL_RIGHT:
        {
             bRet = pTblNd->GetTable().SetColWidth( rAktBox,
                                eType, nAbsDiff, nRelDiff,
                                (bUndo) ? &pUndo : 0 );
        }
        break;
    case nsTblChgWidthHeightType::WH_ROW_TOP:
    case nsTblChgWidthHeightType::WH_ROW_BOTTOM:
    case nsTblChgWidthHeightType::WH_CELL_TOP:
    case nsTblChgWidthHeightType::WH_CELL_BOTTOM:
        bRet = pTblNd->GetTable().SetRowHeight( rAktBox,
                            eType, nAbsDiff, nRelDiff,
                            (bUndo) ? &pUndo : 0 );
        break;
    }

    GetIDocumentUndoRedo().DoUndo(bUndo); // SetColWidth can turn it off
    if( pUndo )
    {
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    if( bRet )
    {
        SetModified();
        if( nsTblChgWidthHeightType::WH_FLAG_INSDEL & eType )
            SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}


void SwDoc::ChkBoxNumFmt( SwTableBox& rBox, sal_Bool bCallUpdate )
{
    //JP 09.07.97: Optimierung: wenn die Box schon sagt, das es Text
    //                          sein soll, dann bleibt das auch Text!
    const SfxPoolItem* pNumFmtItem = 0;
    if( SFX_ITEM_SET == rBox.GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT,
        sal_False, &pNumFmtItem ) && GetNumberFormatter()->IsTextFormat(
            ((SwTblBoxNumFormat*)pNumFmtItem)->GetValue() ))
        return ;

    SwUndoTblNumFmt* pUndo = 0;

    sal_Bool bIsEmptyTxtNd, bChgd = sal_True;
    sal_uInt32 nFmtIdx;
    double fNumber;
    if( rBox.HasNumCntnt( fNumber, nFmtIdx, bIsEmptyTxtNd ) )
    {
        if( !rBox.IsNumberChanged() )
            bChgd = sal_False;
        else
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_AUTOFMT, NULL );
                pUndo = new SwUndoTblNumFmt( rBox );
                pUndo->SetNumFmt( nFmtIdx, fNumber );
            }

            SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rBox.GetFrmFmt();
            SfxItemSet aBoxSet( GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

            sal_Bool bSetNumFmt = IsInsTblFormatNum(), bLockModify = sal_True;
            if( bSetNumFmt )
            {
                if( !IsInsTblChangeNumFormat() )
                {
                    if( !pNumFmtItem )
                        bSetNumFmt = sal_False;
                    else
                    {
                        sal_uLong nOldNumFmt = ((SwTblBoxNumFormat*)pNumFmtItem)->
                                            GetValue();
                        SvNumberFormatter* pNumFmtr = GetNumberFormatter();

                        short nFmtType = pNumFmtr->GetType( nFmtIdx );
                        if( nFmtType == pNumFmtr->GetType( nOldNumFmt ) ||
                            NUMBERFORMAT_NUMBER == nFmtType )
                            // eingstelltes und vorgegebenes NumFormat
                            // stimmen ueberein -> altes Format beibehalten
                            nFmtIdx = nOldNumFmt;
                        else
                            // eingstelltes und vorgegebenes NumFormat
                            // stimmen nicht ueberein -> als Text einfuegen
                            bLockModify = bSetNumFmt = sal_False;
                    }
                }

                if( bSetNumFmt )
                {
                    pBoxFmt = (SwTableBoxFmt*)rBox.ClaimFrmFmt();

                    aBoxSet.Put( SwTblBoxValue( fNumber ));
                    aBoxSet.Put( SwTblBoxNumFormat( nFmtIdx ));
                }
            }

            // JP 28.04.98: Nur Formel zuruecksetzen reicht nicht.
            //              Sorge dafuer, das der Text auch entsprechend
            //              formatiert wird!

            if( !bSetNumFmt && !bIsEmptyTxtNd && pNumFmtItem )
            {
                // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
                //              Sorge dafuer, das der Text auch entsprechend
                //              formatiert wird!
                pBoxFmt->SetFmtAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));
            }

            if( bLockModify ) pBoxFmt->LockModify();
            pBoxFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            if( bLockModify ) pBoxFmt->UnlockModify();

            if( bSetNumFmt )
                pBoxFmt->SetFmtAttr( aBoxSet );
        }
    }
    else
    {
        // es ist keine Zahl
        const SfxPoolItem* pValueItem = 0, *pFmtItem = 0;
        SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rBox.GetFrmFmt();
        if( SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_FORMAT,
                sal_False, &pFmtItem ) ||
            SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_VALUE,
                sal_False, &pValueItem ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_AUTOFMT, NULL );
                pUndo = new SwUndoTblNumFmt( rBox );
            }

            pBoxFmt = (SwTableBoxFmt*)rBox.ClaimFrmFmt();

            // alle Zahlenformate entfernen
            sal_uInt16 nWhich1 = RES_BOXATR_FORMULA;
            if( !bIsEmptyTxtNd )
            {
                nWhich1 = RES_BOXATR_FORMAT;

                // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
                //              Sorge dafuer, das der Text auch entsprechend
                //              formatiert wird!
                pBoxFmt->SetFmtAttr( *GetDfltAttr( nWhich1 ));
            }
            pBoxFmt->ResetFmtAttr( nWhich1, RES_BOXATR_VALUE );
        }
        else
            bChgd = sal_False;
    }

    if( bChgd )
    {
        if( pUndo )
        {
            pUndo->SetBox( rBox );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
            GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
        }

        const SwTableNode* pTblNd = rBox.GetSttNd()->FindTableNode();
        if( bCallUpdate )
        {
            SwTableFmlUpdate aTblUpdate( &pTblNd->GetTable() );
            UpdateTblFlds( &aTblUpdate );

            // TL_CHART2: update charts (when cursor leaves cell and
            // automatic update is enabled)
            if (AUTOUPD_FIELD_AND_CHARTS == getFieldUpdateFlags(true))
                pTblNd->GetTable().UpdateCharts();
        }
        SetModified();
    }
}

void SwDoc::SetTblBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoTblNumFmt(rBox, &rSet) );
    }

    SwFrmFmt* pBoxFmt = rBox.ClaimFrmFmt();
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
    {
        pBoxFmt->LockModify();
        pBoxFmt->ResetFmtAttr( RES_BOXATR_VALUE );
        pBoxFmt->UnlockModify();
    }
    else if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_VALUE ))
    {
        pBoxFmt->LockModify();
        pBoxFmt->ResetFmtAttr( RES_BOXATR_FORMULA );
        pBoxFmt->UnlockModify();
    }
    pBoxFmt->SetFmtAttr( rSet );
    SetModified();
}

void SwDoc::ClearLineNumAttrs( SwPosition & rPos )
{
    SwPaM aPam(rPos);
    aPam.Move(fnMoveBackward);
    SwCntntNode *pNode = aPam.GetCntntNode();
    if ( 0 == pNode )
        return ;
    if( pNode->IsTxtNode() )
    {
        SwTxtNode * pTxtNode = pNode->GetTxtNode();
        if ( pTxtNode && pTxtNode->IsNumbered() && pTxtNode->GetTxt().Len()==0 )
        {
            const SfxPoolItem* pFmtItem = 0;
            SfxItemSet rSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                        RES_PARATR_BEGIN, RES_PARATR_END - 1,
                        0);
            pTxtNode->SwCntntNode::GetAttr( rSet );
            if ( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE , sal_False , &pFmtItem ) )
            {
                SwUndoDelNum * pUndo;
                if( GetIDocumentUndoRedo().DoesUndo() )
                {
                    GetIDocumentUndoRedo().ClearRedo();
                    GetIDocumentUndoRedo().AppendUndo( pUndo = new SwUndoDelNum( aPam ) );
                }
                else
                    pUndo = 0;
                SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : 0 );
                aRegH.RegisterInModify( pTxtNode , *pTxtNode );
                if ( pUndo )
                    pUndo->AddNode( *pTxtNode , sal_False );
                SfxStringItem * pNewItem = (SfxStringItem*)pFmtItem->Clone();
                pNewItem->SetValue(rtl::OUString());
                rSet.Put( *pNewItem );
                pTxtNode->SetAttr( rSet );
                delete pNewItem;
            }
        }
    }
}

void SwDoc::ClearBoxNumAttrs( const SwNodeIndex& rNode )
{
    SwStartNode* pSttNd;
    if( 0 != ( pSttNd = rNode.GetNode().
                                FindSttNodeByType( SwTableBoxStartNode )) &&
        2 == pSttNd->EndOfSectionIndex() - pSttNd->GetIndex() )
    {
        SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
                            GetTblBox( pSttNd->GetIndex() );

        const SfxPoolItem* pFmtItem = 0;
        const SfxItemSet& rSet = pBox->GetFrmFmt()->GetAttrSet();
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT, sal_False, &pFmtItem ) ||
            SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA, sal_False ) ||
            SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_VALUE, sal_False ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoTblNumFmt(*pBox));
            }

            SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();

            //JP 01.09.97: TextFormate bleiben erhalten!
            sal_uInt16 nWhich1 = RES_BOXATR_FORMAT;
            if( pFmtItem && GetNumberFormatter()->IsTextFormat(
                    ((SwTblBoxNumFormat*)pFmtItem)->GetValue() ))
                nWhich1 = RES_BOXATR_FORMULA;
            else
                // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
                //              Sorge dafuer, das der Text auch entsprechend
                //              formatiert wird!
                pBoxFmt->SetFmtAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));

            pBoxFmt->ResetFmtAttr( nWhich1, RES_BOXATR_VALUE );
            SetModified();
        }
    }
}

// kopiert eine Tabelle aus dem selben oder einem anderen Doc in sich
// selbst. Dabei wird eine neue Tabelle angelegt oder eine bestehende
// mit dem Inhalt gefuellt; wobei entweder der Inhalt ab einer Box oder
// in eine bestehende TblSelektion gefuellt wird.
// Gerufen wird es von: edglss.cxx/fecopy.cxx

sal_Bool SwDoc::InsCopyOfTbl( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTbl, sal_Bool bCpyName, sal_Bool bCorrPos )
{
    sal_Bool bRet;

    const SwTableNode* pSrcTblNd = pCpyTbl
            ? pCpyTbl->GetTableNode()
            : rBoxes[ 0 ]->GetSttNd()->FindTableNode();

    SwTableNode * pInsTblNd = rInsPos.nNode.GetNode().FindTableNode();

    bool const bUndo( GetIDocumentUndoRedo().DoesUndo() );
    if( !pCpyTbl && !pInsTblNd )
    {
        SwUndoCpyTbl* pUndo = 0;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoCpyTbl;
        }

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            bRet = pSrcTblNd->GetTable().MakeCopy( this, rInsPos, rBoxes,
                                                sal_True, bCpyName );
        }

        if( pUndo )
        {
            if( !bRet )
            {
                delete pUndo;
                pUndo = 0;
            }
            else
            {
                pInsTblNd = GetNodes()[ rInsPos.nNode.GetIndex() - 1 ]->FindTableNode();

                pUndo->SetTableSttIdx( pInsTblNd->GetIndex() );
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
        }
    }
    else
    {
        RedlineMode_t eOld = GetRedlineMode();
        if( IsRedlineOn() )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON |
                                  nsRedlineMode_t::REDLINE_SHOW_INSERT |
                                  nsRedlineMode_t::REDLINE_SHOW_DELETE));

        SwUndoTblCpyTbl* pUndo = 0;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoTblCpyTbl;
            GetIDocumentUndoRedo().DoUndo(false);
        }

        SwDoc* pCpyDoc = (SwDoc*)pSrcTblNd->GetDoc();
        sal_Bool bDelCpyDoc = pCpyDoc == this;

        if( bDelCpyDoc )
        {
            // kopiere die Tabelle erstmal in ein temp. Doc
            pCpyDoc = new SwDoc;
            pCpyDoc->acquire();

            SwPosition aPos( SwNodeIndex( pCpyDoc->GetNodes().GetEndOfContent() ));
            if( !pSrcTblNd->GetTable().MakeCopy( pCpyDoc, aPos, rBoxes, sal_True, sal_True ))
            {
                if( pCpyDoc->release() == 0 )
                    delete pCpyDoc;

                if( pUndo )
                {
                    GetIDocumentUndoRedo().DoUndo(bUndo);
                    delete pUndo;
                    pUndo = 0;
                }
                return sal_False;
            }
            aPos.nNode -= 1;        // auf den EndNode der Tabelle
            pSrcTblNd = aPos.nNode.GetNode().FindTableNode();
        }

        const SwStartNode* pSttNd = rInsPos.nNode.GetNode().FindTableBoxStartNode();

        rInsPos.nContent.Assign( 0, 0 );

        // no complex into complex, but copy into or from new model is welcome
        if( ( !pSrcTblNd->GetTable().IsTblComplex() || pInsTblNd->GetTable().IsNewModel() )
            && ( bDelCpyDoc || !rBoxes.empty() ) )
        {
            // dann die Tabelle "relativ" kopieren
            const SwSelBoxes* pBoxes;
            SwSelBoxes aBoxes;

            if( bDelCpyDoc )
            {
                SwTableBox* pBox = pInsTblNd->GetTable().GetTblBox(
                                        pSttNd->GetIndex() );
                OSL_ENSURE( pBox, "Box steht nicht in dieser Tabelle" );
                aBoxes.insert( pBox );
                pBoxes = &aBoxes;
            }
            else
                pBoxes = &rBoxes;

            // kopiere die Tabelle in die selktierten Zellen.
            bRet = pInsTblNd->GetTable().InsTable( pSrcTblNd->GetTable(),
                                                        *pBoxes, pUndo );
        }
        else
        {
            SwNodeIndex aNdIdx( *pSttNd, 1 );
            bRet = pInsTblNd->GetTable().InsTable( pSrcTblNd->GetTable(),
                                                    aNdIdx, pUndo );
        }

        if( bDelCpyDoc )
        {
            if( pCpyDoc->release() == 0 )
                delete pCpyDoc;
        }

        if( pUndo )
        {
            // falls die Tabelle nicht kopiert werden konnte, das Undo-Object
            // wieder loeschen
            GetIDocumentUndoRedo().DoUndo(bUndo);
            if( !bRet && pUndo->IsEmpty() )
                delete pUndo;
            else
            {
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
        }

        if( bCorrPos )
        {
            rInsPos.nNode = *pSttNd;
            rInsPos.nContent.Assign( GetNodes().GoNext( &rInsPos.nNode ), 0 );
        }
        SetRedlineMode( eOld );
    }

    if( bRet )
    {
        SetModified();
        SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}



sal_Bool SwDoc::_UnProtectTblCells( SwTable& rTbl )
{
    sal_Bool bChgd = sal_False;
    SwUndoAttrTbl *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoAttrTbl( *rTbl.GetTableNode() )
        :   0;

    SwTableSortBoxes& rSrtBox = rTbl.GetTabSortBoxes();
    for( sal_uInt16 i = rSrtBox.size(); i; )
    {
        SwFrmFmt *pBoxFmt = rSrtBox[ --i ]->GetFrmFmt();
        if( pBoxFmt->GetProtect().IsCntntProtected() )
        {
            pBoxFmt->ResetFmtAttr( RES_PROTECT );
            bChgd = sal_True;
        }
    }

    if( pUndo )
    {
        if( bChgd )
        {
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }
    return bChgd;
}


sal_Bool SwDoc::UnProtectCells( const String& rName )
{
    sal_Bool bChgd = sal_False;
    SwTableFmt* pFmt = FindTblFmtByName( rName );
    if( pFmt )
    {
        bChgd = _UnProtectTblCells( *SwTable::FindTable( pFmt ) );
        if( bChgd )
            SetModified();
    }

    return bChgd;
}

sal_Bool SwDoc::UnProtectCells( const SwSelBoxes& rBoxes )
{
    sal_Bool bChgd = sal_False;
    if( !rBoxes.empty() )
    {
        SwUndoAttrTbl *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
                ? new SwUndoAttrTbl( *rBoxes[0]->GetSttNd()->FindTableNode() )
                : 0;

        std::vector<SwFrmFmt*> aFmts, aNewFmts;
        for( sal_uInt16 i = rBoxes.size(); i; )
        {
            SwTableBox* pBox = rBoxes[ --i ];
            SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
            if( pBoxFmt->GetProtect().IsCntntProtected() )
            {
                std::vector<SwFrmFmt*>::iterator it = std::find( aFmts.begin(), aFmts.end(), pBoxFmt );
                if( aFmts.end() != it )
                    pBox->ChgFrmFmt( (SwTableBoxFmt*)*it );
                else
                {
                    aFmts.push_back( pBoxFmt );
                    pBoxFmt = pBox->ClaimFrmFmt();
                    pBoxFmt->ResetFmtAttr( RES_PROTECT );
                    aNewFmts.push_back( pBoxFmt );
                }
                bChgd = sal_True;
            }
        }

        if( pUndo )
        {
            if( bChgd )
            {
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
            else
                delete pUndo;
        }
    }
    return bChgd;
}

sal_Bool SwDoc::UnProtectTbls( const SwPaM& rPam )
{
    GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

    sal_Bool bChgd = sal_False, bHasSel = rPam.HasMark() ||
                                    rPam.GetNext() != (SwPaM*)&rPam;
    SwFrmFmts& rFmts = *GetTblFrmFmts();
    SwTable* pTbl;
    const SwTableNode* pTblNd;
    for( sal_uInt16 n = rFmts.size(); n ; )
        if( 0 != (pTbl = SwTable::FindTable( rFmts[ --n ] )) &&
            0 != (pTblNd = pTbl->GetTableNode() ) &&
            pTblNd->GetNodes().IsDocNodes() )
        {
            sal_uLong nTblIdx = pTblNd->GetIndex();

            // dann ueberpruefe ob Tabelle in der Selection liegt
            if( bHasSel )
            {
                int bFound = sal_False;
                SwPaM* pTmp = (SwPaM*)&rPam;
                do {
                    const SwPosition *pStt = pTmp->Start(),
                                    *pEnd = pTmp->End();
                    bFound = pStt->nNode.GetIndex() < nTblIdx &&
                            nTblIdx < pEnd->nNode.GetIndex();

                } while( !bFound && &rPam != ( pTmp = (SwPaM*)pTmp->GetNext() ) );
                if( !bFound )
                    continue;       // weitersuchen
            }

            // dann mal den Schutz aufheben
            bChgd |= _UnProtectTblCells( *pTbl );
        }

    GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    if( bChgd )
        SetModified();

    return bChgd;
}

sal_Bool SwDoc::HasTblAnyProtection( const SwPosition* pPos,
                                 const String* pTblName,
                                 sal_Bool* pFullTblProtection )
{
    sal_Bool bHasProtection = sal_False;
    SwTable* pTbl = 0;
    if( pTblName )
        pTbl = SwTable::FindTable( FindTblFmtByName( *pTblName ) );
    else if( pPos )
    {
        SwTableNode* pTblNd = pPos->nNode.GetNode().FindTableNode();
        if( pTblNd )
            pTbl = &pTblNd->GetTable();
    }

    if( pTbl )
    {
        SwTableSortBoxes& rSrtBox = pTbl->GetTabSortBoxes();
        for( sal_uInt16 i = rSrtBox.size(); i; )
        {
            SwFrmFmt *pBoxFmt = rSrtBox[ --i ]->GetFrmFmt();
            if( pBoxFmt->GetProtect().IsCntntProtected() )
            {
                if( !bHasProtection )
                {
                    bHasProtection = sal_True;
                    if( !pFullTblProtection )
                        break;
                    *pFullTblProtection = sal_True;
                }
            }
            else if( bHasProtection && pFullTblProtection )
            {
                *pFullTblProtection = sal_False;
                break;
            }
        }
    }
    return bHasProtection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
