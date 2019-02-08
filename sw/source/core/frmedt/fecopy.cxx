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

#include <memory>
#include <hintids.hxx>

#include <vcl/graph.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/xflasit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <editeng/brushitem.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdouno.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/fmmodel.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx>
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
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <DocumentFieldsManager.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <textboxhelper.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdundo.hxx>

using namespace ::com::sun::star;

// Copy for the internal clipboard. Copies all selections to the clipboard.
void SwFEShell::Copy( SwDoc* pClpDoc, const OUString* pNewClpText )
{
    OSL_ENSURE( pClpDoc, "No Clipboard document"  );

    pClpDoc->GetIDocumentUndoRedo().DoUndo(false); // always false!

    // delete content if ClpDocument contains content
    SwNodeIndex aSttIdx( pClpDoc->GetNodes().GetEndOfExtras(), 2 );
    SwNodeIndex aEndNdIdx( *aSttIdx.GetNode().EndOfSectionNode() );
    SwTextNode* pTextNd = aSttIdx.GetNode().GetTextNode();
    if (!pTextNd || !pTextNd->GetText().isEmpty() ||
        aSttIdx.GetIndex()+1 != pClpDoc->GetNodes().GetEndOfContent().GetIndex() )
    {
        pClpDoc->GetNodes().Delete( aSttIdx,
            pClpDoc->GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );
        pTextNd = pClpDoc->GetNodes().MakeTextNode( aSttIdx,
                            pClpDoc->GetDfltTextFormatColl() );
        --aSttIdx;
    }

    // also delete surrounding FlyFrames if any
    for( const auto pFly : *pClpDoc->GetSpzFrameFormats() )
    {
        SwFormatAnchor const*const pAnchor = &pFly->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        if (pAPos &&
            ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            aSttIdx <= pAPos->nNode && pAPos->nNode <= aEndNdIdx )
        {
            pClpDoc->getIDocumentLayoutAccess().DelLayoutFormat( pFly );
        }
    }

    pClpDoc->GetDocumentFieldsManager().GCFieldTypes();        // delete the FieldTypes

    // if a string was passed, copy it to the clipboard-
    // document. Then also the Calculator can use the internal
    // clipboard
    if( pNewClpText )
    {
        pTextNd->InsertText( *pNewClpText, SwIndex( pTextNd ) );
        return;                // that's it
    }

    pClpDoc->getIDocumentFieldsAccess().LockExpFields();
    pClpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::DeleteRedlines );

    // do we want to copy a FlyFrame?
    if( IsFrameSelected() )
    {
        // get the FlyFormat
        SwFlyFrame* pFly = GetSelectedFlyFrame();
        SwFrameFormat* pFlyFormat = pFly->GetFormat();
        SwFormatAnchor aAnchor( pFlyFormat->GetAnchor() );

        if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
        {
            SwPosition aPos( aSttIdx );
            if ( RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId() )
            {
                aPos.nContent.Assign( pTextNd, 0 );
            }
            aAnchor.SetAnchor( &aPos );
        }
        pFlyFormat = pClpDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFlyFormat, aAnchor, true, true );

        // assure the "RootFormat" is the first element in Spz-Array
        // (if necessary Flys were copied in Flys)
        SwFrameFormats& rSpzFrameFormats = *pClpDoc->GetSpzFrameFormats();
        if( rSpzFrameFormats[ 0 ] != pFlyFormat )
        {
#ifndef NDEBUG
            bool inserted =
#endif
                rSpzFrameFormats.newDefault( pFlyFormat );
            assert( !inserted && "Fly not contained in Spz-Array" );
        }

        if ( RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId() )
        {
            // JP 13.02.99  Bug 61863: if a frameselection is passed to the
            //              clipboard, it should be found at pasting. Therefore
            //              the copied TextAttribut should be removed in the node
            //              otherwise it will be recognised as TextSelektion
            const SwIndex& rIdx = pFlyFormat->GetAnchor().GetContentAnchor()->nContent;
            SwTextFlyCnt *const pTextFly = static_cast<SwTextFlyCnt *>(
                pTextNd->GetTextAttrForCharAt(
                    rIdx.GetIndex(), RES_TXTATR_FLYCNT));
            if( pTextFly )
            {
                const_cast<SwFormatFlyCnt&>(pTextFly->GetFlyCnt()).SetFlyFormat();
                pTextNd->EraseText( rIdx, 1 );
            }
        }
    }
    else if ( IsObjSelected() )
    {
        SwPosition aPos( aSttIdx, SwIndex( pTextNd, 0 ));
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();

            if( Imp()->GetDrawView()->IsGroupEntered() ||
                ( !pObj->GetUserCall() && pObj->getParentSdrObjectFromSdrObject()) )
            {
                SfxItemSet aSet( pClpDoc->GetAttrPool(), aFrameFormatSetRange );

                SwFormatAnchor aAnchor( RndStdIds::FLY_AT_PARA );
                aAnchor.SetAnchor( &aPos );
                aSet.Put( aAnchor );

                SdrObject *const pNew =
                    pClpDoc->CloneSdrObj( *pObj );

                SwPaM aTemp(aPos);
                pClpDoc->getIDocumentContentOperations().InsertDrawObj(aTemp, *pNew, aSet );
            }
            else
            {
                SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
                SwFrameFormat *pFormat = pContact->GetFormat();
                SwFormatAnchor aAnchor( pFormat->GetAnchor() );
                if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                    (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                    (RndStdIds::FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
                    (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
                {
                    aAnchor.SetAnchor( &aPos );
                }

                pClpDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, true, true );
            }
        }
    }
    else
        CopySelToDoc( pClpDoc );     // copy the selections

    pClpDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::NONE );
    pClpDoc->getIDocumentFieldsAccess().UnlockExpFields();
    if( !pClpDoc->getIDocumentFieldsAccess().IsExpFieldsLocked() )
        pClpDoc->getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);
}

static const Point &lcl_FindBasePos( const SwFrame *pFrame, const Point &rPt )
{
    const SwFrame *pF = pFrame;
    while ( pF && !pF->getFrameArea().IsInside( rPt ) )
    {
        if ( pF->IsContentFrame() )
            pF = static_cast<const SwContentFrame*>(pF)->GetFollow();
        else
            pF = nullptr;
    }
    if ( pF )
        return pF->getFrameArea().Pos();
    else
        return pFrame->getFrameArea().Pos();
}

