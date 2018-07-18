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

#include <memory>
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
#include <strings.hrc>
#include <docsh.hxx>
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
        aLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
        aLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    SvxBoxItem aBox(RES_BOX);
    aBox.SetAllDistances(55);
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
        sal_uInt8 const nId, SwTableAutoFormat const*const pAutoFormat = nullptr)
{
    DfltBoxAttrMap_t * pMap = rBoxFormatArr[ nId ];
    if (!pMap)
    {
        pMap = new DfltBoxAttrMap_t;
        rBoxFormatArr[ nId ] = pMap;
    }

    SwTableBoxFormat* pNewTableBoxFormat = nullptr;
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
            pAutoFormat->UpdateToSet( nId, const_cast<SfxItemSet&>(static_cast<SfxItemSet const &>(pNewTableBoxFormat->GetAttrSet())),
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
            pBoxFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE,
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
        rAutoFormat.UpdateToSet( nId, const_cast<SfxItemSet&>(static_cast<SfxItemSet const &>(pBoxFormat->GetAttrSet())),
                                SwTableAutoFormat::UPDATE_BOX,
                                rDoc.GetNumberFormatter( ) );
        if( USHRT_MAX != nCols )
            pBoxFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE,
                                            USHRT_MAX / nCols, 0 ));
        rBoxFormatArr[ nId ] = pBoxFormat;
    }
    return rBoxFormatArr[nId];
}

SwTableNode* SwDoc::IsIdxInTable(const SwNodeIndex& rIdx)
{
    SwTableNode* pTableNd = nullptr;
    sal_uLong nIndex = rIdx.GetIndex();
    do {
        SwNode* pNd = static_cast<SwNode*>(GetNodes()[ nIndex ]->StartOfSectionNode());
        if( nullptr != ( pTableNd = pNd->GetTableNode() ) )
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
    SwTableBox *pPrvBox = nullptr, *pNxtBox = nullptr;
    if( !pLine->GetTabBoxes().empty() )
    {
        if( nInsPos < pLine->GetTabBoxes().size() )
        {
            if( nullptr == (pPrvBox = pLine->FindPreviousBox( pTableNd->GetTable(),
                            pLine->GetTabBoxes()[ nInsPos ] )))
                pPrvBox = pLine->FindPreviousBox( pTableNd->GetTable() );
        }
        else
        {
            if( nullptr == (pNxtBox = pLine->FindNextBox( pTableNd->GetTable(),
                            pLine->GetTabBoxes().back() )))
                pNxtBox = pLine->FindNextBox( pTableNd->GetTable() );
        }
    }
    else if( nullptr == ( pNxtBox = pLine->FindNextBox( pTableNd->GetTable() )))
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
        SwStartNode* pSttNd = new SwStartNode( aEndIdx, SwNodeType::Start,
                                                SwTableBoxStartNode );
        pSttNd->m_pStartOfSection = pTableNd;
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
    assert(nRows && "Table without line?");
    assert(nCols && "Table without rows?");

    {
        // Do not copy into Footnotes!
        if( rPos.nNode < GetNodes().GetEndOfInserts().GetIndex() &&
            rPos.nNode >= GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return nullptr;

        // If the ColumnArray has a wrong count, ignore it!
        if( pColArr &&
            static_cast<size_t>(nCols + ( text::HoriOrientation::NONE == eAdjust ? 2 : 1 )) != pColArr->size() )
            pColArr = nullptr;
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

    bool bDfltBorders( rInsTableOpts.mnInsMode & SwInsertTableFlags::DefaultBorder );

    if( (rInsTableOpts.mnInsMode & SwInsertTableFlags::Headline) && (1 != nRows || !bDfltBorders) )
        pHeadColl = getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TABLE_HDLN );

    const sal_uInt16 nRowsToRepeat =
            SwInsertTableFlags::Headline == (rInsTableOpts.mnInsMode & SwInsertTableFlags::Headline) ?
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
        bCalledFromShell ? &pContentNd->GetSwAttrSet() : nullptr );

    // Create the Box/Line/Table construct
    SwTableLineFormat* pLineFormat = MakeTableLineFormat();
    SwTableFormat* pTableFormat = MakeTableFrameFormat( aTableName, GetDfltFrameFormat() );

    /* If the node to insert the table at is a context node and has a
       non-default FRAMEDIR propagate it to the table. */
    if (pContentNd)
    {
        const SwAttrSet & aNdSet = pContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = nullptr;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != nullptr)
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
            sal_uInt16 nFrameWidth = nLastPos;
            nLastPos = (*pColArr)[ pColArr->size()-2 ];
            pTableFormat->SetFormatAttr( SvxLRSpaceItem( nSttPos, nFrameWidth - nLastPos, 0, 0, RES_LR_SPACE ) );
        }
        nWidth = nLastPos - nSttPos;
    }
    else
    {
        nWidth /= nCols;
        nWidth *= nCols; // to avoid rounding problems
    }
    pTableFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nWidth ));
    if( !(rInsTableOpts.mnInsMode & SwInsertTableFlags::SplitLayout) )
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
    SwTableBoxFormat* pBoxFormat = nullptr;
    if( !bDfltBorders && !pTAFormat )
    {
        pBoxFormat = MakeTableBoxFormat();
        pBoxFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, USHRT_MAX / nCols, 0 ));
    }
    else
    {
        const sal_uInt16 nBoxArrLen = pTAFormat ? 16 : 4;
        aBoxFormatArr.resize( nBoxArrLen, nullptr );
    }
    SfxItemSet aCharSet( GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1>{} );

    SwNodeIndex aNdIdx( *pTableNd, 1 ); // Set to StartNode of first Box
    SwTableLines& rLines = rNdTable.GetTabLines();
    for( sal_uInt16 n = 0; n < nRows; ++n )
    {
        SwTableLine* pLine = new SwTableLine( pLineFormat, nCols, nullptr );
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
                                        SwTableAutoFormat::UPDATE_CHAR, nullptr );
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
                if( pBoxF->GetFrameSize().GetWidth() != nWidth )
                {
                    if( pBoxF->HasWriterListeners() ) // Create new Format
                    {
                        SwTableBoxFormat *pNewFormat = MakeTableBoxFormat();
                        *pNewFormat = *pBoxF;
                        pBoxF = pNewFormat;
                    }
                    pBoxF->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nWidth ));
                }
            }

            SwTableBox *pBox = new SwTableBox( pBoxF, aNdIdx, pLine);
            rBoxes.insert( rBoxes.begin() + i, pBox );
            aNdIdx += 3; // StartNode, TextNode, EndNode  == 3 Nodes
        }
    }
    // Insert Frames
    GetNodes().GoNext( &aNdIdx ); // Go to the next ContentNode
    pTableNd->MakeFrames( &aNdIdx );

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
        return nullptr;

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
            SwStartNode* pSttNd = new SwStartNode( aIdx, SwNodeType::Start,
                                                    SwTableBoxStartNode );
            pSttNd->m_pStartOfSection = pTableNd;

            SwTextNode * pTmpNd = new SwTextNode( aIdx, pTextColl );

            // #i60422# Propagate some more attributes.
            const SfxPoolItem* pItem = nullptr;
            if ( nullptr != pAttrSet )
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
                return nullptr;
    }

    // Save first node in the selection if it is a context node
    SwContentNode * pSttContentNd = pStt->nNode.GetNode().GetContentNode();

    SwPaM aOriginal( *pStt, *pEnd );
    pStt = aOriginal.GetMark();
    pEnd = aOriginal.GetPoint();

    SwUndoTextToTable* pUndo = nullptr;
    if( GetIDocumentUndoRedo().DoesUndo() )
    {
        GetIDocumentUndoRedo().StartUndo( SwUndoId::TEXTTOTABLE, nullptr );
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
            --const_cast<SwNodeIndex&>(pEnd->nNode);
            const_cast<SwIndex&>(pEnd->nContent).Assign(
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

    GetIDocumentUndoRedo().DoUndo( nullptr != pUndo );

    // Create the Box/Line/Table construct
    SwTableBoxFormat* pBoxFormat = MakeTableBoxFormat();
    SwTableLineFormat* pLineFormat = MakeTableLineFormat();
    SwTableFormat* pTableFormat = MakeTableFrameFormat( GetUniqueTableName(), GetDfltFrameFormat() );

    // All Lines have a left-to-right Fill Order
    pLineFormat->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ));
    // The Table's SSize is USHRT_MAX
    pTableFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, USHRT_MAX ));
    if( !(rInsTableOpts.mnInsMode & SwInsertTableFlags::SplitLayout) )
        pTableFormat->SetFormatAttr( SwFormatLayoutSplit( false ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttContentNd)
    {
        const SwAttrSet & aNdSet = pSttContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = nullptr;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != nullptr)
        {
            pTableFormat->SetFormatAttr( *pItem );
        }
    }

    //Resolves: tdf#87977, tdf#78599, disable broadcasting modifications
    //until after RegisterToFormat is completed
    bool bEnableSetModified = getIDocumentState().IsEnableSetModified();
    getIDocumentState().SetEnableSetModified(false);

    SwTableNode* pTableNd = GetNodes().TextToTable(
            aRg, cCh, pTableFormat, pLineFormat, pBoxFormat,
            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ), pUndo );

    SwTable& rNdTable = pTableNd->GetTable();

    const sal_uInt16 nRowsToRepeat =
            SwInsertTableFlags::Headline == (rInsTableOpts.mnInsMode & SwInsertTableFlags::Headline) ?
            rInsTableOpts.mnRowsToRepeat :
            0;
    rNdTable.SetRowsToRepeat(nRowsToRepeat);

    bool bUseBoxFormat = false;
    if( !pBoxFormat->HasWriterListeners() )
    {
        // The Box's Formats already have the right size, we must only set
        // the right Border/AutoFormat.
        bUseBoxFormat = true;
        pTableFormat->SetFormatAttr( pBoxFormat->GetFrameSize() );
        delete pBoxFormat;
        eAdjust = text::HoriOrientation::NONE;
    }

    // Set Orientation in the Table's Format
    pTableFormat->SetFormatAttr( SwFormatHoriOrient( 0, eAdjust ) );
    rNdTable.RegisterToFormat(*pTableFormat);

    if( pTAFormat || ( rInsTableOpts.mnInsMode & SwInsertTableFlags::DefaultBorder) )
    {
        sal_uInt8 nBoxArrLen = pTAFormat ? 16 : 4;
        std::unique_ptr< DfltBoxAttrList_t > aBoxFormatArr1;
        std::unique_ptr< std::vector<SwTableBoxFormat*> > aBoxFormatArr2;
        if( bUseBoxFormat )
        {
            aBoxFormatArr1.reset(new DfltBoxAttrList_t( nBoxArrLen, nullptr ));
        }
        else
        {
            aBoxFormatArr2.reset(new std::vector<SwTableBoxFormat*>( nBoxArrLen, nullptr ));
        }

        SfxItemSet aCharSet( GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1>{} );

        SwHistory* pHistory = pUndo ? &pUndo->GetHistory() : nullptr;

        SwTableBoxFormat *pBoxF = nullptr;
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
                        bChgSz = nullptr == (*aBoxFormatArr2)[ nId ];
                        pBoxF = ::lcl_CreateAFormatBoxFormat( *this, *aBoxFormatArr2,
                                                *pTAFormat, USHRT_MAX, nId );
                    }

                    // Set Paragraph/Character Attributes if needed
                    if( pTAFormat->IsFont() || pTAFormat->IsJustify() )
                    {
                        aCharSet.ClearItem();
                        pTAFormat->UpdateToSet( nId, aCharSet,
                                            SwTableAutoFormat::UPDATE_CHAR, nullptr );
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
                        bChgSz = nullptr == (*aBoxFormatArr2)[ nId ];
                        pBoxF = ::lcl_CreateDfltBoxFormat( *this, *aBoxFormatArr2,
                                                        USHRT_MAX, nId );
                    }
                }

                if( !bUseBoxFormat )
                {
                    if( bChgSz )
                        pBoxF->SetFormatAttr( pBox->GetFrameFormat()->GetFrameSize() );
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
    aNode2Layout.RestoreUpperFrames( GetNodes(), nIdx, nIdx + 1 );

    {
        SwPaM& rTmp = const_cast<SwPaM&>(rRange); // Point always at the Start
        rTmp.DeleteMark();
        rTmp.GetPoint()->nNode = *pTableNd;
        SwContentNode* pCNd = GetNodes().GoNext( &rTmp.GetPoint()->nNode );
        rTmp.GetPoint()->nContent.Assign( pCNd, 0 );
    }

    if( pUndo )
    {
        GetIDocumentUndoRedo().EndUndo( SwUndoId::TEXTTOTABLE, nullptr );
    }

    getIDocumentState().SetEnableSetModified(bEnableSetModified);
    getIDocumentState().SetModified();
    getIDocumentFieldsAccess().SetFieldsDirty(true, nullptr, 0);
    return &rNdTable;
}

