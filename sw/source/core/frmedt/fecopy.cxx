/*************************************************************************
 *
 *  $RCSfile: fecopy.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-09 15:51:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifdef WIN
#define NEEDED_BY_FESHVIEW
#endif

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVX_XEXCH_HXX
#include <svx/xexch.hxx>
#endif
#ifndef _SVX_XFLASIT_HXX //autogen
#include <svx/xflasit.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDCAPT_HXX //autogen
#include <svx/svdocapt.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif


/*************************************************************************
|*
|*  FindPageFrm(), Sucht den PageFrm zum Pt, die StartPage wird hereingereicht.
|*
|*  Ersterstellung      MA 11. Oct. 95
|*  Letzte Aenderung    MA 11. Oct. 95
|
|*************************************************************************/
const SwFrm *FindPage( const SwFrm *pPg, const Point &rPt )
{
    if ( !pPg->Frm().IsInside( rPt ) )
    {
        const long nTop = rPt.Y();
        FASTBOOL bPrvAllowed = TRUE;
        FASTBOOL bNxtAllowed = TRUE;
        do
        {   if ( pPg->Frm().Top() > nTop && bPrvAllowed )
            {
                if ( pPg->GetPrev() )
                {
                    bNxtAllowed = FALSE;
                    pPg = pPg->GetPrev();
                }
                else
                    break;
            }
            else if ( pPg->Frm().Bottom() < nTop && bNxtAllowed )
            {
                if ( pPg->GetNext() )
                {
                    bPrvAllowed = FALSE;
                    pPg = pPg->GetNext();
                }
                else
                    break;
            }
            else
                break;

        } while ( !pPg->Frm().IsInside( rPt ) );
    }
    return pPg;
}


/*************************************************************************
|*
|*  SwFEShell::Copy()   Copy fuer das Interne Clipboard.
|*      Kopiert alle Selektionen in das Clipboard.
|*
|*  Ersterstellung      JP ??
|*  Letzte Aenderung    MA 22. Feb. 95
|
|*************************************************************************/

BOOL SwFEShell::Copy( SwDoc* pClpDoc, const String* pNewClpTxt )
{
    ASSERT( pClpDoc, "kein Clipboard-Dokument"  );

    pClpDoc->DoUndo( FALSE );       // immer auf FALSE !!

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
    for( USHORT n = 0; n < pClpDoc->GetSpzFrmFmts()->Count(); ++n )
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
        pTxtNd->Insert( *pNewClpTxt, SwIndex( pTxtNd ) );
        return TRUE;                // das wars.
    }

    pClpDoc->LockExpFlds();
    pClpDoc->SetRedlineMode_intern( REDLINE_DELETE_REDLINES );
    BOOL bRet;

    // soll ein FlyFrame kopiert werden ?
    if( IsFrmSelected() )
    {
        // hole das FlyFormat
        SwFlyFrm* pFly = FindFlyFrm();
        SwFrmFmt* pFlyFmt = pFly->GetFmt();
        SwFmtAnchor aAnchor( pFlyFmt->GetAnchor() );

        if ( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
             FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
             FLY_AT_FLY == aAnchor.GetAnchorId() ||
             FLY_IN_CNTNT == aAnchor.GetAnchorId() )
        {
            SwPosition aPos( aSttIdx );
            if( FLY_IN_CNTNT == aAnchor.GetAnchorId() )
                aPos.nContent.Assign( pTxtNd, 0 );
            aAnchor.SetAnchor( &aPos );
        }
        pFlyFmt = pClpDoc->CopyLayoutFmt( *pFlyFmt, aAnchor );

        // sorge dafuer das das "RootFmt" als erstes im SpzArray-steht
        // (Es wurden ggf. Flys in Flys kopiert.
        SwSpzFrmFmts& rSpzFrmFmts = *(SwSpzFrmFmts*)pClpDoc->GetSpzFrmFmts();
        if( rSpzFrmFmts[ 0 ] != pFlyFmt )
        {
            USHORT nPos = rSpzFrmFmts.GetPos( pFlyFmt );
            ASSERT( nPos != USHRT_MAX, "Fly steht nicht im Spz-Array" );

            rSpzFrmFmts.Remove( nPos );
            rSpzFrmFmts.Insert( pFlyFmt, 0 );
        }

        if( FLY_IN_CNTNT == aAnchor.GetAnchorId() )
        {
            // JP 13.02.99 Bug 61863: wenn eine Rahmenselektion ins Clipboard
            //              gestellt wird, so muss beim Pasten auch wieder
            //              eine solche vorgefunden werden. Also muss im Node
            //              das kopierte TextAttribut wieder entfernt werden,
            //              sonst wird es als TextSelektion erkannt
            const SwIndex& rIdx = pFlyFmt->GetAnchor().GetCntntAnchor()->nContent;
            SwTxtFlyCnt* pTxtFly = (SwTxtFlyCnt*)pTxtNd->GetTxtAttr(
                                                rIdx, RES_TXTATR_FLYCNT );
            if( pTxtFly )
            {
                ((SwFmtFlyCnt&)pTxtFly->GetFlyCnt()).SetFlyFmt( 0 );
                pTxtNd->Erase( rIdx, 1 );
            }
        }
        bRet = TRUE;
    }
    else if ( IsObjSelected() )
    {
        Size aSiz( 0, - GetDoc()->GetDrawModel()->GetPage( 0 )->
                        GetAllObjBoundRect().Top() );

        SwPosition aPos( aSttIdx, SwIndex( pTxtNd, 0 ));
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();

            if( Imp()->GetDrawView()->IsGroupEntered() ||
                ( !pObj->GetUserCall() && pObj->GetUpGroup()) )
            {
                SfxItemSet aSet( pClpDoc->GetAttrPool(), aFrmFmtSetRange );

                SwFmtAnchor aAnchor( FLY_AT_CNTNT );
                aAnchor.SetAnchor( &aPos );
                aSet.Put( aAnchor );

                SdrObject* pNew = pClpDoc->CloneSdrObj( *pObj, FALSE, TRUE );
//JP 07.01.00: why move??
//              pNew->NbcMove( aSiz );
                pClpDoc->Insert( SwPaM( aPos ), *pNew, &aSet );
            }
            else
            {
                SwDrawContact *pContact = (SwDrawContact*)GetUserCall( pObj );
                SwFrmFmt *pFmt = pContact->GetFmt();
                SwFmtAnchor aAnchor( pFmt->GetAnchor() );
                if ( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
                     FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
                     FLY_AT_FLY == aAnchor.GetAnchorId() ||
                     FLY_IN_CNTNT == aAnchor.GetAnchorId() )
                {
                    aAnchor.SetAnchor( &aPos );
                }

                pClpDoc->CopyLayoutFmt( *pFmt, aAnchor );
            }
        }
        bRet = TRUE;
    }
    else
        bRet = _CopySelToDoc( pClpDoc, 0 );     // kopiere die Selectionen

    pClpDoc->SetRedlineMode_intern( 0 );
    pClpDoc->UnlockExpFlds();
    if( !pClpDoc->IsExpFldsLocked() )
        pClpDoc->UpdateExpFlds();

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

