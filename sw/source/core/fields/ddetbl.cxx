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

#include <comphelper/string.hxx>
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

TYPEINIT1( SwDDETable, SwTable );

/// Ctor moves all lines/boxes from a SwTable into itself.
/// Afterwards the SwTable is empty and must be deleted.
SwDDETable::SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType, bool bUpdate )
    : SwTable( rTable ), aDepend( this, pDDEType )
{
    // copy the table data
    m_TabSortContentBoxes.insert(rTable.GetTabSortBoxes());
    rTable.GetTabSortBoxes().clear();

    aLines.insert( aLines.begin(),
                   rTable.GetTabLines().begin(), rTable.GetTabLines().end() ); // move lines
    rTable.GetTabLines().clear();

    if( !aLines.empty() )
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
    SwDDEFieldType* pFldTyp = static_cast<SwDDEFieldType*>(aDepend.GetRegisteredIn());
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    if( !pDoc->IsInDtor() && !aLines.empty() &&
        GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        pFldTyp->DecRefCnt();

    // If it is the last dependent of the "deleted field" than delete it finally
    if( pFldTyp->IsDeleted() && pFldTyp->HasOnlyOneListener() )
    {
        pFldTyp->Remove( &aDepend );
        delete pFldTyp;
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
    const SwFieldHint* pHint = dynamic_cast<const SwFieldHint*>( &rHint );
    if ( pHint )
        // replace DDETable by real table
        NoDDETable();
}

void SwDDETable::ChangeContent()
{
    OSL_ENSURE( GetFrmFmt(), "No FrameFormat" );

    // Is this the correct NodesArray? (because of UNDO)
    if( aLines.empty() )
        return;
    OSL_ENSURE( !GetTabSortBoxes().empty(), "Table without content?" );
    if( !GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        return;

    // access to DDEFldType
    SwDDEFieldType* pDDEType = static_cast<SwDDEFieldType*>(aDepend.GetRegisteredIn());

    OUString aExpand = comphelper::string::remove(pDDEType->GetExpansion(), '\r');
    sal_Int32 nExpandTokenPos = 0;

    for( size_t n = 0; n < aLines.size(); ++n )
    {
        OUString aLine = aExpand.getToken( 0, '\n', nExpandTokenPos );
        sal_Int32 nLineTokenPos = 0;
        SwTableLine* pLine = aLines[ n ];
        for( size_t i = 0; i < pLine->GetTabBoxes().size(); ++i )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[ i ];
            OSL_ENSURE( pBox->GetSttIdx(), "no content box" );
            SwNodeIndex aNdIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pTxtNode = aNdIdx.GetNode().GetTxtNode();
            OSL_ENSURE( pTxtNode, "No Node" );
            SwIndex aCntIdx( pTxtNode, 0 );
            pTxtNode->EraseText( aCntIdx );
            pTxtNode->InsertText( aLine.getToken( 0, '\t', nLineTokenPos ), aCntIdx );

            SwTableBoxFmt* pBoxFmt = static_cast<SwTableBoxFmt*>(pBox->GetFrmFmt());
            pBoxFmt->LockModify();
            pBoxFmt->ResetFmtAttr( RES_BOXATR_VALUE );
            pBoxFmt->UnlockModify();
        }
    }

    const IDocumentSettingAccess* pIDSA = GetFrmFmt()->getIDocumentSettingAccess();
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    if( AUTOUPD_FIELD_AND_CHARTS == pIDSA->getFieldUpdateFlags(true) )
        pDoc->getIDocumentFieldsAccess().SetFieldsDirty( true, NULL, 0 );
}

SwDDEFieldType* SwDDETable::GetDDEFldType()
{
    return static_cast<SwDDEFieldType*>(aDepend.GetRegisteredIn());
}

bool SwDDETable::NoDDETable()
{
    // search table node
    OSL_ENSURE( GetFrmFmt(), "No FrameFormat" );
    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    // Is this the correct NodesArray? (because of UNDO)
    if( aLines.empty() )
        return false;
    OSL_ENSURE( !GetTabSortBoxes().empty(), "Table without content?" );
    SwNode* pNd = (SwNode*)GetTabSortBoxes()[0]->GetSttNd();
    if( !pNd->GetNodes().IsDocNodes() )
        return false;

    SwTableNode* pTblNd = pNd->FindTableNode();
    OSL_ENSURE( pTblNd, "Where is the table?");

    SwTable* pNewTbl = new SwTable( *this );

    // copy the table data
    pNewTbl->GetTabSortBoxes().insert( GetTabSortBoxes() ); // move content boxes
    GetTabSortBoxes().clear();

    pNewTbl->GetTabLines().insert( pNewTbl->GetTabLines().begin(),
                                   GetTabLines().begin(), GetTabLines().end() ); // move lines
    GetTabLines().clear();

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
        static_cast<SwDDEFieldType*>(aDepend.GetRegisteredIn())->DecRefCnt();

    pTblNd->SetNewTable( pNewTbl );       // replace table

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
