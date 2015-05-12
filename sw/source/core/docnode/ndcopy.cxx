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
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <node.hxx>
#include <frmfmt.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <swtblfmt.hxx>
#include <cellatr.hxx>
#include <docary.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <svtools/fmtfield.hxx>
#include <vector>


#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif



// Structure for the mapping from old and new frame formats to the
// boxes and lines of a table
struct _MapTblFrmFmt
{
    const SwFrmFmt *pOld;
    SwFrmFmt *pNew;
    _MapTblFrmFmt( const SwFrmFmt *pOldFmt, SwFrmFmt*pNewFmt )
        : pOld( pOldFmt ), pNew( pNewFmt )
    {}
};

typedef std::vector<_MapTblFrmFmt> _MapTblFrmFmts;

SwCntntNode* SwTxtNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // the Copy-Textnode is the Node with the Text, the Copy-Attrnode is the
    // node with the collection and hard attributes. Normally is the same
    // node, but if insert a glossary without formatting, then the Attrnode
    // is the prev node of the destionation position in dest. document.
    SwTxtNode* pCpyTxtNd = const_cast<SwTxtNode*>(this);
    SwTxtNode* pCpyAttrNd = pCpyTxtNd;

    // Copy the formats to the other document
    SwTxtFmtColl* pColl = 0;
    if( pDoc->IsInsOnlyTextGlossary() )
    {
        SwNodeIndex aIdx( rIdx, -1 );
        if( aIdx.GetNode().IsTxtNode() )
        {
            pCpyAttrNd = aIdx.GetNode().GetTxtNode();
            pColl = &pCpyAttrNd->GetTxtColl()->GetNextTxtFmtColl();
        }
    }
    if( !pColl )
        pColl = pDoc->CopyTxtColl( *GetTxtColl() );

    SwTxtNode* pTxtNd = pDoc->GetNodes().MakeTxtNode( rIdx, pColl );

    // METADATA: register copy
    pTxtNd->RegisterAsCopyOf(*pCpyTxtNd);

    // Copy Attribute/Text
    if( !pCpyAttrNd->HasSwAttrSet() )
        // An AttrSet was added for numbering, so delete it
        pTxtNd->ResetAllAttr();

    // if Copy-Textnode unequal to Copy-Attrnode, then copy first
    // the attributes into the new Node.
    if( pCpyAttrNd != pCpyTxtNd )
    {
        pCpyAttrNd->CopyAttr( pTxtNd, 0, 0 );
        if( pCpyAttrNd->HasSwAttrSet() )
        {
            SwAttrSet aSet( *pCpyAttrNd->GetpSwAttrSet() );
            aSet.ClearItem( RES_PAGEDESC );
            aSet.ClearItem( RES_BREAK );
            aSet.CopyToModify( *pTxtNd );
        }
    }

    // Is that enough? What about PostIts/Fields/FieldTypes?
    // #i96213# - force copy of all attributes
    pCpyTxtNd->CopyText( pTxtNd, SwIndex( pCpyTxtNd ),
        pCpyTxtNd->GetTxt().getLength(), true );

    if( RES_CONDTXTFMTCOLL == pColl->Which() )
        pTxtNd->ChkCondColl();

    return pTxtNd;
}

static bool lcl_SrchNew( const _MapTblFrmFmt& rMap, SwFrmFmt** pPara )
{
    if( rMap.pOld != *pPara )
        return true;
    *pPara = rMap.pNew;
    return false;
}

struct _CopyTable
{
    SwDoc* m_pDoc;
    sal_uLong m_nOldTblSttIdx;
    _MapTblFrmFmts& m_rMapArr;
    SwTableLine* m_pInsLine;
    SwTableBox* m_pInsBox;
    SwTableNode *m_pTblNd;
    const SwTable *m_pOldTable;

    _CopyTable(SwDoc* pDc, _MapTblFrmFmts& rArr, sal_uLong nOldStt,
               SwTableNode& rTblNd, const SwTable* pOldTable)
        : m_pDoc(pDc), m_nOldTblSttIdx(nOldStt), m_rMapArr(rArr),
          m_pInsLine(0), m_pInsBox(0), m_pTblNd(&rTblNd), m_pOldTable(pOldTable)
    {}
};

static void lcl_CopyTblLine( const SwTableLine* pLine, _CopyTable* pCT );

