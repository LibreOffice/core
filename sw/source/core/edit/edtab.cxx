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

#include <fesh.hxx>
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
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
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
#include <itabenum.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//Added for bug #i119954# Application crashed if undo/redo covert nest table to text
bool ConvertTableToText( const SwTableNode *pTableNode, sal_Unicode cCh );

void    ConvertNestedTablesToText( const SwTableLines &rTableLines, sal_Unicode cCh )
{
    for (size_t n = 0; n < rTableLines.size(); ++n)
    {
        SwTableLine* pTableLine = rTableLines[ n ];
        for (size_t i = 0; i < pTableLine->GetTabBoxes().size(); ++i)
        {
            SwTableBox* pTableBox = pTableLine->GetTabBoxes()[ i ].get();
            if (pTableBox->GetTabLines().empty())
            {
                SwNodeIndex nodeIndex( *pTableBox->GetSttNd(), 1 );
                SwNodeIndex endNodeIndex( *pTableBox->GetSttNd()->EndOfSectionNode() );
                for( ; nodeIndex < endNodeIndex ; ++nodeIndex )
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

bool ConvertTableToText( const SwTableNode *pConstTableNode, sal_Unicode cCh )
{
    SwTableNode *pTableNode = const_cast< SwTableNode* >( pConstTableNode );
    ConvertNestedTablesToText( pTableNode->GetTable().GetTabLines(), cCh );
    return pTableNode->GetDoc()->TableToText( pTableNode, cCh );
}
//End for bug #i119954#

const SwTable& SwEditShell::InsertTable( const SwInsertTableOptions& rInsTableOpts,
                                         sal_uInt16 nRows, sal_uInt16 nCols,
                                         const SwTableAutoFormat* pTAFormat )
{
    StartAllAction();
    SwPosition* pPos = GetCursor()->GetPoint();

    bool bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( SwUndoId::START );
        GetDoc()->getIDocumentContentOperations().SplitNode( *pPos, false );
    }

    // If called from a shell the adjust item is propagated
    // from pPos to the new content nodes in the table.
    const SwTable *pTable = GetDoc()->InsertTable( rInsTableOpts, *pPos,
                                                   nRows, nCols,
                                                   css::text::HoriOrientation::FULL, pTAFormat,
                                                   nullptr, true );
    if( bEndUndo )
        EndUndo( SwUndoId::END );

    EndAllAction();
    return *pTable;
}

bool SwEditShell::TextToTable( const SwInsertTableOptions& rInsTableOpts,
                               sal_Unicode cCh,
                               const SwTableAutoFormat* pTAFormat )
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );
    bool bRet = false;
    StartAllAction();
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( rPaM.HasMark() )
            bRet |= nullptr != GetDoc()->TextToTable( rInsTableOpts, rPaM, cCh,
                                                css::text::HoriOrientation::FULL, pTAFormat );
    }
    EndAllAction();
    return bRet;
}

bool SwEditShell::TableToText( sal_Unicode cCh )
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );
    bool bRet = false;
    SwPaM* pCursor = GetCursor();
    const SwTableNode* pTableNd =
            GetDoc()->IsIdxInTable( pCursor->GetPoint()->nNode );
    if( IsTableMode() )
    {
        ClearMark();
        pCursor = GetCursor();
    }
    else if( !pTableNd || pCursor->GetNext() != pCursor )
        return bRet;

    // TL_CHART2:
    // tell the charts about the table to be deleted and have them use their own data
    GetDoc()->getIDocumentChartDataProviderAccess().CreateChartInternalDataProviders( &pTableNd->GetTable() );

    StartAllAction();

    // move current Cursor out of the listing area
    SwNodeIndex aTabIdx( *pTableNd );
    pCursor->DeleteMark();
    pCursor->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
    pCursor->GetPoint()->nContent.Assign( nullptr, 0 );
    // move sPoint and Mark out of the area!
    pCursor->SetMark();
    pCursor->DeleteMark();

    //Modified for bug #i119954# Application crashed if undo/redo covert nest table to text
    StartUndo();
    bRet = ConvertTableToText( pTableNd, cCh );
    EndUndo();
    //End  for bug #i119954#
    pCursor->GetPoint()->nNode = aTabIdx;

    SwContentNode* pCNd = pCursor->GetContentNode();
    if( !pCNd )
        pCursor->Move( fnMoveForward, GoInContent );
    else
        pCursor->GetPoint()->nContent.Assign( pCNd, 0 );

    EndAllAction();
    return bRet;
}

bool SwEditShell::IsTextToTableAvailable() const
{
    bool bOnlyText = false;
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( rPaM.HasMark() && *rPaM.GetPoint() != *rPaM.GetMark() )
        {
            bOnlyText = true;

            // check if selection is in listing
            sal_uLong nStt = rPaM.GetMark()->nNode.GetIndex(),
                  nEnd = rPaM.GetPoint()->nNode.GetIndex();
            if( nStt > nEnd )   { sal_uLong n = nStt; nStt = nEnd; nEnd = n; }

            for( ; nStt <= nEnd; ++nStt )
                if( !GetDoc()->GetNodes()[ nStt ]->IsTextNode() )
                {
                    bOnlyText = false;
                    break;
                }

            if( !bOnlyText )
                break;
        }
    }

    return bOnlyText;
}