static void lcl_RemoveBreaks(SwContentNode & rNode, SwTableFormat *const pTableFormat)
{
    // delete old layout frames, new ones need to be created...
    rNode.DelFrames();

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
                    nullptr, nBoxes, nMaxBoxes - nBoxes);

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
                    SwFormatFrameSize(ATT_VAR_SIZE, (*pPositions)[n] - nLastPos));
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
        rBoxFormat.SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nLastPos ));
    }
    else
    {
        size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
        rBoxFormat.SetFormatAttr(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));
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
        return nullptr;

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
            SwTextFrameInfo aFInfo( static_cast<SwTextFrame*>(pTextNd->getLayoutFrame( pTextNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() )) );
            if( aFInfo.IsOneLine() ) // only makes sense in this case
            {
                OUString const& rText(pTextNd->GetText());
                for (sal_Int32 nChPos = 0; nChPos < rText.getLength(); ++nChPos)
                {
                    if (rText[nChPos] == cCh)
                    {
                        // sw_redlinehide: no idea if this makes any sense...
                        TextFrameIndex const nPos(aFInfo.GetFrame()->MapModelToView(pTextNd, nChPos));
                        aPosArr.push_back( static_cast<sal_uInt16>(
                            aFInfo.GetCharPos(nPos+TextFrameIndex(1), false)) );
                    }
                }

                aPosArr.push_back(
                                static_cast<sal_uInt16>(aFInfo.GetFrame()->IsVertical() ?
                                aFInfo.GetFrame()->getFramePrintArea().Bottom() :
                                aFInfo.GetFrame()->getFramePrintArea().Right()) );

            }
        }

        lcl_RemoveBreaks(*pTextNd, (0 == nLines) ? pTableFormat : nullptr);

        // Set the TableNode as StartNode for all TextNodes in the Table
        pTextNd->m_pStartOfSection = pTableNd;

        SwTableLine* pLine = new SwTableLine( pLineFormat, 1, nullptr );
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
                    pSttNd = new SwStartNode( aTmpIdx, SwNodeType::Start,
                                                SwTableBoxStartNode );
                    new SwEndNode( aCntPos.nNode, *pSttNd );
                    pNewNd->m_pStartOfSection = pSttNd;

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

        pSttNd = new SwStartNode( aCntPos.nNode, SwNodeType::Start, SwTableBoxStartNode );
        const SwNodeIndex aTmpIdx( aCntPos.nNode, 1 );
        new SwEndNode( aTmpIdx, *pSttNd  );
        pTextNd->m_pStartOfSection = pSttNd;

        pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
        pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

    lcl_BalanceTable(rTable, nMaxBoxes, *pTableNd, *pBoxFormat, *pTextColl,
            pUndo, &aPosArr);
    lcl_SetTableBoxWidths(rTable, nMaxBoxes, *pBoxFormat, *pDoc, &aPosArr);

    return pTableNd;
}

const SwTable* SwDoc::TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes )
{
    if (rTableNodes.empty())
        return nullptr;

    const std::vector<SwNodeRange>& rFirstRange = *rTableNodes.begin();

    if (rFirstRange.empty())
        return nullptr;

    const std::vector<SwNodeRange>& rLastRange = *rTableNodes.rbegin();

    if (rLastRange.empty())
        return nullptr;

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
            --const_cast<SwNodeIndex&>(pEnd->nNode);
            const_cast<SwIndex&>(pEnd->nContent).Assign(
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
    pTableFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, USHRT_MAX ));

    /* If the first node in the selection is a context node and if it
       has an item FRAMEDIR set (no default) propagate the item to the
       replacing table. */
    if (pSttContentNd)
    {
        const SwAttrSet & aNdSet = pSttContentNd->GetSwAttrSet();
        const SfxPoolItem *pItem = nullptr;

        if (SfxItemState::SET == aNdSet.GetItemState( RES_FRAMEDIR, true, &pItem )
            && pItem != nullptr)
        {
            pTableFormat->SetFormatAttr( *pItem );
        }
    }

    //Resolves: tdf#87977, tdf#78599, disable broadcasting modifications
    //until after RegisterToFormat is completed
    bool bEnableSetModified = getIDocumentState().IsEnableSetModified();
    getIDocumentState().SetEnableSetModified(false);

    SwTableNode* pTableNd = GetNodes().TextToTable(
            rTableNodes, pTableFormat, pLineFormat, pBoxFormat );

    SwTable& rNdTable = pTableNd->GetTable();
    rNdTable.RegisterToFormat(*pTableFormat);

    if( !pBoxFormat->HasWriterListeners() )
    {
        // The Box's Formats already have the right size, we must only set
        // the right Border/AutoFormat.
        pTableFormat->SetFormatAttr( pBoxFormat->GetFrameSize() );
        delete pBoxFormat;
    }

    sal_uLong nIdx = pTableNd->GetIndex();
    aNode2Layout.RestoreUpperFrames( GetNodes(), nIdx, nIdx + 1 );

    getIDocumentState().SetEnableSetModified(bEnableSetModified);
    getIDocumentState().SetModified();
    getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
    return &rNdTable;
}

SwNodeRange * SwNodes::ExpandRangeForTableBox(const SwNodeRange & rRange)
{
    SwNodeRange * pResult = nullptr;
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
    while (pNode->IsEndNode() && aIndex < Count() - 1)
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
        assert(!rBoxes.empty()); // ensured by convertToTable
        size_t const nMissing = nMaxBoxes - rBoxes.size();
        if (nMissing)
        {
            // default width for box at the end of an incomplete line
            SwTableBoxFormat *const pNewFormat = rDoc.MakeTableBoxFormat();
            size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
            pNewFormat->SetFormatAttr( SwFormatFrameSize(ATT_VAR_SIZE,
                        nWidth * (nMissing + 1)) );
            pNewFormat->Add(rBoxes.back());
        }
    }
    size_t nWidth = nMaxBoxes ? USHRT_MAX / nMaxBoxes : USHRT_MAX;
    // default width for all boxes not at the end of an incomplete line
    rBoxFormat.SetFormatAttr(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));
}

SwTableNode* SwNodes::TextToTable( const SwNodes::TableRanges_t & rTableNodes,
                                    SwTableFormat* pTableFormat,
                                    SwTableLineFormat* pLineFormat,
                                    SwTableBoxFormat* pBoxFormat  )
{
    if( rTableNodes.empty() )
        return nullptr;

    SwTableNode * pTableNd = new SwTableNode( rTableNodes.begin()->begin()->aStart );
    //insert the end node after the last text node
    SwNodeIndex aInsertIndex( rTableNodes.rbegin()->rbegin()->aEnd );
    ++aInsertIndex;

    //!! ownership will be transferred in c-tor to SwNodes array.
    //!! Thus no real problem here...
    new SwEndNode( aInsertIndex, *pTableNd );

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
                    (0 == nLines) ? pTableFormat : nullptr);
        }
    }

    std::vector<std::vector < SwNodeRange > >::const_iterator aRowIter = rTableNodes.begin();
    for( nLines = 0, nBoxes = 0;
        aRowIter != rTableNodes.end();
        ++aRowIter, nLines++, nBoxes = 0 )
    {
        SwTableLine* pLine = new SwTableLine( pLineFormat, 1, nullptr );
        rTable.GetTabLines().insert(rTable.GetTabLines().begin() + nLines, pLine);

        std::vector< SwNodeRange >::const_iterator aCellIter = aRowIter->begin();

        for( ; aCellIter != aRowIter->end(); ++aCellIter )
        {
                const SwNodeIndex aTmpIdx( aCellIter->aStart, 0 );

               SwNodeIndex aCellEndIdx(aCellIter->aEnd);
               ++aCellEndIdx;
               SwStartNode* pSttNd = new SwStartNode( aTmpIdx, SwNodeType::Start,
                                            SwTableBoxStartNode );

                // Quotation of http://nabble.documentfoundation.org/Some-strange-lines-by-taking-a-look-at-the-bt-of-fdo-51916-tp3994561p3994639.html
                // SwNode's constructor adds itself to the same SwNodes array as the other node (pSttNd).
                // So this statement is only executed for the side-effect.
                new SwEndNode( aCellEndIdx, *pSttNd );

                //set the start node on all node of the current cell
                SwNodeIndex aCellNodeIdx = aCellIter->aStart;
                for(;aCellNodeIdx <= aCellIter->aEnd; ++aCellNodeIdx )
                {
                    aCellNodeIdx.GetNode().m_pStartOfSection = pSttNd;
                    //skip start/end node pairs
                    if( aCellNodeIdx.GetNode().IsStartNode() )
                        aCellNodeIdx.Assign(*aCellNodeIdx.GetNode().EndOfSectionNode());
                }

                // assign Section to the Box
                pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
                pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin() + nBoxes++, pBox );
        }
        if( nMaxBoxes < nBoxes )
            nMaxBoxes = nBoxes;
    }

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
    SwUndoTableToText* pUndo = nullptr;
    SwNodeRange* pUndoRg = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoRg = new SwNodeRange( aRg.aStart, -1, aRg.aEnd, +1 );
        pUndo = new SwUndoTableToText( pTableNd->GetTable(), cCh );
    }

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_eFlags = TBL_BOXNAME;
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
struct DelTabPara
{
    SwTextNode* pLastNd;
    SwNodes& rNds;
    SwUndoTableToText* pUndo;
    sal_Unicode cCh;

