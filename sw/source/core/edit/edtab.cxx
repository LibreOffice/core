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
#include <hintids.hxx>
#include <hints.hxx>


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

    /* If called from a shell the adjust item is propagated
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
    SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
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
    SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
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

    bRet = GetDoc()->TableToText( pTblNd, cCh );
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
                aBoxes.insert( pBox );
            }
        } while( sal_False );
    }

    for (size_t n = 0; n < aBoxes.size(); ++n)
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
                aBoxes.insert( pBox );
            }
        } while( sal_False );
    }

    // beim setzen einer Formel keine Ueberpruefung mehr vornehmen!
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
        ClearTblBoxCntnt();

    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    for (size_t n = 0; n < aBoxes.size(); ++n)
    {
        GetDoc()->SetTblBoxFormulaAttrs( *aBoxes[ n ], rSet );
    }
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
    if( pTblNd && !pTblNd->GetTable().ISA( SwDDETable ))
    {
        sal_Bool bNew = pTblNd->GetTable().IsNewModel();
        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pChkNxtPrv )
        {
            const SwTableNode* pChkNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
            if( pChkNd && !pChkNd->GetTable().ISA( SwDDETable ) &&
                bNew == pChkNd->GetTable().IsNewModel() &&
                // Consider table in table case
                pChkNd->EndOfSectionIndex() == pTblNd->GetIndex() - 1 )
                *pChkNxtPrv = sal_True, bRet = sal_True;        // mit Prev ist moeglich
            else
            {
                pChkNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();
                if( pChkNd && !pChkNd->GetTable().ISA( SwDDETable ) &&
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
                // Consider table in table case
                if ( pTmpTblNd && pTmpTblNd->EndOfSectionIndex() != pTblNd->GetIndex() - 1 )
                    pTmpTblNd = 0;
            }
            else
                pTmpTblNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();

            bRet = pTmpTblNd && !pTmpTblNd->GetTable().ISA( SwDDETable ) &&
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
