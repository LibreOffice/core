/*************************************************************************
 *
 *  $RCSfile: fefly1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-01 12:23:37 $
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

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
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
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>       // Strukturen zum Sichern beim Move/Delete
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//Zum anmelden von Flys in Flys in ...
//definiert in layout/frmtool.cxx
void RegistFlys( SwPageFrm*, const SwLayoutFrm* );

/***********************************************************************
#*  Class       :  SwDoc
#*  Methode     :  UseSpzLayoutFmt
#*  Beschreibung:  Anhand des Request werden zu dem Format entsprechende
#*      Aenderungen an den Spezifischen Layouts vorgenommen.
#*  Datum       :  MA 23. Sep. 92
#*  Update      :  JP 09.03.98
#***********************************************************************/

sal_Bool lcl_SetNewFlyPos( const SwNode& rNode, SwFmtAnchor& rAnchor,
                        const Point& rPt )
{
    sal_Bool bRet = sal_False;
    const SwStartNode* pStNode = rNode.FindFlyStartNode();
    if( pStNode )
    {
        SwPosition aPos( *pStNode );
        rAnchor.SetAnchor( &aPos );
        bRet = sal_True;
    }
    else
    {
        const SwCntntFrm* pCFrm = rNode.GetCntntNode()->GetFrm( &rPt, 0, sal_False );
        const SwPageFrm *pPg = pCFrm ? pCFrm->FindPageFrm() : 0;

        rAnchor.SetPageNum( pPg ? pPg->GetPhyPageNum() : 1 );
        rAnchor.SetType( FLY_PAGE );
    }
    return bRet;
}

BOOL lcl_FindAnchorPos( SwDoc& rDoc, const Point& rPt, const SwFrm& rFrm,
                        SfxItemSet& rSet )
{
    BOOL bRet = TRUE;
    SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();
    const SwFrm *pNewAnch;

    //Neuen Anker ermitteln
    Point aTmpPnt( rPt );
    switch( nNew )
    {
    case FLY_IN_CNTNT:  // sollte der nicht auch mit hinein?
    case FLY_AT_CNTNT:
    case FLY_AUTO_CNTNT: // LAYER_IMPL
        {
            //Ausgehend von der linken oberen Ecke des Fly den
            //dichtesten CntntFrm suchen.
            const SwFrm* pFrm = rFrm.IsFlyFrm() ? ((SwFlyFrm&)rFrm).GetAnchor()
                                                : &rFrm;
            pNewAnch = ::FindAnchor( pFrm, aTmpPnt );
            if( pNewAnch->IsProtected() )
            {
                bRet = FALSE;
                break;
            }

            SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
            if( FLY_AUTO_CNTNT == nNew || FLY_IN_CNTNT == nNew )
            {
                // es muss ein TextNode gefunden werden, denn nur in diesen
                // ist ein Inhaltsgebundene Frames zu verankern
                SwCrsrMoveState aState( MV_SETONLYTEXT );
                aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
                if( !pNewAnch->GetCrsrOfst( &aPos, aTmpPnt, &aState ) )
                {
                    SwCntntNode* pCNd = ((SwCntntFrm*)pNewAnch)->GetNode();
                    if( pNewAnch->Frm().Bottom() < aTmpPnt.Y() )
                        pCNd->MakeStartIndex( &aPos.nContent );
                    else
                        pCNd->MakeEndIndex( &aPos.nContent );
                }
            }
            aNewAnch.SetAnchor( &aPos );
        }
        break;

    case FLY_AT_FLY: // LAYER_IMPL
        {
            //Ausgehend von der linken oberen Ecke des Fly den
            //dichtesten SwFlyFrm suchen.
            SwCrsrMoveState aState( MV_SETONLYTEXT );
            SwPosition aPos( rDoc.GetNodes() );
            aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
            rDoc.GetRootFrm()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
            pNewAnch = ::FindAnchor(
                aPos.nNode.GetNode().GetCntntNode()->GetFrm( 0, 0, sal_False ),
                aTmpPnt )->FindFlyFrm();

            if( pNewAnch && &rFrm != pNewAnch && !pNewAnch->IsProtected() )
            {
                aPos.nNode = *((SwFlyFrm*)pNewAnch)->GetFmt()->GetCntnt().
                                GetCntntIdx();
                aNewAnch.SetAnchor( &aPos );
                break;
            }
        }

        aNewAnch.SetType( nNew = FLY_PAGE );
        // no break

    case FLY_PAGE:
        pNewAnch = rFrm.FindPageFrm();
        aNewAnch.SetPageNum( pNewAnch->GetPhyPageNum() );
        break;

    default:
        ASSERT( !&rDoc, "Falsche ID fuer neuen Anker." );
    }

    rSet.Put( aNewAnch );
    return bRet;
}

sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet )
{
    const SwFrmFmt& rFmt = *rFly.GetFmt();
    const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    RndStdIds nNew = ((SwFmtAnchor&)rSet.Get( RES_ANCHOR )).GetAnchorId();

    if( nOld == nNew )
        return sal_False;

    SwDoc* pDoc = (SwDoc*)rFmt.GetDoc();

#ifndef PRODUCT
    ASSERT( !(nNew == FLY_PAGE &&
        (FLY_AT_CNTNT==nOld || FLY_AUTO_CNTNT==nOld || FLY_IN_CNTNT==nOld ) &&
        pDoc->IsInHeaderFooter( rOldAnch.GetCntntAnchor()->nNode )),
            "Unerlaubter Ankerwechsel in Head/Foot." );
#endif

    return ::lcl_FindAnchorPos( *pDoc, rFly.Frm().Pos(), rFly, rSet );
}

void SwFEShell::SelectFlyFrm( SwFlyFrm& rFrm, sal_Bool bNew )
{
    SET_CURR_SHELL( this );

    //  Wenn es ein neuer Rahmen ist, so soll er selektiert sein.
    //  !!Rahmen immer selektieren, wenn sie nicht selektiert sind.
    //  - Es kann ein neuer 'alter' sein weil der Anker gewechselt wurde.
    //  - 'alte' Rahmen sind vorher immer selektiert denn sonst wird nix
    //    an ihnen veraendert.
    //  Der Rahmen darf nicht per Dokumentposition selektiert werden, weil er
    //  auf jedenfall selektiert sein muss!
    SwViewImp *pImp = Imp();
    if( GetWin() && (bNew || !pImp->GetDrawView()->HasMarkedObj()) )
    {
        ASSERT( rFrm.IsFlyFrm(), "SelectFlyFrm will einen Fly" );

        //Wenn der Fly bereits selektiert ist gibt es hier ja wohl nichts
        //zu tun.
        if ( FindFlyFrm() == &rFrm )
            return;

        //Damit der Anker ueberhaupt noch gepaintet wird.
        if( rFrm.IsFlyInCntFrm() && rFrm.GetAnchor() )
             rFrm.GetAnchor()->SetCompletePaint();

        //Hier wurde immer kalkuliert. Leider ist der Sonderfall Fly in Fly mit
        //Spalten u.U. sehr kritisch wenn der innenliegende zuerst formatiert
        //wird. Um kein Risiko einzugehen entschaerfen wir nur diesen Sonderfall.
        if( !rFrm.GetAnchor()->IsInFly() )
            rFrm.Calc();

        if( pImp->GetDrawView()->HasMarkedObj() )
            pImp->GetDrawView()->UnmarkAll();

        pImp->GetDrawView()->MarkObj( rFrm.GetVirtDrawObj(),
                                      pImp->GetPageView(), sal_False, sal_False );
        KillPams();
        ClearMark();
        SelFlyGrabCrsr();
    }
}