    DelTabPara( SwNodes& rNodes, sal_Unicode cChar, SwUndoTableToText* pU ) :
        pLastNd(nullptr), rNds( rNodes ), pUndo( pU ), cCh( cChar ) {}
};

// Forward declare so that the Lines and Boxes can use recursion
static void lcl_DelBox( SwTableBox* pBox, DelTabPara* pDelPara );

static void lcl_DelLine( SwTableLine* pLine, DelTabPara* pPara )
{
    assert(pPara && "The parameters are missing!");
    DelTabPara aPara( *pPara );
    for( SwTableBoxes::iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
        lcl_DelBox(*it, &aPara );
    if( pLine->GetUpper() ) // Is there a parent Box?
        // Return the last TextNode
        pPara->pLastNd = aPara.pLastNd;
}

static void lcl_DelBox( SwTableBox* pBox, DelTabPara* pDelPara )
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
        const SwTextNode* pCurTextNd = nullptr;
        if (T2T_PARA != pDelPara->cCh && pDelPara->pLastNd)
            pCurTextNd = aDelRg.aStart.GetNode().GetTextNode();
        if (nullptr != pCurTextNd)
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
    if (rRange.aStart.GetIndex() >= rRange.aEnd.GetIndex())
        return false;
    SwTableNode *const pTableNd(rRange.aStart.GetNode().GetTableNode());
    if (nullptr == pTableNd ||
        &rRange.aEnd.GetNode() != pTableNd->EndOfSectionNode() )
        return false;

    // If the Table was alone in a Section, create the Frames via the Table's Upper
    SwNode2Layout* pNode2Layout = nullptr;
    SwNodeIndex aFrameIdx( rRange.aStart );
    SwNode* pFrameNd = FindPrvNxtFrameNode( aFrameIdx, &rRange.aEnd.GetNode() );
    if( !pFrameNd )
        // Collect all Uppers
        pNode2Layout = new SwNode2Layout( *pTableNd );

    // Delete the Frames
    pTableNd->DelFrames();

    // "Delete" the Table and merge all Lines/Boxes
    DelTabPara aDelPara( *this, cCh, pUndo );
    for( SwTableLine *pLine : pTableNd->m_pTable->GetTabLines() )
        lcl_DelLine( pLine, &aDelPara );

    // We just created a TextNode with fitting separator for every TableLine.
    // Now we only need to delete the TableSection and create the Frames for the
    // new TextNode.
    SwNodeRange aDelRg( rRange.aStart, rRange.aEnd );

    // If the Table has PageDesc/Break Attributes, carry them over to the
    // first Text Node
    {
        // What about UNDO?
        const SfxItemSet& rTableSet = pTableNd->m_pTable->GetFrameFormat()->GetAttrSet();
        const SfxPoolItem *pBreak, *pDesc;
        if( SfxItemState::SET != rTableSet.GetItemState( RES_PAGEDESC, false, &pDesc ))
            pDesc = nullptr;
        if( SfxItemState::SET != rTableSet.GetItemState( RES_BREAK, false, &pBreak ))
            pBreak = nullptr;

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
    if( !pFrameNd )
    {
        pNode2Layout->RestoreUpperFrames( *this,
                        aDelRg.aStart.GetIndex(), aDelRg.aEnd.GetIndex() );
        delete pNode2Layout;
    }
    else
    {
        SwContentNode *pCNd;
        SwSectionNode *pSNd;
        while( aDelRg.aStart.GetIndex() < nEnd )
        {
            if( nullptr != ( pCNd = aDelRg.aStart.GetNode().GetContentNode()))
            {
                if( pFrameNd->IsContentNode() )
                    static_cast<SwContentNode*>(pFrameNd)->MakeFrames( *pCNd );
                else if( pFrameNd->IsTableNode() )
                    static_cast<SwTableNode*>(pFrameNd)->MakeFrames( aDelRg.aStart );
                else if( pFrameNd->IsSectionNode() )
                    static_cast<SwSectionNode*>(pFrameNd)->MakeFrames( aDelRg.aStart );
                pFrameNd = pCNd;
            }
            else if( nullptr != ( pSNd = aDelRg.aStart.GetNode().GetSectionNode()))
            {
                if( !pSNd->GetSection().IsHidden() && !pSNd->IsContentHidden() )
                {
                    pSNd->MakeFrames( &aFrameIdx, &aDelRg.aEnd );
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
            ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
            nStt <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nEnd )
        {
            pFormat->MakeFrames();
        }
    }

    return true;
}

/**
 * Inserting Columns/Rows
 */
void SwDoc::InsertCol( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    if( !::CheckSplitCells( rCursor, nCnt + 1, SwTableSearchType::Col ) )
        return;

    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    ::GetTableSel( rCursor, aBoxes, SwTableSearchType::Col );

    if( !aBoxes.empty() )
        InsertCol( aBoxes, nCnt, bBehind );
}

bool SwDoc::InsertCol( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    SwTable& rTable = pTableNd->GetTable();
    if( dynamic_cast<const SwDDETable*>( &rTable) !=  nullptr)
        return false;

    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( SwUndoId::TABLE_INSCOL, rBoxes, *pTableNd,
                                     0, 0, nCnt, bBehind, false );
        aTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &rTable );
        aMsgHint.m_eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        bRet = rTable.InsertCol( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            getIDocumentState().SetModified();
            ::ClearFEShellTabCols(*this, nullptr);
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
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

void SwDoc::InsertRow( const SwCursor& rCursor, sal_uInt16 nCnt, bool bBehind )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, SwTableSearchType::Row );

    if( !aBoxes.empty() )
        InsertRow( aBoxes, nCnt, bBehind );
}

bool SwDoc::InsertRow( const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    SwTable& rTable = pTableNd->GetTable();
    if( dynamic_cast<const SwDDETable*>( &rTable) !=  nullptr)
        return false;

    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( SwUndoId::TABLE_INSROW,rBoxes, *pTableNd,
                                     0, 0, nCnt, bBehind, false );
        aTmpLst.insert( rTable.GetTabSortBoxes() );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &rTable );
        aMsgHint.m_eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        bRet = rTable.InsertRow( this, rBoxes, nCnt, bBehind );
        if (bRet)
        {
            getIDocumentState().SetModified();
            ::ClearFEShellTabCols(*this, nullptr);
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
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
void SwDoc::DeleteRow( const SwCursor& rCursor )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, SwTableSearchType::Row );
    if( ::HasProtectedCells( aBoxes ))
        return;

    // Remove the Cursor from the to-be-deleted Section.
    // The Cursor is placed after the table, except for
    //  - when there's another Line, we place it in that one
    //  - when a Line precedes it, we place it in that one
    {
        SwTableNode* pTableNd = rCursor.GetNode().FindTableNode();

        if(dynamic_cast<const SwDDETable*>( & pTableNd->GetTable()) !=  nullptr)
            return;

        // Find all Boxes/Lines
        FndBox_ aFndBox( nullptr, nullptr );
        {
            FndPara aPara( aBoxes, &aFndBox );
            ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
        }

        if( aFndBox.GetLines().empty() )
            return;

        SwEditShell* pESh = GetEditShell();
        if( pESh )
        {
            pESh->KillPams();
            // FIXME: actually we should be iterating over all Shells!
        }

        FndBox_* pFndBox = &aFndBox;
        while( 1 == pFndBox->GetLines().size() &&
                1 == pFndBox->GetLines().front()->GetBoxes().size() )
        {
            FndBox_ *const pTmp = pFndBox->GetLines().front()->GetBoxes()[0].get();
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
    GetIDocumentUndoRedo().StartUndo(SwUndoId::ROW_DELETE, nullptr);
    DeleteRowCol( aBoxes );
    GetIDocumentUndoRedo().EndUndo(SwUndoId::ROW_DELETE, nullptr);
}

void SwDoc::DeleteCol( const SwCursor& rCursor )
{
    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    GetTableSel( rCursor, aBoxes, SwTableSearchType::Col );
    if( ::HasProtectedCells( aBoxes ))
        return;

    // The Cursors need to be removed from the to-be-deleted range.
    // Always place them after/on top of the Table; they are always set
    // to the old position via the document position.
    SwEditShell* pESh = GetEditShell();
    if( pESh )
    {
        const SwNode* pNd = rCursor.GetNode().FindTableBoxStartNode();
        pESh->ParkCursor( SwNodeIndex( *pNd ) );
    }

    // Thus delete the Columns
    GetIDocumentUndoRedo().StartUndo(SwUndoId::COL_DELETE, nullptr);
    DeleteRowCol( aBoxes, true );
    GetIDocumentUndoRedo().EndUndo(SwUndoId::COL_DELETE, nullptr);
}

bool SwDoc::DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn )
{
    if( ::HasProtectedCells( rBoxes ))
        return false;

    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    if( dynamic_cast<const SwDDETable*>( &pTableNd->GetTable() ) !=  nullptr)
        return false;

    ::ClearFEShellTabCols(*this, nullptr);
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
            if( ! aSavePaM.Move( fnMoveForward, GoInNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTableNd );
                aSavePaM.Move( fnMoveBackward, GoInNode );
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
            if( ! aSavePaM.Move( fnMoveForward, GoInNode ) )
            {
                *aSavePaM.GetMark() = SwPosition( *pTableNd );
                aSavePaM.Move( fnMoveBackward, GoInNode );
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

            pTableNd->DelFrames();
            getIDocumentContentOperations().DeleteSection( pTableNd );
        }

        GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

        getIDocumentState().SetModified();
        getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );

        return true;
    }

    SwUndoTableNdsChg* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( SwUndoId::TABLE_DELBOX, aSelBoxes, *pTableNd,
                                     nMin, nMax, 0, false, false );
    }

    bool bRet(false);
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
        aMsgHint.m_eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        if (rTable.IsNewModel())
        {
            if (bColumn)
                rTable.PrepareDeleteCol( nMin, nMax );
            rTable.FindSuperfluousRows( aSelBoxes );
            if (pUndo)
                pUndo->ReNewBoxes( aSelBoxes );
        }
        bRet = rTable.DeleteSel( this, aSelBoxes, nullptr, pUndo, true, true );
        if (bRet)
        {
            GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
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
    if( dynamic_cast<const SwDDETable*>( &rTable) !=  nullptr)
        return false;

    std::vector<sal_uLong> aNdsCnts;
    SwTableSortBoxes aTmpLst;
    SwUndoTableNdsChg* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoTableNdsChg( SwUndoId::TABLE_SPLIT, rBoxes, *pTableNd, 0, 0,
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
        aMsgHint.m_eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        if (bVert)
            bRet = rTable.SplitCol( this, rBoxes, nCnt );
        else
            bRet = rTable.SplitRow( this, rBoxes, nCnt, bSameHeight );

        if (bRet)
        {
            GetDocShell()->GetFEShell()->UpdateTableStyleFormatting();

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
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

TableMergeErr SwDoc::MergeTable( SwPaM& rPam )
{
    // Check if the current cursor's Point/Mark are inside a Table
    SwTableNode* pTableNd = rPam.GetNode().FindTableNode();
    if( !pTableNd )
        return TableMergeErr::NoSelection;
    SwTable& rTable = pTableNd->GetTable();
    if( dynamic_cast<const SwDDETable*>( &rTable) !=  nullptr )
        return TableMergeErr::NoSelection;
    TableMergeErr nRet = TableMergeErr::NoSelection;
    if( !rTable.IsNewModel() )
    {
        nRet =::CheckMergeSel( rPam );
        if( TableMergeErr::Ok != nRet )
            return nRet;
        nRet = TableMergeErr::NoSelection;
    }

    // #i33394#
    GetIDocumentUndoRedo().StartUndo( SwUndoId::TABLE_MERGE, nullptr );

    RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();
    getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);

    SwUndoTableMerge *const pUndo( (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTableMerge( rPam )
        :   nullptr );

    // Find the Boxes via the Layout
    SwSelBoxes aBoxes;
    SwSelBoxes aMerged;
    SwTableBox* pMergeBox;

    if( !rTable.PrepareMerge( rPam, aBoxes, aMerged, &pMergeBox, pUndo ) )
    {   // No cells found to merge
        getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
        if( pUndo )
        {
            delete pUndo;
            SwUndoId nLastUndoId(SwUndoId::EMPTY);
            if (GetIDocumentUndoRedo().GetLastUndoInfo(nullptr, & nLastUndoId)
                && (SwUndoId::REDLINE == nLastUndoId))
            {
                // FIXME: why is this horrible cleanup necessary?
                SwUndoRedline *const pU = dynamic_cast<SwUndoRedline*>(
                        GetUndoManager().RemoveLastUndo());
                if (pU && pU->GetRedlSaveCount())
                {
                    SwEditShell *const pEditShell(GetEditShell());
                    assert(pEditShell);
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
            rPam.GetPoint()->nContent.Assign( nullptr, 0 );
            rPam.SetMark();
            rPam.DeleteMark();

            SwPaM* pTmp = &rPam;
            while( &rPam != ( pTmp = pTmp->GetNext() ))
                for( int i = 0; i < 2; ++i )
                    pTmp->GetBound( static_cast<bool>(i) ) = *rPam.GetPoint();
        }

        // Merge them
        SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
        aMsgHint.m_eFlags = TBL_BOXPTR;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

        if( pTableNd->GetTable().Merge( this, aBoxes, aMerged, pMergeBox, pUndo ))
        {
            nRet = TableMergeErr::Ok;

            getIDocumentState().SetModified();
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
            if( pUndo )
            {
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
        }
        else
            delete pUndo;

        rPam.GetPoint()->nNode = *pMergeBox->GetSttNd();
        rPam.Move();

        ::ClearFEShellTabCols(*this, nullptr);
        getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }
    GetIDocumentUndoRedo().EndUndo( SwUndoId::TABLE_MERGE, nullptr );
    return nRet;
}

SwTableNode::SwTableNode( const SwNodeIndex& rIdx )
    : SwStartNode( rIdx, SwNodeType::Table )
{
    m_pTable.reset(new SwTable);
}

SwTableNode::~SwTableNode()
{
    // Notify UNO wrappers
    SwFrameFormat* pTableFormat = GetTable().GetFrameFormat();
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
                                pTableFormat );
    pTableFormat->ModifyNotification( &aMsgHint, &aMsgHint );
    DelFrames();
    m_pTable->SetTableNode(this); // set this so that ~SwDDETable can read it!
    m_pTable.reset();
}

SwTabFrame *SwTableNode::MakeFrame( SwFrame* pSib )
{
    return new SwTabFrame( *m_pTable, pSib );
}

/**
 * Creates all Views from the Document for the preceding Node. The resulting ContentFrames
 * are added to the corresponding Layout.
 */
void SwTableNode::MakeFrames(const SwNodeIndex & rIdx )
{
    if( !GetTable().GetFrameFormat()->HasWriterListeners()) // Do we actually have Frame?
        return;

    SwFrame *pFrame;
    SwContentNode * pNode = rIdx.GetNode().GetContentNode();

    OSL_ENSURE( pNode, "No ContentNode or CopyNode and new Node is identical");

    bool bBefore = rIdx < GetIndex();

    SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );

    while( nullptr != (pFrame = aNode2Layout.NextFrame()) )
    {
        SwFrame *pNew = pNode->MakeFrame( pFrame );
        // Will the Node receive Frames before or after?
        if ( bBefore )
            // The new one precedes me
            pNew->Paste( pFrame->GetUpper(), pFrame );
        else
            // The new one succeeds me
            pNew->Paste( pFrame->GetUpper(), pFrame->GetNext() );
    }
}

/**
 * Create a TableFrame for every Shell and insert before the corresponding ContentFrame.
 */
void SwTableNode::MakeFrames( SwNodeIndex* pIdxBehind )
{
    OSL_ENSURE( pIdxBehind, "No Index" );
    *pIdxBehind = *this;
    SwNode *pNd = GetNodes().FindPrvNxtFrameNode( *pIdxBehind, EndOfSectionNode() );
    if( !pNd )
        return ;

    SwFrame *pFrame( nullptr );
    SwLayoutFrame *pUpper( nullptr );
    SwNode2Layout aNode2Layout( *pNd, GetIndex() );
    while( nullptr != (pUpper = aNode2Layout.UpperFrame( pFrame, *this )) )
    {
        SwTabFrame* pNew = MakeFrame( pUpper );
        pNew->Paste( pUpper, pFrame );
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
        {
            SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pNew->FindPrevCnt()) );
            }
        }
        pNew->RegistFlys();
    }
}

void SwTableNode::DelFrames()
{
    /* For a start, cut out and delete the TabFrames (which will also delete the Columns and Rows)
       The TabFrames are attached to the FrameFormat of the SwTable.
       We need to delete them in a more cumbersome way, for the Master to also delete the Follows. */

    SwIterator<SwTabFrame,SwFormat> aIter( *(m_pTable->GetFrameFormat()) );
    SwTabFrame *pFrame = aIter.First();
    while ( pFrame )
    {
        bool bAgain = false;
        {
            if ( !pFrame->IsFollow() )
            {
                while ( pFrame->HasFollow() )
                    pFrame->JoinAndDelFollows();
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for current next paragraph will change
                // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
                {
                    SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pFrame->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pFrame->FindPrevCnt()) );
                    }
                }
                pFrame->Cut();
                SwFrame::DestroyFrame(pFrame);
                bAgain = true;
            }
        }
        pFrame = bAgain ? aIter.First() : aIter.Next();
    }
}

