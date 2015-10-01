/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <fesh.hxx>
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
#include <DocumentSettingManager.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
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
#include <cstdlib>
#include <map>
#include <algorithm>
#include <rootfrm.hxx>
#include <fldupde.hxx>
#include <calbck.hxx>
#include <o3tl/numeric.hxx>
#include <tools/datetimeutils.hxx>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

const sal_Unicode T2T_PARA = 0x0a;

static void lcl_SetDfltBoxAttr( SwFrameFormat& rFormat, sal_uInt8 nId )
{
    bool bTop = false, bBottom = false, bLeft = false, bRight = false;
    switch ( nId )
    {
    case 0: bTop = bBottom = bLeft = true;          break;
    case 1: bTop = bBottom = bLeft = bRight = true; break;
    case 2: bBottom = bLeft = true;                 break;
    case 3: bBottom = bLeft = bRight = true;        break;
    }

    const bool bHTML = rFormat.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
    Color aCol( bHTML ? COL_GRAY : COL_BLACK );
    SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
    if ( bHTML )
    {
        aLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
        aLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    SvxBoxItem aBox(RES_BOX); aBox.SetDistance( 55 );
    if ( bTop )
        aBox.SetLine( &aLine, SvxBoxItemLine::TOP );
    if ( bBottom )
        aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
    if ( bLeft )
        aBox.SetLine( &aLine, SvxBoxItemLine::LEFT );
    if ( bRight )
        aBox.SetLine( &aLine, SvxBoxItemLine::RIGHT );
    rFormat.SetFormatAttr( aBox );
}

typedef std::map<SwFrameFormat *, SwTableBoxFormat *> DfltBoxAttrMap_t;
typedef std::vector<DfltBoxAttrMap_t *> DfltBoxAttrList_t;

static void
lcl_SetDfltBoxAttr(SwTableBox& rBox, DfltBoxAttrList_t & rBoxFormatArr,
        sal_uInt8 const nId, SwTableAutoFormat const*const pAutoFormat = 0)
{
    DfltBoxAttrMap_t * pMap = rBoxFormatArr[ nId ];
    if (!pMap)
    {
        pMap = new DfltBoxAttrMap_t;
        rBoxFormatArr[ nId ] = pMap;
    }

    SwTableBoxFormat* pNewTableBoxFormat = 0;
    SwFrameFormat* pBoxFrameFormat = rBox.GetFrameFormat();
    DfltBoxAttrMap_t::iterator const iter(pMap->find(pBoxFrameFormat));
    if (pMap->end() != iter)
    {
        pNewTableBoxFormat = iter->second;
    }
    else
    {
        SwDoc* pDoc = pBoxFrameFormat->GetDoc();
        // format does not exist, so create it
        pNewTableBoxFormat = pDoc->MakeTableBoxFormat();
        pNewTableBoxFormat->SetFormatAttr( pBoxFrameFormat->GetAttrSet().Get( RES_FRM_SIZE ) );

        if( pAutoFormat )
            pAutoFormat->UpdateToSet( nId, (SfxItemSet&)pNewTableBoxFormat->GetAttrSet(),
                                    SwTableAutoFormat::UPDATE_BOX,
                                    pDoc->GetNumberFormatter() );
        else
            ::lcl_SetDfltBoxAttr( *pNewTableBoxFormat, nId );

        (*pMap)[pBoxFrameFormat] = pNewTableBoxFormat;
    }
    rBox.ChgFrameFormat( pNewTableBoxFormat );
}

static SwTableBoxFormat *lcl_CreateDfltBoxFormat( SwDoc &rDoc, std::vector<SwTableBoxFormat*> &rBoxFormatArr,
                                    sal_uInt16 nCols, sal_uInt8 nId )
{
    if ( !rBoxFormatArr[nId] )
    {
        SwTableBoxFormat* pBoxFormat = rDoc.MakeTableBoxFormat();
        if( USHRT_MAX != nCols )
            pBoxFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        ::lcl_SetDfltBoxAttr( *pBoxFormat, nId );
        rBoxFormatArr[ nId ] = pBoxFormat;
    }
    return rBoxFormatArr[nId];
}

static SwTableBoxFormat *lcl_CreateAFormatBoxFormat( SwDoc &rDoc, std::vector<SwTableBoxFormat*> &rBoxFormatArr,
                                    const SwTableAutoFormat& rAutoFormat,
                                    sal_uInt16 nCols, sal_uInt8 nId )
{
    if( !rBoxFormatArr[nId] )
    {
        SwTableBoxFormat* pBoxFormat = rDoc.MakeTableBoxFormat();
        rAutoFormat.UpdateToSet( nId, (SfxItemSet&)pBoxFormat->GetAttrSet(),
                                SwTableAutoFormat::UPDATE_BOX,
                                rDoc.GetNumberFormatter( ) );
        if( USHRT_MAX != nCols )
            pBoxFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        rBoxFormatArr[ nId ] = pBoxFormat;
    }
    return rBoxFormatArr[nId];
}

SwTableNode* SwDoc::IsIdxInTable(const SwNodeIndex& rIdx)
{
    SwTableNode* pTableNd = 0;
    sal_uLong nIndex = rIdx.GetIndex();
    do {
        SwNode* pNd = static_cast<SwNode*>(GetNodes()[ nIndex ]->StartOfSectionNode());
        if( 0 != ( pTableNd = pNd->GetTableNode() ) )
            break;

        nIndex = pNd->GetIndex();
    } while ( nIndex );
    return pTableNd;
}

/**
 * Insert a new Box before the InsPos
 */
bool SwNodes::InsBoxen( SwTableNode* pTableNd,
                        SwTableLine* pLine,
                        SwTableBoxFormat* pBoxFormat,
                        SwTextFormatColl* pTextColl,
                        const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos,
                        sal_uInt16 nCnt )
{
    if( !nCnt )
        return false;
    OSL_ENSURE( pLine, "No valid Line" );

    // Move Index after the Line's last Box
    sal_uLong nIdxPos = 0;
    SwTableBox *pPrvBox = 0, *pNxtBox = 0;
    if( !pLine->GetTabBoxes().empty() )
    {
        if( nInsPos < pLine->GetTabBoxes().size() )
        {
            if( 0 == (pPrvBox = pLine->FindPreviousBox( pTableNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos ] )))
                pPrvBox = pLine->FindPreviousBox( pTableNd->GetTable() );
        }
        else
        {
            if( 0 == (pNxtBox = pLine->FindNextBox( pTableNd->GetTable(),
                            pLine->GetTabBoxes().back() )))
                pNxtBox = pLine->FindNextBox( pTableNd->GetTable() );
        }
    }
    else if( 0 == ( pNxtBox = pLine->FindNextBox( pTableNd->GetTable() )))
        pPrvBox = pLine->FindPreviousBox( pTableNd->GetTable() );

    if( !pPrvBox && !pNxtBox )
    {
        bool bSetIdxPos = true;
        if( !pTableNd->GetTable().GetTabLines().empty() && !nInsPos )
        {
            const SwTableLine* pTableLn = pLine;
            while( pTableLn->GetUpper() )
                pTableLn = pTableLn->GetUpper()->GetUpper();

            if( pTableNd->GetTable().GetTabLines()[ 0 ] == pTableLn )
            {
                // Before the Table's first Box
                while( !( pNxtBox = pLine->GetTabBoxes()[0])->GetTabLines().empty() )
                    pLine = pNxtBox->GetTabLines()[0];
                nIdxPos = pNxtBox->GetSttIdx();
                bSetIdxPos = false;
            }
        }
        if( bSetIdxPos )
            // Tables without content or at the end; move before the End
            nIdxPos = pTableNd->EndOfSectionIndex();
    }
    else if( pNxtBox ) // There is a successor
        nIdxPos = pNxtBox->GetSttIdx();
    else // There is a predecessor
        nIdxPos = pPrvBox->GetSttNd()->EndOfSectionIndex() + 1;

    SwNodeIndex aEndIdx( *this, nIdxPos );
    for( sal_uInt16 n = 0; n < nCnt; ++n )
    {
        SwStartNode* pSttNd = new SwStartNode( aEndIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
        pSttNd->pStartOfSection = pTableNd;
        new SwEndNode( aEndIdx, *pSttNd );

        pPrvBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );

        SwTableBoxes & rTabBoxes = pLine->GetTabBoxes();
        sal_uInt16 nRealInsPos = nInsPos + n;
        if (nRealInsPos > rTabBoxes.size())
            nRealInsPos = rTabBoxes.size();

        rTabBoxes.insert( rTabBoxes.begin() + nRealInsPos, pPrvBox );

        if( ! pTextColl->IsAssignedToListLevelOfOutlineStyle()
//FEATURE::CONDCOLL
            && RES_CONDTXTFMTCOLL != pTextColl->Which()
//FEATURE::CONDCOLL
        )
            new SwTextNode( SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                                pTextColl, pAutoAttr );
        else
        {
            // Handle Outline numbering correctly!
            SwTextNode* pTNd = new SwTextNode(
                            SwNodeIndex( *pSttNd->EndOfSectionNode() ),
                            GetDoc()->GetDfltTextFormatColl(),
                            pAutoAttr );
            pTNd->ChgFormatColl( pTextColl );
        }
    }
    return true;
}

/**
 * Insert a new Table
 */
const SwTable* SwDoc::InsertTable( const SwInsertTableOptions& rInsTableOpts,
                                   const SwPosition& rPos, sal_uInt16 nRows,
                                   sal_uInt16 nCols, sal_Int16 eAdjust,
                                   const SwTableAutoFormat* pTAFormat,
                                   const std::vector<sal_uInt16> *pColArr,
                                   bool bCalledFromShell,
                                   bool bNewModel )
{
    OSL_ENSURE( nRows, "Table without line?" );
    OSL_ENSURE( nCols, "Table without rows?" );

    {
        // Do not copy into Footnotes!
        if( rPos.nNode < GetNodes().GetEndOfInserts().GetIndex() &&
            rPos.nNode >= GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;

        // If the ColumnArray has a wrong count, ignore it!
        if( pColArr &&
            (size_t)(nCols + ( text::HoriOrientation::NONE == eAdjust ? 2 : 1 )) != pColArr->size() )
            pColArr = 0;
    }

    OUString aTableName = GetUniqueTableName();

    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoInsTable( rPos, nCols, nRows, static_cast<sal_uInt16>(eAdjust),
                                      rInsTableOpts, pTAFormat, pColArr,
                                      aTableName));
    }

    // Start with inserting the Nodes and get the AutoFormat for the Table
    SwTextFormatColl *pBodyColl = getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE ),
                 *pHeadColl = pBodyColl;

    bool bDfltBorders = 0 != ( rInsTableOpts.mnInsMode & tabopts::DEFAULT_BORDER );

    if( (rInsTableOpts.mnInsMode & tabopts::HEADLINE) && (1 != nRows || !bDfltBorders) )
        pHeadColl = getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE_HDLN );

    const sal_uInt16 nRowsToRepeat =
            tabopts::HEADLINE == (rInsTableOpts.mnInsMode & tabopts::HEADLINE) ?
            rInsTableOpts.mnRowsToRepeat :
            0;

    /* Save content node to extract FRAMEDIR from. */
    const SwContentNode * pContentNd = rPos.nNode.GetNode().GetContentNode();

    /* If we are called from a shell pass the attrset from
        pContentNd (aka the node the table is inserted at) thus causing
        SwNodes::InsertTable to propagate an adjust item if
        necessary. */
    SwTableNode *pTableNd = SwNodes::InsertTable(
        rPos.nNode,
        nCols,
        pBodyColl,
        nRows,
        nRowsToRepeat,
        pHeadColl,
        bCalledFromShell ? &pContentNd->GetSwAttrSet() : 0 );

    // Create the Box/Line/Table construct
    SwTableLineFormat* pLineFormat = MakeTableLineFormat();
    SwTableFormat* pTableFormat = MakeTableFrameFormat( aTableName, GetDfltFrameFormat() );

    /* If the node to insert the table at is a context node and has a
       non-default FRAMEDIR propagate it to the table. */
    if (pContentNd)
    {
        const SwAttrSet & aNdSet = pContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFormat->SetFormatAttr( *pItem );
        }
    }

    // Set Orientation at the Table's Format
    pTableFormat->SetFormatAttr( SwFormatHoriOrient( 0, eAdjust ) );
    // All lines use the left-to-right Fill-Order!
    pLineFormat->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ));

    // Set USHRT_MAX as the Table's default SSize
    SwTwips nWidth = USHRT_MAX;
    if( pColArr )
    {
        sal_uInt16 nSttPos = pColArr->front();
        sal_uInt16 nLastPos = pColArr->back();
        if( text::HoriOrientation::NONE == eAdjust )
        {
            sal_uInt16 nFrmWidth = nLastPos;
            nLastPos = (*pColArr)[ pColArr->size()-2 ];
            pTableFormat->SetFormatAttr( SvxLRSpaceItem( nSttPos, nFrmWidth - nLastPos, 0, 0, RES_LR_SPACE ) );
        }
        nWidth = nLastPos - nSttPos;
    }
    else if( nCols )
    {
        nWidth /= nCols;
        nWidth *= nCols; // to avoid rounding problems
    }
    pTableFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, nWidth ));
    if( !(rInsTableOpts.mnInsMode & tabopts::SPLIT_LAYOUT) )
        pTableFormat->SetFormatAttr( SwFormatLayoutSplit( false ));

    // Move the hard PageDesc/PageBreak Attributes if needed
    SwContentNode* pNextNd = GetNodes()[ pTableNd->EndOfSectionIndex()+1 ]
                            ->GetContentNode();
    if( pNextNd && pNextNd->HasSwAttrSet() )
    {
        const SfxItemSet* pNdSet = pNextNd->GetpSwAttrSet();
        const SfxPoolItem *pItem;
        if( SfxItemState::SET == pNdSet->GetItemState( RES_PAGEDESC, false,
            &pItem ) )
        {
            pTableFormat->SetFormatAttr( *pItem );
            pNextNd->ResetAttr( RES_PAGEDESC );
            pNdSet = pNextNd->GetpSwAttrSet();
        }
        if( pNdSet && SfxItemState::SET == pNdSet->GetItemState( RES_BREAK, false,
             &pItem ) )
        {
            pTableFormat->SetFormatAttr( *pItem );
            pNextNd->ResetAttr( RES_BREAK );
        }
    }

    SwTable& rNdTable = pTableNd->GetTable();
    rNdTable.RegisterToFormat( *pTableFormat );

    rNdTable.SetRowsToRepeat( nRowsToRepeat );
    rNdTable.SetTableModel( bNewModel );

    std::vector<SwTableBoxFormat*> aBoxFormatArr;
    SwTableBoxFormat* pBoxFormat = 0;
    if( !bDfltBorders && !pTAFormat )
    {
        pBoxFormat = MakeTableBoxFormat();
        pBoxFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, USHRT_MAX / nCols, 0 ));
    }
    else
    {
        const sal_uInt16 nBoxArrLen = pTAFormat ? 16 : 4;
        aBoxFormatArr.resize( nBoxArrLen, NULL );
    }
    SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1 );

    SwNodeIndex aNdIdx( *pTableNd, 1 ); // Set to StartNode of first Box
    SwTableLines& rLines = rNdTable.GetTabLines();
    for( sal_uInt16 n = 0; n < nRows; ++n )
    {
        SwTableLine* pLine = new SwTableLine( pLineFormat, nCols, 0 );
        rLines.insert( rLines.begin() + n, pLine );
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 i = 0; i < nCols; ++i )
        {
            SwTableBoxFormat *pBoxF;
            if( pTAFormat )
            {
                sal_uInt8 nId = static_cast<sal_uInt8>(!n ? 0 : (( n+1 == nRows )
                                        ? 12 : (4 * (1 + ((n-1) & 1 )))));
                nId = nId + static_cast<sal_uInt8>( !i ? 0 :
                            ( i+1 == nCols ? 3 : (1 + ((i-1) & 1))));
                pBoxF = ::lcl_CreateAFormatBoxFormat( *this, aBoxFormatArr, *pTAFormat,
                                                nCols, nId );

                // Set the Paragraph/Character Attributes if needed
                if( pTAFormat->IsFont() || pTAFormat->IsJustify() )
                {
                    aCharSet.ClearItem();
                    pTAFormat->UpdateToSet( nId, aCharSet,
                                        SwTableAutoFormat::UPDATE_CHAR, 0 );
                    if( aCharSet.Count() )
                        GetNodes()[ aNdIdx.GetIndex()+1 ]->GetContentNode()->
                            SetAttr( aCharSet );
                }
            }
            else if( bDfltBorders )
            {
                sal_uInt8 nBoxId = (i < nCols - 1 ? 0 : 1) + (n ? 2 : 0 );
                pBoxF = ::lcl_CreateDfltBoxFormat( *this, aBoxFormatArr, nCols, nBoxId);
            }
            else
                pBoxF = pBoxFormat;

            // For AutoFormat on input: the columns are set when inserting the Table
            // The Array contains the columns positions and not their widths!
            if( pColArr )
            {
                nWidth = (*pColArr)[ i + 1 ] - (*pColArr)[ i ];
                if( pBoxF->GetFrmSize().GetWidth() != nWidth )
                {
                    if( pBoxF->HasWriterListeners() ) // Create new Format
                    {
                        SwTableBoxFormat *pNewFormat = MakeTableBoxFormat();
                        *pNewFormat = *pBoxF;
                        pBoxF = pNewFormat;
                    }
                    pBoxF->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, nWidth ));
                }
            }

            SwTableBox *pBox = new SwTableBox( pBoxF, aNdIdx, pLine);
            rBoxes.insert( rBoxes.begin() + i, pBox );
            aNdIdx += 3; // StartNode, TextNode, EndNode  == 3 Nodes
        }
    }
    // Insert Frms
    GetNodes().GoNext( &aNdIdx ); // Go to the next ContentNode
    pTableNd->MakeFrms( &aNdIdx );

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( getIDocumentRedlineAccess().IsRedlineOn() || (!getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( *pTableNd->EndOfSectionNode(), *pTableNd, 1 );
        if( getIDocumentRedlineAccess().IsRedlineOn() )
            getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        else
            getIDocumentRedlineAccess().SplitRedline( aPam );
    }

    getIDocumentState().SetModified();
    CHECK_TABLE(rNdTable);
    return &rNdTable;
}

