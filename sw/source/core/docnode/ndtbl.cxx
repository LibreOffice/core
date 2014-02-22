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

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/boxitem.hxx>
#include <svl/stritem.hxx>

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


extern sal_Bool sw_GC_Line_Border( const SwTableLine*& , void* pPara );

static void lcl_SetDfltBoxAttr( SwFrmFmt& rFmt, sal_uInt8 nId )
{
    sal_Bool bTop = false, bBottom = false, bLeft = false, bRight = false;
    switch ( nId )
    {
    case 0: bTop = bBottom = bLeft = true;          break;
    case 1: bTop = bBottom = bLeft = bRight = true; break;
    case 2: bBottom = bLeft = true;                 break;
    case 3: bBottom = bLeft = bRight = true;        break;
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

static SwTableBoxFmt *lcl_CreateDfltBoxFmt( SwDoc &rDoc, std::vector<SwTableBoxFmt*> &rBoxFmtArr,
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

static SwTableBoxFmt *lcl_CreateAFmtBoxFmt( SwDoc &rDoc, std::vector<SwTableBoxFmt*> &rBoxFmtArr,
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

/**
 * Insert a new Box before the InsPos
 */
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
    OSL_ENSURE( pLine, "No valid Line" );

    
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
        bool bSetIdxPos = true;
        if( pTblNd->GetTable().GetTabLines().size() && !nInsPos )
        {
            const SwTableLine* pTblLn = pLine;
            while( pTblLn->GetUpper() )
                pTblLn = pTblLn->GetUpper()->GetUpper();

            if( pTblNd->GetTable().GetTabLines()[ 0 ] == pTblLn )
            {
                
                while( ( pNxtBox = pLine->GetTabBoxes()[0])->GetTabLines().size() )
                    pLine = pNxtBox->GetTabLines()[0];
                nIdxPos = pNxtBox->GetSttIdx();
                bSetIdxPos = false;
            }
        }
        if( bSetIdxPos )
            
            nIdxPos = pTblNd->EndOfSectionIndex();
    }
    else if( pNxtBox ) 
        nIdxPos = pNxtBox->GetSttIdx();
    else 
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

        if( ! pTxtColl->IsAssignedToListLevelOfOutlineStyle()

            && RES_CONDTXTFMTCOLL != pTxtColl->Which()

        )
            new SwTxtNode( SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                                pTxtColl, pAutoAttr );
        else
        {
            
            SwTxtNode* pTNd = new SwTxtNode(
                            SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                            (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl(),
                            pAutoAttr );
            pTNd->ChgFmtColl( pTxtColl );
        }
    }
    return sal_True;
}

/**
 * Insert a new Table
 */
const SwTable* SwDoc::InsertTable( const SwInsertTableOptions& rInsTblOpts,
                                   const SwPosition& rPos, sal_uInt16 nRows,
                                   sal_uInt16 nCols, sal_Int16 eAdjust,
                                   const SwTableAutoFmt* pTAFmt,
                                   const std::vector<sal_uInt16> *pColArr,
                                   sal_Bool bCalledFromShell,
                                   sal_Bool bNewModel )
{
    OSL_ENSURE( nRows, "Table without line?" );
    OSL_ENSURE( nCols, "Table without rows?" );

    {
        
        if( rPos.nNode < GetNodes().GetEndOfInserts().GetIndex() &&
            rPos.nNode >= GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;

        
        if( pColArr &&
            (size_t)(nCols + ( text::HoriOrientation::NONE == eAdjust ? 2 : 1 )) != pColArr->size() )
            pColArr = 0;
    }

    OUString aTblName = GetUniqueTblName();

    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsTbl( rPos, nCols, nRows, static_cast<sal_uInt16>(eAdjust),
                                      rInsTblOpts, pTAFmt, pColArr,
                                      aTblName));
    }

    
    SwTxtFmtColl *pBodyColl = GetTxtCollFromPool( RES_POOLCOLL_TABLE ),
                 *pHeadColl = pBodyColl;

    bool bDfltBorders = 0 != ( rInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER );

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

    
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( aTblName, GetDfltFrmFmt() );

    /* If the node to insert the table at is a context node and has a
       non-default FRAMEDIR propagate it to the table. */
    if (pCntntNd)
    {
        const SwAttrSet & aNdSet = pCntntNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    
    pTableFmt->SetFmtAttr( SwFmtHoriOrient( 0, eAdjust ) );
    
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));

    
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
        nWidth *= nCols; 
    }
    pTableFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth ));
    if( !(rInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT) )
        pTableFmt->SetFmtAttr( SwFmtLayoutSplit( sal_False ));

    
    SwCntntNode* pNextNd = GetNodes()[ pTblNd->EndOfSectionIndex()+1 ]
                            ->GetCntntNode();
    if( pNextNd && pNextNd->HasSwAttrSet() )
    {
        const SfxItemSet* pNdSet = pNextNd->GetpSwAttrSet();
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, false,
            &pItem ) )
        {
            pTableFmt->SetFmtAttr( *pItem );
            pNextNd->ResetAttr( RES_PAGEDESC );
            pNdSet = pNextNd->GetpSwAttrSet();
        }
        if( pNdSet && SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, false,
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

    SwNodeIndex aNdIdx( *pTblNd, 1 ); 
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

            
            
            if( pColArr )
            {
                nWidth = (*pColArr)[ i + 1 ] - (*pColArr)[ i ];
                if( pBoxF->GetFrmSize().GetWidth() != nWidth )
                {
                    if( pBoxF->GetDepends() ) 
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
            aNdIdx += 3; 
        }
    }
    
    GetNodes().GoNext( &aNdIdx ); 
    pTblNd->MakeFrms( &aNdIdx );

    
    if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
    {
        SwPaM aPam( *pTblNd->EndOfSectionNode(), *pTblNd, 1 );
        if( IsRedlineOn() )
            AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
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

    
    if( !pHeadlineTxtColl || !nLines )
        pHeadlineTxtColl = pCntntTxtColl;

    SwTableNode * pTblNd = new SwTableNode( rNdIdx );
    SwEndNode* pEndNd = new SwEndNode( rNdIdx, *pTblNd );

    if( !nLines ) 
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
                         SFX_ITEM_SET == pAttrSet->GetItemState( *pIdx, true, &pItem ) )
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

/**
 * Text to Table
 */
const SwTable* SwDoc::TextToTable( const SwInsertTableOptions& rInsTblOpts,
                                   const SwPaM& rRange, sal_Unicode cCh,
                                   sal_Int16 eAdjust,
                                   const SwTableAutoFmt* pTAFmt )
{
    
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    {
        sal_uLong nCnt = pStt->nNode.GetIndex();
        for( ; nCnt <= pEnd->nNode.GetIndex(); ++nCnt )
            if( !GetNodes()[ nCnt ]->IsTxtNode() )
                return 0;
    }

    
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

        
        GetIDocumentUndoRedo().DoUndo( false );
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        SplitNode( *pStt, false );

    bool bEndCntnt = 0 != pEnd->nContent.GetIndex();

    
    if( bEndCntnt )
    {
        if( pEnd->nNode.GetNode().GetCntntNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            SplitNode( *pEnd, false );
            ((SwNodeIndex&)pEnd->nNode)--;
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetCntntNode(), 0 );
            
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                aRg.aStart--;
        }
        else
            aRg.aEnd++;
    }


    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "empty range" );
        aRg.aEnd++;
    }

    
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo( 0 != pUndo );

    
    SwTableBoxFmt* pBoxFmt = MakeTableBoxFmt();
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( GetUniqueTblName(), GetDfltFrmFmt() );

    
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));
    
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

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    SwTableNode* pTblNd = GetNodes().TextToTable(
            aRg, cCh, pTableFmt, pLineFmt, pBoxFmt,
            GetTxtCollFromPool( RES_POOLCOLL_STANDARD ), pUndo );

    SwTable * pNdTbl = &pTblNd->GetTable();
    OSL_ENSURE( pNdTbl, "No Table Node created" );

    const sal_uInt16 nRowsToRepeat =
            tabopts::HEADLINE == (rInsTblOpts.mnInsMode & tabopts::HEADLINE) ?
            rInsTblOpts.mnRowsToRepeat :
            0;
    pNdTbl->SetRowsToRepeat( nRowsToRepeat );

    bool bUseBoxFmt = false;
    if( !pBoxFmt->GetDepends() )
    {
        
        
        bUseBoxFmt = true;
        pTableFmt->SetFmtAttr( pBoxFmt->GetFrmSize() );
        delete pBoxFmt;
        eAdjust = text::HoriOrientation::NONE;
    }

    
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
                bool bChgSz = false;

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

    
    if( IsInsTblFormatNum() )
    {
        for (size_t nBoxes = pNdTbl->GetTabSortBoxes().size(); nBoxes; )
        {
            ChkBoxNumFmt( *pNdTbl->GetTabSortBoxes()[ --nBoxes ], sal_False );
        }
    }

    sal_uLong nIdx = pTblNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    {
        SwPaM& rTmp = (SwPaM&)rRange; 
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

static void lcl_RemoveBreaks(SwCntntNode & rNode, SwTableFmt *const pTableFmt)
{
    
    rNode.DelFrms();

    if (!rNode.IsTxtNode())
    {
        return;
    }

    SwTxtNode & rTxtNode = static_cast<SwTxtNode&>(rNode);
    
    SfxItemSet const* pSet = rTxtNode.GetpSwAttrSet();
    if (pSet)
    {
        const SfxPoolItem* pItem;
        if (SFX_ITEM_SET == pSet->GetItemState(RES_BREAK, false, &pItem))
        {
            if (pTableFmt)
            {
                pTableFmt->SetFmtAttr(*pItem);
            }
            rTxtNode.ResetAttr(RES_BREAK);
            pSet = rTxtNode.GetpSwAttrSet();
        }

        if (pSet
            && (SFX_ITEM_SET == pSet->GetItemState(RES_PAGEDESC, false, &pItem))
            && static_cast<SwFmtPageDesc const*>(pItem)->GetPageDesc())
        {
            if (pTableFmt)
            {
                pTableFmt->SetFmtAttr(*pItem);
            }
            rTxtNode.ResetAttr(RES_PAGEDESC);
        }
    }
}

/**
 * balance lines in table, insert empty boxes so all lines have the size
 */
static void
lcl_BalanceTable(SwTable & rTable, size_t const nMaxBoxes,
    SwTableNode & rTblNd, SwTableBoxFmt & rBoxFmt, SwTxtFmtColl & rTxtColl,
    SwUndoTxtToTbl *const pUndo, std::vector<sal_uInt16> *const pPositions)
{
    for (size_t n = 0; n < rTable.GetTabLines().size(); ++n)
    {
        SwTableLine *const pCurrLine = rTable.GetTabLines()[ n ];
        size_t const nBoxes = pCurrLine->GetTabBoxes().size();
        if (nMaxBoxes != nBoxes)
        {
            rTblNd.GetNodes().InsBoxen(&rTblNd, pCurrLine, &rBoxFmt, &rTxtColl,
                    0, nBoxes, nMaxBoxes - nBoxes);

            if (pUndo)
            {
                for (size_t i = nBoxes; i < nMaxBoxes; ++i)
                {
                    pUndo->AddFillBox( *pCurrLine->GetTabBoxes()[i] );
                }
            }

            
            if (!n && pPositions)
            {
                pPositions->clear();
            }
        }
    }
}

static void
lcl_SetTableBoxWidths(SwTable & rTable, size_t const nMaxBoxes,
        SwTableBoxFmt & rBoxFmt, SwDoc & rDoc,
        std::vector<sal_uInt16> *const pPositions)
{
    if (pPositions && !pPositions->empty())
    {
        SwTableLines& rLns = rTable.GetTabLines();
        sal_uInt16 nLastPos = 0;
        for (size_t n = 0; n < pPositions->size(); ++n)
        {
            SwTableBoxFmt *pNewFmt = rDoc.MakeTableBoxFmt();
            pNewFmt->SetFmtAttr(
                    SwFmtFrmSize(ATT_VAR_SIZE, (*pPositions)[n] - nLastPos));
            for (size_t nTmpLine = 0; nTmpLine < rLns.size(); ++nTmpLine)
            {
                
                
                pNewFmt->Add( rLns[ nTmpLine ]->GetTabBoxes()[ n ] );
            }

            nLastPos = (*pPositions)[ n ];
        }

        
        SAL_WARN_IF(rBoxFmt.GetDepends(), "sw.core",
                "who is still registered in the format?");
        rBoxFmt.SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nLastPos ));
    }
    else
    {
        rBoxFmt.SetFmtAttr(SwFmtFrmSize(ATT_VAR_SIZE, USHRT_MAX / nMaxBoxes));
    }
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
        OSL_ENSURE( pTxtNd, "Only add TextNodes to the Table" );

        if( !nLines && 0x0b == cCh )
        {
            cCh = 0x09;

            
            SwTxtFrmInfo aFInfo( (SwTxtFrm*)pTxtNd->getLayoutFrm( pTxtNd->GetDoc()->GetCurrentLayout() ) );
            if( aFInfo.IsOneLine() ) 
            {
                OUString const& rTxt(pTxtNd->GetTxt());
                for (sal_Int32 nChPos = 0; nChPos < rTxt.getLength(); ++nChPos)
                {
                    if (rTxt[nChPos] == cCh)
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

        lcl_RemoveBreaks(*pTxtNd, (0 == nLines) ? pTblFmt : 0);

        
        pTxtNd->pStartOfSection = pTblNd;

        pLine = new SwTableLine( pLineFmt, 1, 0 );
        pTable->GetTabLines().insert( pTable->GetTabLines().begin() + nLines, pLine );

        SwStartNode* pSttNd;
        SwPosition aCntPos( aSttIdx, SwIndex( pTxtNd ));

        std::vector<sal_uLong> aBkmkArr;
        _SaveCntntIdx( pDoc, aSttIdx.GetIndex(), pTxtNd->GetTxt().getLength(),
                       aBkmkArr );

        if( T2T_PARA != cCh )
        {
            for (sal_Int32 nChPos = 0; nChPos < pTxtNd->GetTxt().getLength();)
            {
                if (pTxtNd->GetTxt()[nChPos] == cCh)
                {
                    aCntPos.nContent = nChPos;
                    SwCntntNode* pNewNd = pTxtNd->SplitCntntNode( aCntPos );

                    if( !aBkmkArr.empty() )
                        _RestoreCntntIdx( aBkmkArr, *pNewNd, nChPos,
                                            nChPos + 1 );

                    
                    pTxtNd->EraseText( aCntPos.nContent, 1 );
                    nChPos = 0;

                    
                    const SwNodeIndex aTmpIdx( aCntPos.nNode, -1 );
                    pSttNd = new SwStartNode( aTmpIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
                    new SwEndNode( aCntPos.nNode, *pSttNd );
                    pNewNd->pStartOfSection = pSttNd;

                    
                    pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
                    pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
                }
                else
                {
                    ++nChPos;
                }
            }
        }

        
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( aBkmkArr, *pTxtNd, pTxtNd->GetTxt().getLength(),
                                pTxtNd->GetTxt().getLength()+1 );

        pSttNd = new SwStartNode( aCntPos.nNode, ND_STARTNODE, SwTableBoxStartNode );
        const SwNodeIndex aTmpIdx( aCntPos.nNode, 1 );
        new SwEndNode( aTmpIdx, *pSttNd  );
        pTxtNd->pStartOfSection = pSttNd;

        pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
        pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    lcl_BalanceTable(*pTable, nMaxBoxes, *pTblNd, *pBoxFmt, *pTxtColl,
            pUndo, &aPosArr);
    lcl_SetTableBoxWidths(*pTable, nMaxBoxes, *pBoxFmt, *pDoc, &aPosArr);

    return pTblNd;
}

const SwTable* SwDoc::TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes )
{
    if (rTableNodes.empty())
        return NULL;

    const std::vector<SwNodeRange>& rFirstRange = *rTableNodes.begin();

    if (rFirstRange.empty())
        return NULL;

    const std::vector<SwNodeRange>& rLastRange = *rTableNodes.rbegin();

    if (rLastRange.empty())
        return NULL;

    /* Save first node in the selection if it is a content node. */
    SwCntntNode * pSttCntntNd = rFirstRange.begin()->aStart.GetNode().GetCntntNode();

    const SwNodeRange& rStartRange = *rFirstRange.begin();
    const SwNodeRange& rEndRange = *rLastRange.rbegin();

    
    SwPaM aOriginal( rStartRange.aStart, rEndRange.aEnd );
    const SwPosition *pStt = aOriginal.GetMark();
    const SwPosition *pEnd = aOriginal.GetPoint();

    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        
        GetIDocumentUndoRedo().DoUndo(false);
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        SplitNode( *pStt, false );

    bool bEndCntnt = 0 != pEnd->nContent.GetIndex();

    
    if( bEndCntnt )
    {
        if( pEnd->nNode.GetNode().GetCntntNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            SplitNode( *pEnd, false );
            ((SwNodeIndex&)pEnd->nNode)--;
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetCntntNode(), 0 );
            
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                aRg.aStart--;
        }
        else
            aRg.aEnd++;
    }


    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "empty range" );
        aRg.aEnd++;
    }

    
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo(bUndo);

    
    SwTableBoxFmt* pBoxFmt = MakeTableBoxFmt();
    SwTableLineFmt* pLineFmt = MakeTableLineFmt();
    SwTableFmt* pTableFmt = MakeTblFrmFmt( GetUniqueTblName(), GetDfltFrmFmt() );

    
    pLineFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ));
    
    pTableFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttCntntNd)
    {
        const SwAttrSet & aNdSet = pSttCntntNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SFX_ITEM_SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFmt->SetFmtAttr( *pItem );
        }
    }

    SwTableNode* pTblNd = GetNodes().TextToTable(
            rTableNodes, pTableFmt, pLineFmt, pBoxFmt,
            GetTxtCollFromPool( RES_POOLCOLL_STANDARD )/*, pUndo*/ );

    SwTable * pNdTbl = &pTblNd->GetTable();
    OSL_ENSURE( pNdTbl, "No Table Node created"  );
    pNdTbl->RegisterToFormat( *pTableFmt );

    if( !pBoxFmt->GetDepends() )
    {
        
        
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

static void
lcl_SetTableBoxWidths2(SwTable & rTable, size_t const nMaxBoxes,
        SwTableBoxFmt & rBoxFmt, SwDoc & rDoc)
{
    
    SwTableLines & rLines = rTable.GetTabLines();
    for (size_t nTmpLine = 0; nTmpLine < rLines.size(); ++nTmpLine)
    {
        SwTableBoxes & rBoxes = rLines[nTmpLine]->GetTabBoxes();
        size_t const nMissing = nMaxBoxes - rBoxes.size();
        if (nMissing)
        {
            
            SwTableBoxFmt *const pNewFmt = rDoc.MakeTableBoxFmt();
            pNewFmt->SetFmtAttr( SwFmtFrmSize(ATT_VAR_SIZE,
                        (USHRT_MAX / nMaxBoxes) * (nMissing + 1)) );
            pNewFmt->Add(rBoxes.back());
        }
    }
    
    rBoxFmt.SetFmtAttr(SwFmtFrmSize(ATT_VAR_SIZE, USHRT_MAX / nMaxBoxes));
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
    
   SwNodeIndex aInsertIndex( rTableNodes.rbegin()->rbegin()->aEnd );
   ++aInsertIndex;

   
   
   new SwEndNode( aInsertIndex, *pTblNd );

#if OSL_DEBUG_LEVEL > 1
    const SwNodeRange& rStartRange = *rTableNodes.begin()->begin();
    const SwNodeRange& rEndRange = *rTableNodes.rbegin()->rbegin();
    (void) rStartRange;
    (void) rEndRange;
#endif

    SwDoc* pDoc = GetDoc();
    SwTable * pTable = &pTblNd->GetTable();
    SwTableLine* pLine;
    SwTableBox* pBox;
    sal_uInt16 nBoxes, nLines, nMaxBoxes = 0;

    SwNodeIndex aNodeIndex = rTableNodes.begin()->begin()->aStart;
    
    for( nLines = 0; aNodeIndex <= rTableNodes.rbegin()->rbegin()->aEnd; ++aNodeIndex,++nLines )
    {
        SwNode& rNode = aNodeIndex.GetNode();
        if( rNode.IsCntntNode() )
        {
            lcl_RemoveBreaks(static_cast<SwCntntNode&>(rNode),
                    (0 == nLines) ? pTblFmt : 0);
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

                
                
                
                new SwEndNode( aCellEndIdx, *pSttNd );

                
                SwNodeIndex aCellNodeIdx = aCellIter->aStart;
                for(;aCellNodeIdx <= aCellIter->aEnd; ++aCellNodeIdx )
                {
                    aCellNodeIdx.GetNode().pStartOfSection = pSttNd;
                    
                    if( aCellNodeIdx.GetNode().IsStartNode() )
                        aCellNodeIdx = SwNodeIndex( *aCellNodeIdx.GetNode().EndOfSectionNode() );
                }

                
                pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
                pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        }
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    lcl_SetTableBoxWidths2(*pTable, nMaxBoxes, *pBoxFmt, *pDoc);

    return pTblNd;
}

/**
 * Table to Text
 */
sal_Bool SwDoc::TableToText( const SwTableNode* pTblNd, sal_Unicode cCh )
{
    if( !pTblNd )
        return sal_False;

    
    
    
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

/**
 * Use the ForEach method from PtrArray to recreate Text from a Table.
 * The Boxes can also contain Lines!
 */
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


static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara );

static void lcl_DelLine( SwTableLine* pLine, _DelTabPara* pPara )
{
    OSL_ENSURE( pPara, "The parameters are missing!" );
    _DelTabPara aPara( *pPara );
    for( SwTableBoxes::iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        lcl_DelBox(*it, &aPara );
    if( pLine->GetUpper() ) 
        
        pPara->pLastNd = aPara.pLastNd;
}

static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara )
{
    OSL_ENSURE( pDelPara, "The parameters are missing" );

    
    if( !pBox->GetTabLines().empty() )
    {
        BOOST_FOREACH( SwTableLine* pLine, pBox->GetTabLines() )
            lcl_DelLine( pLine, pDelPara );
    }
    else
    {
        SwDoc* pDoc = pDelPara->rNds.GetDoc();
        SwNodeRange aDelRg( *pBox->GetSttNd(), 0,
                            *pBox->GetSttNd()->EndOfSectionNode() );
        
        pDelPara->rNds.SectionUp( &aDelRg );
        const SwTxtNode* pCurTxtNd;
        if( T2T_PARA != pDelPara->cCh && pDelPara->pLastNd &&
            0 != ( pCurTxtNd = aDelRg.aStart.GetNode().GetTxtNode() ))
        {
            
            sal_uLong nNdIdx = aDelRg.aStart.GetIndex();
            aDelRg.aStart--;
            if( pDelPara->pLastNd == &aDelRg.aStart.GetNode() )
            {
                
                SwIndex aCntIdx( pDelPara->pLastNd,
                        pDelPara->pLastNd->GetTxt().getLength());
                pDelPara->pLastNd->InsertText( OUString(pDelPara->cCh), aCntIdx,
                    IDocumentContentOperations::INS_EMPTYEXPAND );
                if( pDelPara->pUndo )
                    pDelPara->pUndo->AddBoxPos( *pDoc, nNdIdx, aDelRg.aEnd.GetIndex(),
                                                aCntIdx.GetIndex() );

                std::vector<sal_uLong> aBkmkArr;
                const sal_Int32 nOldTxtLen = aCntIdx.GetIndex();
                _SaveCntntIdx( pDoc, nNdIdx, pCurTxtNd->GetTxt().getLength(),
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

        
        if( pDelPara->pLastNd && pDelPara->pLastNd->HasSwAttrSet() )
            pDelPara->pLastNd->ResetAttr( RES_PARATR_ADJUST );
    }
}

sal_Bool SwNodes::TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                            SwUndoTblToTxt* pUndo )
{
    
    SwTableNode* pTblNd;
    if( rRange.aStart.GetIndex() >= rRange.aEnd.GetIndex() ||
        0 == ( pTblNd = rRange.aStart.GetNode().GetTableNode()) ||
        &rRange.aEnd.GetNode() != pTblNd->EndOfSectionNode() )
        return sal_False;

    
    SwNode2Layout* pNode2Layout = 0;
    SwNodeIndex aFrmIdx( rRange.aStart );
    SwNode* pFrmNd = FindPrvNxtFrmNode( aFrmIdx, &rRange.aEnd.GetNode() );
    if( !pFrmNd )
        
        pNode2Layout = new SwNode2Layout( *pTblNd );

    
    pTblNd->DelFrms();

    
    _DelTabPara aDelPara( *this, cCh, pUndo );
    BOOST_FOREACH( SwTableLine *pLine, pTblNd->pTable->GetTabLines() )
        lcl_DelLine( pLine, &aDelPara );

    
    
    
    SwNodeRange aDelRg( rRange.aStart, rRange.aEnd );

    
    
    {
        
        const SfxItemSet& rTblSet = pTblNd->pTable->GetFrmFmt()->GetAttrSet();
        const SfxPoolItem *pBreak, *pDesc;
        if( SFX_ITEM_SET != rTblSet.GetItemState( RES_PAGEDESC, false, &pDesc ))
            pDesc = 0;
        if( SFX_ITEM_SET != rTblSet.GetItemState( RES_BREAK, false, &pBreak ))
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

    SectionUp( &aDelRg ); 
    
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

/**
 * Inserting Columns/Rows
 */
bool SwDoc::InsertCol( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    if( !::CheckSplitCells( rCursor, nCnt + 1, nsSwTblSearchType::TBLSEARCH_COL ) )
        return false;

    
    SwSelBoxes aBoxes;
    ::GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );

    bool bRet = false;
    if( !aBoxes.empty() )
        bRet = InsertCol( aBoxes, nCnt, bBehind );
    return bRet;
}

bool SwDoc::InsertCol( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    SwTable& rTbl = pTblNd->GetTable();
    if( rTbl.ISA( SwDDETable ))
        return false;

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

bool SwDoc::InsertRow( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );

    bool bRet = false;
    if( !aBoxes.empty() )
        bRet = InsertRow( aBoxes, nCnt, bBehind );
    return bRet;
}

bool SwDoc::InsertRow( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    SwTable& rTbl = pTblNd->GetTable();
    if( rTbl.ISA( SwDDETable ))
        return false;

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

/**
 * Deleting Columns/Rows
 */
sal_Bool SwDoc::DeleteRow( const SwCursor& rCursor )
{
    
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );
    if( ::HasProtectedCells( aBoxes ))
        return sal_False;

    
    
    
    
    {
        SwTableNode* pTblNd = rCursor.GetNode()->FindTableNode();

        if( pTblNd->GetTable().ISA( SwDDETable ))
            return sal_False;

        
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
            
        }

        _FndBox* pFndBox = &aFndBox;
        while( 1 == pFndBox->GetLines().size() &&
                1 == pFndBox->GetLines().front().GetBoxes().size() )
        {
            _FndBox *const pTmp = & pFndBox->GetLines().front().GetBoxes()[0];
            if( pTmp->GetBox()->GetSttNd() )
                break; 
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
                                                        pDelBox, true );
        while( pNextBox &&
                pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
            pNextBox = pNextBox->FindNextBox( pTblNd->GetTable(), pNextBox );

        if( !pNextBox ) 
        {
            pDelLine = pFndBox->GetLines().front().GetLine();
            pDelBox = pDelLine->GetTabBoxes()[ 0 ];
            while( !pDelBox->GetSttNd() )
                pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
            pNextBox = pDelLine->FindPreviousBox( pTblNd->GetTable(),
                                                        pDelBox, true );
            while( pNextBox &&
                    pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                pNextBox = pNextBox->FindPreviousBox( pTblNd->GetTable(), pNextBox );
        }

        sal_uLong nIdx;
        if( pNextBox ) 
            nIdx = pNextBox->GetSttIdx() + 1;
        else 
            nIdx = pTblNd->EndOfSectionIndex() + 1;

        SwNodeIndex aIdx( GetNodes(), nIdx );
        SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
        if( !pCNd )
            pCNd = GetNodes().GoNext( &aIdx );

        if( pCNd )
        {
            
            SwPaM* pPam = (SwPaM*)&rCursor;
            pPam->GetPoint()->nNode = aIdx;
            pPam->GetPoint()->nContent.Assign( pCNd, 0 );
            pPam->SetMark(); 
            pPam->DeleteMark();
        }
    }

    
    GetIDocumentUndoRedo().StartUndo(UNDO_ROW_DELETE, NULL);
    sal_Bool bResult = DeleteRowCol( aBoxes );
    GetIDocumentUndoRedo().EndUndo(UNDO_ROW_DELETE, NULL);

    return bResult;
}

sal_Bool SwDoc::DeleteCol( const SwCursor& rCursor )
{
    
    SwSelBoxes aBoxes;
    GetTblSel( rCursor, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
    if( ::HasProtectedCells( aBoxes ))
        return sal_False;

    
    
    
    SwEditShell* pESh = GetEditShell();
    if( pESh )
    {
        const SwNode* pNd = rCursor.GetNode()->FindTableBoxStartNode();
        pESh->ParkCrsr( SwNodeIndex( *pNd ) );
    }

    
    GetIDocumentUndoRedo().StartUndo(UNDO_COL_DELETE, NULL);
    sal_Bool bResult = DeleteRowCol( aBoxes, true );
    GetIDocumentUndoRedo().EndUndo(UNDO_COL_DELETE, NULL);

    return bResult;
}

sal_Bool SwDoc::DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn )
{
    if( ::HasProtectedCells( rBoxes ))
        return sal_False;

    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
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

    
    const sal_uLong nTmpIdx1 = pTblNd->GetIndex();
    const sal_uLong nTmpIdx2 = aSelBoxes.back()->GetSttNd()->EndOfSectionIndex() + 1;
    if( pTblNd->GetTable().GetTabSortBoxes().size() == aSelBoxes.size() &&
        aSelBoxes[0]->GetSttIdx()-1 == nTmpIdx1 &&
        nTmpIdx2 == pTblNd->EndOfSectionIndex() )
    {
        bool bNewTxtNd = false;
        
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
                        
                        DelLayoutFmt( pFmt );
                        return sal_True;
                    }
                }
                
                
                bNewTxtNd = true;
            }
        }

        
        ++aIdx;
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
        bRet = rTable.DeleteSel( this, aSelBoxes, 0, pUndo, true, true );
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

/**
 * Split up/merge Boxes in the Table
 */
sal_Bool SwDoc::SplitTbl( const SwSelBoxes& rBoxes, sal_Bool bVert, sal_uInt16 nCnt,
                      sal_Bool bSameHeight )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box list" );
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
            for (size_t n = 0; n < rBoxes.size(); ++n)
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

    
    GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_MERGE, NULL );

    RedlineMode_t eOld = GetRedlineMode();
    SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

    SwUndoTblMerge *const pUndo( (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTblMerge( rPam )
        :   0 );

    
    SwSelBoxes aBoxes;
    SwSelBoxes aMerged;
    SwTableBox* pMergeBox;

    if( !rTable.PrepareMerge( rPam, aBoxes, aMerged, &pMergeBox, pUndo ) )
    {   
        SetRedlineMode_intern( eOld );
        if( pUndo )
        {
            delete pUndo;
            SwUndoId nLastUndoId(UNDO_EMPTY);
            if (GetIDocumentUndoRedo().GetLastUndoInfo(0, & nLastUndoId)
                && (UNDO_REDLINE == nLastUndoId))
            {
                
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

/**
 * SwTableNode
 *
 */
SwTableNode::SwTableNode( const SwNodeIndex& rIdx )
    : SwStartNode( rIdx, ND_TABLENODE )
{
    pTable = new SwTable( 0 );
}

SwTableNode::~SwTableNode()
{
    
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

/**
 * Creates all Views from the Document for the preceeding Node. The resulting ContentFrames
 * are added to the corresponding Layout.
 */
void SwTableNode::MakeFrms(const SwNodeIndex & rIdx )
{
    if( !GetTable().GetFrmFmt()->GetDepends()) 
        return;

    SwFrm *pFrm, *pNew;
    SwCntntNode * pNode = rIdx.GetNode().GetCntntNode();

    OSL_ENSURE( pNode, "No ContentNode or CopyNode and new Node is identical");

    bool bBefore = rIdx < GetIndex();

    SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );

    while( 0 != (pFrm = aNode2Layout.NextFrm()) )
    {
        pNew = pNode->MakeFrm( pFrm );
        
        if ( bBefore )
            
            pNew->Paste( pFrm->GetUpper(), pFrm );
        else
            
            pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
    }
}

/**
 * Create a TblFrm for every Shell and insert before the corresponding CntntFrm.
 */
void SwTableNode::MakeFrms( SwNodeIndex* pIdxBehind )
{
    OSL_ENSURE( pIdxBehind, "No Index" );
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
        
        
        
        
        
        {
            SwViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
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
    /* For a start, cut out and delete the TabFrms (which will also delete the Columns and Rows)
       The TabFrms are attached to the FrmFmt of the SwTable.
       We need to delete them in a more cumbersome way, for the Master to also delete the Follows. */

    SwIterator<SwTabFrm,SwFmt> aIter( *(pTable->GetFrmFmt()) );
    SwTabFrm *pFrm = aIter.First();
    while ( pFrm )
    {
        bool bAgain = false;
        {
            if ( !pFrm->IsFollow() )
            {
                while ( pFrm->HasFollow() )
                    pFrm->JoinAndDelFollows();
                
                
                
                
                
                {
                    SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
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
                bAgain = true;
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
        OSL_ENSURE( !this, "One of them needs to be specified!" );
        return ;
    }

    
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



#define ROWFUZZY 25

struct FuzzyCompare
{
    bool operator() ( long s1, long s2 ) const;
};

bool FuzzyCompare::operator() ( long s1, long s2 ) const
{
    return ( s1 < s2 && abs( s1 - s2 ) > ROWFUZZY );
}

static bool lcl_IsFrmInColumn( const SwCellFrm& rFrm, SwSelBoxes& rBoxes )
{
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        if ( rFrm.GetTabBox() == rBoxes[ i ] )
            return true;
    }

    return false;
}

void SwDoc::GetTabRows( SwTabCols &rFill, const SwCursor* ,
                        const SwCellFrm* pBoxFrm ) const
{
    OSL_ENSURE( pBoxFrm, "GetTabRows called without pBoxFrm" );

    
    if ( !pBoxFrm )
        return;

    
    
    
    SwDeletionChecker aDelCheck( pBoxFrm );

    SwSelBoxes aBoxes;
    const SwCntntFrm* pCntnt = ::GetCellCntnt( *pBoxFrm );
    if ( pCntnt && pCntnt->IsTxtFrm() )
    {
        const SwPosition aPos( *((SwTxtFrm*)pCntnt)->GetTxtNode() );
        const SwCursor aTmpCrsr( aPos, 0, false );
        ::GetTblSel( aTmpCrsr, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
    }

    
    if ( aDelCheck.HasBeenDeleted() )
    {
        OSL_FAIL( "Current box has been deleted during GetTabRows()" );
        return;
    }

    
    const SwTabFrm* pTab = pBoxFrm->FindTabFrm();
    OSL_ENSURE( pTab, "GetTabRows called without a table" );
    if ( !pTab )
        return;

    const SwFrm* pFrm = pTab->GetNextLayoutLeaf();

    
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
            
            long nUpperBorder = (pFrm->Frm().*fnRect->fnGetTop)();
            long nLowerBorder = (pFrm->Frm().*fnRect->fnGetBottom)();

            
            aIter = aBoundaries.find( nUpperBorder );
            if ( aIter == aBoundaries.end() )
            {
                aPair.first = nUpperBorder; aPair.second = LONG_MAX;
                aBoundaries[ nUpperBorder ] = aPair;
            }

            
            aIter = aBoundaries.find( nLowerBorder );
            if ( aIter == aBoundaries.end() )
            {
                aPair.first = nUpperBorder; aPair.second = LONG_MAX;
            }
            else
            {
                nLowerBorder = (*aIter).first;
                long nNewLowerBorderUpperBoundary = std::max( (*aIter).second.first, nUpperBorder );
                aPair.first = nNewLowerBorderUpperBoundary; aPair.second = LONG_MAX;
            }
            aBoundaries[ nLowerBorder ] = aPair;

            
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

    
    OSL_ENSURE( rFill.Count(), "Deleting from empty vector. Fasten your seatbelts!" );
    
    
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
        OSL_ENSURE( !this, "One of them needs to be specified!" );
        return ;
    }

    
    
    SwTable& rTab = *pTab->GetTable();
    const SwFmtFrmSize& rTblFrmSz = rTab.GetFrmFmt()->GetFrmSize();
    SWRECTFN( pTab )
    
    
    
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

    
    
    SWRECTFN( pTab )
    SwTabCols aOld( rNew.Count() );

    
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
            
            
            
            
            
            SwTxtFrm* pTxtFrm = 0;
            const SwTableLine* pLine = 0;

            
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
                                if( nRowSpan > 0 ) 
                                    pTxtFrm = (SwTxtFrm*)pCntnt;
                                if( nRowSpan < 2 ) 
                                    pLine = pBox->GetUpper();
                                if( pLine && pTxtFrm ) 
                                {
                                    
                                    SwFmtFrmSize aNew( pLine->GetFrmFmt()->GetFrmSize() );
                                    const long nNewSize = (pFrm->Frm().*fnRect->fnGetHeight)() + nDiff;
                                    if( nNewSize != aNew.GetHeight() )
                                    {
                                        aNew.SetHeight( nNewSize );
                                        if ( ATT_VAR_SIZE == aNew.GetHeightSizeType() )
                                            aNew.SetHeightSizeType( ATT_MIN_SIZE );
                                        
                                        const SwPosition aPos( *((SwTxtFrm*)pCntnt)->GetTxtNode() );
                                        const SwCursor aTmpCrsr( aPos, 0, false );
                                        SetRowHeight( aTmpCrsr, aNew );
                                        
                                        
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

/**
 * Direct access for UNO
 */
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

bool SwCollectTblLineBoxes::Resize( sal_uInt16 nOffset, sal_uInt16 nOldWidth )
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

bool sw_Line_CollectBox( const SwTableLine*& rpLine, void* pPara )
{
    SwCollectTblLineBoxes* pSplPara = (SwCollectTblLineBoxes*)pPara;
    if( pSplPara->IsGetValues() )
        for( SwTableBoxes::iterator it = ((SwTableLine*)rpLine)->GetTabBoxes().begin();
                 it != ((SwTableLine*)rpLine)->GetTabBoxes().end(); ++it)
            sw_Box_CollectBox(*it, pSplPara );
    else
        for( SwTableBoxes::iterator it = ((SwTableLine*)rpLine)->GetTabBoxes().begin();
                 it != ((SwTableLine*)rpLine)->GetTabBoxes().end(); ++it)
            sw_BoxSetSplitBoxFmts(*it, pSplPara );
    return true;
}

void sw_Box_CollectBox( const SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara )
{
    sal_uInt16 nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = pBox->GetTabLines()[ nLen ];
        sw_Line_CollectBox( pLn, pSplPara );
    }
    else
        pSplPara->AddBox( *pBox );
}

void sw_BoxSetSplitBoxFmts( SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara )
{
    sal_uInt16 nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        
        if( pSplPara->IsGetFromTop() )
            nLen = 0;
        else
            --nLen;

        const SwTableLine* pLn = pBox->GetTabLines()[ nLen ];
        sw_Line_CollectBox( pLn, pSplPara );
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

                
                if( 2 == pDNd->EndOfSectionIndex() -
                        pDNd->StartOfSectionIndex() )
                {
                    pSplPara->AddToUndoHistory( *pDNd );
                    pDNd->ChgFmtColl( pCNd->GetFmtColl() );
                }
            }

            
            pBox->GetSttNd()->CheckSectionCondColl();
        }
    }
}

/**
 * Splits a Table in the top-level Line which contains the Index.
 * All succeeding top-level Lines go into a new Table/Node.
 *
 * @param bCalcNewSize true
 *                     Calculate the new Size for both from the
 *                     Boxes' Max; but only if Size is using absolute
 *                     values (USHRT_MAX)
 */
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
    rTbl.SetHTMLTableLayout( 0 ); 

    SwTableFmlUpdate aMsgHnt( &rTbl );

    SwHistory aHistory;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        aMsgHnt.pHistory = &aHistory;
    }

    {
        sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

        
        SwTableBox* pBox = rTbl.GetTblBox( nSttIdx );
        if( pBox )
        {
            SwTableLine* pLine = pBox->GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            
            aMsgHnt.nSplitLine = rTbl.GetTabLines().GetPos( pLine );
        }

        OUString sNewTblNm( GetUniqueTblName() );
        aMsgHnt.DATA.pNewTblNm = &sNewTblNm;
        aMsgHnt.eFlags = TBL_SPLITTBL;
        UpdateTblFlds( &aMsgHnt );
    }

    
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
        
        
        case HEADLINE_BORDERCOPY:
            {
                SwCollectTblLineBoxes aPara( false, eHdlnMode );
                SwTableLine* pLn = rTbl.GetTabLines()[
                            rTbl.GetTabLines().size() - 1 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_Box_CollectBox(*it, &aPara );

                aPara.SetValues( true );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFmts(*it, &aPara );

                
                pNew->GetTable().SetRowsToRepeat( 0 );
            }
            break;

        
        case HEADLINE_BOXATTRCOPY:
        case HEADLINE_BOXATRCOLLCOPY:
            {
                SwHistory* pHst = 0;
                if( HEADLINE_BOXATRCOLLCOPY == eHdlnMode && pUndo )
                    pHst = pUndo->GetHistory();

                SwCollectTblLineBoxes aPara( true, eHdlnMode, pHst );
                SwTableLine* pLn = rTbl.GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_Box_CollectBox(*it, &aPara );

                aPara.SetValues( true );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFmts(*it, &aPara );
            }
            break;

        case HEADLINE_CNTNTCOPY:
            rTbl.CopyHeadlineIntoTable( *pNew );
            if( pUndo )
                pUndo->SetTblNodeOffset( pNew->GetIndex() );
            break;

        case HEADLINE_NONE:
            
            pNew->GetTable().SetRowsToRepeat( 0 );
            break;
        }

        
        SwNodeIndex aNdIdx( *pNew->EndOfSectionNode() );
        GetNodes().GoNext( &aNdIdx ); 
        pNew->MakeFrms( &aNdIdx );

        
        GetNodes().MakeTxtNode( SwNodeIndex( *pNew ),
                                GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );
    }

    
    aFndBox.MakeFrms( rTbl );

    
    UpdateCharts( rTbl.GetFrmFmt()->GetName() );

    SetFieldsDirty( true, NULL, 0 );

    return 0 != pNew;
}

static bool lcl_ChgTblSize( SwTable& rTbl )
{
    
    
    
    SwFrmFmt* pFmt = rTbl.GetFrmFmt();
    SwFmtFrmSize aTblMaxSz( pFmt->GetFrmSize() );

    if( USHRT_MAX == aTblMaxSz.GetWidth() )
        return false;

    bool bLocked = pFmt->IsModifyLocked();
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
    if( !bLocked ) 
        pFmt->UnlockModify();

    return true;
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

    
    SwTable& rTbl = pTNd->GetTable();
    SwTableBox* pBox = rTbl.GetTblBox( nSttIdx );
    if( !pBox )
        return 0;

    SwTableLine* pLine = pBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    
    sal_uInt16 nLinePos = rTbl.GetTabLines().GetPos( pLine );
    if( USHRT_MAX == nLinePos ||
        ( bAfter ? ++nLinePos >= rTbl.GetTabLines().size() : !nLinePos ))
        return 0; 

    
    SwTableLine* pNextLine = rTbl.GetTabLines()[ nLinePos ];
    pBox = pNextLine->GetTabBoxes()[0];
    while( !pBox->GetSttNd() )
        pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];

    
    SwTableNode * pNewTblNd;
    {
        SwEndNode* pOldTblEndNd = (SwEndNode*)pTNd->EndOfSectionNode()->GetEndNode();
        OSL_ENSURE( pOldTblEndNd, "Where is the EndNode?" );

        SwNodeIndex aIdx( *pBox->GetSttNd() );
        new SwEndNode( aIdx, *pTNd );
        pNewTblNd = new SwTableNode( aIdx );
        pNewTblNd->GetTable().SetTableModel( rTbl.IsNewModel() );

        pOldTblEndNd->pStartOfSection = pNewTblNd;
        pNewTblNd->pEndOfSection = pOldTblEndNd;

        SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
        do {
            OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
            pBoxNd->pStartOfSection = pNewTblNd;
            pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
        } while( pBoxNd != pOldTblEndNd );
    }

    {
        
        SwTable& rNewTbl = pNewTblNd->GetTable();
        rNewTbl.GetTabLines().insert( rNewTbl.GetTabLines().begin(),
                      rTbl.GetTabLines().begin() + nLinePos, rTbl.GetTabLines().end() );

        /* From the back (bottom right) to the front (top left) deregister all Boxes from the
           Chart Data Provider. The Modify event is triggered in the calling function.
         TL_CHART2: */
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

        
        sal_uInt16 nDeleted = rTbl.GetTabLines().size() - nLinePos;
        rTbl.GetTabLines().erase( rTbl.GetTabLines().begin() + nLinePos, rTbl.GetTabLines().end() );

        
        _SplitTable_Para aPara( pNewTblNd, rTbl );
        BOOST_FOREACH( SwTableLine* pNewLine, rNewTbl.GetTabLines() )
            lcl_SplitTable_CpyLine( pNewLine, &aPara );
        rTbl.CleanUpBottomRowSpan( nDeleted );
    }

    {
        
        SwFrmFmt* pOldTblFmt = rTbl.GetFrmFmt();
        SwFrmFmt* pNewTblFmt = pOldTblFmt->GetDoc()->MakeTblFrmFmt(
                                pOldTblFmt->GetDoc()->GetUniqueTblName(),
                                pOldTblFmt->GetDoc()->GetDfltFrmFmt() );

        *pNewTblFmt = *pOldTblFmt;
        pNewTblNd->GetTable().RegisterToFormat( *pNewTblFmt );

        
        
        
        if( bCalcNewSize && lcl_ChgTblSize( rTbl ) )
            lcl_ChgTblSize( pNewTblNd->GetTable() );
    }

    
    rTbl.UpdateCharts();

    return pNewTblNd; 
}

/**
 * rPos needs to be in the Table that remains
 *
 * @param bWithPrev  merge the current Table with the preceeding
 *                   or succeeding one
 */
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

    
    pTblNd->GetTable().SetHTMLTableLayout( 0 );
    pDelTblNd->GetTable().SetHTMLTableLayout( 0 );

    
    SwUndoMergeTbl* pUndo = 0;
    SwHistory* pHistory = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMergeTbl( *pTblNd, *pDelTblNd, bWithPrev, nMode );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        pHistory = new SwHistory;
    }

    
    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.DATA.pDelTbl = &pDelTblNd->GetTable();
    aMsgHnt.eFlags = TBL_MERGETBL;
    aMsgHnt.pHistory = pHistory;
    UpdateTblFlds( &aMsgHnt );

    
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
    OSL_ENSURE( pDelTblNd, "Where did the TableNode go?" );

    SwTableNode* pTblNd = (*this)[ rPos.GetIndex() - 1]->FindTableNode();
    OSL_ENSURE( pTblNd, "Where did the TableNode go?" );

    if( !pDelTblNd || !pTblNd )
        return sal_False;

    pDelTblNd->DelFrms();

    SwTable& rDelTbl = pDelTblNd->GetTable();
    SwTable& rTbl = pTblNd->GetTable();

    
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rTbl );
    aFndBox.DelFrms( rTbl );

    
    
    GetDoc()->CreateChartInternalDataProviders( &rDelTbl );

    
    {
        const SwFmtFrmSize& rTblSz = rTbl.GetFrmFmt()->GetFrmSize();
        const SwFmtFrmSize& rDelTblSz = rDelTbl.GetFrmFmt()->GetFrmSize();
        if( rTblSz != rDelTblSz )
        {
            
            if( bWithPrev )
                rDelTbl.GetFrmFmt()->SetFmtAttr( rTblSz );
            else
                rTbl.GetFrmFmt()->SetFmtAttr( rDelTblSz );
        }
    }

    if( !bWithPrev )
    {
        
        
        rTbl.SetRowsToRepeat( rDelTbl.GetRowsToRepeat() );
        rTbl.SetTblChgMode( rDelTbl.GetTblChgMode() );

        rTbl.GetFrmFmt()->LockModify();
        *rTbl.GetFrmFmt() = *rDelTbl.GetFrmFmt();
        
        rTbl.GetFrmFmt()->SetName( rDelTbl.GetFrmFmt()->GetName() );
        rTbl.GetFrmFmt()->UnlockModify();
    }

    
    sal_uInt16 nOldSize = rTbl.GetTabLines().size();
    rTbl.GetTabLines().insert( rTbl.GetTabLines().begin() + nOldSize,
                               rDelTbl.GetTabLines().begin(), rDelTbl.GetTabLines().end() );
    rDelTbl.GetTabLines().clear();

    rTbl.GetTabSortBoxes().insert( rDelTbl.GetTabSortBoxes() );
    rDelTbl.GetTabSortBoxes().clear();

    
    SwEndNode* pTblEndNd = pDelTblNd->EndOfSectionNode();
    pTblNd->pEndOfSection = pTblEndNd;

    SwNodeIndex aIdx( *pDelTblNd, 1 );

    SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
    do {
        OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
        pBoxNd->pStartOfSection = pTblNd;
        pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
    } while( pBoxNd != pTblEndNd );
    pBoxNd->pStartOfSection = pTblNd;

    aIdx -= 2;
    DelNodes( aIdx, 2 );

    
    const SwTableLine* pFirstLn = rTbl.GetTabLines()[ nOldSize ];
    if( 1 == nMode )
    {
        
        
    }
    sw_LineSetHeadCondColl( pFirstLn );

    
    if( nOldSize )
    {
        _SwGCLineBorder aPara( rTbl );
        aPara.nLinePos = --nOldSize;
        pFirstLn = rTbl.GetTabLines()[ nOldSize ];
        sw_GC_Line_Border( pFirstLn, &aPara );
    }

    
    aFndBox.MakeFrms( rTbl );

    return sal_True;
}


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


static bool lcl_SetAFmtBox(_FndBox &, _SetAFmtTabPara *pSetPara);
static bool lcl_SetAFmtLine(_FndLine &, _SetAFmtTabPara *pPara);

static bool lcl_SetAFmtLine(_FndLine & rLine, _SetAFmtTabPara *pPara)
{
    for (_FndBoxes::iterator it = rLine.GetBoxes().begin();
         it != rLine.GetBoxes().end(); ++it)
    {
        lcl_SetAFmtBox(*it, pPara);
    }
    return true;
}

static bool lcl_SetAFmtBox( _FndBox & rBox, _SetAFmtTabPara *pSetPara )
{
    if (!rBox.GetUpper()->GetUpper()) 
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

    if (!rBox.GetUpper()->GetUpper()) 
        ++pSetPara->nCurBox;
    return true;
}

/**
 * AutoFormat for the Table/TableSelection
 */
sal_Bool SwDoc::SetTableAutoFmt( const SwSelBoxes& rBoxes, const SwTableAutoFmt& rNew )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    
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

    if( pFndBox->GetLines().empty() ) 
        pFndBox = pFndBox->GetUpper()->GetUpper();

    
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

/**
 * Find out who has the Attributes
 */
sal_Bool SwDoc::GetTableAutoFmt( const SwSelBoxes& rBoxes, SwTableAutoFmt& rGet )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return sal_False;

    
    SwTable &table = pTblNd->GetTable();
    rGet.StoreTableProperties(table);

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front().GetBoxes().size() )
    {
        pFndBox = &pFndBox->GetLines().front().GetBoxes()[0];
    }

    if( pFndBox->GetLines().empty() ) 
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

OUString SwDoc::GetUniqueTblName() const
{
    ResId aId( STR_TABLE_DEFNAME, *pSwResMgr );
    const OUString aName( aId );

    sal_uInt16 nNum, nTmp, nFlagSize = ( mpTblFrmFmtTbl->size() / 8 ) +2;
    sal_uInt16 n;

    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( n = 0; n < mpTblFrmFmtTbl->size(); ++n )
    {
        const SwFrmFmt* pFmt = (*mpTblFrmFmtTbl)[ n ];
        if( !pFmt->IsDefault() && IsUsed( *pFmt )  &&
            pFmt->GetName().startsWith( aName ) )
        {
            
            const sal_Int32 nNmLen = aName.getLength();
            nNum = static_cast<sal_uInt16>(pFmt->GetName().copy( nNmLen ).toInt32());
            if( nNum-- && nNum < mpTblFrmFmtTbl->size() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    
    nNum = mpTblFrmFmtTbl->size();
    for( n = 0; n < nFlagSize; ++n )
        if( 0xff != ( nTmp = pSetFlags[ n ] ))
        {
            
            nNum = n * 8;
            while( nTmp & 1 )
                ++nNum, nTmp >>= 1;
            break;
        }

    delete [] pSetFlags;
    return aName + OUString::number( ++nNum );
}

SwTableFmt* SwDoc::FindTblFmtByName( const OUString& rName, sal_Bool bAll ) const
{
    const SwFmt* pRet = 0;
    if( bAll )
        pRet = FindFmtByName( *mpTblFrmFmtTbl, rName );
    else
    {
        
        for( sal_uInt16 n = 0; n < mpTblFrmFmtTbl->size(); ++n )
        {
            const SwFrmFmt* pFmt = (*mpTblFrmFmtTbl)[ n ];
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

    GetIDocumentUndoRedo().DoUndo(bUndo); 
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
    
    const SfxPoolItem* pNumFmtItem = 0;
    if( SFX_ITEM_SET == rBox.GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT,
        sal_False, &pNumFmtItem ) && GetNumberFormatter()->IsTextFormat(
            ((SwTblBoxNumFormat*)pNumFmtItem)->GetValue() ))
        return ;

    SwUndoTblNumFmt* pUndo = 0;

    sal_Bool bIsEmptyTxtNd;
    bool bChgd = true;
    sal_uInt32 nFmtIdx;
    double fNumber;
    if( rBox.HasNumCntnt( fNumber, nFmtIdx, bIsEmptyTxtNd ) )
    {
        if( !rBox.IsNumberChanged() )
            bChgd = false;
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

            bool bLockModify = true;
            bool bSetNumberFormat = false;
            const bool bForceNumberFormat = IsInsTblFormatNum() && IsInsTblChangeNumFormat();

            
            
            
            if( pNumFmtItem && !bForceNumberFormat )
            {
                sal_uLong nOldNumFmt = ((SwTblBoxNumFormat*)pNumFmtItem)->GetValue();
                SvNumberFormatter* pNumFmtr = GetNumberFormatter();

                short nFmtType = pNumFmtr->GetType( nFmtIdx );
                if( nFmtType == pNumFmtr->GetType( nOldNumFmt ) || NUMBERFORMAT_NUMBER == nFmtType )
                {
                    
                    
                    nFmtIdx = nOldNumFmt;
                    bSetNumberFormat = true;
                }
                else
                {
                    
                    
                    bLockModify = bSetNumberFormat = false;
                }
            }

            if( bSetNumberFormat || bForceNumberFormat )
            {
                pBoxFmt = (SwTableBoxFmt*)rBox.ClaimFrmFmt();

                aBoxSet.Put( SwTblBoxValue( fNumber ));
                aBoxSet.Put( SwTblBoxNumFormat( nFmtIdx ));
            }

            
            
            if( !bSetNumberFormat && !bIsEmptyTxtNd && pNumFmtItem )
            {
                
                
                pBoxFmt->SetFmtAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));
            }

            if( bLockModify ) pBoxFmt->LockModify();
            pBoxFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            if( bLockModify ) pBoxFmt->UnlockModify();

            if( bSetNumberFormat )
                pBoxFmt->SetFmtAttr( aBoxSet );
        }
    }
    else
    {
        
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

            
            sal_uInt16 nWhich1 = RES_BOXATR_FORMULA;
            if( !bIsEmptyTxtNd )
            {
                nWhich1 = RES_BOXATR_FORMAT;

                
                
                pBoxFmt->SetFmtAttr( *GetDfltAttr( nWhich1 ));
            }
            pBoxFmt->ResetFmtAttr( nWhich1, RES_BOXATR_VALUE );
        }
        else
            bChgd = false;
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
        if (pTxtNode && pTxtNode->IsNumbered()
            && pTxtNode->GetTxt().isEmpty())
        {
            const SfxPoolItem* pFmtItem = 0;
            SfxItemSet rSet( const_cast<SwAttrPool&>(pTxtNode->GetDoc()->GetAttrPool()),
                        RES_PARATR_BEGIN, RES_PARATR_END - 1,
                        0);
            pTxtNode->SwCntntNode::GetAttr( rSet );
            if ( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE , false , &pFmtItem ) )
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
                pNewItem->SetValue(OUString());
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
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT, false, &pFmtItem ) ||
            SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA, false ) ||
            SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_VALUE, false ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoTblNumFmt(*pBox));
            }

            SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();

            
            sal_uInt16 nWhich1 = RES_BOXATR_FORMAT;
            if( pFmtItem && GetNumberFormatter()->IsTextFormat(
                    ((SwTblBoxNumFormat*)pFmtItem)->GetValue() ))
                nWhich1 = RES_BOXATR_FORMULA;
            else
                
                
                pBoxFmt->SetFmtAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));

            pBoxFmt->ResetFmtAttr( nWhich1, RES_BOXATR_VALUE );
            SetModified();
        }
    }
}

