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
// --> OD 2005-08-03 #i50824#
#include <svx/svditer.hxx>
// <--
// --> OD 2006-03-01 #b6382898#
#include <svx/svdograf.hxx>
// <--
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
#include <svx/svdlegacy.hxx>
#include <svx/svdundo.hxx>

using namespace ::com::sun::star;

/*************************************************************************
|*
|*  SwFEShell::Copy()   Copy fuer das Interne Clipboard.
|*      Kopiert alle Selektionen in das Clipboard.
|*
|*  Ersterstellung      JP ??
|*  Letzte Aenderung    MA 22. Feb. 95
|
|*************************************************************************/

sal_Bool SwFEShell::Copy( SwDoc* pClpDoc, const String* pNewClpTxt )
{
    ASSERT( pClpDoc, "kein Clipboard-Dokument"  );

    pClpDoc->GetIDocumentUndoRedo().DoUndo(false); // always false!

    // steht noch Inhalt im ClpDocument, dann muss dieser geloescht werden
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

    // stehen noch FlyFrames rum, loesche auch diese
    for( sal_uInt16 n = 0; n < pClpDoc->GetSpzFrmFmts()->Count(); ++n )
    {
        SwFlyFrmFmt* pFly = (SwFlyFrmFmt*)(*pClpDoc->GetSpzFrmFmts())[n];
        pClpDoc->DelLayoutFmt( pFly );
    }
    pClpDoc->GCFieldTypes();        // loesche die FieldTypes

    // wurde ein String uebergeben, so kopiere diesen in das Clipboard-
    // Dokument. Somit kann auch der Calculator das interne Clipboard
    // benutzen.
    if( pNewClpTxt )
    {
        pTxtNd->InsertText( *pNewClpTxt, SwIndex( pTxtNd ) );
        return sal_True;                // das wars.
    }

    pClpDoc->LockExpFlds();
    pClpDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_DELETE_REDLINES );
    sal_Bool bRet;

    // soll ein FlyFrame kopiert werden ?
    if( IsFrmSelected() )
    {
        // hole das FlyFormat
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

        // sorge dafuer das das "RootFmt" als erstes im SpzArray-steht
        // (Es wurden ggf. Flys in Flys kopiert.
        SwSpzFrmFmts& rSpzFrmFmts = *(SwSpzFrmFmts*)pClpDoc->GetSpzFrmFmts();
        if( rSpzFrmFmts[ 0 ] != pFlyFmt )
        {
            sal_uInt16 nPos = rSpzFrmFmts.GetPos( pFlyFmt );
            ASSERT( nPos != USHRT_MAX, "Fly steht nicht im Spz-Array" );

            rSpzFrmFmts.Remove( nPos );
            rSpzFrmFmts.Insert( pFlyFmt, 0 );
        }

        if ( FLY_AS_CHAR == aAnchor.GetAnchorId() )
        {
            // JP 13.02.99 Bug 61863: wenn eine Rahmenselektion ins Clipboard
            //              gestellt wird, so muss beim Pasten auch wieder
            //              eine solche vorgefunden werden. Also muss im Node
            //              das kopierte TextAttribut wieder entfernt werden,
            //              sonst wird es als TextSelektion erkannt
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
        const SdrObjectVector aSelection(Imp()->GetDrawView()->getSelectedSdrObjectVectorFromSdrMarkView());

        for ( sal_uInt32 i(0); i < aSelection.size(); ++i )
        {
            SdrObject *pObj = aSelection[i];

            if( Imp()->GetDrawView()->IsGroupEntered() ||
                ( !findConnectionToSdrObjectDirect(pObj) && pObj->GetParentSdrObject()) )
            {
                SfxItemSet aSet( pClpDoc->GetAttrPool(), aFrmFmtSetRange );

                SwFmtAnchor aAnchor( FLY_AT_PARA );
                aAnchor.SetAnchor( &aPos );
                aSet.Put( aAnchor );

                SdrObject *const pNew =
                    pClpDoc->CloneSdrObj( *pObj, sal_False, sal_True );

                SwPaM aTemp(aPos);
                pClpDoc->InsertDrawObj(aTemp, *pNew, aSet );
            }
            else
            {
                SwDrawContact *pContact = (SwDrawContact*)findConnectionToSdrObject( pObj );
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
        bRet = _CopySelToDoc( pClpDoc, 0 );     // kopiere die Selectionen

    pClpDoc->SetRedlineMode_intern((RedlineMode_t)0 );
    pClpDoc->UnlockExpFlds();
    if( !pClpDoc->IsExpFldsLocked() )
        pClpDoc->UpdateExpFlds(NULL, true);

    return bRet;
}

const Point &lcl_FindBasePos( const SwFrm *pFrm, const Point &rPt )
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

sal_Bool lcl_SetAnchor( const SwPosition& rPos, const SwNode& rNd, SwFlyFrm* pFly,
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

    if(Imp()->GetDrawView()->areSdrObjectsSelected())
    {
    //Die Liste muss kopiert werden, weil unten die neuen Objekte
    //selektiert werden.
        const SdrObjectVector aSelection(Imp()->GetDrawView()->getSelectedSdrObjectVectorFromSdrMarkView());

    if( !pDestShell->Imp()->GetDrawView() )
        // sollte mal eine erzeugt werden
        pDestShell->MakeDrawView();
    else if( bSelectInsert )
        pDestShell->Imp()->GetDrawView()->UnmarkAll();

    SwDrawView *pDestDrwView = pDestShell->Imp()->GetDrawView(),
                *pSrcDrwView = Imp()->GetDrawView();
    SwDoc* pDestDoc = pDestShell->GetDoc();
    Size aSiz( rInsPt.X() - rSttPt.X(), rInsPt.Y() - rSttPt.Y() );

        for( sal_uInt32 i(0); i < aSelection.size(); ++i )
    {
            SdrObject *pObj = aSelection[i];

            SwDrawContact *pContact = (SwDrawContact*)findConnectionToSdrObject( pObj );
        SwFrmFmt *pFmt = pContact->GetFmt();
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();

        sal_Bool bInsWithFmt = sal_True;

        if( pDestDrwView->IsGroupEntered() )
        {
            // in die Gruppe einfuegen, wenns aus einer betretenen Gruppe
            // kommt oder das Object nicht zeichengebunden ist
            if( pSrcDrwView->IsGroupEntered() ||
                (FLY_AS_CHAR != rAnchor.GetAnchorId()) )

            {
                    SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove && GetDoc() == pDestDoc, false);
                    sdr::legacy::transformSdrObject(*pNew, basegfx::tools::createTranslateB2DHomMatrix(aSiz.Width(), aSiz.Height()));
                    pDestDrwView->InsertObjectAtView( *pNew );
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
                    //gleiche Shell? Dann erfrage die Position an der
                    //uebergebenen DokumentPosition
                    SwPosition aPos( *GetCrsr()->GetPoint() );
                    Point aPt( rInsPt );
                        aPt -= rSttPt - sdr::legacy::GetSnapRect(*pObj).TopLeft();
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
                        ( !findConnectionToSdrObjectDirect(pObj) && pObj->GetParentSdrObject()) )
                {
                    SfxItemSet aSet( pDestDoc->GetAttrPool(),aFrmFmtSetRange);
                    aSet.Put( aAnchor );
                    SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                                GetDoc() == pDestDoc, sal_True );
                    pFmt = pDestDoc->InsertDrawObj( *pDestShell->GetCrsr(), *pNew, aSet );
                }
                else
                    pFmt = pDestDoc->CopyLayoutFmt( *pFmt, aAnchor, true, true );

                //Kann 0 sein, weil Draws in Kopf-/Fusszeilen nicht erlaubt sind.
                if ( pFmt )
                {
                    SdrObject* pNew = pFmt->FindSdrObject();
                    if ( FLY_AS_CHAR != aAnchor.GetAnchorId() )
                    {
                        Point aPos( rInsPt );
                        aPos -= aNewAnch;
                            aPos -= rSttPt - sdr::legacy::GetSnapRect(*pObj).TopLeft();
                        // OD 2004-04-05 #i26791# - change attributes instead of
                        // direct positioning
                        pFmt->SetFmtAttr( SwFmtHoriOrient( aPos.X(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
                        pFmt->SetFmtAttr( SwFmtVertOrient( aPos.Y(), text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
                        // --> OD 2005-04-15 #i47455# - notify draw frame format
                        // that position attributes are already set.
                            if ( dynamic_cast< SwDrawFrmFmt* >(pFmt) )
                        {
                            static_cast<SwDrawFrmFmt*>(pFmt)->PosAttrSet();
                        }
                        // <--
                    }
                    if( bSelectInsert )
                            pDestDrwView->MarkObj( *pNew );
                }
            }
        }
    }

    if ( bIsMove && bRet )
    {
        if( pDestShell == this )
        {
                const SdrObjectVector aList(pSrcDrwView->getSelectedSdrObjectVectorFromSdrMarkView());
                sal_uInt32 i(0);

            pSrcDrwView->UnmarkAll();

                for ( i = 0; i < aSelection.size(); ++i )
            {
                    SdrObject *pObj = aSelection[i];
                    pSrcDrwView->MarkObj( *pObj );
            }

            DelSelectedObj();

                for ( i = 0; i < aList.size(); ++i )
            {
                    SdrObject *pObj = aList[i];
                    pSrcDrwView->MarkObj( *pObj );
            }
        }
        else
            {
            DelSelectedObj();
    }
        }
    }

    return bRet;
}

sal_Bool SwFEShell::Copy( SwFEShell* pDestShell, const Point& rSttPt,
                    const Point& rInsPt, sal_Bool bIsMove, sal_Bool bSelectInsert )
{
    sal_Bool bRet = sal_False;

    ASSERT( pDestShell, "Copy ohne DestShell." );
    ASSERT( this == pDestShell || !pDestShell->IsObjSelected(),
            "Dest-Shell darf nie im Obj-Modus sein" );

    SET_CURR_SHELL( pDestShell );

    pDestShell->StartAllAction();
    pDestShell->GetDoc()->LockExpFlds();

    // Referenzen sollen verschoben werden.
    sal_Bool bCopyIsMove = pDoc->IsCopyIsMove();
    if( bIsMove )
        // am Doc ein Flag setzen, damit in den TextNodes
        pDoc->SetCopyIsMove( sal_True );

    RedlineMode_t eOldRedlMode = pDestShell->GetDoc()->GetRedlineMode();
    pDestShell->GetDoc()->SetRedlineMode_intern( (RedlineMode_t)(eOldRedlMode | nsRedlineMode_t::REDLINE_DELETE_REDLINES));

    // sind Tabellen-Formeln im Bereich, dann muss erst die Tabelle
    // angezeigt werden, damit die Tabellen-Formel den neuen Wert errechnen
    // kann (bei Bereichen wird sich ueber das Layout die einzelnen Boxen
    // besorgt)
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
                // gleiche Shell? Dann erfrage die Position an der
                // uebergebenen DokumentPosition
                SwPosition aPos( *GetCrsr()->GetPoint() );
                Point aPt( rInsPt );
                aPt -= rSttPt - pFly->Frm().Pos();
                SwCrsrMoveState aState( MV_SETONLYTEXT );
                GetLayout()->GetCrsrOfst( &aPos, aPt, &aState );
                const SwNode *pNd;
                if( (pNd = &aPos.nNode.GetNode())->IsNoTxtNode() )
                    bRet = sal_False;
                else
                {   //Nicht in sich selbst kopieren
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
            ASSERT( !this, "was fuer ein Anchor ist es denn?" );
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

            // nur selektieren wenn es in der gleichen Shell verschoben/
            //  kopiert wird
            if( bSelectInsert )
            {
                SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aPt, sal_False );
                if( pFlyFrm )
                {
                    //JP 12.05.98: sollte das nicht im SelectFlyFrm stehen???
                    pDestShell->Imp()->GetDrawView()->UnmarkAll();
                    pDestShell->SelectFlyFrm( *pFlyFrm, sal_True );
                }
            }
        }
    }
    else if ( IsObjSelected() )
        bRet = CopyDrawSel( pDestShell, rSttPt, rInsPt, bIsMove, bSelectInsert );
    else if( IsTableMode() )
    {
        // kopiere Teile aus einer Tabelle: lege eine Tabelle mit der Breite
        // von der Originalen an und kopiere die selectierten Boxen.
        // Die Groessen werden prozentual korrigiert.

        // lasse ueber das Layout die Boxen suchen
        const SwTableNode* pTblNd;
        SwSelBoxes aBoxes;
        GetTblSel( *this, aBoxes );
        if( aBoxes.Count() &&
            0 != (pTblNd = aBoxes[0]->GetSttNd()->FindTableNode()) )
        {
            SwPosition* pDstPos = 0;
            if( this == pDestShell )
            {
                // gleiche Shell? Dann erzeuge einen Crsr an der
                // uebergebenen DokumentPosition
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
                                        aBoxes.Count() == pTblNd->GetTable().
                                        GetTabSortBoxes().Count(),
                                        this != pDestShell );

                if( this != pDestShell )
                    *pDestShell->GetCrsr()->GetPoint() = *pDstPos;

                // wieder alle geparkten Crsr erzeugen?
                if( GetDoc() == pDestShell->GetDoc() )
                    GetCrsr();

                // JP 16.04.99: Bug 64908 - InsPos setzen, damit der geparkte
                //              Cursor auf die EinfuegePos. positioniert wird
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
            // gleiche Shell? Dann erfrage die Position an der
            // uebergebenen DokumentPosition
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

    // wurden neue Tabellenformeln eingefuegt ?
    if( pTblFldTyp->GetDepends() )
    {
        // alte Actions beenden; die Tabellen-Frames werden angelegt und
        // eine SSelection kann erzeugt werden
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
|*  SwFEShell::Paste()  Paste fuer das Interne Clipboard.
|*      Kopiert den Inhalt vom Clipboard in das Dokument.
|*
|*  Ersterstellung      JP ??
|*  Letzte Aenderung    MA 22. Feb. 95
|
|*************************************************************************/

namespace {
    typedef boost::shared_ptr<SwPaM> PaMPtr;
    typedef boost::shared_ptr<SwPosition> PositionPtr;
    typedef std::pair< PaMPtr, PositionPtr > Insertion;
}

sal_Bool SwFEShell::Paste( SwDoc* pClpDoc, sal_Bool bIncludingPageFrames )
{
    SET_CURR_SHELL( this );
    ASSERT( pClpDoc, "kein Clipboard-Dokument"  );
    const sal_uInt16 nStartPageNumber = GetPhyPageNum();
    // dann bis zum Ende vom Nodes Array
    SwNodeIndex aIdx( pClpDoc->GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aIdx ); //DocStart

    // sind Tabellen-Formeln im Bereich, dann muss erst die Tabelle
    // angezeigt werden, damit die Tabellen-Formel den neuen Wert errechnen
    // kann (bei Bereichen wird sich ueber das Layout die einzelnen Boxen
    // besorgt)
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
            ASSERT( aIdx.GetNode().GetCntntNode(), "Who filled the clipboard?!" )
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
                    aIndexBefore++;
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

            // TABLE IN TABLE: Tabelle in Tabelle kopieren
            // lasse ueber das Layout die Boxen suchen
            SwSelBoxes aBoxes;
            if( IsTableMode() )     // Tabellen-Selecktion ??
            {
                GetTblSel( *this, aBoxes );
                ParkTblCrsr();
                bParkTblCrsr = sal_True;
            }
            else if( !PCURCRSR->HasMark() && PCURCRSR->GetNext() == PCURCRSR &&
                     ( !pSrcNd->GetTable().IsTblComplex() ||
                       pDestNd->GetTable().IsNewModel() ) )
            {
                // dann die Tabelle "relativ" kopieren
                SwTableBox* pBox = pDestNd->GetTable().GetTblBox(
                                        pSttNd->GetIndex() );
                ASSERT( pBox, "Box steht nicht in dieser Tabelle" );
                aBoxes.Insert( pBox );
            }

            SwNodeIndex aNdIdx( *pDestNd->EndOfSectionNode());
            if( !bParkTblCrsr )
            {
                // erstmal aus der gesamten Tabelle raus
// ????? was ist mit Tabelle alleine im Rahmen ???????
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
                // und wieder in die Box zurueck
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

            break;      // aus der "while"-Schleife heraus
        }
        else if( *aCpyPam.GetPoint() == *aCpyPam.GetMark() &&
                 pClpDoc->GetSpzFrmFmts()->Count() )
        {
            // so langsam sollte mal eine DrawView erzeugt werden
            if( !Imp()->GetDrawView() )
                MakeDrawView();

            for ( sal_uInt16 i = 0; i < pClpDoc->GetSpzFrmFmts()->Count(); ++i )
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
                                                            sal_False, sal_False );

                        // Insert object sets any anchor position to 0.
                        // Therefore we calculate the absolute position here
                        // and after the insert the anchor of the object
                        // is set to the anchor of the group object.
                        Rectangle aSnapRect(sdr::legacy::GetSnapRect(*pNew));
                        if( !pNew->GetAnchorPos().equalZero() )
                        {
                            // OD 2004-04-05 #i26791# - direct drawing object
                            // positioning for group members
                            pNew->SetAnchorPos( basegfx::B2DPoint() );
                            sdr::legacy::SetSnapRect(*pNew, aSnapRect);
                        }

                        Imp()->GetDrawView()->InsertObjectAtView( *pNew );

                        basegfx::B2DPoint aGrpAnchor( 0.0, 0.0 );
                        SdrObjList* pList = pNew->getParentOfSdrObject();
                        if ( pList )
                        {
                            SdrObject* pOwner = pList->getSdrObjectFromSdrObjList();
                            if ( pOwner )
                            {
                                SdrObjGroup* pThisGroup = dynamic_cast< SdrObjGroup* >( pOwner);

                                if(pThisGroup)
                                {
                                aGrpAnchor = pThisGroup->GetAnchorPos();
                            }
                                else
                                {
                                    OSL_ENSURE(false, "Not only SdrObjGroup can be parent of SdrObject(s)");
                                }
                            }
                        }

                        // OD 2004-04-05 #i26791# - direct drawing object
                        // positioning for group members
                        pNew->SetAnchorPos(aGrpAnchor);
                        sdr::legacy::SetSnapRect(*pNew, aSnapRect);

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
                        // #108784# allow shapes (no controls) in header/footer
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
                            // immer nur den ersten Fly-Frame nehmen; die anderen
                            // wurden ueber Fly in Fly ins ClipBoard kopiert !
                            break;
                        }
                        else
                        {
                            ASSERT( RES_DRAWFRMFMT == pNew->Which(), "Neues Format.");
                            // --> OD 2005-09-01 #i52780# - drawing object has
                            // to be made visible on paste.
                            {
                                SwDrawContact* pContact =
                                    static_cast<SwDrawContact*>(pNew->FindContactObj());
                                pContact->MoveObjToVisibleLayer( pContact->GetMaster() );
                            }
                            // <--
                            SdrObject *pObj = pNew->FindSdrObject();
                            SwDrawView  *pDV = Imp()->GetDrawView();
                            pDV->MarkObj( *pObj );
                            // --> OD 2005-04-15 #i47455# - notify draw frame format
                            // that position attributes are already set.
                            if ( dynamic_cast< SwDrawFrmFmt* >(pNew) )
                            {
                                static_cast<SwDrawFrmFmt*>(pNew)->PosAttrSet();
                            }
                            // <--
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
                // es wird mehr als 1 Node in die akt. Box kopiert. Dann
                // muessen die BoxAttribute aber entfernt werden.
                GetDoc()->ClearBoxNumAttrs( rInsPos.nNode );
            }
            //find out if the clipboard document starts with a table
            bool bStartWithTable = 0 != aCpyPam.Start()->nNode.GetNode().FindTableNode();
            SwPosition aInsertPosition( rInsPos );

            {
                SwNodeIndex aIndexBefore(rInsPos.nNode);

                aIndexBefore--;

                pClpDoc->CopyRange( aCpyPam, rInsPos, false );

                {
                    aIndexBefore++;
                    SwPaM aPaM(SwPosition(aIndexBefore),
                               SwPosition(rInsPos.nNode));

                    aPaM.GetDoc()->MakeUniqueNumRules(aPaM);
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
            if( bIncludingPageFrames && pClpDoc->GetSpzFrmFmts()->Count() )
            {
                // create a draw view if necessary
                if( !Imp()->GetDrawView() )
                    MakeDrawView();

                for ( sal_uInt16 i = 0; i < pClpDoc->GetSpzFrmFmts()->Count(); ++i )
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

    // wurden neue Tabellenformeln eingefuegt ?
    if( pTblFldTyp->GetDepends() )
    {
        // alte Actions beenden; die Tabellen-Frames werden angelegt und
        // eine Selection kann erzeugt werden
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

/*-- 14.06.2004 13:31:17---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    //additionally copy page bound frames
    if( GetDoc()->GetSpzFrmFmts()->Count() )
    {
        // create a draw view if necessary
        if( !rToFill.Imp()->GetDrawView() )
            rToFill.MakeDrawView();

        for ( sal_uInt16 i = 0; i < GetDoc()->GetSpzFrmFmts()->Count(); ++i )
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
    ASSERT( Imp()->HasDrawView(), "GetDrawObjGraphic without DrawView?" );
    sal_Bool bConvert = sal_True;

    if(Imp()->GetDrawView()->areSdrObjectsSelected())
    {
        const SwVirtFlyDrawObj* pSingleSelected = dynamic_cast< SwVirtFlyDrawObj* >(Imp()->GetDrawView()->getSelectedIfSingle());

        if(pSingleSelected)
        {
            // Rahmen selektiert
            if( CNT_GRF == GetCntType() )
            {
                // --> OD 2005-02-09 #119353# - robust
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
                        //fix(23806): Nicht die Originalgroesse, sondern die
                        //aktuelle. Anderfalls kann es passieren, dass z.B. bei
                        //Vektorgrafiken mal eben zig MB angefordert werden.
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
                // <--
            }
        }
        else if( SOT_FORMAT_GDIMETAFILE == nFmt )
        {
            rGrf = Imp()->GetDrawView()->GetMarkedObjMetaFile();
        }
        else if( SOT_FORMAT_BITMAP == nFmt || SOT_FORMATSTR_ID_PNG == nFmt )
        {
            rGrf = Imp()->GetDrawView()->GetMarkedObjBitmapEx();
        }
    }

    return bConvert;
}

// --> OD 2005-08-03 #i50824#
// --> OD 2006-03-01 #b6382898#
// replace method <lcl_RemoveOleObjsFromSdrModel> by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
void lcl_ConvertSdrOle2ObjsToSdrGrafObjs( SdrModel* _pModel )
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
                SdrObjList* pObjList = pOle2Obj->getParentOfSdrObject();

                // get its graphic
                Graphic aGraphic;
                pOle2Obj->Connect();
                Graphic* pGraphic = pOle2Obj->GetGraphic();
                if( pGraphic )
                    aGraphic = *pGraphic;
                pOle2Obj->Disconnect();

                // create new graphic shape with the ole graphic and shape size
                SdrGrafObj* pGraphicObj = new SdrGrafObj(
                    *_pModel,
                    aGraphic,
                    pOle2Obj->getSdrObjectTransformation());

                // apply layer of ole2 shape at graphic shape
                pGraphicObj->SetLayer( pOle2Obj->GetLayer() );

                // replace ole2 shape with the new graphic object and delete the ol2 shape
                SdrObject* pRemovedObject = pObjList->ReplaceObjectInSdrObjList(
                    *pGraphicObj, pOle2Obj->GetNavigationPosition() );
                deleteSdrObjectSafeAndClearPointer( pRemovedObject );
            }
        }
    }
}
// <--
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

    //Drop auf bestehendes Objekt: Objekt ersetzen oder neu Attributieren.
    SdrObject* pOldObj = pView->getSelectedIfSingle();
    if( pModel->GetPageCount() > 0 &&
        1 == pModel->GetPage(0)->GetObjCount() &&
        pOldObj )
    {
        // OD 10.07.2003 #110742# - replace a marked 'virtual' drawing object
        // by its corresponding 'master' drawing object in the mark list.
        SwDrawView::ReplaceMarkedDrawVirtObjs( *pView );
        SdrObject* pClpObj = pModel->GetPage(0)->GetObj(0);

        if( SW_PASTESDR_SETATTR == nAction && dynamic_cast< SwVirtFlyDrawObj* >(pOldObj) )
            nAction = SW_PASTESDR_REPLACE;

        switch( nAction )
        {
        case SW_PASTESDR_REPLACE:
            {
                const SwFrmFmt* pFmt(0);
                const SwFrm* pAnchor(0);
                if( dynamic_cast< SwVirtFlyDrawObj* >(pOldObj) )
                {
                    pFmt = FindFrmFmt( pOldObj );

                    Point aNullPt;
                    SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFmt)->GetFrm( &aNullPt );
                    pAnchor = pFlyFrm->GetAnchorFrm();

                    if( pAnchor->FindFooterOrHeader() )
                    {
                        // wenn TextRahmen in der Kopf/Fusszeile steht, dann
                        // nicht ersetzen, sondern nur einfuegen
                        nAction = SW_PASTESDR_INSERT;
                        break;
                    }
                }

                SdrObject* pNewObj = pClpObj->CloneSdrObject(&pView->getSdrModelFromSdrView());

                // copy transformation
                pNewObj->setSdrObjectTransformation(pOldObj->getSdrObjectTransformation());

                if( dynamic_cast< SdrUnoObj* >(pNewObj) )
                {
                    pNewObj->SetLayer( GetDoc()->GetControlsId() );
                }
                else if( dynamic_cast< SdrUnoObj* >(pOldObj) )
                {
                    pNewObj->SetLayer( GetDoc()->GetHeavenId() );
                }
                else
                {
                    pNewObj->SetLayer( pOldObj->GetLayer() );
                }

                if( dynamic_cast< SwVirtFlyDrawObj* >(pOldObj) )
                {
                    // Attribute sichern und dam SdrObject setzen
                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                            RES_SURROUND, RES_ANCHOR );
                    aFrmSet.Set( pFmt->GetAttrSet() );

                    if( pAnchor->IsTxtFrm() && ((SwTxtFrm*)pAnchor)->IsFollow() )
                    {
                        const SwTxtFrm* pTmp = (SwTxtFrm*)pAnchor;
                        do {
                            pTmp = pTmp->FindMaster();
                            ASSERT( pTmp, "Where's my Master?" );
                        } while( pTmp->IsFollow() );
                        pAnchor = pTmp;
                    }

                    const Point aNewAnchor = pAnchor->GetFrmAnchorPos( ::HasWrap( pOldObj ) );
                    // OD 2004-04-05 #i26791# - direct positioning of Writer
                    // fly frame object for <SwDoc::Insert(..)>

                    // #i108739#
                    pNewObj->SetAnchorPos( basegfx::B2DPoint( aNewAnchor.X(), aNewAnchor.Y() ) );
                    DelSelectedObj();

                    pFmt = GetDoc()->InsertDrawObj( *GetCrsr(), *pNewObj, aFrmSet );
                }
                else
                {
                    // #123922#  for handling MasterObject and virtual ones correctly, SW
                    // wants us to call ReplaceObject at the page, but that also
                    // triggers the same assertion (I tried it), so stay at the view method
                    pView->ReplaceObjectAtView(*pOldObj, *pNewObj);
                }
            }
            break;

        case SW_PASTESDR_SETATTR:
            {
                SfxItemSet aSet( GetAttrPool() );
                const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pClpObj);

                if(pSdrGrafObj)
                {
                    SdrObject* pTarget = pView->getSelectedIfSingle();

                    if(pTarget)
                    {
                        // copy ItemSet from target
                        aSet.Set(pTarget->GetMergedItemSet());
                    }

                    // for SdrGrafObj, use the graphic as fill style argument
                    const Graphic& rGraphic = pSdrGrafObj->GetGraphic();

                    if(GRAPHIC_NONE != rGraphic.GetType() && GRAPHIC_DEFAULT != rGraphic.GetType())
                    {
                        aSet.Put(XFillBitmapItem(String(), rGraphic));
                        aSet.Put(XFillStyleItem(XFILL_BITMAP));
                    }
                }
                else
                {
                    aSet.Put(pClpObj->GetMergedItemSet());
                }

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

        bool bDesignMode = pView->IsDesignMode();
        if( !bDesignMode )
            pView->SetDesignMode( true );

        // --> OD 2005-08-03 #i50824#
        // --> OD 2006-03-01 #b6382898#
        // method <lcl_RemoveOleObjsFromSdrModel> replaced by <lcl_ConvertSdrOle2ObjsToSdrGrafObjs>
        lcl_ConvertSdrOle2ObjsToSdrGrafObjs( pModel );
        // <--
        pView->Paste( *pModel, basegfx::B2DPoint(aPos.X(),aPos.Y()) );

        if( pView->areSdrObjectsSelected() )
        {
            const SdrObjectVector aSelection(pView->getSelectedSdrObjectVectorFromSdrMarkView());

            for( sal_uInt32 i=0; i < aSelection.size(); ++i )
            {
                SdrObject *pObj = aSelection[i];
                pObj->SetAnchorPos( basegfx::B2DPoint() );
            }

            pView->setSdrObjectCreationInfo(SdrObjectCreationInfo(OBJ_GRUP));

            if ( aSelection.size() > 1 )
                pView->GroupMarked();
            SdrObject *pObj = aSelection[0];
            if( dynamic_cast< SdrUnoObj* >(pObj) )
            {
                pObj->SetLayer( GetDoc()->GetControlsId() );
                bDesignMode = true;
            }
            else
                pObj->SetLayer( GetDoc()->GetHeavenId() );
            const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(*pObj));
            pView->MoveMarkedObj(aSnapRange.getRange() * 0.5);
            ImpEndCreate();
            if( !bDesignMode )
                pView->SetDesignMode( false );
        }
    }
    EndUndo();
    EndAllAction();
    delete pModel;
}

bool SwFEShell::Paste( const Graphic &rGrf, const String& rURL )
{
    SET_CURR_SHELL( this );
    SdrView* pView = Imp()->GetDrawView();
    SdrObject* pSingleSelected = pView ? pView->getSelectedIfSingle() : 0;

    const bool bRet(
        pSingleSelected &&
        pSingleSelected->IsClosedObj() &&
        !dynamic_cast< SdrOle2Obj* >(pSingleSelected));

    if(bRet)
    {
        // #123922# added code to handle the two cases of SdrGrafObj and a fillable, non-
        // OLE object in focus
        SdrObject* pResult = pSingleSelected;

        if(dynamic_cast< SdrGrafObj* >(pSingleSelected))
        {
            SdrGrafObj* pNewGrafObj = (SdrGrafObj*)pSingleSelected->CloneSdrObject();

            pNewGrafObj->SetGraphic(rGrf);

            // #123922#  for handling MasterObject and virtual ones correctly, SW
            // wants us to call ReplaceObject at the page, but that also
            // triggers the same assertion (I tried it), so stay at the view method
            pView->ReplaceObjectAtView(*pSingleSelected, *pNewGrafObj);

            // set in all cases - the Clone() will have copied an existing link (!)
            pNewGrafObj->SetGraphicLink(rURL, String());

            pResult = pNewGrafObj;
        }
        else
        {
            pView->AddUndo(new SdrUndoAttrObj(*pSingleSelected));

            SfxItemSet aSet(pView->getSdrModelFromSdrView().GetItemPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);

            aSet.Put(XFillStyleItem(XFILL_BITMAP));
            aSet.Put(XFillBitmapItem(String(), rGrf));
            pSingleSelected->SetMergedItemSetAndBroadcast(aSet);
        }

        // we are done; mark the modified/new object
        pView->MarkObj(*pResult);
    }

    return bRet;
}
