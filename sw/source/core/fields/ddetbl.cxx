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

#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <index.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swddetbl.hxx>
#include <ddefld.hxx>
#include <ndindex.hxx>
#include <fldupde.hxx>
#include <swtblfmt.hxx>
#include <fieldhint.hxx>

/// Ctor moves all lines/boxes from a SwTable into itself.
/// Afterwards the SwTable is empty and must be deleted.
SwDDETable::SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType, bool bUpdate )
    : SwTable(rTable), m_aDepends(*this), m_pDDEType(pDDEType)
{
    m_aDepends.StartListening(m_pDDEType);
    // copy the table data
    m_TabSortContentBoxes.insert(rTable.GetTabSortBoxes());
    rTable.GetTabSortBoxes().clear();

    m_aLines.insert( m_aLines.begin(),
                   rTable.GetTabLines().begin(), rTable.GetTabLines().end() ); // move lines
    rTable.GetTabLines().clear();

    if( !m_aLines.empty() )
    {
        const SwNode& rNd = *GetTabSortBoxes()[0]->GetSttNd();
        if( rNd.GetNodes().IsDocNodes() )
        {
            pDDEType->IncRefCnt();

            // update box content only if update flag is set (false in import)
            if (bUpdate)
                ChangeContent();
        }
    }
}

SwDDETable::~SwDDETable()
{
    SwDoc* pDoc = GetFrameFormat()->GetDoc();
    if (!pDoc->IsInDtor() && !m_aLines.empty())
    {
        assert(m_pTableNode);
        if (m_pTableNode->GetNodes().IsDocNodes())
        {
            m_pDDEType->DecRefCnt();
        }
    }

    // If it is the last dependent of the "deleted field" than delete it finally
    if( m_pDDEType->IsDeleted() && m_pDDEType->HasOnlyOneListener() )
    {
        m_aDepends.EndListeningAll();
        delete m_pDDEType;
        m_pDDEType = nullptr;
    }
}

void SwDDETable::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pNew && RES_UPDATEDDETBL == pNew->Which() )
        ChangeContent();
    else
        SwTable::Modify( pOld, pNew );
}

void SwDDETable::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    if(dynamic_cast<const SwFieldHint*>(&rHint))
        // replace DDETable by real table
        NoDDETable();
    else if(const auto pLinkAnchorHint = dynamic_cast<const sw::LinkAnchorSearchHint*>(&rHint))
    {
        if(pLinkAnchorHint->m_rpFoundNode)
            return;
        const auto pNd = GetTabSortBoxes()[0]->GetSttNd();
        if( pNd && &pLinkAnchorHint->m_rNodes == &pNd->GetNodes() )
            pLinkAnchorHint->m_rpFoundNode = pNd;
    }
    else if(const sw::InRangeSearchHint* pInRangeHint = dynamic_cast<const sw::InRangeSearchHint*>(&rHint))
    {
        if(pInRangeHint->m_rIsInRange)
            return;
        const SwTableNode* pTableNd = GetTabSortBoxes()[0]->GetSttNd()->FindTableNode();
        if( pTableNd->GetNodes().IsDocNodes() &&
                pInRangeHint->m_nSttNd < pTableNd->EndOfSectionIndex() &&
                pInRangeHint->m_nEndNd > pTableNd->GetIndex() )
            pInRangeHint->m_rIsInRange = true;
    }
    else if (auto pModifyChangedHint = dynamic_cast<const sw::ModifyChangedHint*>(&rHint))
    {
        if(m_pDDEType == &rModify)
            m_pDDEType = const_cast<SwDDEFieldType*>(static_cast<const SwDDEFieldType*>(pModifyChangedHint->m_pNew));
    }
}

void SwDDETable::ChangeContent()
{
    OSL_ENSURE( GetFrameFormat(), "No FrameFormat" );

    // Is this the correct NodesArray? (because of UNDO)
    if( m_aLines.empty() )
        return;
    OSL_ENSURE( !GetTabSortBoxes().empty(), "Table without content?" );
    if( !GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        return;


    OUString aExpand = m_pDDEType->GetExpansion().replaceAll("\r", "");
    sal_Int32 nExpandTokenPos = 0;

    for( size_t n = 0; n < m_aLines.size(); ++n )
    {
        OUString aLine = aExpand.getToken( 0, '\n', nExpandTokenPos );
        sal_Int32 nLineTokenPos = 0;
        SwTableLine* pLine = m_aLines[ n ];
        for( size_t i = 0; i < pLine->GetTabBoxes().size(); ++i )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[ i ].get();
            OSL_ENSURE( pBox->GetSttIdx(), "no content box" );
            SwNodeIndex aNdIdx( *pBox->GetSttNd(), 1 );
            SwTextNode* pTextNode = aNdIdx.GetNode().GetTextNode();
            OSL_ENSURE( pTextNode, "No Node" );
            SwIndex aCntIdx( pTextNode, 0 );
            pTextNode->EraseText( aCntIdx );
            pTextNode->InsertText( aLine.getToken( 0, '\t', nLineTokenPos ), aCntIdx );

            SwTableBoxFormat* pBoxFormat = static_cast<SwTableBoxFormat*>(pBox->GetFrameFormat());
            pBoxFormat->LockModify();
            pBoxFormat->ResetFormatAttr( RES_BOXATR_VALUE );
            pBoxFormat->UnlockModify();
        }
    }

    const IDocumentSettingAccess& rIDSA = GetFrameFormat()->getIDocumentSettingAccess();
    SwDoc* pDoc = GetFrameFormat()->GetDoc();
    if( AUTOUPD_FIELD_AND_CHARTS == rIDSA.getFieldUpdateFlags(true) )
        pDoc->getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, 0 );
}

SwDDEFieldType* SwDDETable::GetDDEFieldType()
{
    return m_pDDEType;
}

bool SwDDETable::NoDDETable()
{
    // search table node
    OSL_ENSURE( GetFrameFormat(), "No FrameFormat" );
    SwDoc* pDoc = GetFrameFormat()->GetDoc();

    // Is this the correct NodesArray? (because of UNDO)
    if( m_aLines.empty() )
        return false;
    OSL_ENSURE( !GetTabSortBoxes().empty(), "Table without content?" );
    SwNode* pNd = const_cast<SwNode*>(static_cast<SwNode const *>(GetTabSortBoxes()[0]->GetSttNd()));
    if( !pNd->GetNodes().IsDocNodes() )
        return false;

    SwTableNode* pTableNd = pNd->FindTableNode();
    OSL_ENSURE( pTableNd, "Where is the table?");

    std::unique_ptr<SwTable> pNewTable(new SwTable( *this ));

    // copy the table data
    pNewTable->GetTabSortBoxes().insert( GetTabSortBoxes() ); // move content boxes
    GetTabSortBoxes().clear();

    pNewTable->GetTabLines().insert( pNewTable->GetTabLines().begin(),
                                   GetTabLines().begin(), GetTabLines().end() ); // move lines
    GetTabLines().clear();

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
        m_pDDEType->DecRefCnt();

    pTableNd->SetNewTable( std::move(pNewTable) );       // replace table

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