SwTableNode* SwNodes::InsertTable( const SwNodeIndex& rNdIdx,
                                   sal_uInt16 nBoxes,
                                   SwTextFormatColl* pContentTextColl,
                                   sal_uInt16 nLines,
                                   sal_uInt16 nRepeat,
                                   SwTextFormatColl* pHeadlineTextColl,
                                   const SwAttrSet * pAttrSet)
{
    if( !nBoxes )
        return 0;

    // If Lines is given, create the Matrix from Lines and Boxes
    if( !pHeadlineTextColl || !nLines )
        pHeadlineTextColl = pContentTextColl;

    SwTableNode * pTableNd = new SwTableNode( rNdIdx );
    SwEndNode* pEndNd = new SwEndNode( rNdIdx, *pTableNd );

    if( !nLines ) // For the for loop
        ++nLines;

    SwNodeIndex aIdx( *pEndNd );
    SwTextFormatColl* pTextColl = pHeadlineTextColl;
    for( sal_uInt16 nL = 0; nL < nLines; ++nL )
    {
        for( sal_uInt16 nB = 0; nB < nBoxes; ++nB )
        {
            SwStartNode* pSttNd = new SwStartNode( aIdx, ND_STARTNODE,
                                                    SwTableBoxStartNode );
            pSttNd->pStartOfSection = pTableNd;

            SwTextNode * pTmpNd = new SwTextNode( aIdx, pTextColl );

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
                    if ( SfxItemState::SET != pTmpNd->GetSwAttrSet().GetItemState( *pIdx ) &&
                         SfxItemState::SET == pAttrSet->GetItemState( *pIdx, true, &pItem ) )
                        static_cast<SwContentNode *>(pTmpNd)->SetAttr(*pItem);
                    ++pIdx;
                }
            }

            new SwEndNode( aIdx, *pSttNd );
        }
        if ( nL + 1 >= nRepeat )
            pTextColl = pContentTextColl;
    }
    return pTableNd;
}

/**
 * Text to Table
 */
const SwTable* SwDoc::TextToTable( const SwInsertTableOptions& rInsTableOpts,
                                   const SwPaM& rRange, sal_Unicode cCh,
                                   sal_Int16 eAdjust,
                                   const SwTableAutoFormat* pTAFormat )
{
    // See if the selection contains a Table
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    {
        sal_uLong nCnt = pStt->nNode.GetIndex();
        for( ; nCnt <= pEnd->nNode.GetIndex(); ++nCnt )
            if( !GetNodes()[ nCnt ]->IsTextNode() )
                return 0;
    }

    // Save first node in the selection if it is a context node
    SwContentNode * pSttContentNd = pStt->nNode.GetNode().GetContentNode();

    SwPaM aOriginal( *pStt, *pEnd );
    pStt = aOriginal.GetMark();
    pEnd = aOriginal.GetPoint();

    SwUndoTextToTable* pUndo = 0;
    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_TEXTTOTABLE, NULL );
        pUndo = new SwUndoTextToTable( aOriginal, rInsTableOpts, cCh,
                    static_cast<sal_uInt16>(eAdjust), pTAFormat );
        GetIDocumentUndoRedo().AppendUndo( pUndo );

        // Do not add splitting the TextNode to the Undo history
        GetIDocumentUndoRedo().DoUndo( false );
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    // Make sure that the range is on Node Edges
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        getIDocumentContentOperations().SplitNode( *pStt, false );

    bool bEndContent = 0 != pEnd->nContent.GetIndex();

    // Do not split at the End of a Line (except at the End of the Doc)
    if( bEndContent )
    {
        if( pEnd->nNode.GetNode().GetContentNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            getIDocumentContentOperations().SplitNode( *pEnd, false );
            --((SwNodeIndex&)pEnd->nNode);
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetContentNode(), 0 );
            // A Node and at the End?
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                --aRg.aStart;
        }
        else
            ++aRg.aEnd;
    }

    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "empty range" );
        ++aRg.aEnd;
    }

    // We always use Upper to insert the Table
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo( 0 != pUndo );

    // Create the Box/Line/Table construct
    SwTableBoxFormat* pBoxFormat = MakeTableBoxFormat();
    SwTableLineFormat* pLineFormat = MakeTableLineFormat();
    SwTableFormat* pTableFormat = MakeTableFrameFormat( GetUniqueTableName(), GetDfltFrameFormat() );

    // All Lines have a left-to-right Fill Order
    pLineFormat->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ));
    // The Table's SSize is USHRT_MAX
    pTableFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, USHRT_MAX ));
    if( !(rInsTableOpts.mnInsMode & tabopts::SPLIT_LAYOUT) )
        pTableFormat->SetFormatAttr( SwFormatLayoutSplit( false ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttContentNd)
    {
        const SwAttrSet & aNdSet = pSttContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFormat->SetFormatAttr( *pItem );
        }
    }

    SwTableNode* pTableNd = GetNodes().TextToTable(
            aRg, cCh, pTableFormat, pLineFormat, pBoxFormat,
            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ), pUndo );

    SwTable& rNdTable = pTableNd->GetTable();

    const sal_uInt16 nRowsToRepeat =
            tabopts::HEADLINE == (rInsTableOpts.mnInsMode & tabopts::HEADLINE) ?
            rInsTableOpts.mnRowsToRepeat :
            0;
    rNdTable.SetRowsToRepeat(nRowsToRepeat);

    bool bUseBoxFormat = false;
    if( !pBoxFormat->HasWriterListeners() )
    {
        // The Box's Formats already have the right size, we must only set
        // the right Border/AutoFormat.
        bUseBoxFormat = true;
        pTableFormat->SetFormatAttr( pBoxFormat->GetFrmSize() );
        delete pBoxFormat;
        eAdjust = text::HoriOrientation::NONE;
    }

    // Set Orientation in the Table's Format
    pTableFormat->SetFormatAttr( SwFormatHoriOrient( 0, eAdjust ) );

    if( pTAFormat || ( rInsTableOpts.mnInsMode & tabopts::DEFAULT_BORDER) )
    {
        sal_uInt8 nBoxArrLen = pTAFormat ? 16 : 4;
        std::unique_ptr< DfltBoxAttrList_t > aBoxFormatArr1;
        std::unique_ptr< std::vector<SwTableBoxFormat*> > aBoxFormatArr2;
        if( bUseBoxFormat )
        {
            aBoxFormatArr1.reset(new DfltBoxAttrList_t( nBoxArrLen, NULL ));
        }
        else
        {
            aBoxFormatArr2.reset(new std::vector<SwTableBoxFormat*>( nBoxArrLen, NULL ));
        }

        SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1 );

        SwHistory* pHistory = pUndo ? &pUndo->GetHistory() : 0;

        SwTableBoxFormat *pBoxF = 0;
        SwTableLines& rLines = rNdTable.GetTabLines();
        const SwTableLines::size_type nRows = rLines.size();
        for( SwTableLines::size_type n = 0; n < nRows; ++n )
        {
            SwTableBoxes& rBoxes = rLines[ n ]->GetTabBoxes();
            const SwTableBoxes::size_type nCols = rBoxes.size();
            for( SwTableBoxes::size_type i = 0; i < nCols; ++i )
            {
                SwTableBox* pBox = rBoxes[ i ];
                bool bChgSz = false;

                if( pTAFormat )
                {
                    sal_uInt8 nId = static_cast<sal_uInt8>(!n ? 0 : (( n+1 == nRows )
                                            ? 12 : (4 * (1 + ((n-1) & 1 )))));
                    nId = nId + static_cast<sal_uInt8>(!i ? 0 :
                                ( i+1 == nCols ? 3 : (1 + ((i-1) & 1))));
                    if( bUseBoxFormat )
                        ::lcl_SetDfltBoxAttr( *pBox, *aBoxFormatArr1, nId, pTAFormat );
                    else
                    {
                        bChgSz = 0 == (*aBoxFormatArr2)[ nId ];
                        pBoxF = ::lcl_CreateAFormatBoxFormat( *this, *aBoxFormatArr2,
                                                *pTAFormat, USHRT_MAX, nId );
                    }

                    // Set Paragraph/Character Attributes if needed
                    if( pTAFormat->IsFont() || pTAFormat->IsJustify() )
                    {
                        aCharSet.ClearItem();
                        pTAFormat->UpdateToSet( nId, aCharSet,
                                            SwTableAutoFormat::UPDATE_CHAR, 0 );
                        if( aCharSet.Count() )
                        {
                            sal_uLong nSttNd = pBox->GetSttIdx()+1;
                            sal_uLong nEndNd = pBox->GetSttNd()->EndOfSectionIndex();
                            for( ; nSttNd < nEndNd; ++nSttNd )
                            {
                                SwContentNode* pNd = GetNodes()[ nSttNd ]->GetContentNode();
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
                    if( bUseBoxFormat )
                        ::lcl_SetDfltBoxAttr( *pBox, *aBoxFormatArr1, nId );
                    else
                    {
                        bChgSz = 0 == (*aBoxFormatArr2)[ nId ];
                        pBoxF = ::lcl_CreateDfltBoxFormat( *this, *aBoxFormatArr2,
                                                        USHRT_MAX, nId );
                    }
                }

                if( !bUseBoxFormat )
                {
                    if( bChgSz )
                        pBoxF->SetFormatAttr( pBox->GetFrameFormat()->GetFrmSize() );
                    pBox->ChgFrameFormat( pBoxF );
                }
            }
        }

        if( bUseBoxFormat )
        {
            for( sal_uInt8 i = 0; i < nBoxArrLen; ++i )
            {
                delete (*aBoxFormatArr1)[ i ];
            }
        }
    }

    // Check the boxes for numbers
    if( IsInsTableFormatNum() )
    {
        for (size_t nBoxes = rNdTable.GetTabSortBoxes().size(); nBoxes; )
        {
            ChkBoxNumFormat(*rNdTable.GetTabSortBoxes()[ --nBoxes ], false);
        }
    }

    sal_uLong nIdx = pTableNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    {
        SwPaM& rTmp = (SwPaM&)rRange; // Point always at the Start
        rTmp.DeleteMark();
        rTmp.GetPoint()->nNode = *pTableNd;
        SwContentNode* pCNd = GetNodes().GoNext( &rTmp.GetPoint()->nNode );
        rTmp.GetPoint()->nContent.Assign( pCNd, 0 );
    }

    if( pUndo )
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_TEXTTOTABLE, NULL );
    }

    getIDocumentState().SetModified();
    getIDocumentFieldsAccess().SetFieldsDirty(true, NULL, 0);
    return &rNdTable;
}

static void lcl_RemoveBreaks(SwContentNode & rNode, SwTableFormat *const pTableFormat)
{
    // delete old layout frames, new ones need to be created...
    rNode.DelFrms();

    if (!rNode.IsTextNode())
    {
        return;
    }

    SwTextNode & rTextNode = *rNode.GetTextNode();
    // remove PageBreaks/PageDesc/ColBreak
    SfxItemSet const* pSet = rTextNode.GetpSwAttrSet();
    if (pSet)
    {
        const SfxPoolItem* pItem;
        if (SfxItemState::SET == pSet->GetItemState(RES_BREAK, false, &pItem))
        {
            if (pTableFormat)
            {
                pTableFormat->SetFormatAttr(*pItem);
            }
            rTextNode.ResetAttr(RES_BREAK);
            pSet = rTextNode.GetpSwAttrSet();
        }

        if (pSet
            && (SfxItemState::SET == pSet->GetItemState(RES_PAGEDESC, false, &pItem))
            && static_cast<SwFormatPageDesc const*>(pItem)->GetPageDesc())
        {
            if (pTableFormat)
            {
                pTableFormat->SetFormatAttr(*pItem);
            }
            rTextNode.ResetAttr(RES_PAGEDESC);
        }
    }
}

/**
 * balance lines in table, insert empty boxes so all lines have the size
 */
static void
lcl_BalanceTable(SwTable & rTable, size_t const nMaxBoxes,
    SwTableNode & rTableNd, SwTableBoxFormat & rBoxFormat, SwTextFormatColl & rTextColl,
    SwUndoTextToTable *const pUndo, std::vector<sal_uInt16> *const pPositions)
{
    for (size_t n = 0; n < rTable.GetTabLines().size(); ++n)
    {
        SwTableLine *const pCurrLine = rTable.GetTabLines()[ n ];
        size_t const nBoxes = pCurrLine->GetTabBoxes().size();
        if (nMaxBoxes != nBoxes)
        {
            rTableNd.GetNodes().InsBoxen(&rTableNd, pCurrLine, &rBoxFormat, &rTextColl,
                    0, nBoxes, nMaxBoxes - nBoxes);

            if (pUndo)
            {
                for (size_t i = nBoxes; i < nMaxBoxes; ++i)
                {
                    pUndo->AddFillBox( *pCurrLine->GetTabBoxes()[i] );
                }
            }

            // if the first line is missing boxes, the width array is useless!
            if (!n && pPositions)
            {
                pPositions->clear();
            }
        }
    }
}

static void
lcl_SetTableBoxWidths(SwTable & rTable, size_t const nMaxBoxes,
        SwTableBoxFormat & rBoxFormat, SwDoc & rDoc,
        std::vector<sal_uInt16> *const pPositions)
{
    if (pPositions && !pPositions->empty())
    {
        SwTableLines& rLns = rTable.GetTabLines();
        sal_uInt16 nLastPos = 0;
        for (size_t n = 0; n < pPositions->size(); ++n)
        {
            SwTableBoxFormat *pNewFormat = rDoc.MakeTableBoxFormat();
            pNewFormat->SetFormatAttr(
                    SwFormatFrmSize(ATT_VAR_SIZE, (*pPositions)[n] - nLastPos));
            for (size_t nTmpLine = 0; nTmpLine < rLns.size(); ++nTmpLine)
            {
                // Have to do an Add here, because the BoxFormat
                // is still needed by the caller
                pNewFormat->Add( rLns[ nTmpLine ]->GetTabBoxes()[ n ] );
            }

            nLastPos = (*pPositions)[ n ];
        }

        // propagate size upwards from format, so the table gets the right size
        SAL_WARN_IF(rBoxFormat.HasWriterListeners(), "sw.core",
                "who is still registered in the format?");
        rBoxFormat.SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, nLastPos ));
    }
    else
    {
        size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
        rBoxFormat.SetFormatAttr(SwFormatFrmSize(ATT_VAR_SIZE, nWidth));
    }
}