/*************************************************************************
|*
|*  SwFEShell::FindFlyFrm()
|*
|*  Beschreibung        Liefert den Fly wenn einer Selektiert ist.
|*  Ersterstellung      MA 03. Nov. 92
|*  Letzte Aenderung    MA 05. Mar. 96
|*
*************************************************************************/

SwFlyFrm *SwFEShell::FindFlyFrm() const
{
    if ( Imp()->HasDrawView() )
    {
        // Ein Fly ist genau dann erreichbar, wenn er selektiert ist.
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
        if( rMrkList.GetMarkCount() != 1 )
            return 0;

        SdrObject *pO = rMrkList.GetMark( 0 )->GetObj();
        return pO->IsWriterFlyFrame() ? ((SwVirtFlyDrawObj*)pO)->GetFlyFrm() : 0;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::IsFlyInFly()
|*
|*  Beschreibung        Liefert sal_True, wenn der aktuelle Fly an einem anderen
|*                      verankert werden koennte (also innerhalb ist)
|*  Ersterstellung      AMA 11. Sep. 97
|*  Letzte Aenderung    AMA 14. Jan. 98
|*
*************************************************************************/

const SwFrmFmt* SwFEShell::IsFlyInFly()
{
    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return NULL;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
    if ( !rMrkList.GetMarkCount() )
    {
        SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
        if( !pCntnt )
            return NULL;
        SwFlyFrm *pFly = pCntnt->FindFlyFrm();
        if ( !pFly )
            return NULL;
        return pFly->GetFmt();
    }
    else if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetObj()) )
        return NULL;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();

    SwFrmFmt *pFmt = FindFrmFmt( pObj );
    if( pFmt && FLY_AT_FLY == pFmt->GetAnchor().GetAnchorId() )
    {
        SwFrm* pFly = pObj->IsWriterFlyFrame() ?
            ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetAnchor() :
            ((SwDrawContact*)GetUserCall(pObj))->GetAnchor();
        ASSERT( pFly, "IsFlyInFly: Where's my anchor?" );
        ASSERT( pFly->IsFlyFrm(), "IsFlyInFly: Funny anchor!" );
        return ((SwFlyFrm*)pFly)->GetFmt();
    }

    Point aTmpPos = pObj->GetBoundRect().TopLeft();

    SwFrm *pTxtFrm;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwNodeIndex aSwNodeIndex( GetDoc()->GetNodes() );
        SwPosition aPos( aSwNodeIndex );
        Point aPoint( aTmpPos );
        aPoint.X() -= 1;                    //nicht im Fly landen!!
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
        pTxtFrm = aPos.nNode.GetNode().GetCntntNode()->GetFrm( 0, 0, sal_False );
    }
    const SwFrm *pTmp = ::FindAnchor( pTxtFrm, aTmpPos );
    const SwFlyFrm *pFly = pTmp->FindFlyFrm();
    if( pFly )
        return pFly->GetFmt();
    return NULL;
}

/*************************************************************************
|*
|*  SwFEShell::SetFlyPos
|*
|*  Ersterstellung      MA 14. Jan. 93
|*  Letzte Aenderung    MA 14. Feb. 95
|*
*************************************************************************/

void SwFEShell::SetFlyPos( const Point& rAbsPos )
{
    SET_CURR_SHELL( this );

    //Bezugspunkt in Dokumentkoordinaten bestimmen
    SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
    if( !pCntnt )
        return;
    SwFlyFrm *pFly = pCntnt->FindFlyFrm();
    if ( !pFly )
        return;

    SwSaveHdl aSaveX( Imp() );

    //Bei Absatzgebundenen Flys muss ausgehend von der absoluten
    //Position ein neuer Anker gesetzt werden. Anker und neue RelPos werden
    //vom Fly selbst berechnet und gesetzt.
    if ( pFly->IsFlyAtCntFrm() )
        ((SwFlyAtCntFrm*)pFly)->SetAbsPos( rAbsPos );
    else
    {
        const SwFrm *pAnch = pFly->GetAnchor();
        pAnch->Calc();
        Point aOrient( pAnch->Frm().Pos() );

        if ( pFly->IsFlyInCntFrm() )
            aOrient.X() = rAbsPos.X();

        //RelPos errechnen.
        aOrient.X() = rAbsPos.X() - aOrient.X();
        aOrient.Y() = rAbsPos.Y() - aOrient.Y();
        pFly->ChgRelPos( aOrient );
    }
    pFly->Calc();
    CallChgLnk();       // rufe das AttrChangeNotify auf der UI-Seite.
}

/*************************************************************************
|*
|*  SwFEShell::FindAnchorPos
|*
|*  Ersterstellung      AMA 24. Sep. 97
|*  Letzte Aenderung    AMA 24. Sep. 97
|*
*************************************************************************/