static bool lcl_SetAnchor( const SwPosition& rPos, const SwNode& rNd, SwFlyFrame const * pFly,
                const Point& rInsPt, SwFEShell const & rDestShell, SwFormatAnchor& rAnchor,
                Point& rNewPos, bool bCheckFlyRecur )
{
    bool bRet = true;
    rAnchor.SetAnchor( &rPos );
    std::pair<Point, bool> const tmp(rInsPt, false);
    SwContentFrame *const pTmpFrame = rNd.GetContentNode()->getLayoutFrame(
            rDestShell.GetLayout(), nullptr, &tmp);
    SwFlyFrame *pTmpFly = pTmpFrame->FindFlyFrame();
    if( pTmpFly && bCheckFlyRecur && pFly->IsUpperOf( *pTmpFly ) )
    {
        bRet = false;
    }
    else if ( RndStdIds::FLY_AT_FLY == rAnchor.GetAnchorId() )
    {
        if( pTmpFly )
        {
            const SwNodeIndex& rIdx = *pTmpFly->GetFormat()->GetContent().GetContentIdx();
            SwPosition aPos( rIdx );
            rAnchor.SetAnchor( &aPos );
            rNewPos = pTmpFly->getFrameArea().Pos();
        }
        else
        {
            rAnchor.SetType( RndStdIds::FLY_AT_PAGE );
            rAnchor.SetPageNum( rDestShell.GetPageNumber( rInsPt ) );
            const SwFrame *pPg = pTmpFrame->FindPageFrame();
            rNewPos = pPg->getFrameArea().Pos();
        }
    }
    else
        rNewPos = ::lcl_FindBasePos( pTmpFrame, rInsPt );
    return bRet;
}