SwTableNode* SwNodes::TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                    SwTableFormat* pTableFormat,
                                    SwTableLineFormat* pLineFormat,
                                    SwTableBoxFormat* pBoxFormat,
                                    SwTextFormatColl* pTextColl,
                                    SwUndoTextToTable* pUndo )
{
    if( rRange.aStart >= rRange.aEnd )
        return 0;

    SwTableNode * pTableNd = new SwTableNode( rRange.aStart );
    new SwEndNode( rRange.aEnd, *pTableNd );

    SwDoc* pDoc = GetDoc();
    std::vector<sal_uInt16> aPosArr;
    SwTable& rTable = pTableNd->GetTable();
    SwTableBox* pBox;
    sal_uInt16 nBoxes, nLines, nMaxBoxes = 0;

    SwNodeIndex aSttIdx( *pTableNd, 1 );
    SwNodeIndex aEndIdx( rRange.aEnd, -1 );
    for( nLines = 0, nBoxes = 0;
        aSttIdx.GetIndex() < aEndIdx.GetIndex();
        aSttIdx += 2, nLines++, nBoxes = 0 )
    {
        SwTextNode* pTextNd = aSttIdx.GetNode().GetTextNode();
        OSL_ENSURE( pTextNd, "Only add TextNodes to the Table" );

        if( !nLines && 0x0b == cCh )
        {
            cCh = 0x09;

            // Get the separator's position from the first Node, in order for the Boxes to be set accordingly
            SwTextFrmInfo aFInfo( static_cast<SwTextFrm*>(pTextNd->getLayoutFrm( pTextNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() )) );
            if( aFInfo.IsOneLine() ) // only makes sense in this case
            {
                OUString const& rText(pTextNd->GetText());
                for (sal_Int32 nChPos = 0; nChPos < rText.getLength(); ++nChPos)
                {
                    if (rText[nChPos] == cCh)
                    {
                        aPosArr.push_back( static_cast<sal_uInt16>(
                                        aFInfo.GetCharPos( nChPos+1, false )) );
                    }
                }

                aPosArr.push_back(
                                static_cast<sal_uInt16>(aFInfo.GetFrm()->IsVertical() ?
                                aFInfo.GetFrm()->Prt().Bottom() :
                                aFInfo.GetFrm()->Prt().Right()) );

            }
        }

        lcl_RemoveBreaks(*pTextNd, (0 == nLines) ? pTableFormat : 0);

        // Set the TableNode as StartNode for all TextNodes in the Table
        pTextNd->pStartOfSection = pTableNd;

        SwTableLine* pLine = new SwTableLine( pLineFormat, 1, 0 );
        rTable.GetTabLines().insert(rTable.GetTabLines().begin() + nLines, pLine);

        SwStartNode* pSttNd;
        SwPosition aCntPos( aSttIdx, SwIndex( pTextNd ));

        const std::shared_ptr< sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save( pDoc, aSttIdx.GetIndex(), pTextNd->GetText().getLength() );

        if( T2T_PARA != cCh )
        {
            for (sal_Int32 nChPos = 0; nChPos < pTextNd->GetText().getLength();)
            {
                if (pTextNd->GetText()[nChPos] == cCh)
                {
                    aCntPos.nContent = nChPos;
                    SwContentNode* pNewNd = pTextNd->SplitContentNode( aCntPos );

                    if( !pContentStore->Empty() )
                        pContentStore->Restore( *pNewNd, nChPos, nChPos + 1 );

                    // Delete separator and correct search string
                    pTextNd->EraseText( aCntPos.nContent, 1 );
                    nChPos = 0;

                    // Set the TableNode as StartNode for all TextNodes in the Table
                    const SwNodeIndex aTmpIdx( aCntPos.nNode, -1 );
                    pSttNd = new SwStartNode( aTmpIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
                    new SwEndNode( aCntPos.nNode, *pSttNd );
                    pNewNd->pStartOfSection = pSttNd;

                    // Assign Section to the Box
                    pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
                    pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
                }
                else
                {
                    ++nChPos;
                }
            }
        }

        // Now for the last substring
        if( !pContentStore->Empty())
            pContentStore->Restore( *pTextNd, pTextNd->GetText().getLength(), pTextNd->GetText().getLength()+1 );

        pSttNd = new SwStartNode( aCntPos.nNode, ND_STARTNODE, SwTableBoxStartNode );
        const SwNodeIndex aTmpIdx( aCntPos.nNode, 1 );
        new SwEndNode( aTmpIdx, *pSttNd  );
        pTextNd->pStartOfSection = pSttNd;

        pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
        pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    lcl_BalanceTable(rTable, nMaxBoxes, *pTableNd, *pBoxFormat, *pTextColl,
            pUndo, &aPosArr);
    rTable.RegisterToFormat(*pTableFormat);
    lcl_SetTableBoxWidths(rTable, nMaxBoxes, *pBoxFormat, *pDoc, &aPosArr);

    return pTableNd;
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
    SwContentNode * pSttContentNd = rFirstRange.begin()->aStart.GetNode().GetContentNode();

    const SwNodeRange& rStartRange = *rFirstRange.begin();
    const SwNodeRange& rEndRange = *rLastRange.rbegin();

    //!!! not necessarily TextNodes !!!
    SwPaM aOriginal( rStartRange.aStart, rEndRange.aEnd );
    const SwPosition *pStt = aOriginal.GetMark();
    const SwPosition *pEnd = aOriginal.GetPoint();

    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        // Do not add splitting the TextNode to the Undo history
        GetIDocumentUndoRedo().DoUndo(false);
    }

    ::PaMCorrAbs( aOriginal, *pEnd );

    // make sure that the range is on Node Edges
    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    if( pStt->nContent.GetIndex() )
        getIDocumentContentOperations().SplitNode( *pStt, false );

    bool bEndContent = 0 != pEnd->nContent.GetIndex();

    // Do not split at the End of a Line (except at the End of the Doc)
    if( bEndContent )
    {
        if( pEnd->nNode.GetNode().GetContentNode()->Len() != pEnd->nContent.GetIndex()
            || pEnd->nNode.GetIndex() >= GetNodes().GetEndOfContent().GetIndex()-1 )
        {
            getIDocumentContentOperations().SplitNode( *pEnd, false );
            --((SwNodeIndex&)pEnd->nNode);
            ((SwIndex&)pEnd->nContent).Assign(
                                pEnd->nNode.GetNode().GetContentNode(), 0 );
            // A Node and at the End?
            if( pStt->nNode.GetIndex() >= pEnd->nNode.GetIndex() )
                --aRg.aStart;
        }
        else
            ++aRg.aEnd;
    }

    assert(aRg.aEnd == pEnd->nNode);
    assert(aRg.aStart == pStt->nNode);
    if( aRg.aEnd.GetIndex() == aRg.aStart.GetIndex() )
    {
        OSL_FAIL( "empty range" );
        ++aRg.aEnd;
    }


    {
        // TODO: this is not Undo-able - only good enough for file import
        IDocumentRedlineAccess & rIDRA(getIDocumentRedlineAccess());
        SwNodeIndex const prev(rTableNodes.begin()->begin()->aStart, -1);
        SwNodeIndex const* pPrev(&prev);
        // pPrev could point to non-textnode now
        for (auto row = rTableNodes.begin(); row != rTableNodes.end(); ++row)
        {
            for (auto cell = row->begin(); cell != row->end(); ++cell)
            {
                assert(SwNodeIndex(*pPrev, +1) == cell->aStart);
                SwPaM pam(cell->aStart, 0, *pPrev,
                        (pPrev->GetNode().IsContentNode())
                            ? pPrev->GetNode().GetContentNode()->Len() : 0);
                rIDRA.SplitRedline(pam);
                pPrev = &cell->aEnd;
            }
        }
        // another one to break between last cell and node after table
        SwPaM pam(SwNodeIndex(*pPrev, +1), 0, *pPrev,
                    (pPrev->GetNode().IsContentNode())
                        ? pPrev->GetNode().GetContentNode()->Len() : 0);
        rIDRA.SplitRedline(pam);
    }

    // We always use Upper to insert the Table
    SwNode2Layout aNode2Layout( aRg.aStart.GetNode() );

    GetIDocumentUndoRedo().DoUndo(bUndo);

    // Create the Box/Line/Table construct
    SwTableBoxFormat* pBoxFormat = MakeTableBoxFormat();
    SwTableLineFormat* pLineFormat = MakeTableLineFormat();
    SwTableFormat* pTableFormat = MakeTableFrameFormat( GetUniqueTableName(), GetDfltFrameFormat() );

    // All Lines have a left-to-right Fill Order
    pLineFormat->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ));
    // The Table's SSize is USHRT_MAX
    pTableFormat->SetFormatAttr( SwFormatFrmSize( ATT_VAR_SIZE, USHRT_MAX ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttContentNd)
    {
        const SwAttrSet & aNdSet = pSttContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = NULL;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != NULL)
        {
            pTableFormat->SetFormatAttr( *pItem );
        }
    }

    SwTableNode* pTableNd = GetNodes().TextToTable(
            rTableNodes, pTableFormat, pLineFormat, pBoxFormat,
            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD )/*, pUndo*/ );

    SwTable& rNdTable = pTableNd->GetTable();

    if( !pBoxFormat->HasWriterListeners() )
    {
        // The Box's Formats already have the right size, we must only set
        // the right Border/AutoFormat.
        pTableFormat->SetFormatAttr( pBoxFormat->GetFrmSize() );
        delete pBoxFormat;
    }

    sal_uLong nIdx = pTableNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    getIDocumentState().SetModified();
    getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
    return &rNdTable;
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

static void
lcl_SetTableBoxWidths2(SwTable & rTable, size_t const nMaxBoxes,
        SwTableBoxFormat & rBoxFormat, SwDoc & rDoc)
{
    // rhbz#820283, fdo#55462: set default box widths so table width is covered
    SwTableLines & rLines = rTable.GetTabLines();
    for (size_t nTmpLine = 0; nTmpLine < rLines.size(); ++nTmpLine)
    {
        SwTableBoxes & rBoxes = rLines[nTmpLine]->GetTabBoxes();
        size_t const nMissing = nMaxBoxes - rBoxes.size();
        if (nMissing)
        {
            // default width for box at the end of an incomplete line
            SwTableBoxFormat *const pNewFormat = rDoc.MakeTableBoxFormat();
            size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
            pNewFormat->SetFormatAttr( SwFormatFrmSize(ATT_VAR_SIZE,
                        nWidth * (nMissing + 1)) );
            pNewFormat->Add(rBoxes.back());
        }
    }
    size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
    // default width for all boxes not at the end of an incomplete line
    rBoxFormat.SetFormatAttr(SwFormatFrmSize(ATT_VAR_SIZE, nWidth));
}

SwTableNode* SwNodes::TextToTable( const SwNodes::TableRanges_t & rTableNodes,
                                    SwTableFormat* pTableFormat,
                                    SwTableLineFormat* pLineFormat,
                                    SwTableBoxFormat* pBoxFormat,
                                    SwTextFormatColl* /*pTextColl*/  /*, SwUndo... pUndo*/  )
{
    if( rTableNodes.empty() )
        return 0;

    SwTableNode * pTableNd = new SwTableNode( rTableNodes.begin()->begin()->aStart );
    //insert the end node after the last text node
    SwNodeIndex aInsertIndex( rTableNodes.rbegin()->rbegin()->aEnd );
    ++aInsertIndex;

    //!! ownership will be transferred in c-tor to SwNodes array.
    //!! Thus no real problem here...
    new SwEndNode( aInsertIndex, *pTableNd );

#if OSL_DEBUG_LEVEL > 1
    const SwNodeRange& rStartRange = *rTableNodes.begin()->begin();
    const SwNodeRange& rEndRange = *rTableNodes.rbegin()->rbegin();
    (void) rStartRange;
    (void) rEndRange;
#endif

    SwDoc* pDoc = GetDoc();
    SwTable& rTable = pTableNd->GetTable();
    SwTableBox* pBox;
    sal_uInt16 nBoxes, nLines, nMaxBoxes = 0;

    SwNodeIndex aNodeIndex = rTableNodes.begin()->begin()->aStart;
    // delete frames of all contained content nodes
    for( nLines = 0; aNodeIndex <= rTableNodes.rbegin()->rbegin()->aEnd; ++aNodeIndex,++nLines )
    {
        SwNode& rNode = aNodeIndex.GetNode();
        if( rNode.IsContentNode() )
        {
            lcl_RemoveBreaks(static_cast<SwContentNode&>(rNode),
                    (0 == nLines) ? pTableFormat : 0);
        }
    }

    std::vector<std::vector < SwNodeRange > >::const_iterator aRowIter = rTableNodes.begin();
    for( nLines = 0, nBoxes = 0;
        aRowIter != rTableNodes.end();
        ++aRowIter, nLines++, nBoxes = 0 )
    {
        SwTableLine* pLine = new SwTableLine( pLineFormat, 1, 0 );
        rTable.GetTabLines().insert(rTable.GetTabLines().begin() + nLines, pLine);

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

                // assign Section to the Box
                pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
                pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        }
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    rTable.RegisterToFormat(*pTableFormat);
    lcl_SetTableBoxWidths2(rTable, nMaxBoxes, *pBoxFormat, *pDoc);

    return pTableNd;
}

/**
 * Table to Text
 */
bool SwDoc::TableToText( const SwTableNode* pTableNd, sal_Unicode cCh )
{
    if( !pTableNd )
        return false;

    // #i34471#
    // If this is trigged by SwUndoTableToText::Repeat() nobody ever deleted
    // the table cursor.
    SwEditShell* pESh = GetEditShell();
    if( pESh && pESh->IsTableMode() )
        pESh->ClearMark();

    SwNodeRange aRg( *pTableNd, 0, *pTableNd->EndOfSectionNode() );
    SwUndoTableToText* pUndo = 0;
    SwNodeRange* pUndoRg = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoRg = new SwNodeRange( aRg.aStart, -1, aRg.aEnd, +1 );
        pUndo = new SwUndoTableToText( pTableNd->GetTable(), cCh );
    }

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.eFlags = TBL_BOXNAME;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    bool bRet = GetNodes().TableToText( aRg, cCh, pUndo );
    if( pUndoRg )
    {
        ++pUndoRg->aStart;
        --pUndoRg->aEnd;
        pUndo->SetRange( *pUndoRg );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        delete pUndoRg;
    }

    if( bRet )
        getIDocumentState().SetModified();

    return bRet;
}

/**
 * Use the ForEach method from PtrArray to recreate Text from a Table.
 * The Boxes can also contain Lines!
 */
struct _DelTabPara
{
    SwTextNode* pLastNd;
    SwNodes& rNds;
    SwUndoTableToText* pUndo;
    sal_Unicode cCh;

    _DelTabPara( SwNodes& rNodes, sal_Unicode cChar, SwUndoTableToText* pU ) :
        pLastNd(0), rNds( rNodes ), pUndo( pU ), cCh( cChar ) {}
    _DelTabPara( const _DelTabPara& rPara ) :
        pLastNd(rPara.pLastNd), rNds( rPara.rNds ),
        pUndo( rPara.pUndo ), cCh( rPara.cCh ) {}
};

// Forward declare so that the Lines and Boxes can use recursion
static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara );

static void lcl_DelLine( SwTableLine* pLine, _DelTabPara* pPara )
{
    assert(pPara && "The parameters are missing!");
    _DelTabPara aPara( *pPara );
    for( SwTableBoxes::iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        lcl_DelBox(*it, &aPara );
    if( pLine->GetUpper() ) // Is there a parent Box?
        // Return the last TextNode
        pPara->pLastNd = aPara.pLastNd;
}

static void lcl_DelBox( SwTableBox* pBox, _DelTabPara* pDelPara )
{
    assert(pDelPara && "The parameters are missing");

    // Delete the Box's Lines
    if( !pBox->GetTabLines().empty() )
    {
        for( SwTableLine* pLine : pBox->GetTabLines() )
            lcl_DelLine( pLine, pDelPara );
    }
    else
    {
        SwDoc* pDoc = pDelPara->rNds.GetDoc();
        SwNodeRange aDelRg( *pBox->GetSttNd(), 0,
                            *pBox->GetSttNd()->EndOfSectionNode() );
        // Delete the Section
        pDelPara->rNds.SectionUp( &aDelRg );
        const SwTextNode* pCurTextNd;
        if( T2T_PARA != pDelPara->cCh && pDelPara->pLastNd &&
            0 != ( pCurTextNd = aDelRg.aStart.GetNode().GetTextNode() ))
        {
            // Join the current text node with the last from the previous box if possible
            sal_uLong nNdIdx = aDelRg.aStart.GetIndex();
            --aDelRg.aStart;
            if( pDelPara->pLastNd == &aDelRg.aStart.GetNode() )
            {
                // Inserting the separator
                SwIndex aCntIdx( pDelPara->pLastNd,
                        pDelPara->pLastNd->GetText().getLength());
                pDelPara->pLastNd->InsertText( OUString(pDelPara->cCh), aCntIdx,
                    SwInsertFlags::EMPTYEXPAND );
                if( pDelPara->pUndo )
                    pDelPara->pUndo->AddBoxPos( *pDoc, nNdIdx, aDelRg.aEnd.GetIndex(),
                                                aCntIdx.GetIndex() );

                const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
                const sal_Int32 nOldTextLen = aCntIdx.GetIndex();
                pContentStore->Save( pDoc, nNdIdx, pCurTextNd->GetText().getLength() );

                pDelPara->pLastNd->JoinNext();

                if( !pContentStore->Empty() )
                    pContentStore->Restore( pDoc, pDelPara->pLastNd->GetIndex(), nOldTextLen );
            }
            else if( pDelPara->pUndo )
            {
                ++aDelRg.aStart;
                pDelPara->pUndo->AddBoxPos( *pDoc, nNdIdx, aDelRg.aEnd.GetIndex() );
            }
        }
        else if( pDelPara->pUndo )
            pDelPara->pUndo->AddBoxPos( *pDoc, aDelRg.aStart.GetIndex(), aDelRg.aEnd.GetIndex() );
        --aDelRg.aEnd;
        pDelPara->pLastNd = aDelRg.aEnd.GetNode().GetTextNode();

        // Do not take over the NumberFormatting's adjustment
        if( pDelPara->pLastNd && pDelPara->pLastNd->HasSwAttrSet() )
            pDelPara->pLastNd->ResetAttr( RES_PARATR_ADJUST );
    }
}

bool SwNodes::TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                            SwUndoTableToText* pUndo )
{
    // Is a Table selected?
    SwTableNode* pTableNd;
    if( rRange.aStart.GetIndex() >= rRange.aEnd.GetIndex() ||
        0 == ( pTableNd = rRange.aStart.GetNode().GetTableNode()) ||
        &rRange.aEnd.GetNode() != pTableNd->EndOfSectionNode() )
        return false;

    // If the Table was alone in a Section, create the Frames via the Table's Upper
    SwNode2Layout* pNode2Layout = 0;
    SwNodeIndex aFrmIdx( rRange.aStart );
    SwNode* pFrmNd = FindPrvNxtFrmNode( aFrmIdx, &rRange.aEnd.GetNode() );
    if( !pFrmNd )
        // Collect all Uppers
        pNode2Layout = new SwNode2Layout( *pTableNd );

    // Delete the Frames
    pTableNd->DelFrms();

    // "Delete" the Table and merge all Lines/Boxes
    _DelTabPara aDelPara( *this, cCh, pUndo );
    for( SwTableLine *pLine : pTableNd->pTable->GetTabLines() )
        lcl_DelLine( pLine, &aDelPara );

    // We just created a TextNode with fitting separator for every TableLine.
    // Now we only need to delete the TableSection and create the Frames for the
    // new TextNode.
    SwNodeRange aDelRg( rRange.aStart, rRange.aEnd );

    // If the Table has PageDesc/Break Attributes, carry them over to the
    // first Text Node
    {
        // What about UNDO?
        const SfxItemSet& rTableSet = pTableNd->pTable->GetFrameFormat()->GetAttrSet();
        const SfxPoolItem *pBreak, *pDesc;
        if( SfxItemState::SET != rTableSet.GetItemState( RES_PAGEDESC, false, &pDesc ))
            pDesc = 0;
        if( SfxItemState::SET != rTableSet.GetItemState( RES_BREAK, false, &pBreak ))
            pBreak = 0;

        if( pBreak || pDesc )
        {
            SwNodeIndex aIdx( *pTableNd  );
            SwContentNode* pCNd = GoNext( &aIdx );
            if( pBreak )
                pCNd->SetAttr( *pBreak );
            if( pDesc )
                pCNd->SetAttr( *pDesc );
        }
    }

    SectionUp( &aDelRg ); // Delete this Section and by that the Table
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
        SwContentNode *pCNd;
        SwSectionNode *pSNd;
        while( aDelRg.aStart.GetIndex() < nEnd )
        {
            if( 0 != ( pCNd = aDelRg.aStart.GetNode().GetContentNode()))
            {
                if( pFrmNd->IsContentNode() )
                    static_cast<SwContentNode*>(pFrmNd)->MakeFrms( *pCNd );
                else if( pFrmNd->IsTableNode() )
                    static_cast<SwTableNode*>(pFrmNd)->MakeFrms( aDelRg.aStart );
                else if( pFrmNd->IsSectionNode() )
                    static_cast<SwSectionNode*>(pFrmNd)->MakeFrms( aDelRg.aStart );
                pFrmNd = pCNd;
            }
            else if( 0 != ( pSNd = aDelRg.aStart.GetNode().GetSectionNode()))
            {
                if( !pSNd->GetSection().IsHidden() && !pSNd->IsContentHidden() )
                {
                    pSNd->MakeFrms( &aFrmIdx, &aDelRg.aEnd );
                    pFrmNd = pSNd;
                    break;
                }
                aDelRg.aStart = *pSNd->EndOfSectionNode();
            }
            ++aDelRg.aStart;
        }
    }

    // #i28006# Fly frames have to be restored even if the table was
    // #alone in the section
    const SwFrameFormats& rFlyArr = *GetDoc()->GetSpzFrameFormats();
    for( auto pFly : rFlyArr )
    {
        SwFrameFormat *const pFormat = pFly;
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
        SwPosition const*const pAPos = rAnchor.GetContentAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
            nStt <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nEnd )
        {
            pFormat->MakeFrms();
        }
    }

    return true;
}