BOOL lcl_SetAnchor( const SwPosition& rPos, const SwNode& rNd, SwFlyFrm* pFly,
                const Point& rInsPt, SwFEShell& rDestShell, SwFmtAnchor& rAnchor,
                Point& rNewPos, BOOL bCheckFlyRecur )
{
    BOOL bRet = TRUE;
    rAnchor.SetAnchor( &rPos );
    SwCntntFrm* pTmpFrm = rNd.GetCntntNode()->GetFrm( &rInsPt, 0, FALSE );
    SwFlyFrm *pTmpFly = pTmpFrm->FindFlyFrm();
    if( pTmpFly && bCheckFlyRecur && pFly->IsUpperOf( pTmpFly ) )
        bRet = FALSE;
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
            rAnchor.SetType( FLY_PAGE );
            rAnchor.SetPageNum( rDestShell.GetPageNumber( rInsPt ) );
            const SwFrm *pPg = pTmpFrm->FindPageFrm();
            rNewPos = pPg->Frm().Pos();
        }
    }
    else
        rNewPos = ::lcl_FindBasePos( pTmpFrm, rInsPt );
    return bRet;
}

BOOL SwFEShell::CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                                const Point& rInsPt, BOOL bIsMove )
{
    BOOL bRet = TRUE;

    //Die Liste muss kopiert werden, weil unten die neuen Objekte
    //selektiert werden.
    const SdrMarkList aMrkList( Imp()->GetDrawView()->GetMarkList() );
    ULONG nMarkCount = aMrkList.GetMarkCount();
    if( pDestShell->Imp()->GetDrawView() )
        pDestShell->Imp()->GetDrawView()->UnmarkAll();
    else
        // sollte mal eine erzeugt werden
        pDestShell->MakeDrawView();

    SdrPageView *pDestPgView = pDestShell->Imp()->GetPageView(),
                *pSrcPgView = Imp()->GetPageView();
    SwDrawView *pDestDrwView = pDestShell->Imp()->GetDrawView(),
                *pSrcDrwView = Imp()->GetDrawView();
    SwDoc* pDestDoc = pDestShell->GetDoc();

    Size aSiz( rInsPt.X() - rSttPt.X(), rInsPt.Y() - rSttPt.Y() );
    for( USHORT i = 0; i < nMarkCount; ++i )
    {
        SdrObject *pObj = aMrkList.GetMark( i )->GetObj();

        SwDrawContact *pContact = (SwDrawContact*)GetUserCall( pObj );
        SwFrmFmt *pFmt = pContact->GetFmt();
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();

        BOOL bInsWithFmt = TRUE;

        if( pDestDrwView->IsGroupEntered() )
        {
            // in die Gruppe einfuegen, wenns aus einer betretenen Gruppe
            // kommt oder das Object nicht zeichengebunden ist
            if( pSrcDrwView->IsGroupEntered() ||
                FLY_IN_CNTNT != rAnchor.GetAnchorId() )

            {
                SdrObject* pNew = pDestDoc->CloneSdrObj( *pObj, bIsMove &&
                                        GetDoc() == pDestDoc, FALSE );
                pNew->NbcMove( aSiz );
                pDestDrwView->InsertObject( pNew, *pDestPgView );
                bInsWithFmt = FALSE;
            }
        }

        if( bInsWithFmt )
        {
            SwFmtAnchor aAnchor( rAnchor );
            Point aNewAnch;

            if ( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
                    FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
                    FLY_AT_FLY == aAnchor.GetAnchorId() ||
                    FLY_IN_CNTNT == aAnchor.GetAnchorId() )
            {
                if ( this == pDestShell )
                {
                    //gleiche Shell? Dann erfrage die Position an der
                    //uebergebenen DokumentPosition
                    SwPosition aPos( *GetCrsr()->GetPoint() );
                    Point aPt( rInsPt );
                    aPt -= rSttPt - pObj->GetSnapRect().TopLeft();
                    SwCrsrMoveState aState( MV_SETONLYTEXT );
                    GetLayout()->GetCrsrOfst( &aPos, aPt, &aState );
                    const SwNode *pNd;
                    if( (pNd = &aPos.nNode.GetNode())->IsNoTxtNode() )
                        bRet = FALSE;
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, 0, rInsPt,
                                *pDestShell, aAnchor, aNewAnch, FALSE );
                }
                else
                {
                    SwPaM *pCrsr = pDestShell->GetCrsr();
                    if( pCrsr->GetNode()->IsNoTxtNode() )
                        bRet = FALSE;
                    else
                        bRet = ::lcl_SetAnchor( *pCrsr->GetPoint(),
                                                *pCrsr->GetNode(), 0, rInsPt,
                                                *pDestShell, aAnchor,
                                                aNewAnch, FALSE );
                }
            }
            else if( FLY_PAGE == aAnchor.GetAnchorId() )
            {
                aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
                const SwFrm *pPg = ::FindPage( pDestShell->GetLayout()->Lower(), rInsPt);
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
                                                GetDoc() == pDestDoc, TRUE );
                    pFmt = pDestDoc->Insert( *pDestShell->GetCrsr(),
                                            *pNew, &aSet );
                }
                else
                    pFmt = pDestDoc->CopyLayoutFmt( *pFmt, aAnchor );

                //Kann 0 sein, weil Draws in Kopf-/Fusszeilen nicht erlaubt sind.
                if ( pFmt )
                {
                    SdrObject *pNew = pFmt->FindSdrObject();
                    if( FLY_IN_CNTNT != aAnchor.GetAnchorId() )
                    {
                        Point aPos( rInsPt );
                        aPos -= aNewAnch;
                        aPos -= rSttPt - pObj->GetSnapRect().TopLeft();
                        pNew->ImpSetAnchorPos( aNewAnch );
                        pNew->SetRelativePos( aPos );
                    }
                    pDestDrwView->MarkObj( pNew, pDestPgView );
                }
            }
        }
    }

    if ( bIsMove && bRet )
    {
        if( pDestShell == this )
        {
            const SdrMarkList aList( pSrcDrwView->GetMarkList() );
            pSrcDrwView->UnmarkAll();

            ULONG nMarkCount = aMrkList.GetMarkCount();
            for ( USHORT i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = aMrkList.GetMark( i )->GetObj();
                pSrcDrwView->MarkObj( pObj, pSrcPgView );
            }
            DelSelectedObj();
            nMarkCount = aList.GetMarkCount();
            for ( i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = aList.GetMark( i )->GetObj();
                pSrcDrwView->MarkObj( pObj, pSrcPgView );
            }
        }
        else
            DelSelectedObj();
    }

