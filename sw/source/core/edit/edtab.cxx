/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


sal_Bool ConvertTableToText( const SwTableNode *pTableNode, sal_Unicode cCh );

void    ConvertNestedTablesToText( const SwTableLines &rTableLines, sal_Unicode cCh )
{
    for (size_t n = 0; n < rTableLines.size(); ++n)
    {
        SwTableLine* pTableLine = rTableLines[ n ];
        for (size_t i = 0; i < pTableLine->GetTabBoxes().size(); ++i)
        {
            SwTableBox* pTableBox = pTableLine->GetTabBoxes()[ i ];
            if (pTableBox->GetTabLines().empty())
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


const SwTable& SwEditShell::InsertTable( const SwInsertTableOptions& rInsTblOpts,
                                         sal_uInt16 nRows, sal_uInt16 nCols,
                                         sal_Int16 eAdj,
                                         const SwTableAutoFmt* pTAFmt )
{
    StartAllAction();
    SwPosition* pPos = GetCrsr()->GetPoint();

    bool bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( UNDO_START );
        GetDoc()->SplitNode( *pPos, false );
    }

    
    
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
    bool bRet = false;
    StartAllAction();
    FOREACHPAM_START(GetCrsr())
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

    
    
    GetDoc()->CreateChartInternalDataProviders( &pTblNd->GetTable() );

    StartAllAction();

    
    SwNodeIndex aTabIdx( *pTblNd );
    pCrsr->DeleteMark();
    pCrsr->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    pCrsr->GetPoint()->nContent.Assign( 0, 0 );
    
    pCrsr->SetMark();
    pCrsr->DeleteMark();

    
    StartUndo();
    bRet = ConvertTableToText( pTblNd, cCh );
    EndUndo();
    
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
    FOREACHPAM_START(GetCrsr())
        if( PCURCRSR->HasMark() && *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() )
        {
            bOnlyText = sal_True;

            
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

    bool bEndUndo = 0 != pPos->nContent.GetIndex();
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
    pTblNode->SetNewTable( pDDETbl );       

    if( bEndUndo )
        EndUndo( UNDO_END );

    EndAllAction();
}

/** update fields of a listing */
void SwEditShell::UpdateTable()
{
    const SwTableNode* pTblNd = IsCrsrInTbl();

    
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

    if( pTblNd )
    {
        ((SwTable&)pTblNd->GetTable()).SetTblChgMode( eMode );
        if( !GetDoc()->IsModified() )   
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
        } while( false );
    }

    for (size_t n = 0; n < aBoxes.size(); ++n)
    {
        const SwTableBox* pSelBox = aBoxes[ n ];
        const SwTableBoxFmt* pTblFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        if( !n )
        {
            
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
        } while( false );
    }

    
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

    sal_uInt32 nFmt = 0;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet().GetItemState(
        RES_BOXATR_FORMAT, true, &pItem ))
    {
        nFmt = ((SwTblBoxNumFormat*)pItem)->GetValue();
        return GetDoc()->GetNumberFormatter()->IsTextFormat( nFmt ) ||
                NUMBERFORMAT_TEXT == nFmt;
    }

    sal_uLong nNd = pBox->IsValidNumTxtNd();
    if( ULONG_MAX == nNd )
        return sal_True;

    const OUString& rTxt = GetDoc()->GetNodes()[ nNd ]->GetTxtNode()->GetTxt();
    if( rTxt.isEmpty() )
        return sal_False;

    double fVal;
    return !GetDoc()->GetNumberFormatter()->IsNumberFormat( rTxt, nFmt, fVal );
}

OUString SwEditShell::GetTableBoxText() const
{
    OUString sRet;
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
                
                pChkNd->EndOfSectionIndex() == pTblNd->GetIndex() - 1 )
                *pChkNxtPrv = sal_True, bRet = sal_True;        
            else
            {
                pChkNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();
                if( pChkNd && !pChkNd->GetTable().ISA( SwDDETable ) &&
                    bNew == pChkNd->GetTable().IsNewModel() )
                    *pChkNxtPrv = sal_False, bRet = sal_True;   
            }
        }
        else
        {
            const SwTableNode* pTmpTblNd = 0;

            if( bWithPrev )
            {
                pTmpTblNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
                
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

/** create InsertDB as table Undo */
void SwEditShell::AppendUndoForInsertFromDB( sal_Bool bIsTable )
{
    GetDoc()->AppendUndoForInsertFromDB( *GetCrsr(), bIsTable );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