/**
 * Inserting Columns/Rows
 */
bool SwDoc::InsertCol( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    if( !::CheckSplitCells( rCursor, nCnt + 1, nsSwTableSearchType::TBLSEARCH_COL ) )
        return false;

    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    ::GetTableSel( rCursor, aBoxes, nsSwTableSearchType::TBLSEARCH_COL );

    bool bRet = false;
    if( !aBoxes.empty() )
        bRet = InsertCol( aBoxes, nCnt, bBehind );
    return bRet;
}

bool SwDoc::InsertCol( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    SwTable& rTable = pTableNd->GetTable();
    if( rTable.ISA( SwDDETable ))
        return false;

    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( UNDO_TABLE_INSCOL, rBoxes, *pTableNd,
                                     0, 0, nCnt, bBehind, false );
        aTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &rTable );
        aMsgHint.eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        bRet = rTable.InsertCol( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            getIDocumentState().SetModified();
            ::ClearFEShellTabCols();
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            pUndo->SaveNewBoxes( *pTableNd, aTmpLst );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }
    return bRet;
}

bool SwDoc::InsertRow( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, nsSwTableSearchType::TBLSEARCH_ROW );

    bool bRet = false;
    if( !aBoxes.empty() )
        bRet = InsertRow( aBoxes, nCnt, bBehind );
    return bRet;
}

bool SwDoc::InsertRow( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    SwTable& rTable = pTableNd->GetTable();
    if( rTable.ISA( SwDDETable ))
        return false;

    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( UNDO_TABLE_INSROW,rBoxes, *pTableNd,
                                     0, 0, nCnt, bBehind, false );
        aTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &rTable );
        aMsgHint.eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        bRet = rTable.InsertRow( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            getIDocumentState().SetModified();
            ::ClearFEShellTabCols();
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            pUndo->SaveNewBoxes( *pTableNd, aTmpLst );
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
bool SwDoc::DeleteRow( const SwCursor& rCursor )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, nsSwTableSearchType::TBLSEARCH_ROW );
    if( ::HasProtectedCells( aBoxes ))
        return false;

    // Remove the Crsr from the to-be-deleted Section.
    // The Cursor is placed after the table, except for
    //  - when there's another Line, we place it in that one
    //  - when a Line precedes it, we place it in that one
    {
        SwTableNode* pTableNd = rCursor.GetNode().FindTableNode();

        if( pTableNd->GetTable().ISA( SwDDETable ))
            return false;

        // Find all Boxes/Lines
        _FndBox aFndBox( 0, 0 );
        {
            _FndPara aPara( aBoxes, &aFndBox );
            ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
        }

        if( aFndBox.GetLines().empty() )
            return false;

        SwEditShell* pESh = GetEditShell();
        if( pESh )
        {
            pESh->KillPams();
            // FIXME: Actually we should be interating over all Shells!
        }

        _FndBox* pFndBox = &aFndBox;
        while( 1 == pFndBox->GetLines().size() &&
                1 == pFndBox->GetLines().front()->GetBoxes().size() )
        {
            _FndBox *const pTmp = pFndBox->GetLines().front()->GetBoxes()[0].get();
            if( pTmp->GetBox()->GetSttNd() )
                break; // Else it gets too far
            pFndBox = pTmp;
        }

        SwTableLine* pDelLine = pFndBox->GetLines().back()->GetLine();
        SwTableBox* pDelBox = pDelLine->GetTabBoxes().back();
        while( !pDelBox->GetSttNd() )
        {
            SwTableLine* pLn = pDelBox->GetTabLines()[
                        pDelBox->GetTabLines().size()-1 ];
            pDelBox = pLn->GetTabBoxes().back();
        }
        SwTableBox* pNextBox = pDelLine->FindNextBox( pTableNd->GetTable(),
                                                        pDelBox );
        while( pNextBox &&
                pNextBox->GetFrameFormat()->GetProtect().IsContentProtected() )
            pNextBox = pNextBox->FindNextBox( pTableNd->GetTable(), pNextBox );

        if( !pNextBox ) // No succeeding Boxes? Then take the preceding one
        {
            pDelLine = pFndBox->GetLines().front()->GetLine();
            pDelBox = pDelLine->GetTabBoxes()[ 0 ];
            while( !pDelBox->GetSttNd() )
                pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
            pNextBox = pDelLine->FindPreviousBox( pTableNd->GetTable(),
                                                        pDelBox );
            while( pNextBox &&
                    pNextBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                pNextBox = pNextBox->FindPreviousBox( pTableNd->GetTable(), pNextBox );
        }

        sal_uLong nIdx;
        if( pNextBox ) // Place the Cursor here
            nIdx = pNextBox->GetSttIdx() + 1;
        else // Else after the Table
            nIdx = pTableNd->EndOfSectionIndex() + 1;

        SwNodeIndex aIdx( GetNodes(), nIdx );
        SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
        if( !pCNd )
            pCNd = GetNodes().GoNext( &aIdx );

        if( pCNd )
        {
            // Change the Shell's Cursor or the one passed?
            SwPaM* pPam = const_cast<SwPaM*>(static_cast<SwPaM const *>(&rCursor));
            pPam->GetPoint()->nNode = aIdx;
            pPam->GetPoint()->nContent.Assign( pCNd, 0 );
            pPam->SetMark(); // Both want a part of it
            pPam->DeleteMark();
        }
    }

    // Thus delete the Rows
    GetIDocumentUndoRedo().StartUndo(UNDO_ROW_DELETE, NULL);
    bool bResult = DeleteRowCol( aBoxes );
    GetIDocumentUndoRedo().EndUndo(UNDO_ROW_DELETE, NULL);

    return bResult;
}

bool SwDoc::DeleteCol( const SwCursor& rCursor )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, nsSwTableSearchType::TBLSEARCH_COL );
    if( ::HasProtectedCells( aBoxes ))
        return false;

    // The Cursors need to be removed from the to-be-deleted range.
    // Always place them after/on top of the Table; they are always set
    // to the old position via the document position.
    SwEditShell* pESh = GetEditShell();
    if( pESh )
    {
        const SwNode* pNd = rCursor.GetNode().FindTableBoxStartNode();
        pESh->ParkCrsr( SwNodeIndex( *pNd ) );
    }

    // Thus delete the Columns
    GetIDocumentUndoRedo().StartUndo(UNDO_COL_DELETE, NULL);
    bool bResult = DeleteRowCol( aBoxes, true );
    GetIDocumentUndoRedo().EndUndo(UNDO_COL_DELETE, NULL);

    return bResult;
}

bool SwDoc::DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn )
{
    if( ::HasProtectedCells( rBoxes ))
        return false;

    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    if( pTableNd->GetTable().ISA( SwDDETable ))
        return false;

    ::ClearFEShellTabCols();
    SwSelBoxes aSelBoxes( rBoxes );
    SwTable &rTable = pTableNd->GetTable();
    long nMin = 0;
    long nMax = 0;
    if( rTable.IsNewModel() )
    {
        if( bColumn )
            rTable.ExpandColumnSelection( aSelBoxes, nMin, nMax );
        else
            rTable.FindSuperfluousRows( aSelBoxes );
    }

    // Are we deleting the whole Table?
    const sal_uLong nTmpIdx1 = pTableNd->GetIndex();
    const sal_uLong nTmpIdx2 = aSelBoxes.back()->GetSttNd()->EndOfSectionIndex() + 1;
    if( pTableNd->GetTable().GetTabSortBoxes().size() == aSelBoxes.size() &&
        aSelBoxes[0]->GetSttIdx()-1 == nTmpIdx1 &&
        nTmpIdx2 == pTableNd->EndOfSectionIndex() )
    {
        bool bNewTextNd = false;
        // Is it alone in a FlyFrame?
        SwNodeIndex aIdx( *pTableNd, -1 );
        const SwStartNode* pSttNd = aIdx.GetNode().GetStartNode();
        if( pSttNd )
        {
            const sal_uLong nTableEnd = pTableNd->EndOfSectionIndex() + 1;
            const sal_uLong nSectEnd = pSttNd->EndOfSectionIndex();
            if( nTableEnd == nSectEnd )
            {
                if( SwFlyStartNode == pSttNd->GetStartNodeType() )
                {
                    SwFrameFormat* pFormat = pSttNd->GetFlyFormat();
                    if( pFormat )
                    {
                        // That's the FlyFormat we're looking for
                        getIDocumentLayoutAccess().DelLayoutFormat( pFormat );
                        return true;
                    }
                }
                // No Fly? Thus Header or Footer: always leave a TextNode
                // We can forget about Undo then!
                bNewTextNd = true;
            }
        }

        // No Fly? Then it is a Header or Footer, so keep always a TextNode
        ++aIdx;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().ClearRedo();
            SwPaM aPaM( *pTableNd->EndOfSectionNode(), aIdx.GetNode() );

            if( bNewTextNd )
            {
                const SwNodeIndex aTmpIdx( *pTableNd->EndOfSectionNode(), 1 );
                GetNodes().MakeTextNode( aTmpIdx,
                            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            // Save the cursors (UNO and otherwise)
            SwPaM aSavePaM( SwNodeIndex( *pTableNd->EndOfSectionNode() ) );
            if( ! aSavePaM.Move( fnMoveForward, fnGoNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTableNd );
                aSavePaM.Move( fnMoveBackward, fnGoNode );
            }
            {
                SwPaM const tmpPaM(*pTableNd, *pTableNd->EndOfSectionNode());
                ::PaMCorrAbs(tmpPaM, *aSavePaM.GetMark());
            }

            // Move hard PageBreaks to the succeeding Node
            bool bSavePageBreak = false, bSavePageDesc = false;
            sal_uLong nNextNd = pTableNd->EndOfSectionIndex()+1;
            SwContentNode* pNextNd = GetNodes()[ nNextNd ]->GetContentNode();
            if( pNextNd )
            {
                {
                    SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();
                    const SfxPoolItem *pItem;
                    if( SfxItemState::SET == pTableFormat->GetItemState( RES_PAGEDESC,
                        false, &pItem ) )
                    {
                        pNextNd->SetAttr( *pItem );
                        bSavePageDesc = true;
                    }

                    if( SfxItemState::SET == pTableFormat->GetItemState( RES_BREAK,
                        false, &pItem ) )
                    {
                        pNextNd->SetAttr( *pItem );
                        bSavePageBreak = true;
                    }
                }
            }
            SwUndoDelete* pUndo = new SwUndoDelete( aPaM );
            if( bNewTextNd )
                pUndo->SetTableDelLastNd();
            pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
            pUndo->SetTableName(pTableNd->GetTable().GetFrameFormat()->GetName());
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
        {
            if( bNewTextNd )
            {
                const SwNodeIndex aTmpIdx( *pTableNd->EndOfSectionNode(), 1 );
                GetNodes().MakeTextNode( aTmpIdx,
                            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            // Save the cursors (UNO and otherwise)
            SwPaM aSavePaM( SwNodeIndex( *pTableNd->EndOfSectionNode() ) );
            if( ! aSavePaM.Move( fnMoveForward, fnGoNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTableNd );
                aSavePaM.Move( fnMoveBackward, fnGoNode );
            }
            {
                SwPaM const tmpPaM(*pTableNd, *pTableNd->EndOfSectionNode());
                ::PaMCorrAbs(tmpPaM, *aSavePaM.GetMark());
            }

            // Move hard PageBreaks to the succeeding Node
            SwContentNode* pNextNd = GetNodes()[ pTableNd->EndOfSectionIndex()+1 ]->GetContentNode();
            if( pNextNd )
            {
                SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();
                const SfxPoolItem *pItem;
                if( SfxItemState::SET == pTableFormat->GetItemState( RES_PAGEDESC,
                    false, &pItem ) )
                    pNextNd->SetAttr( *pItem );

                if( SfxItemState::SET == pTableFormat->GetItemState( RES_BREAK,
                    false, &pItem ) )
                    pNextNd->SetAttr( *pItem );
            }

            pTableNd->DelFrms();
            getIDocumentContentOperations().DeleteSection( pTableNd );
        }

        GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

        getIDocumentState().SetModified();
        getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );

        return true;
    }

    SwUndoTableNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( UNDO_TABLE_DELBOX, aSelBoxes, *pTableNd,
                                     nMin, nMax, 0, false, false );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
        aMsgHint.eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

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
            GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
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
bool SwDoc::SplitTable( const SwSelBoxes& rBoxes, bool bVert, sal_uInt16 nCnt,
                      bool bSameHeight )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    SwTable& rTable = pTableNd->GetTable();
    if( rTable.ISA( SwDDETable ))
        return false;

    std::vector<sal_uLong> aNdsCnts;
    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( UNDO_TABLE_SPLIT, rBoxes, *pTableNd, 0, 0,
                                     nCnt, bVert, bSameHeight );

        aTmpLst.insert( rTable.GetTabSortBoxes() );
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

        SwTableFormulaUpdate aMsgHint( &rTable );
        aMsgHint.eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        if (bVert)
            bRet = rTable.SplitCol( this, rBoxes, nCnt );
        else
            bRet = rTable.SplitRow( this, rBoxes, nCnt, bSameHeight );

        if (bRet)
        {
            GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
        }
    }

    if( pUndo )
    {
        if( bRet )
        {
            if( bVert )
                pUndo->SaveNewBoxes( *pTableNd, aTmpLst );
            else
                pUndo->SaveNewBoxes( *pTableNd, aTmpLst, rBoxes, aNdsCnts );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }

    return bRet;
}

sal_uInt16 SwDoc::MergeTable( SwPaM& rPam )
{
    // Check if the current cursor's Point/Mark are inside a Table
    SwTableNode* pTableNd = rPam.GetNode().FindTableNode();
    if( !pTableNd )
        return TBLMERGE_NOSELECTION;
    SwTable& rTable = pTableNd->GetTable();
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

    RedlineMode_t eOld = getIDocumentRedlineAccess().GetRedlineMode();
    getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

    SwUndoTableMerge *const pUndo( (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTableMerge( rPam )
        :   0 );

    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    SwSelBoxes aMerged;
    SwTableBox* pMergeBox;

    if( !rTable.PrepareMerge( rPam, aBoxes, aMerged, &pMergeBox, pUndo ) )
    {   // No cells found to merge
        getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
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
                if (pU && pU->GetRedlSaveCount())
                {
                    SwEditShell *const pEditShell(GetEditShell());
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
        // The PaMs need to be removed from the to-be-deleted range. Thus always place
        // them at the end of/on top of the Table; it's always set to the old position via
        // the Document Position.
        // For a start remember an index for the temporary position, because we cannot
        // access it after GetMergeSel
        {
            rPam.DeleteMark();
            rPam.GetPoint()->nNode = *pMergeBox->GetSttNd();
            rPam.GetPoint()->nContent.Assign( 0, 0 );
            rPam.SetMark();
            rPam.DeleteMark();

            SwPaM* pTmp = &rPam;
            while( &rPam != ( pTmp = static_cast<SwPaM*>(pTmp->GetNext()) ))
                for( int i = 0; i < 2; ++i )
                    pTmp->GetBound( (bool)i ) = *rPam.GetPoint();
        }

        // Merge them
        SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
        aMsgHint.eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        if( pTableNd->GetTable().Merge( this, aBoxes, aMerged, pMergeBox, pUndo ))
        {
            nRet = TBLMERGE_OK;

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
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
        getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    }
    GetIDocumentUndoRedo().EndUndo( UNDO_TABLE_MERGE, NULL );
    return nRet;
}

SwTableNode::SwTableNode( const SwNodeIndex& rIdx )
    : SwStartNode( rIdx, ND_TABLENODE )
{
    pTable = new SwTable( 0 );
}

SwTableNode::~SwTableNode()
{
    // Notify UNO wrappers
    SwFrameFormat* pTableFormat = GetTable().GetFrameFormat();
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
                                pTableFormat );
    pTableFormat->ModifyNotification( &aMsgHint, &aMsgHint );
    DelFrms();
    delete pTable;
}

SwTabFrm *SwTableNode::MakeFrm( SwFrm* pSib )
{
    return new SwTabFrm( *pTable, pSib );
}

/**
 * Creates all Views from the Document for the preceding Node. The resulting ContentFrames
 * are added to the corresponding Layout.
 */
void SwTableNode::MakeFrms(const SwNodeIndex & rIdx )
{
    if( !GetTable().GetFrameFormat()->HasWriterListeners()) // Do we actually have Frame?
        return;

    SwFrm *pFrm;
    SwContentNode * pNode = rIdx.GetNode().GetContentNode();

    OSL_ENSURE( pNode, "No ContentNode or CopyNode and new Node is identical");

    bool bBefore = rIdx < GetIndex();

    SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );

    while( 0 != (pFrm = aNode2Layout.NextFrm()) )
    {
        SwFrm *pNew = pNode->MakeFrm( pFrm );
        // Will the Node receive Frames before or after?
        if ( bBefore )
            // The new one precedes me
            pNew->Paste( pFrm->GetUpper(), pFrm );
        else
            // The new one succeeds me
            pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
    }
}

/**
 * Create a TableFrm for every Shell and insert before the corresponding ContentFrm.
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
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
        {
            SwViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrm*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTextFrm*>(pNew->FindPrevCnt( true )) );
            }
        }
        pNew->RegistFlys();
    }
}

void SwTableNode::DelFrms()
{
    /* For a start, cut out and delete the TabFrms (which will also delete the Columns and Rows)
       The TabFrms are attached to the FrameFormat of the SwTable.
       We need to delete them in a more cumbersome way, for the Master to also delete the Follows. */

    SwIterator<SwTabFrm,SwFormat> aIter( *(pTable->GetFrameFormat()) );
    SwTabFrm *pFrm = aIter.First();
    while ( pFrm )
    {
        bool bAgain = false;
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
                    SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTextFrm*>(pFrm->FindPrevCnt( true )) );
                    }
                }
                pFrm->Cut();
                SwFrm::DestroyFrm(pFrm);
                bAgain = true;
            }
        }
        pFrm = bAgain ? aIter.First() : aIter.Next();
    }
}

