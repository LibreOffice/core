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
#include <index.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swddetbl.hxx>
#include <ddefld.hxx> // for FieldType
#include <ndindex.hxx>
#include <fldupde.hxx>
#include <swtblfmt.hxx>
#include <fieldhint.hxx>

TYPEINIT1( SwDDETable, SwTable );

/// Ctor moves all lines/boxes from a SwTable into itself.
/// Afterwards the SwTable is empty and must be deleted.
SwDDETable::SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType, sal_Bool bUpdate )
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
            // mba: swclient refactoring - this code shouldn't have done anything!
            // the ModifyLock Flag is evaluated in SwModify only, though it was accessible via SwClient
            // This has been fixed now
//          aDepend.LockModify();
            pDDEType->IncRefCnt();
//          aDepend.UnlockModify();

            // update box content only if update flag is set (false in import)
            if (bUpdate)
                ChangeContent();
        }
    }
}

SwDDETable::~SwDDETable()
{
    SwDDEFieldType* pFldTyp = (SwDDEFieldType*)aDepend.GetRegisteredIn();
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    if( !pDoc->IsInDtor() && !aLines.empty() &&
        GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        pFldTyp->DecRefCnt();

    // If it is the last dependant of the "deleted field" than delete it finally
    if( pFldTyp->IsDeleted() && pFldTyp->IsLastDepend() )
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

void SwDDETable::SwClientNotify( const SwModify&, const SfxHint& rHint )
{
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
    SwDDEFieldType* pDDEType = (SwDDEFieldType*)aDepend.GetRegisteredIn();

    String aExpand = comphelper::string::remove(pDDEType->GetExpansion(), '\r');

    for( sal_uInt16 n = 0; n < aLines.size(); ++n )
    {
        String aLine = aExpand.GetToken( n, '\n' );
        SwTableLine* pLine = aLines[ n ];
        for( sal_uInt16 i = 0; i < pLine->GetTabBoxes().size(); ++i )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[ i ];
            OSL_ENSURE( pBox->GetSttIdx(), "no content box" );
            SwNodeIndex aNdIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pTxtNode = aNdIdx.GetNode().GetTxtNode();
            OSL_ENSURE( pTxtNode, "No Node" );
            SwIndex aCntIdx( pTxtNode, 0 );
            pTxtNode->EraseText( aCntIdx );
            pTxtNode->InsertText( aLine.GetToken( i, '\t' ), aCntIdx );

            SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
            pBoxFmt->LockModify();
            pBoxFmt->ResetFmtAttr( RES_BOXATR_VALUE );
            pBoxFmt->UnlockModify();
        }
    }

    const IDocumentSettingAccess* pIDSA = GetFrmFmt()->getIDocumentSettingAccess();
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    if( AUTOUPD_FIELD_AND_CHARTS == pIDSA->getFieldUpdateFlags(true) )
        pDoc->SetFieldsDirty( true, NULL, 0 );
}

SwDDEFieldType* SwDDETable::GetDDEFldType()
{
    return (SwDDEFieldType*)aDepend.GetRegisteredIn();
}

sal_Bool SwDDETable::NoDDETable()
{
    // search table node
    OSL_ENSURE( GetFrmFmt(), "No FrameFormat" );
    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    // Is this the correct NodesArray? (because of UNDO)
    if( aLines.empty() )
        return sal_False;
    OSL_ENSURE( !GetTabSortBoxes().empty(), "Table without content?" );
    SwNode* pNd = (SwNode*)GetTabSortBoxes()[0]->GetSttNd();
    if( !pNd->GetNodes().IsDocNodes() )
        return sal_False;

    SwTableNode* pTblNd = pNd->FindTableNode();
    OSL_ENSURE( pTblNd, "Where is the table?");

    SwTable* pNewTbl = new SwTable( *this );

    // copy the table data
    pNewTbl->GetTabSortBoxes().insert( GetTabSortBoxes() ); // move content boxes
    GetTabSortBoxes().clear();

    pNewTbl->GetTabLines().insert( pNewTbl->GetTabLines().begin(),
                                   GetTabLines().begin(), GetTabLines().end() ); // move lines
    GetTabLines().clear();

    if( pDoc->GetCurrentViewShell() )   //swmod 071108//swmod 071225
        ((SwDDEFieldType*)aDepend.GetRegisteredIn())->DecRefCnt();

    pTblNd->SetNewTable( pNewTbl );       // replace table

    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
