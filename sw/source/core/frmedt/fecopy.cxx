/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <hintids.hxx>

#include <vcl/graph.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/xexch.hxx>
#include <svx/xflasit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <editeng/brshitem.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdouno.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/fmmodel.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx> // #i50824#
#include <svx/svdograf.hxx>
#include <unotools/streamwrap.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <frmfmt.hxx>
#include <docary.hxx>
#include <txtfrm.hxx>
#include <txtflcnt.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <tblsel.hxx>
#include <swtable.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <fldbas.hxx>
#include <edimp.hxx>
#include <swundo.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <docsh.hxx>
#include <pagedesc.hxx>
#include <mvsave.hxx>
#include <vcl/virdev.hxx>


using namespace ::com::sun::star;

/*************************************************************************
|*
|*  SwFEShell::Copy()   copy for the internal clipboard.
|*      Copies all selections to the clipboard.
|*
|*************************************************************************/

sal_Bool SwFEShell::Copy( SwDoc* pClpDoc, const String* pNewClpTxt )
{
    OSL_ENSURE( pClpDoc, "kein Clipboard-Dokument"  );

    pClpDoc->GetIDocumentUndoRedo().DoUndo(false); // always false!

    // delete content if ClpDocument contains content
    SwNodeIndex aSttIdx( pClpDoc->GetNodes().GetEndOfExtras(), 2 );
    SwTxtNode* pTxtNd = aSttIdx.GetNode().GetTxtNode();
    if( !pTxtNd || pTxtNd->GetTxt().Len() ||
        aSttIdx.GetIndex()+1 != pClpDoc->GetNodes().GetEndOfContent().GetIndex() )
    {
        pClpDoc->GetNodes().Delete( aSttIdx,
            pClpDoc->GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );
        pTxtNd = pClpDoc->GetNodes().MakeTxtNode( aSttIdx,
                            (SwTxtFmtColl*)pClpDoc->GetDfltTxtFmtColl() );
        aSttIdx--;
    }

    // also delete surrounding FlyFrames if any
    for( sal_uInt16 n = 0; n < pClpDoc->GetSpzFrmFmts()->size(); ++n )
    {
        SwFlyFrmFmt* pFly = (SwFlyFrmFmt*)(*pClpDoc->GetSpzFrmFmts())[n];
        pClpDoc->DelLayoutFmt( pFly );
    }
    pClpDoc->GCFieldTypes();        // delete the FieldTypes

    // if a string was passed, copy it to the clipboard-
    // document. Then also the Calculator can use the internal
    // clipboard
    if( pNewClpTxt )
    {
        pTxtNd->InsertText( *pNewClpTxt, SwIndex( pTxtNd ) );
        return sal_True;                // das wars.
    }

    pClpDoc->LockExpFlds();
    pClpDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_DELETE_REDLINES );
    sal_Bool bRet;

    // do we want to copy a FlyFrame?
    if( IsFrmSelected() )
    {
        // get the FlyFormat
        SwFlyFrm* pFly = FindFlyFrm();
        SwFrmFmt* pFlyFmt = pFly->GetFmt();
        SwFmtAnchor aAnchor( pFlyFmt->GetAnchor() );

        if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
            (FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
            (FLY_AS_CHAR == aAnchor.GetAnchorId()))
        {
            SwPosition aPos( aSttIdx );
            if ( FLY_AS_CHAR == aAnchor.GetAnchorId() )
            {
                aPos.nContent.Assign( pTxtNd, 0 );
            }
            aAnchor.SetAnchor( &aPos );
        }
        pFlyFmt = pClpDoc->CopyLayoutFmt( *pFlyFmt, aAnchor, true, true );

       // assure the "RootFmt" is the first element in Spz-Array
        // (if necessary Flys were copied in Flys)
        SwFrmFmts& rSpzFrmFmts = *(SwFrmFmts*)pClpDoc->GetSpzFrmFmts();
        if( rSpzFrmFmts[ 0 ] != pFlyFmt )
        {
            SwFrmFmts::iterator it = std::find( rSpzFrmFmts.begin(), rSpzFrmFmts.end(), pFlyFmt );
            OSL_ENSURE( it != rSpzFrmFmts.end(), "Fly not contained in Spz-Array" );

            rSpzFrmFmts.erase( it );
            rSpzFrmFmts.insert( rSpzFrmFmts.begin(), pFlyFmt );
        }

        if ( FLY_AS_CHAR == aAnchor.GetAnchorId() )
        {
            // JP 13.02.99  Bug 61863: if a frameselection is passed to the
            //              clipboard, it should be found at pasting. Therefore
            //              the copied TextAttribut should be removed in the node
            //              otherwise it will be recognised as TextSelektion
            const SwIndex& rIdx = pFlyFmt->GetAnchor().GetCntntAnchor()->nContent;
            SwTxtFlyCnt *const pTxtFly = static_cast<SwTxtFlyCnt *>(
                pTxtNd->GetTxtAttrForCharAt(
                    rIdx.GetIndex(), RES_TXTATR_FLYCNT));
            if( pTxtFly )
            {
                ((SwFmtFlyCnt&)pTxtFly->GetFlyCnt()).SetFlyFmt( 0 );
                pTxtNd->EraseText( rIdx, 1 );
            }
        }
        bRet = sal_True;
    }
    else if ( IsObjSelected() )
    {
        SwPosition aPos( aSttIdx, SwIndex( pTxtNd, 0 ));
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();

            if( Imp()->GetDrawView()->IsGroupEntered() ||
                ( !pObj->GetUserCall() && pObj->GetUpGroup()) )
            {
                SfxItemSet aSet( pClpDoc->GetAttrPool(), aFrmFmtSetRange );

                SwFmtAnchor aAnchor( FLY_AT_PARA );
                aAnchor.SetAnchor( &aPos );
                aSet.Put( aAnchor );

                SdrObject *const pNew =
                    pClpDoc->CloneSdrObj( *pObj, false, true );

                SwPaM aTemp(aPos);
                pClpDoc->Insert(aTemp, *pNew, &aSet, NULL);
            }
            else
            {
                SwDrawContact *pContact = (SwDrawContact*)GetUserCall( pObj );
                SwFrmFmt *pFmt = pContact->GetFmt();
                SwFmtAnchor aAnchor( pFmt->GetAnchor() );
                if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                    (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                    (FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
                    (FLY_AS_CHAR == aAnchor.GetAnchorId()))
                {
                    aAnchor.SetAnchor( &aPos );
                }

                pClpDoc->CopyLayoutFmt( *pFmt, aAnchor, true, true );
            }
        }
        bRet = sal_True;
    }
    else
        bRet = _CopySelToDoc( pClpDoc, 0 );     // copy the selections

    pClpDoc->SetRedlineMode_intern((RedlineMode_t)0 );
    pClpDoc->UnlockExpFlds();
    if( !pClpDoc->IsExpFldsLocked() )
        pClpDoc->UpdateExpFlds(NULL, true);

    return bRet;
}

static const Point &lcl_FindBasePos( const SwFrm *pFrm, const Point &rPt )
{
    const SwFrm *pF = pFrm;
    while ( pF && !pF->Frm().IsInside( rPt ) )
    {
        if ( pF->IsCntntFrm() )
            pF = ((SwCntntFrm*)pF)->GetFollow();
        else
            pF = 0;
    }
    if ( pF )
        return pF->Frm().Pos();
    else
        return pFrm->Frm().Pos();
}

