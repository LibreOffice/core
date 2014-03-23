/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <hintids.hxx>
#include <hints.hxx>

#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <editeng/boxitem.hxx>
#include <swwait.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <tblsel.hxx>
#include <edimp.hxx>
#include <tabfrm.hxx>
#include <cellfrm.hxx>
#include <cellatr.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <mdiexp.hxx>
#include <unochart.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern void ClearFEShellTabCols();

//Added for bug 119954:Application crashed if undo/redo covert nest table to text
sal_Bool ConvertTableToText( const SwTableNode *pTableNode, sal_Unicode cCh );

void    ConvertNestedTablesToText( const SwTableLines &rTableLines, sal_Unicode cCh )
{
    for( sal_uInt16 n = 0; n < rTableLines.Count(); ++n )
    {
        SwTableLine* pTableLine = rTableLines[ n ];
        for( sal_uInt16 i = 0; i < pTableLine->GetTabBoxes().Count(); ++i )
        {
            SwTableBox* pTableBox = pTableLine->GetTabBoxes()[ i ];
            if ( !pTableBox->GetTabLines().Count() )
            {
                SwNodeIndex nodeIndex( *pTableBox->GetSttNd(), 1 );
                SwNodeIndex endNodeIndex( *pTableBox->GetSttNd()->EndOfSectionNode() );
                for( ; nodeIndex < endNodeIndex ; nodeIndex++ )
                {
                    if ( SwTableNode* pTableNode = nodeIndex.GetNode().GetTableNode() )
                        ConvertTableToText( pTableNode, cCh );
                }
            }
            else
            {
                ConvertNestedTablesToText( pTableBox->GetTabLines(), cCh );
            }
        }
    }
}

sal_Bool ConvertTableToText( const SwTableNode *pConstTableNode, sal_Unicode cCh )
{
    SwTableNode *pTableNode = const_cast< SwTableNode* >( pConstTableNode );
    ConvertNestedTablesToText( pTableNode->GetTable().GetTabLines(), cCh );
    return pTableNode->GetDoc()->TableToText( pTableNode, cCh );
}
//End for bug 119954
const SwTable& SwEditShell::InsertTable( const SwInsertTableOptions& rInsTblOpts,
                                         sal_uInt16 nRows, sal_uInt16 nCols,
                                         sal_Int16 eAdj,
                                         const SwTableAutoFmt* pTAFmt )
{
    StartAllAction();
    SwPosition* pPos = GetCrsr()->GetPoint();

    sal_Bool bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( UNDO_START );
        GetDoc()->SplitNode( *pPos, false );
    }

    /* #109161# If called from a shell the adjust item is propagated
        from pPos to the new content nodes in the table.
     */
    const SwTable *pTable = GetDoc()->InsertTable( rInsTblOpts, *pPos,
                                                   nRows, nCols,
                                                   eAdj, pTAFmt,
                                                   0, sal_True );
    if( bEndUndo )
        EndUndo( UNDO_END );

    EndAllAction();
    return *pTable;
}

sal_Bool SwEditShell::TextToTable( const SwInsertTableOptions& rInsTblOpts,
                               sal_Unicode cCh,
                               sal_Int16 eAdj,
                               const SwTableAutoFmt* pTAFmt )
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );
    sal_Bool bRet = sal_False;
    StartAllAction();
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() )
            bRet |= 0 != GetDoc()->TextToTable( rInsTblOpts, *PCURCRSR, cCh,
                                                eAdj, pTAFmt );
    FOREACHPAM_END()
    EndAllAction();
    return bRet;
}

sal_Bool SwEditShell::TableToText( sal_Unicode cCh )
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );
    sal_Bool bRet = sal_False;
    SwPaM* pCrsr = GetCrsr();
    const SwTableNode* pTblNd =
            GetDoc()->IsIdxInTbl( pCrsr->GetPoint()->nNode );
    if( IsTableMode() )
    {
        ClearMark();
        pCrsr = GetCrsr();
    }
    else if( !pTblNd || pCrsr->GetNext() != pCrsr )
        return bRet;

    // TL_CHART2:
    // tell the charts about the table to be deleted and have them use their own data
    GetDoc()->CreateChartInternalDataProviders( &pTblNd->GetTable() );

    StartAllAction();

    // verschiebe den akt. Cursor aus dem Tabellen Bereich
    // angemeldet ist
    SwNodeIndex aTabIdx( *pTblNd );
    pCrsr->DeleteMark();
    pCrsr->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    pCrsr->GetPoint()->nContent.Assign( 0, 0 );
    // SPoint und Mark aus dem Bereich verschieben !!!
    pCrsr->SetMark();
    pCrsr->DeleteMark();

    //Modified for bug 119954:Application crashed if undo/redo covert nest table to text
    StartUndo();//UNDO_START
    bRet = ConvertTableToText( pTblNd, cCh );
    EndUndo();//UNDO_END
    //End  for bug 119954
    pCrsr->GetPoint()->nNode = aTabIdx;

    SwCntntNode* pCNd = pCrsr->GetCntntNode();
    if( !pCNd )
        pCrsr->Move( fnMoveForward, fnGoCntnt );
    else
        pCrsr->GetPoint()->nContent.Assign( pCNd, 0 );

    EndAllAction();
    return bRet;
}