void SwEditShell::InsertDDETable( const SwInsertTableOptions& rInsTableOpts,
                                  SwDDEFieldType* pDDEType,
                                  sal_uInt16 nRows, sal_uInt16 nCols )
{
    SwPosition* pPos = GetCursor()->GetPoint();

    StartAllAction();

    bool bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( SwUndoId::START );
        GetDoc()->getIDocumentContentOperations().SplitNode( *pPos, false );
    }

    const SwInsertTableOptions aInsTableOpts( rInsTableOpts.mnInsMode | SwInsertTableFlags::DefaultBorder,
                                            rInsTableOpts.mnRowsToRepeat );
    SwTable* pTable = const_cast<SwTable*>(GetDoc()->InsertTable( aInsTableOpts, *pPos,
                                                     nRows, nCols, css::text::HoriOrientation::FULL ));

    SwTableNode* pTableNode = const_cast<SwTableNode*>(pTable->GetTabSortBoxes()[ 0 ]->
                                                GetSttNd()->FindTableNode());
    std::unique_ptr<SwDDETable> pDDETable(new SwDDETable( *pTable, pDDEType ));
    pTableNode->SetNewTable( std::move(pDDETable) );   // set the DDE table

    if( bEndUndo )
        EndUndo( SwUndoId::END );

    EndAllAction();
}

/** update fields of a listing */
void SwEditShell::UpdateTable()
{
    const SwTableNode* pTableNd = IsCursorInTable();

    if( pTableNd )
    {
        StartAllAction();
        if( DoesUndo() )
            StartUndo();
        EndAllTableBoxEdit();
        SwTableFormulaUpdate aTableUpdate( &pTableNd->GetTable() );
        GetDoc()->getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );
        if( DoesUndo() )
            EndUndo();
        EndAllAction();
    }
}

// get/set Change Mode

TableChgMode SwEditShell::GetTableChgMode() const
{
    TableChgMode eMode;
    const SwTableNode* pTableNd = IsCursorInTable();
    if( pTableNd )
        eMode = pTableNd->GetTable().GetTableChgMode();
    else
        eMode = GetTableChgDefaultMode();
    return eMode;
}

void SwEditShell::SetTableChgMode( TableChgMode eMode )
{
    const SwTableNode* pTableNd = IsCursorInTable();

    if( pTableNd )
    {
        const_cast<SwTable&>(pTableNd->GetTable()).SetTableChgMode( eMode );
        if( !GetDoc()->getIDocumentState().IsModified() )   // Bug 57028
        {
            GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
        GetDoc()->getIDocumentState().SetModified();
    }
}

bool SwEditShell::GetTableBoxFormulaAttrs( SfxItemSet& rSet ) const
{
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTableSelCrs( *this, aBoxes );
    else
    {
        do {
            SwFrame *pFrame = GetCurrFrame();
            do {
                pFrame = pFrame->GetUpper();
            } while ( pFrame && !pFrame->IsCellFrame() );
            if ( pFrame )
            {
                SwTableBox *pBox = const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox());
                aBoxes.insert( pBox );
            }
        } while( false );
    }

    for (size_t n = 0; n < aBoxes.size(); ++n)
    {
        const SwTableBox* pSelBox = aBoxes[ n ];
        const SwTableBoxFormat* pTableFormat = static_cast<SwTableBoxFormat*>(pSelBox->GetFrameFormat());
        if( !n )
        {
            // Convert formulae into external presentation
            const SwTable& rTable = pSelBox->GetSttNd()->FindTableNode()->GetTable();

            SwTableFormulaUpdate aTableUpdate( &rTable );
            aTableUpdate.m_eFlags = TBL_BOXNAME;
            GetDoc()->getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );

            rSet.Put( pTableFormat->GetAttrSet() );
        }
        else
            rSet.MergeValues( pTableFormat->GetAttrSet() );
    }
    return 0 != rSet.Count();
}

void SwEditShell::SetTableBoxFormulaAttrs( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTableSelCrs( *this, aBoxes );
    else
    {
        do {
            SwFrame *pFrame = GetCurrFrame();
            do {
                pFrame = pFrame->GetUpper();
            } while ( pFrame && !pFrame->IsCellFrame() );
            if ( pFrame )
            {
                SwTableBox *pBox = const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox());
                aBoxes.insert( pBox );
            }
        } while( false );
    }

    // When setting a formula, do not check further!
    if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
        ClearTableBoxContent();

    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
    for (size_t n = 0; n < aBoxes.size(); ++n)
    {
        GetDoc()->SetTableBoxFormulaAttrs( *aBoxes[ n ], rSet );
    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    EndAllAction();
}