static sal_Bool lcl_SetAnchor( const SwPosition& rPos, const SwNode& rNd, SwFlyFrm* pFly,
                const Point& rInsPt, SwFEShell& rDestShell, SwFmtAnchor& rAnchor,
                Point& rNewPos, sal_Bool bCheckFlyRecur )
{
    sal_Bool bRet = sal_True;
    rAnchor.SetAnchor( &rPos );
    SwCntntFrm* pTmpFrm = rNd.GetCntntNode()->getLayoutFrm( rDestShell.GetLayout(), &rInsPt, 0, sal_False );
    SwFlyFrm *pTmpFly = pTmpFrm->FindFlyFrm();
    if( pTmpFly && bCheckFlyRecur && pFly->IsUpperOf( *pTmpFly ) )
    {
        bRet = sal_False;
    }
    else if ( FLY_AT_FLY == rAnchor.GetAnchorId() )
    {
        if( pTmpFly )
        {
            const SwNodeIndex& rIdx = *pTmpFly->GetFmt()->GetCntnt().GetCntntIdx();
            SwPosition aPos( rIdx );
            rAnchor.SetAnchor( &aPos );
            rNewPos = pTmpFly->Frm().Pos();
        }
        else
        {
            rAnchor.SetType( FLY_AT_PAGE );
            rAnchor.SetPageNum( rDestShell.GetPageNumber( rInsPt ) );
            const SwFrm *pPg = pTmpFrm->FindPageFrm();
            rNewPos = pPg->Frm().Pos();
        }
    }
    else
        rNewPos = ::lcl_FindBasePos( pTmpFrm, rInsPt );
    return bRet;
}