void SwTableNode::SetNewTable( SwTable* pNewTable, bool bNewFrames )
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

void SwTableNode::RemoveRedlines()
{
    SwDoc* pDoc = GetDoc();
    if (pDoc)
    {
        SwTable& rTable = GetTable();
        if ( pDoc->getIDocumentRedlineAccess().HasExtraRedlineTable() )
            pDoc->getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteAllTableRedlines( pDoc, rTable, true, USHRT_MAX );
    }
}

void SwDoc::GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                        const SwCellFrm* pBoxFrm )
{
    const SwTableBox* pBox = 0;
    SwTabFrm *pTab = 0;

    if( pBoxFrm )
    {
        pTab = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoxFrm))->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( pCrsr )
    {
        const SwContentNode* pCNd = pCrsr->GetContentNode();
        if( !pCNd )
            return ;

        Point aPt;
        const SwShellCrsr *pShCrsr = dynamic_cast<const SwShellCrsr*>(pCrsr);
        if( pShCrsr )
            aPt = pShCrsr->GetPtPos();

        const SwFrm* pTmpFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
        do {
            pTmpFrm = pTmpFrm->GetUpper();
        } while ( !pTmpFrm->IsCellFrm() );

        pBoxFrm = static_cast<const SwCellFrm*>(pTmpFrm);
        pTab = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoxFrm))->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( !pCrsr && !pBoxFrm )
    {
        OSL_ENSURE( false, "One of them needs to be specified!" );
        return ;
    }

    // Set fixed points, LeftMin in Document coordinates, all others relative
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

// Here are some little helpers used in SwDoc::GetTabRows

#define ROWFUZZY 25

struct FuzzyCompare
{
    bool operator() ( long s1, long s2 ) const;
};

bool FuzzyCompare::operator() ( long s1, long s2 ) const
{
    return ( s1 < s2 && std::abs( s1 - s2 ) > ROWFUZZY );
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
                        const SwCellFrm* pBoxFrm )
{
    OSL_ENSURE( pBoxFrm, "GetTabRows called without pBoxFrm" );

    // Make code robust:
    if ( !pBoxFrm )
        return;

    // #i39552# Collection of the boxes of the current
    // column has to be done at the beginning of this function, because
    // the table may be formatted in ::GetTableSel.
    SwDeletionChecker aDelCheck( pBoxFrm );

    SwSelBoxes aBoxes;
    const SwContentFrm* pContent = ::GetCellContent( *pBoxFrm );
    if ( pContent && pContent->IsTextFrm() )
    {
        const SwPosition aPos( *static_cast<const SwTextFrm*>(pContent)->GetTextNode() );
        const SwCursor aTmpCrsr( aPos, 0, false );
        ::GetTableSel( aTmpCrsr, aBoxes, nsSwTableSearchType::TBLSEARCH_COL );
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

    // Set fixed points, LeftMin in Document coordinates, all others relative
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
                long nNewLowerBorderUpperBoundary = std::max( (*aIter).second.first, nUpperBorder );
                aPair.first = nNewLowerBorderUpperBoundary; aPair.second = LONG_MAX;
            }
            aBoundaries[ nLowerBorder ] = aPair;

            // calculate hidden flags for entry nUpperBorder/nLowerBorder:
            long nTmpVal = nUpperBorder;
            for ( sal_uInt8 i = 0; i < 2; ++i )
            {
                aHiddenIter = aHidden.find( nTmpVal );
                if ( aHiddenIter == aHidden.end() )
                    aHidden[ nTmpVal ] = !lcl_IsFrmInColumn( *static_cast<const SwCellFrm*>(pFrm), aBoxes );
                else
                {
                    if ( aHidden[ nTmpVal ] &&
                         lcl_IsFrmInColumn( *static_cast<const SwCellFrm*>(pFrm), aBoxes ) )
                        aHidden[ nTmpVal ] = false;
                }
                nTmpVal = nLowerBorder;
            }
        }

        pFrm = pFrm->GetNextLayoutLeaf();
    }

    // transfer calculated values from BoundaryMap and HiddenMap into rFill:
    size_t nIdx = 0;
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
    if ( rFill.Count() ) rFill.Remove( 0 );
    if ( rFill.Count() ) rFill.Remove( rFill.Count() - 1 );
    rFill.SetLastRowAllowedToChange( !pTab->HasFollowFlowLine() );
}

void SwDoc::SetTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                        const SwCursor* pCrsr, const SwCellFrm* pBoxFrm )
{
    const SwTableBox* pBox = 0;
    SwTabFrm *pTab = 0;

    if( pBoxFrm )
    {
        pTab = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoxFrm))->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( pCrsr )
    {
        const SwContentNode* pCNd = pCrsr->GetContentNode();
        if( !pCNd )
            return ;

        Point aPt;
        const SwShellCrsr *pShCrsr = dynamic_cast<const SwShellCrsr*>(pCrsr);
        if( pShCrsr )
            aPt = pShCrsr->GetPtPos();

        const SwFrm* pTmpFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
        do {
            pTmpFrm = pTmpFrm->GetUpper();
        } while ( !pTmpFrm->IsCellFrm() );

        pBoxFrm = static_cast<const SwCellFrm*>(pTmpFrm);
        pTab = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoxFrm))->ImplFindTabFrm();
        pBox = pBoxFrm->GetTabBox();
    }
    else if( !pCrsr && !pBoxFrm )
    {
        OSL_ENSURE( false, "One of them needs to be specified!" );
        return ;
    }

    // If the Table is still using relative values (USHRT_MAX)
    // we need to switch to absolute ones.
    SwTable& rTab = *pTab->GetTable();
    const SwFormatFrmSize& rTableFrmSz = rTab.GetFrameFormat()->GetFrmSize();
    SWRECTFN( pTab )
    // #i17174# - With fix for #i9040# the shadow size is taken
    // from the table width. Thus, add its left and right size to current table
    // printing area width in order to get the correct table size attribute.
    SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
    {
        SvxShadowItem aShadow( rTab.GetFrameFormat()->GetShadow() );
        nPrtWidth += aShadow.CalcShadowSpace( SvxShadowItemSide::LEFT ) +
                     aShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT );
    }
    if( nPrtWidth != rTableFrmSz.GetWidth() )
    {
        SwFormatFrmSize aSz( rTableFrmSz );
        aSz.SetWidth( nPrtWidth );
        rTab.GetFrameFormat()->SetFormatAttr( aSz );
    }

    SwTabCols aOld( rNew.Count() );

    const SwPageFrm* pPage = pTab->FindPageFrm();
    const sal_uLong nLeftMin = (pTab->Frm().*fnRect->fnGetLeft)() -
                           (pPage->Frm().*fnRect->fnGetLeft)();
    const sal_uLong nRightMax = (pTab->Frm().*fnRect->fnGetRight)() -
                            (pPage->Frm().*fnRect->fnGetLeft)();

    // Set fixed points, LeftMin in Document coordinates, all others relative
    aOld.SetLeftMin ( nLeftMin );
    aOld.SetLeft    ( (pTab->Prt().*fnRect->fnGetLeft)() );
    aOld.SetRight   ( (pTab->Prt().*fnRect->fnGetRight)());
    aOld.SetRightMax( nRightMax - nLeftMin );

    rTab.GetTabCols( aOld, pBox );
    SetTabCols(rTab, rNew, aOld, pBox, bCurRowOnly );
}

