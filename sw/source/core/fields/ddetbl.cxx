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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <frmfmt.hxx>
#include <doc.hxx>
#include <index.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swddetbl.hxx>
#include <ddefld.hxx>           // fuer den FieldType
#include <ndindex.hxx>
#include <fldupde.hxx>
#include <swtblfmt.hxx>


TYPEINIT1( SwDDETable, SwTable );

    // Constructor movet alle Lines/Boxen aus der SwTable zu sich.
    // Die SwTable ist danach Leer und muss geloescht werden.
SwDDETable::SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType,
                        sal_Bool bUpdate )
    : SwTable( rTable ), aDepend( this, pDDEType )
{
    // Kopiere/move die Daten der Tabelle
    aSortCntBoxes.Insert( &rTable.GetTabSortBoxes(), 0,
                          rTable.GetTabSortBoxes().Count()  ); // move die Inh. Boxen
    rTable.GetTabSortBoxes().Remove( (sal_uInt16)0, rTable.GetTabSortBoxes().Count() );

    aLines.Insert( &rTable.GetTabLines(),0 );                       // move die Lines
    rTable.GetTabLines().Remove( 0, rTable.GetTabLines().Count() );

    if( aLines.Count() )
    {
        const SwNode& rNd = *GetTabSortBoxes()[0]->GetSttNd();
        if( rNd.GetNodes().IsDocNodes() )
        {
            // "aktivieren der Updates" (Modify nicht noch mal rufen)
            aDepend.LockModify();
            pDDEType->IncRefCnt();
            aDepend.UnlockModify();

            // Setzen der Werte in die einzelnen Boxen
            // update box content only if update flag is set (false in import)
            if (bUpdate)
                ChangeContent();
        }
    }
}

__EXPORT SwDDETable::~SwDDETable()
{
    SwDDEFieldType* pFldTyp = (SwDDEFieldType*)aDepend.GetRegisteredIn();
    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    if( !pDoc->IsInDtor() && aLines.Count() &&
        GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        pFldTyp->DecRefCnt();

    // sind wir der letzte Abhaengige vom "geloeschten Feld" dann loesche dieses
    if( pFldTyp->IsDeleted() && pFldTyp->IsLastDepend() )
    {
        pFldTyp->Remove( &aDepend );
        delete pFldTyp;
    }
}

void SwDDETable::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( pNew && RES_UPDATEDDETBL == pNew->Which() )
        ChangeContent();
    else
        SwTable::Modify( pOld, pNew );
}

void SwDDETable::ChangeContent()
{
    ASSERT( GetFrmFmt(), "Kein FrameFormat" );

    // Stehen wir im richtigen NodesArray (Wegen UNDO)
    if( !aLines.Count() )
        return;
    ASSERT( GetTabSortBoxes().Count(), "Tabelle ohne Inhalt?" );
    if( !GetTabSortBoxes()[0]->GetSttNd()->GetNodes().IsDocNodes() )
        return;

    // zugriff auf den DDEFldType
    SwDDEFieldType* pDDEType = (SwDDEFieldType*)aDepend.GetRegisteredIn();

    String aExpand = pDDEType->GetExpansion();
    aExpand.EraseAllChars( '\r' );

    for( sal_uInt16 n = 0; n < aLines.Count(); ++n )
    {
        String aLine = aExpand.GetToken( n, '\n' );
        SwTableLine* pLine = aLines[ n ];
        for( sal_uInt16 i = 0; i < pLine->GetTabBoxes().Count(); ++i )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[ i ];
            ASSERT( pBox->GetSttIdx(), "keine InhaltsBox" );
            SwNodeIndex aNdIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pTxtNode = aNdIdx.GetNode().GetTxtNode();
            ASSERT( pTxtNode, "Kein Node" );
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
    // suche den TabellenNode
    ASSERT( GetFrmFmt(), "Kein FrameFormat" );
    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    // Stehen wir im richtigen NodesArray (Wegen UNDO)
    if( !aLines.Count() )
        return sal_False;
    ASSERT( GetTabSortBoxes().Count(), "Tabelle ohne Inhalt?" );
    SwNode* pNd = (SwNode*)GetTabSortBoxes()[0]->GetSttNd();
    if( !pNd->GetNodes().IsDocNodes() )
        return sal_False;

    SwTableNode* pTblNd = pNd->FindTableNode();
    ASSERT( pTblNd, "wo steht denn die Tabelle ?");

    SwTable* pNewTbl = new SwTable( *this );

    // Kopiere/move die Daten der Tabelle
    pNewTbl->GetTabSortBoxes().Insert( &GetTabSortBoxes(), 0,
                          GetTabSortBoxes().Count()  ); // move die Inh. Boxen
    GetTabSortBoxes().Remove( (sal_uInt16)0, GetTabSortBoxes().Count() );

    pNewTbl->GetTabLines().Insert( &GetTabLines(),0 );                      // move die Lines
    GetTabLines().Remove( 0, GetTabLines().Count() );

    if( pDoc->GetRootFrm() )
        ((SwDDEFieldType*)aDepend.GetRegisteredIn())->DecRefCnt();

    pTblNd->SetNewTable( pNewTbl );       // setze die Tabelle

    return sal_True;
}