sal_Bool SwFEShell::CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                    const Point& rInsPt, sal_Bool bIsMove, sal_Bool bSelectInsert )
{
    sal_Bool bRet = sal_True;

    // The list should be copied, because below new objects will be selected
    const SdrMarkList aMrkList( Imp()->GetDrawView()->GetMarkedObjectList() );
    sal_uLong nMarkCount = aMrkList.GetMarkCount();
    if( !pDestShell->Imp()->GetDrawView() )
        // should create it now
        pDestShell->MakeDrawView();
    else if( bSelectInsert )
        pDestShell->Imp()->GetDrawView()->UnmarkAll();

    SdrPageView *pDestPgView = pDestShell->Imp()->GetPageView(),
                *pSrcPgView = Imp()->GetPageView();
    SwDrawView *pDestDrwView = pDestShell->Imp()->GetDrawView(),
                *pSrcDrwView = Imp()->GetDrawView();
    SwDoc* pDestDoc = pDestShell->GetDoc();

    Size aSiz( rInsPt.X() - rSttPt.X(), rInsPt.Y() - rSttPt.Y() );
    for( sal_uInt16 i = 0; i < nMarkCount; ++i )
    {
        SdrObject *pObj = aMrkList.GetMark( i )->GetMarkedSdrObj();

        SwDrawContact *pContact = (SwDrawContact*)GetUserCall( pObj );
        SwFrmFmt *pFmt = pContact->GetFmt();
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();

        sal_Bool bInsWithFmt = sal_True;

        if( pDestDrwView->IsGroupEntered() )
        {
            // insert into the group, when it belongs to an entered group
            // or when the object is not anchored as a character
            if( pSrcDrwView->IsGroupEntered() ||
                (FLY_AS_CHAR != rAnchor.GetAnchorId()) )

            {
                SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                        GetDoc() == pDestDoc, false );
                pNew->NbcMove( aSiz );
                pDestDrwView->InsertObjectAtView( pNew, *pDestPgView );
                bInsWithFmt = sal_False;
            }
        }

        if( bInsWithFmt )
        {
            SwFmtAnchor aAnchor( rAnchor );
            Point aNewAnch;

            if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                (FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
                (FLY_AS_CHAR == aAnchor.GetAnchorId()))
            {
                if ( this == pDestShell )
                {
                    // same shell? Then request the position
                    // from the passed DocumentPosition
                    SwPosition aPos( *GetCrsr()->GetPoint() );
                    Point aPt( rInsPt );
                    aPt -= rSttPt - pObj->GetSnapRect().TopLeft();
                    SwCrsrMoveState aState( MV_SETONLYTEXT );
                    GetLayout()->GetCrsrOfst( &aPos, aPt, &aState );
                    const SwNode *pNd;
                    if( (pNd = &aPos.nNode.GetNode())->IsNoTxtNode() )
                        bRet = sal_False;
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, 0, rInsPt,
                                *pDestShell, aAnchor, aNewAnch, sal_False );
                }
                else
                {
                    SwPaM *pCrsr = pDestShell->GetCrsr();
                    if( pCrsr->GetNode()->IsNoTxtNode() )
                        bRet = sal_False;
                    else
                        bRet = ::lcl_SetAnchor( *pCrsr->GetPoint(),
                                                *pCrsr->GetNode(), 0, rInsPt,
                                                *pDestShell, aAnchor,
                                                aNewAnch, sal_False );
                }
            }
            else if ( FLY_AT_PAGE == aAnchor.GetAnchorId() )
            {
                aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
                const SwRootFrm* pTmpRoot = pDestShell->GetLayout();
                const SwFrm* pPg = pTmpRoot->GetPageAtPos( rInsPt, 0, true );
                if ( pPg )
                    aNewAnch = pPg->Frm().Pos();
            }

            if( bRet )
            {
                if( pSrcDrwView->IsGroupEntered() ||
                    ( !pObj->GetUserCall() && pObj->GetUpGroup()) )
                {
                    SfxItemSet aSet( pDestDoc->GetAttrPool(),aFrmFmtSetRange);
                    aSet.Put( aAnchor );
                    SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                                GetDoc() == pDestDoc, true );
                    pFmt = pDestDoc->Insert( *pDestShell->GetCrsr(),
                                            *pNew, &aSet, NULL );
                }
                else
                    pFmt = pDestDoc->CopyLayoutFmt( *pFmt, aAnchor, true, true );

                // Can be 0, as Draws are not allowed in Headers/Footers
                if ( pFmt )
                {
                    SdrObject* pNew = pFmt->FindSdrObject();
                    if ( FLY_AS_CHAR != aAnchor.GetAnchorId() )
                    {
                        Point aPos( rInsPt );
                        aPos -= aNewAnch;
                        aPos -= rSttPt - pObj->GetSnapRect().TopLeft();
                        // OD 2004-04-05 #i26791# - change attributes instead of
                        // direct positioning
                        pFmt->SetFmtAttr( SwFmtHoriOrient( aPos.X(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
                        pFmt->SetFmtAttr( SwFmtVertOrient( aPos.Y(), text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
                        // #i47455# - notify draw frame format
                        // that position attributes are already set.
                        if ( pFmt->ISA(SwDrawFrmFmt) )
                        {
                            static_cast<SwDrawFrmFmt*>(pFmt)->PosAttrSet();
                        }
                    }
                    if( bSelectInsert )
                        pDestDrwView->MarkObj( pNew, pDestPgView );
                }
            }
        }
    }

    if ( bIsMove && bRet )
    {
        if( pDestShell == this )
        {
            const SdrMarkList aList( pSrcDrwView->GetMarkedObjectList() );
            pSrcDrwView->UnmarkAll();

            sal_uLong nMrkCnt = aMrkList.GetMarkCount();
            sal_uInt16 i;
            for ( i = 0; i < nMrkCnt; ++i )
            {
                SdrObject *pObj = aMrkList.GetMark( i )->GetMarkedSdrObj();
                pSrcDrwView->MarkObj( pObj, pSrcPgView );
            }
            DelSelectedObj();
            nMrkCnt = aList.GetMarkCount();
            for ( i = 0; i < nMrkCnt; ++i )
            {
                SdrObject *pObj = aList.GetMark( i )->GetMarkedSdrObj();
                pSrcDrwView->MarkObj( pObj, pSrcPgView );
            }
        }
        else
            DelSelectedObj();
    }

    return bRet;
}

sal_Bool SwFEShell::Copy( SwFEShell* pDestShell, const Point& rSttPt,
                    const Point& rInsPt, sal_Bool bIsMove, sal_Bool bSelectInsert )
{
    sal_Bool bRet = sal_False;

    OSL_ENSURE( pDestShell, "Copy without DestShell." );
    OSL_ENSURE( this == pDestShell || !pDestShell->IsObjSelected(),
            "Dest-Shell cannot be in Obj-Mode" );

    SET_CURR_SHELL( pDestShell );

    pDestShell->StartAllAction();
    pDestShell->GetDoc()->LockExpFlds();

    // Shift references
    sal_Bool bCopyIsMove = pDoc->IsCopyIsMove();
    if( bIsMove )
        // set a flag in Doc, handled in TextNodes
        pDoc->SetCopyIsMove( sal_True );

    RedlineMode_t eOldRedlMode = pDestShell->GetDoc()->GetRedlineMode();
    pDestShell->GetDoc()->SetRedlineMode_intern( (RedlineMode_t)(eOldRedlMode | nsRedlineMode_t::REDLINE_DELETE_REDLINES));

    // If there are table formulas in the area, then display the table first
    // so that the table formula can calculate a new value first
    // (individual boxes in the area are retrieved via the layout)
     SwFieldType* pTblFldTyp = pDestShell->GetDoc()->GetSysFldType( RES_TABLEFLD );

    if( IsFrmSelected() )
    {
        SwFlyFrm* pFly = FindFlyFrm();
        SwFrmFmt* pFlyFmt = pFly->GetFmt();
        SwFmtAnchor aAnchor( pFlyFmt->GetAnchor() );
        bRet = sal_True;
        Point aNewAnch;

        if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
            (FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
            (FLY_AS_CHAR == aAnchor.GetAnchorId()))
        {
            if ( this == pDestShell )
            {
                // same shell? Then request the position
                // from the passed DocumentPosition
                SwPosition aPos( *GetCrsr()->GetPoint() );
                Point aPt( rInsPt );
                aPt -= rSttPt - pFly->Frm().Pos();
                SwCrsrMoveState aState( MV_SETONLYTEXT );
                GetLayout()->GetCrsrOfst( &aPos, aPt, &aState );
                const SwNode *pNd;
                if( (pNd = &aPos.nNode.GetNode())->IsNoTxtNode() )
                    bRet = sal_False;
                else
                {
                    // do not copy in itself
                    const SwNodeIndex *pTmp = pFlyFmt->GetCntnt().GetCntntIdx();
                    if ( aPos.nNode > *pTmp && aPos.nNode <
                        pTmp->GetNode().EndOfSectionIndex() )
                    {
                        bRet = sal_False;
                    }
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, pFly, rInsPt,
                                        *pDestShell, aAnchor, aNewAnch, sal_True );
                }
            }
            else
            {
                const SwPaM *pCrsr = pDestShell->GetCrsr();
                if( pCrsr->GetNode()->IsNoTxtNode() )
                    bRet = sal_False;
                else
                    bRet = ::lcl_SetAnchor( *pCrsr->GetPoint(), *pCrsr->GetNode(),
                                            pFly, rInsPt, *pDestShell, aAnchor,
                                    aNewAnch, GetDoc() == pDestShell->GetDoc());
            }
        }
        else if ( FLY_AT_PAGE == aAnchor.GetAnchorId() )
        {
            aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
            const SwRootFrm* pTmpRoot = pDestShell->GetLayout();
            const SwFrm* pPg = pTmpRoot->GetPageAtPos( rInsPt, 0, true );
            if ( pPg )
                aNewAnch = pPg->Frm().Pos();
        }
        else {
            OSL_ENSURE( !this, "what anchor is it?" );
        }

        if( bRet )
        {
            SwFrmFmt *pOldFmt = pFlyFmt;
            pFlyFmt = pDestShell->GetDoc()->CopyLayoutFmt( *pFlyFmt, aAnchor, true, true );

            if ( FLY_AS_CHAR != aAnchor.GetAnchorId() )
            {
                Point aPos( rInsPt );
                aPos -= aNewAnch;
                aPos -= rSttPt - pFly->Frm().Pos();
                pFlyFmt->SetFmtAttr( SwFmtHoriOrient( aPos.X(),text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
                pFlyFmt->SetFmtAttr( SwFmtVertOrient( aPos.Y(),text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
            }

            const Point aPt( pDestShell->GetCrsrDocPos() );

            if( bIsMove )
                GetDoc()->DelLayoutFmt( pOldFmt );

            // only select if it can be shifted/copied in the same shell
            if( bSelectInsert )
            {
                SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aPt, sal_False );
                if( pFlyFrm )
                {
                    //JP 12.05.98: should this be in SelectFlyFrm???
                    pDestShell->Imp()->GetDrawView()->UnmarkAll();
                    pDestShell->SelectFlyFrm( *pFlyFrm, sal_True );
                }
            }

            if( this != pDestShell && !pDestShell->HasShFcs() )
                pDestShell->Imp()->GetDrawView()->hideMarkHandles();
        }
    }
    else if ( IsObjSelected() )
        bRet = CopyDrawSel( pDestShell, rSttPt, rInsPt, bIsMove, bSelectInsert );
    else if( IsTableMode() )
    {
        // Copy parts from a table: create a table with the same
        // width as the original and copy the selected boxes.
        // Sizes will be corrected by percentage.

        // find boxes via the layout
        const SwTableNode* pTblNd;
        SwSelBoxes aBoxes;
        GetTblSel( *this, aBoxes );
        if( !aBoxes.empty() &&
            0 != (pTblNd = aBoxes[0]->GetSttNd()->FindTableNode()) )
        {
            SwPosition* pDstPos = 0;
            if( this == pDestShell )
            {
                // same shell? Then create new Crsr at the
                // DocumentPosition passed
                pDstPos = new SwPosition( *GetCrsr()->GetPoint() );
                Point aPt( rInsPt );
                GetLayout()->GetCrsrOfst( pDstPos, aPt );
                if( !pDstPos->nNode.GetNode().IsNoTxtNode() )
                    bRet = sal_True;
            }
            else if( !pDestShell->GetCrsr()->GetNode()->IsNoTxtNode() )
            {
                pDstPos = new SwPosition( *pDestShell->GetCrsr()->GetPoint() );
                bRet = sal_True;
            }

            if( bRet )
            {
                if( GetDoc() == pDestShell->GetDoc() )
                    ParkTblCrsr();

                bRet = pDestShell->GetDoc()->InsCopyOfTbl( *pDstPos, aBoxes,0,
                                        bIsMove && this == pDestShell &&
                                        aBoxes.size() == pTblNd->GetTable().
                                        GetTabSortBoxes().size(),
                                        this != pDestShell );

                if( this != pDestShell )
                    *pDestShell->GetCrsr()->GetPoint() = *pDstPos;

                // create all parked Crsr?
                if( GetDoc() == pDestShell->GetDoc() )
                    GetCrsr();

                // JP 16.04.99: Bug 64908 - Set InsPos, to assure the parked
                //              Cursor is positioned at the insert position
                if( this == pDestShell )
                    GetCrsrDocPos() = rInsPt;
            }
            delete pDstPos;
        }
    }
    else
    {
        bRet = sal_True;
        if( this == pDestShell )
        {
            // same shell? then request the postion
            // at the passed document position
            SwPosition aPos( *GetCrsr()->GetPoint() );
            Point aPt( rInsPt );
            GetLayout()->GetCrsrOfst( &aPos, aPt );
            bRet = !aPos.nNode.GetNode().IsNoTxtNode();
        }
        else if( pDestShell->GetCrsr()->GetNode()->IsNoTxtNode() )
            bRet = sal_False;

        if( bRet )
            bRet = 0 != SwEditShell::Copy( pDestShell );
    }

    pDestShell->GetDoc()->SetRedlineMode_intern( eOldRedlMode );
    pDoc->SetCopyIsMove( bCopyIsMove );

    // have new table formules been inserted?
    if( pTblFldTyp->GetDepends() )
    {
        // finish old actions: the table frames are created and
        // a selection can be made
        sal_uInt16 nActCnt;
        for( nActCnt = 0; pDestShell->ActionPend(); ++nActCnt )
            pDestShell->EndAllAction();

        for( ; nActCnt; --nActCnt )
            pDestShell->StartAllAction();
    }
    pDestShell->GetDoc()->UnlockExpFlds();
    pDestShell->GetDoc()->UpdateFlds(NULL, false);

    pDestShell->EndAllAction();
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::Paste()  Paste for  the interal clipboard.
|*      Copy the content of the clipboard in the document
|*
|*************************************************************************/

namespace {
    typedef boost::shared_ptr<SwPaM> PaMPtr;
    typedef boost::shared_ptr<SwPosition> PositionPtr;
    typedef std::pair< PaMPtr, PositionPtr > Insertion;
}

sal_Bool SwFEShell::Paste( SwDoc* pClpDoc, sal_Bool bIncludingPageFrames )
{
    SET_CURR_SHELL( this );
    OSL_ENSURE( pClpDoc, "no clipboard document"  );
    const sal_uInt16 nStartPageNumber = GetPhyPageNum();
    // then till end of the nodes array
    SwNodeIndex aIdx( pClpDoc->GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aIdx ); //DocStart

    // If there are table formulas in the area, then display the table first
    // so that the table formula can calculate a new value first
    // (individual boxes in the area are retrieved via the layout)
     SwFieldType* pTblFldTyp = GetDoc()->GetSysFldType( RES_TABLEFLD );

    SwTableNode *pDestNd, *pSrcNd = aCpyPam.GetNode()->GetTableNode();
    if( !pSrcNd )                               // TabellenNode ?
    {                                           // nicht ueberspringen!!
        SwCntntNode* pCNd = aCpyPam.GetNode()->GetCntntNode();
        if( pCNd )
            aCpyPam.GetPoint()->nContent.Assign( pCNd, 0 );
        else if( !aCpyPam.Move( fnMoveForward, fnGoNode ))
            aCpyPam.Move( fnMoveBackward, fnGoNode );
    }

    aCpyPam.SetMark();
    aCpyPam.Move( fnMoveForward, fnGoDoc );

    sal_Bool bRet = sal_True, bDelTbl = sal_True;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, NULL );
    GetDoc()->LockExpFlds();

    // When the clipboard content has been created by a rectangular selection
    // the pasting is more sophisticated:
    // every paragraph will be inserted into another position.
    // The first positions are given by the actual cursor ring,
    // if there are more text portions to insert than cursor in this ring,
    // the additional insert positions will be created by moving the last
    // cursor position into the next line (like pressing the cursor down key)
    if( pClpDoc->IsColumnSelection() && !IsTableMode() )
    {
        // Creation of the list of insert positions
        std::list< Insertion > aCopyList;
        // The number of text portions of the rectangular selection
        const sal_uInt32 nSelCount = aCpyPam.GetPoint()->nNode.GetIndex()
                       - aCpyPam.GetMark()->nNode.GetIndex();
        sal_uInt32 nCount = nSelCount;
        SwNodeIndex aClpIdx( aIdx );
        SwPaM* pStartCursor = GetCrsr();
        SwPaM* pCurrCrsr = pStartCursor;
        sal_uInt32 nCursorCount = pStartCursor->numberOf();
        // If the target selection is a multi-selection, often the last and first
        // cursor of the ring points to identical document positions. Then
        // we should avoid double insertion of text portions...
        while( nCursorCount > 1 && *pCurrCrsr->GetPoint() ==
            *(dynamic_cast<SwPaM*>(pCurrCrsr->GetPrev())->GetPoint()) )
        {
            --nCursorCount;
            pCurrCrsr = dynamic_cast<SwPaM*>(pCurrCrsr->GetNext());
            pStartCursor = pCurrCrsr;
        }
        SwPosition aStartPos( *pStartCursor->GetPoint() );
        SwPosition aInsertPos( aStartPos ); // first insertion position
        bool bCompletePara = false;
        sal_uInt16 nMove = 0;
        while( nCount )
        {
            --nCount;
            OSL_ENSURE( aIdx.GetNode().GetCntntNode(), "Who filled the clipboard?!" );
            if( aIdx.GetNode().GetCntntNode() ) // robust
            {
                Insertion aInsertion( PaMPtr( new SwPaM( aIdx ) ),
                    PositionPtr( new SwPosition( aInsertPos ) ) );
                ++aIdx;
                aInsertion.first->SetMark();
                if( pStartCursor == pCurrCrsr->GetNext() )
                {   // Now we have to look for insertion positions...
                    if( !nMove ) // Annotate the last given insert position
                        aStartPos = aInsertPos;
                    SwCursor aCrsr( aStartPos, 0, false);
                    // Check if we find another insert position by moving
                    // down the last given position
                    if( aCrsr.UpDown( sal_False, ++nMove, 0, 0 ) )
                        aInsertPos = *aCrsr.GetPoint();
                    else // if there is no paragraph we have to create it
                        bCompletePara = nCount > 0;
                    nCursorCount = 0;
                }
                else // as long as we find more insert positions in the cursor ring
                {    // we'll take them
                    pCurrCrsr = dynamic_cast<SwPaM*>(pCurrCrsr->GetNext());
                    aInsertPos = *pCurrCrsr->GetPoint();
                    --nCursorCount;
                }
                // If there are no more paragraphs e.g. at the end of a document,
                // we insert complete paragraphs instead of text portions
                if( bCompletePara )
                    aInsertion.first->GetPoint()->nNode = aIdx;
                else
                    aInsertion.first->GetPoint()->nContent =
                        aInsertion.first->GetCntntNode()->Len();
                aCopyList.push_back( aInsertion );
            }
            // If there are no text portions left but there are some more
            // cursor positions to fill we have to restart with the first
            // text portion
            if( !nCount && nCursorCount )
            {
                nCount = std::min( nSelCount, nCursorCount );
                aIdx = aClpIdx; // Start of clipboard content
            }
        }
        std::list< Insertion >::const_iterator pCurr = aCopyList.begin();
        std::list< Insertion >::const_iterator pEnd = aCopyList.end();
        while( pCurr != pEnd )
        {
            SwPosition& rInsPos = *pCurr->second;
            SwPaM& rCopy = *pCurr->first;
            const SwStartNode* pBoxNd = rInsPos.nNode.GetNode().FindTableBoxStartNode();
            if( pBoxNd && 2 == pBoxNd->EndOfSectionIndex() - pBoxNd->GetIndex() &&
                rCopy.GetPoint()->nNode != rCopy.GetMark()->nNode )
            {
                // if more than one node will be copied into a cell
                // the box attributes have to be removed
                GetDoc()->ClearBoxNumAttrs( rInsPos.nNode );
            }
            {
                SwNodeIndex aIndexBefore(rInsPos.nNode);
                aIndexBefore--;
                pClpDoc->CopyRange( rCopy, rInsPos, false );
                {
                    ++aIndexBefore;
                    SwPaM aPaM(SwPosition(aIndexBefore),
                               SwPosition(rInsPos.nNode));
                    aPaM.GetDoc()->MakeUniqueNumRules(aPaM);
                }
            }
            SaveTblBoxCntnt( &rInsPos );
            ++pCurr;
        }
    }
    else
    {
        FOREACHPAM_START(this)

        if( pSrcNd &&
            0 != ( pDestNd = GetDoc()->IsIdxInTbl( PCURCRSR->GetPoint()->nNode )))
        {
            SwPosition aDestPos( *PCURCRSR->GetPoint() );

            sal_Bool bParkTblCrsr = sal_False;
            const SwStartNode* pSttNd =  PCURCRSR->GetNode()->FindTableBoxStartNode();

            // TABLE IN TABLE: copy table in table
            // search boxes via the layout
            SwSelBoxes aBoxes;
            if( IsTableMode() )     // table selection?
            {
                GetTblSel( *this, aBoxes );
                ParkTblCrsr();
                bParkTblCrsr = sal_True;
            }
            else if( !PCURCRSR->HasMark() && PCURCRSR->GetNext() == PCURCRSR &&
                     ( !pSrcNd->GetTable().IsTblComplex() ||
                       pDestNd->GetTable().IsNewModel() ) )
            {
                // make relative table copy
                SwTableBox* pBox = pDestNd->GetTable().GetTblBox(
                                        pSttNd->GetIndex() );
                OSL_ENSURE( pBox, "Box steht nicht in dieser Tabelle" );
                aBoxes.insert( pBox );
            }

            SwNodeIndex aNdIdx( *pDestNd->EndOfSectionNode());
            if( !bParkTblCrsr )
            {
                // exit first the complete table
                // ???? what about only table in a frame ?????
                SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aNdIdx );
                SwPosition aPos( aNdIdx, SwIndex( pCNd, 0 ));
                // #i59539: Don't remove all redline
                SwPaM const tmpPaM(*pDestNd, *pDestNd->EndOfSectionNode());
                ::PaMCorrAbs(tmpPaM, aPos);
            }

            bRet = GetDoc()->InsCopyOfTbl( aDestPos, aBoxes, &pSrcNd->GetTable(),
                                            sal_False, sal_False );

            if( bParkTblCrsr )
                GetCrsr();
            else
            {
                // return to the box
                aNdIdx = *pSttNd;
                SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aNdIdx );
                SwPosition aPos( aNdIdx, SwIndex( pCNd, 0 ));
                // #i59539: Don't remove all redline
                SwNode & rNode(PCURCRSR->GetPoint()->nNode.GetNode());
                SwCntntNode *const pCntntNode( rNode.GetCntntNode() );
                SwPaM const tmpPam(rNode, 0,
                                   rNode, (pCntntNode) ? pCntntNode->Len() : 0);
                ::PaMCorrAbs(tmpPam, aPos);
            }

            break;      // exit the "while-loop"
        }
        else if( *aCpyPam.GetPoint() == *aCpyPam.GetMark() &&
                 pClpDoc->GetSpzFrmFmts()->size() )
        {
            // we need a DrawView
            if( !Imp()->GetDrawView() )
                MakeDrawView();

            for ( sal_uInt16 i = 0; i < pClpDoc->GetSpzFrmFmts()->size(); ++i )
            {
                sal_Bool bInsWithFmt = sal_True;
                const SwFrmFmt& rCpyFmt = *(*pClpDoc->GetSpzFrmFmts())[i];

                if( Imp()->GetDrawView()->IsGroupEntered() &&
                    RES_DRAWFRMFMT == rCpyFmt.Which() &&
                    (FLY_AS_CHAR != rCpyFmt.GetAnchor().GetAnchorId()) )
                {
                    const SdrObject* pSdrObj = rCpyFmt.FindSdrObject();
                    if( pSdrObj )
                    {
                        SdrObject* pNew = GetDoc()->CloneSdrObj( *pSdrObj,
                                                            false, false );

                        // Insert object sets any anchor position to 0.
                        // Therefore we calculate the absolute position here
                        // and after the insert the anchor of the object
                        // is set to the anchor of the group object.
                        Rectangle aSnapRect = pNew->GetSnapRect();
                        if( pNew->GetAnchorPos().X() || pNew->GetAnchorPos().Y() )
                        {
                            const Point aPoint( 0, 0 );
                            // OD 2004-04-05 #i26791# - direct drawing object
                            // positioning for group members
                            pNew->NbcSetAnchorPos( aPoint );
                            pNew->NbcSetSnapRect( aSnapRect );
                        }

                        Imp()->GetDrawView()->InsertObjectAtView( pNew, *Imp()->GetPageView() );

                        Point aGrpAnchor( 0, 0 );
                        SdrObjList* pList = pNew->GetObjList();
                        if ( pList )
                        {
                            SdrObject* pOwner = pList->GetOwnerObj();
                            if ( pOwner )
                            {
                                SdrObjGroup* pThisGroup = PTR_CAST(SdrObjGroup, pOwner);
                                aGrpAnchor = pThisGroup->GetAnchorPos();
                            }
                        }

                        // OD 2004-04-05 #i26791# - direct drawing object
                        // positioning for group members
                        pNew->NbcSetAnchorPos( aGrpAnchor );
                        pNew->SetSnapRect( aSnapRect );

                        bInsWithFmt = sal_False;
                    }
                }

                if( bInsWithFmt  )
                {
                    SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
                    if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                        (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                        (FLY_AS_CHAR == aAnchor.GetAnchorId()))
                    {
                        SwPosition* pPos = PCURCRSR->GetPoint();
                        // allow shapes (no controls) in header/footer
                        if( RES_DRAWFRMFMT == rCpyFmt.Which() &&
                            GetDoc()->IsInHeaderFooter( pPos->nNode ) &&
                            CheckControlLayer( rCpyFmt.FindSdrObject() ) )
                            continue;

                        aAnchor.SetAnchor( pPos );
                    }
                    else if ( FLY_AT_PAGE == aAnchor.GetAnchorId() )
                    {
                        aAnchor.SetPageNum( GetPhyPageNum() );
                    }
                    else if( FLY_AT_FLY == aAnchor.GetAnchorId() )
                    {
                        Point aPt;
                        lcl_SetAnchor( *PCURCRSR->GetPoint(), *PCURCRSR->GetNode(),
                                        0, aPt, *this, aAnchor, aPt, sal_False );
                    }

                    SwFrmFmt * pNew = GetDoc()->CopyLayoutFmt( rCpyFmt, aAnchor, true, true );

                    if( pNew )
                    {
                        if( RES_FLYFRMFMT == pNew->Which() )
                        {
                            const Point aPt( GetCrsrDocPos() );
                            SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pNew)->
                                                        GetFrm( &aPt, sal_False );
                            if( pFlyFrm )
                                SelectFlyFrm( *pFlyFrm, sal_True );
                            // always pick the first FlyFrame only; the others
                            // were copied to the clipboard via Fly in Fly
                            break;
                        }
                        else
                        {
                            OSL_ENSURE( RES_DRAWFRMFMT == pNew->Which(), "Neues Format.");
                            // #i52780# - drawing object has
                            // to be made visible on paste.
                            {
                                SwDrawContact* pContact =
                                    static_cast<SwDrawContact*>(pNew->FindContactObj());
                                pContact->MoveObjToVisibleLayer( pContact->GetMaster() );
                            }
                            SdrObject *pObj = pNew->FindSdrObject();
                            SwDrawView  *pDV = Imp()->GetDrawView();
                            pDV->MarkObj( pObj, pDV->GetSdrPageView() );
                            // #i47455# - notify draw frame format
                            // that position attributes are already set.
                            if ( pNew->ISA(SwDrawFrmFmt) )
                            {
                                static_cast<SwDrawFrmFmt*>(pNew)->PosAttrSet();
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if( bDelTbl && IsTableMode() )
            {
                SwEditShell::Delete();
                bDelTbl = sal_False;
            }

            SwPosition& rInsPos = *PCURCRSR->GetPoint();
            const SwStartNode* pBoxNd = rInsPos.nNode.GetNode().
                                                    FindTableBoxStartNode();
            if( pBoxNd && 2 == pBoxNd->EndOfSectionIndex() -
                                pBoxNd->GetIndex() &&
                aCpyPam.GetPoint()->nNode != aCpyPam.GetMark()->nNode )
            {
                // Copy more than 1 node in the current box. But
                // then the BoxAttribute should be removed
                GetDoc()->ClearBoxNumAttrs( rInsPos.nNode );
            }

            // find out if the clipboard document starts with a table
            bool bStartWithTable = 0 != aCpyPam.Start()->nNode.GetNode().FindTableNode();
            SwPosition aInsertPosition( rInsPos );

            {
                SwNodeIndex aIndexBefore(rInsPos.nNode);

                aIndexBefore--;

                pClpDoc->CopyRange( aCpyPam, rInsPos, false );

                {
                    ++aIndexBefore;
                    SwPaM aPaM(SwPosition(aIndexBefore),
                               SwPosition(rInsPos.nNode));

                    aPaM.GetDoc()->MakeUniqueNumRules(aPaM);
                }
            }

            // Update the rsid of each pasted text node.
            {
                xub_StrLen nNodesCnt = aCpyPam.End()->nNode.GetIndex() - aCpyPam.Start()->nNode.GetIndex();
                SwNodes &rDestNodes = GetDoc()->GetNodes();
                xub_StrLen nDestStart = PCURCRSR->GetPoint()->nNode.GetIndex() - nNodesCnt;

                for ( sal_uInt64 nIdx = 0; nIdx <= nNodesCnt; nIdx++ )
                {
                    SwTxtNode *pTxtNode = rDestNodes[ nDestStart + nIdx ]->GetTxtNode();
                    if ( pTxtNode )
                    {
                        GetDoc()->UpdateParRsid( pTxtNode );
                    }
                }
            }

            SaveTblBoxCntnt( &rInsPos );
            if(bIncludingPageFrames && bStartWithTable)
            {
                //remove the paragraph in front of the table
                SwPaM aPara(aInsertPosition);
                GetDoc()->DelFullPara(aPara);
            }
            //additionally copy page bound frames
            if( bIncludingPageFrames && pClpDoc->GetSpzFrmFmts()->size() )
            {
                // create a draw view if necessary
                if( !Imp()->GetDrawView() )
                    MakeDrawView();

                for ( sal_uInt16 i = 0; i < pClpDoc->GetSpzFrmFmts()->size(); ++i )
                {
                    sal_Bool bInsWithFmt = sal_True;
                    const SwFrmFmt& rCpyFmt = *(*pClpDoc->GetSpzFrmFmts())[i];
                    if( bInsWithFmt  )
                    {
                        SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
                        if ( FLY_AT_PAGE == aAnchor.GetAnchorId() )
                        {
                            aAnchor.SetPageNum( aAnchor.GetPageNum() + nStartPageNumber - 1 );
                        }
                        else
                            continue;
                        GetDoc()->CopyLayoutFmt( rCpyFmt, aAnchor, true, true );
                    }
                }
            }
        }

        FOREACHPAM_END()
    }

    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_INSGLOSSARY, NULL );

    // have new table formulas been inserted?
    if( pTblFldTyp->GetDepends() )
    {
        // finish old action: table-frames have been created
        // a selection can be made now
        sal_uInt16 nActCnt;
        for( nActCnt = 0; ActionPend(); ++nActCnt )
            EndAllAction();

        for( ; nActCnt; --nActCnt )
            StartAllAction();
    }
    GetDoc()->UnlockExpFlds();
    GetDoc()->UpdateFlds(NULL, false);
    EndAllAction();

    return bRet;
}

sal_Bool SwFEShell::PastePages( SwFEShell& rToFill, sal_uInt16 nStartPage, sal_uInt16 nEndPage)
{
    Push();
    if(!GotoPage(nStartPage))
    {
        Pop(sal_False);
        return sal_False;
    }
    MovePage( fnPageCurr, fnPageStart );
    SwPaM aCpyPam( *GetCrsr()->GetPoint() );
    String sStartingPageDesc = GetPageDesc( GetCurPageDesc()).GetName();
    SwPageDesc* pDesc = rToFill.FindPageDescByName( sStartingPageDesc, sal_True );
    if( pDesc )
        rToFill.ChgCurPageDesc( *pDesc );

    if(!GotoPage(nEndPage))
    {
        Pop(sal_False);
        return sal_False;
    }
    //if the page starts with a table a paragraph has to be inserted before
    SwNode* pTableNode = aCpyPam.GetNode()->FindTableNode();
    if(pTableNode)
    {
        //insert a paragraph
        StartUndo(UNDO_INSERT);
        SwNodeIndex aTblIdx(  *pTableNode, -1 );
        SwPosition aBefore(aTblIdx);
        if(GetDoc()->AppendTxtNode( aBefore ))
        {
            SwPaM aTmp(aBefore);
            aCpyPam = aTmp;
        }
        EndUndo(UNDO_INSERT);
    }

    MovePage( fnPageCurr, fnPageEnd );
    aCpyPam.SetMark();
    *aCpyPam.GetMark() = *GetCrsr()->GetPoint();

    SET_CURR_SHELL( this );

    StartAllAction();
    GetDoc()->LockExpFlds();
    SetSelection(aCpyPam);
    // copy the text of the selection
    SwEditShell::Copy(&rToFill);

    if(pTableNode)
    {
        //remove the inserted paragraph
        Undo();
        //remove the paragraph in the second doc, too
        SwNodeIndex aIdx( rToFill.GetDoc()->GetNodes().GetEndOfExtras(), 2 );
        SwPaM aPara( aIdx ); //DocStart
        rToFill.GetDoc()->DelFullPara(aPara);
    }
    // now the page bound objects
    // additionally copy page bound frames
    if( GetDoc()->GetSpzFrmFmts()->size() )
    {
        // create a draw view if necessary
        if( !rToFill.Imp()->GetDrawView() )
            rToFill.MakeDrawView();

        for ( sal_uInt16 i = 0; i < GetDoc()->GetSpzFrmFmts()->size(); ++i )
        {
            const SwFrmFmt& rCpyFmt = *(*GetDoc()->GetSpzFrmFmts())[i];
            SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
            if ((FLY_AT_PAGE == aAnchor.GetAnchorId()) &&
                    aAnchor.GetPageNum() >= nStartPage && aAnchor.GetPageNum() <= nEndPage)
            {
                aAnchor.SetPageNum( aAnchor.GetPageNum() - nStartPage + 1);
            }
            else
                continue;
            rToFill.GetDoc()->CopyLayoutFmt( rCpyFmt, aAnchor, true, true );
        }
    }
    GetDoc()->UnlockExpFlds();
    GetDoc()->UpdateFlds(NULL, false);
    Pop(sal_False);
    EndAllAction();

    return sal_True;
}

sal_Bool SwFEShell::GetDrawObjGraphic( sal_uLong nFmt, Graphic& rGrf ) const
{
    OSL_ENSURE( Imp()->HasDrawView(), "GetDrawObjGraphic without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    sal_Bool bConvert = sal_True;
    if( rMrkList.GetMarkCount() )
    {
        if( rMrkList.GetMarkCount() == 1 &&
            rMrkList.GetMark( 0 )->GetMarkedSdrObj()->ISA(SwVirtFlyDrawObj) )
        {
            // select frame
            if( CNT_GRF == GetCntType() )
            {
                const Graphic* pGrf( GetGraphic() );
                if ( pGrf )
                {
                    Graphic aGrf( *pGrf );
                    if( SOT_FORMAT_GDIMETAFILE == nFmt )
                    {
                        if( GRAPHIC_BITMAP != aGrf.GetType() )
                        {
                            rGrf = aGrf;
                            bConvert = sal_False;
                        }
                        else if( GetWin() )
                        {
                            Size aSz;
                            Point aPt;
                            GetGrfSize( aSz );

                            VirtualDevice aVirtDev;
                            aVirtDev.EnableOutput( sal_False );

                            MapMode aTmp( GetWin()->GetMapMode() );
                            aTmp.SetOrigin( aPt );
                            aVirtDev.SetMapMode( aTmp );

                            GDIMetaFile aMtf;
                            aMtf.Record( &aVirtDev );
                            aGrf.Draw( &aVirtDev, aPt, aSz );
                            aMtf.Stop();
                            aMtf.SetPrefMapMode( aTmp );
                            aMtf.SetPrefSize( aSz );
                            rGrf = aMtf;
                        }
                    }
                    else if( GRAPHIC_BITMAP == aGrf.GetType() )
                    {
                        rGrf = aGrf;
                        bConvert = sal_False;
                    }
                    else
                    {
                        // fix(23806): not the origial size, but the current one.
                        // Otherwise it could happen that for vector graphics
                        // many MB's of memory are allocated.
                        const Size aSz( FindFlyFrm()->Prt().SSize() );
                        VirtualDevice aVirtDev( *GetWin() );

                        MapMode aTmp( MAP_TWIP );
                        aVirtDev.SetMapMode( aTmp );
                        if( aVirtDev.SetOutputSize( aSz ) )
                        {
                            aGrf.Draw( &aVirtDev, Point(), aSz );
                            rGrf = aVirtDev.GetBitmap( Point(), aSz );
                        }
                        else
                        {
                            rGrf = aGrf;
                            bConvert = sal_False;
                        }
                    }
                }
            }
        }
        else if( SOT_FORMAT_GDIMETAFILE == nFmt )
            rGrf = Imp()->GetDrawView()->GetAllMarkedMetaFile();
        else if( SOT_FORMAT_BITMAP == nFmt )
            rGrf = Imp()->GetDrawView()->GetAllMarkedBitmap();
    }
    return bConvert;
}

// #i50824#
// replace method <lcl_RemoveOleObjsFromSdrModel> by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
static void lcl_ConvertSdrOle2ObjsToSdrGrafObjs( SdrModel* _pModel )
{
    for ( sal_uInt16 nPgNum = 0; nPgNum < _pModel->GetPageCount(); ++nPgNum )
    {
        // setup object iterator in order to iterate through all objects
        // including objects in group objects, but exclusive group objects.
        SdrObjListIter aIter(*(_pModel->GetPage( nPgNum )));
        while( aIter.IsMore() )
        {
            SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( aIter.Next() );
            if( pOle2Obj )
            {
                // found an ole2 shape
                SdrObjList* pObjList = pOle2Obj->GetObjList();

                // get its graphic
                Graphic aGraphic;
                pOle2Obj->Connect();
                Graphic* pGraphic = pOle2Obj->GetGraphic();
                if( pGraphic )
                    aGraphic = *pGraphic;
                pOle2Obj->Disconnect();

                // create new graphic shape with the ole graphic and shape size
                SdrGrafObj* pGraphicObj = new SdrGrafObj( aGraphic, pOle2Obj->GetCurrentBoundRect() );
                // apply layer of ole2 shape at graphic shape
                pGraphicObj->SetLayer( pOle2Obj->GetLayer() );

                // replace ole2 shape with the new graphic object and delete the ol2 shape
                SdrObject* pRemovedObject = pObjList->ReplaceObject( pGraphicObj, pOle2Obj->GetOrdNum() );
                SdrObject::Free( pRemovedObject );
            }
        }
    }
}

void SwFEShell::Paste( SvStream& rStrm, sal_uInt16 nAction, const Point* pPt )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo();

    SvtPathOptions aPathOpt;
    FmFormModel* pModel = new FmFormModel( aPathOpt.GetPalettePath(),
                                            0, GetDoc()->GetDocShell() );
    pModel->GetItemPool().FreezeIdRanges();

    rStrm.Seek(0);

    uno::Reference< io::XInputStream > xInputStream( new utl::OInputStreamWrapper( rStrm ) );
    SvxDrawingLayerImport( pModel, xInputStream );

    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    Point aPos( pPt ? *pPt : GetCharRect().Pos() );
    SdrView *pView = Imp()->GetDrawView();

    // drop on the existing object: replace object or apply new attributes
    if( pModel->GetPageCount() > 0 &&
        1 == pModel->GetPage(0)->GetObjCount() &&
        1 == pView->GetMarkedObjectList().GetMarkCount() )
    {
        // replace a marked 'virtual' drawing object
        // by its corresponding 'master' drawing object in the mark list.
        SwDrawView::ReplaceMarkedDrawVirtObjs( *pView );

        SdrObject* pClpObj = pModel->GetPage(0)->GetObj(0);
        SdrObject* pOldObj = pView->GetMarkedObjectList().GetMark( 0 )->GetMarkedSdrObj();

        if( SW_PASTESDR_SETATTR == nAction && pOldObj->ISA(SwVirtFlyDrawObj) )
            nAction = SW_PASTESDR_REPLACE;

        switch( nAction )
        {
        case SW_PASTESDR_REPLACE:
            {
                const SwFrmFmt* pFmt(0);
                const SwFrm* pAnchor(0);
                if( pOldObj->ISA(SwVirtFlyDrawObj) )
                {
                    pFmt = FindFrmFmt( pOldObj );

                    Point aNullPt;
                    SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFmt)->GetFrm( &aNullPt );
                    pAnchor = pFlyFrm->GetAnchorFrm();

                    if( pAnchor->FindFooterOrHeader() )
                    {
                        // if there is a textframe in the header/footer:
                        // do not replace but insert
                        nAction = SW_PASTESDR_INSERT;
                        break;
                    }
                }

                SdrObject* pNewObj = pClpObj->Clone();
                Rectangle aOldObjRect( pOldObj->GetCurrentBoundRect() );
                Size aOldObjSize( aOldObjRect.GetSize() );
                Rectangle aNewRect( pNewObj->GetCurrentBoundRect() );
                Size aNewSize( aNewRect.GetSize() );

                Fraction aScaleWidth( aOldObjSize.Width(), aNewSize.Width() );
                Fraction aScaleHeight( aOldObjSize.Height(), aNewSize.Height());
                pNewObj->NbcResize( aNewRect.TopLeft(), aScaleWidth, aScaleHeight);

                Point aVec = aOldObjRect.TopLeft() - aNewRect.TopLeft();
                pNewObj->NbcMove(Size(aVec.X(), aVec.Y()));

                if( pNewObj->ISA( SdrUnoObj ) )
                    pNewObj->SetLayer( GetDoc()->GetControlsId() );
                else if( pOldObj->ISA( SdrUnoObj ) )
                    pNewObj->SetLayer( GetDoc()->GetHeavenId() );
                else
                    pNewObj->SetLayer( pOldObj->GetLayer() );

                if( pOldObj->ISA(SwVirtFlyDrawObj) )
                {
                    // store attributes, then set SdrObject
                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                            RES_SURROUND, RES_ANCHOR );
                    aFrmSet.Set( pFmt->GetAttrSet() );

                    Point aNullPt;
                    if( pAnchor->IsTxtFrm() && ((SwTxtFrm*)pAnchor)->IsFollow() )
                    {
                        const SwTxtFrm* pTmp = (SwTxtFrm*)pAnchor;
                        do {
                            pTmp = pTmp->FindMaster();
                            OSL_ENSURE( pTmp, "Where's my Master?" );
                        } while( pTmp->IsFollow() );
                        pAnchor = pTmp;
                    }
                    if( pOldObj->ISA( SdrCaptionObj ))
                        aNullPt = ((SdrCaptionObj*)pOldObj)->GetTailPos();
                    else
                        aNullPt = aOldObjRect.TopLeft();

                    Point aNewAnchor = pAnchor->GetFrmAnchorPos( ::HasWrap( pOldObj ) );
                    // OD 2004-04-05 #i26791# - direct positioning of Writer
                    // fly frame object for <SwDoc::Insert(..)>
                    pNewObj->NbcSetRelativePos( aNullPt - aNewAnchor );
                    pNewObj->NbcSetAnchorPos( aNewAnchor );

                    pOldObj->GetOrdNum();

                    DelSelectedObj();

                    pFmt = GetDoc()->Insert( *GetCrsr(), *pNewObj, &aFrmSet, NULL );
                }
                else
                    pView->ReplaceObjectAtView( pOldObj, *Imp()->GetPageView(), pNewObj, sal_True );
            }
            break;

        case SW_PASTESDR_SETATTR:
            {
                SfxItemSet aSet( GetAttrPool() );
                aSet.Put(pClpObj->GetMergedItemSet());
                pView->SetAttributes( aSet, sal_False );
            }
            break;

        default:
            nAction = SW_PASTESDR_INSERT;
            break;
        }
    }
    else
        nAction = SW_PASTESDR_INSERT;

    if( SW_PASTESDR_INSERT == nAction )
    {
        ::sw::DrawUndoGuard drawUndoGuard(GetDoc()->GetIDocumentUndoRedo());

        sal_Bool bDesignMode = pView->IsDesignMode();
        if( !bDesignMode )
            pView->SetDesignMode( sal_True );

        // #i50824#
        // method <lcl_RemoveOleObjsFromSdrModel> replaced by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
        lcl_ConvertSdrOle2ObjsToSdrGrafObjs( pModel );
        pView->Paste( *pModel, aPos );

        sal_uLong nCnt = pView->GetMarkedObjectList().GetMarkCount();
        if( nCnt )
        {
            const Point aNull( 0, 0 );
            for( sal_uLong i=0; i < nCnt; ++i )
            {
                SdrObject *pObj = pView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
                pObj->ImpSetAnchorPos( aNull );
            }

            pView->SetCurrentObj( OBJ_GRUP, SdrInventor );
            if ( nCnt > 1 )
                pView->GroupMarked();
            SdrObject *pObj = pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            if( pObj->ISA( SdrUnoObj ) )
            {
                pObj->SetLayer( GetDoc()->GetControlsId() );
                bDesignMode = sal_True;
            }
            else
                pObj->SetLayer( GetDoc()->GetHeavenId() );
            const Rectangle &rSnap = pObj->GetSnapRect();
            const Size aDiff( rSnap.GetWidth()/2, rSnap.GetHeight()/2 );
            pView->MoveMarkedObj( aDiff );
            ImpEndCreate();
            if( !bDesignMode )
                pView->SetDesignMode( sal_False );
        }
    }
    EndUndo();
    EndAllAction();
    delete pModel;
}

sal_Bool SwFEShell::Paste( const Graphic &rGrf )
{
    SET_CURR_SHELL( this );
    SdrObject* pObj;
    SdrView *pView = Imp()->GetDrawView();

    sal_Bool bRet = 1 == pView->GetMarkedObjectList().GetMarkCount() &&
        (pObj = pView->GetMarkedObjectList().GetMark( 0 )->GetMarkedSdrObj())->IsClosedObj() &&
        !pObj->ISA( SdrOle2Obj );

    if( bRet )
    {
        XOBitmap aXOBitmap( rGrf.GetBitmap() );
        SfxItemSet aSet( GetAttrPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP );
        aSet.Put( XFillStyleItem( XFILL_BITMAP ));
        aSet.Put( XFillBitmapItem( aEmptyStr, aXOBitmap ));
        pView->SetAttributes( aSet, sal_False );
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