void SwTableNode::SetNewTable( std::unique_ptr<SwTable> pNewTable, bool bNewFrames )
{
    DelFrames();
    m_pTable->SetTableNode(this);
    m_pTable = std::move(pNewTable);
    if( bNewFrames )
    {
        SwNodeIndex aIdx( *EndOfSectionNode());
        GetNodes().GoNext( &aIdx );
        MakeFrames( &aIdx );
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

void SwDoc::GetTabCols( SwTabCols &rFill, const SwCellFrame* pBoxFrame )
{
    OSL_ENSURE( pBoxFrame, "pBoxFrame needs to be specified!" );
    if( !pBoxFrame )
        return;

    SwTabFrame *pTab = const_cast<SwFrame*>(static_cast<SwFrame const *>(pBoxFrame))->ImplFindTabFrame();
    const SwTableBox* pBox = pBoxFrame->GetTabBox();

    // Set fixed points, LeftMin in Document coordinates, all others relative
    SwRectFnSet aRectFnSet(pTab);
    const SwPageFrame* pPage = pTab->FindPageFrame();
    const sal_uLong nLeftMin = aRectFnSet.GetLeft(pTab->getFrameArea()) -
                           aRectFnSet.GetLeft(pPage->getFrameArea());
    const sal_uLong nRightMax = aRectFnSet.GetRight(pTab->getFrameArea()) -
                            aRectFnSet.GetLeft(pPage->getFrameArea());

    rFill.SetLeftMin ( nLeftMin );
    rFill.SetLeft    ( aRectFnSet.GetLeft(pTab->getFramePrintArea()) );
    rFill.SetRight   ( aRectFnSet.GetRight(pTab->getFramePrintArea()));
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

static bool lcl_IsFrameInColumn( const SwCellFrame& rFrame, SwSelBoxes const & rBoxes )
{
    for (size_t i = 0; i < rBoxes.size(); ++i)
    {
        if ( rFrame.GetTabBox() == rBoxes[ i ] )
            return true;
    }

    return false;
}

void SwDoc::GetTabRows( SwTabCols &rFill, const SwCellFrame* pBoxFrame )
{
    OSL_ENSURE( pBoxFrame, "GetTabRows called without pBoxFrame" );

    // Make code robust:
    if ( !pBoxFrame )
        return;

    // #i39552# Collection of the boxes of the current
    // column has to be done at the beginning of this function, because
    // the table may be formatted in ::GetTableSel.
    SwDeletionChecker aDelCheck( pBoxFrame );

    SwSelBoxes aBoxes;
    const SwContentFrame* pContent = ::GetCellContent( *pBoxFrame );
    if ( pContent && pContent->IsTextFrame() )
    {
        const SwPosition aPos(*static_cast<const SwTextFrame*>(pContent)->GetTextNodeFirst());
        const SwCursor aTmpCursor( aPos, nullptr );
        ::GetTableSel( aTmpCursor, aBoxes, SwTableSearchType::Col );
    }

    // Make code robust:
    if ( aDelCheck.HasBeenDeleted() )
    {
        OSL_FAIL( "Current box has been deleted during GetTabRows()" );
        return;
    }

    // Make code robust:
    const SwTabFrame* pTab = pBoxFrame->FindTabFrame();
    OSL_ENSURE( pTab, "GetTabRows called without a table" );
    if ( !pTab )
        return;

    const SwFrame* pFrame = pTab->GetNextLayoutLeaf();

    // Set fixed points, LeftMin in Document coordinates, all others relative
    SwRectFnSet aRectFnSet(pTab);
    const SwPageFrame* pPage = pTab->FindPageFrame();
    const long nLeftMin  = ( aRectFnSet.IsVert() ?
                             pTab->GetPrtLeft() - pPage->getFrameArea().Left() :
                             pTab->GetPrtTop() - pPage->getFrameArea().Top() );
    const long nLeft     = aRectFnSet.IsVert() ? LONG_MAX : 0;
    const long nRight    = aRectFnSet.GetHeight(pTab->getFramePrintArea());
    const long nRightMax = aRectFnSet.IsVert() ? nRight : LONG_MAX;

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

    while ( pFrame && pTab->IsAnLower( pFrame ) )
    {
        if ( pFrame->IsCellFrame() && pFrame->FindTabFrame() == pTab )
        {
            // upper and lower borders of current cell frame:
            long nUpperBorder = aRectFnSet.GetTop(pFrame->getFrameArea());
            long nLowerBorder = aRectFnSet.GetBottom(pFrame->getFrameArea());

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
                    aHidden[ nTmpVal ] = !lcl_IsFrameInColumn( *static_cast<const SwCellFrame*>(pFrame), aBoxes );
                else
                {
                    if ( aHidden[ nTmpVal ] &&
                         lcl_IsFrameInColumn( *static_cast<const SwCellFrame*>(pFrame), aBoxes ) )
                        aHidden[ nTmpVal ] = false;
                }
                nTmpVal = nLowerBorder;
            }
        }

        pFrame = pFrame->GetNextLayoutLeaf();
    }

    // transfer calculated values from BoundaryMap and HiddenMap into rFill:
    size_t nIdx = 0;
    for ( aIter = aBoundaries.begin(); aIter != aBoundaries.end(); ++aIter )
    {
        const long nTabTop = aRectFnSet.GetPrtTop(*pTab);
        const long nKey = aRectFnSet.YDiff( (*aIter).first, nTabTop );
        const std::pair< long, long > aTmpPair = (*aIter).second;
        const long nFirst = aRectFnSet.YDiff( aTmpPair.first, nTabTop );
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
                        const SwCellFrame* pBoxFrame )
{
    const SwTableBox* pBox = nullptr;
    SwTabFrame *pTab = nullptr;

    if( pBoxFrame )
    {
        pTab = const_cast<SwFrame*>(static_cast<SwFrame const *>(pBoxFrame))->ImplFindTabFrame();
        pBox = pBoxFrame->GetTabBox();
    }
    else
    {
        OSL_ENSURE( false, "must specify pBoxFrame" );
        return ;
    }

    // If the Table is still using relative values (USHRT_MAX)
    // we need to switch to absolute ones.
    SwTable& rTab = *pTab->GetTable();
    const SwFormatFrameSize& rTableFrameSz = rTab.GetFrameFormat()->GetFrameSize();
    SwRectFnSet aRectFnSet(pTab);
    // #i17174# - With fix for #i9040# the shadow size is taken
    // from the table width. Thus, add its left and right size to current table
    // printing area width in order to get the correct table size attribute.
    SwTwips nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
    {
        SvxShadowItem aShadow( rTab.GetFrameFormat()->GetShadow() );
        nPrtWidth += aShadow.CalcShadowSpace( SvxShadowItemSide::LEFT ) +
                     aShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT );
    }
    if( nPrtWidth != rTableFrameSz.GetWidth() )
    {
        SwFormatFrameSize aSz( rTableFrameSz );
        aSz.SetWidth( nPrtWidth );
        rTab.GetFrameFormat()->SetFormatAttr( aSz );
    }

    SwTabCols aOld( rNew.Count() );

    const SwPageFrame* pPage = pTab->FindPageFrame();
    const sal_uLong nLeftMin = aRectFnSet.GetLeft(pTab->getFrameArea()) -
                           aRectFnSet.GetLeft(pPage->getFrameArea());
    const sal_uLong nRightMax = aRectFnSet.GetRight(pTab->getFrameArea()) -
                            aRectFnSet.GetLeft(pPage->getFrameArea());

    // Set fixed points, LeftMin in Document coordinates, all others relative
    aOld.SetLeftMin ( nLeftMin );
    aOld.SetLeft    ( aRectFnSet.GetLeft(pTab->getFramePrintArea()) );
    aOld.SetRight   ( aRectFnSet.GetRight(pTab->getFramePrintArea()));
    aOld.SetRightMax( nRightMax - nLeftMin );

    rTab.GetTabCols( aOld, pBox );
    SetTabCols(rTab, rNew, aOld, pBox, bCurRowOnly );
}

void SwDoc::SetTabRows( const SwTabCols &rNew, bool bCurColOnly,
                        const SwCellFrame* pBoxFrame )
{
    SwTabFrame *pTab = nullptr;

    if( pBoxFrame )
    {
        pTab = const_cast<SwFrame*>(static_cast<SwFrame const *>(pBoxFrame))->ImplFindTabFrame();
    }
    else
    {
        OSL_ENSURE( false, "must specify pBoxFrame" );
        return ;
    }

    // If the Table is still using relative values (USHRT_MAX)
    // we need to switch to absolute ones.
    SwRectFnSet aRectFnSet(pTab);
    SwTabCols aOld( rNew.Count() );

    // Set fixed points, LeftMin in Document coordinates, all others relative
    const SwPageFrame* pPage = pTab->FindPageFrame();

    aOld.SetRight( aRectFnSet.GetHeight(pTab->getFramePrintArea()) );
    long nLeftMin;
    if ( aRectFnSet.IsVert() )
    {
        nLeftMin = pTab->GetPrtLeft() - pPage->getFrameArea().Left();
        aOld.SetLeft    ( LONG_MAX );
        aOld.SetRightMax( aOld.GetRight() );

    }
    else
    {
        nLeftMin = pTab->GetPrtTop() - pPage->getFrameArea().Top();
        aOld.SetLeft    ( 0 );
        aOld.SetRightMax( LONG_MAX );
    }
    aOld.SetLeftMin ( nLeftMin );

    GetTabRows( aOld, pBoxFrame );

    GetIDocumentUndoRedo().StartUndo( SwUndoId::TABLE_ATTR, nullptr );

    // check for differences between aOld and rNew:
    const size_t nCount = rNew.Count();
    const SwTable* pTable = pTab->GetTable();
    OSL_ENSURE( pTable, "My colleague told me, this couldn't happen" );

    for ( size_t i = 0; i <= nCount; ++i )
    {
        const size_t nIdxStt = aRectFnSet.IsVert() ? nCount - i : i - 1;
        const size_t nIdxEnd = aRectFnSet.IsVert() ? nCount - i - 1 : i;

        const long nOldRowStart = i == 0  ? 0 : aOld[ nIdxStt ];
        const long nOldRowEnd =   i == nCount ? aOld.GetRight() : aOld[ nIdxEnd ];
        const long nOldRowHeight = nOldRowEnd - nOldRowStart;

        const long nNewRowStart = i == 0  ? 0 : rNew[ nIdxStt ];
        const long nNewRowEnd =   i == nCount ? rNew.GetRight() : rNew[ nIdxEnd ];
        const long nNewRowHeight = nNewRowEnd - nNewRowStart;

        const long nDiff = nNewRowHeight - nOldRowHeight;
        if ( std::abs( nDiff ) >= ROWFUZZY )
        {
            // For the old table model pTextFrame and pLine will be set for every box.
            // For the new table model pTextFrame will be set if the box is not covered,
            // but the pLine will be set if the box is not an overlapping box
            // In the new table model the row height can be adjusted,
            // when both variables are set.
            const SwTextFrame* pTextFrame = nullptr;
            const SwTableLine* pLine = nullptr;

            // Iterate over all SwCellFrames with Bottom = nOldPos
            const SwFrame* pFrame = pTab->GetNextLayoutLeaf();
            while ( pFrame && pTab->IsAnLower( pFrame ) )
            {
                if ( pFrame->IsCellFrame() && pFrame->FindTabFrame() == pTab )
                {
                    const long nLowerBorder = aRectFnSet.GetBottom(pFrame->getFrameArea());
                    const sal_uLong nTabTop = aRectFnSet.GetPrtTop(*pTab);
                    if ( std::abs( aRectFnSet.YInc( nTabTop, nOldRowEnd ) - nLowerBorder ) <= ROWFUZZY )
                    {
                        if ( !bCurColOnly || pFrame == pBoxFrame )
                        {
                            const SwFrame* pContent = ::GetCellContent( static_cast<const SwCellFrame&>(*pFrame) );

                            if ( pContent && pContent->IsTextFrame() )
                            {
                                const SwTableBox* pBox = static_cast<const SwCellFrame*>(pFrame)->GetTabBox();
                                const long nRowSpan = pBox->getRowSpan();
                                if( nRowSpan > 0 ) // Not overlapped
                                    pTextFrame = static_cast<const SwTextFrame*>(pContent);
                                if( nRowSpan < 2 ) // Not overlapping for row height
                                    pLine = pBox->GetUpper();
                                if( pLine && pTextFrame ) // always for old table model
                                {
                                    // The new row height must not to be calculated from a overlapping box
                                    SwFormatFrameSize aNew( pLine->GetFrameFormat()->GetFrameSize() );
                                    const long nNewSize = aRectFnSet.GetHeight(pFrame->getFrameArea()) + nDiff;
                                    if( nNewSize != aNew.GetHeight() )
                                    {
                                        aNew.SetHeight( nNewSize );
                                        if ( ATT_VAR_SIZE == aNew.GetHeightSizeType() )
                                            aNew.SetHeightSizeType( ATT_MIN_SIZE );
                                        // This position must not be in an overlapped box
                                        const SwPosition aPos(*static_cast<const SwTextFrame*>(pContent)->GetTextNodeFirst());
                                        const SwCursor aTmpCursor( aPos, nullptr );
                                        SetRowHeight( aTmpCursor, aNew );
                                        // For the new table model we're done, for the old one
                                        // there might be another (sub)row to adjust...
                                        if( pTable->IsNewModel() )
                                            break;
                                    }
                                    pLine = nullptr;
                                }
                            }
                        }
                    }
                }
                pFrame = pFrame->GetNextLayoutLeaf();
            }
        }
    }

    GetIDocumentUndoRedo().EndUndo( SwUndoId::TABLE_ATTR, nullptr );

    ::ClearFEShellTabCols(*this, nullptr);
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
      ::ClearFEShellTabCols(*this, nullptr);
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
        pHst->Add( rNd.GetFormatColl(), rNd.GetIndex(), SwNodeType::Text );
}

void SwCollectTableLineBoxes::AddBox( const SwTableBox& rBox )
{
    aPosArr.push_back(nWidth);
    SwTableBox* p = const_cast<SwTableBox*>(&rBox);
    m_Boxes.push_back(p);
    nWidth = nWidth + static_cast<sal_uInt16>(rBox.GetFrameFormat()->GetFrameSize().GetWidth());
}

const SwTableBox* SwCollectTableLineBoxes::GetBoxOfPos( const SwTableBox& rBox )
{
    const SwTableBox* pRet = nullptr;

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

        nWidth = nWidth + static_cast<sal_uInt16>(rBox.GetFrameFormat()->GetFrameSize().GetWidth());
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

        if( SplitTable_HeadlineOption::BorderCopy == pSplPara->GetMode() )
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
            sal_uInt16 const aTableSplitBoxSetRange[] {
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

            if( SplitTable_HeadlineOption::BoxAttrAllCopy == pSplPara->GetMode() )
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
bool SwDoc::SplitTable( const SwPosition& rPos, SplitTable_HeadlineOption eHdlnMode,
                        bool bCalcNewSize )
{
    SwNode* pNd = &rPos.nNode.GetNode();
    SwTableNode* pTNd = pNd->FindTableNode();
    if( !pTNd || pNd->IsTableNode() )
        return false;

    if( dynamic_cast<const SwDDETable*>( &pTNd->GetTable() ) !=  nullptr)
        return false;

    SwTable& rTable = pTNd->GetTable();
    rTable.SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>()); // Delete HTML Layout

    SwTableFormulaUpdate aMsgHint( &rTable );

    SwHistory aHistory;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        aMsgHint.m_pHistory = &aHistory;
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
            aMsgHint.m_nSplitLine = rTable.GetTabLines().GetPos( pLine );
        }

        OUString sNewTableNm( GetUniqueTableName() );
        aMsgHint.m_aData.pNewTableNm = &sNewTableNm;
        aMsgHint.m_eFlags = TBL_SPLITTBL;
        getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    // Find Lines for the Layout update
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( rTable );
    aFndBox.DelFrames( rTable );

    SwTableNode* pNew = GetNodes().SplitTable( rPos.nNode, false, bCalcNewSize );

    if( pNew )
    {
        std::unique_ptr<SwSaveRowSpan> pSaveRowSp = pNew->GetTable().CleanUpTopRowSpan( rTable.GetTabLines().size() );
        SwUndoSplitTable* pUndo = nullptr;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo = new SwUndoSplitTable(
                        *pNew, std::move(pSaveRowSp), eHdlnMode, bCalcNewSize);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
            if( aHistory.Count() )
                pUndo->SaveFormula( aHistory );
        }

        switch( eHdlnMode )
        {
        // Set the lower Border of the preceding Line to
        // the upper Border of the current one
        case SplitTable_HeadlineOption::BorderCopy:
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
        case SplitTable_HeadlineOption::BoxAttrCopy:
        case SplitTable_HeadlineOption::BoxAttrAllCopy:
            {
                SwHistory* pHst = nullptr;
                if( SplitTable_HeadlineOption::BoxAttrAllCopy == eHdlnMode && pUndo )
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

        case SplitTable_HeadlineOption::ContentCopy:
            rTable.CopyHeadlineIntoTable( *pNew );
            if( pUndo )
                pUndo->SetTableNodeOffset( pNew->GetIndex() );
            break;

        case SplitTable_HeadlineOption::NONE:
            // Switch off repeating the Header
            pNew->GetTable().SetRowsToRepeat( 0 );
            break;
        }

        // And insert Frames
        SwNodeIndex aNdIdx( *pNew->EndOfSectionNode() );
        GetNodes().GoNext( &aNdIdx ); // To the next ContentNode
        pNew->MakeFrames( &aNdIdx );

        // Insert a paragraph between the Table
        GetNodes().MakeTextNode( SwNodeIndex( *pNew ),
                                getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ) );
    }

    // Update Layout
    aFndBox.MakeFrames( rTable );

    // TL_CHART2: need to inform chart of probably changed cell names
    UpdateCharts( rTable.GetFrameFormat()->GetName() );

    // update table style formatting of both the tables
    GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(pTNd);
    GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(pNew);

    getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );

    return nullptr != pNew;
}