//  if( this != pDestShell )
//      pDestDrwView->SetMarkHdlHidden( TRUE );

    return bRet;
}

BOOL SwFEShell::Copy( SwFEShell* pDestShell, const Point& rSttPt,
                        const Point& rInsPt, BOOL bIsMove )
{
    BOOL bRet = FALSE;

    ASSERT( pDestShell, "Copy ohne DestShell." );
    ASSERT( this == pDestShell || !pDestShell->IsObjSelected(),
            "Dest-Shell darf nie im Obj-Modus sein" );

    SET_CURR_SHELL( pDestShell );

    pDestShell->StartAllAction();
    pDestShell->GetDoc()->LockExpFlds();

    // Referenzen sollen verschoben werden.
    BOOL bCopyIsMove = pDoc->IsCopyIsMove();
    if( bIsMove )
        // am Doc ein Flag setzen, damit in den TextNodes
        pDoc->SetCopyIsMove( TRUE );

    SwRedlineMode eOldRedlMode = pDestShell->GetDoc()->GetRedlineMode();
    pDestShell->GetDoc()->SetRedlineMode_intern( eOldRedlMode | REDLINE_DELETE_REDLINES );

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
        bRet = TRUE;
        Point aNewAnch;

        if ( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
             FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
             FLY_AT_FLY == aAnchor.GetAnchorId() ||
             FLY_IN_CNTNT == aAnchor.GetAnchorId() )
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
                if( (pNd = GetDoc()->GetNodes()[ aPos.nNode ])->IsNoTxtNode() )
                    bRet = FALSE;
                else
                {   //Nicht in sich selbst kopieren
                    const SwNodeIndex *pTmp = pFlyFmt->GetCntnt().GetCntntIdx();
                    if ( aPos.nNode > *pTmp && aPos.nNode <
                        GetDoc()->GetNodes()[*pTmp]->EndOfSectionIndex() )
                    {
                        bRet = FALSE;
                    }
                    else
                        bRet = ::lcl_SetAnchor( aPos, *pNd, pFly, rInsPt,
                                        *pDestShell, aAnchor, aNewAnch, TRUE );
                }
            }
            else
            {
                const SwPaM *pCrsr = pDestShell->GetCrsr();
                if( pCrsr->GetNode()->IsNoTxtNode() )
                    bRet = FALSE;
                else
                    bRet = ::lcl_SetAnchor( *pCrsr->GetPoint(), *pCrsr->GetNode(),
                                            pFly, rInsPt, *pDestShell, aAnchor,
                                    aNewAnch, GetDoc() == pDestShell->GetDoc());
            }
        }
        else if( FLY_PAGE == aAnchor.GetAnchorId() )
        {
            aAnchor.SetPageNum( pDestShell->GetPageNumber( rInsPt ) );
            const SwFrm *pPg = ::FindPage( pDestShell->GetLayout()->Lower(), rInsPt);
            aNewAnch = pPg->Frm().Pos();
        }
        else
            ASSERT( !this, "was fuer ein Anchor ist es denn?" );

        if( bRet )
        {
            SwFrmFmt *pOldFmt = pFlyFmt;
            pFlyFmt = pDestShell->GetDoc()->CopyLayoutFmt( *pFlyFmt, aAnchor );

            if( FLY_IN_CNTNT != aAnchor.GetAnchorId() )
            {
                Point aPos( rInsPt );
                aPos -= aNewAnch;
                aPos -= rSttPt - pFly->Frm().Pos();
                pFlyFmt->SetAttr( SwFmtHoriOrient( aPos.X(),HORI_NONE,FRAME ) );
                pFlyFmt->SetAttr( SwFmtVertOrient( aPos.Y(),VERT_NONE,FRAME ) );
            }

            const Point aPt( pDestShell->GetCrsrDocPos() );

            if( bIsMove )
                GetDoc()->DelLayoutFmt( pOldFmt );

            // nur selektieren wenn es in der gleichen Shell verschoben/
            //  kopiert wird
            SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aPt, FALSE );
            if( pFlyFrm )
            {
                //JP 12.05.98: sollte das nicht im SelectFlyFrm stehen???
                pDestShell->Imp()->GetDrawView()->UnmarkAll();
                pDestShell->SelectFlyFrm( *pFlyFrm, TRUE );
            }

            if( this != pDestShell && !pDestShell->HasShFcs() )
                pDestShell->Imp()->GetDrawView()->SetMarkHdlHidden( TRUE );
        }
    }
    else if ( IsObjSelected() )
        bRet = CopyDrawSel( pDestShell, rSttPt, rInsPt, bIsMove );
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
                if( !GetDoc()->GetNodes()[ pDstPos->nNode ]->IsNoTxtNode() )
                    bRet = TRUE;
            }
            else if( !pDestShell->GetCrsr()->GetNode()->IsNoTxtNode() )
            {
                pDstPos = new SwPosition( *pDestShell->GetCrsr()->GetPoint() );
                bRet = TRUE;
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
        bRet = TRUE;
        if( this == pDestShell )
        {
            // gleiche Shell? Dann erfrage die Position an der
            // uebergebenen DokumentPosition
            SwPosition aPos( *GetCrsr()->GetPoint() );
            Point aPt( rInsPt );
            GetLayout()->GetCrsrOfst( &aPos, aPt );
            bRet = !GetDoc()->GetNodes()[ aPos.nNode ]->IsNoTxtNode();
        }
        else if( pDestShell->GetCrsr()->GetNode()->IsNoTxtNode() )
            bRet = FALSE;

        if( bRet )
        {

            bRet = 0 != SwEditShell::Copy( pDestShell );
        }
    }

    pDestShell->GetDoc()->SetRedlineMode_intern( eOldRedlMode );
    pDoc->SetCopyIsMove( bCopyIsMove );

    // wurden neue Tabellenformeln eingefuegt ?
    if( pTblFldTyp->GetDepends() )
    {
        // alte Actions beenden; die Tabellen-Frames werden angelegt und
        // eine SSelection kann erzeugt werden
        for( USHORT nActCnt = 0; pDestShell->ActionPend(); ++nActCnt )
            pDestShell->EndAllAction();

        for( ; nActCnt; --nActCnt )
            pDestShell->StartAllAction();
    }
    pDestShell->GetDoc()->UnlockExpFlds();
    pDestShell->GetDoc()->UpdateFlds();

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