sal_Bool SwEditShell::IsTextToTableAvailable() const
{
    sal_Bool bOnlyText = sal_False;
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() && *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() )
        {
            bOnlyText = sal_True;

            // pruefe ob in der Selection eine Tabelle liegt
            sal_uLong nStt = PCURCRSR->GetMark()->nNode.GetIndex(),
                  nEnd = PCURCRSR->GetPoint()->nNode.GetIndex();
            if( nStt > nEnd )   { sal_uLong n = nStt; nStt = nEnd; nEnd = n; }

            for( ; nStt <= nEnd; ++nStt )
                if( !GetDoc()->GetNodes()[ nStt ]->IsTxtNode() )
                {
                    bOnlyText = sal_False;
                    break;
                }

            if( !bOnlyText )
                break;
        }
    FOREACHPAM_END()

    return bOnlyText;
}

void SwEditShell::InsertDDETable( const SwInsertTableOptions& rInsTblOpts,
                                  SwDDEFieldType* pDDEType,
                                  sal_uInt16 nRows, sal_uInt16 nCols,
                                  sal_Int16 eAdj )
{
    SwPosition* pPos = GetCrsr()->GetPoint();

    StartAllAction();

    sal_Bool bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( UNDO_START );
        GetDoc()->SplitNode( *pPos, false );
    }

    const SwInsertTableOptions aInsTblOpts( rInsTblOpts.mnInsMode | tabopts::DEFAULT_BORDER,
                                            rInsTblOpts.mnRowsToRepeat );
    SwTable* pTbl = (SwTable*)GetDoc()->InsertTable( aInsTblOpts, *pPos,
                                                     nRows, nCols, eAdj );

    SwTableNode* pTblNode = (SwTableNode*)pTbl->GetTabSortBoxes()[ 0 ]->
                                                GetSttNd()->FindTableNode();
    SwDDETable* pDDETbl = new SwDDETable( *pTbl, pDDEType );
    pTblNode->SetNewTable( pDDETbl );       // setze die DDE-Tabelle

    if( bEndUndo )
        EndUndo( UNDO_END );

    EndAllAction();
}

/*--------------------------------------------------------------------
    Beschreibung: Tabellenfelder einer Tabelle updaten
 --------------------------------------------------------------------*/
void SwEditShell::UpdateTable()
{
    const SwTableNode* pTblNd = IsCrsrInTbl();

    // Keine Arme keine Kekse
    if( pTblNd )
    {
        StartAllAction();
        if( DoesUndo() )
            StartUndo();
        EndAllTblBoxEdit();
        SwTableFmlUpdate aTblUpdate( (SwTable*)&pTblNd->GetTable() );
        GetDoc()->UpdateTblFlds( &aTblUpdate );
        if( DoesUndo() )
            EndUndo();
        EndAllAction();
    }
}

    // Change Modus erfragen/setzen
TblChgMode SwEditShell::GetTblChgMode() const
{
    TblChgMode eMode;
    const SwTableNode* pTblNd = IsCrsrInTbl();
    if( pTblNd )
        eMode = pTblNd->GetTable().GetTblChgMode();
    else
        eMode = GetTblChgDefaultMode();
    return eMode;
}

void SwEditShell::SetTblChgMode( TblChgMode eMode )
{
    const SwTableNode* pTblNd = IsCrsrInTbl();

    // Keine Arme keine Kekse
    if( pTblNd )
    {
        ((SwTable&)pTblNd->GetTable()).SetTblChgMode( eMode );
        if( !GetDoc()->IsModified() )   // Bug 57028
        {
            GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
        GetDoc()->SetModified();
    }
}

sal_Bool SwEditShell::GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const
{
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        do {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if ( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.Insert( pBox );
            }
        } while( sal_False );
    }

    for( sal_uInt16 n = 0; n < aBoxes.Count(); ++n )
    {
        const SwTableBox* pSelBox = aBoxes[ n ];
        const SwTableBoxFmt* pTblFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        if( !n )
        {
            // Formeln in die externe Darstellung bringen!
            const SwTable& rTbl = pSelBox->GetSttNd()->FindTableNode()->GetTable();

            SwTableFmlUpdate aTblUpdate( (SwTable*)&rTbl );
            aTblUpdate.eFlags = TBL_BOXNAME;
            ((SwDoc*)GetDoc())->UpdateTblFlds( &aTblUpdate );

            rSet.Put( pTblFmt->GetAttrSet() );
        }
        else
            rSet.MergeValues( pTblFmt->GetAttrSet() );
    }
    return 0 != rSet.Count();
}