static bool lcl_ChgTableSize( SwTable& rTable )
{
    // The Attribute must not be set via the Modify or else all Boxes are
    // set back to 0.
    // So lock the Format.
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SwFormatFrameSize aTableMaxSz( pFormat->GetFrameSize() );

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
            nMaxLnWidth += pBox->GetFrameFormat()->GetFrameSize().GetWidth();

        if( nMaxLnWidth > aTableMaxSz.GetWidth() )
            aTableMaxSz.SetWidth( nMaxLnWidth );
    }
    pFormat->SetFormatAttr( aTableMaxSz );
    if( !bLocked ) // Release the Lock if appropriate
        pFormat->UnlockModify();

    return true;
}

class SplitTable_Para
{
    std::map<SwFrameFormat const *, SwFrameFormat*> aSrcDestMap;
    SwTableNode* pNewTableNd;
    SwTable& rOldTable;

public:
    SplitTable_Para( SwTableNode* pNew, SwTable& rOld )
        : aSrcDestMap(), pNewTableNd( pNew ), rOldTable( rOld )
    {}
    SwFrameFormat* GetDestFormat( SwFrameFormat* pSrcFormat ) const
    {
        auto it = aSrcDestMap.find( pSrcFormat );
        return it == aSrcDestMap.end() ? nullptr : it->second;
    }