Point SwFEShell::FindAnchorPos( const Point& rAbsPos, sal_Bool bMoveIt )
{
    Point aRet;

    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return aRet;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
    if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetObj()) )
        return aRet;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
    SwFmt *pFmt = ::FindFrmFmt( pObj );
    SwFmtAnchor aAnch( pFmt->GetAnchor() );
    RndStdIds nAnchorId = aAnch.GetAnchorId();

    if ( FLY_IN_CNTNT == nAnchorId )
        return aRet;

    sal_Bool bFlyFrame = pObj->IsWriterFlyFrame();

    SwFlyFrm *pFly;
    const SwFrm *pOldAnch;
    const SwFrm* pFooterOrHeader = NULL;

    if( bFlyFrame )
    {
        //Bezugspunkt in Dokumentkoordinaten bestimmen
        SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
        if( !pCntnt )
            return aRet;
        pFly = pCntnt->FindFlyFrm();
        if ( !pFly )
            return aRet;
        pOldAnch = pFly->GetAnchor();
        if( !pOldAnch )
            return aRet;
        if( FLY_PAGE != nAnchorId )
            pFooterOrHeader = pCntnt->FindFooterOrHeader();
    }

    //Ausgehend von der linken oberen Ecke des Fly den
    //dichtesten SwFlyFrm suchen.
    SwCntntFrm *pTxtFrm;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwPosition aPos( GetDoc()->GetNodes().GetEndOfExtras() );
        Point aTmpPnt( rAbsPos );
        GetLayout()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
        pTxtFrm = aPos.nNode.GetNode().GetCntntNode()->GetFrm(0,&aPos,FALSE );
    }
    const SwFrm *pNewAnch;
    if( pTxtFrm )
    {
        if( FLY_PAGE == nAnchorId )
            pNewAnch = pTxtFrm->FindPageFrm();
        else
        {
            pNewAnch = ::FindAnchor( pTxtFrm, rAbsPos );

            if( FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
                pNewAnch = pNewAnch->FindFlyFrm();
        }
    }
    else
        pNewAnch = 0;

    if( pNewAnch && !pNewAnch->IsProtected() )
    {
        const SwFlyFrm* pCheck = bFlyFrame ? pNewAnch->FindFlyFrm() : 0;
        // Falls wir innerhalb eines Rahmens landen, muss sichergestellt werden,
        // dass der Rahmen nicht in seinem eigenen Inhalt landet!
        while( pCheck )
        {
            if( pCheck == pFly )
                break;
            const SwFrm *pTmp = pCheck->GetAnchor();
            pCheck = pTmp ? pTmp->FindFlyFrm() : NULL;
        }
        // Es darf nicht aus einer Kopf-/Fusszeile in einen anderen Bereich
        // gewechselt werden, es darf nicht in eine Kopf-/Fusszeile hinein-
        // gewechselt werden.
        if( !pCheck &&
            pFooterOrHeader == pNewAnch->FindFooterOrHeader() )
        {
            aRet = pNewAnch->Frm().Pos();
            if( bMoveIt || nAnchorId == FLY_AUTO_CNTNT )
            {
                SwFmtAnchor aAnch( pFmt->GetAnchor() );
                switch ( nAnchorId )
                {
                    case FLY_AT_CNTNT:
                    {
                        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                        pPos->nNode = *pTxtFrm->GetNode();
                        pPos->nContent.Assign(0,0);
                        break;
                    }
                    case FLY_PAGE:
                    {
                        aAnch.SetPageNum( ((const SwPageFrm*)pNewAnch)->
                                          GetPhyPageNum() );
                        break;
                    }
                    case FLY_AT_FLY:
                    {
                        SwPosition aPos( *((SwFlyFrm*)pNewAnch)->GetFmt()->
                                                  GetCntnt().GetCntntIdx() );
                        aAnch.SetAnchor( &aPos );
                        break;
                    }
                    case FLY_AUTO_CNTNT:
                    {
                        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                        Point aTmpPnt( rAbsPos );
                        if( pTxtFrm->GetCrsrOfst( pPos, aTmpPnt, NULL ) )
                        {
                            SwRect aTmpRect;
                            pTxtFrm->GetCharRect( aTmpRect, *pPos );
                            aRet = aTmpRect.Pos();
                        }
                        else
                        {
                            pPos->nNode = *pTxtFrm->GetNode();
                            pPos->nContent.Assign(0,0);
                        }
                        break;
                    }
                }
                if( bMoveIt )
                {
                    StartAllAction();
                    pFmt->GetDoc()->SetAttr( aAnch, *pFmt );
                    if( nAnchorId == FLY_AUTO_CNTNT && bFlyFrame &&
                        pFly->IsFlyAtCntFrm() )
                        ((SwFlyAtCntFrm*)pFly)->CheckCharRect();
                    EndAllAction();
                }
            }

            SwRect aTmpRect( aRet, rAbsPos );
            if( aTmpRect.HasArea() )
                MakeVisible( aTmpRect );
#ifndef PRODUCT
            //TODO: That doesn't seem to be intended
            if( Color(COL_TRANSPARENT) != GetOut()->GetLineColor() )
            {
                ASSERT( FALSE, "Hey, Joe: Where's my Null Pen?" );
                GetOut()->SetLineColor( Color(COL_TRANSPARENT) );
            }
/*          if( PEN_NULL != GetOut()->GetPen().GetStyle() )
            {
                ASSERT( FALSE, "Hey, Joe: Where's my Null Pen?" );
                Pen aPen( PEN_NULL );
                GetOut()->SetPen( aPen );
            }
*/#endif
        }
    }

    return aRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  NewFlyFrm
#*  Beschreibung:
#*  Datum       :  MA 03. Nov. 92
#*  Update      :  JP 11. Aug. 93
#***********************************************************************/

const SwFrmFmt *SwFEShell::NewFlyFrm( const SfxItemSet& rSet, sal_Bool bAnchValid,
                           SwFrmFmt *pParent )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    const SwPosition& rPos = *pCrsr->Start();
    const Point aPt( GetCrsrDocPos() );

    SwSelBoxes aBoxes;
    sal_Bool bMoveCntnt = sal_True;
    if( IsTableMode() )
    {
        GetTblSel( *this, aBoxes );
        if( aBoxes.Count() )
        {
            // die Crsr muessen noch aus dem Loeschbereich entfernt
            // werden. Setze sie immer hinter/auf die Tabelle; ueber die
            // Dokument-Position werden sie dann immer an die alte
            // Position gesetzt.
            ParkCrsr( SwNodeIndex( *aBoxes[0]->GetSttNd() ));
//          KillPams();
        }
        else
            bMoveCntnt = sal_False;
    }
    else if( !pCrsr->HasMark() && pCrsr->GetNext() == pCrsr )
        bMoveCntnt = sal_False;

    SwFmtAnchor& rAnch = (SwFmtAnchor&)rSet.Get( RES_ANCHOR );
    RndStdIds eRndId = rAnch.GetAnchorId();
    switch( eRndId )
    {
    case FLY_PAGE:
        if( !rAnch.GetPageNum() )       //HotFix: Bug in UpdateByExample
            rAnch.SetPageNum( 1 );
        break;

    case FLY_AT_FLY:
    case FLY_AT_CNTNT:
    case FLY_AUTO_CNTNT:
    case FLY_IN_CNTNT:
        if( !bAnchValid )
        {
            if( FLY_AT_FLY != eRndId )
                rAnch.SetAnchor( &rPos );
            else if( lcl_SetNewFlyPos( rPos.nNode.GetNode(), rAnch, aPt ) )
                eRndId = FLY_PAGE;
        }
        break;

#ifndef PRODUCT
    default:
            ASSERT( !this, "Was sollte das fuer ein Fly werden?" );
#endif
    }

    SwFlyFrmFmt *pRet;
    if( bMoveCntnt )
    {
        GetDoc()->StartUndo( UNDO_INSLAYFMT );
        SwFmtAnchor* pOldAnchor = 0;
        sal_Bool bHOriChgd = sal_False, bVOriChgd = sal_False;
        SwFmtVertOrient aOldV;
        SwFmtHoriOrient aOldH;

        if( FLY_PAGE != eRndId )
        {
            // erstmal als mit Seitenbindung, Absatz/Zeichenbindung erst wenn
            // alles verschoben ist. Dann ist die Position gueltig!
            // JP 13.05.98: ggfs. auch noch die Hori/Vert-Orientierung
            //              umsetzen, damit diese beim Umanker NICHT
            //              korrigiert wird
            pOldAnchor = new SwFmtAnchor( rAnch );
            ((SfxItemSet&)rSet).Put( SwFmtAnchor( FLY_PAGE, 1 ) );

            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rSet.GetItemState( RES_HORI_ORIENT, sal_False, &pItem )
                && HORI_NONE == ((SwFmtHoriOrient*)pItem)->GetHoriOrient() )
            {
                bHOriChgd = sal_True;
                aOldH = *((SwFmtHoriOrient*)pItem);
                ((SfxItemSet&)rSet).Put( SwFmtHoriOrient( 0, HORI_LEFT ) );
            }
            if( SFX_ITEM_SET == rSet.GetItemState( RES_VERT_ORIENT, sal_False, &pItem )
                && VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() )
            {
                bVOriChgd = sal_True;
                aOldV = *((SwFmtVertOrient*)pItem);
                ((SfxItemSet&)rSet).Put( SwFmtVertOrient( 0, VERT_TOP ) );
            }
        }

        pRet = GetDoc()->MakeFlyAndMove( *pCrsr, rSet, &aBoxes, pParent );

        KillPams();

        if( pOldAnchor )
        {
            if( pRet )
            {
                // neue Position bestimmen
                //JP 24.03.97: immer ueber die Seitenbindung gehen - der
                //           chaos::Anchor darf nie im verschobenen Bereich
                //              liegen
                pRet->DelFrms();

                const SwFrm* pAnch = ::FindAnchor( GetLayout(), aPt, sal_False );
                SwPosition aPos( *((SwCntntFrm*)pAnch)->GetNode() );
                if( FLY_IN_CNTNT == eRndId )
                    aPos.nContent.Assign( ((SwCntntFrm*)pAnch)->GetNode(), 0 );
                pOldAnchor->SetAnchor( &aPos );

                // das verschieben von TabelleSelektion ist noch nicht
                // Undofaehig - also darf das UmAnkern auch nicht
                // aufgezeichnet werden.
                sal_Bool bDoesUndo = GetDoc()->DoesUndo();
                if( bDoesUndo && UNDO_INSLAYFMT == GetDoc()->GetUndoIds() )
                    GetDoc()->DoUndo( sal_False );

                ((SfxItemSet&)rSet).Put( *pOldAnchor );

                if( bHOriChgd )
                    ((SfxItemSet&)rSet).Put( aOldH );
                if( bVOriChgd )
                    ((SfxItemSet&)rSet).Put( aOldV );

                GetDoc()->SetFlyFrmAttr( *pRet, (SfxItemSet&)rSet );
                GetDoc()->DoUndo( bDoesUndo );
            }
            delete pOldAnchor;
        }
        GetDoc()->EndUndo( UNDO_INSLAYFMT );
    }
    else
        pRet = GetDoc()->MakeFlySection( eRndId, &rPos, &rSet, pParent );

    if( pRet )
    {
        SwFlyFrm* pFrm = pRet->GetFrm( &aPt );
        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
        {
            GetLayout()->SetAssertFlyPages();
            pRet = 0;
        }
    }
    EndAllActionAndCall();

    return pRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  Insert