static void lcl_CopyTblBox( SwTableBox* pBox, _CopyTable* pCT )
{
    SwTableBoxFmt * pBoxFmt = static_cast<SwTableBoxFmt*>(pBox->GetFrmFmt());
    for (_MapTblFrmFmts::const_iterator it = pCT->m_rMapArr.begin(); it != pCT->m_rMapArr.end(); ++it)
        if ( !lcl_SrchNew( *it, reinterpret_cast<SwFrmFmt**>(&pBoxFmt) ) )
            break;

    if (pBoxFmt == pBox->GetFrmFmt()) // Create a new one?
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pBoxFmt->GetItemState( RES_BOXATR_FORMULA, false,
            &pItem ) && static_cast<const SwTblBoxFormula*>(pItem)->IsIntrnlName() )
        {
            const_cast<SwTblBoxFormula*>(static_cast<const SwTblBoxFormula*>(pItem))->PtrToBoxNm(pCT->m_pOldTable);
        }

        pBoxFmt = pCT->m_pDoc->MakeTableBoxFmt();
        pBoxFmt->CopyAttrs( *pBox->GetFrmFmt() );

        if( pBox->GetSttIdx() )
        {
            SvNumberFormatter* pN = pCT->m_pDoc->GetNumberFormatter(false);
            if( pN && pN->HasMergeFmtTbl() && SfxItemState::SET == pBoxFmt->
                GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
            {
                sal_uLong nOldIdx = static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue();
                sal_uLong nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                if( nNewIdx != nOldIdx )
                    pBoxFmt->SetFmtAttr( SwTblBoxNumFormat( nNewIdx ));

            }
        }

        pCT->m_rMapArr.push_back(_MapTblFrmFmt(pBox->GetFrmFmt(), pBoxFmt));
    }

    sal_uInt16 nLines = pBox->GetTabLines().size();
    SwTableBox* pNewBox;
    if( nLines )
        pNewBox = new SwTableBox(pBoxFmt, nLines, pCT->m_pInsLine);
    else
    {
        SwNodeIndex aNewIdx(*pCT->m_pTblNd, pBox->GetSttIdx() - pCT->m_nOldTblSttIdx);
        OSL_ENSURE( aNewIdx.GetNode().IsStartNode(), "Index is not on the start node" );

        pNewBox = new SwTableBox(pBoxFmt, aNewIdx, pCT->m_pInsLine);
        pNewBox->setRowSpan( pBox->getRowSpan() );
    }

    pCT->m_pInsLine->GetTabBoxes().push_back( pNewBox );

    if (nLines)
    {
        _CopyTable aPara(*pCT);
        aPara.m_pInsBox = pNewBox;
        for( const SwTableLine* pLine : pBox->GetTabLines() )
            lcl_CopyTblLine( pLine, &aPara );
    }
    else if (pNewBox->IsInHeadline(&pCT->m_pTblNd->GetTable()))
    {
        // In the headline, the paragraphs must match conditional styles
        pNewBox->GetSttNd()->CheckSectionCondColl();
    }
}

static void lcl_CopyTblLine( const SwTableLine* pLine, _CopyTable* pCT )
{
    SwTableLineFmt * pLineFmt = static_cast<SwTableLineFmt*>(pLine->GetFrmFmt());
    for (_MapTblFrmFmts::const_iterator it = pCT->m_rMapArr.begin(); it != pCT->m_rMapArr.end(); ++it)
        if ( !lcl_SrchNew( *it, reinterpret_cast<SwFrmFmt**>(&pLineFmt) ) )
            break;

    if( pLineFmt == pLine->GetFrmFmt() ) // Create a new one?
    {
        pLineFmt = pCT->m_pDoc->MakeTableLineFmt();
        pLineFmt->CopyAttrs( *pLine->GetFrmFmt() );
        pCT->m_rMapArr.push_back(_MapTblFrmFmt(pLine->GetFrmFmt(), pLineFmt));
    }

    SwTableLine* pNewLine = new SwTableLine(pLineFmt, pLine->GetTabBoxes().size(), pCT->m_pInsBox);
    // Insert the new row into the table
    if (pCT->m_pInsBox)
    {
        pCT->m_pInsBox->GetTabLines().push_back(pNewLine);
    }
    else
    {
        pCT->m_pTblNd->GetTable().GetTabLines().push_back(pNewLine);
    }

    pCT->m_pInsLine = pNewLine;
    for( SwTableBoxes::iterator it = const_cast<SwTableLine*>(pLine)->GetTabBoxes().begin();
             it != const_cast<SwTableLine*>(pLine)->GetTabBoxes().end(); ++it)
        lcl_CopyTblBox(*it, pCT );
}

SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // In which array are we? Nodes? UndoNodes?
    SwNodes& rNds = (SwNodes&)GetNodes();

    {
        if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;
    }

    // Copy the TableFrmFmt
    OUString sTblName( GetTable().GetFrmFmt()->GetName() );
    if( !pDoc->IsCopyIsMove() )
    {
        const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
        for( size_t n = rTblFmts.size(); n; )
            if( rTblFmts[ --n ]->GetName() == sTblName )
            {
                sTblName = pDoc->GetUniqueTblName();
                break;
            }
    }

    SwFrmFmt* pTblFmt = pDoc->MakeTblFrmFmt( sTblName, pDoc->GetDfltFrmFmt() );
    pTblFmt->CopyAttrs( *GetTable().GetFrmFmt() );
    SwTableNode* pTblNd = new SwTableNode( rIdx );
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pTblNd );
    SwNodeIndex aInsPos( *pEndNd );

    SwTable& rTbl = (SwTable&)pTblNd->GetTable();
    rTbl.RegisterToFormat( *pTblFmt );

    rTbl.SetRowsToRepeat( GetTable().GetRowsToRepeat() );
    rTbl.SetTblChgMode( GetTable().GetTblChgMode() );
    rTbl.SetTableModel( GetTable().IsNewModel() );

    SwDDEFieldType* pDDEType = 0;
    if( IS_TYPE( SwDDETable, &GetTable() ))
    {
        // We're copying a DDE table
        // Is the field type available in the new document?
        pDDEType = const_cast<SwDDETable&>(static_cast<const SwDDETable&>(GetTable())).GetDDEFldType();
        if( pDDEType->IsDeleted() )
            pDoc->getIDocumentFieldsAccess().InsDeletedFldType( *pDDEType );
        else
            pDDEType = static_cast<SwDDEFieldType*>(pDoc->getIDocumentFieldsAccess().InsertFldType( *pDDEType ));
        OSL_ENSURE( pDDEType, "unknown FieldType" );

        // Swap the table pointers in the node
        SwDDETable* pNewTable = new SwDDETable( pTblNd->GetTable(), pDDEType );
        pTblNd->SetNewTable( pNewTable, false );
    }
    // First copy the content of the tables, we will later assign the
    // boxes/lines and create the frames
    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );

    // If there is a table in this table, the table format for the outer table
    // does not seem to be used, because the table does not have any contents yet
    // (see IsUsed). Therefore the inner table gets the same name as the outer table.
    // We have to make sure that the table node of the SwTable is accessible, even
    // without any content in m_TabSortContentBoxes. #i26629#
    pTblNd->GetTable().SetTableNode( pTblNd );
    rNds._Copy( aRg, aInsPos, false );
    pTblNd->GetTable().SetTableNode( 0 );

    // Special case for a single box
    if( 1 == GetTable().GetTabSortBoxes().size() )
    {
        aRg.aStart.Assign( *pTblNd, 1 );
        aRg.aEnd.Assign( *pTblNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRg, SwTableBoxStartNode );
    }

    // Delete all frames from the copied area, they will be created
    // during the generation of the table frame
    pTblNd->DelFrms();

    _MapTblFrmFmts aMapArr;
    _CopyTable aPara( pDoc, aMapArr, GetIndex(), *pTblNd, &GetTable() );

    for( const SwTableLine* pLine : GetTable().GetTabLines() )
        lcl_CopyTblLine( pLine, &aPara );

    if( pDDEType )
        pDDEType->IncRefCnt();

    CHECK_TABLE( GetTable() );
    return pTblNd;
}

void SwTxtNode::CopyCollFmt( SwTxtNode& rDestNd )
{
    // Copy the formats into the other document:
    // Special case for PageBreak/PageDesc/ColBrk
    SwDoc* pDestDoc = rDestNd.GetDoc();
    SwAttrSet aPgBrkSet( pDestDoc->GetAttrPool(), aBreakSetRange );
    const SwAttrSet* pSet;

    if( 0 != ( pSet = rDestNd.GetpSwAttrSet() ) )
    {
        // Special cases for Break-Attributes
        const SfxPoolItem* pAttr;
        if( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );

        if( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );
    }

    rDestNd.ChgFmtColl( pDestDoc->CopyTxtColl( *GetTxtColl() ));
    if( 0 != ( pSet = GetpSwAttrSet() ) )
        pSet->CopyToModify( rDestNd );

    if( aPgBrkSet.Count() )
        rDestNd.SetAttr( aPgBrkSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