    void InsertSrcDest( SwFrameFormat const * pSrcFormat, SwFrameFormat* pDestFormat )
            { aSrcDestMap[ pSrcFormat ] = pDestFormat; }

    void ChgBox( SwTableBox* pBox )
    {
        rOldTable.GetTabSortBoxes().erase( pBox );
        pNewTableNd->GetTable().GetTabSortBoxes().insert( pBox );
    }
};

static void lcl_SplitTable_CpyBox( SwTableBox* pBox, SplitTable_Para* pPara );

static void lcl_SplitTable_CpyLine( SwTableLine* pLn, SplitTable_Para* pPara )
{
    SwFrameFormat *pSrcFormat = pLn->GetFrameFormat();
    SwTableLineFormat* pDestFormat = static_cast<SwTableLineFormat*>( pPara->GetDestFormat( pSrcFormat ) );
    if( pDestFormat == nullptr )
    {
        pPara->InsertSrcDest( pSrcFormat, pLn->ClaimFrameFormat() );
    }
    else
        pLn->ChgFrameFormat( pDestFormat );

    for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
             it != pLn->GetTabBoxes().end(); ++it)
        lcl_SplitTable_CpyBox(*it, pPara );
}

static void lcl_SplitTable_CpyBox( SwTableBox* pBox, SplitTable_Para* pPara )
{
    SwFrameFormat *pSrcFormat = pBox->GetFrameFormat();
    SwTableBoxFormat* pDestFormat = static_cast<SwTableBoxFormat*>(pPara->GetDestFormat( pSrcFormat ));
    if( pDestFormat == nullptr )
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
        return nullptr;

    sal_uLong nSttIdx = pNd->FindTableBoxStartNode()->GetIndex();

    // Find this Box/top-level line
    SwTable& rTable = pTNd->GetTable();
    SwTableBox* pBox = rTable.GetTableBox( nSttIdx );
    if( !pBox )
        return nullptr;

    SwTableLine* pLine = pBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    // pLine now contains the top-level line
    sal_uInt16 nLinePos = rTable.GetTabLines().GetPos( pLine );
    if( USHRT_MAX == nLinePos ||
        ( bAfter ? ++nLinePos >= rTable.GetTabLines().size() : !nLinePos ))
        return nullptr; // Not found or last Line!

    // Find the first Box of the succeeding Line
    SwTableLine* pNextLine = rTable.GetTabLines()[ nLinePos ];
    pBox = pNextLine->GetTabBoxes()[0];
    while( !pBox->GetSttNd() )
        pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];

    // Insert an EndNode and TableNode into the Nodes Array
    SwTableNode * pNewTableNd;
    {
        SwEndNode* pOldTableEndNd = pTNd->EndOfSectionNode()->GetEndNode();
        assert(pOldTableEndNd && "Where is the EndNode?");

        SwNodeIndex aIdx( *pBox->GetSttNd() );
        new SwEndNode( aIdx, *pTNd );
        pNewTableNd = new SwTableNode( aIdx );
        pNewTableNd->GetTable().SetTableModel( rTable.IsNewModel() );

        pOldTableEndNd->m_pStartOfSection = pNewTableNd;
        pNewTableNd->m_pEndOfSection = pOldTableEndNd;

        SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
        do {
            OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
            pBoxNd->m_pStartOfSection = pNewTableNd;
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
        SplitTable_Para aPara( pNewTableNd, rTable );
        for( SwTableLine* pNewLine : rNewTable.GetTabLines() )
            lcl_SplitTable_CpyLine( pNewLine, &aPara );
        rTable.CleanUpBottomRowSpan( nDeleted );
    }

    {
        // Copy the Table FrameFormat
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

    if( dynamic_cast<const SwDDETable*>( &pTableNd->GetTable() ) !=  nullptr ||
        dynamic_cast<const SwDDETable*>( &pDelTableNd->GetTable() ) !=  nullptr)
        return false;

    // Delete HTML Layout
    pTableNd->GetTable().SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());
    pDelTableNd->GetTable().SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());

    // Both Tables are present; we can start
    SwUndoMergeTable* pUndo = nullptr;
    SwHistory* pHistory = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMergeTable( *pTableNd, *pDelTableNd, bWithPrev, nMode );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        pHistory = new SwHistory;
    }

    // Adapt all "TableFormulas"
    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_aData.pDelTable = &pDelTableNd->GetTable();
    aMsgHint.m_eFlags = TBL_MERGETBL;
    aMsgHint.m_pHistory = pHistory;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    // The actual merge
    SwNodeIndex aIdx( bWithPrev ? *pTableNd : *pDelTableNd );
    bool bRet = rNds.MergeTable( aIdx, !bWithPrev, nMode );

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
        getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
    }
    return bRet;
}