void SwDoc::SetTabRows( const SwTabCols &rNew, bool bCurColOnly, const SwCursor*,
                        const SwCellFrm* pBoxFrm )
{
    SwTabFrm *pTab;

    OSL_ENSURE( pBoxFrm, "SetTabRows called without pBoxFrm" );

    pTab = const_cast<SwFrm*>(static_cast<SwFrm const *>(pBoxFrm))->ImplFindTabFrm();

    // If the Table is still using relative values (USHRT_MAX)
    // we need to switch to absolute ones.
    SWRECTFN( pTab )
    SwTabCols aOld( rNew.Count() );

    // Set fixed points, LeftMin in Document coordinates, all others relative
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
    const size_t nCount = rNew.Count();
    const SwTable* pTable = pTab->GetTable();
    OSL_ENSURE( pTable, "My colleague told me, this couldn't happen" );

    for ( size_t i = 0; i <= nCount; ++i )
    {
        const size_t nIdxStt = bVert ? nCount - i : i - 1;
        const size_t nIdxEnd = bVert ? nCount - i - 1 : i;

        const long nOldRowStart = i == 0  ? 0 : aOld[ nIdxStt ];
        const long nOldRowEnd =   i == nCount ? aOld.GetRight() : aOld[ nIdxEnd ];
        const long nOldRowHeight = nOldRowEnd - nOldRowStart;

        const long nNewRowStart = i == 0  ? 0 : rNew[ nIdxStt ];
        const long nNewRowEnd =   i == nCount ? rNew.GetRight() : rNew[ nIdxEnd ];
        const long nNewRowHeight = nNewRowEnd - nNewRowStart;

        const long nDiff = nNewRowHeight - nOldRowHeight;
        if ( std::abs( nDiff ) >= ROWFUZZY )
        {
            // For the old table model pTextFrm and pLine will be set for every box.
            // For the new table model pTextFrm will be set if the box is not covered,
            // but the pLine will be set if the box is not an overlapping box
            // In the new table model the row height can be adjusted,
            // when both variables are set.
            const SwTextFrm* pTextFrm = 0;
            const SwTableLine* pLine = 0;

            // Iterate over all SwCellFrms with Bottom = nOldPos
            const SwFrm* pFrm = pTab->GetNextLayoutLeaf();
            while ( pFrm && pTab->IsAnLower( pFrm ) )
            {
                if ( pFrm->IsCellFrm() && pFrm->FindTabFrm() == pTab )
                {
                    const long nLowerBorder = (pFrm->Frm().*fnRect->fnGetBottom)();
                    const sal_uLong nTabTop = (pTab->*fnRect->fnGetPrtTop)();
                    if ( std::abs( (*fnRect->fnYInc)( nTabTop, nOldRowEnd ) - nLowerBorder ) <= ROWFUZZY )
                    {
                        if ( !bCurColOnly || pFrm == pBoxFrm )
                        {
                            const SwFrm* pContent = ::GetCellContent( static_cast<const SwCellFrm&>(*pFrm) );

                            if ( pContent && pContent->IsTextFrm() )
                            {
                                const SwTableBox* pBox = static_cast<const SwCellFrm*>(pFrm)->GetTabBox();
                                const long nRowSpan = pBox->getRowSpan();
                                if( nRowSpan > 0 ) // Not overlapped
                                    pTextFrm = static_cast<const SwTextFrm*>(pContent);
                                if( nRowSpan < 2 ) // Not overlapping for row height
                                    pLine = pBox->GetUpper();
                                if( pLine && pTextFrm ) // always for old table model
                                {
                                    // The new row height must not to be calculated from a overlapping box
                                    SwFormatFrmSize aNew( pLine->GetFrameFormat()->GetFrmSize() );
                                    const long nNewSize = (pFrm->Frm().*fnRect->fnGetHeight)() + nDiff;
                                    if( nNewSize != aNew.GetHeight() )
                                    {
                                        aNew.SetHeight( nNewSize );
                                        if ( ATT_VAR_SIZE == aNew.GetHeightSizeType() )
                                            aNew.SetHeightSizeType( ATT_MIN_SIZE );
                                        // This position must not be in an overlapped box
                                        const SwPosition aPos( *static_cast<const SwTextFrm*>(pContent)->GetTextNode() );
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

/**
 * Direct access for UNO
 */
void SwDoc::SetTabCols(SwTable& rTab, const SwTabCols &rNew, const SwTabCols &rOld,
                                const SwTableBox *pStart, bool bCurRowOnly )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoAttrTable( *rTab.GetTableNode(), true ));
    }
    rTab.SetTabCols( rNew, rOld, pStart, bCurRowOnly );
      ::ClearFEShellTabCols();
    getIDocumentState().SetModified();
}

void SwDoc::SetRowsToRepeat( SwTable &rTable, sal_uInt16 nSet )
{
    if( nSet == rTable.GetRowsToRepeat() )
        return;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            new SwUndoTableHeadline(rTable, rTable.GetRowsToRepeat(), nSet) );
    }

    SwMsgPoolItem aChg( RES_TBLHEADLINECHG );
    rTable.SetRowsToRepeat( nSet );
    rTable.GetFrameFormat()->ModifyNotification( &aChg, &aChg );
    getIDocumentState().SetModified();
}

void SwCollectTableLineBoxes::AddToUndoHistory( const SwContentNode& rNd )
{
    if( pHst )
        pHst->Add( rNd.GetFormatColl(), rNd.GetIndex(), ND_TEXTNODE );
}

void SwCollectTableLineBoxes::AddBox( const SwTableBox& rBox )
{
    aPosArr.push_back(nWidth);
    SwTableBox* p = const_cast<SwTableBox*>(&rBox);
    m_Boxes.push_back(p);
    nWidth = nWidth + (sal_uInt16)rBox.GetFrameFormat()->GetFrmSize().GetWidth();
}

const SwTableBox* SwCollectTableLineBoxes::GetBoxOfPos( const SwTableBox& rBox )
{
    const SwTableBox* pRet = 0;

    if( !aPosArr.empty() )
    {
        std::vector<sal_uInt16>::size_type n;
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

        nWidth = nWidth + (sal_uInt16)rBox.GetFrameFormat()->GetFrmSize().GetWidth();
        pRet = m_Boxes[ n ];
    }
    return pRet;
}

bool SwCollectTableLineBoxes::Resize( sal_uInt16 nOffset, sal_uInt16 nOldWidth )
{
    if( !aPosArr.empty() )
    {
        std::vector<sal_uInt16>::size_type n;
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

        size_t nArrSize = aPosArr.size();
        if (nArrSize)
        {
            if (nOldWidth == 0)
                throw o3tl::divide_by_zero();

            // Adapt the positions to the new Size
            for( n = 0; n < nArrSize; ++n )
            {
                sal_uLong nSize = nWidth;
                nSize *= ( aPosArr[ n ] - nOffset );
                nSize /= nOldWidth;
                aPosArr[ n ] = sal_uInt16( nSize );
            }
        }
    }
    return !aPosArr.empty();
}

bool sw_Line_CollectBox( const SwTableLine*& rpLine, void* pPara )
{
    SwCollectTableLineBoxes* pSplPara = static_cast<SwCollectTableLineBoxes*>(pPara);
    if( pSplPara->IsGetValues() )
        for( SwTableBoxes::iterator it = const_cast<SwTableLine*>(rpLine)->GetTabBoxes().begin();
                 it != const_cast<SwTableLine*>(rpLine)->GetTabBoxes().end(); ++it)
            sw_Box_CollectBox(*it, pSplPara );
    else
        for( SwTableBoxes::iterator it = const_cast<SwTableLine*>(rpLine)->GetTabBoxes().begin();
                 it != const_cast<SwTableLine*>(rpLine)->GetTabBoxes().end(); ++it)
            sw_BoxSetSplitBoxFormats(*it, pSplPara );
    return true;
}

void sw_Box_CollectBox( const SwTableBox* pBox, SwCollectTableLineBoxes* pSplPara )
{
    auto nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        // Continue with the actual Line
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

void sw_BoxSetSplitBoxFormats( SwTableBox* pBox, SwCollectTableLineBoxes* pSplPara )
{
    auto nLen = pBox->GetTabLines().size();
    if( nLen )
    {
        // Continue with the actual Line
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
        SwFrameFormat* pFormat = pSrcBox->GetFrameFormat();

        if( HEADLINE_BORDERCOPY == pSplPara->GetMode() )
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrameFormat()->GetBox();
            if( !rBoxItem.GetTop() )
            {
                SvxBoxItem aNew( rBoxItem );
                aNew.SetLine( pFormat->GetBox().GetBottom(), SvxBoxItemLine::TOP );
                if( aNew != rBoxItem )
                    pBox->ClaimFrameFormat()->SetFormatAttr( aNew );
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

            SfxItemSet aTmpSet( pFormat->GetDoc()->GetAttrPool(),
                                aTableSplitBoxSetRange );
            aTmpSet.Put( pFormat->GetAttrSet() );
            if( aTmpSet.Count() )
                pBox->ClaimFrameFormat()->SetFormatAttr( aTmpSet );

            if( HEADLINE_BOXATRCOLLCOPY == pSplPara->GetMode() )
            {
                SwNodeIndex aIdx( *pSrcBox->GetSttNd(), 1 );
                SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
                if( !pCNd )
                    pCNd = aIdx.GetNodes().GoNext( &aIdx );
                aIdx = *pBox->GetSttNd();
                SwContentNode* pDNd = aIdx.GetNodes().GoNext( &aIdx );

                // If the Node is alone in the Section
                if( 2 == pDNd->EndOfSectionIndex() -
                        pDNd->StartOfSectionIndex() )
                {
                    pSplPara->AddToUndoHistory( *pDNd );
                    pDNd->ChgFormatColl( pCNd->GetFormatColl() );
                }
            }

            // note conditional template
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
bool SwDoc::SplitTable( const SwPosition& rPos, sal_uInt16 eHdlnMode,
                        bool bCalcNewSize )
{
    SwNode* pNd = &rPos.nNode.GetNode();
    SwTableNode* pTNd = pNd->FindTableNode();
    if( !pTNd || pNd->IsTableNode() )
        return false;

    if( pTNd->GetTable().ISA( SwDDETable ))
        return false;

    SwTable& rTable = pTNd->GetTable();
    rTable.SetHTMLTableLayout( 0 ); // Delete HTML Layout

    SwTableFormulaUpdate aMsgHint( &rTable );

    SwHistory aHistory;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        aMsgHint.pHistory = &aHistory;
    }

    {
        sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

        // Find top-level Line
        SwTableBox* pBox = rTable.GetTableBox( nSttIdx );
        if( pBox )
        {
            SwTableLine* pLine = pBox->GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            // pLine contains the top-level Line now
            aMsgHint.nSplitLine = rTable.GetTabLines().GetPos( pLine );
        }

        OUString sNewTableNm( GetUniqueTableName() );
        aMsgHint.DATA.pNewTableNm = &sNewTableNm;
        aMsgHint.eFlags = TBL_SPLITTBL;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    // Find Lines for the Layout update
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rTable );
    aFndBox.DelFrms( rTable );

    SwTableNode* pNew = GetNodes().SplitTable( rPos.nNode, false, bCalcNewSize );

    if( pNew )
    {
        SwSaveRowSpan* pSaveRowSp = pNew->GetTable().CleanUpTopRowSpan( rTable.GetTabLines().size() );
        SwUndoSplitTable* pUndo = 0;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo = new SwUndoSplitTable(
                        *pNew, pSaveRowSp, eHdlnMode, bCalcNewSize);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
            if( aHistory.Count() )
                pUndo->SaveFormula( aHistory );
        }

        switch( eHdlnMode )
        {
        // Set the lower Border of the preceding Line to
        // the upper Border of the current one
        case HEADLINE_BORDERCOPY:
            {
                SwCollectTableLineBoxes aPara( false, eHdlnMode );
                SwTableLine* pLn = rTable.GetTabLines()[
                            rTable.GetTabLines().size() - 1 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_Box_CollectBox(*it, &aPara );

                aPara.SetValues( true );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFormats(*it, &aPara );

                // Switch off repeating Header
                pNew->GetTable().SetRowsToRepeat( 0 );
            }
            break;

        // Take over the Attributes of the first Line to the new one
        case HEADLINE_BOXATTRCOPY:
        case HEADLINE_BOXATRCOLLCOPY:
            {
                SwHistory* pHst = 0;
                if( HEADLINE_BOXATRCOLLCOPY == eHdlnMode && pUndo )
                    pHst = pUndo->GetHistory();

                SwCollectTableLineBoxes aPara( true, eHdlnMode, pHst );
                SwTableLine* pLn = rTable.GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_Box_CollectBox(*it, &aPara );

                aPara.SetValues( true );
                pLn = pNew->GetTable().GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFormats(*it, &aPara );
            }
            break;

        case HEADLINE_CNTNTCOPY:
            rTable.CopyHeadlineIntoTable( *pNew );
            if( pUndo )
                pUndo->SetTableNodeOffset( pNew->GetIndex() );
            break;

        case HEADLINE_NONE:
            // Switch off repeating the Header
            pNew->GetTable().SetRowsToRepeat( 0 );
            break;
        }

        // And insert Frms
        SwNodeIndex aNdIdx( *pNew->EndOfSectionNode() );
        GetNodes().GoNext( &aNdIdx ); // To the next ContentNode
        pNew->MakeFrms( &aNdIdx );

        // Insert a paragraph between the Table
        GetNodes().MakeTextNode( SwNodeIndex( *pNew ),
                                getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ) );
    }

    // Update Layout
    aFndBox.MakeFrms( rTable );

    // TL_CHART2: need to inform chart of probably changed cell names
    UpdateCharts( rTable.GetFrameFormat()->GetName() );

    // update table style formatting of both the tables
    GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(pTNd);
    GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(pNew);

    getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );

    return 0 != pNew;
}

static bool lcl_ChgTableSize( SwTable& rTable )
{
    // The Attribute must not be set via the Modify or else all Boxes are
    // set back to 0.
    // So lock the Format.
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SwFormatFrmSize aTableMaxSz( pFormat->GetFrmSize() );

    if( USHRT_MAX == aTableMaxSz.GetWidth() )
        return false;

    bool bLocked = pFormat->IsModifyLocked();
    pFormat->LockModify();

    aTableMaxSz.SetWidth( 0 );

    SwTableLines& rLns = rTable.GetTabLines();
    for( auto pLn : rLns )
    {
        SwTwips nMaxLnWidth = 0;
        SwTableBoxes& rBoxes = pLn->GetTabBoxes();
        for( auto pBox : rBoxes )
            nMaxLnWidth += pBox->GetFrameFormat()->GetFrmSize().GetWidth();

        if( nMaxLnWidth > aTableMaxSz.GetWidth() )
            aTableMaxSz.SetWidth( nMaxLnWidth );
    }
    pFormat->SetFormatAttr( aTableMaxSz );
    if( !bLocked ) // Release the Lock if appropriate
        pFormat->UnlockModify();

    return true;
}

class _SplitTable_Para
{
    std::map<SwFrameFormat*, SwFrameFormat*> aSrcDestMap;
    SwTableNode* pNewTableNd;
    SwTable& rOldTable;

public:
    _SplitTable_Para( SwTableNode* pNew, SwTable& rOld )
        : aSrcDestMap(), pNewTableNd( pNew ), rOldTable( rOld )
    {}
    SwFrameFormat* GetDestFormat( SwFrameFormat* pSrcFormat ) const
    {
        std::map<SwFrameFormat*, SwFrameFormat*>::const_iterator it = aSrcDestMap.find( pSrcFormat );
        return it == aSrcDestMap.end() ? NULL : it->second;
    }

    void InsertSrcDest( SwFrameFormat* pSrcFormat, SwFrameFormat* pDestFormat )
            { aSrcDestMap[ pSrcFormat ] = pDestFormat; }

    void ChgBox( SwTableBox* pBox )
    {
        rOldTable.GetTabSortBoxes().erase( pBox );
        pNewTableNd->GetTable().GetTabSortBoxes().insert( pBox );
    }
};

static void lcl_SplitTable_CpyBox( SwTableBox* pBox, _SplitTable_Para* pPara );

static void lcl_SplitTable_CpyLine( SwTableLine* pLn, _SplitTable_Para* pPara )
{
    SwFrameFormat *pSrcFormat = pLn->GetFrameFormat();
    SwTableLineFormat* pDestFormat = static_cast<SwTableLineFormat*>( pPara->GetDestFormat( pSrcFormat ) );
    if( pDestFormat == NULL )
    {
        pPara->InsertSrcDest( pSrcFormat, pLn->ClaimFrameFormat() );
    }
    else
        pLn->ChgFrameFormat( pDestFormat );

    for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
             it != pLn->GetTabBoxes().end(); ++it)
        lcl_SplitTable_CpyBox(*it, pPara );
}

static void lcl_SplitTable_CpyBox( SwTableBox* pBox, _SplitTable_Para* pPara )
{
    SwFrameFormat *pSrcFormat = pBox->GetFrameFormat();
    SwTableBoxFormat* pDestFormat = static_cast<SwTableBoxFormat*>(pPara->GetDestFormat( pSrcFormat ));
    if( pDestFormat == NULL )
    {
        pPara->InsertSrcDest( pSrcFormat, pBox->ClaimFrameFormat() );
    }
    else
        pBox->ChgFrameFormat( pDestFormat );

    if( pBox->GetSttNd() )
        pPara->ChgBox( pBox );
    else
        for( SwTableLine* pLine : pBox->GetTabLines() )
            lcl_SplitTable_CpyLine( pLine, pPara );
}

SwTableNode* SwNodes::SplitTable( const SwNodeIndex& rPos, bool bAfter,
                                    bool bCalcNewSize )
{
    SwNode* pNd = &rPos.GetNode();
    SwTableNode* pTNd = pNd->FindTableNode();
    if( !pTNd || pNd->IsTableNode() )
        return 0;

    sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

    // Find this Box/top-level line
    SwTable& rTable = pTNd->GetTable();
    SwTableBox* pBox = rTable.GetTableBox( nSttIdx );
    if( !pBox )
        return 0;

    SwTableLine* pLine = pBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    // pLine now contains the top-level line
    sal_uInt16 nLinePos = rTable.GetTabLines().GetPos( pLine );
    if( USHRT_MAX == nLinePos ||
        ( bAfter ? ++nLinePos >= rTable.GetTabLines().size() : !nLinePos ))
        return 0; // Not found or last Line!

    // Find the first Box of the succeeding Line
    SwTableLine* pNextLine = rTable.GetTabLines()[ nLinePos ];
    pBox = pNextLine->GetTabBoxes()[0];
    while( !pBox->GetSttNd() )
        pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];

    // Insert an EndNode and TableNode into the Nodes Array
    SwTableNode * pNewTableNd;
    {
        SwEndNode* pOldTableEndNd = pTNd->EndOfSectionNode()->GetEndNode();
        OSL_ENSURE( pOldTableEndNd, "Where is the EndNode?" );

        SwNodeIndex aIdx( *pBox->GetSttNd() );
        new SwEndNode( aIdx, *pTNd );
        pNewTableNd = new SwTableNode( aIdx );
        pNewTableNd->GetTable().SetTableModel( rTable.IsNewModel() );

        pOldTableEndNd->pStartOfSection = pNewTableNd;
        pNewTableNd->pEndOfSection = pOldTableEndNd;

        SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
        do {
            OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
            pBoxNd->pStartOfSection = pNewTableNd;
            pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
        } while( pBoxNd != pOldTableEndNd );
    }

    {
        // Move the Lines
        SwTable& rNewTable = pNewTableNd->GetTable();
        rNewTable.GetTabLines().insert( rNewTable.GetTabLines().begin(),
                      rTable.GetTabLines().begin() + nLinePos, rTable.GetTabLines().end() );

        /* From the back (bottom right) to the front (top left) deregister all Boxes from the
           Chart Data Provider. The Modify event is triggered in the calling function.
         TL_CHART2: */
        SwChartDataProvider *pPCD = rTable.GetFrameFormat()->getIDocumentChartDataProviderAccess().GetChartDataProvider();
        if( pPCD )
        {
            for (SwTableLines::size_type k = nLinePos;  k < rTable.GetTabLines().size(); ++k)
            {
                const SwTableLines::size_type nLineIdx = (rTable.GetTabLines().size() - 1) - k + nLinePos;
                const SwTableBoxes::size_type nBoxCnt = rTable.GetTabLines()[ nLineIdx ]->GetTabBoxes().size();
                for (SwTableBoxes::size_type j = 0;  j < nBoxCnt;  ++j)
                {
                    const SwTableBoxes::size_type nIdx = nBoxCnt - 1 - j;
                    pPCD->DeleteBox( &rTable, *rTable.GetTabLines()[ nLineIdx ]->GetTabBoxes()[nIdx] );
                }
            }
        }

        // Delete
        sal_uInt16 nDeleted = rTable.GetTabLines().size() - nLinePos;
        rTable.GetTabLines().erase( rTable.GetTabLines().begin() + nLinePos, rTable.GetTabLines().end() );

        // Move the affected Boxes. Make the Formats unique and correct the StartNodes
        _SplitTable_Para aPara( pNewTableNd, rTable );
        for( SwTableLine* pNewLine : rNewTable.GetTabLines() )
            lcl_SplitTable_CpyLine( pNewLine, &aPara );
        rTable.CleanUpBottomRowSpan( nDeleted );
    }

    {
        // Copy the Table FrmFormat
        SwFrameFormat* pOldTableFormat = rTable.GetFrameFormat();
        SwFrameFormat* pNewTableFormat = pOldTableFormat->GetDoc()->MakeTableFrameFormat(
                                pOldTableFormat->GetDoc()->GetUniqueTableName(),
                                pOldTableFormat->GetDoc()->GetDfltFrameFormat() );

        *pNewTableFormat = *pOldTableFormat;
        pNewTableNd->GetTable().RegisterToFormat( *pNewTableFormat );

        pNewTableNd->GetTable().SetTableStyleName(rTable.GetTableStyleName());

        // Calculate a new Size?
        // lcl_ChgTableSize: Only execute the second call if the first call was
        // successful, thus has an absolute Size
        if( bCalcNewSize && lcl_ChgTableSize( rTable ) )
            lcl_ChgTableSize( pNewTableNd->GetTable() );
    }

    // TL_CHART2: need to inform chart of probably changed cell names
    rTable.UpdateCharts();

    return pNewTableNd; // That's it!
}

/**
 * rPos needs to be in the Table that remains
 *
 * @param bWithPrev  merge the current Table with the preceding
 *                   or succeeding one
 */
bool SwDoc::MergeTable( const SwPosition& rPos, bool bWithPrev, sal_uInt16 nMode )
{
    SwTableNode* pTableNd = rPos.nNode.GetNode().FindTableNode(), *pDelTableNd;
    if( !pTableNd )
        return false;

    SwNodes& rNds = GetNodes();
    if( bWithPrev )
        pDelTableNd = rNds[ pTableNd->GetIndex() - 1 ]->FindTableNode();
    else
        pDelTableNd = rNds[ pTableNd->EndOfSectionIndex() + 1 ]->GetTableNode();
    if( !pDelTableNd )
        return false;

    if( pTableNd->GetTable().ISA( SwDDETable ) ||
        pDelTableNd->GetTable().ISA( SwDDETable ))
        return false;

    // Delete HTML Layout
    pTableNd->GetTable().SetHTMLTableLayout( 0 );
    pDelTableNd->GetTable().SetHTMLTableLayout( 0 );

    // Both Tables are present; we can start
    SwUndoMergeTable* pUndo = 0;
    SwHistory* pHistory = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMergeTable( *pTableNd, *pDelTableNd, bWithPrev, nMode );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        pHistory = new SwHistory;
    }

    // Adapt all "TableFormulas"
    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.DATA.pDelTable = &pDelTableNd->GetTable();
    aMsgHint.eFlags = TBL_MERGETBL;
    aMsgHint.pHistory = pHistory;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    // The actual merge
    SwNodeIndex aIdx( bWithPrev ? *pTableNd : *pDelTableNd );
    bool bRet = rNds.MergeTable( aIdx, !bWithPrev, nMode, pHistory );

    if( pHistory )
    {
        if( pHistory->Count() )
            pUndo->SaveFormula( *pHistory );
        delete pHistory;
    }
    if( bRet )
    {
        GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

        getIDocumentState().SetModified();
        getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}

bool SwNodes::MergeTable( const SwNodeIndex& rPos, bool bWithPrev,
                            sal_uInt16 nMode, SwHistory* )
{
    SwTableNode* pDelTableNd = rPos.GetNode().GetTableNode();
    OSL_ENSURE( pDelTableNd, "Where did the TableNode go?" );

    SwTableNode* pTableNd = (*this)[ rPos.GetIndex() - 1]->FindTableNode();
    OSL_ENSURE( pTableNd, "Where did the TableNode go?" );

    if( !pDelTableNd || !pTableNd )
        return false;

    pDelTableNd->DelFrms();

    SwTable& rDelTable = pDelTableNd->GetTable();
    SwTable& rTable = pTableNd->GetTable();

    // Find Lines for the Layout update
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rTable );
    aFndBox.DelFrms( rTable );

    // TL_CHART2:
    // tell the charts about the table to be deleted and have them use their own data
    GetDoc()->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( &rDelTable );

    // Sync the TableFormat's Width
    {
        const SwFormatFrmSize& rTableSz = rTable.GetFrameFormat()->GetFrmSize();
        const SwFormatFrmSize& rDelTableSz = rDelTable.GetFrameFormat()->GetFrmSize();
        if( rTableSz != rDelTableSz )
        {
            // The needs correction
            if( bWithPrev )
                rDelTable.GetFrameFormat()->SetFormatAttr( rTableSz );
            else
                rTable.GetFrameFormat()->SetFormatAttr( rDelTableSz );
        }
    }

    if( !bWithPrev )
    {
        // Transfer all Attributes of the succeeding Table to the preceding one
        // We do this, because the succeeding one is deleted when deleting the Node
        rTable.SetRowsToRepeat( rDelTable.GetRowsToRepeat() );
        rTable.SetTableChgMode( rDelTable.GetTableChgMode() );

        rTable.GetFrameFormat()->LockModify();
        *rTable.GetFrameFormat() = *rDelTable.GetFrameFormat();
        // Also switch the Name
        rTable.GetFrameFormat()->SetName( rDelTable.GetFrameFormat()->GetName() );
        rTable.GetFrameFormat()->UnlockModify();
    }

    // Move the Lines and Boxes
    SwTableLines::size_type nOldSize = rTable.GetTabLines().size();
    rTable.GetTabLines().insert( rTable.GetTabLines().begin() + nOldSize,
                               rDelTable.GetTabLines().begin(), rDelTable.GetTabLines().end() );
    rDelTable.GetTabLines().clear();

    rTable.GetTabSortBoxes().insert( rDelTable.GetTabSortBoxes() );
    rDelTable.GetTabSortBoxes().clear();

    // The preceding Table always remains, while the succeeding one is deleted
    SwEndNode* pTableEndNd = pDelTableNd->EndOfSectionNode();
    pTableNd->pEndOfSection = pTableEndNd;

    SwNodeIndex aIdx( *pDelTableNd, 1 );

    SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
    do {
        OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
        pBoxNd->pStartOfSection = pTableNd;
        pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
    } while( pBoxNd != pTableEndNd );
    pBoxNd->pStartOfSection = pTableNd;

    aIdx -= 2;
    DelNodes( aIdx, 2 );

    // tweak the conditional styles at the first inserted Line
    const SwTableLine* pFirstLn = rTable.GetTabLines()[ nOldSize ];
    if( 1 == nMode )
    {
        // Set Header Template in the Line and save in the History
        // if needed for Undo!
    }
    sw_LineSetHeadCondColl( pFirstLn );

    // Clean up the Borders
    if( nOldSize )
    {
        _SwGCLineBorder aPara( rTable );
        aPara.nLinePos = --nOldSize;
        pFirstLn = rTable.GetTabLines()[ nOldSize ];
        sw_GC_Line_Border( pFirstLn, &aPara );
    }

    // Update Layout
    aFndBox.MakeFrms( rTable );

    return true;
}

// Use the PtrArray's ForEach method
struct _SetAFormatTabPara
{
    SwTableAutoFormat& rTableFormat;
    SwUndoTableAutoFormat* pUndo;
    sal_uInt16 nEndBox, nCurBox;
    sal_uInt8 nAFormatLine, nAFormatBox;

    explicit _SetAFormatTabPara( const SwTableAutoFormat& rNew )
        : rTableFormat( (SwTableAutoFormat&)rNew ), pUndo( 0 ),
        nEndBox( 0 ), nCurBox( 0 ), nAFormatLine( 0 ), nAFormatBox( 0 )
    {}
};

// Forward declare so that the Lines and Boxes can use recursion
static bool lcl_SetAFormatBox(_FndBox &, _SetAFormatTabPara *pSetPara, bool bResetDirect);
static bool lcl_SetAFormatLine(_FndLine &, _SetAFormatTabPara *pPara, bool bResetDirect);

static bool lcl_SetAFormatLine(_FndLine & rLine, _SetAFormatTabPara *pPara, bool bResetDirect)
{
    for (auto const& it : rLine.GetBoxes())
    {
        lcl_SetAFormatBox(*it, pPara, bResetDirect);
    }
    return true;
}

static bool lcl_SetAFormatBox(_FndBox & rBox, _SetAFormatTabPara *pSetPara, bool bResetDirect)
{
    if (!rBox.GetUpper()->GetUpper()) // Box on first level?
    {
        if( !pSetPara->nCurBox )
            pSetPara->nAFormatBox = 0;
        else if( pSetPara->nCurBox == pSetPara->nEndBox )
            pSetPara->nAFormatBox = 3;
        else
            pSetPara->nAFormatBox = (sal_uInt8)(1 + ((pSetPara->nCurBox-1) & 1));
    }

    if (rBox.GetBox()->GetSttNd())
    {
        SwTableBox* pSetBox = static_cast<SwTableBox*>(rBox.GetBox());
        if (!pSetBox->HasDirectFormatting() || bResetDirect)
        {
            if (bResetDirect)
                pSetBox->SetDirectFormatting(false);

            SwDoc* pDoc = pSetBox->GetFrameFormat()->GetDoc();
            SfxItemSet aCharSet(pDoc->GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1);
            SfxItemSet aBoxSet(pDoc->GetAttrPool(), aTableBoxSetRange);
            sal_uInt8 nPos = pSetPara->nAFormatLine * 4 + pSetPara->nAFormatBox;
            pSetPara->rTableFormat.UpdateToSet(nPos, aCharSet, SwTableAutoFormat::UPDATE_CHAR, 0);
            pSetPara->rTableFormat.UpdateToSet(nPos, aBoxSet, SwTableAutoFormat::UPDATE_BOX, pDoc->GetNumberFormatter());

            if (aCharSet.Count())
            {
                sal_uLong nSttNd = pSetBox->GetSttIdx()+1;
                sal_uLong nEndNd = pSetBox->GetSttNd()->EndOfSectionIndex();
                for (; nSttNd < nEndNd; ++nSttNd)
                {
                    SwContentNode* pNd = pDoc->GetNodes()[ nSttNd ]->GetContentNode();
                    if (pNd)
                        pNd->SetAttr(aCharSet);
                }
            }

            if (aBoxSet.Count())
            {
                if (pSetPara->pUndo && SfxItemState::SET == aBoxSet.GetItemState(RES_BOXATR_FORMAT))
                    pSetPara->pUndo->SaveBoxContent( *pSetBox );

                pSetBox->ClaimFrameFormat()->SetFormatAttr(aBoxSet);
            }
        }
    }
    else
    {
        for (auto const& rpFndLine : rBox.GetLines())
        {
            lcl_SetAFormatLine(*rpFndLine, pSetPara, bResetDirect);
        }
    }

    if (!rBox.GetUpper()->GetUpper()) // a BaseLine
        ++pSetPara->nCurBox;
    return true;
}

bool SwDoc::SetTableAutoFormat(const SwSelBoxes& rBoxes, const SwTableAutoFormat& rNew, bool bResetDirect)
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    SwTable &table = pTableNd->GetTable();
    table.SetHTMLTableLayout( 0 );

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front()->GetBoxes().size())
    {
        pFndBox = pFndBox->GetLines().front()->GetBoxes()[0].get();
    }

    if( pFndBox->GetLines().empty() ) // One too far? (only one sel. Box)
        pFndBox = pFndBox->GetUpper()->GetUpper();

    // Disable Undo, but first store parameters
    SwUndoTableAutoFormat* pUndo = 0;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndo = new SwUndoTableAutoFormat( *pTableNd, rNew );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        GetIDocumentUndoRedo().DoUndo(false);
    }

    rNew.RestoreTableProperties(table);

    _SetAFormatTabPara aPara( rNew );
    FndLines_t& rFLns = pFndBox->GetLines();

    for (FndLines_t::size_type n = 0; n < rFLns.size(); ++n)
    {
        _FndLine* pLine = rFLns[n].get();

        // Set Upper to 0 (thus simulate BaseLine)
        _FndBox* pSaveBox = pLine->GetUpper();
        pLine->SetUpper( 0 );

        if( !n )
            aPara.nAFormatLine = 0;
        else if (static_cast<size_t>(n+1) == rFLns.size())
            aPara.nAFormatLine = 3;
        else
            aPara.nAFormatLine = (sal_uInt8)(1 + ((n-1) & 1 ));

        aPara.nAFormatBox = 0;
        aPara.nCurBox = 0;
        aPara.nEndBox = pLine->GetBoxes().size()-1;
        aPara.pUndo = pUndo;
        for (auto const& it : pLine->GetBoxes())
        {
            lcl_SetAFormatBox(*it, &aPara, bResetDirect);
        }

        pLine->SetUpper( pSaveBox );
    }

    if( pUndo )
    {
        GetIDocumentUndoRedo().DoUndo(bUndo);
    }

    getIDocumentState().SetModified();
    getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );

    return true;
}