void SwEditShell::SetTblBoxFormulaAttrs( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        do {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if ( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.Insert( pBox );
            }
        } while( sal_False );
    }

    // beim setzen einer Formel keine Ueberpruefung mehr vornehmen!
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
        ClearTblBoxCntnt();

    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    for( sal_uInt16 n = 0; n < aBoxes.Count(); ++n )
        GetDoc()->SetTblBoxFormulaAttrs( *aBoxes[ n ], rSet );
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    EndAllAction();
}

sal_Bool SwEditShell::IsTableBoxTextFormat() const
{
    if( IsTableMode() )
        return sal_False;

    SwTableBox *pBox = 0;
    {
        SwFrm *pFrm = GetCurrFrm();
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );
        if ( pFrm )
            pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
    }

    if( !pBox )
        return sal_False;

    sal_uInt32 nFmt;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet().GetItemState(
        RES_BOXATR_FORMAT, sal_True, &pItem ))
    {
        nFmt = ((SwTblBoxNumFormat*)pItem)->GetValue();
        return GetDoc()->GetNumberFormatter()->IsTextFormat( nFmt ) ||
                NUMBERFORMAT_TEXT == nFmt;
    }

    sal_uLong nNd = pBox->IsValidNumTxtNd();
    if( ULONG_MAX == nNd )
        return sal_True;

    const String& rTxt = GetDoc()->GetNodes()[ nNd ]->GetTxtNode()->GetTxt();
    if( !rTxt.Len() )
        return sal_False;

    double fVal;
    return !GetDoc()->GetNumberFormatter()->IsNumberFormat( rTxt, nFmt, fVal );
}

String SwEditShell::GetTableBoxText() const
{
    String sRet;
    if( !IsTableMode() )
    {
        SwTableBox *pBox = 0;
        {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if ( pFrm )
                pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
        }

        sal_uLong nNd;
        if( pBox && ULONG_MAX != ( nNd = pBox->IsValidNumTxtNd() ) )
            sRet = GetDoc()->GetNodes()[ nNd ]->GetTxtNode()->GetTxt();
    }
    return sRet;
}

sal_Bool SwEditShell::SplitTable( sal_uInt16 eMode )
{
    sal_Bool bRet = sal_False;
    SwPaM *pCrsr = GetCrsr();
    if( pCrsr->GetNode()->FindTableNode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

        bRet = GetDoc()->SplitTable( *pCrsr->GetPoint(), eMode, sal_True );

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
        ClearFEShellTabCols();
        EndAllAction();
    }
    return bRet;
}

sal_Bool SwEditShell::MergeTable( sal_Bool bWithPrev, sal_uInt16 nMode )
{
    sal_Bool bRet = sal_False;
    SwPaM *pCrsr = GetCrsr();
    if( pCrsr->GetNode()->FindTableNode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

        bRet = GetDoc()->MergeTable( *pCrsr->GetPoint(), bWithPrev, nMode );

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
        ClearFEShellTabCols();
        EndAllAction();
    }
    return bRet;
}

sal_Bool SwEditShell::CanMergeTable( sal_Bool bWithPrev, sal_Bool* pChkNxtPrv ) const
{
    sal_Bool bRet = sal_False;
    const SwPaM *pCrsr = GetCrsr();
    const SwTableNode* pTblNd = pCrsr->GetNode()->FindTableNode();
    if( pTblNd && !dynamic_cast< const SwDDETable* >(&pTblNd->GetTable()))
    {
        sal_Bool bNew = pTblNd->GetTable().IsNewModel();
        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pChkNxtPrv )
        {
            const SwTableNode* pChkNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
            if( pChkNd && !dynamic_cast< const SwDDETable* >(&pChkNd->GetTable()) &&
                bNew == pChkNd->GetTable().IsNewModel() &&
                // --> FME 2004-09-17 #117418# Consider table in table case
                pChkNd->EndOfSectionIndex() == pTblNd->GetIndex() - 1 )
                // <--
                *pChkNxtPrv = sal_True, bRet = sal_True;        // mit Prev ist moeglich
            else
            {
                pChkNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();
                if( pChkNd && !dynamic_cast< const SwDDETable* >(&pChkNd->GetTable()) &&
                    bNew == pChkNd->GetTable().IsNewModel() )
                    *pChkNxtPrv = sal_False, bRet = sal_True;       // mit Next ist moeglich
            }
        }
        else
        {
            const SwTableNode* pTmpTblNd = 0;

            if( bWithPrev )
            {
                pTmpTblNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
                // --> FME 2004-09-17 #117418# Consider table in table case
                if ( pTmpTblNd && pTmpTblNd->EndOfSectionIndex() != pTblNd->GetIndex() - 1 )
                    pTmpTblNd = 0;
                // <--
            }
            else
                pTmpTblNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();

            bRet = pTmpTblNd && !dynamic_cast< const SwDDETable* >(&pTmpTblNd->GetTable()) &&
                   bNew == pTmpTblNd->GetTable().IsNewModel();
        }
    }
    return bRet;
}

        // setze das InsertDB als Tabelle Undo auf:
void SwEditShell::AppendUndoForInsertFromDB( sal_Bool bIsTable )
{
    GetDoc()->AppendUndoForInsertFromDB( *GetCrsr(), bIsTable );
}

