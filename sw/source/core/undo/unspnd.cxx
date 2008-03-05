/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unspnd.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:10:45 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "doc.hxx"
#include "pam.hxx"
#include "swtable.hxx"
#include "ndtxt.hxx"
#include "swundo.hxx"           // fuer die UndoIds

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#include "undobj.hxx"
#include "rolbck.hxx"
#include "redline.hxx"
#include "docary.hxx"


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


//------------------------------------------------------------------

// SPLITNODE


SwUndoSplitNode::SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos,
                                    BOOL bChkTable )
    : SwUndo( UNDO_SPLITNODE ), pHistory( 0 ), pRedlData( 0 ), nNode( rPos.nNode.GetIndex() ),
        nCntnt( rPos.nContent.GetIndex() ),
        bTblFlag( FALSE ), bChkTblStt( bChkTable )
{
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ rPos.nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "nur beim TextNode rufen!" );
    if( pTxtNd->GetpSwpHints() )
    {
        pHistory = new SwHistory;
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nNode, 0,
                            pTxtNd->GetTxt().Len(), FALSE );
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



void SwUndoSplitNode::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();
    SwPaM& rPam = *rUndoIter.pAktPam;
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
                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, FALSE,
                    &pItem ) )
                    pTableFmt->SetAttr( *pItem );

                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, FALSE,
                     &pItem ) )
                    pTableFmt->SetAttr( *pItem );
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

                pDoc->RstTxtAttrs( rPam, TRUE );
                pHistory->TmpRollback( pDoc, 0, FALSE );
            }
        }
    }

    // setze noch den Cursor auf den Undo-Bereich
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nCntnt );
}


void SwUndoSplitNode::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_SPLITNODE == rUndoIter.GetLastUndoId() )
        return;
    rUndoIter.GetDoc().SplitNode( *rUndoIter.pAktPam->GetPoint(), bChkTblStt );
    rUndoIter.pLastUndoObj = this;
}


void SwUndoSplitNode::Redo( SwUndoIter& rUndoIter )
{
    SwPaM& rPam = *rUndoIter.pAktPam;
    ULONG nOldNode = rPam.GetPoint()->nNode.GetIndex();
    rPam.GetPoint()->nNode = nNode;
    SwTxtNode * pTNd = rPam.GetNode()->GetTxtNode();
    if( pTNd )              // sollte eigentlich immer ein TextNode sein !!
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
    else
        rPam.GetPoint()->nNode = nOldNode;
}

