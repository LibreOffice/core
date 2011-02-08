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

#include <UndoSplitMove.hxx>

#include "doc.hxx"
#include "pam.hxx"
#include "swtable.hxx"
#include "ndtxt.hxx"
#include "swundo.hxx"           // fuer die UndoIds
#include <editeng/brkitem.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <UndoCore.hxx>
#include "rolbck.hxx"
#include "redline.hxx"
#include "docary.hxx"
#include <IShellCursorSupplier.hxx>


//------------------------------------------------------------------

// SPLITNODE


SwUndoSplitNode::SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos,
                                    sal_Bool bChkTable )
    : SwUndo( UNDO_SPLITNODE ), pHistory( 0 ), pRedlData( 0 ), nNode( rPos.nNode.GetIndex() ),
        nCntnt( rPos.nContent.GetIndex() ),
        bTblFlag( sal_False ), bChkTblStt( bChkTable )
{
    SwTxtNode *const pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    OSL_ENSURE(pTxtNd, "SwUndoSplitNode: TextNode expected!");
    if( pTxtNd->GetpSwpHints() )
    {
        pHistory = new SwHistory;
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nNode, 0,
                            pTxtNd->GetTxt().Len(), false );
        if( !pHistory->Count() )
            DELETEZ( pHistory );
    }
    // Redline beachten
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

SwUndoSplitNode::~SwUndoSplitNode()
{
    delete pHistory;
    delete pRedlData;
}

void SwUndoSplitNode::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.DeleteMark();
    if( bTblFlag )
    {
        // dann wurde direkt vor der akt. Tabelle ein TextNode eingefuegt.
        SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        rIdx = nNode;
        SwTxtNode* pTNd;
        SwNode* pCurrNd = pDoc->GetNodes()[ nNode + 1 ];
        SwTableNode* pTblNd = pCurrNd->FindTableNode();
        if( pCurrNd->IsCntntNode() && pTblNd &&
            0 != ( pTNd = pDoc->GetNodes()[ pTblNd->GetIndex()-1 ]->GetTxtNode() ))
        {
            // verschiebe die BreakAttribute noch
            SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
            const SfxItemSet* pNdSet = pTNd->GetpSwAttrSet();
            if( pNdSet )
            {
                const SfxPoolItem *pItem;
                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, sal_False,
                    &pItem ) )
                    pTableFmt->SetFmtAttr( *pItem );

                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, sal_False,
                     &pItem ) )
                    pTableFmt->SetFmtAttr( *pItem );
            }

            // dann loesche den wieder
            SwNodeIndex aDelNd( *pTblNd, -1 );
            rPam.GetPoint()->nContent.Assign( (SwCntntNode*)pCurrNd, 0 );
            RemoveIdxRel( aDelNd.GetIndex(), *rPam.GetPoint() );
            pDoc->GetNodes().Delete( aDelNd );
        }
    }
    else
    {
        SwTxtNode * pTNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
        if( pTNd )
        {
            rPam.GetPoint()->nNode = *pTNd;
            rPam.GetPoint()->nContent.Assign( pTNd, pTNd->GetTxt().Len() );

            if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
            {
                rPam.SetMark();
                rPam.GetMark()->nNode++;
                rPam.GetMark()->nContent.Assign( rPam.GetMark()->
                                    nNode.GetNode().GetCntntNode(), 0 );
                pDoc->DeleteRedline( rPam, true, USHRT_MAX );
                rPam.DeleteMark();
            }

            RemoveIdxRel( nNode+1, *rPam.GetPoint() );

            pTNd->JoinNext();
            if( pHistory )
            {
                rPam.GetPoint()->nContent = 0;
                rPam.SetMark();
                rPam.GetPoint()->nContent = pTNd->GetTxt().Len();

                pDoc->RstTxtAttrs( rPam, sal_True );
                pHistory->TmpRollback( pDoc, 0, false );
            }
        }
    }

    // setze noch den Cursor auf den Undo-Bereich
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nCntnt );
}

void SwUndoSplitNode::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.GetPoint()->nNode = nNode;
    SwTxtNode * pTNd = rPam.GetNode()->GetTxtNode();
    OSL_ENSURE(pTNd, "SwUndoSplitNode::RedoImpl(): SwTxtNode expected");
    if (pTNd)
    {
        rPam.GetPoint()->nContent.Assign( pTNd, nCntnt );

        SwDoc* pDoc = rPam.GetDoc();
        pDoc->SplitNode( *rPam.GetPoint(), bChkTblStt );

        if( pHistory )
            pHistory->SetTmpEnd( pHistory->Count() );

        if( ( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() )) ||
            ( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                pDoc->GetRedlineTbl().Count() ))
        {
            rPam.SetMark();
            if( rPam.Move( fnMoveBackward ))
            {
                if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
                {
                    RedlineMode_t eOld = pDoc->GetRedlineMode();
                    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
                    pDoc->AppendRedline( new SwRedline( *pRedlData, rPam ), true);
                    pDoc->SetRedlineMode_intern( eOld );
                }
                else
                    pDoc->SplitRedline( rPam );
                rPam.Exchange();
            }
            rPam.DeleteMark();
        }
    }
}

void SwUndoSplitNode::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().SplitNode(
        *rContext.GetRepeatPaM().GetPoint(), bChkTblStt );
}