bool SwNodes::MergeTable( const SwNodeIndex& rPos, bool bWithPrev,
                            sal_uInt16 nMode )
{
    SwTableNode* pDelTableNd = rPos.GetNode().GetTableNode();
    OSL_ENSURE( pDelTableNd, "Where did the TableNode go?" );

    SwTableNode* pTableNd = (*this)[ rPos.GetIndex() - 1]->FindTableNode();
    OSL_ENSURE( pTableNd, "Where did the TableNode go?" );

    if( !pDelTableNd || !pTableNd )
        return false;

    pDelTableNd->DelFrames();

    SwTable& rDelTable = pDelTableNd->GetTable();
    SwTable& rTable = pTableNd->GetTable();

    // Find Lines for the Layout update
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( rTable );
    aFndBox.DelFrames( rTable );

    // TL_CHART2:
    // tell the charts about the table to be deleted and have them use their own data
    GetDoc()->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( &rDelTable );

    // Sync the TableFormat's Width
    {
        const SwFormatFrameSize& rTableSz = rTable.GetFrameFormat()->GetFrameSize();
        const SwFormatFrameSize& rDelTableSz = rDelTable.GetFrameFormat()->GetFrameSize();
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
    pTableNd->m_pEndOfSection = pTableEndNd;

    SwNodeIndex aIdx( *pDelTableNd, 1 );

    SwNode* pBoxNd = aIdx.GetNode().GetStartNode();
    do {
        OSL_ENSURE( pBoxNd->IsStartNode(), "This needs to be a StartNode!" );
        pBoxNd->m_pStartOfSection = pTableNd;
        pBoxNd = (*this)[ pBoxNd->EndOfSectionIndex() + 1 ];
    } while( pBoxNd != pTableEndNd );
    pBoxNd->m_pStartOfSection = pTableNd;

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
        SwGCLineBorder aPara( rTable );
        aPara.nLinePos = --nOldSize;
        pFirstLn = rTable.GetTabLines()[ nOldSize ];
        sw_GC_Line_Border( pFirstLn, &aPara );
    }

    // Update Layout
    aFndBox.MakeFrames( rTable );

    return true;
}

// Use the PtrArray's ForEach method
struct SetAFormatTabPara
{
    SwTableAutoFormat& rTableFormat;
    SwUndoTableAutoFormat* pUndo;
    sal_uInt16 nEndBox, nCurBox;
    sal_uInt8 nAFormatLine, nAFormatBox;

    explicit SetAFormatTabPara( const SwTableAutoFormat& rNew )
        : rTableFormat( const_cast<SwTableAutoFormat&>(rNew) ), pUndo( nullptr ),
        nEndBox( 0 ), nCurBox( 0 ), nAFormatLine( 0 ), nAFormatBox( 0 )
    {}
};

// Forward declare so that the Lines and Boxes can use recursion
static bool lcl_SetAFormatBox(FndBox_ &, SetAFormatTabPara *pSetPara, bool bResetDirect);
static bool lcl_SetAFormatLine(FndLine_ &, SetAFormatTabPara *pPara, bool bResetDirect);

static bool lcl_SetAFormatLine(FndLine_ & rLine, SetAFormatTabPara *pPara, bool bResetDirect)
{
    for (auto const& it : rLine.GetBoxes())
    {
        lcl_SetAFormatBox(*it, pPara, bResetDirect);
    }
    return true;
}

static bool lcl_SetAFormatBox(FndBox_ & rBox, SetAFormatTabPara *pSetPara, bool bResetDirect)
{
    if (!rBox.GetUpper()->GetUpper()) // Box on first level?
    {
        if( !pSetPara->nCurBox )
            pSetPara->nAFormatBox = 0;
        else if( pSetPara->nCurBox == pSetPara->nEndBox )
            pSetPara->nAFormatBox = 3;
        else
            pSetPara->nAFormatBox = static_cast<sal_uInt8>(1 + ((pSetPara->nCurBox-1) & 1));
    }

    if (rBox.GetBox()->GetSttNd())
    {
        SwTableBox* pSetBox = rBox.GetBox();
        if (!pSetBox->HasDirectFormatting() || bResetDirect)
        {
            if (bResetDirect)
                pSetBox->SetDirectFormatting(false);

            SwDoc* pDoc = pSetBox->GetFrameFormat()->GetDoc();
            SfxItemSet aCharSet(pDoc->GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_PARATR_LIST_END-1>{});
            SfxItemSet aBoxSet(pDoc->GetAttrPool(), aTableBoxSetRange);
            sal_uInt8 nPos = pSetPara->nAFormatLine * 4 + pSetPara->nAFormatBox;
            pSetPara->rTableFormat.UpdateToSet(nPos, aCharSet, SwTableAutoFormat::UPDATE_CHAR, nullptr);
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

bool SwDoc::SetTableAutoFormat(const SwSelBoxes& rBoxes, const SwTableAutoFormat& rNew, bool bResetDirect, bool const isSetStyleName)
{
    OSL_ENSURE( !rBoxes.empty(), "No valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // Find all Boxes/Lines
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    SwTable &table = pTableNd->GetTable();
    table.SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());

    FndBox_* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front()->GetBoxes().size())
    {
        pFndBox = pFndBox->GetLines().front()->GetBoxes()[0].get();
    }

    if( pFndBox->GetLines().empty() ) // One too far? (only one sel. Box)
        pFndBox = pFndBox->GetUpper()->GetUpper();

    // Disable Undo, but first store parameters
    SwUndoTableAutoFormat* pUndo = nullptr;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndo = new SwUndoTableAutoFormat( *pTableNd, rNew );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
        GetIDocumentUndoRedo().DoUndo(false);
    }

    if (isSetStyleName)
    {   // tdf#98226 do this here where undo can record it
        pTableNd->GetTable().SetTableStyleName(rNew.GetName());
    }

    rNew.RestoreTableProperties(table);

    SetAFormatTabPara aPara( rNew );
    FndLines_t& rFLns = pFndBox->GetLines();

    for (FndLines_t::size_type n = 0; n < rFLns.size(); ++n)
    {
        FndLine_* pLine = rFLns[n].get();

        // Set Upper to 0 (thus simulate BaseLine)
        FndBox_* pSaveBox = pLine->GetUpper();
        pLine->SetUpper( nullptr );

        if( !n )
            aPara.nAFormatLine = 0;
        else if (static_cast<size_t>(n+1) == rFLns.size())
            aPara.nAFormatLine = 3;
        else
            aPara.nAFormatLine = static_cast<sal_uInt8>(1 + ((n-1) & 1 ));

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
    getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );

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
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    // Store table properties
    SwTable &table = pTableNd->GetTable();
    rGet.StoreTableProperties(table);

    FndBox_* pFndBox = &aFndBox;
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
        FndLine_& rLine = *rFLns[ aLnArr[ nLine ] ];

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
                                    SwTableAutoFormat::UPDATE_CHAR, nullptr );
            rGet.UpdateFromSet( nPos, pFBox->GetFrameFormat()->GetAttrSet(),
                                SwTableAutoFormat::UPDATE_BOX,
                                GetNumberFormatter() );
        }
    }

    return true;
}

SwTableAutoFormatTable& SwDoc::GetTableStyles()
{
    if (!m_pTableStyles)
    {
        m_pTableStyles.reset(new SwTableAutoFormatTable);
        m_pTableStyles->Load();
    }
    return *m_pTableStyles.get();
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

    const OUString aName(SwResId(STR_TABLE_DEFNAME));

    const size_t nFlagSize = ( mpTableFrameFormatTable->size() / 8 ) + 2;

    std::unique_ptr<sal_uInt8[]> pSetFlags( new sal_uInt8[ nFlagSize ] );
    memset( pSetFlags.get(), 0, nFlagSize );

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

    return aName + OUString::number( ++nNum );
}