#*  Datum       :  ??
#*  Update      :  MA 12. Sep. 94
#***********************************************************************/

void SwFEShell::Insert( const String& rGrfName, const String& rFltName,
                        const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    SwFlyFrmFmt* pFmt = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
    FOREACHCURSOR_START( this )

        // Anker noch nicht oder unvollstaendig gesetzt ?
        if( pFlyAttrSet )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False,
                    &pItem ) )
            {
                SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
                switch( pAnchor->GetAnchorId())
                {
                case FLY_AT_CNTNT:
                case FLY_AUTO_CNTNT: // LAYER_IMPL
                case FLY_IN_CNTNT:
                    if( !pAnchor->GetCntntAnchor() )
                        pAnchor->SetAnchor( PCURCRSR->GetPoint() );
                    break;
                case FLY_AT_FLY:
                    if( !pAnchor->GetCntntAnchor() )
                        lcl_SetNewFlyPos( *PCURCRSR->GetNode(),
                                            *pAnchor, GetCrsrDocPos() );
                    break;
                case FLY_PAGE:
                    if( !pAnchor->GetPageNum() )
                    {
                        pAnchor->SetPageNum( PCURCRSR->GetPageNum(
                                    sal_True, &PCURCRSR->GetPtPos() ) );
                    }
                    break;
                }
            }
        }
        pFmt = GetDoc()->Insert(*PCURCRSR, rGrfName,
                                rFltName, pGraphic,
                                pFlyAttrSet,
                                pGrfAttrSet, pFrmFmt );
        ASSERT( pFmt, "Doc->Insert(notxt) failed." );

    FOREACHCURSOR_END()
    EndAllAction();

    if( pFmt )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFmt->GetFrm( &aPt );

        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
            GetLayout()->SetAssertFlyPages();
    }
}

void SwFEShell::Insert( SvInPlaceObject *pObj,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    SwFlyFrmFmt* pFmt = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
        FOREACHPAM_START( this )
            pFmt = GetDoc()->Insert(*PCURCRSR, pObj,
                                    pFlyAttrSet, pGrfAttrSet, pFrmFmt );
            ASSERT( pFmt, "Doc->Insert(notxt) failed." );

        FOREACHPAM_END()
    EndAllAction();

    if( pFmt )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFmt->GetFrm( &aPt );

        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
            GetLayout()->SetAssertFlyPages();
    }
}


void SwFEShell::Insert(  SdrObject& rDrawObj,
                         const SfxItemSet* pFlyAttrSet,
                         SwFrmFmt* pFrmFmt, const Point* pPt )
{
    SwDrawFrmFmt* pFmt = 0;
    SET_CURR_SHELL( this );

    if( pPt )
    {
        SfxItemSet* pSet = 0;
        const SfxPoolItem* pItem;
        if( !pFlyAttrSet ||
            !pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False, &pItem ) ||
            FLY_PAGE != ((SwFmtAnchor*)pItem)->GetAnchorId() )
        {
            pSet = new SfxItemSet( GetDoc()->GetAttrPool(), aFrmFmtSetRange );
            pSet->Put( SwFmtAnchor( FLY_AT_CNTNT ));
            pFlyAttrSet = pSet;
        }

        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwPaM aPam( pDoc->GetNodes() );
        Point aTmpPt( *pPt );
        GetDoc()->GetRootFrm()->GetCrsrOfst( aPam.GetPoint(), aTmpPt, &aState );
        SwFrm* pFrm = aPam.GetCntntNode()->GetFrm( 0, 0, sal_False );
        const Point aRelPos( pPt->X() - pFrm->Frm().Left(),
                             pPt->Y() - pFrm->Frm().Top() );
        rDrawObj.SetRelativePos( aRelPos );
        ::lcl_FindAnchorPos( *GetDoc(), *pPt, *pFrm, *(SfxItemSet*)pFlyAttrSet );
        pFmt = GetDoc()->Insert( aPam, rDrawObj, pFlyAttrSet, pFrmFmt );
        if( pSet )
            delete pSet;
    }
    else
    {
        StartAllAction();
            FOREACHPAM_START( this )
                pFmt = GetDoc()->Insert(*PCURCRSR, rDrawObj,
                                        pFlyAttrSet, pFrmFmt );
                ASSERT( pFmt, "Doc->Insert(sdrobj) failed." );

            FOREACHPAM_END()
        EndAllAction();
    }

    if( pFmt )
        // das DrawObject selektieren
        Imp()->GetDrawView()->MarkObj( &rDrawObj, Imp()->GetPageView(),
                                            sal_False, sal_False );
    else
        GetLayout()->SetAssertFlyPages();
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetPageObjs
#*  Datum       :  ??
#*  Update      :  MA 11. Jan. 95
#***********************************************************************/