/**
 * Find out who has the Attributes
 */
bool SwDoc::GetTableAutoFormat( const SwSelBoxes& rBoxes, SwTableAutoFormat& rGet )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    // Store table properties
    SwTable &table = pTableNd->GetTable();
    rGet.StoreTableProperties(table);

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front()->GetBoxes().size())
    {
        pFndBox = pFndBox->GetLines().front()->GetBoxes()[0].get();
    }

    if( pFndBox->GetLines().empty() ) // One too far? (only one sel. Box)
        pFndBox = pFndBox->GetUpper()->GetUpper();

    FndLines_t& rFLns = pFndBox->GetLines();

    sal_uInt16 aLnArr[4];
    aLnArr[0] = 0;
    aLnArr[1] = 1 < rFLns.size() ? 1 : 0;
    aLnArr[2] = 2 < rFLns.size() ? 2 : aLnArr[1];
    aLnArr[3] = rFLns.size() - 1;

    for( sal_uInt8 nLine = 0; nLine < 4; ++nLine )
    {
        _FndLine& rLine = *rFLns[ aLnArr[ nLine ] ];

        sal_uInt16 aBoxArr[4];
        aBoxArr[0] = 0;
        aBoxArr[1] = 1 < rLine.GetBoxes().size() ? 1 : 0;
        aBoxArr[2] = 2 < rLine.GetBoxes().size() ? 2 : aBoxArr[1];
        aBoxArr[3] = rLine.GetBoxes().size() - 1;

        for( sal_uInt8 nBox = 0; nBox < 4; ++nBox )
        {
            SwTableBox* pFBox = rLine.GetBoxes()[ aBoxArr[ nBox ] ]->GetBox();
            // Always apply to the first ones
            while( !pFBox->GetSttNd() )
                pFBox = pFBox->GetTabLines()[0]->GetTabBoxes()[0];

            sal_uInt8 nPos = nLine * 4 + nBox;
            SwNodeIndex aIdx( *pFBox->GetSttNd(), 1 );
            SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = GetNodes().GoNext( &aIdx );

            if( pCNd )
                rGet.UpdateFromSet( nPos, pCNd->GetSwAttrSet(),
                                    SwTableAutoFormat::UPDATE_CHAR, 0 );
            rGet.UpdateFromSet( nPos, pFBox->GetFrameFormat()->GetAttrSet(),
                                SwTableAutoFormat::UPDATE_BOX,
                                GetNumberFormatter() );
        }
    }

    return true;
}

OUString SwDoc::GetUniqueTableName() const
{
    if( IsInMailMerge())
    {
        OUString newName = "MailMergeTable"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( mpTableFrameFormatTable->size() + 1 );
        return newName;
    }

    ResId aId( STR_TABLE_DEFNAME, *pSwResMgr );
    const OUString aName( aId );

    const size_t nFlagSize = ( mpTableFrameFormatTable->size() / 8 ) + 2;

    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( size_t n = 0; n < mpTableFrameFormatTable->size(); ++n )
    {
        const SwFrameFormat* pFormat = (*mpTableFrameFormatTable)[ n ];
        if( !pFormat->IsDefault() && IsUsed( *pFormat )  &&
            pFormat->GetName().startsWith( aName ) )
        {
            // Get number and set the Flag
            const sal_Int32 nNmLen = aName.getLength();
            size_t nNum = pFormat->GetName().copy( nNmLen ).toInt32();
            if( nNum-- && nNum < mpTableFrameFormatTable->size() )
                pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
        }
    }

    // All numbers are flagged properly, thus calculate the right number
    size_t nNum = mpTableFrameFormatTable->size();
    for( size_t n = 0; n < nFlagSize; ++n )
    {
        auto nTmp = pSetFlags[ n ];
        if( nTmp != 0xFF )
        {
            // Calculate the number
            nNum = n * 8;
            while( nTmp & 1 )
            {
                ++nNum;
                nTmp >>= 1;
            }
            break;
        }
    }

    delete [] pSetFlags;
    return aName + OUString::number( ++nNum );
}

SwTableFormat* SwDoc::FindTableFormatByName( const OUString& rName, bool bAll ) const
{
    const SwFormat* pRet = 0;
    if( bAll )
        pRet = FindFormatByName( *mpTableFrameFormatTable, rName );
    else
    {
        // Only the ones set in the Doc
        for( size_t n = 0; n < mpTableFrameFormatTable->size(); ++n )
        {
            const SwFrameFormat* pFormat = (*mpTableFrameFormatTable)[ n ];
            if( !pFormat->IsDefault() && IsUsed( *pFormat ) &&
                pFormat->GetName() == rName )
            {
                pRet = pFormat;
                break;
            }
        }
    }
    return const_cast<SwTableFormat*>(static_cast<const SwTableFormat*>(pRet));
}