SwTableFormat* SwDoc::FindTableFormatByName( const OUString& rName, bool bAll ) const
{
    const SwFormat* pRet = nullptr;
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

bool SwDoc::SetColRowWidthHeight( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                                    SwTwips nAbsDiff, SwTwips nRelDiff )
{
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rCurrentBox.GetSttNd()->FindTableNode());
    SwUndo* pUndo = nullptr;

    if( (TableChgWidthHeightType::InsertDeleteMode & eType) && dynamic_cast<const SwDDETable*>( &pTableNd->GetTable()) !=  nullptr)
        return false;

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_eFlags = TBL_BOXPTR;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    bool bRet = false;
    switch( extractPosition(eType) )
    {
    case TableChgWidthHeightType::ColLeft:
    case TableChgWidthHeightType::ColRight:
    case TableChgWidthHeightType::CellLeft:
    case TableChgWidthHeightType::CellRight:
        {
             bRet = pTableNd->GetTable().SetColWidth( rCurrentBox,
                                eType, nAbsDiff, nRelDiff,
                                bUndo ? &pUndo : nullptr );
        }
        break;
    case TableChgWidthHeightType::RowBottom:
    case TableChgWidthHeightType::CellTop:
    case TableChgWidthHeightType::CellBottom:
        bRet = pTableNd->GetTable().SetRowHeight( rCurrentBox,
                            eType, nAbsDiff, nRelDiff,
                            bUndo ? &pUndo : nullptr );
        break;
    default: break;
    }

    GetIDocumentUndoRedo().DoUndo(bUndo); // SetColWidth can turn it off
    if( pUndo )
    {
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    if( bRet )
    {
        getIDocumentState().SetModified();
        if( TableChgWidthHeightType::InsertDeleteMode & eType )
            getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
    }
    return bRet;
}

bool SwDoc::IsNumberFormat( const OUString& rString, sal_uInt32& F_Index, double& fOutNumber )
{
    if( rString.getLength() > 308 ) // optimization matches svl:IsNumberFormat arbitrary value
        return false;

    // remove any comment anchor marks
    OUStringBuffer sStringBuffer(rString);
    sal_Int32 nCommentPosition = sStringBuffer.indexOf( CH_TXTATR_INWORD );
    while( nCommentPosition != -1 )
    {
        sStringBuffer.remove( nCommentPosition, 1 );
        nCommentPosition = sStringBuffer.indexOf( CH_TXTATR_INWORD, nCommentPosition );
    }

    return GetNumberFormatter()->IsNumberFormat( sStringBuffer.makeStringAndClear(), F_Index, fOutNumber );
}

void SwDoc::ChkBoxNumFormat( SwTableBox& rBox, bool bCallUpdate )
{
    // Optimization: If the Box says it's Text, it remains Text
    const SfxPoolItem* pNumFormatItem = nullptr;
    if( SfxItemState::SET == rBox.GetFrameFormat()->GetItemState( RES_BOXATR_FORMAT,
        false, &pNumFormatItem ) && GetNumberFormatter()->IsTextFormat(
            static_cast<const SwTableBoxNumFormat*>(pNumFormatItem)->GetValue() ))
        return ;

    SwUndoTableNumFormat* pUndo = nullptr;

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
                GetIDocumentUndoRedo().StartUndo( SwUndoId::TABLE_AUTOFMT, nullptr );
                pUndo = new SwUndoTableNumFormat( rBox );
                pUndo->SetNumFormat( nFormatIdx, fNumber );
            }

            SwTableBoxFormat* pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.GetFrameFormat());
            SfxItemSet aBoxSet( GetAttrPool(), svl::Items<RES_BOXATR_FORMAT, RES_BOXATR_VALUE>{} );

            bool bLockModify = true;
            bool bSetNumberFormat = IsInsTableFormatNum();
            const bool bForceNumberFormat = IsInsTableFormatNum() && IsInsTableChangeNumFormat();

            // if the user forced a number format in this cell previously,
            // keep it, unless the user set that she wants the full number
            // format recognition
            if( pNumFormatItem && !bForceNumberFormat )
            {
                sal_uLong nOldNumFormat = static_cast<const SwTableBoxNumFormat*>(pNumFormatItem)->GetValue();
                SvNumberFormatter* pNumFormatr = GetNumberFormatter();

                SvNumFormatType nFormatType = pNumFormatr->GetType( nFormatIdx );
                if( nFormatType == pNumFormatr->GetType( nOldNumFormat ) || SvNumFormatType::NUMBER == nFormatType )
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
        const SfxPoolItem* pValueItem = nullptr, *pFormatItem = nullptr;
        SwTableBoxFormat* pBoxFormat = static_cast<SwTableBoxFormat*>(rBox.GetFrameFormat());
        if( SfxItemState::SET == pBoxFormat->GetItemState( RES_BOXATR_FORMAT,
                false, &pFormatItem ) ||
            SfxItemState::SET == pBoxFormat->GetItemState( RES_BOXATR_VALUE,
                false, &pValueItem ))
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo( SwUndoId::TABLE_AUTOFMT, nullptr );
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
            GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
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

void SwDoc::ClearLineNumAttrs( SwPosition const & rPos )
{
    SwPaM aPam(rPos);
    aPam.Move(fnMoveBackward);
    SwContentNode *pNode = aPam.GetContentNode();
    if ( nullptr == pNode )
        return ;
    if( pNode->IsTextNode() )
    {
        SwTextNode * pTextNode = pNode->GetTextNode();
        if (pTextNode && pTextNode->IsNumbered()
            && pTextNode->GetText().isEmpty())
        {
            const SfxPoolItem* pFormatItem = nullptr;
            SfxItemSet rSet( pTextNode->GetDoc()->GetAttrPool(),
                        svl::Items<RES_PARATR_BEGIN, RES_PARATR_END - 1>{});
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
                    pUndo = nullptr;
                SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : nullptr );
                aRegH.RegisterInModify( pTextNode , *pTextNode );
                if ( pUndo )
                    pUndo->AddNode( *pTextNode );
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
    if( nullptr != ( pSttNd = rNode.GetNode().
                                FindSttNodeByType( SwTableBoxStartNode )) &&
        2 == pSttNd->EndOfSectionIndex() - pSttNd->GetIndex() )
    {
        SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
                            GetTableBox( pSttNd->GetIndex() );

        const SfxPoolItem* pFormatItem = nullptr;
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
        SwUndoCpyTable* pUndo = nullptr;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoCpyTable(this);
        }

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            bRet = pSrcTableNd->GetTable().MakeCopy( this, rInsPos, rBoxes,
                                                bCpyName );
        }

        if( pUndo )
        {
            if( !bRet )
            {
                delete pUndo;
                pUndo = nullptr;
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
        RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();
        if( getIDocumentRedlineAccess().IsRedlineOn() )
            getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::On |
                                  RedlineFlags::ShowInsert |
                                  RedlineFlags::ShowDelete );

        SwUndoTableCpyTable* pUndo = nullptr;
        if (bUndo)
        {
            GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoTableCpyTable(this);
            GetIDocumentUndoRedo().DoUndo(false);
        }

        rtl::Reference<SwDoc> xCpyDoc( const_cast<SwDoc*>(pSrcTableNd->GetDoc()) );
        bool bDelCpyDoc = xCpyDoc == this;

        if( bDelCpyDoc )
        {
            // Copy the Table into a temporary Doc
            xCpyDoc = new SwDoc;

            SwPosition aPos( SwNodeIndex( xCpyDoc->GetNodes().GetEndOfContent() ));
            if( !pSrcTableNd->GetTable().MakeCopy( xCpyDoc.get(), aPos, rBoxes, true ))
            {
                xCpyDoc.clear();

                if( pUndo )
                {
                    GetIDocumentUndoRedo().DoUndo(bUndo);
                    delete pUndo;
                    pUndo = nullptr;
                }
                return false;
            }
            aPos.nNode -= 1; // Set to the Table's EndNode
            pSrcTableNd = aPos.nNode.GetNode().FindTableNode();
        }

        const SwStartNode* pSttNd = rInsPos.nNode.GetNode().FindTableBoxStartNode();

        rInsPos.nContent.Assign( nullptr, 0 );

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

        xCpyDoc.clear();

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
        getIDocumentRedlineAccess().SetRedlineFlags( eOld );
    }

    if( bRet )
    {
        getIDocumentState().SetModified();
        getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
    }
    return bRet;
}

bool SwDoc::UnProtectTableCells( SwTable& rTable )
{
    bool bChgd = false;
    SwUndoAttrTable *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoAttrTable( *rTable.GetTableNode() )
        :   nullptr;

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

void SwDoc::UnProtectCells( const OUString& rName )
{
    bool bChgd = false;
    SwTableFormat* pFormat = FindTableFormatByName( rName );
    if( pFormat )
    {
        bChgd = UnProtectTableCells( *SwTable::FindTable( pFormat ) );
        if( bChgd )
            getIDocumentState().SetModified();
    }
}

bool SwDoc::UnProtectCells( const SwSelBoxes& rBoxes )
{
    bool bChgd = false;
    if( !rBoxes.empty() )
    {
        SwUndoAttrTable *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
                ? new SwUndoAttrTable( *rBoxes[0]->GetSttNd()->FindTableNode() )
                : nullptr;

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

void SwDoc::UnProtectTables( const SwPaM& rPam )
{
    GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    bool bChgd = false, bHasSel = rPam.HasMark() ||
                                    rPam.GetNext() != &rPam;
    SwFrameFormats& rFormats = *GetTableFrameFormats();
    SwTable* pTable;
    const SwTableNode* pTableNd;
    for( auto n = rFormats.size(); n ; )
        if( nullptr != (pTable = SwTable::FindTable( rFormats[ --n ] )) &&
            nullptr != (pTableNd = pTable->GetTableNode() ) &&
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

                } while( !bFound && &rPam != ( pTmp = pTmp->GetNext() ) );
                if( !bFound )
                    continue; // Continue searching
            }

            // Lift the protection
            bChgd |= UnProtectTableCells( *pTable );
        }

    GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
    if( bChgd )
        getIDocumentState().SetModified();
}

bool SwDoc::HasTableAnyProtection( const SwPosition* pPos,
                                 const OUString* pTableName,
                                 bool* pFullTableProtection )
{
    bool bHasProtection = false;
    SwTable* pTable = nullptr;
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

SwTableAutoFormat* SwDoc::MakeTableStyle(const OUString& rName, bool bBroadcast)
{
    SwTableAutoFormat aTableFormat(rName);
    GetTableStyles().AddAutoFormat(aTableFormat);
    SwTableAutoFormat* pTableFormat = GetTableStyles().FindAutoFormat(rName);

    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoTableStyleMake(rName, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rName, SfxStyleFamily::Table, SfxHintId::StyleSheetCreated);

    return pTableFormat;
}

std::unique_ptr<SwTableAutoFormat> SwDoc::DelTableStyle(const OUString& rName, bool bBroadcast)
{
    if (bBroadcast)
        BroadcastStyleOperation(rName, SfxStyleFamily::Table, SfxHintId::StyleSheetErased);

    std::unique_ptr<SwTableAutoFormat> pReleasedFormat = GetTableStyles().ReleaseAutoFormat(rName);

    std::vector<SwTable*> vAffectedTables;
    if (pReleasedFormat.get())
    {
        size_t nTableCount = GetTableFrameFormatCount(true);
        for (size_t i=0; i < nTableCount; ++i)
        {
            SwFrameFormat* pFrameFormat = &GetTableFrameFormat(i, true);
            SwTable* pTable = SwTable::FindTable(pFrameFormat);
            if (pTable->GetTableStyleName() == pReleasedFormat->GetName())
            {
                pTable->SetTableStyleName("");
                vAffectedTables.push_back(pTable);
            }
        }

        getIDocumentState().SetModified();

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo = new SwUndoTableStyleDelete(std::move(pReleasedFormat), vAffectedTables, this);

            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }

    return pReleasedFormat;
}

void SwDoc::ChgTableStyle(const OUString& rName, const SwTableAutoFormat& rNewFormat)
{
    SwTableAutoFormat* pFormat = GetTableStyles().FindAutoFormat(rName);
    if (pFormat)
    {
        SwTableAutoFormat aOldFormat = *pFormat;
        *pFormat = rNewFormat;
        pFormat->SetName(rName);

        size_t nTableCount = GetTableFrameFormatCount(true);
        for (size_t i=0; i < nTableCount; ++i)
        {
            SwFrameFormat* pFrameFormat = &GetTableFrameFormat(i, true);
            SwTable* pTable = SwTable::FindTable(pFrameFormat);
            if (pTable->GetTableStyleName() == rName)
                GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(pTable->GetTableNode());
        }

        getIDocumentState().SetModified();

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo = new SwUndoTableStyleUpdate(*pFormat, aOldFormat, this);

            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
