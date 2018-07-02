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
#include <ndindex.hxx>
#include <vector>


#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif


// Structure for the mapping from old and new frame formats to the
// boxes and lines of a table
struct MapTableFrameFormat
{
    const SwFrameFormat *pOld;
    SwFrameFormat *pNew;
    MapTableFrameFormat( const SwFrameFormat *pOldFormat, SwFrameFormat*pNewFormat )
        : pOld( pOldFormat ), pNew( pNewFormat )
    {}
};

typedef std::vector<MapTableFrameFormat> MapTableFrameFormats;

SwContentNode* SwTextNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // the Copy-Textnode is the Node with the Text, the Copy-Attrnode is the
    // node with the collection and hard attributes. Normally is the same
    // node, but if insert a glossary without formatting, then the Attrnode
    // is the prev node of the destination position in dest. document.
    SwTextNode* pCpyTextNd = const_cast<SwTextNode*>(this);
    SwTextNode* pCpyAttrNd = pCpyTextNd;

    // Copy the formats to the other document
    SwTextFormatColl* pColl = nullptr;
    if( pDoc->IsInsOnlyTextGlossary() )
    {
        SwNodeIndex aIdx( rIdx, -1 );
        if( aIdx.GetNode().IsTextNode() )
        {
            pCpyAttrNd = aIdx.GetNode().GetTextNode();
            pColl = &pCpyAttrNd->GetTextColl()->GetNextTextFormatColl();
        }
    }
    if( !pColl )
        pColl = pDoc->CopyTextColl( *GetTextColl() );

    SwTextNode* pTextNd = pDoc->GetNodes().MakeTextNode( rIdx, pColl );

    // METADATA: register copy
    pTextNd->RegisterAsCopyOf(*pCpyTextNd);

    // Copy Attribute/Text
    if( !pCpyAttrNd->HasSwAttrSet() )
        // An AttrSet was added for numbering, so delete it
        pTextNd->ResetAllAttr();

    // if Copy-Textnode unequal to Copy-Attrnode, then copy first
    // the attributes into the new Node.
    if( pCpyAttrNd != pCpyTextNd )
    {
        pCpyAttrNd->CopyAttr( pTextNd, 0, 0 );
        if( pCpyAttrNd->HasSwAttrSet() )
        {
            SwAttrSet aSet( *pCpyAttrNd->GetpSwAttrSet() );
            aSet.ClearItem( RES_PAGEDESC );
            aSet.ClearItem( RES_BREAK );
            aSet.CopyToModify( *pTextNd );
        }
    }

    // Is that enough? What about PostIts/Fields/FieldTypes?
    // #i96213# - force copy of all attributes
    pCpyTextNd->CopyText( pTextNd, SwIndex( pCpyTextNd ),
        pCpyTextNd->GetText().getLength(), true );

    if( RES_CONDTXTFMTCOLL == pColl->Which() )
        pTextNd->ChkCondColl();

    return pTextNd;
}

static bool lcl_SrchNew( const MapTableFrameFormat& rMap, SwFrameFormat** pPara )
{
    if( rMap.pOld != *pPara )
        return true;
    *pPara = rMap.pNew;
    return false;
}

struct CopyTable
{
    SwDoc* m_pDoc;
    sal_uLong m_nOldTableSttIdx;
    MapTableFrameFormats& m_rMapArr;
    SwTableLine* m_pInsLine;
    SwTableBox* m_pInsBox;
    SwTableNode *m_pTableNd;
    const SwTable *m_pOldTable;

    CopyTable(SwDoc* pDc, MapTableFrameFormats& rArr, sal_uLong nOldStt,
               SwTableNode& rTableNd, const SwTable* pOldTable)
        : m_pDoc(pDc), m_nOldTableSttIdx(nOldStt), m_rMapArr(rArr),
          m_pInsLine(nullptr), m_pInsBox(nullptr), m_pTableNd(&rTableNd), m_pOldTable(pOldTable)
    {}
};

static void lcl_CopyTableLine( const SwTableLine* pLine, CopyTable* pCT );