bool SwFEShell::CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                    const Point& rInsPt, bool bIsMove, bool bSelectInsert )
{
    bool bRet = true;

    // The list should be copied, because below new objects will be selected
    const SdrMarkList aMrkList( Imp()->GetDrawView()->GetMarkedObjectList() );
    const size_t nMarkCount = aMrkList.GetMarkCount();
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
    for( size_t i = 0; i < nMarkCount; ++i )
    {
        SdrObject *pObj = aMrkList.GetMark( i )->GetMarkedSdrObj();

        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
        SwFrameFormat *pFormat = pContact->GetFormat();
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();

        bool bInsWithFormat = true;

        if( pDestDrwView->IsGroupEntered() )
        {
            // insert into the group, when it belongs to an entered group
            // or when the object is not anchored as a character
            if( pSrcDrwView->IsGroupEntered() ||
                (RndStdIds::FLY_AS_CHAR != rAnchor.GetAnchorId()) )

            {
                SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                        GetDoc() == pDestDoc, false );
                pNew->NbcMove( aSiz );
                pDestDrwView->InsertObjectAtView( pNew, *pDestPgView );
                bInsWithFormat = false;
            }
        }

        if( bInsWithFormat )
        {
            SwFormatAnchor aAnchor( rAnchor );
            Point aNewAnch;

            if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                (RndStdIds::FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
                (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
            {
                if ( this == pDestShell )
                {
                    // same shell? Then request the position
                    // from the passed DocumentPosition
                    SwPosition aPos( *GetCursor()->GetPoint() );
                    Point aPt( rInsPt );
                    aPt -= rSttPt - pObj->GetSnapRect().TopLeft();
                    SwCursorMoveState aState( MV_SETONLYTEXT );
                    GetLayout()->GetCursorOfst( &aPos, aPt, &aState );
                    const SwNode *pNd;
                    if( (pNd = &aPos.nNode.GetNode())->IsNoTextNode() )
                        bRet = false;
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, nullptr, rInsPt,
                                *pDestShell, aAnchor, aNewAnch, false );
                }
                else
                {
                    SwPaM *pCursor = pDestShell->GetCursor();
                    if( pCursor->GetNode().IsNoTextNode() )
                        bRet = false;
                    else
                        bRet = ::lcl_SetAnchor( *pCursor->GetPoint(),
                                                pCursor->GetNode(), nullptr, rInsPt,
                                                *pDestShell, aAnchor,
                                                aNewAnch, false );
                }
            }
            else if ( RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId() )
            {
                aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
                const SwRootFrame* pTmpRoot = pDestShell->GetLayout();
                const SwFrame* pPg = pTmpRoot->GetPageAtPos( rInsPt, nullptr, true );
                if ( pPg )
                    aNewAnch = pPg->getFrameArea().Pos();
            }

            if( bRet )
            {
                if( pSrcDrwView->IsGroupEntered() ||
                    ( !pObj->GetUserCall() && pObj->getParentSdrObjectFromSdrObject()) )
                {
                    SfxItemSet aSet( pDestDoc->GetAttrPool(),aFrameFormatSetRange);
                    aSet.Put( aAnchor );
                    SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                                GetDoc() == pDestDoc );
                    pFormat = pDestDoc->getIDocumentContentOperations().InsertDrawObj( *pDestShell->GetCursor(), *pNew, aSet );
                }
                else
                    pFormat = pDestDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, true, true );

                // Can be 0, as Draws are not allowed in Headers/Footers
                if ( pFormat )
                {
                    // #tdf33692 - drawing object has to be made visible on ctrl+drag copy.
                    pFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREPPASTING));
                    SdrObject* pNew = pFormat->FindSdrObject();
                    if ( RndStdIds::FLY_AS_CHAR != aAnchor.GetAnchorId() )
                    {
                        Point aPos( rInsPt );
                        aPos -= aNewAnch;
                        aPos -= rSttPt - pObj->GetSnapRect().TopLeft();
                        // OD 2004-04-05 #i26791# - change attributes instead of
                        // direct positioning
                        pFormat->SetFormatAttr( SwFormatHoriOrient( aPos.getX(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
                        pFormat->SetFormatAttr( SwFormatVertOrient( aPos.getY(), text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
                        // #i47455# - notify draw frame format
                        // that position attributes are already set.
                        if ( dynamic_cast<const SwDrawFrameFormat*>( pFormat) !=  nullptr )
                        {
                            static_cast<SwDrawFrameFormat*>(pFormat)->PosAttrSet();
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

            for ( size_t i = 0, nMrkCnt = aMrkList.GetMarkCount(); i < nMrkCnt; ++i )
            {
                SdrObject *pObj = aMrkList.GetMark( i )->GetMarkedSdrObj();
                pSrcDrwView->MarkObj( pObj, pSrcPgView );
            }
            DelSelectedObj();
            for ( size_t i = 0, nMrkCnt = aList.GetMarkCount(); i < nMrkCnt; ++i )
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

bool SwFEShell::Copy( SwFEShell* pDestShell, const Point& rSttPt,
                    const Point& rInsPt, bool bIsMove, bool bSelectInsert )
{
    bool bRet = false;

    OSL_ENSURE( pDestShell, "Copy without DestShell." );
    OSL_ENSURE( this == pDestShell || !pDestShell->IsObjSelected(),
            "Dest-Shell cannot be in Obj-Mode" );

    SET_CURR_SHELL( pDestShell );

    pDestShell->StartAllAction();
    pDestShell->GetDoc()->getIDocumentFieldsAccess().LockExpFields();

    // Shift references
    bool bCopyIsMove = mxDoc->IsCopyIsMove();
    if( bIsMove )
        // set a flag in Doc, handled in TextNodes
        mxDoc->SetCopyIsMove( true );

    RedlineFlags eOldRedlMode = pDestShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
    pDestShell->GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOldRedlMode | RedlineFlags::DeleteRedlines );

    // If there are table formulas in the area, then display the table first
    // so that the table formula can calculate a new value first
    // (individual boxes in the area are retrieved via the layout)
    SwFieldType* pTableFieldTyp = pDestShell->GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Table );

    if( IsFrameSelected() )
    {
        SwFlyFrame* pFly = GetSelectedFlyFrame();
        SwFrameFormat* pFlyFormat = pFly->GetFormat();
        SwFormatAnchor aAnchor( pFlyFormat->GetAnchor() );
        bRet = true;
        Point aNewAnch;

        if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AT_FLY  == aAnchor.GetAnchorId()) ||
            (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
        {
            if ( this == pDestShell )
            {
                // same shell? Then request the position
                // from the passed DocumentPosition
                SwPosition aPos( *GetCursor()->GetPoint() );
                Point aPt( rInsPt );
                aPt -= rSttPt - pFly->getFrameArea().Pos();
                SwCursorMoveState aState( MV_SETONLYTEXT );
                GetLayout()->GetCursorOfst( &aPos, aPt, &aState );
                const SwNode *pNd;
                if( (pNd = &aPos.nNode.GetNode())->IsNoTextNode() )
                    bRet = false;
                else
                {
                    // do not copy in itself
                    const SwNodeIndex *pTmp = pFlyFormat->GetContent().GetContentIdx();
                    if ( aPos.nNode > *pTmp && aPos.nNode <
                        pTmp->GetNode().EndOfSectionIndex() )
                    {
                        bRet = false;
                    }
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, pFly, rInsPt,
                                        *pDestShell, aAnchor, aNewAnch, true );
                }
            }
            else
            {
                const SwPaM *pCursor = pDestShell->GetCursor();
                if( pCursor->GetNode().IsNoTextNode() )
                    bRet = false;
                else
                    bRet = ::lcl_SetAnchor( *pCursor->GetPoint(), pCursor->GetNode(),
                                            pFly, rInsPt, *pDestShell, aAnchor,
                                    aNewAnch, GetDoc() == pDestShell->GetDoc());
            }
        }
        else if ( RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId() )
        {
            aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
            const SwRootFrame* pTmpRoot = pDestShell->GetLayout();
            const SwFrame* pPg = pTmpRoot->GetPageAtPos( rInsPt, nullptr, true );
            if ( pPg )
                aNewAnch = pPg->getFrameArea().Pos();
        }
        else {
            OSL_ENSURE( false, "what anchor is it?" );
        }

        if( bRet )
        {
            SwFrameFormat *pOldFormat = pFlyFormat;
            pFlyFormat = pDestShell->GetDoc()->getIDocumentLayoutAccess().CopyLayoutFormat( *pFlyFormat, aAnchor, true, true );

            if ( RndStdIds::FLY_AS_CHAR != aAnchor.GetAnchorId() )
            {
                Point aPos( rInsPt );
                aPos -= aNewAnch;
                aPos -= rSttPt - pFly->getFrameArea().Pos();
                pFlyFormat->SetFormatAttr( SwFormatHoriOrient( aPos.getX(),text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
                pFlyFormat->SetFormatAttr( SwFormatVertOrient( aPos.getY(),text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
            }

            const Point aPt( pDestShell->GetCursorDocPos() );

            if( bIsMove )
                GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat( pOldFormat );

            // only select if it can be shifted/copied in the same shell
            if( bSelectInsert )
            {
                SwFlyFrame* pFlyFrame = static_cast<SwFlyFrameFormat*>(pFlyFormat)->GetFrame( &aPt );
                if( pFlyFrame )
                {
                    //JP 12.05.98: should this be in SelectFlyFrame???
                    pDestShell->Imp()->GetDrawView()->UnmarkAll();
                    pDestShell->SelectFlyFrame( *pFlyFrame );
                }
            }

            if (this != pDestShell && !pDestShell->HasShellFocus())
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
        SwSelBoxes aBoxes;
        GetTableSel( *this, aBoxes );
        SwTableNode const*const pTableNd(
            aBoxes.empty() ? nullptr : aBoxes[0]->GetSttNd()->FindTableNode());
        if (nullptr != pTableNd)
        {
            std::unique_ptr<SwPosition> pDstPos;
            if( this == pDestShell )
            {
                // same shell? Then create new Cursor at the
                // DocumentPosition passed
                pDstPos.reset(new SwPosition( *GetCursor()->GetPoint() ));
                Point aPt( rInsPt );
                GetLayout()->GetCursorOfst( pDstPos.get(), aPt );
                if( !pDstPos->nNode.GetNode().IsNoTextNode() )
                    bRet = true;
            }
            else if( !pDestShell->GetCursor()->GetNode().IsNoTextNode() )
            {
                pDstPos.reset(new SwPosition( *pDestShell->GetCursor()->GetPoint() ));
                bRet = true;
            }

            if( bRet )
            {
                if( GetDoc() == pDestShell->GetDoc() )
                    ParkTableCursor();

                bRet = pDestShell->GetDoc()->InsCopyOfTable( *pDstPos, aBoxes,nullptr,
                                        bIsMove && this == pDestShell &&
                                        aBoxes.size() == pTableNd->GetTable().
                                        GetTabSortBoxes().size(),
                                        this != pDestShell );

                if( this != pDestShell )
                    *pDestShell->GetCursor()->GetPoint() = *pDstPos;

                // create all parked Cursor?
                if( GetDoc() == pDestShell->GetDoc() )
                    GetCursor();

                // JP 16.04.99: Bug 64908 - Set InsPos, to assure the parked
                //              Cursor is positioned at the insert position
                if( this == pDestShell )
                    GetCursorDocPos() = rInsPt;
            }
        }
    }
    else
    {
        bRet = true;
        if( this == pDestShell )
        {
            // same shell? then request the position
            // at the passed document position
            SwPosition aPos( *GetCursor()->GetPoint() );
            Point aPt( rInsPt );
            GetLayout()->GetCursorOfst( &aPos, aPt );
            bRet = !aPos.nNode.GetNode().IsNoTextNode();
        }
        else if( pDestShell->GetCursor()->GetNode().IsNoTextNode() )
            bRet = false;

        if( bRet )
            bRet = SwEditShell::Copy( pDestShell );
    }

    pDestShell->GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOldRedlMode );
    mxDoc->SetCopyIsMove( bCopyIsMove );

    // have new table formulas been inserted?
    if( pTableFieldTyp->HasWriterListeners() )
    {
        // finish old actions: the table frames are created and
        // a selection can be made
        sal_uInt16 nActCnt;
        for( nActCnt = 0; pDestShell->ActionPend(); ++nActCnt )
            pDestShell->EndAllAction();

        for( ; nActCnt; --nActCnt )
            pDestShell->StartAllAction();
    }
    pDestShell->GetDoc()->getIDocumentFieldsAccess().UnlockExpFields();
    pDestShell->GetDoc()->getIDocumentFieldsAccess().UpdateFields(false);

    pDestShell->EndAllAction();
    return bRet;
}

// Paste for the internal clipboard. Copy the content of the clipboard
// in the document
namespace {
    typedef std::shared_ptr<SwPaM> PaMPtr;
    typedef std::shared_ptr<SwPosition> PositionPtr;
    typedef std::pair< PaMPtr, PositionPtr > Insertion;

    bool PamHasSelection(const SwPaM& rPaM)
    {
        return rPaM.HasMark() && *rPaM.GetPoint() != *rPaM.GetMark();
    }
}

bool SwFEShell::Paste( SwDoc* pClpDoc )
{
    SET_CURR_SHELL( this );
    OSL_ENSURE( pClpDoc, "no clipboard document"  );
    // then till end of the nodes array
    SwNodeIndex aIdx( pClpDoc->GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aIdx ); //DocStart

    // If there are table formulas in the area, then display the table first
    // so that the table formula can calculate a new value first
    // (individual boxes in the area are retrieved via the layout)
    SwFieldType* pTableFieldTyp = GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Table );

    SwTableNode *const pSrcNd = aCpyPam.GetNode().GetTableNode();
    if( !pSrcNd )                               // table node ?
    {                                           // don't skip !!
        SwContentNode* pCNd = aCpyPam.GetNode().GetContentNode();
        if( pCNd )
            aCpyPam.GetPoint()->nContent.Assign( pCNd, 0 );
        else if( !aCpyPam.Move( fnMoveForward, GoInNode ))
            aCpyPam.Move( fnMoveBackward, GoInNode );
    }

    aCpyPam.SetMark();
    aCpyPam.Move( fnMoveForward, GoInDoc );

    bool bRet = true;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSGLOSSARY, nullptr );
    GetDoc()->getIDocumentFieldsAccess().LockExpFields();

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
        std::vector< Insertion > aCopyVector;
        // The number of text portions of the rectangular selection
        const sal_uInt32 nSelCount = aCpyPam.GetPoint()->nNode.GetIndex()
                       - aCpyPam.GetMark()->nNode.GetIndex();
        sal_uInt32 nCount = nSelCount;
        SwNodeIndex aClpIdx( aIdx );
        SwPaM* pStartCursor = GetCursor();
        SwPaM* pCurrCursor = pStartCursor;
        sal_uInt32 nCursorCount = pStartCursor->GetRingContainer().size();
        // If the target selection is a multi-selection, often the last and first
        // cursor of the ring points to identical document positions. Then
        // we should avoid double insertion of text portions...
        while( nCursorCount > 1 && *pCurrCursor->GetPoint() ==
            *(pCurrCursor->GetPrev()->GetPoint()) )
        {
            --nCursorCount;
            pCurrCursor = pCurrCursor->GetNext();
            pStartCursor = pCurrCursor;
        }
        SwPosition aStartPos( *pStartCursor->GetPoint() );
        SwPosition aInsertPos( aStartPos ); // first insertion position
        bool bCompletePara = false;
        sal_uInt16 nMove = 0;
        while( nCount )
        {
            --nCount;
            OSL_ENSURE( aIdx.GetNode().GetContentNode(), "Who filled the clipboard?!" );
            if( aIdx.GetNode().GetContentNode() ) // robust
            {
                Insertion aInsertion( std::make_shared<SwPaM>( aIdx ),
                    std::make_shared<SwPosition>( aInsertPos ) );
                ++aIdx;
                aInsertion.first->SetMark();
                if( pStartCursor == pCurrCursor->GetNext() )
                {   // Now we have to look for insertion positions...
                    if( !nMove ) // Annotate the last given insert position
                        aStartPos = aInsertPos;
                    SwCursor aCursor( aStartPos, nullptr);
                    // Check if we find another insert position by moving
                    // down the last given position
                    if (aCursor.UpDown(false, ++nMove, nullptr, 0, *GetLayout()))
                        aInsertPos = *aCursor.GetPoint();
                    else // if there is no paragraph we have to create it
                        bCompletePara = nCount > 0;
                    nCursorCount = 0;
                }
                else // as long as we find more insert positions in the cursor ring
                {    // we'll take them
                    pCurrCursor = pCurrCursor->GetNext();
                    aInsertPos = *pCurrCursor->GetPoint();
                    --nCursorCount;
                }
                // If there are no more paragraphs e.g. at the end of a document,
                // we insert complete paragraphs instead of text portions
                if( bCompletePara )
                    aInsertion.first->GetPoint()->nNode = aIdx;
                else
                    aInsertion.first->GetPoint()->nContent =
                        aInsertion.first->GetContentNode()->Len();
                aCopyVector.push_back( aInsertion );
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
        for (auto const& item : aCopyVector)
        {
            SwPosition& rInsPos = *item.second;
            SwPaM& rCopy = *item.first;
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
                --aIndexBefore;
                pClpDoc->getIDocumentContentOperations().CopyRange( rCopy, rInsPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
                {
                    ++aIndexBefore;
                    SwPaM aPaM(SwPosition(aIndexBefore),
                               SwPosition(rInsPos.nNode));
                    aPaM.GetDoc()->MakeUniqueNumRules(aPaM);
                }
            }
            SaveTableBoxContent( &rInsPos );
        }
    }
    else
    {
        bool bDelTable = true;

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {

            SwTableNode *const pDestNd(GetDoc()->IsIdxInTable(rPaM.GetPoint()->nNode));
            if (pSrcNd && nullptr != pDestNd &&
                // are we at the beginning of the cell? (if not, we will insert a nested table)
                // first paragraph of the cell?
                rPaM.GetNode().GetIndex() == rPaM.GetNode().FindTableBoxStartNode()->GetIndex()+1 &&
                // beginning of the paragraph?
                !rPaM.GetPoint()->nContent.GetIndex())
            {
                SwPosition aDestPos( *rPaM.GetPoint() );

                bool bParkTableCursor = false;
                const SwStartNode* pSttNd =  rPaM.GetNode().FindTableBoxStartNode();

                // TABLE IN TABLE: copy table in table
                // search boxes via the layout
                SwSelBoxes aBoxes;
                if( IsTableMode() )     // table selection?
                {
                    GetTableSel( *this, aBoxes );
                    ParkTableCursor();
                    bParkTableCursor = true;
                }
                else if( !PamHasSelection(rPaM) && rPaM.GetNext() == &rPaM &&
                     ( !pSrcNd->GetTable().IsTableComplex() ||
                       pDestNd->GetTable().IsNewModel() ) )
                {
                    // make relative table copy
                    SwTableBox* pBox = pDestNd->GetTable().GetTableBox(
                                        pSttNd->GetIndex() );
                    OSL_ENSURE( pBox, "Box is not in this table" );
                    aBoxes.insert( pBox );
                }

                SwNodeIndex aNdIdx( *pDestNd->EndOfSectionNode());
                if( !bParkTableCursor )
                {
                    // exit first the complete table
                    // ???? what about only table in a frame ?????
                    SwContentNode* pCNd = GetDoc()->GetNodes().GoNext( &aNdIdx );
                    SwPosition aPos( aNdIdx, SwIndex( pCNd, 0 ));
                    // #i59539: Don't remove all redline
                    SwPaM const tmpPaM(*pDestNd, *pDestNd->EndOfSectionNode());
                    ::PaMCorrAbs(tmpPaM, aPos);
                }

                bRet = GetDoc()->InsCopyOfTable( aDestPos, aBoxes, &pSrcNd->GetTable() );

                if( bParkTableCursor )
                    GetCursor();
                else
                {
                    // return to the box
                    aNdIdx = *pSttNd;
                    SwContentNode* pCNd = GetDoc()->GetNodes().GoNext( &aNdIdx );
                    SwPosition aPos( aNdIdx, SwIndex( pCNd, 0 ));
                    // #i59539: Don't remove all redline
                    SwNode & rNode(rPaM.GetPoint()->nNode.GetNode());
                    SwContentNode *const pContentNode( rNode.GetContentNode() );
                    SwPaM const tmpPam(rNode, 0,
                                   rNode, pContentNode ? pContentNode->Len() : 0);
                    ::PaMCorrAbs(tmpPam, aPos);
                }

                break;      // exit the "while-loop"
            }
            else if( *aCpyPam.GetPoint() == *aCpyPam.GetMark() &&
                 !pClpDoc->GetSpzFrameFormats()->empty() )
            {
                // we need a DrawView
                if( !Imp()->GetDrawView() )
                    MakeDrawView();

                for ( auto pCpyFormat : *pClpDoc->GetSpzFrameFormats() )
                {
                    bool bInsWithFormat = true;

                    if( Imp()->GetDrawView()->IsGroupEntered() &&
                        RES_DRAWFRMFMT == pCpyFormat->Which() &&
                        (RndStdIds::FLY_AS_CHAR != pCpyFormat->GetAnchor().GetAnchorId()) )
                    {
                        const SdrObject* pSdrObj = pCpyFormat->FindSdrObject();
                        if( pSdrObj )
                        {
                            SdrObject* pNew = GetDoc()->CloneSdrObj( *pSdrObj,
                                                            false, false );

                            // Insert object sets any anchor position to 0.
                            // Therefore we calculate the absolute position here
                            // and after the insert the anchor of the object
                            // is set to the anchor of the group object.
                            tools::Rectangle aSnapRect = pNew->GetSnapRect();
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
                            SdrObjList* pList = pNew->getParentSdrObjListFromSdrObject();
                            if ( pList )
                            {
                                SdrObjGroup* pOwner(dynamic_cast< SdrObjGroup* >(pList->getSdrObjectFromSdrObjList()));

                                if(nullptr != pOwner)
                                {
                                    aGrpAnchor = pOwner->GetAnchorPos();
                                }
                            }

                            // OD 2004-04-05 #i26791# - direct drawing object
                            // positioning for group members
                            pNew->NbcSetAnchorPos( aGrpAnchor );
                            pNew->SetSnapRect( aSnapRect );

                            bInsWithFormat = false;
                        }
                    }

                    if( bInsWithFormat  )
                    {
                        SwFormatAnchor aAnchor( pCpyFormat->GetAnchor() );
                        if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
                            (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
                            (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
                        {
                            SwPosition* pPos = rPaM.GetPoint();
                            // allow shapes (no controls) in header/footer
                            if( RES_DRAWFRMFMT == pCpyFormat->Which() &&
                                GetDoc()->IsInHeaderFooter( pPos->nNode ) )
                            {
                                const SdrObject *pCpyObj = pCpyFormat->FindSdrObject();
                                if (pCpyObj && CheckControlLayer(pCpyObj))
                                    continue;
                            }

                            // Ignore TextBoxes, they are already handled in sw::DocumentLayoutManager::CopyLayoutFormat().
                            if (SwTextBoxHelper::isTextBox(pCpyFormat, RES_FLYFRMFMT))
                                continue;

                            aAnchor.SetAnchor( pPos );
                        }
                        else if ( RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId() )
                        {
                            aAnchor.SetPageNum( GetPhyPageNum() );
                        }
                        else if( RndStdIds::FLY_AT_FLY == aAnchor.GetAnchorId() )
                        {
                            Point aPt;
                            (void)lcl_SetAnchor( *rPaM.GetPoint(), rPaM.GetNode(),
                                        nullptr, aPt, *this, aAnchor, aPt, false );
                        }

                        SwFrameFormat * pNew = GetDoc()->getIDocumentLayoutAccess().CopyLayoutFormat( *pCpyFormat, aAnchor, true, true );

                        if( pNew )
                        {
                            if( RES_FLYFRMFMT == pNew->Which() )
                            {
                                const Point aPt( GetCursorDocPos() );
                                SwFlyFrame* pFlyFrame = static_cast<SwFlyFrameFormat*>(pNew)->
                                                        GetFrame( &aPt );
                                if( pFlyFrame )
                                    SelectFlyFrame( *pFlyFrame );
                                // always pick the first FlyFrame only; the others
                                // were copied to the clipboard via Fly in Fly
                                break;
                            }
                            else
                            {
                                OSL_ENSURE( RES_DRAWFRMFMT == pNew->Which(), "New format.");
                                // #i52780# - drawing object has to be made visible on paste.
                                pNew->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREPPASTING));
                                SdrObject *pObj = pNew->FindSdrObject();
                                SwDrawView  *pDV = Imp()->GetDrawView();
                                pDV->MarkObj( pObj, pDV->GetSdrPageView() );
                                // #i47455# - notify draw frame format
                                // that position attributes are already set.
                                if ( dynamic_cast<const SwDrawFrameFormat*>( pNew) !=  nullptr )
                                {
                                    static_cast<SwDrawFrameFormat*>(pNew)->PosAttrSet();
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if( bDelTable && IsTableMode() )
                {
                    SwEditShell::Delete();
                    bDelTable = false;
                }

                SwPosition& rInsPos = *rPaM.GetPoint();
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

                // **
                // ** Update SwDoc::Append, if you change the following code **
                // **
                {
                    SwNodeIndex aIndexBefore(rInsPos.nNode);

                    --aIndexBefore;

                    pClpDoc->getIDocumentContentOperations().CopyRange( aCpyPam, rInsPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
                    // Note: aCpyPam is invalid now

                    ++aIndexBefore;
                    SwPaM aPaM(SwPosition(aIndexBefore),
                           SwPosition(rInsPos.nNode));

                    aPaM.GetDoc()->MakeUniqueNumRules(aPaM);

                    // Update the rsid of each pasted text node.
                    SwNodes &rDestNodes = GetDoc()->GetNodes();
                    sal_uLong const nEndIdx = aPaM.End()->nNode.GetIndex();

                    for (sal_uLong nIdx = aPaM.Start()->nNode.GetIndex();
                        nIdx <= nEndIdx; ++nIdx)
                    {
                        SwTextNode *const pTextNode = rDestNodes[nIdx]->GetTextNode();
                        if ( pTextNode )
                        {
                            GetDoc()->UpdateParRsid( pTextNode );
                        }
                    }
                }

                SaveTableBoxContent( &rInsPos );
            }
        }
    }

    GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSGLOSSARY, nullptr );

    // have new table formulas been inserted?
    if( pTableFieldTyp->HasWriterListeners() )
    {
        // finish old action: table-frames have been created
        // a selection can be made now
        sal_uInt16 nActCnt;
        for( nActCnt = 0; ActionPend(); ++nActCnt )
            EndAllAction();

        for( ; nActCnt; --nActCnt )
            StartAllAction();
    }
    GetDoc()->getIDocumentFieldsAccess().UnlockExpFields();
    GetDoc()->getIDocumentFieldsAccess().UpdateFields(false);
    EndAllAction();

    return bRet;
}

void SwFEShell::PastePages( SwFEShell& rToFill, sal_uInt16 nStartPage, sal_uInt16 nEndPage)
{
    Push();
    if(!GotoPage(nStartPage))
    {
        Pop(PopMode::DeleteCurrent);
        return;
    }
    MovePage( GetThisFrame, GetFirstSub );
    SwPaM aCpyPam( *GetCursor()->GetPoint() );
    OUString sStartingPageDesc = GetPageDesc( GetCurPageDesc()).GetName();
    SwPageDesc* pDesc = rToFill.FindPageDescByName( sStartingPageDesc, true );
    if( pDesc )
        rToFill.ChgCurPageDesc( *pDesc );

    if(!GotoPage(nEndPage))
    {
        Pop(PopMode::DeleteCurrent);
        return;
    }
    //if the page starts with a table a paragraph has to be inserted before
    SwNode* pTableNode = aCpyPam.GetNode().FindTableNode();
    if(pTableNode)
    {
        //insert a paragraph
        StartUndo(SwUndoId::INSERT);
        SwNodeIndex aTableIdx(  *pTableNode, -1 );
        SwPosition aBefore(aTableIdx);
        if(GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore ))
        {
            SwPaM aTmp(aBefore);
            aCpyPam = aTmp;
        }
        EndUndo(SwUndoId::INSERT);
    }

    MovePage( GetThisFrame, GetLastSub );
    aCpyPam.SetMark();
    *aCpyPam.GetMark() = *GetCursor()->GetPoint();

    SET_CURR_SHELL( this );

    StartAllAction();
    GetDoc()->getIDocumentFieldsAccess().LockExpFields();
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
        rToFill.GetDoc()->getIDocumentContentOperations().DelFullPara(aPara);
    }
    // now the page bound objects
    // additionally copy page bound frames
    if( !GetDoc()->GetSpzFrameFormats()->empty() )
    {
        // create a draw view if necessary
        if( !rToFill.Imp()->GetDrawView() )
            rToFill.MakeDrawView();

        for ( auto pCpyFormat : *GetDoc()->GetSpzFrameFormats() )
        {
            SwFormatAnchor aAnchor( pCpyFormat->GetAnchor() );
            if ((RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId()) &&
                    aAnchor.GetPageNum() >= nStartPage && aAnchor.GetPageNum() <= nEndPage)
            {
                aAnchor.SetPageNum( aAnchor.GetPageNum() - nStartPage + 1);
            }
            else
                continue;
            rToFill.GetDoc()->getIDocumentLayoutAccess().CopyLayoutFormat( *pCpyFormat, aAnchor, true, true );
        }
    }
    GetDoc()->getIDocumentFieldsAccess().UnlockExpFields();
    GetDoc()->getIDocumentFieldsAccess().UpdateFields(false);
    Pop(PopMode::DeleteCurrent);
    EndAllAction();
}

comphelper::OInterfaceContainerHelper2& SwFEShell::GetPasteListeners() { return m_aPasteListeners; }

bool SwFEShell::GetDrawObjGraphic( SotClipboardFormatId nFormat, Graphic& rGrf ) const
{
    OSL_ENSURE( Imp()->HasDrawView(), "GetDrawObjGraphic without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    bool bConvert = true;
    if( rMrkList.GetMarkCount() )
    {
        if( rMrkList.GetMarkCount() == 1 &&
            dynamic_cast< const SwVirtFlyDrawObj* >(rMrkList.GetMark( 0 )->GetMarkedSdrObj()) != nullptr )
        {
            // select frame
            if( CNT_GRF == GetCntType() )
            {
                const Graphic* pGrf( GetGraphic() );
                if ( pGrf )
                {
                    Graphic aGrf( *pGrf );
                    if( SotClipboardFormatId::GDIMETAFILE == nFormat )
                    {
                        if( GraphicType::Bitmap != aGrf.GetType() )
                        {
                            rGrf = aGrf;
                            bConvert = false;
                        }
                        else if( GetWin() )
                        {
                            Size aSz;
                            Point aPt;
                            GetGrfSize( aSz );

                            ScopedVclPtrInstance< VirtualDevice > pVirtDev;
                            pVirtDev->EnableOutput( false );

                            MapMode aTmp( GetWin()->GetMapMode() );
                            aTmp.SetOrigin( aPt );
                            pVirtDev->SetMapMode( aTmp );

                            GDIMetaFile aMtf;
                            aMtf.Record( pVirtDev.get() );
                            aGrf.Draw( pVirtDev, aPt, aSz );
                            aMtf.Stop();
                            aMtf.SetPrefMapMode( aTmp );
                            aMtf.SetPrefSize( aSz );
                            rGrf = aMtf;
                        }
                    }
                    else if( GraphicType::Bitmap == aGrf.GetType() )
                    {
                        rGrf = aGrf;
                        bConvert = false;
                    }
                    else
                    {
                        // Not the original size, but the current one.
                        // Otherwise it could happen that for vector graphics
                        // many MB's of memory are allocated.
                        const Size aSz( GetSelectedFlyFrame()->getFramePrintArea().SSize() );
                        ScopedVclPtrInstance< VirtualDevice > pVirtDev(*GetWin());

                        MapMode aTmp( MapUnit::MapTwip );
                        pVirtDev->SetMapMode( aTmp );
                        if( pVirtDev->SetOutputSize( aSz ) )
                        {
                            aGrf.Draw( pVirtDev.get(), Point(), aSz );
                            rGrf = pVirtDev->GetBitmapEx( Point(), aSz );
                        }
                        else
                        {
                            rGrf = aGrf;
                            bConvert = false;
                        }
                    }
                }
            }
        }
        else if( SotClipboardFormatId::GDIMETAFILE == nFormat )
            rGrf = Imp()->GetDrawView()->GetMarkedObjMetaFile();
        else if( SotClipboardFormatId::BITMAP == nFormat || SotClipboardFormatId::PNG == nFormat )
            rGrf = Imp()->GetDrawView()->GetMarkedObjBitmapEx();
    }
    return bConvert;
}

// #i50824#
// replace method <lcl_RemoveOleObjsFromSdrModel> by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
static void lcl_ConvertSdrOle2ObjsToSdrGrafObjs( SdrModel& _rModel )
{
    for ( sal_uInt16 nPgNum = 0; nPgNum < _rModel.GetPageCount(); ++nPgNum )
    {
        // setup object iterator in order to iterate through all objects
        // including objects in group objects, but exclusive group objects.
        SdrObjListIter aIter(_rModel.GetPage(nPgNum));
        while( aIter.IsMore() )
        {
            SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( aIter.Next() );
            if( pOle2Obj )
            {
                // found an ole2 shape
                SdrObjList* pObjList = pOle2Obj->getParentSdrObjListFromSdrObject();

                // get its graphic
                Graphic aGraphic;
                pOle2Obj->Connect();
                const Graphic* pGraphic = pOle2Obj->GetGraphic();
                if( pGraphic )
                    aGraphic = *pGraphic;
                pOle2Obj->Disconnect();

                // create new graphic shape with the ole graphic and shape size
                SdrGrafObj* pGraphicObj = new SdrGrafObj(
                    _rModel,
                    aGraphic,
                    pOle2Obj->GetCurrentBoundRect());
                // apply layer of ole2 shape at graphic shape
                pGraphicObj->SetLayer( pOle2Obj->GetLayer() );

                // replace ole2 shape with the new graphic object and delete the ol2 shape
                SdrObject* pRemovedObject = pObjList->ReplaceObject( pGraphicObj, pOle2Obj->GetOrdNum() );
                SdrObject::Free( pRemovedObject );
            }
        }
    }
}

void SwFEShell::Paste( SvStream& rStrm, SwPasteSdr nAction, const Point* pPt )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo();

    std::unique_ptr< FmFormModel > pModel(
        new FmFormModel(
            nullptr,
            GetDoc()->GetDocShell()));

    pModel->GetItemPool().FreezeIdRanges();

    rStrm.Seek(0);

    uno::Reference< io::XInputStream > xInputStream( new utl::OInputStreamWrapper( rStrm ) );
    SvxDrawingLayerImport( pModel.get(), xInputStream );

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

        if( SwPasteSdr::SetAttr == nAction && dynamic_cast<const SwVirtFlyDrawObj*>( pOldObj) !=  nullptr )
            nAction = SwPasteSdr::Replace;

        switch( nAction )
        {
        case SwPasteSdr::Replace:
            {
                const SwFrameFormat* pFormat(nullptr);
                const SwFrame* pAnchor(nullptr);
                if( dynamic_cast<const SwVirtFlyDrawObj*>( pOldObj) !=  nullptr )
                {
                    pFormat = FindFrameFormat( pOldObj );

                    Point aNullPt;
                    SwFlyFrame* pFlyFrame = static_cast<const SwFlyFrameFormat*>(pFormat)->GetFrame( &aNullPt );
                    pAnchor = pFlyFrame ? pFlyFrame->GetAnchorFrame() : nullptr;

                    if (!pAnchor || pAnchor->FindFooterOrHeader())
                    {
                        // if there is a textframe in the header/footer:
                        // do not replace but insert
                        nAction = SwPasteSdr::Insert;
                        break;
                    }
                }

                SdrObject* pNewObj(pClpObj->CloneSdrObject(pOldObj->getSdrModelFromSdrObject()));
                tools::Rectangle aOldObjRect( pOldObj->GetCurrentBoundRect() );
                Size aOldObjSize( aOldObjRect.GetSize() );
                tools::Rectangle aNewRect( pNewObj->GetCurrentBoundRect() );
                Size aNewSize( aNewRect.GetSize() );

                Fraction aScaleWidth( aOldObjSize.Width(), aNewSize.Width() );
                Fraction aScaleHeight( aOldObjSize.Height(), aNewSize.Height());
                pNewObj->NbcResize( aNewRect.TopLeft(), aScaleWidth, aScaleHeight);

                Point aVec = aOldObjRect.TopLeft() - aNewRect.TopLeft();
                pNewObj->NbcMove(Size(aVec.getX(), aVec.getY()));

                if( dynamic_cast<const SdrUnoObj*>( pNewObj) !=  nullptr )
                    pNewObj->SetLayer( GetDoc()->getIDocumentDrawModelAccess().GetControlsId() );
                else if( dynamic_cast<const SdrUnoObj*>( pOldObj) !=  nullptr )
                    pNewObj->SetLayer( GetDoc()->getIDocumentDrawModelAccess().GetHeavenId() );
                else
                    pNewObj->SetLayer( pOldObj->GetLayer() );

                if( dynamic_cast<const SwVirtFlyDrawObj*>( pOldObj) !=  nullptr )
                {
                    // store attributes, then set SdrObject
                    SfxItemSet aFrameSet( mxDoc->GetAttrPool(),
                                            svl::Items<RES_SURROUND, RES_ANCHOR>{} );
                    aFrameSet.Set( pFormat->GetAttrSet() );

                    Point aNullPt;
                    if( pAnchor->IsTextFrame() && static_cast<const SwTextFrame*>(pAnchor)->IsFollow() )
                    {
                        const SwTextFrame* pTmp = static_cast<const SwTextFrame*>(pAnchor);
                        do {
                            pTmp = pTmp->FindMaster();
                            OSL_ENSURE( pTmp, "Where's my Master?" );
                        } while( pTmp->IsFollow() );
                        pAnchor = pTmp;
                    }
                    if( dynamic_cast<const SdrCaptionObj*>( pOldObj) !=  nullptr)
                        aNullPt = static_cast<SdrCaptionObj*>(pOldObj)->GetTailPos();
                    else
                        aNullPt = aOldObjRect.TopLeft();

                    Point aNewAnchor = pAnchor->GetFrameAnchorPos( ::HasWrap( pOldObj ) );
                    // OD 2004-04-05 #i26791# - direct positioning of Writer
                    // fly frame object for <SwDoc::Insert(..)>
                    pNewObj->NbcSetRelativePos( aNullPt - aNewAnchor );
                    pNewObj->NbcSetAnchorPos( aNewAnchor );

                    pOldObj->GetOrdNum();

                    DelSelectedObj();

                    GetDoc()->getIDocumentContentOperations().InsertDrawObj( *GetCursor(), *pNewObj, aFrameSet );
                }
                else
                {
                    // #i123922#  for handling MasterObject and virtual ones correctly, SW
                    // wants us to call ReplaceObject at the page, but that also
                    // triggers the same assertion (I tried it), so stay at the view method
                    pView->ReplaceObjectAtView(pOldObj, *Imp()->GetPageView(), pNewObj);
                }
            }
            break;

        case SwPasteSdr::SetAttr:
            {
                SfxItemSet aSet( GetAttrPool() );
                const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pClpObj);

                if(pSdrGrafObj)
                {
                    SdrObject* pTarget = nullptr;

                    if(0 != pView->GetMarkedObjectList().GetMarkCount())
                    {
                        // try to get target (if it's at least one, take first)
                        SdrMark* pMark = pView->GetMarkedObjectList().GetMark(0);

                        if(pMark)
                        {
                            pTarget = pMark->GetMarkedSdrObj();
                        }
                    }

                    if(pTarget)
                    {
                        // copy ItemSet from target
                        aSet.Set(pTarget->GetMergedItemSet());
                    }

                    // for SdrGrafObj, use the graphic as fill style argument
                    const Graphic& rGraphic = pSdrGrafObj->GetGraphic();

                    if(GraphicType::NONE != rGraphic.GetType() && GraphicType::Default != rGraphic.GetType())
                    {
                        aSet.Put(XFillBitmapItem(OUString(), rGraphic));
                        aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                    }
                }
                else
                {
                    aSet.Put(pClpObj->GetMergedItemSet());
                }

                pView->SetAttributes( aSet );
            }
            break;

        default:
            nAction = SwPasteSdr::Insert;
            break;
        }
    }
    else
        nAction = SwPasteSdr::Insert;

    if( SwPasteSdr::Insert == nAction )
    {
        ::sw::DrawUndoGuard drawUndoGuard(GetDoc()->GetIDocumentUndoRedo());

        bool bDesignMode = pView->IsDesignMode();
        if( !bDesignMode )
            pView->SetDesignMode();

        // #i50824#
        // method <lcl_RemoveOleObjsFromSdrModel> replaced by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
        lcl_ConvertSdrOle2ObjsToSdrGrafObjs(*pModel);
        pView->Paste(*pModel, aPos, nullptr, SdrInsertFlags::NONE);

        const size_t nCnt = pView->GetMarkedObjectList().GetMarkCount();
        if( nCnt )
        {
            const Point aNull( 0, 0 );
            for( size_t i=0; i < nCnt; ++i )
            {
                SdrObject *pObj = pView->GetMarkedObjectList().GetMark(i)->GetMarkedSdrObj();
                pObj->ImpSetAnchorPos( aNull );
            }

            pView->SetCurrentObj( OBJ_GRUP );
            if ( nCnt > 1 )
                pView->GroupMarked();
            SdrObject *pObj = pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            if( dynamic_cast<const SdrUnoObj*>( pObj) !=  nullptr )
            {
                pObj->SetLayer( GetDoc()->getIDocumentDrawModelAccess().GetControlsId() );
                bDesignMode = true;
            }
            else
                pObj->SetLayer( GetDoc()->getIDocumentDrawModelAccess().GetHeavenId() );
            const tools::Rectangle &rSnap = pObj->GetSnapRect();
            const Size aDiff( rSnap.GetWidth()/2, rSnap.GetHeight()/2 );
            pView->MoveMarkedObj( aDiff );
            ImpEndCreate();
            if( !bDesignMode )
                pView->SetDesignMode( false );
        }
    }
    EndUndo();
    EndAllAction();
}

bool SwFEShell::Paste(const Graphic &rGrf, const OUString& rURL)
{
    SET_CURR_SHELL( this );
    SdrObject* pObj = nullptr;
    SdrView *pView = Imp()->GetDrawView();

    bool bRet = 1 == pView->GetMarkedObjectList().GetMarkCount();
    if (bRet)
    {
        pObj = pView->GetMarkedObjectList().GetMark( 0 )->GetMarkedSdrObj();
        bRet = pObj->IsClosedObj() && dynamic_cast<const SdrOle2Obj*>( pObj) == nullptr;
    }

    if( bRet && pObj )
    {
        // #i123922# added code to handle the two cases of SdrGrafObj and a fillable, non-
        // OLE object in focus
        SdrObject* pResult = pObj;

        if(dynamic_cast< SdrGrafObj* >(pObj))
        {
            SdrGrafObj* pNewGrafObj(static_cast<SdrGrafObj*>(pObj->CloneSdrObject(pObj->getSdrModelFromSdrObject())));

            pNewGrafObj->SetGraphic(rGrf);

            // #i123922#  for handling MasterObject and virtual ones correctly, SW
            // wants us to call ReplaceObject at the page, but that also
            // triggers the same assertion (I tried it), so stay at the view method
            pView->ReplaceObjectAtView(pObj, *pView->GetSdrPageView(), pNewGrafObj);

            OUString aReferer;
            SwDocShell *pDocShell = GetDoc()->GetDocShell();
            if (pDocShell->HasName()) {
                aReferer = pDocShell->GetMedium()->GetName();
            }

            // set in all cases - the Clone() will have copied an existing link (!)
            pNewGrafObj->SetGraphicLink(rURL, aReferer, OUString());

            pResult = pNewGrafObj;
        }
        else
        {
            pView->AddUndo(std::make_unique<SdrUndoAttrObj>(*pObj));

            SfxItemSet aSet(pView->GetModel()->GetItemPool(), svl::Items<XATTR_FILLSTYLE, XATTR_FILLBITMAP>{});

            aSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
            aSet.Put(XFillBitmapItem(OUString(), rGrf));
            pObj->SetMergedItemSetAndBroadcast(aSet);
        }

        // we are done; mark the modified/new object
        pView->MarkObj(pResult, pView->GetSdrPageView());
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
