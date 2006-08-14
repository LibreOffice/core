/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddetbl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:12:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _INDEX_HXX
#include <index.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>           // fuer den FieldType
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif


TYPEINIT1( SwDDETable, SwTable );

    // Constructor movet alle Lines/Boxen aus der SwTable zu sich.
    // Die SwTable ist danach Leer und muss geloescht werden.
SwDDETable::SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType,
                        BOOL bUpdate )
    : SwTable( rTable ), aDepend( this, pDDEType )
{
    // Kopiere/move die Daten der Tabelle
    aSortCntBoxes.Insert( &rTable.GetTabSortBoxes(), 0,
                          rTable.GetTabSortBoxes().Count()  ); // move die Inh. Boxen
    rTable.GetTabSortBoxes().Remove( (USHORT)0, rTable.GetTabSortBoxes().Count() );

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

    for( USHORT n = 0; n < aLines.Count(); ++n )
    {
        String aLine = aExpand.GetToken( n, '\n' );
        SwTableLine* pLine = aLines[ n ];
        for( USHORT i = 0; i < pLine->GetTabBoxes().Count(); ++i )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[ i ];
            ASSERT( pBox->GetSttIdx(), "keine InhaltsBox" );
            SwNodeIndex aNdIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pTxtNode = aNdIdx.GetNode().GetTxtNode();
            ASSERT( pTxtNode, "Kein Node" );
            SwIndex aCntIdx( pTxtNode, 0 );
            pTxtNode->Erase( aCntIdx );
            pTxtNode->Insert( aLine.GetToken( i, '\t' ), aCntIdx );

            SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
            pBoxFmt->LockModify();
            pBoxFmt->ResetAttr( RES_BOXATR_VALUE );
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

BOOL SwDDETable::NoDDETable()
{
    // suche den TabellenNode
    ASSERT( GetFrmFmt(), "Kein FrameFormat" );
    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    // Stehen wir im richtigen NodesArray (Wegen UNDO)
    if( !aLines.Count() )
        return FALSE;
    ASSERT( GetTabSortBoxes().Count(), "Tabelle ohne Inhalt?" );
    SwNode* pNd = (SwNode*)GetTabSortBoxes()[0]->GetSttNd();
    if( !pNd->GetNodes().IsDocNodes() )
        return FALSE;

    SwTableNode* pTblNd = pNd->FindTableNode();
    ASSERT( pTblNd, "wo steht denn die Tabelle ?");

    SwTable* pNewTbl = new SwTable( *this );

    // Kopiere/move die Daten der Tabelle
    pNewTbl->GetTabSortBoxes().Insert( &GetTabSortBoxes(), 0,
                          GetTabSortBoxes().Count()  ); // move die Inh. Boxen
    GetTabSortBoxes().Remove( (USHORT)0, GetTabSortBoxes().Count() );

    pNewTbl->GetTabLines().Insert( &GetTabLines(),0 );                      // move die Lines
    GetTabLines().Remove( 0, GetTabLines().Count() );

    if( pDoc->GetRootFrm() )
        ((SwDDEFieldType*)aDepend.GetRegisteredIn())->DecRefCnt();

    pTblNd->SetNewTable( pNewTbl );       // setze die Tabelle

    return TRUE;
}