static void lcl_CopyTableBox( SwTableBox* pBox, CopyTable* pCT )
{
    SwTableBoxFormat * pBoxFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());
    for (MapTableFrameFormats::const_iterator it = pCT->m_rMapArr.begin(); it != pCT->m_rMapArr.end(); ++it)
        if ( !lcl_SrchNew( *it, reinterpret_cast<SwFrameFormat**>(&pBoxFormat) ) )
            break;

    if (pBoxFormat == pBox->GetFrameFormat()) // Create a new one?
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pBoxFormat->GetItemState( RES_BOXATR_FORMULA, false,
            &pItem ) && static_cast<const SwTableBoxFormula*>(pItem)->IsIntrnlName() )
        {
            const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem))->PtrToBoxNm(pCT->m_pOldTable);
        }

        pBoxFormat = pCT->m_pDoc->MakeTableBoxFormat();
        pBoxFormat->CopyAttrs( *pBox->GetFrameFormat() );

        if( pBox->GetSttIdx() )
        {
            SvNumberFormatter* pN = pCT->m_pDoc->GetNumberFormatter(false);
            if( pN && pN->HasMergeFormatTable() && SfxItemState::SET == pBoxFormat->
                GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
            {
                sal_uLong nOldIdx = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();
                sal_uLong nNewIdx = pN->GetMergeFormatIndex( nOldIdx );
                if( nNewIdx != nOldIdx )
                    pBoxFormat->SetFormatAttr( SwTableBoxNumFormat( nNewIdx ));

            }
        }

        pCT->m_rMapArr.emplace_back(pBox->GetFrameFormat(), pBoxFormat);
    }

    sal_uInt16 nLines = pBox->GetTabLines().size();
    SwTableBox* pNewBox;
    if( nLines )
        pNewBox = new SwTableBox(pBoxFormat, nLines, pCT->m_pInsLine);
    else
    {
        SwNodeIndex aNewIdx(*pCT->m_pTableNd, pBox->GetSttIdx() - pCT->m_nOldTableSttIdx);
        assert(aNewIdx.GetNode().IsStartNode() && "Index is not on the start node");

        pNewBox = new SwTableBox(pBoxFormat, aNewIdx, pCT->m_pInsLine);
        pNewBox->setRowSpan( pBox->getRowSpan() );
    }

    pCT->m_pInsLine->GetTabBoxes().emplace_back( pNewBox );

    if (nLines)
    {
        CopyTable aPara(*pCT);
        aPara.m_pInsBox = pNewBox;
        for( const SwTableLine* pLine : pBox->GetTabLines() )
            lcl_CopyTableLine( pLine, &aPara );
    }
    else if (pNewBox->IsInHeadline(&pCT->m_pTableNd->GetTable()))
    {
        // In the headline, the paragraphs must match conditional styles
        pNewBox->GetSttNd()->CheckSectionCondColl();
    }
}

static void lcl_CopyTableLine( const SwTableLine* pLine, CopyTable* pCT )
{
    SwTableLineFormat * pLineFormat = static_cast<SwTableLineFormat*>(pLine->GetFrameFormat());
    for (MapTableFrameFormats::const_iterator it = pCT->m_rMapArr.begin(); it != pCT->m_rMapArr.end(); ++it)
        if ( !lcl_SrchNew( *it, reinterpret_cast<SwFrameFormat**>(&pLineFormat) ) )
            break;

    if( pLineFormat == pLine->GetFrameFormat() ) // Create a new one?
    {
        pLineFormat = pCT->m_pDoc->MakeTableLineFormat();
        pLineFormat->CopyAttrs( *pLine->GetFrameFormat() );
        pCT->m_rMapArr.emplace_back(pLine->GetFrameFormat(), pLineFormat);
    }

    SwTableLine* pNewLine = new SwTableLine(pLineFormat, pLine->GetTabBoxes().size(), pCT->m_pInsBox);
    // Insert the new row into the table
    if (pCT->m_pInsBox)
    {
        pCT->m_pInsBox->GetTabLines().push_back(pNewLine);
    }
    else
    {
        pCT->m_pTableNd->GetTable().GetTabLines().push_back(pNewLine);
    }

    pCT->m_pInsLine = pNewLine;
    for( std::unique_ptr<SwTableBox> const & rpBox : const_cast<SwTableLine*>(pLine)->GetTabBoxes() )
        lcl_CopyTableBox(rpBox.get(), pCT );
}

SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // In which array are we? Nodes? UndoNodes?
    SwNodes& rNds = const_cast<SwNodes&>(GetNodes());

    {
        if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return nullptr;
    }

    // Copy the TableFrameFormat
    OUString sTableName( GetTable().GetFrameFormat()->GetName() );
    if( !pDoc->IsCopyIsMove() )
    {
        const SwFrameFormats& rTableFormats = *pDoc->GetTableFrameFormats();
        for( size_t n = rTableFormats.size(); n; )
            if( rTableFormats[ --n ]->GetName() == sTableName )
            {
                sTableName = pDoc->GetUniqueTableName();
                break;
            }
    }

    SwFrameFormat* pTableFormat = pDoc->MakeTableFrameFormat( sTableName, pDoc->GetDfltFrameFormat() );
    pTableFormat->CopyAttrs( *GetTable().GetFrameFormat() );
    SwTableNode* pTableNd = new SwTableNode( rIdx );
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pTableNd );
    SwNodeIndex aInsPos( *pEndNd );

    SwTable& rTable = pTableNd->GetTable();
    rTable.RegisterToFormat( *pTableFormat );

    rTable.SetRowsToRepeat( GetTable().GetRowsToRepeat() );
    rTable.SetTableChgMode( GetTable().GetTableChgMode() );
    rTable.SetTableModel( GetTable().IsNewModel() );

    SwDDEFieldType* pDDEType = nullptr;
    if( auto pSwDDETable = dynamic_cast<const SwDDETable*>( &GetTable() ) )
    {
        // We're copying a DDE table
        // Is the field type available in the new document?
        pDDEType = const_cast<SwDDETable*>(pSwDDETable)->GetDDEFieldType();
        if( pDDEType->IsDeleted() )
            pDoc->getIDocumentFieldsAccess().InsDeletedFieldType( *pDDEType );
        else
            pDDEType = static_cast<SwDDEFieldType*>(pDoc->getIDocumentFieldsAccess().InsertFieldType( *pDDEType ));
        OSL_ENSURE( pDDEType, "unknown FieldType" );

        // Swap the table pointers in the node
        std::unique_ptr<SwDDETable> pNewTable(new SwDDETable( pTableNd->GetTable(), pDDEType ));
        pTableNd->SetNewTable( std::move(pNewTable), false );
    }
    // First copy the content of the tables, we will later assign the
    // boxes/lines and create the frames
    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );

    // If there is a table in this table, the table format for the outer table
    // does not seem to be used, because the table does not have any contents yet
    // (see IsUsed). Therefore the inner table gets the same name as the outer table.
    // We have to make sure that the table node of the SwTable is accessible, even
    // without any content in m_TabSortContentBoxes. #i26629#
    pTableNd->GetTable().SetTableNode( pTableNd );
    rNds.Copy_( aRg, aInsPos, false );
    pTableNd->GetTable().SetTableNode( nullptr );

    // Special case for a single box
    if( 1 == GetTable().GetTabSortBoxes().size() )
    {
        aRg.aStart.Assign( *pTableNd, 1 );
        aRg.aEnd.Assign( *pTableNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRg, SwTableBoxStartNode );
    }

    // Delete all frames from the copied area, they will be created
    // during the generation of the table frame
    pTableNd->DelFrames();

    MapTableFrameFormats aMapArr;
    CopyTable aPara( pDoc, aMapArr, GetIndex(), *pTableNd, &GetTable() );

    for( const SwTableLine* pLine : GetTable().GetTabLines() )
        lcl_CopyTableLine( pLine, &aPara );

    if( pDDEType )
        pDDEType->IncRefCnt();

    CHECK_TABLE( GetTable() );
    return pTableNd;
}

void SwTextNode::CopyCollFormat( SwTextNode& rDestNd )
{
    // Copy the formats into the other document:
    // Special case for PageBreak/PageDesc/ColBrk
    SwDoc* pDestDoc = rDestNd.GetDoc();
    SwAttrSet aPgBrkSet( pDestDoc->GetAttrPool(), aBreakSetRange );
    const SwAttrSet* pSet;

    if( nullptr != ( pSet = rDestNd.GetpSwAttrSet() ) )
    {
        // Special cases for Break-Attributes
        const SfxPoolItem* pAttr;
        if( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );

        if( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );
    }

    rDestNd.ChgFormatColl( pDestDoc->CopyTextColl( *GetTextColl() ));
    if( nullptr != ( pSet = GetpSwAttrSet() ) )
        pSet->CopyToModify( rDestNd );

    if( aPgBrkSet.Count() )
        rDestNd.SetAttr( aPgBrkSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