bool SwDoc::SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                    SwTwips nAbsDiff, SwTwips nRelDiff )
{
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rAktBox.GetSttNd()->FindTableNode());
    SwUndo* pUndo = 0;

    if( nsTableChgWidthHeightType::WH_FLAG_INSDEL & eType && pTableNd->GetTable().ISA( SwDDETable ))
        return false;

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.eFlags = TBL_BOXPTR;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    bool bRet = false;
    switch( eType & 0xff )
    {
    case nsTableChgWidthHeightType::WH_COL_LEFT:
    case nsTableChgWidthHeightType::WH_COL_RIGHT:
    case nsTableChgWidthHeightType::WH_CELL_LEFT:
    case nsTableChgWidthHeightType::WH_CELL_RIGHT:
        {
             bRet = pTableNd->GetTable().SetColWidth( rAktBox,
                                eType, nAbsDiff, nRelDiff,
                                (bUndo) ? &pUndo : 0 );
        }
        break;
    case nsTableChgWidthHeightType::WH_ROW_TOP:
    case nsTableChgWidthHeightType::WH_ROW_BOTTOM:
    case nsTableChgWidthHeightType::WH_CELL_TOP:
    case nsTableChgWidthHeightType::WH_CELL_BOTTOM:
        bRet = pTableNd->GetTable().SetRowHeight( rAktBox,
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
        getIDocumentState().SetModified();
        if( nsTableChgWidthHeightType::WH_FLAG_INSDEL & eType )
            getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}

void SwDoc::ChkBoxNumFormat( SwTableBox& rBox, bool bCallUpdate )
{
    // Optimization: If the Box says it's Text, it remains Text
    const SfxPoolItem* pNumFormatItem = 0;
    if( SfxItemState::SET == rBox.GetFrameFormat()->GetItemState( RES_BOXATR_FORMAT,
        false, &pNumFormatItem ) && GetNumberFormatter()->IsTextFormat(
            static_cast<const SwTableBoxNumFormat*>(pNumFormatItem)->GetValue() ))
        return ;

    SwUndoTableNumFormat* pUndo = 0;

    bool bIsEmptyTextNd;
    bool bChgd = true;
    sal_uInt32 nFormatIdx;
    double fNumber;
    if( rBox.HasNumContent( fNumber, nFormatIdx, bIsEmptyTextNd ) )
    {
        if( !rBox.IsNumberChanged() )
            bChgd = false;
        else
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_AUTOFMT, NULL );
                pUndo = new SwUndoTableNumFormat( rBox );
                pUndo->SetNumFormat( nFormatIdx, fNumber );
            }

            SwTableBoxFormat* pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.GetFrameFormat());
            SfxItemSet aBoxSet( GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

            bool bLockModify = true;
            bool bSetNumberFormat = false;
            const bool bForceNumberFormat = IsInsTableFormatNum() && IsInsTableChangeNumFormat();

            // if the user forced a number format in this cell previously,
            // keep it, unless the user set that she wants the full number
            // format recognition
            if( pNumFormatItem && !bForceNumberFormat )
            {
                sal_uLong nOldNumFormat = static_cast<const SwTableBoxNumFormat*>(pNumFormatItem)->GetValue();
                SvNumberFormatter* pNumFormatr = GetNumberFormatter();

                short nFormatType = pNumFormatr->GetType( nFormatIdx );
                if( nFormatType == pNumFormatr->GetType( nOldNumFormat ) || css::util::NumberFormat::NUMBER == nFormatType )
                {
                    // Current and specified NumFormat match
                    // -> keep old Format
                    nFormatIdx = nOldNumFormat;
                    bSetNumberFormat = true;
                }
                else
                {
                    // Current and specified NumFormat do not match
                    // -> insert as Text
                    bLockModify = bSetNumberFormat = false;
                }
            }

            if( bSetNumberFormat || bForceNumberFormat )
            {
                pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.ClaimFrameFormat());

                aBoxSet.Put( SwTableBoxValue( fNumber ));
                aBoxSet.Put( SwTableBoxNumFormat( nFormatIdx ));
            }

            // It's not enough to only reset the Formula.
            // Make sure that the Text is formatted accordingly
            if( !bSetNumberFormat && !bIsEmptyTextNd && pNumFormatItem )
            {
                // Just resetting Attributes is not enough
                // Make sure that the Text is formatted accordingly
                pBoxFormat->SetFormatAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));
            }

            if( bLockModify ) pBoxFormat->LockModify();
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            if( bLockModify ) pBoxFormat->UnlockModify();

            if( bSetNumberFormat )
                pBoxFormat->SetFormatAttr( aBoxSet );
        }
    }
    else
    {
        // It's not a number
        const SfxPoolItem* pValueItem = 0, *pFormatItem = 0;
        SwTableBoxFormat* pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.GetFrameFormat());
        if( SfxItemState::SET == pBoxFormat->GetItemState( RES_BOXATR_FORMAT,
                false, &pFormatItem ) ||
            SfxItemState::SET == pBoxFormat->GetItemState( RES_BOXATR_VALUE,
                false, &pValueItem ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo( UNDO_TABLE_AUTOFMT, NULL );
                pUndo = new SwUndoTableNumFormat( rBox );
            }

            pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.ClaimFrameFormat());

            // Remove all number formats
            sal_uInt16 nWhich1 = RES_BOXATR_FORMULA;
            if( !bIsEmptyTextNd )
            {
                nWhich1 = RES_BOXATR_FORMAT;

                // Just resetting Attributes is not enough
                // Make sure that the Text is formatted accordingly
                pBoxFormat->SetFormatAttr( *GetDfltAttr( nWhich1 ));
            }
            pBoxFormat->ResetFormatAttr( nWhich1, RES_BOXATR_VALUE );
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

        const SwTableNode* pTableNd = rBox.GetSttNd()->FindTableNode();
        if( bCallUpdate )
        {
            SwTableFormulaUpdate aTableUpdate( &pTableNd->GetTable() );
            getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );

            // TL_CHART2: update charts (when cursor leaves cell and
            // automatic update is enabled)
            if (AUTOUPD_FIELD_AND_CHARTS == GetDocumentSettingManager().getFieldUpdateFlags(true))
                pTableNd->GetTable().UpdateCharts();
        }
        getIDocumentState().SetModified();
    }
}

void SwDoc::SetTableBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoTableNumFormat(rBox, &rSet) );
    }

    SwFrameFormat* pBoxFormat = rBox.ClaimFrameFormat();
    if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
    {
        pBoxFormat->LockModify();
        pBoxFormat->ResetFormatAttr( RES_BOXATR_VALUE );
        pBoxFormat->UnlockModify();
    }
    else if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_VALUE ))
    {
        pBoxFormat->LockModify();
        pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        pBoxFormat->UnlockModify();
    }
    pBoxFormat->SetFormatAttr( rSet );
    getIDocumentState().SetModified();
}

void SwDoc::ClearLineNumAttrs( SwPosition & rPos )
{
    SwPaM aPam(rPos);
    aPam.Move(fnMoveBackward);
    SwContentNode *pNode = aPam.GetContentNode();
    if ( 0 == pNode )
        return ;
    if( pNode->IsTextNode() )
    {
        SwTextNode * pTextNode = pNode->GetTextNode();
        if (pTextNode && pTextNode->IsNumbered()
            && pTextNode->GetText().isEmpty())
        {
            const SfxPoolItem* pFormatItem = 0;
            SfxItemSet rSet( const_cast<SwAttrPool&>(pTextNode->GetDoc()->GetAttrPool()),
                        RES_PARATR_BEGIN, RES_PARATR_END - 1,
                        0);
            pTextNode->SwContentNode::GetAttr( rSet );
            if ( SfxItemState::SET == rSet.GetItemState( RES_PARATR_NUMRULE , false , &pFormatItem ) )
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
                aRegH.RegisterInModify( pTextNode , *pTextNode );
                if ( pUndo )
                    pUndo->AddNode( *pTextNode , false );
                SfxStringItem * pNewItem = static_cast<SfxStringItem*>(pFormatItem->Clone());
                pNewItem->SetValue(OUString());
                rSet.Put( *pNewItem );
                pTextNode->SetAttr( rSet );
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
                            GetTableBox( pSttNd->GetIndex() );

        const SfxPoolItem* pFormatItem = 0;
        const SfxItemSet& rSet = pBox->GetFrameFormat()->GetAttrSet();
        if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT, false, &pFormatItem ) ||
            SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMULA, false ) ||
            SfxItemState::SET == rSet.GetItemState( RES_BOXATR_VALUE, false ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoTableNumFormat(*pBox));
            }

            SwFrameFormat* pBoxFormat = pBox->ClaimFrameFormat();

            // Keep TextFormats!
            sal_uInt16 nWhich1 = RES_BOXATR_FORMAT;
            if( pFormatItem && GetNumberFormatter()->IsTextFormat(
                    static_cast<const SwTableBoxNumFormat*>(pFormatItem)->GetValue() ))
                nWhich1 = RES_BOXATR_FORMULA;
            else
                // Just resetting Attributes is not enough
                // Make sure that the Text is formatted accordingly
                pBoxFormat->SetFormatAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));

            pBoxFormat->ResetFormatAttr( nWhich1, RES_BOXATR_VALUE );
            getIDocumentState().SetModified();
        }
    }
}

/**
 * Copies a Table from the same or another Doc into itself
 * We create a new Table or an existing one is filled with the Content.
 * We either fill in the Content from a certain Box or a certain TableSelection
 *
 * This method is called by edglss.cxx/fecopy.cxx
 */
bool SwDoc::InsCopyOfTable( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTable, bool bCpyName, bool bCorrPos )
{
    bool bRet;

    const SwTableNode* pSrcTableNd = pCpyTable
            ? pCpyTable->GetTableNode()
            : rBoxes[ 0 ]->GetSttNd()->FindTableNode();

    SwTableNode * pInsTableNd = rInsPos.nNode.GetNode().FindTableNode();

    bool const bUndo( GetIDocumentUndoRedo().DoesUndo() );
    if( !pCpyTable && !pInsTableNd )
    {
        SwUndoCpyTable* pUndo = 0;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoCpyTable;
        }

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            bRet = pSrcTableNd->GetTable().MakeCopy( this, rInsPos, rBoxes,
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
                pInsTableNd = GetNodes()[ rInsPos.nNode.GetIndex() - 1 ]->FindTableNode();

                pUndo->SetTableSttIdx( pInsTableNd->GetIndex() );
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
        }
    }
    else
    {
        RedlineMode_t eOld = getIDocumentRedlineAccess().GetRedlineMode();
        if( getIDocumentRedlineAccess().IsRedlineOn() )
      getIDocumentRedlineAccess().SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON |
                                  nsRedlineMode_t::REDLINE_SHOW_INSERT |
                                  nsRedlineMode_t::REDLINE_SHOW_DELETE));

        SwUndoTableCpyTable* pUndo = 0;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoTableCpyTable;
            GetIDocumentUndoRedo().DoUndo(false);
        }

        SwDoc* pCpyDoc = const_cast<SwDoc*>(pSrcTableNd->GetDoc());
        bool bDelCpyDoc = pCpyDoc == this;

        if( bDelCpyDoc )
        {
            // Copy the Table into a temporary Doc
            pCpyDoc = new SwDoc;
            pCpyDoc->acquire();

            SwPosition aPos( SwNodeIndex( pCpyDoc->GetNodes().GetEndOfContent() ));
            if( !pSrcTableNd->GetTable().MakeCopy( pCpyDoc, aPos, rBoxes, true, true ))
            {
                if( pCpyDoc->release() == 0 )
                    delete pCpyDoc;

                if( pUndo )
                {
                    GetIDocumentUndoRedo().DoUndo(bUndo);
                    delete pUndo;
                    pUndo = 0;
                }
                return false;
            }
            aPos.nNode -= 1; // Set to the Table's EndNode
            pSrcTableNd = aPos.nNode.GetNode().FindTableNode();
        }

        const SwStartNode* pSttNd = rInsPos.nNode.GetNode().FindTableBoxStartNode();

        rInsPos.nContent.Assign( 0, 0 );

        // no complex into complex, but copy into or from new model is welcome
        if( ( !pSrcTableNd->GetTable().IsTableComplex() || pInsTableNd->GetTable().IsNewModel() )
            && ( bDelCpyDoc || !rBoxes.empty() ) )
        {
            // Copy the Table "relatively"
            const SwSelBoxes* pBoxes;
            SwSelBoxes aBoxes;

            if( bDelCpyDoc )
            {
                SwTableBox* pBox = pInsTableNd->GetTable().GetTableBox(
                                        pSttNd->GetIndex() );
                OSL_ENSURE( pBox, "Box is not in this Table" );
                aBoxes.insert( pBox );
                pBoxes = &aBoxes;
            }
            else
                pBoxes = &rBoxes;

            // Copy Table to the selected Lines
            bRet = pInsTableNd->GetTable().InsTable( pSrcTableNd->GetTable(),
                                                        *pBoxes, pUndo );
        }
        else
        {
            SwNodeIndex aNdIdx( *pSttNd, 1 );
            bRet = pInsTableNd->GetTable().InsTable( pSrcTableNd->GetTable(),
                                                    aNdIdx, pUndo );
        }

        if( bDelCpyDoc )
        {
            if( pCpyDoc->release() == 0 )
                delete pCpyDoc;
        }

        if( pUndo )
        {
            // If the Table could not be copied, delete the Undo object
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
        getIDocumentRedlineAccess().SetRedlineMode( eOld );
    }

    if( bRet )
    {
        getIDocumentState().SetModified();
        getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
    }
    return bRet;
}

bool SwDoc::_UnProtectTableCells( SwTable& rTable )
{
    bool bChgd = false;
    SwUndoAttrTable *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoAttrTable( *rTable.GetTableNode() )
        :   0;

    SwTableSortBoxes& rSrtBox = rTable.GetTabSortBoxes();
    for (size_t i = rSrtBox.size(); i; )
    {
        SwFrameFormat *pBoxFormat = rSrtBox[ --i ]->GetFrameFormat();
        if( pBoxFormat->GetProtect().IsContentProtected() )
        {
            pBoxFormat->ResetFormatAttr( RES_PROTECT );
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

bool SwDoc::UnProtectCells( const OUString& rName )
{
    bool bChgd = false;
    SwTableFormat* pFormat = FindTableFormatByName( rName );
    if( pFormat )
    {
        bChgd = _UnProtectTableCells( *SwTable::FindTable( pFormat ) );
        if( bChgd )
            getIDocumentState().SetModified();
    }

    return bChgd;
}

bool SwDoc::UnProtectCells( const SwSelBoxes& rBoxes )
{
    bool bChgd = false;
    if( !rBoxes.empty() )
    {
        SwUndoAttrTable *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
                ? new SwUndoAttrTable( *rBoxes[0]->GetSttNd()->FindTableNode() )
                : 0;

        std::map<SwFrameFormat*, SwTableBoxFormat*> aFormatsMap;
        for (size_t i = rBoxes.size(); i; )
        {
            SwTableBox* pBox = rBoxes[ --i ];
            SwFrameFormat* pBoxFormat = pBox->GetFrameFormat();
            if( pBoxFormat->GetProtect().IsContentProtected() )
            {
                std::map<SwFrameFormat*, SwTableBoxFormat*>::const_iterator const it =
                    aFormatsMap.find(pBoxFormat);
                if (aFormatsMap.end() != it)
                    pBox->ChgFrameFormat(it->second);
                else
                {
                    SwTableBoxFormat *const pNewBoxFormat(
                        static_cast<SwTableBoxFormat*>(pBox->ClaimFrameFormat()));
                    pNewBoxFormat->ResetFormatAttr( RES_PROTECT );
                    aFormatsMap.insert(std::make_pair(pBoxFormat, pNewBoxFormat));
                }
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
    }
    return bChgd;
}

bool SwDoc::UnProtectTables( const SwPaM& rPam )
{
    GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

    bool bChgd = false, bHasSel = rPam.HasMark() ||
                                    rPam.GetNext() != &rPam;
    SwFrameFormats& rFormats = *GetTableFrameFormats();
    SwTable* pTable;
    const SwTableNode* pTableNd;
    for( auto n = rFormats.size(); n ; )
        if( 0 != (pTable = SwTable::FindTable( rFormats[ --n ] )) &&
            0 != (pTableNd = pTable->GetTableNode() ) &&
            pTableNd->GetNodes().IsDocNodes() )
        {
            sal_uLong nTableIdx = pTableNd->GetIndex();

            // Check whether the Table is within the Selection
            if( bHasSel )
            {
                bool bFound = false;
                SwPaM* pTmp = const_cast<SwPaM*>(&rPam);
                do {
                    const SwPosition *pStt = pTmp->Start(),
                                    *pEnd = pTmp->End();
                    bFound = pStt->nNode.GetIndex() < nTableIdx &&
                            nTableIdx < pEnd->nNode.GetIndex();

                } while( !bFound && &rPam != ( pTmp = static_cast<SwPaM*>(pTmp->GetNext()) ) );
                if( !bFound )
                    continue; // Continue searching
            }

            // Lift the protection
            bChgd |= _UnProtectTableCells( *pTable );
        }

    GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    if( bChgd )
        getIDocumentState().SetModified();

    return bChgd;
}

bool SwDoc::HasTableAnyProtection( const SwPosition* pPos,
                                 const OUString* pTableName,
                                 bool* pFullTableProtection )
{
    bool bHasProtection = false;
    SwTable* pTable = 0;
    if( pTableName )
        pTable = SwTable::FindTable( FindTableFormatByName( *pTableName ) );
    else if( pPos )
    {
        SwTableNode* pTableNd = pPos->nNode.GetNode().FindTableNode();
        if( pTableNd )
            pTable = &pTableNd->GetTable();
    }

    if( pTable )
    {
        SwTableSortBoxes& rSrtBox = pTable->GetTabSortBoxes();
        for (size_t i = rSrtBox.size(); i; )
        {
            SwFrameFormat *pBoxFormat = rSrtBox[ --i ]->GetFrameFormat();
            if( pBoxFormat->GetProtect().IsContentProtected() )
            {
                if( !bHasProtection )
                {
                    bHasProtection = true;
                    if( !pFullTableProtection )
                        break;
                    *pFullTableProtection = true;
                }
            }
            else if( bHasProtection && pFullTableProtection )
            {
                *pFullTableProtection = false;
                break;
            }
        }
    }
    return bHasProtection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