bool SwEditShell::IsTableBoxTextFormat() const
{
    if( IsTableMode() )
        return false;

    const SwTableBox *pBox = nullptr;
    {
        SwFrame *pFrame = GetCurrFrame();
        do {
            pFrame = pFrame->GetUpper();
        } while ( pFrame && !pFrame->IsCellFrame() );
        if ( pFrame )
            pBox = static_cast<SwCellFrame*>(pFrame)->GetTabBox();
    }

    if( !pBox )
        return false;

    sal_uInt32 nFormat = 0;
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == pBox->GetFrameFormat()->GetAttrSet().GetItemState(
        RES_BOXATR_FORMAT, true, &pItem ))
    {
        nFormat = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();
        return GetDoc()->GetNumberFormatter()->IsTextFormat( nFormat );
    }

    sal_uLong nNd = pBox->IsValidNumTextNd();
    if( ULONG_MAX == nNd )
        return true;

    const OUString& rText = GetDoc()->GetNodes()[ nNd ]->GetTextNode()->GetText();
    if( rText.isEmpty() )
        return false;

    double fVal;
    return !GetDoc()->IsNumberFormat( rText, nFormat, fVal );
}

OUString SwEditShell::GetTableBoxText() const
{
    OUString sRet;
    if( !IsTableMode() )
    {
        const SwTableBox *pBox = nullptr;
        {
            SwFrame *pFrame = GetCurrFrame();
            do {
                pFrame = pFrame->GetUpper();
            } while ( pFrame && !pFrame->IsCellFrame() );
            if ( pFrame )
                pBox = static_cast<SwCellFrame*>(pFrame)->GetTabBox();
        }

        sal_uLong nNd;
        if( pBox && ULONG_MAX != ( nNd = pBox->IsValidNumTextNd() ) )
            sRet = GetDoc()->GetNodes()[ nNd ]->GetTextNode()->GetText();
    }
    return sRet;
}

void SwEditShell::SplitTable( SplitTable_HeadlineOption eMode )
{
    SwPaM *pCursor = GetCursor();
    if( pCursor->GetNode().FindTableNode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

        GetDoc()->SplitTable( *pCursor->GetPoint(), eMode, true );

        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
        ClearFEShellTabCols(*GetDoc(), nullptr);
        EndAllAction();
    }
}

bool SwEditShell::MergeTable( bool bWithPrev )
{
    bool bRet = false;
    SwPaM *pCursor = GetCursor();
    if( pCursor->GetNode().FindTableNode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

        bRet = GetDoc()->MergeTable( *pCursor->GetPoint(), bWithPrev );

        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
        ClearFEShellTabCols(*GetDoc(), nullptr);
        EndAllAction();
    }
    return bRet;
}

bool SwEditShell::CanMergeTable( bool bWithPrev, bool* pChkNxtPrv ) const
{
    bool bRet = false;
    const SwPaM *pCursor = GetCursor();
    const SwTableNode* pTableNd = pCursor->GetNode().FindTableNode();
    if( pTableNd && dynamic_cast< const SwDDETable* >(&pTableNd->GetTable()) ==  nullptr)
    {
        bool bNew = pTableNd->GetTable().IsNewModel();
        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pChkNxtPrv )
        {
            const SwTableNode* pChkNd = rNds[ pTableNd->GetIndex() - 1 ]->FindTableNode();
            if( pChkNd && dynamic_cast< const SwDDETable* >(&pChkNd->GetTable()) ==  nullptr &&
                bNew == pChkNd->GetTable().IsNewModel() &&
                // Consider table in table case
                pChkNd->EndOfSectionIndex() == pTableNd->GetIndex() - 1 )
            {
                *pChkNxtPrv = true;
                bRet = true;        // using Prev is possible
            }
            else
            {
                pChkNd = rNds[ pTableNd->EndOfSectionIndex() + 1 ]->GetTableNode();
                if( pChkNd && dynamic_cast< const SwDDETable* >(&pChkNd->GetTable()) ==  nullptr &&
                    bNew == pChkNd->GetTable().IsNewModel() )
                {
                    *pChkNxtPrv = false;
                    bRet = true;   // using Next is possible
                }
            }
        }
        else
        {
            const SwTableNode* pTmpTableNd = nullptr;

            if( bWithPrev )
            {
                pTmpTableNd = rNds[ pTableNd->GetIndex() - 1 ]->FindTableNode();
                // Consider table in table case
                if ( pTmpTableNd && pTmpTableNd->EndOfSectionIndex() != pTableNd->GetIndex() - 1 )
                    pTmpTableNd = nullptr;
            }
            else
                pTmpTableNd = rNds[ pTableNd->EndOfSectionIndex() + 1 ]->GetTableNode();

            bRet = pTmpTableNd && dynamic_cast< const SwDDETable* >(&pTmpTableNd->GetTable()) ==  nullptr &&
                   bNew == pTmpTableNd->GetTable().IsNewModel();
        }
    }
    return bRet;
}

/** create InsertDB as table Undo */
void SwEditShell::AppendUndoForInsertFromDB( bool bIsTable )
{
    GetDoc()->AppendUndoForInsertFromDB( *GetCursor(), bIsTable );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