void SwFEShell::GetPageObjs( SvPtrarr& rFillArr )
{
    if( rFillArr.Count() )
        rFillArr.Remove( 0, rFillArr.Count() );

    const SwFrmFmt* pFmt;
    for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
    {
        pFmt = (const SwFrmFmt*)(*pDoc->GetSpzFrmFmts())[n];
        if( FLY_PAGE == pFmt->GetAnchor().GetAnchorId() )
            rFillArr.Insert( (VoidPtr)pFmt, rFillArr.Count() );
    }
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  SetPageFlysNewPage
#*  Datum       :  ??
#*  Update      :  MA 14. Feb. 95
#***********************************************************************/

void SwFEShell::SetPageObjsNewPage( SvPtrarr& rFillArr, int nOffset )
{
    if( !rFillArr.Count() || !nOffset )
        return;

    StartAllAction();
    StartUndo();

    SwFrmFmt* pFmt;
    long nNewPage;
    sal_uInt16 nMaxPage = GetDoc()->GetRootFrm()->GetPageNum();
    sal_Bool bAssert = sal_False;
    for( sal_uInt16 n = 0; n < rFillArr.Count(); ++n )
    {
        pFmt = (SwFrmFmt*)rFillArr[n];
        if( USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( pFmt ))
        {
            // FlyFmt ist noch gueltig, also behandeln
            SwFmtAnchor aNewAnchor( pFmt->GetAnchor() );
            if( FLY_PAGE != aNewAnchor.GetAnchorId() ||
                0 >= ( nNewPage = aNewAnchor.GetPageNum() + nOffset ) )
                // chaos::Anchor wurde veraendert oder ungueltige SeitenNummer,
                // also nicht veraendern !!
                continue;

            if( sal_uInt16(nNewPage) > nMaxPage )
            {
                if ( RES_DRAWFRMFMT == pFmt->Which() )
                {
                    SwContact *pCon = pFmt->FindContactObj();
                    if( pCon )
                        ((SwDrawContact*)pCon)->DisconnectFromLayout();
                }
                else
                    pFmt->DelFrms();
                bAssert = sal_True;
            }
            aNewAnchor.SetPageNum( sal_uInt16(nNewPage) );
            pDoc->SetAttr( aNewAnchor, *pFmt );
        }
    }

    if( bAssert )
        GetDoc()->GetRootFrm()->SetAssertFlyPages();

    EndUndo();
    EndAllAction();
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetFlyFrmAttr
#*  Beschreibung:  Alle Attribute in dem 'Koerbchen' werden mit den
#*                 Attributen des aktuellen FlyFrms gefuellt.
#*                 Sind Attribute nicht zu fuellen weil fehl am Platz oder
#*                 uneindeutig (Mehrfachtselektionen) so werden sie entfernt.
#*  Datum       :  MA 03. Nov. 92
#*  Update      :  MA 03. Feb. 94
#***********************************************************************/

sal_Bool SwFEShell::GetFlyFrmAttr( SfxItemSet &rSet ) const
{
    SwFlyFrm *pFly = FindFlyFrm();
    if ( !pFly )
    {
        ASSERT( GetCurrFrm(), "Crsr in parking zone" );
        pFly = GetCurrFrm()->FindFlyFrm();
        if ( !pFly )
        {
            ASSERT( !this, "GetFlyFrmAttr, no Fly selected." );
            return sal_False;
        }
    }

    SET_CURR_SHELL( (ViewShell*)this );

    if( !rSet.Set( pFly->GetFmt()->GetAttrSet(), sal_True ) )
        return sal_False;

    //Und die Attribute durchschaufeln. Unerlaubte Attribute entfernen, dann
    //alle restlichen Attribute besorgen und eintragen.
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False, &pItem ) )
    {
        SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
        RndStdIds eType = pAnchor->GetAnchorId();

        if( FLY_PAGE != eType )
        {
            rSet.Put( SwFmtAnchor( eType ));
            if( FLY_IN_CNTNT == eType )
            {
                rSet.ClearItem( RES_OPAQUE );
                rSet.ClearItem( RES_SURROUND );
            }
        }
    }
    rSet.SetParent( pFly->GetFmt()->GetAttrSet().GetParent() );
    //JP 11.02.97: Bug #35894#: die Attribute MUESSEN entfern werden!
    rSet.ClearItem( RES_FILL_ORDER );
    rSet.ClearItem( RES_CNTNT );
    //MA: Ersteinmal entfernen (Template by example usw.)
    rSet.ClearItem( RES_CHAIN );
    return sal_True;
}
/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  SetFlyFrmAttr
#*  Beschreibung:  Die Attribute des aktuellen Flys aendern sich.
#*  Datum       :  MA 03. Nov. 92
#*  Update      :  MA 01. Aug. 95
#***********************************************************************/