/**
 * Copies a Table from the same or another Doc into itself
 * We create a new Table or an existing one is filled with the Content.
 * We either fill in the Content from a certain Box or a certain TblSelection
 *
 * This method is called by edglss.cxx/fecopy.cxx
 */
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
                                                true, bCpyName );
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
        bool bDelCpyDoc = pCpyDoc == this;

        if( bDelCpyDoc )
        {
            
            pCpyDoc = new SwDoc;
            pCpyDoc->acquire();

            SwPosition aPos( SwNodeIndex( pCpyDoc->GetNodes().GetEndOfContent() ));
            if( !pSrcTblNd->GetTable().MakeCopy( pCpyDoc, aPos, rBoxes, true, true ))
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
            aPos.nNode -= 1; 
            pSrcTblNd = aPos.nNode.GetNode().FindTableNode();
        }

        const SwStartNode* pSttNd = rInsPos.nNode.GetNode().FindTableBoxStartNode();

        rInsPos.nContent.Assign( 0, 0 );

        
        if( ( !pSrcTblNd->GetTable().IsTblComplex() || pInsTblNd->GetTable().IsNewModel() )
            && ( bDelCpyDoc || !rBoxes.empty() ) )
        {
            
            const SwSelBoxes* pBoxes;
            SwSelBoxes aBoxes;

            if( bDelCpyDoc )
            {
                SwTableBox* pBox = pInsTblNd->GetTable().GetTblBox(
                                        pSttNd->GetIndex() );
                OSL_ENSURE( pBox, "Box is not in this Table" );
                aBoxes.insert( pBox );
                pBoxes = &aBoxes;
            }
            else
                pBoxes = &rBoxes;

            
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
    bool bChgd = false;
    SwUndoAttrTbl *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoAttrTbl( *rTbl.GetTableNode() )
        :   0;

    SwTableSortBoxes& rSrtBox = rTbl.GetTabSortBoxes();
    for (size_t i = rSrtBox.size(); i; )
    {
        SwFrmFmt *pBoxFmt = rSrtBox[ --i ]->GetFrmFmt();
        if( pBoxFmt->GetProtect().IsCntntProtected() )
        {
            pBoxFmt->ResetFmtAttr( RES_PROTECT );
            bChgd = true;
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

sal_Bool SwDoc::UnProtectCells( const OUString& rName )
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

        std::map<SwFrmFmt*, SwTableBoxFmt*> aFmtsMap;
        for (size_t i = rBoxes.size(); i; )
        {
            SwTableBox* pBox = rBoxes[ --i ];
            SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
            if( pBoxFmt->GetProtect().IsCntntProtected() )
            {
                std::map<SwFrmFmt*, SwTableBoxFmt*>::const_iterator const it =
                    aFmtsMap.find(pBoxFmt);
                if (aFmtsMap.end() != it)
                    pBox->ChgFrmFmt(it->second);
                else
                {
                    SwTableBoxFmt *const pNewBoxFmt(
                        static_cast<SwTableBoxFmt*>(pBox->ClaimFrmFmt()));
                    pNewBoxFmt->ResetFmtAttr( RES_PROTECT );
                    aFmtsMap.insert(std::make_pair(pBoxFmt, pNewBoxFmt));
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

            
            if( bHasSel )
            {
                bool bFound = false;
                SwPaM* pTmp = (SwPaM*)&rPam;
                do {
                    const SwPosition *pStt = pTmp->Start(),
                                    *pEnd = pTmp->End();
                    bFound = pStt->nNode.GetIndex() < nTblIdx &&
                            nTblIdx < pEnd->nNode.GetIndex();

                } while( !bFound && &rPam != ( pTmp = (SwPaM*)pTmp->GetNext() ) );
                if( !bFound )
                    continue; 
            }

            
            bChgd |= _UnProtectTblCells( *pTbl );
        }

    GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    if( bChgd )
        SetModified();

    return bChgd;
}

sal_Bool SwDoc::HasTblAnyProtection( const SwPosition* pPos,
                                 const OUString* pTblName,
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
        for (size_t i = rSrtBox.size(); i; )
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