BOOL SwFEShell::Paste( SwDoc* pClpDoc )
{
    SET_CURR_SHELL( this );
    ASSERT( pClpDoc, "kein Clipboard-Dokument"  );

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

    BOOL bRet = TRUE, bDelTbl = TRUE;
    StartAllAction();
    GetDoc()->StartUndo( UNDO_INSGLOSSARY );
    GetDoc()->LockExpFlds();

    FOREACHPAM_START(this)

        if( pSrcNd &&
            0 != ( pDestNd = GetDoc()->IsIdxInTbl( PCURCRSR->GetPoint()->nNode )))
        {
            SwPosition aDestPos( *PCURCRSR->GetPoint() );

            BOOL bParkTblCrsr = FALSE;
            const SwStartNode* pSttNd =  PCURCRSR->GetNode()->FindTableBoxStartNode();

            // Tabelle in Tabelle kopieren
            // lasse ueber das Layout die Boxen suchen
            SwSelBoxes aBoxes;
            if( IsTableMode() )     // Tabellen-Selecktion ??
            {
                GetTblSel( *this, aBoxes );
                ParkTblCrsr();
                bParkTblCrsr = TRUE;
            }
            else if( !PCURCRSR->HasMark() && PCURCRSR->GetNext() == PCURCRSR &&
                    !pSrcNd->GetTable().IsTblComplex() )
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
                PaMCorrAbs( SwNodeIndex( *pDestNd ),
                            SwNodeIndex( *pDestNd->EndOfSectionNode() ),
                            aPos );
            }

            bRet = GetDoc()->InsCopyOfTbl( aDestPos, aBoxes, &pSrcNd->GetTable(),
                                            FALSE, FALSE );

            if( bParkTblCrsr )
                GetCrsr();
            else
            {
                // und wieder in die Box zurueck
                aNdIdx = *pSttNd;
                SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aNdIdx );
                SwPosition aPos( aNdIdx, SwIndex( pCNd, 0 ));
                PaMCorrAbs( PCURCRSR->GetPoint()->nNode, aPos );
            }

            break;      // aus der "while"-Schleife heraus
        }
        else if( *aCpyPam.GetPoint() == *aCpyPam.GetMark() &&
                 pClpDoc->GetSpzFrmFmts()->Count() )
        {
            // so langsam sollte mal eine DrawView erzeugt werden
            if( !Imp()->GetDrawView() )
                MakeDrawView();

            Size aSiz( 0, GetCharRect().Top() );
            for ( USHORT i = 0; i < pClpDoc->GetSpzFrmFmts()->Count(); ++i )
            {
                BOOL bInsWithFmt = TRUE;
                const SwFrmFmt& rCpyFmt = *(*pClpDoc->GetSpzFrmFmts())[i];

                if( Imp()->GetDrawView()->IsGroupEntered() &&
                    RES_DRAWFRMFMT == rCpyFmt.Which() &&
                    FLY_IN_CNTNT != rCpyFmt.GetAnchor().GetAnchorId() )
                {
                    const SdrObject* pSdrObj = rCpyFmt.FindSdrObject();
                    if( pSdrObj )
                    {
                        SdrObject* pNew = GetDoc()->CloneSdrObj( *pSdrObj,
                                                            FALSE, FALSE );
                        pNew->NbcMove( aSiz );
                        Imp()->GetDrawView()->InsertObject( pNew,
                                                        *Imp()->GetPageView() );
                        bInsWithFmt = FALSE;
                    }
                }

                if( bInsWithFmt  )
                {
                    SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
                    if( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
                        FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
                        FLY_IN_CNTNT == aAnchor.GetAnchorId() )
                    {
                        SwPosition* pPos = PCURCRSR->GetPoint();
                        if( RES_DRAWFRMFMT == rCpyFmt.Which() &&
                            GetDoc()->IsInHeaderFooter( pPos->nNode ))
                            continue;       // Header / Footer  -> nicht kopieren!!!

                        aAnchor.SetAnchor( pPos );
                    }
                    else if( FLY_PAGE == aAnchor.GetAnchorId() )
                    {
                        aAnchor.SetPageNum( GetPhyPageNum() );
                    }
                    else if( FLY_AT_FLY == aAnchor.GetAnchorId() )
                    {
                        Point aPt;
                        lcl_SetAnchor( *PCURCRSR->GetPoint(), *PCURCRSR->GetNode(),
                                        0, aPt, *this, aAnchor, aPt, FALSE );
                    }

                    SwFrmFmt * pNew = GetDoc()->CopyLayoutFmt( rCpyFmt, aAnchor );

                    //Kann 0 sein, weil Draws in Kopf-/Fusszeilen nicht erlaubt sind.
                    if( pNew )
                    {
                        if( RES_FLYFRMFMT == pNew->Which() )
                        {
                            const Point aPt( GetCrsrDocPos() );
                            SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pNew)->
                                                        GetFrm( &aPt, FALSE );
                            if( pFlyFrm )
                                SelectFlyFrm( *pFlyFrm, TRUE );
                            // immer nur den ersten Fly-Frame nehmen; die anderen
                            // wurden ueber Fly in Fly ins ClipBoard kopiert !
                            break;
                        }
                        else
                        {
                            ASSERT( RES_DRAWFRMFMT == pNew->Which(), "Neues Format.");
                            SdrObject *pObj = pNew->FindSdrObject();
                            SwDrawView  *pDV = Imp()->GetDrawView();
                            pDV->MarkObj( pObj, pDV->GetPageView( pObj->GetPage() ) );
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
                bDelTbl = FALSE;
            }

            SwPosition& rInsPos = *PCURCRSR->GetPoint();
            const SwStartNode* pBoxNd = GetDoc()->GetNodes()[
                                rInsPos.nNode ]->FindTableBoxStartNode();
            if( pBoxNd && 2 == pBoxNd->EndOfSectionIndex() -
                                pBoxNd->GetIndex() &&
                aCpyPam.GetPoint()->nNode != aCpyPam.GetMark()->nNode )
            {
                // es wird mehr als 1 Node in die akt. Box kopiert. Dann
                // muessen die BoxAttribute aber entfernt werden.
                GetDoc()->ClearBoxNumAttrs( rInsPos.nNode );
            }
            pClpDoc->Copy( aCpyPam, rInsPos );
            SaveTblBoxCntnt( &rInsPos );
        }

    FOREACHPAM_END()
    GetDoc()->EndUndo( UNDO_INSGLOSSARY );

    // wurden neue Tabellenformeln eingefuegt ?
    if( pTblFldTyp->GetDepends() )
    {
        // alte Actions beenden; die Tabellen-Frames werden angelegt und
        // eine Selection kann erzeugt werden
        for( USHORT nActCnt = 0; ActionPend(); ++nActCnt )
            EndAllAction();

        for( ; nActCnt; --nActCnt )
            StartAllAction();
    }
    GetDoc()->UnlockExpFlds();
    GetDoc()->UpdateFlds();
    EndAllAction();

    return bRet;
}

BOOL SwFEShell::GetDrawObjGraphic( ULONG nFmt, Graphic& rGrf ) const
{
    ASSERT( Imp()->HasDrawView(), "GetDrawObjGraphic without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
    BOOL bConvert = TRUE;
    if( rMrkList.GetMarkCount() )
    {
        if( rMrkList.GetMarkCount() == 1 &&
            rMrkList.GetMark( 0 )->GetObj()->IsWriterFlyFrame() )
        {
            // Rahmen selektiert
            if( CNT_GRF == GetCntType() )
            {
                Graphic aGrf( GetGraphic() );
                if( FORMAT_GDIMETAFILE == nFmt )
                {
                    if( GRAPHIC_BITMAP != aGrf.GetType() )
                    {
                        rGrf = aGrf;
                        bConvert = FALSE;
                    }
                    else if( GetWin() )
                    {
                        Size aSz;
                        Point aPt;
                        GetGrfSize( aSz );

                        VirtualDevice aVirtDev;
                        aVirtDev.EnableOutput( FALSE );

                        MapMode aTmp( GetWin()->GetMapMode() );
                        aTmp.SetOrigin( aPt );
                        aVirtDev.SetMapMode( aTmp );

                        GDIMetaFile aMtf;
                        aMtf.Record( &aVirtDev );
                        aGrf.Draw( &aVirtDev, aPt, aSz );
                        aMtf.Stop();
                        aMtf.SetPrefMapMode( aTmp );
                        aMtf.SetPrefSize( aSz );
#ifndef VCL
                        aMtf.SetPrefPalette( OutputDevice::GetDitherPalette() );
#endif
                        rGrf = aMtf;
                    }
                }
                else if( GRAPHIC_BITMAP == aGrf.GetType() )
                {
                    rGrf = aGrf;
                    bConvert = FALSE;
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
                        bConvert = FALSE;
                    }
                }
            }
        }
        else if( FORMAT_GDIMETAFILE == nFmt )
            rGrf = Imp()->GetDrawView()->GetAllMarkedMetaFile();
        else if( FORMAT_BITMAP == nFmt )
            rGrf = Imp()->GetDrawView()->GetAllMarkedBitmap();
    }
    return bConvert;
}

void SwFEShell::Paste( SvStream& rStrm, USHORT nAction, const Point* pPt )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo();

    SvtPathOptions aPathOpt;
    FmFormModel* pModel = new FmFormModel( aPathOpt.GetPalettePath(),
                    (SfxItemPool*)0, (SvPersist*)GetDoc()->GetDocShell() );
    pModel->SetStreamingSdrModel(TRUE);
    rStrm.Seek(0);
    pModel->GetItemPool().Load( rStrm );
    rStrm >> *pModel;
    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    Point aPos( pPt ? *pPt : GetCharRect().Pos() );
    SdrView *pView = Imp()->GetDrawView();

    //Drop auf bestehendes Objekt: Objekt ersetzen oder neu Attributieren.
    if( 1 == pModel->GetPage(0)->GetObjCount() &&
        1 == pView->GetMarkList().GetMarkCount() )
    {
        SdrObject* pClpObj = pModel->GetPage(0)->GetObj(0);
        SdrObject* pOldObj = pView->GetMarkList().GetMark( 0 )->GetObj();

        if( SW_PASTESDR_SETATTR == nAction && pOldObj->IsWriterFlyFrame() )
            nAction = SW_PASTESDR_REPLACE;

        switch( nAction )
        {
        case SW_PASTESDR_REPLACE:
            {
                const SwFrmFmt* pFmt;
                const SwFrm* pAnchor;
                if( pOldObj->IsWriterFlyFrame() )
                {
                    pFmt = FindFrmFmt( pOldObj );

                    Point aNullPt;
                    SwFlyFrm* pFlyFrm = ((SwFlyFrmFmt*)pFmt)->GetFrm( &aNullPt );
                    pAnchor = pFlyFrm->GetAnchor();

                    if( pAnchor->FindFooterOrHeader() )
                    {
                        // wenn TextRahmen in der Kopf/Fusszeile steht, dann
                        // nicht ersetzen, sondern nur einfuegen
                        nAction = SW_PASTESDR_INSERT;
                        break;
                    }
                }

                SdrObject* pNewObj = pClpObj->Clone();
                Rectangle aOldObjRect( pOldObj->GetBoundRect() );
                Size aOldObjSize( aOldObjRect.GetSize() );
                Rectangle aNewRect( pNewObj->GetBoundRect() );
                Size aNewSize( aNewRect.GetSize() );

                Fraction aScaleWidth( aOldObjSize.Width(), aNewSize.Width() );
                Fraction aScaleHeight( aOldObjSize.Height(), aNewSize.Height());
                pNewObj->NbcResize( aNewRect.TopLeft(), aScaleWidth, aScaleHeight);

                Point aVec = aOldObjRect.TopLeft() - aNewRect.TopLeft();
                pNewObj->NbcMove(Size(aVec.X(), aVec.Y()));

                pNewObj->SetLayer( pOldObj->GetLayer() );

                if( pOldObj->IsWriterFlyFrame() )
                {
                    // Attribute sichern und dam SdrObject setzen
                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                            RES_SURROUND, RES_ANCHOR );
                    aFrmSet.Set( pFmt->GetAttrSet() );

                    Point aNullPt;
                    if( pAnchor->IsTxtFrm() && ((SwTxtFrm*)pAnchor)->IsFollow() )
                    {
                        const SwTxtFrm* pTmp = (SwTxtFrm*)pAnchor;
                        do {
                            pTmp = pTmp->FindMaster();
                            ASSERT( pTmp, "Where's my Master?" );
                        } while( pTmp->IsFollow() );
                        pAnchor = pTmp;
                    }
                    if( pOldObj->ISA( SdrCaptionObj ))
                        aNullPt = ((SdrCaptionObj*)pOldObj)->GetTailPos();
                    else
                        aNullPt = aOldObjRect.TopLeft();
                    pNewObj->NbcSetRelativePos( aNullPt - pAnchor->Frm().Pos() );
                    pNewObj->NbcSetAnchorPos( pAnchor->Frm().Pos() );

                    UINT32 nOrdNum = pOldObj->GetOrdNum();

                    DelSelectedObj();

                    pFmt = GetDoc()->Insert( *GetCrsr(), *pNewObj, &aFrmSet );

                    // die Ordnungsnummer (Z-Order) noch uebertragen
                    // JP 04.07.98: klappt aber nicht richtig!
                    // pNewObj->SetOrdNum( nOrdNum );
                }
                else
                    pView->ReplaceObject( pOldObj, *Imp()->GetPageView(),
                                            pNewObj, TRUE );
            }
            break;

        case SW_PASTESDR_SETATTR:
            {
                SfxItemSet aSet( GetAttrPool() );
//-/                pClpObj->TakeAttributes( aSet, TRUE, FALSE );
                aSet.Put(pClpObj->GetItemSet());
                pView->SetAttributes( aSet, FALSE );
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
        GetDoc()->SetNoDrawUndoObj( TRUE );

        pView->Paste( *pModel, aPos );

        ULONG nCnt = pView->GetMarkList().GetMarkCount();
        if( nCnt )
        {
            const Point aNull( 0, 0 );
            for( ULONG i=0; i < nCnt; ++i )
            {
                SdrObject *pObj = pView->GetMarkList().GetMark(i)->GetObj();
                pObj->ImpSetAnchorPos( aNull );
            }

            pView->SetCurrentObj( OBJ_GRUP, SdrInventor );
            if ( nCnt > 1 )
                pView->GroupMarked();
            SdrObject *pObj = pView->GetMarkList().GetMark(0)->GetObj();
            pObj->SetLayer( pObj->ISA( SdrUnoObj )
                                ? GetDoc()->GetControlsId()
                                : GetDoc()->GetHeavenId() );
            const Rectangle &rSnap = pObj->GetSnapRect();
            const Size aDiff( rSnap.GetWidth()/2, rSnap.GetHeight()/2 );
            pView->MoveMarkedObj( aDiff );
            ImpEndCreate();
        }
        GetDoc()->SetNoDrawUndoObj( FALSE );
    }
    EndUndo();
    EndAllAction();
    delete pModel;
}

BOOL SwFEShell::Paste( const Graphic &rGrf )
{
    SET_CURR_SHELL( this );
    SdrObject* pObj;
    SdrView *pView = Imp()->GetDrawView();

    BOOL bRet = 1 == pView->GetMarkList().GetMarkCount() &&
        (pObj = pView->GetMarkList().GetMark( 0 )->GetObj())->IsClosedObj() &&
        !pObj->ISA( SdrOle2Obj );

    if( bRet )
    {
        XOBitmap aXOBitmap( rGrf.GetBitmap() );
        SfxItemSet aSet( GetAttrPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP );
        aSet.Put( XFillStyleItem( XFILL_BITMAP ));
        aSet.Put( XFillBitmapItem( aEmptyStr, aXOBitmap ));
        pView->SetAttributes( aSet, FALSE );
    }
    return bRet;
}

#ifdef USED
BOOL SwFEShell::Paste( SotDataObject& rObj, const Point& rPt )
{
    SET_CURR_SHELL( this );
    BOOL bRet = FALSE;

    SvData aData( XFillExchangeData::RegisterClipboardFormatName() );
    XFillExchangeData* pFillData = NULL;

    if( rObj.GetData( &aData ) )
    {
        if (aData.GetData( (SvDataCopyStream**) &pFillData, XFillExchangeData::StaticType(), TRANSFER_MOVE))
        {
/*              BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
            AddUndo(new SdrUndoAttrObj(*pPickObj));
            EndUndo();*/

            XFillAttrSetItem* pSetItem = pFillData->GetXFillAttrSetItem();
            SfxItemSet rSet = pSetItem->GetItemSet();

            XFillStyle eFill= ((XFillStyleItem&) rSet.Get(XATTR_FILLSTYLE)).GetValue();

            if (eFill == XFILL_SOLID)
            {
                const XFillColorItem& rColItem = (XFillColorItem&) rSet.Get(XATTR_FILLCOLOR);
                Color aColor = rColItem.GetValue();
                String aName = rColItem.GetName();

                SdrView *pSdrView = Imp()->GetDrawView();

                if(pSdrView)
                {
                    SdrObject* pPickObj = NULL;
                    // Ist ein Objekt getroffen worden?
                    SdrPageView* pPV = NULL;
                    pSdrView->PickObj( rPt, pPickObj, pPV);

                    if ( pPickObj )
                    {
/*                          SfxItemSet aSet(pDoc->GetPool());

                        BOOL bClosed = pPickObj->IsClosedObj();
                        SdWindow* pWin = pViewSh->GetActiveWindow();
                        USHORT nHitLog = USHORT ( pWin->PixelToLogic(Size(HITPIX,0)).Width() );
                        const long  n2HitLog = nHitLog * 2;
                        Point aHitPosR( aPos );
                        Point aHitPosL( aPos );
                        Point aHitPosT( aPos );
                        Point aHitPosB( aPos );

                        aHitPosR.X() += n2HitLog;
                        aHitPosL.X() -= n2HitLog;
                        aHitPosT.Y() += n2HitLog;
                        aHitPosB.Y() -= n2HitLog;

                        const SetOfByte* pVisiLayer = &GetPageViewPvNum(0)->GetVisibleLayers();

                        if (bClosed                                          &&
                            pPickObj->IsHit( aHitPosR, nHitLog, pVisiLayer ) &&
                            pPickObj->IsHit( aHitPosL, nHitLog, pVisiLayer ) &&
                            pPickObj->IsHit( aHitPosT, nHitLog, pVisiLayer ) &&
                            pPickObj->IsHit( aHitPosB, nHitLog, pVisiLayer ) )
                        {
                            // Flaechenfuellung
                            aSet.Put(XFillColorItem(aName, aColor));
                            aSet.Put(XFillStyleItem(XFILL_SOLID));
                        }
                        else
                        {
                            // Linienstil hinzufuegen
                            aSet.Put(XLineColorItem(aName, aColor));
                        }

                        // Textfarbe hinzufuegen
    //                    aSet.Put(SvxColorItem(aColor, ITEMID_COLOR));

                        pPickObj->SetAttributes(aSet, FALSE);*/
                        bRet = TRUE;
                    }
                }
                if (!bRet)  // Kein DrawObjekt
                {
                    SvxBrushItem aBrushItem(aColor, SID_ATTR_BRUSH);
                    SfxViewShell* pViewShell = GetSfxViewShell();
                    DBG_ASSERT(pViewShell, "no SfxViewShell?")
                    if(pViewShell)
                        pViewShell->GetViewFrame()->GetDispatcher()->
                            Execute(    SID_ATTR_BRUSH, SFX_CALLMODE_SLOT,
                                                &aBrushItem, 0L);
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}
#endif