sal_Bool SwFEShell::SetFlyFrmAttr( SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if( rSet.Count() )
    {
        SwFlyFrm *pFly = FindFlyFrm();
        if( !pFly )
        {
            ASSERT( GetCurrFrm(), "Crsr in parking zone" );
            pFly = GetCurrFrm()->FindFlyFrm();
            ASSERT( pFly, "SetFlyFrmAttr, no Fly selected." );
        }
        if( pFly )
        {
            StartAllAction();
            const Point aPt( pFly->Frm().Pos() );

            if( SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False ))
                ::lcl_ChkAndSetNewAnchor( *pFly, rSet );
            SwFlyFrmFmt* pFlyFmt = (SwFlyFrmFmt*)pFly->GetFmt();

            if( GetDoc()->SetFlyFrmAttr( *pFlyFmt, rSet ))
            {
                bRet = sal_True;
                SwFlyFrm* pFrm = pFlyFmt->GetFrm( &aPt );
                if( pFrm )
                    SelectFlyFrm( *pFrm, sal_True );
                else
                    GetLayout()->SetAssertFlyPages();
            }

            EndAllActionAndCall();
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  ResetFlyFrmAttr
#*  Beschreibung:  Das gewuenschte Attribut oder die im Set befindlichen
#*                  werden zurueckgesetzt.
#*  Datum       :  MA 14. Mar. 97
#*  Update      :  MA 14. Mar. 97
#***********************************************************************/

sal_Bool SwFEShell::ResetFlyFrmAttr( sal_uInt16 nWhich, const SfxItemSet* pSet )
{
    sal_Bool bRet = sal_False;

    if( RES_ANCHOR != nWhich && RES_CHAIN != nWhich && RES_CNTNT != nWhich )
    {
        SET_CURR_SHELL( this );

        SwFlyFrm *pFly = FindFlyFrm();
        if( !pFly )
        {
            ASSERT( GetCurrFrm(), "Crsr in parking zone" );
            pFly = GetCurrFrm()->FindFlyFrm();
            ASSERT( pFly, "SetFlyFrmAttr, no Fly selected." );
        }

        if( pFly )
        {
            StartAllAction();

            if( pSet )
            {
                SfxItemIter aIter( *pSet );
                const SfxPoolItem* pItem = aIter.FirstItem();
                while( pItem )
                {
                    if( !IsInvalidItem( pItem ) &&
                        RES_ANCHOR != ( nWhich = pItem->Which() ) &&
                        RES_CHAIN != nWhich && RES_CNTNT != nWhich )
                        pFly->GetFmt()->ResetAttr( nWhich );
                    pItem = aIter.NextItem();
                }
            }
            else
                pFly->GetFmt()->ResetAttr( nWhich );

            bRet = sal_True;
            EndAllActionAndCall();
            GetDoc()->SetModified();
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetCurFrmFmt
#*  Beschreibung:  liefert wenn Rahmen, dann Rahmenvorlage, sonst 0
#*  Datum       :  ST 04. Jun. 93
#*  Update      :
#***********************************************************************/

SwFrmFmt* SwFEShell::GetCurFrmFmt() const
{
    SwFrmFmt* pRet = 0;
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly && ( pRet = (SwFrmFmt*)pFly->GetFmt()->DerivedFrom() ) ==
                                            GetDoc()->GetDfltFrmFmt() )
        pRet = 0;
    return pRet;
}

/******************************************************************************
 *  Methode     :   void SwFEShell::SetFrmFmt(SwFrmFmt *pNewFmt)
 *  Beschreibung:
 *  Erstellt    :   OK 14.04.94 15:40
 *  Aenderung   :   MA 23. Apr. 97
 ******************************************************************************/

void SwFEShell::SetFrmFmt( SwFrmFmt *pNewFmt, sal_Bool bKeepOrient, Point* pDocPos )
{
    SwFlyFrm *pFly = 0;
    if(pDocPos)
    {
        const SwFrmFmt* pFmt = GetFmtFromObj( *pDocPos );

        if(PTR_CAST(SwFlyFrmFmt, pFmt))
            pFly = ((SwFlyFrmFmt*)pFmt)->GetFrm();
    }
    else
        pFly = FindFlyFrm();
    ASSERT( pFly, "SetFrmFmt: kein Frame" );
    if( pFly )
    {
        StartAllAction();
        SET_CURR_SHELL( this );

        SwFlyFrmFmt* pFlyFmt = (SwFlyFrmFmt*)pFly->GetFmt();
        const Point aPt( pFly->Frm().Pos() );

        SfxItemSet* pSet = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pNewFmt->GetItemState( RES_ANCHOR, sal_False, &pItem ))
        {
            pSet = new SfxItemSet( GetDoc()->GetAttrPool(), aFrmFmtSetRange );
            pSet->Put( *pItem );
            if( !::lcl_ChkAndSetNewAnchor( *pFly, *pSet ))
                delete pSet, pSet = 0;
        }

        if( GetDoc()->SetFrmFmtToFly( *pFlyFmt, *pNewFmt, pSet, bKeepOrient ))
        {
            SwFlyFrm* pFrm = pFlyFmt->GetFrm( &aPt );
            if( pFrm )
                SelectFlyFrm( *pFrm, sal_True );
            else
                GetLayout()->SetAssertFlyPages();
        }
        if( pSet )
            delete pSet;

        EndAllActionAndCall();
    }
}

/*************************************************************************
|*
|*  SwFEShell::GetFlyFrmFmt()
|*
|*  Ersterstellung      OK 23.06.93 13:15
|*  Letzte Aenderung    OK 23.06.93 13:15
|*
*************************************************************************/

const SwFrmFmt* SwFEShell::GetFlyFrmFmt() const
{
    const SwFlyFrm* pFly = FindFlyFrm();
    if ( !pFly )
        pFly = GetCurrFrm()->FindFlyFrm();
    if( pFly )
        return pFly->GetFmt();
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::GetFlyRect()
|*
|*  Ersterstellung      AMA 6. Mae. 97
|*  Letzte Aenderung    AMA 6. Mae. 97
|*
*************************************************************************/

SwRect SwFEShell::GetFlyRect() const
{
    SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
    SwFlyFrm *pFly = pCntnt->FindFlyFrm();
    if ( !pFly )
    {
        SwRect aRect;
        return aRect;
    }
    else
        return pFly->Frm();
}

/*************************************************************************
|*
|*  SwFEShell::GetObjRect()
|*
|*  Ersterstellung      MA 22. Aug. 93
|*  Letzte Aenderung    MA 11. Jan. 95
|*
*************************************************************************/

SwRect SwFEShell::GetObjRect() const
{
/*  const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
     Rectangle aRect;
    for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        aRect.Union( rMrkList.GetMark( i )->GetObj()->GetBoundRect() );
    return SwRect( aRect );*/
    if( Imp()->HasDrawView() )
        return Imp()->GetDrawView()->GetAllMarkedRect();
    else
    {
        SwRect aRect;
        return aRect;
    }
}

void SwFEShell::SetObjRect( const SwRect& rRect )
{
    if ( Imp()->HasDrawView() )
    {
        Imp()->GetDrawView()->SetAllMarkedRect( rRect.SVRect() );
        CallChgLnk();   // rufe das AttrChangeNotify auf der UI-Seite.
    }
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  RequestObjectResize()
#*  Datum       :  MA 10. Feb. 95
#*  Update      :  MA 13. Jul. 95
#***********************************************************************/

void SwFEShell::RequestObjectResize( const SwRect &rRect, SvEmbeddedObject *pIPObj )
{
    SwFlyFrm *pFly = FindFlyFrm( pIPObj );
    if ( !pFly )
        return;

    StartAllAction();

    //MA wir lassen den Fly nicht Clippen, damit die Ole-Server mit
    //beliebigen Wuenschen kommen koennen. Die Formatierung uebernimmt das
    //Clippen. Die richtige Darstellung wird per Scalierung erledigt.
    //Die Scalierung wird von SwNoTxtFrm::Format durch einen Aufruf von
    //SwWrtShell::CalcAndSetScale() erledigt.

    if ( rRect.SSize() != pFly->Prt().SSize() )
    {
         Size aSz( rRect.SSize() );

        //JP 28.02.2001: Task 74707 - ask for fly in fly with automatic size
        //
        const SwFrm* pAnchor;
        const SwTxtNode* pTNd;
        const SwpHints* pHts;
        const SwFmtFrmSize& rFrmSz = pFly->GetFmt()->GetFrmSize();
        if( bCheckForOLEInCaption &&
            0 != rFrmSz.GetWidthPercent() &&
            0 != (pAnchor = pFly->GetAnchor()) &&
            pAnchor->IsTxtFrm() &&
            !pAnchor->GetNext() && !pAnchor->GetPrev() &&
            pAnchor->GetUpper()->IsFlyFrm() &&
            0 != ( pTNd = ((SwTxtFrm*)pAnchor)->GetNode()->GetTxtNode()) &&
            0 != ( pHts = pTNd->GetpSwpHints() ))
        {
            // search for a sequence field:
            const SfxPoolItem* pItem;
            for( USHORT n = 0, nEnd = pHts->Count(); n < nEnd; ++n )
                if( RES_TXTATR_FIELD == ( pItem =
                            &(*pHts)[ n ]->GetAttr())->Which() &&
                    TYP_SEQFLD == ((SwFmtFld*)pItem)->GetFld()->GetTypeId() )
                {
                    // sequence field found
                    SwFlyFrm* pChgFly = (SwFlyFrm*)pAnchor->GetUpper();
                    // calculate the changed size:
                    // width must change, height can change
                    Size aNewSz( aSz.Width() + pChgFly->Frm().Width() -
                                   pFly->Prt().Width(), aSz.Height() );

                    SwFrmFmt *pFmt = pChgFly->GetFmt();
                    SwFmtFrmSize aFrmSz( pFmt->GetFrmSize() );
                    aFrmSz.SetWidth( aNewSz.Width() );
                    if( ATT_MIN_SIZE != aFrmSz.GetSizeType() )
                    {
                        aNewSz.Height() += pChgFly->Frm().Height() -
                                               pFly->Prt().Height();
                        if( Abs( aNewSz.Height() - pChgFly->Frm().Height()) > 1 )
                            aFrmSz.SetHeight( aNewSz.Height() );
                    }
                    // uebers Doc fuers Undo!
                    pFmt->GetDoc()->SetAttr( aFrmSz, *pFmt );
                    break;
                }
        }

        // set the new Size at the fly themself
        aSz.Width() += pFly->Frm().Width() - pFly->Prt().Width();
        aSz.Height()+= pFly->Frm().Height()- pFly->Prt().Height();
        pFly->ChgSize( aSz );

        //Wenn sich das Objekt aendert ist die Kontur hoechstwahrscheinlich daneben.
        ASSERT( pFly->Lower()->IsNoTxtFrm(), "Request ohne NoTxt" );
        SwNoTxtNode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetNoTxtNode();
        ASSERT( pNd, "Request ohne Node" );
        pNd->SetContour( 0 );
        ClrContourCache();
    }

    //Wenn nur die Size angepasst werden soll, so wird eine Pos mit
    //ausgezeichneten Werten transportiert.
    Point aPt( pFly->Prt().Pos() );
    aPt += pFly->Frm().Pos();
    if ( rRect.Top() != LONG_MIN && rRect.Pos() != aPt )
    {
        aPt = rRect.Pos();
        aPt.X() -= pFly->Prt().Left();
        aPt.Y() -= pFly->Prt().Top();
        //Bei Absatzgebundenen Flys muss ausgehend von der neuen Position ein
        //neuer Anker gesetzt werden. Anker und neue RelPos werden vom Fly
        //selbst berechnet und gesetzt.
        if( pFly->IsFlyAtCntFrm() )
            ((SwFlyAtCntFrm*)pFly)->SetAbsPos( aPt );
        else
        {
            const SwFrmFmt *pFmt = pFly->GetFmt();
            const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
            const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
            const long lXDiff = aPt.X() - pFly->Frm().Left();
            const long lYDiff = aPt.Y() - pFly->Frm().Top();
            const Point aTmp( rHori.GetPos() + lXDiff,
                              rVert.GetPos() + lYDiff );
            pFly->ChgRelPos( aTmp );
        }
    }
    EndAllAction();
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  DeleteCurrFrmFmt
#*  Datum       :  JP 28.07.95
#*  Update      :  JP 28.07.95
#***********************************************************************/

sal_Bool SwFEShell::WizzardDelFly()
{
    // mal nicht uebers Layout den Fly suchen. Dann kann auch ohne gueltiges
    // Layout ein Rahmen geloescht werden. ( z.B.: fuer die Wizard's )
    SwFrmFmt* pDelFmt = WizzardGetFly();
    if( pDelFmt )
    {
        // gefunden: dann also loeschen
        // bei Zeichen/Absatz gebundenen Flys den Crsr auf den Anker umsetzen,
        // dadurch bleibt der Crsr auf einer "definierten" Position
        // (Im Wizzard wird das Layout nicht aktualisiert!)
        SwPosition* pCrsrPos = GetCrsr()->GetPoint(), *pNewPos = 0;
        const SwFmtAnchor& rAnch = pDelFmt->GetAnchor();
        if( rAnch.GetCntntAnchor() &&
            ( FLY_AT_CNTNT == rAnch.GetAnchorId() ||
              FLY_AUTO_CNTNT == rAnch.GetAnchorId() ||
                FLY_IN_CNTNT == rAnch.GetAnchorId() ))
        {
            pNewPos = new SwPosition( *rAnch.GetCntntAnchor() );
            if( FLY_IN_CNTNT != rAnch.GetAnchorId() )
                pNewPos->nContent.Assign( GetDoc()->GetNodes()[ pNewPos->nNode ]
                                                    ->GetCntntNode(), 0 );
        }

        GetDoc()->DelLayoutFmt( pDelFmt );

        if( pNewPos )
        {
            *pCrsrPos = *pNewPos;
            delete pNewPos;
        }
        return sal_True;
    }
    return sal_False;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  WizzardFindCurFrmFmt
#*  Datum       :  JP 31.07.95
#*  Update      :  JP 31.07.95
#***********************************************************************/

SwFrmFmt* SwFEShell::WizzardGetFly()
{
    // mal nicht uebers Layout den Fly suchen. Dann kann auch ohne gueltiges
    // Layout ein Rahmen geloescht werden. ( z.B.: fuer die Wizard's )
    SwSpzFrmFmts& rSpzArr = *pDoc->GetSpzFrmFmts();
    sal_uInt16 nCnt = rSpzArr.Count();
    if( nCnt )
    {
        SwNodeIndex& rCrsrNd = GetCrsr()->GetPoint()->nNode;
        if( rCrsrNd.GetIndex() > pDoc->GetNodes().GetEndOfExtras().GetIndex() )
            // Cusor steht im Body-Bereich!
            return 0;

        for( sal_uInt16 n = 0; n < nCnt; ++n )
        {
            SwFrmFmt* pFmt = rSpzArr[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt( sal_False ).GetCntntIdx();
            SwStartNode* pSttNd;
            if( pIdx &&
                0 != ( pSttNd = pIdx->GetNode().GetStartNode() ) &&
                pSttNd->GetIndex() < rCrsrNd.GetIndex() &&
                rCrsrNd.GetIndex() < pSttNd->EndOfSectionIndex() )
            {
                // gefunden: also raus damit
                return pFmt;
            }
        }
    }
    return 0;
}

void SwFEShell::SetFlyName( const String& rName )
{
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly )
        GetDoc()->SetFlyName( *(SwFlyFrmFmt*)pFly->GetFmt(), rName );
    else
        ASSERT( !this, "kein FlyFrame selektiert" )
}

const String& SwFEShell::GetFlyName() const
{
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly )
        return pFly->GetFmt()->GetName();

    ASSERT( !this, "kein FlyFrame selektiert" )
    return aEmptyStr;
}


String SwFEShell::GetUniqueGrfName() const
{
    return GetDoc()->GetUniqueGrfName();
}

const SwFrmFmt* SwFEShell::IsURLGrfAtPos( const Point& rPt, String* pURL,
                                        String *pTargetFrameName,
                                        String *pDescription ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    const SwFrmFmt* pRet = 0;
    SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

    sal_uInt16 nOld = pDView->GetHitTolerancePixel();
    pDView->SetHitTolerancePixel( 2 );

    if( pDView->PickObj( rPt, pObj, pPV,SDRSEARCH_PICKMACRO ) &&
        pObj->IsWriterFlyFrame() )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        const SwFmtURL &rURL = pFly->GetFmt()->GetURL();
        if( rURL.GetURL().Len() || rURL.GetMap() )
        {
            FASTBOOL bSetTargetFrameName = pTargetFrameName != 0;
            FASTBOOL bSetDescription = pDescription != 0;
            if ( rURL.GetMap() )
            {
                IMapObject *pObj = pFly->GetFmt()->GetIMapObject( rPt, pFly );
                if ( pObj && pObj->GetURL().Len() )
                {
                    if( pURL )
                        *pURL = pObj->GetURL();
                    if ( bSetTargetFrameName && pObj->GetTarget().Len() )
                    {
                        bSetTargetFrameName = sal_False;
                        *pTargetFrameName = pObj->GetTarget();
                    }
                    if ( bSetDescription )
                    {
                        bSetDescription = sal_False;
                        *pDescription = pObj->GetDescription();
                    }
                    pRet = pFly->GetFmt();
                }
            }
            else
            {
                if( pURL )
                {
                    *pURL = rURL.GetURL();
                    if( rURL.IsServerMap() )
                    {
                        // dann die rel. Pixel Position anhaengen !!
                        Point aPt( rPt );
                        aPt -= pFly->Frm().Pos();
                        // ohne MapMode-Offset, ohne Offset, o ... !!!!!
                        aPt = (Point&)(Size&)GetOut()->LogicToPixel(
                                (Size&)aPt, MapMode( MAP_TWIP ) );
                        ((( *pURL += '?' ) += String::CreateFromInt32( aPt.X() ))
                                  += ',' ) += String::CreateFromInt32(aPt.Y() );
                    }
                }
                pRet = pFly->GetFmt();
            }
            if ( bSetTargetFrameName )
                *pTargetFrameName = rURL.GetTargetFrameName();
            if ( bSetDescription )
                *pDescription = pFly->GetFmt()->GetName();
        }
    }
    pDView->SetHitTolerancePixel( nOld );
    return pRet;
}

const Graphic *SwFEShell::GetGrfAtPos( const Point &rPt,
                                       String &rName, sal_Bool &rbLink ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

    if( pDView->PickObj( rPt, pObj, pPV ) && pObj->IsWriterFlyFrame() )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
        {
            SwGrfNode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode();
            if ( pNd )
            {
                if ( pNd->IsGrfLink() )
                {
                    //Halbfertige Grafik?
                    SvPseudoObject* pLnkObj = pNd->GetLink()->GetObj();
                    if ( pLnkObj && ERRCODE_SO_PENDING == pLnkObj->GetUpToDateStatus() )
                        return 0;
                    rbLink = sal_True;
                }

                pNd->GetFileFilterNms( &rName, 0 );
                if ( !rName.Len() )
                    rName = pFly->GetFmt()->GetName();
                pNd->SwapIn( sal_True );
                return &pNd->GetGrf();
            }
        }
    }
    return 0;
}


const SwFrmFmt* SwFEShell::GetFmtFromObj( const Point& rPt, SwRect** pRectToFill ) const
{
    SwFrmFmt* pRet = 0;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;

        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        // Tattergrenze fuer Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pObj, pPView, SDRSEARCH_PICKMARKABLE ) )
        {
            // dann teste mal was es ist:
            if ( pObj->IsWriterFlyFrame() )
                pRet = ((SwVirtFlyDrawObj*)pObj)->GetFmt();
            else if ( pObj->GetUserCall() ) //nicht fuer Gruppenobjekte
                pRet = ((SwDrawContact*)pObj->GetUserCall())->GetFmt();
            if(pRet && pRectToFill)
                **pRectToFill = pObj->GetBoundRect();
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// returns a format too, if the point is over the text of any fly
const SwFrmFmt* SwFEShell::GetFmtFromAnyObj( const Point& rPt ) const
{
    const SwFrmFmt* pRet = GetFmtFromObj( rPt );
    if( !pRet || RES_FLYFRMFMT == pRet->Which() )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( rPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        SwFrm* pFrm = pNd->GetFrm( &rPt )->FindFlyFrm();
        pRet = pFrm ? ((SwLayoutFrm*)pFrm)->GetFmt() : 0;
    }
    return pRet;
}

ObjCntType SwFEShell::GetObjCntType( const SdrObject& rObj ) const
{
    ObjCntType eType;
    if( FmFormInventor == rObj.GetObjInventor() )
    {
        eType = OBJCNT_CONTROL;
        uno::Reference< awt::XControlModel >  xModel = ((SdrUnoObj&)rObj).GetUnoControlModel();
        if( xModel.is() )
        {
            uno::Any aVal;
            OUString sName = OUString::createFromAscii("ButtonType");
            uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);

            uno::Reference< beans::XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
            if(xInfo->hasPropertyByName( sName ))
            {
                beans::Property xProperty = xInfo->getPropertyByName( sName );
                aVal = xSet->getPropertyValue( sName );
                if( aVal.getValue() && form::FormButtonType_URL == *((form::FormButtonType*)aVal.getValue()) )
                    eType = OBJCNT_URLBUTTON;
            }
        }
    }
    else if( rObj.IsWriterFlyFrame() )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj&)rObj).GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
        {
            if ( ((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode() )
                eType = OBJCNT_GRF;
            else
                eType = OBJCNT_OLE;
        }
        else
            eType = OBJCNT_FLY;
    }
    else if( rObj.ISA( SdrObjGroup ) && FLY_IN_CNTNT !=
        ((SwDrawContact*)GetUserCall(&rObj))->GetFmt()->GetAnchor().GetAnchorId() )
        eType = OBJCNT_GROUPOBJ;
    else
        eType = OBJCNT_SIMPLE;
    return eType;
}

ObjCntType SwFEShell::GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;

        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        // Tattergrenze fuer Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pObj, pPView, SDRSEARCH_PICKMARKABLE ) )
            eType = GetObjCntType( *(rpObj = pObj) );

        pDView->SetHitTolerancePixel( nOld );
    }
    return eType;
}

ObjCntType SwFEShell::GetObjCntTypeOfSelection( SdrObject** ppObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    if( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
        for( sal_uInt32 i = 0, nE = rMrkList.GetMarkCount(); i < nE; ++i )
        {
            SdrObject* pObj = rMrkList.GetMark( i )->GetObj();
            ObjCntType eTmp = GetObjCntType( *pObj );
            if( !i )
            {
                eType = eTmp;
                if( ppObj ) *ppObj = pObj;
            }
            else if( eTmp != eType )
            {
                eType = OBJCNT_DONTCARE;
                // einmal DontCare, immer DontCare!
                break;
            }
        }
    }
    return eType;
}


sal_Bool SwFEShell::ReplaceSdrObj( const String& rGrfName, const String& rFltName,
                                const Graphic* pGrf )
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;
    const SdrMarkList *pMrkList;
    if( Imp()->HasDrawView() &&  1 ==
        ( pMrkList = &Imp()->GetDrawView()->GetMarkList())->GetMarkCount() )
    {
        SdrObject* pObj = pMrkList->GetMark( 0 )->GetObj();
        SwFrmFmt *pFmt = FindFrmFmt( pObj );

        // Attribute sichern und dann an der Grafik setzen
        SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                            pFmt->GetAttrSet().GetRanges() );
        aFrmSet.Set( pFmt->GetAttrSet() );

        // Groesse und Position setzen ??
        if( !pObj->IsWriterFlyFrame() )
        {
            // dann mal los:
            const Rectangle &rBound = pObj->GetSnapRect();
            Point aRelPos( pObj->GetRelativePos() );

            const long nWidth = rBound.Right()  - rBound.Left();
            const long nHeight= rBound.Bottom() - rBound.Top();
            aFrmSet.Put( SwFmtFrmSize( ATT_MIN_SIZE,
                                Max( nWidth,  long(MINFLY) ),
                                Max( nHeight, long(MINFLY) )));

            if( SFX_ITEM_SET != aFrmSet.GetItemState( RES_HORI_ORIENT ))
                aFrmSet.Put( SwFmtHoriOrient( aRelPos.X(), HORI_NONE, FRAME ));

            if( SFX_ITEM_SET != aFrmSet.GetItemState( RES_VERT_ORIENT ))
                aFrmSet.Put( SwFmtVertOrient( aRelPos.Y(), VERT_NONE, FRAME ));

        }

        sal_uInt32 nOrdNum = pObj->GetOrdNum();

        StartAllAction();
        StartUndo();

        // das "Sdr-Object" loeschen und dafuer die Grafik einfuegen
        DelSelectedObj();

        pFmt = GetDoc()->Insert( *GetCrsr(), rGrfName, rFltName, pGrf, &aFrmSet );

        // die Ordnungsnummer (Z-Order) noch uebertragen
        // JP 04.07.98: klappt aber nicht richtig!
        //SdrObject* pNewObj = ::FindSdrObject( pFmt );
        //pNewObj->SetOrdNum( nOrdNum );

        EndUndo();
        EndAllAction();
        bRet = sal_True;
    }
    return bRet;
}


