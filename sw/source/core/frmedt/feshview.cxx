/*************************************************************************
 *
 *  $RCSfile: feshview.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:03:20 $
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


#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include "hintids.hxx"

#ifdef WIN
#define NEEDED_BY_FESHVIEW
#define _FESHVIEW_ONLY_INLINE_NEEDED
#endif

#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOCIRC_HXX
#include <svx/svdocirc.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif
#ifndef _SXCIAITM_HXX
#include <svx/sxciaitm.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#ifndef _XPOLY_HXX //autogen
#include <svx/xpoly.hxx>
#endif
#ifndef _SVDCAPT_HXX //autogen
#include <svx/svdocapt.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen wg. SdrPageView
#include <svx/svdpagv.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>      // fuer InitFldTypes
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _TBLSEL_HXX //autogen
#include <tblsel.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#include "fesh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "sectfrm.hxx"
#include "doc.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "viewimp.hxx"
#include "flyfrm.hxx"
#include "pam.hxx"
#include "ndole.hxx"
#include "ndgrf.hxx"
#include "ndtxt.hxx"
#include "viewopt.hxx"                  // fuer GetHTMLMode
#include "swundo.hxx"
#include "notxtfrm.hxx"
#include "txtfrm.hxx"
#include "txatbase.hxx"
#include "mdiexp.hxx"                   // fuer Update der Statuszeile bei drag
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#define SCROLLVAL 75

//Tattergrenze fuer Drawing-SS
#define MINMOVE ((USHORT)GetOut()->PixelToLogic(Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())

SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, ViewShell *pSh )
{
    if ( !pLst )
        pLst = pSh->HasDrawView() ? &pSh->Imp()->GetDrawView()->GetMarkedObjectList():0;

    if ( pLst && pLst->GetMarkCount() == 1 )
    {
        SdrObject *pO = pLst->GetMark( 0 )->GetObj();
        if ( pO->ISA(SwVirtFlyDrawObj) )
            return ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
    }
    return 0;
}

void lcl_GrabCursor( SwFEShell* pSh, SwFlyFrm* pOldSelFly)
{
    const SwFrmFmt *pFlyFmt = pSh->SelFlyGrabCrsr();
    if( pFlyFmt && !pSh->ActionPend() &&
                        (!pOldSelFly || pOldSelFly->GetFmt() != pFlyFmt) )
    {
        // dann das evt. gesetzte Macro rufen
        pSh->GetFlyMacroLnk().Call( (void*)pFlyFmt );
extern BOOL bNoInterrupt;       // in swapp.cxx
        // wir in dem Makro ein Dialog gestartet, dann kommt das
        // MouseButtonUp zu diesem und nicht zu uns. Dadurch ist
        // Flag bei uns immer gesetzt und schaltet nie die auf die
        // entsp. Shell um !!!!!!!
        bNoInterrupt = FALSE;
    }
    else if( !pFlyFmt || RES_DRAWFRMFMT == pFlyFmt->Which() )
        pSh->SetCrsr( pSh->Imp()->GetDrawView()->GetAllMarkedRect().TopLeft(), TRUE);
}

/*************************************************************************
|*
|*  SwFEShell::SelectObj()
|*
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    MA 22. Oct. 96
|*
*************************************************************************/

BOOL SwFEShell::SelectObj( const Point& rPt, BYTE nFlag, SdrObject *pObj )
{
    SET_CURR_SHELL( this );
    StartAction();          //Aktion ist Notwendig, damit nicht mehrere
                            //AttrChgdNotify (etwa durch Unmark->MarkListHasChgd)
                            //durchkommen

    ASSERT( Imp()->HasDrawView(), "SelectObj without DrawView?" );
    SwDrawView *pDView = Imp()->GetDrawView();
    const SdrMarkList &rMrkList = pDView->GetMarkedObjectList();
    const BOOL bHadSelection = rMrkList.GetMarkCount() ? TRUE : FALSE;
    const BOOL bAddSelect = 0 != (SW_ADD_SELECT & nFlag);
    const BOOL bEnterGroup = 0 != (SW_ENTER_GROUP & nFlag);
    SwFlyFrm* pOldSelFly = 0;
    const Point aOldPos( pDView->GetAllMarkedRect().TopLeft() );

    if( bHadSelection )
    {
        //Unmark rufen wenn !bAddSelect oder wenn ein Fly selektiert ist.
        BOOL bUnmark = !bAddSelect;

        if ( rMrkList.GetMarkCount() == 1 )
        {
            //Wenn ein Fly selektiert ist, so muss er erst deselektiert werden.
            pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );
            if ( pOldSelFly )
            {
                const USHORT nType = GetCntType();
                if( nType != CNT_TXT || (SW_LEAVE_FRAME & nFlag) ||
                    ( pOldSelFly->GetFmt()->GetProtect().IsCntntProtected()
                     && !IsReadOnlyAvailable() ))
                {
                    //Wenn ein Fly deselektiert wird, der Grafik, Ole o.ae.
                    //enthaelt, so muss der Crsr aus diesem entfernt werden.
                    //Desgleichen wenn ein Fly mit geschuetztem Inhalt deselektiert
                    //wird. Der Einfachheit halber wire der Crsr 'grad so neben die
                    //linke obere Ecke gesetzt.
                    Point aPt( pOldSelFly->Frm().Pos() );
                    aPt.X() -= 1;
                    BOOL bUnLockView = !IsViewLocked();
                    LockView( TRUE );
                    SetCrsr( aPt, TRUE );
                    if( bUnLockView )
                        LockView( FALSE );
                }
                if ( nType & CNT_GRF &&
                     ((SwNoTxtFrm*)pOldSelFly->Lower())->HasAnimation() )
                {
                    GetWin()->Invalidate( pOldSelFly->Frm().SVRect() );
                }
                bUnmark = TRUE;
            }
#ifdef USED
            else
            {
                SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
                SwFrmFmt *pFrmFmt = FindFrmFmt( pObj );
                if( pFrmFmt &&
                    FLY_IN_CNTNT == pFrmFmt->GetAnchor().GetAnchorId() )
                    bUnmark = TRUE;
            }
#endif
        }
        if ( bUnmark )
            pDView->UnmarkAll();
    }
    else
    {
        KillPams();
        ClearMark();
    }

    if ( pObj )
    {
        ASSERT( !bEnterGroup, "SW_ENTER_GROUP is not supported" );
        pDView->MarkObj( pObj, Imp()->GetPageView() );
    }
    else
    {
        pDView->MarkObj( rPt, MINMOVE, bAddSelect, bEnterGroup );
    }

    const FASTBOOL bRet = 0 != rMrkList.GetMarkCount();

    if ( rMrkList.GetMarkCount() > 1 )
    {
        //Ganz dumm ist es, wenn Zeichenobjekte Selektiert waren und
        //nun ein Fly hinzuselektiert wird.
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            BOOL bForget = pObj->ISA(SwVirtFlyDrawObj);
            if( bForget )
            {
                pDView->UnmarkAll();
                if ( pObj )
                    pDView->MarkObj( pObj, Imp()->GetPageView(), bAddSelect, bEnterGroup );
                else
                    pDView->MarkObj( rPt, MINMOVE );
                break;
            }
        }
    }

    if ( bRet )
    {
        ::lcl_GrabCursor(this, pOldSelFly);
        if ( GetCntType() & CNT_GRF )
        {
            const SwFlyFrm *pTmp = GetFlyFromMarked( &rMrkList, this );
            ASSERT( pTmp, "Graphic without Fly" );
            if ( ((SwNoTxtFrm*)pTmp->Lower())->HasAnimation() )
                ((SwNoTxtFrm*)pTmp->Lower())->StopAnimation( GetOut() );
        }
    }
    else if ( !pOldSelFly && bHadSelection )
        SetCrsr( aOldPos, TRUE);

    if( bRet || !bHadSelection )
        CallChgLnk();

    // update der Statuszeile
    ::FrameNotify( this, bRet ? FLY_DRAG_START : FLY_DRAG_END );

    EndAction();
    return bRet;
}

/*************************************************************************
|*
|*  sal_Bool SwFEShell::MoveAnchor( USHORT nDir )
|*
|*  Created        AMA 05/28/2002
|*  Last modify    AMA 05/30/2002
|*
|*  Description: MoveAnchor( nDir ) looked for an another Anchor for
|*  the selected drawing object (or fly frame) in the given direction.
|*  An object "as character" doesn't moves anyway.
|*  A page bounded object could move to the previous/next page with up/down,
|*  an object bounded "at paragraph" moves to the previous/next paragraph, too.
|*  An object bounded "at character" moves to the previous/next paragraph
|*  with up/down and to the previous/next character with left/right.
|*  If the anchor for at paragraph/character bounded objects has vertical or
|*  right_to_left text direction, the directions for up/down/left/right will
|*  interpreted accordingly.
|*  An object bounded "at fly" takes the center of the actual anchor and looks
|*  for the nearest fly frame in the given direction.
|*
*************************************************************************/

#define LESS_X( aPt1, aPt2, bOld ) ( aPt1.X() < aPt2.X() || \
        ( aPt1.X() == aPt2.X() && ( aPt1.Y() < aPt2.Y() || \
        ( aPt1.Y() == aPt2.Y() && bOld ) ) ) )
#define LESS_Y( aPt1, aPt2, bOld ) ( aPt1.Y() < aPt2.Y() || \
        ( aPt1.Y() == aPt2.Y() && ( aPt1.X() < aPt2.X() || \
        ( aPt1.X() == aPt2.X() && bOld ) ) ) )

sal_Bool SwFEShell::MoveAnchor( USHORT nDir )
{
    const SdrMarkList* pMrkList;
    if( !Imp()->GetDrawView() ||
        0 == (pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList()) ||
        1 != pMrkList->GetMarkCount())
        return sal_False;
    SwFrm* pOld;
    SwFlyFrm* pFly = NULL;
    SdrObject *pObj = pMrkList->GetMark( 0 )->GetObj();
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        pOld = pFly->AnchorFrm();
    }
    else
        pOld = ((SwDrawContact*)GetUserCall(pObj))->GetAnchorFrm( pObj );
    sal_Bool bRet = sal_False;
    if( pOld )
    {
        SwFrm* pNew = pOld;
        // --> OD 2004-07-16 #i28701#
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
        SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
        SwFmtAnchor aAnch( rFmt.GetAnchor() );
        RndStdIds nAnchorId = aAnch.GetAnchorId();
        if ( FLY_IN_CNTNT == nAnchorId )
            return sal_False;
        if( pOld->IsVertical() )
        {
            if( pOld->IsTxtFrm() )
            {
                switch( nDir ) {
                    case SW_MOVE_UP: nDir = SW_MOVE_LEFT; break;
                    case SW_MOVE_DOWN: nDir = SW_MOVE_RIGHT; break;
                    case SW_MOVE_LEFT: nDir = SW_MOVE_DOWN; break;
                    case SW_MOVE_RIGHT: nDir = SW_MOVE_UP; break;
                }
                if( pOld->IsRightToLeft() )
                {
                    if( nDir == SW_MOVE_LEFT )
                        nDir = SW_MOVE_RIGHT;
                    else if( nDir == SW_MOVE_RIGHT )
                        nDir = SW_MOVE_LEFT;
                }
            }
        }
        switch ( nAnchorId ) {
            case FLY_PAGE:
            {
                ASSERT( pOld->IsPageFrm(), "Wrong anchor, page exspected." );
                if( SW_MOVE_UP == nDir )
                    pNew = pOld->GetPrev();
                else if( SW_MOVE_DOWN == nDir )
                    pNew = pOld->GetNext();
                if( pNew && pNew != pOld )
                {
                    aAnch.SetPageNum( ((SwPageFrm*)pNew)->GetPhyPageNum() );
                    bRet = sal_True;
                }
                break;
            }
            case FLY_AUTO_CNTNT:
            {
                ASSERT( pOld->IsCntntFrm(), "Wrong anchor, page exspected." );
                if( SW_MOVE_LEFT == nDir || SW_MOVE_RIGHT == nDir )
                {
                    SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pOld)->GetTxtNode();
                    xub_StrLen nAct = pPos->nContent.GetIndex();
                    if( SW_MOVE_LEFT == nDir )
                    {
                        bRet = sal_True;
                        if( nAct )
                        {
                            --nAct;
                            pPos->nContent.Assign( pTxtNd, nAct );
                        }
                        else
                            nDir = SW_MOVE_UP;
                    }
                    else
                    {
                        xub_StrLen nMax =
                            ((SwTxtFrm*)pOld)->GetTxtNode()->GetTxt().Len();
                        if( nAct < nMax )
                        {
                            ++nAct;
                            bRet = sal_True;
                            pPos->nContent.Assign( pTxtNd, nAct );
                        }
                        else
                            nDir = SW_MOVE_DOWN;
                    }
                }
            } // no break!
            case FLY_AT_CNTNT:
            {
                ASSERT( pOld->IsCntntFrm(), "Wrong anchor, page exspected." );
                if( SW_MOVE_UP == nDir )
                    pNew = pOld->FindPrev();
                else if( SW_MOVE_DOWN == nDir )
                    pNew = pOld->FindNext();
                if( pNew && pNew != pOld && pNew->IsCntntFrm() )
                {
                    SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pNew)->GetTxtNode();
                    pPos->nNode = *pTxtNd;
                    xub_StrLen nTmp = 0;
                    if( bRet )
                    {
                        nTmp = ((SwTxtFrm*)pNew)->GetTxtNode()->GetTxt().Len();
                        if( nTmp )
                            --nTmp;
                    }
                    pPos->nContent.Assign( pTxtNd, nTmp );
                    bRet = sal_True;
                }
                else if( SW_MOVE_UP == nDir || SW_MOVE_DOWN == nDir )
                    bRet = sal_False;
                break;
            }
            case FLY_AT_FLY:
            {
                ASSERT( pOld->IsFlyFrm(), "Wrong anchor, fly frame exspected.");
                SwPageFrm* pPage = pOld->FindPageFrm();
                ASSERT( pPage, "Where's my page?" );
                SwFlyFrm* pNewFly = NULL;
                if( pPage->GetSortedObjs() )
                {
                    int i;
                    sal_Bool bOld = sal_False;
                    Point aCenter( pOld->Frm().Left() + pOld->Frm().Width()/2,
                                   pOld->Frm().Top() + pOld->Frm().Height()/2 );
                    Point aBest;
                    for( i = 0; (USHORT)i<pPage->GetSortedObjs()->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj =
                                                (*pPage->GetSortedObjs())[i];
                        if( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            SwFlyFrm* pTmp = static_cast<SwFlyFrm*>(pAnchoredObj);
                            if( pTmp == pOld )
                                bOld = sal_True;
                            else
                            {
                                const SwFlyFrm* pCheck = pFly ? pTmp : 0;
                                while( pCheck )
                                {
                                    if( pCheck == pFly )
                                        break;
                                    const SwFrm *pNxt = pCheck->GetAnchorFrm();
                                    pCheck = pNxt ? pNxt->FindFlyFrm() : NULL;
                                }
                                if( pCheck || pTmp->IsProtected() )
                                    continue;
                                Point aNew( pTmp->Frm().Left() +
                                            pTmp->Frm().Width()/2,
                                            pTmp->Frm().Top() +
                                            pTmp->Frm().Height()/2 );
                                sal_Bool bAccept = sal_False;
                                switch( nDir ) {
                                    case SW_MOVE_RIGHT:
                                    {
                                        bAccept = LESS_X( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aNew, aBest, sal_False ) );
                                        break;
                                    }
                                    case SW_MOVE_LEFT:
                                    {
                                        bAccept = LESS_X( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aBest, aNew, sal_True ) );
                                        break;
                                    }
                                    case SW_MOVE_UP:
                                    {
                                        bAccept = LESS_Y( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aBest, aNew, sal_True ) );
                                        break;
                                    }
                                    case SW_MOVE_DOWN:
                                    {
                                        bAccept = LESS_Y( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aNew, aBest, sal_False ) );
                                        break;
                                    }
                                }
                                if( bAccept )
                                {
                                    pNewFly = pTmp;
                                    aBest = aNew;
                                }
                            }
                        }
                    }
                }

                if( pNewFly )
                {
                    SwPosition aPos( *pNewFly->GetFmt()->
                                        GetCntnt().GetCntntIdx());
                    aAnch.SetAnchor( &aPos );
                    bRet = sal_True;
                }
                break;
            }
        }
        if( bRet )
        {
            StartAllAction();
            rFmt.GetDoc()->SetAttr( aAnch, rFmt );
            // --> OD 2004-06-24 #i28701# - no call of method
            // <CheckCharRectAndTopOfLine()> for to-character anchored
            // Writer fly frame needed. This method call can cause a
            // format of the anchor frame, which is no longer intended.
                    // Instead clear the anchor character rectangle and
                    // the top of line values for all to-character anchored objects.
//            if ( nAnchorId == FLY_AUTO_CNTNT && pFly && pFly->IsFlyAtCntFrm() )
//            {
//                // OD 11.11.2003 #i22341#
//                static_cast<SwFlyAtCntFrm*>(pFly)->CheckCharRectAndTopOfLine();
//            }
            pAnchoredObj->ClearCharRectAndTopOfLine();
            EndAllAction();
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetSelFrmType()
|*
|*  Ersterstellung      MA 12. Jan. 93
|*  Letzte Aenderung    JP 19.03.96
|*
*************************************************************************/

const SdrMarkList* SwFEShell::_GetMarkList() const
{
    const SdrMarkList* pMarkList = NULL;
    if( Imp()->GetDrawView() != NULL )
        pMarkList = &Imp()->GetDrawView()->GetMarkedObjectList();
    return pMarkList;
}

USHORT SwFEShell::GetSelFrmType() const
{
    enum FrmType eType;

    // get marked frame list, and check if anything is selected
    const SdrMarkList* pMarkList = _GetMarkList();
    if( pMarkList == NULL  ||  pMarkList->GetMarkCount() == 0 )
        eType = FRMTYPE_NONE;
    else
    {
        // obtain marked item as fly frame; if no fly frame, it must
        // be a draw object
        const SwFlyFrm* pFly = ::GetFlyFromMarked(pMarkList, (ViewShell*)this);
        if ( pFly != NULL )
        {
            if( pFly->IsFlyLayFrm() )
                eType = FRMTYPE_FLY_FREE;
            else if( pFly->IsFlyAtCntFrm() )
                eType = FRMTYPE_FLY_ATCNT;
            else
            {
                ASSERT( pFly->IsFlyInCntFrm(), "Neuer Rahmentyp?" );
                eType = FRMTYPE_FLY_INCNT;
            }
        }
        else
            eType = FRMTYPE_DRAWOBJ;
    }

    return eType;
}

// #108784# does the draw selection contain a control?
bool SwFEShell::IsSelContainsControl() const
{
    bool bRet = false;

    // basically, copy the mechanism from GetSelFrmType(), but call
    // CheckControl... if you get a drawing object
    const SdrMarkList* pMarkList = _GetMarkList();
    if( pMarkList != NULL  &&  pMarkList->GetMarkCount() == 1 )
    {
        // if we have one marked object, get the SdrObject and check
        // whether it contains a control
        const SdrObject* pSdrObject = pMarkList->GetMark( 0 )->GetObj();
        bRet = CheckControlLayer( pSdrObject );
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::Scroll()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 27. Jul. 95
|*
*************************************************************************/

void SwFEShell::Scroll( const Point &rPt )
{
    const SwRect aRect( rPt, rPt );
    if ( IsScrollMDI( this, aRect ) &&
         (!Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() ||
          Imp()->IsDragPossible( rPt )) )
    {
        SwSaveHdl aSave( Imp() );
        ScrollMDI( this, aRect, SCROLLVAL, SCROLLVAL );
    }
}

/*************************************************************************
|*
|*  SwFEShell::SetDragMode()
|*
|*  Ersterstellung      MA 30. Jan. 95
|*  Letzte Aenderung    MA 30. Jan. 95
|*
*************************************************************************/

void SwFEShell::SetDragMode( UINT16 eDragMode )
{
    if ( Imp()->HasDrawView() )
        Imp()->GetDrawView()->SetDragMode( (SdrDragMode)eDragMode );
}

/*************************************************************************
|*
|*  SwFEShell::BeginDrag()
|*
|*  Ersterstellung      MS 10.06.92
|*  Letzte Aenderung    MA 13. Mar. 96
|*
*************************************************************************/

long SwFEShell::BeginDrag( const Point* pPt, BOOL )
{
    SdrView *pView = Imp()->GetDrawView();
    if ( pView && pView->AreObjectsMarked() )
    {
        delete pChainFrom; delete pChainTo; pChainFrom = pChainTo = 0;
        SdrHdl* pHdl = pView->HitHandle( *pPt, *GetWin() );
        pView->BegDragObj( *pPt, GetWin(), pHdl );
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}
/*************************************************************************
|*
|*  SwFEShell::Drag()
|*
|*  Ersterstellung      MS 10.06.92
|*  Letzte Aenderung    MA 13. Mar. 96
|*
*************************************************************************/

long SwFEShell::Drag( const Point *pPt, BOOL )
{
    ASSERT( Imp()->HasDrawView(), "Drag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
    {
        Scroll( *pPt );

#ifdef USED
JP 31.01.96: der Wunsch ist, das Orthogonale Resizen nur vom Benutzer
             vorgegeben werden kann.  (beschlossen und verkuendet: ST/WP/..)
        //OLE wird immer proportional resized.
        if ( !Imp()->GetDrawView()->IsMoveOnlyDragObj( TRUE ) &&
             GetCntType() == CNT_OLE )
            Imp()->GetDrawView()->SetOrtho( TRUE );
#endif

        Imp()->GetDrawView()->MovDragObj( *pPt );
        Imp()->GetDrawView()->ShowDragAnchor();
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::EndDrag()
|*
|*  Ersterstellung      MS 10.06.92
|*  Letzte Aenderung    MA 13. Mar. 96
|*
*************************************************************************/

long SwFEShell::EndDrag( const Point *, BOOL )
{
    ASSERT( Imp()->HasDrawView(), "EndDrag without DrawView?" );
    SdrView *pView = Imp()->GetDrawView();
    if ( pView->IsDragObj() )
    {
        //Start-/EndActions nur an der ViewShell aufsetzen
        ViewShell *pSh = this;
        do {
            pSh->StartAction();
        } while ( this != (pSh = (ViewShell*)pSh->GetNext()) );

        StartUndo( UNDO_START );

        //#50778# Bug im Draging: Im StartAction wird ein HideShowXor gerufen.
        //Im EndDragObj() wird dies unsinniger und faelschlicherweise wieder
        //Rueckgaengig gemacht. Um Konsistenz herzustellen muessen wir das
        //Xor also wieder zur Anzeige bringen.

        // Reanimation from the hack #50778 to fix bug #97057
        // May be not the best solution, but the one with lowest risc at the moment.
        pView->ShowShownXor( GetOut() );

        pView->EndDragObj();
        // JP 18.08.95: DrawUndo-Action auf FlyFrames werden nicht gespeichert
        //              Die Fly aendern das Flag
        GetDoc()->SetNoDrawUndoObj( FALSE );
        ChgAnchor( 0, TRUE );

        EndUndo( UNDO_END );

        do {
            pSh->EndAction();
            if( pSh->IsA( TYPE( SwCrsrShell ) ) )
                ((SwCrsrShell*)pSh)->CallChgLnk();
        } while ( this != (pSh = (ViewShell*)pSh->GetNext()) );

        GetDoc()->SetModified();
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::BreakDrag()
|*
|*  Ersterstellung      OM 02. Okt. 95
|*  Letzte Aenderung    OM 02. Okt. 95
|*
*************************************************************************/

void SwFEShell::BreakDrag()
{
    ASSERT( Imp()->HasDrawView(), "BreakDrag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
        Imp()->GetDrawView()->BrkDragObj();
    SetChainMarker();
}

/*************************************************************************
|*
|*  SwFEShell::SelFlyGrabCrsr()
|*
|*  Beschreibung        Wenn ein Fly selektiert ist, zieht er den Crsr in
|*                      den ersten CntntFrm
|*  Ersterstellung      MA 11. Dec. 92
|*  Letzte Aenderung    MA 07. Oct. 96
|*
*************************************************************************/

const SwFrmFmt* SwFEShell::SelFlyGrabCrsr()
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );

        if( pFly )
        {
            // --> OD 2004-06-11 #i28701# - no format here
//            pFly->GetAnchorFrm()->Calc();
            SwCntntFrm *pCFrm = pFly->ContainsCntnt();
            if ( pCFrm )
            {
                SwCntntNode *pCNode = pCFrm->GetNode();
                SwPaM       *pCrsr  = GetCrsr();

                pCrsr->GetPoint()->nNode = *pCNode;
                pCrsr->GetPoint()->nContent.Assign( pCNode, 0 );

                SwRect& rChrRect = (SwRect&)GetCharRect();
                rChrRect = pFly->Prt();
                rChrRect.Pos() += pFly->Frm().Pos();
                GetCrsrDocPos() = rChrRect.Pos();
            }
            return pFly->GetFmt();
        }
    }
    return 0;
}


/*************************************************************************
|*
|*  SwFEShell::SelectionToTop(), SelectionToBottom()
|*
|*  Beschreibung        Selektion nach oben/unten (Z-Order)
|*
|*  Ersterstellung      MA 05. Nov. 92
|*  Letzte Aenderung    MA 03. Jun. 96
|*
*************************************************************************/

void lcl_NotifyNeighbours( const SdrMarkList *pLst )
{
    //Die Regeln fuer die Ausweichmanoever haben sich veraendert.
    //1. Die Umgebung des Fly und aller innenliegenden muss benachrichtigt
    //   werden.
    //2. Der Inhalt des Rahmen selbst muss benachrichtigt werden.
    //3. Rahmen die dem Rahmen ausweichen bzw. wichen muessen benachrichtigt werden.
    //4. Auch Zeichenobjekte koennen Rahmen verdraengen

    for( USHORT j = 0; j < pLst->GetMarkCount(); ++j )
    {
        SwPageFrm *pPage;
        BOOL bCheckNeighbours = FALSE;
        SwHoriOrient aHori;
        SwRect aRect;
        SdrObject *pO = pLst->GetMark( 0 )->GetObj();
        if ( pO->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

            const SwFmtHoriOrient &rHori = pFly->GetFmt()->GetHoriOrient();
            aHori = rHori.GetHoriOrient();
            if( HORI_NONE != aHori && HORI_CENTER != aHori &&
                pFly->IsFlyAtCntFrm() )
            {
                bCheckNeighbours = TRUE;
                pFly->InvalidatePos();
                pFly->Frm().Pos().Y() += 1;
            }

            pPage = pFly->FindPageFrm();
            aRect = pFly->Frm();
        }
        else
        {
            SwFrm* pAnch = ( (SwDrawContact*)GetUserCall(pO) )->GetAnchorFrm( pO );
            if( !pAnch )
                continue;
            pPage = pAnch->FindPageFrm();
            aRect = GetBoundRect( pO );
        }

        sal_uInt32 nCount = pPage->GetSortedObjs() ? pPage->GetSortedObjs()->Count() : 0;
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( !pAnchoredObj->ISA(SwFlyFrm) )
                continue;

            SwFlyFrm* pAct = static_cast<SwFlyFrm*>(pAnchoredObj);
            SwRect aTmpCalcPnt( pAct->Prt() );
            aTmpCalcPnt += pAct->Frm().Pos();
            if ( aRect.IsOver( aTmpCalcPnt ) )
            {
                SwCntntFrm *pCnt = pAct->ContainsCntnt();
                while ( pCnt )
                {
                    aTmpCalcPnt = pCnt->Prt();
                    aTmpCalcPnt += pCnt->Frm().Pos();
                    if ( aRect.IsOver( aTmpCalcPnt ) )
                        ((SwFrm*)pCnt)->Prepare( PREP_FLY_ATTR_CHG );
                    pCnt = pCnt->GetNextCntntFrm();
                }
            }
            if ( bCheckNeighbours && pAct->IsFlyAtCntFrm() )
            {
                const SwFmtHoriOrient &rH = pAct->GetFmt()->GetHoriOrient();
                if ( rH.GetHoriOrient() == aHori &&
                     pAct->Frm().Top()    <= aRect.Bottom() &&
                     pAct->Frm().Bottom() >= aRect.Top() )
                {
                    pAct->InvalidatePos();
                    pAct->Frm().Pos().Y() += 1;
                }
            }
        }
    }
}

void SwFEShell::SelectionToTop( BOOL bTop )
{
    ASSERT( Imp()->HasDrawView(), "SelectionToTop without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    ASSERT( rMrkList.GetMarkCount(), "Kein Object Selektiert." );

    SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInCntFrm() )
        return;

    StartAllAction();
    if ( bTop )
        Imp()->GetDrawView()->PutMarkedToTop();
    else
        Imp()->GetDrawView()->MovMarkedToTop();
    ::lcl_NotifyNeighbours( &rMrkList );
    GetDoc()->SetModified();
    EndAllAction();
}

void SwFEShell::SelectionToBottom( BOOL bBottom )
{
    ASSERT( Imp()->HasDrawView(), "SelectionToBottom without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    ASSERT( rMrkList.GetMarkCount(), "Kein Object Selektiert." );

    SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInCntFrm() )
        return;

    StartAllAction();
    if ( bBottom )
        Imp()->GetDrawView()->PutMarkedToBtm();
    else
        Imp()->GetDrawView()->MovMarkedToBtm();
    ::lcl_NotifyNeighbours( &rMrkList );
    GetDoc()->SetModified();
    EndAllAction();
}

/*************************************************************************
|*
|*  SwFEShell::GetLayerId()
|*
|*  Beschreibung        Objekt ueber/unter dem Dokument?
|*                      2 Controls, 1 Heaven, 0 Hell, -1 Uneindeutig
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 20. Dec. 94
|*
*************************************************************************/

short SwFEShell::GetLayerId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            const SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            if ( nRet == SHRT_MAX )
                nRet = pObj->GetLayer();
            else if ( nRet != pObj->GetLayer() )
            {
                nRet = -1;
                break;
            }
        }
    }
    if ( nRet == SHRT_MAX )
        nRet = -1;
    return nRet;
}

/*************************************************************************
|*
|*  SwFEShell::SelectionToHeaven(), SelectionToHell()
|*
|*  Beschreibung        Objekt ueber/unter dem Dokument
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    AMA 04. Jun. 98
|*
*************************************************************************/
// OD 25.06.2003 #108784#
// Note: only visible objects can be marked. Thus, objects with invisible
//       layer IDs have not to be considered.
//       If <SwFEShell> exists, layout exists!!
void SwFEShell::ChangeOpaque( SdrLayerID nLayerId )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        // OD 25.06.2003 #108784# - correct type of <nControls>
        const SdrLayerID nControls = GetDoc()->GetControlsId();
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject* pObj = rMrkList.GetMark( i )->GetObj();
            // OD 21.08.2003 #i18447# - no change of layer for controls
            // or group objects containing controls.
            const bool bControlObj = ::CheckControlLayer( pObj );
            //if ( pObj->GetLayer() != nLayerId && pObj->GetLayer() != nControls )
            if ( !bControlObj && pObj->GetLayer() != nLayerId )
            {
                pObj->SetLayer( nLayerId );
                InvalidateWindows( SwRect( pObj->GetCurrentBoundRect() ) );
                if ( pObj->ISA(SwVirtFlyDrawObj) )
                {
                    SwFmt *pFmt = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetFmt();
                    SvxOpaqueItem aOpa( pFmt->GetOpaque() );
                    aOpa.SetValue(  nLayerId == GetDoc()->GetHellId() );
                    pFmt->SetAttr( aOpa );
                }
            }
        }
        GetDoc()->SetModified();
    }
}

void SwFEShell::SelectionToHeaven()
{
    ChangeOpaque( GetDoc()->GetHeavenId() );
}

void SwFEShell::SelectionToHell()
{
    ChangeOpaque( GetDoc()->GetHellId() );
}

/*************************************************************************
|*
|*  SwFEShell::IsObjSelected(), IsFrmSelected()
|*
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    MA 17. Jan. 95
|*
*************************************************************************/

USHORT SwFEShell::IsObjSelected() const
{
    if ( IsFrmSelected() || !Imp()->HasDrawView() )
        return 0;
    else
        return USHORT( Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() );
}

BOOL SwFEShell::IsFrmSelected() const
{
    if ( !Imp()->HasDrawView() )
        return FALSE;
    else
        return 0 != ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
                                        (ViewShell*)this );
}

#ifdef ACCESSIBLE_LAYOUT
sal_Bool SwFEShell::IsObjSelected( const SdrObject& rObj ) const
{
    if ( IsFrmSelected() || !Imp()->HasDrawView() )
        return sal_False;
    else
        return Imp()->GetDrawView()
                    ->IsObjMarked( const_cast< SdrObject * >( &rObj ) );
}
#endif

Rectangle *SwFEShell::IsAnchorAtPos( const Point &rPt ) const
{
    if ( !Imp()->HasDrawView() )
        return 0;
    else
        return Imp()->GetDrawView()->IsAnchorAtPos( rPt );
}

/*************************************************************************
|*
|*  SwFEShell::EndTextEdit()
|*
|*  Ersterstellung      MA 19. Feb. 96
|*  Letzte Aenderung    MA 19. Feb. 96
|*
*************************************************************************/

void SwFEShell::EndTextEdit()
{
    //Beenden des TextEditModus. Wenn gewuenscht (default wenn das Objekt
    //keinen Text mehr enthaelt und keine Attribute traegt) wird das
    //Objekt gel”scht. Alle anderen markierten Objekte bleiben erhalten.

    ASSERT( Imp()->HasDrawView() && Imp()->GetDrawView()->IsTextEdit(),
            "EndTextEdit an no Object" );

    StartAllAction();
    SdrView *pView = Imp()->GetDrawView();
    SdrObject *pObj = pView->GetTextEditObject();
    SdrObjUserCall* pUserCall;
    if( 0 != ( pUserCall = GetUserCall(pObj) ) )
    {
        SdrObject *pTmp = ((SwContact*)pUserCall)->GetMaster();
        if( !pTmp )
            pTmp = pObj;
        pUserCall->Changed( *pTmp, SDRUSERCALL_RESIZE, pTmp->GetLastBoundRect() );
    }
    if ( !pObj->GetUpGroup() )
    {
        if ( SDRENDTEXTEDIT_SHOULDBEDELETED == pView->EndTextEdit( TRUE ) )
        {
            if ( pView->GetMarkedObjectList().GetMarkCount() > 1 )
            {
                {
                    SdrMarkList aSave( pView->GetMarkedObjectList() );
                    aSave.DeleteMark( aSave.FindObject( pObj ) );
                    if ( aSave.GetMarkCount() )
                    {
                        pView->UnmarkAll();
                        pView->MarkObj( pObj, Imp()->GetPageView() );
                    }
                    DelSelectedObj();
                    if ( aSave.GetMarkCount() )
                    {
                        for ( USHORT i = 0; i < aSave.GetMarkCount(); ++i )
                            pView->MarkObj( aSave.GetMark( i )->GetObj(),
                                            Imp()->GetPageView() );
                    }
                }
            }
            else
                DelSelectedObj();
        }
    }
    else
        pView->EndTextEdit();
    EndAllAction();
}

/*************************************************************************
|*
|*  SwFEShell::IsInsideSelectedObj()
|*
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    MA 08. Nov. 96
|*
*************************************************************************/

int SwFEShell::IsInsideSelectedObj( const Point &rPt )
{
    if( Imp()->HasDrawView() )
    {
        SwDrawView *pDView = Imp()->GetDrawView();

        if( pDView->GetMarkedObjectList().GetMarkCount() &&
            pDView->IsMarkedObjHit( rPt ) )
        {
            return SDRHIT_OBJECT;
        }
    }
    return SDRHIT_NONE;
}

/*************************************************************************
|*
|*  SwFEShell::IsObjSelectable()
|*
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    MA 02. Feb. 95
|*
*************************************************************************/

int SwFEShell::IsObjSelectable( const Point& rPt )
{
    SET_CURR_SHELL(this);
#ifdef OLD
    if( Imp()->HasDrawView() )
        return Imp()->GetDrawView()->PickSomething( rPt, MINMOVE );
    return 0;
#else
    SwDrawView *pDView = Imp()->GetDrawView();
    BOOL bRet = FALSE;
    if( pDView )
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        USHORT nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        bRet = pDView->PickObj( rPt, pObj, pPV, SDRSEARCH_PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
    }
    return bRet ? 1 : 0;
#endif
}

// #107513#
// Test if there is a draw object at that position and if it should be selected.
// The 'should' is aimed at Writer text fly frames which may be in front of
// the draw object.
sal_Bool SwFEShell::ShouldObjectBeSelected(const Point& rPt)
{
    SET_CURR_SHELL(this);
    SwDrawView *pDrawView = Imp()->GetDrawView();
    sal_Bool bRet(sal_False);

    if(pDrawView)
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        sal_uInt16 nOld(pDrawView->GetHitTolerancePixel());

        pDrawView->SetHitTolerancePixel(pDrawView->GetMarkHdlSizePixel()/2);
        bRet = pDrawView->PickObj(rPt, pObj, pPV, SDRSEARCH_PICKMARKABLE);
        pDrawView->SetHitTolerancePixel(nOld);

        if(bRet && pObj)
        {
            SdrPage* pPage = GetDoc()->GetDrawModel()->GetPage(0);

            for(sal_uInt32 a(pObj->GetOrdNumDirect() + 1); bRet && a < pPage->GetObjCount(); a++)
            {
                SdrObject *pCandidate = pPage->GetObj(a);

                if(pCandidate->ISA(SwVirtFlyDrawObj) && ((SwVirtFlyDrawObj*)pCandidate)->GetCurrentBoundRect().IsInside(rPt))
                {
                    bRet = sal_False;
                }
            }
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GotoObj()
|*
|*  Beschreibung        Wenn ein Obj selektiert ist, gehen wir von dessen
|*      TopLeft aus, andernfalls von der Mitte des aktuellen CharRects.
|*  Ersterstellung      MA 01. Jun. 95
|*  Letzte Aenderung    MA 30. Apr. 96
|*
*************************************************************************/
/* -----------------23.09.98 10:29-------------------
 * Beinhaltet das Objekt ein Control oder Gruppen,
 * die nur aus Controls bestehen
 * --------------------------------------------------*/
FASTBOOL lcl_IsControlGroup( const SdrObject *pObj )
{
    BOOL bRet = FALSE;
    if(pObj->ISA(SdrUnoObj))
        bRet = TRUE;
    else if( pObj->ISA( SdrObjGroup ) )
    {
        bRet = TRUE;
        const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( USHORT i = 0; i < pLst->GetObjCount(); ++i )
            if( !::lcl_IsControlGroup( pLst->GetObj( i ) ) )
                return FALSE;
    }
    return bRet;
}

BOOL SwFEShell::GotoObj( BOOL bNext, GotoObjType eType )
{
    if( !Imp()->HasDrawView() )
        return FALSE;
    else
    {
        const SdrObject *pBest  = 0,
                        *pTop   = 0;

        const long nTmp = bNext ? LONG_MAX : 0;
        Point aBestPos( nTmp, nTmp );
        Point aTopPos(  nTmp, nTmp );
        Point aCurPos;
        Point aPos;
        BOOL  bRet = FALSE;
        BOOL bNoDraw = 0 == (DRAW_ANY & eType);
        BOOL bNoFly = 0 == (FLY_ANY & eType);

        if( !bNoFly && bNoDraw )
        {
            SwFlyFrm *pFly = GetCurrFrm( FALSE )->FindFlyFrm();
            if( pFly )
                pBest = pFly->GetVirtDrawObj();
        }
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        SdrPageView* pPV = Imp()->GetDrawView()->GetPageViewPvNum( 0 );

        if( !pBest || rMrkList.GetMarkCount() == 1 )
        {
            // Ausgangspunkt bestimmen.
            SdrObjList* pList = NULL;
            if ( rMrkList.GetMarkCount() )
            {
                const SdrObject* pStartObj = rMrkList.GetMark(0)->GetObj();
                if( pStartObj->ISA(SwVirtFlyDrawObj) )
                    aPos = ((SwVirtFlyDrawObj*)pStartObj)->GetFlyFrm()->Frm().Pos();
                else
                    aPos = pStartObj->GetSnapRect().TopLeft();

                // If an object inside a group is selected, we want to
                // iterate over the group members.
                if ( ! pStartObj->GetUserCall() )
                    pList = pStartObj->GetObjList();
            }
            else
            {
                // If no object is selected, we check if we just entered a group.
                // In this case we want to iterate over the group members.
                aPos = GetCharRect().Center();
                const SdrObject* pStartObj = pPV ? pPV->GetAktGroup() : 0;
                if ( pStartObj && pStartObj->ISA( SdrObjGroup ) )
                    pList = pStartObj->GetSubList();
            }

            if ( ! pList )
            {
                // Here we are if
                // A  No object has been selected and no group has been entered or
                // B  An object has been selected and it is not inside a group
                pList = GetDoc()->GetDrawModel()->GetPage( 0 );
            }


            ASSERT( pList, "No object list to iterate" )

            const ULONG nObjs = pList->GetObjCount();
            for( ULONG nObj = 0; nObj < nObjs; ++nObj )
            {
                SdrObject* pObj = pList->GetObj( nObj );
                BOOL bFlyFrm = pObj->ISA(SwVirtFlyDrawObj);
                if( ( bNoFly && bFlyFrm ) ||
                    ( bNoDraw && !bFlyFrm ) ||
                    ( eType == DRAW_SIMPLE && lcl_IsControlGroup( pObj ) ) ||
                    ( eType == DRAW_CONTROL && !lcl_IsControlGroup( pObj ) ) ||
                    ( pPV && ! pPV->GetView().IsObjMarkable( pObj, pPV ) ) )
                    continue;
                if( bFlyFrm )
                {
                    SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pObj;
                    SwFlyFrm *pFly = pO->GetFlyFrm();
                    if( FLY_ANY != ( FLY_ANY & eType ) )
                    {
                        switch ( eType )
                        {
                            case FLY_FRM:
                                if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                                    continue;
                            break;
                            case FLY_GRF:
                                if ( pFly->Lower() &&
                                    (pFly->Lower()->IsLayoutFrm() ||
                                    !((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode()))
                                    continue;
                            break;
                            case FLY_OLE:
                                if ( pFly->Lower() &&
                                    (pFly->Lower()->IsLayoutFrm() ||
                                    !((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode()))
                                    continue;
                            break;
                        }
                    }
                    aCurPos = pFly->Frm().Pos();
                }
                else
                    aCurPos = pObj->GetCurrentBoundRect().TopLeft();

                // Sonderfall wenn ein anderes Obj auf selber Y steht.
                if( aCurPos != aPos &&          // nur wenn ich es nicht selber bin
                    aCurPos.Y() == aPos.Y() &&  // ist die Y Position gleich
                    (bNext? (aCurPos.X() > aPos.X()) :  // liegt neben mir
                            (aCurPos.X() < aPos.X())) ) // " reverse
                {
                    aBestPos = Point( nTmp, nTmp );
                    for( ULONG i = 0; i < nObjs; ++i )
                    {
                        SdrObject *pObj = pList->GetObj( i );
                        bFlyFrm = pObj->ISA(SwVirtFlyDrawObj);
                        if( ( bNoFly && bFlyFrm ) || ( bNoDraw && !bFlyFrm ) )
                            continue;
                        if( bFlyFrm )
                        {
                            SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pObj;
                            aCurPos = pO->GetFlyFrm()->Frm().Pos();
                        }
                        else
                            aCurPos = pObj->GetCurrentBoundRect().TopLeft();

                        if( aCurPos != aPos && aCurPos.Y() == aPos.Y() &&
                            (bNext? (aCurPos.X() > aPos.X()) :  // liegt neben mir
                                    (aCurPos.X() < aPos.X())) &&    // " reverse
                            (bNext? (aCurPos.X() < aBestPos.X()) :  // besser als Beste
                                    (aCurPos.X() > aBestPos.X())) ) // " reverse
                        {
                            aBestPos = aCurPos;
                            pBest = pObj;
                        }
                    }
                    break;
                }

                if( (bNext? (aPos.Y() < aCurPos.Y()) :          // nur unter mir
                            (aPos.Y() > aCurPos.Y())) &&        // " reverse
                    (bNext? (aBestPos.Y() > aCurPos.Y()) :      // naeher drunter
                            (aBestPos.Y() < aCurPos.Y())) ||    // " reverse
                            (aBestPos.Y() == aCurPos.Y() &&
                    (bNext? (aBestPos.X() > aCurPos.X()) :      // weiter links
                            (aBestPos.X() < aCurPos.X()))))     // " reverse

                {
                    aBestPos = aCurPos;
                    pBest = pObj;
                }

                if( (bNext? (aTopPos.Y() > aCurPos.Y()) :       // hoeher als Beste
                            (aTopPos.Y() < aCurPos.Y())) ||     // " reverse
                            (aTopPos.Y() == aCurPos.Y() &&
                    (bNext? (aTopPos.X() > aCurPos.X()) :       // weiter links
                            (aTopPos.X() < aCurPos.X()))))      // " reverse
                {
                    aTopPos = aCurPos;
                    pTop = pObj;
                }
            }
            // leider nichts gefunden
            if( (bNext? (aBestPos.X() == LONG_MAX) : (aBestPos.X() == 0)) )
                pBest = pTop;
        }

        if( pBest )
        {
            BOOL bFlyFrm = pBest->ISA(SwVirtFlyDrawObj);
            if( bFlyFrm )
            {
                SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pBest;
                const SwRect& rFrm = pO->GetFlyFrm()->Frm();
                SelectObj( rFrm.Pos(), 0, (SdrObject*)pBest );
                if( !ActionPend() )
                    MakeVisible( rFrm );
            }
            else
            {
                SelectObj( Point(), 0, (SdrObject*)pBest );
                if( !ActionPend() )
                    MakeVisible( pBest->GetCurrentBoundRect() );
            }
            CallChgLnk();
            bRet = TRUE;
        }
        return bRet;
    }
}

/*************************************************************************
|*
|*  SwFEShell::ControlCount(), GotoControl()
|*
|*  Ersterstellung      MA 22. Jul. 96
|*  Letzte Aenderung    MA 22. Jul. 96
|*
*************************************************************************/

ULONG SwFEShell::ControlCount() const
{
    INT32 nRet = 0;
    if ( Imp()->HasDrawView() )
    {
        SdrPage* pPage = GetDoc()->GetDrawModel()->GetPage( 0 );
        const ULONG nObjs = pPage->GetObjCount();
        for( ULONG nObj = 0; nObj < nObjs; ++nObj )
        {
            if ( pPage->GetObj( nObj )->ISA( SdrUnoObj ) )
                ++nRet;
        }
    }
    return nRet;
}

BOOL SwFEShell::GotoControl( ULONG nIndex )
{
    if ( Imp()->HasDrawView() )
    {
        ULONG nIdx = 0;
        SdrPage* pPage = GetDoc()->GetDrawModel()->GetPage( 0 );
        const ULONG nObjs = pPage->GetObjCount();
        for( ULONG nObj = 0; nObj < nObjs; ++nObj )
        {
            SdrObject *pObj = pPage->GetObj( nObj );
            if ( pObj->ISA( SdrUnoObj ) )
            {
                if ( nIdx == nIndex )
                {
                    SelectObj( Point(), 0, pObj );
                    if( !ActionPend() )
                        MakeVisible( pObj->GetCurrentBoundRect() );
                    CallChgLnk();
                    return TRUE;
                }
                ++nIdx;
            }
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*  SwFEShell::BeginCreate()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 21. Mar. 95
|*
*************************************************************************/

BOOL SwFEShell::BeginCreate( UINT16 eSdrObjectKind, const Point &rPos )
{
    BOOL bRet = FALSE;

    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        Imp()->GetDrawView()->SetCurrentObj( eSdrObjectKind );
        if ( eSdrObjectKind == OBJ_CAPTION )
            bRet = Imp()->GetDrawView()->BegCreateCaptionObj(
                        rPos, Size( lMinBorder - MINFLY, lMinBorder - MINFLY ),
                        GetOut() );
        else
            bRet = Imp()->GetDrawView()->BegCreateObj( rPos, GetOut() );
    }
    if ( bRet )
    {
        ::FrameNotify( this, FLY_DRAG_START );
    }
    return bRet;
}

BOOL SwFEShell::BeginCreate( UINT16 eSdrObjectKind, UINT32 eObjInventor,
                             const Point &rPos )
{
    BOOL bRet = FALSE;

    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        Imp()->GetDrawView()->SetCurrentObj( eSdrObjectKind, eObjInventor );
        bRet = Imp()->GetDrawView()->BegCreateObj( rPos, GetOut() );
    }
    if ( bRet )
        ::FrameNotify( this, FLY_DRAG_START );
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::MoveCreate()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 24. Jan. 95
|*
*************************************************************************/

void SwFEShell::MoveCreate( const Point &rPos )
{
    ASSERT( Imp()->HasDrawView(), "MoveCreate without DrawView?" );
    if ( GetPageNumber( rPos ) )
    {
        Scroll( rPos );
        Imp()->GetDrawView()->MovCreateObj( rPos );
        ::FrameNotify( this, FLY_DRAG );
    }
}

/*************************************************************************
|*
|*  SwFEShell::EndCreate(), ImpEndCreate()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 14. Oct. 96
|*
*************************************************************************/

BOOL SwFEShell::EndCreate( UINT16 eSdrCreateCmd )
{
    // JP 18.08.95: Damit das Undo-Object aus der DrawEngine nicht bei uns
    // gespeichert wird, (wir erzeugen ein eigenes Undo-Object!) hier kurz
    // das Undo abschalten
    ASSERT( Imp()->HasDrawView(), "EndCreate without DrawView?" );
    if( !Imp()->GetDrawView()->IsGroupEntered() )
        GetDoc()->SetNoDrawUndoObj( TRUE );
    FASTBOOL bCreate = Imp()->GetDrawView()->EndCreateObj(
                                    SdrCreateCmd( eSdrCreateCmd ) );
    GetDoc()->SetNoDrawUndoObj( FALSE );

    if ( !bCreate )
    {
        ::FrameNotify( this, FLY_DRAG_END );
        return FALSE;
    }

    if ( (SdrCreateCmd)eSdrCreateCmd == SDRCREATE_NEXTPOINT )
    {
        ::FrameNotify( this, FLY_DRAG );
        return TRUE;
    }
    return ImpEndCreate();
}


BOOL SwFEShell::ImpEndCreate()
{
    ASSERT( Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1,
            "Neues Object nicht selektiert." );

    SdrObject& rSdrObj = *Imp()->GetDrawView()->GetMarkedObjectList().GetMark(0)->GetObj();

    if( rSdrObj.GetSnapRect().IsEmpty() )
    {
        //JP 10.04.95: das Object vergessen wir lieber, fuerht nur
        //              zu Problemen
        Imp()->GetDrawView()->DeleteMarked();
        Imp()->GetDrawView()->UnmarkAll();
        ::FrameNotify( this, FLY_DRAG_END );
        return FALSE;
    }

    if( rSdrObj.GetUpGroup() )
    {
        Point aTmpPos( rSdrObj.GetSnapRect().TopLeft() );
        Point aNewAnchor( rSdrObj.GetUpGroup()->GetAnchorPos() );
        // OD 2004-04-05 #i26791# - direct object positioning for group members
        rSdrObj.NbcSetRelativePos( aTmpPos - aNewAnchor );
        rSdrObj.NbcSetAnchorPos( aNewAnchor );
        ::FrameNotify( this, FLY_DRAG );
        return TRUE;
    }

    StartAllAction();

    Imp()->GetDrawView()->UnmarkAll();

    SwPaM* pCrsr = GetCrsr();

    const Rectangle &rBound = rSdrObj.GetSnapRect();
    Point aPt( rBound.TopRight() );

    //Fremde Identifier sollen in den Default laufen.
    //Ueberschneidungen sind moeglich!!
    UINT16 nIdent = SdrInventor == rSdrObj.GetObjInventor()
                        ? rSdrObj.GetObjIdentifier()
                        : 0xFFFF;

    //Default fuer Controls ist Zeichengebunden, Absatzgebunden sonst.
    SwFmtAnchor aAnch;
    const SwFrm *pAnch = 0;
    FASTBOOL bCharBound = FALSE;
    if( rSdrObj.ISA( SdrUnoObj ) )
    {
        SwPosition aPos( GetDoc()->GetNodes() );
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt.X(), aPt.Y() + rBound.GetHeight()/2 );
        GetDoc()->GetRootFrm()->GetCrsrOfst( &aPos, aPoint, &aState );

        //JP 22.01.99: Zeichenbindung ist im ReadnOnly-Inhalt nicht erlaubt
        if( !aPos.nNode.GetNode().IsProtect() )
        {
            pAnch = aPos.nNode.GetNode().GetCntntNode()->GetFrm( &aPoint, &aPos );
            SwRect aTmp;
            pAnch->GetCharRect( aTmp, aPos );

            //Der Crsr darf nicht zu weit entfernt sein.
            bCharBound = TRUE;
            Rectangle aRect( aTmp.SVRect() );
            aRect.Left()  -= MM50*2;
            aRect.Top()   -= MM50*2;
            aRect.Right() += MM50*2;
            aRect.Bottom()+= MM50*2;

            if( !aRect.IsOver( rBound ) && !::GetHtmlMode( GetDoc()->GetDocShell() ))
                bCharBound = FALSE;

                //Bindung in Kopf-/Fusszeilen ist ebenfalls nicht erlaubt.
            if( bCharBound )
                bCharBound = !GetDoc()->IsInHeaderFooter( aPos.nNode );

            if( bCharBound )
            {
                aAnch.SetType( FLY_IN_CNTNT );
                aAnch.SetAnchor( &aPos );
            }
        }
    }

    if( !bCharBound )
    {
        // OD 16.05.2003 #108784# - allow native drawing objects in header/footer.
        // Thus, set <bBodyOnly> to <false> for these objects using value
        // of <nIdent> - value <0xFFFF> indicates control objects, which aren't
        // allowed in header/footer.
        //bool bBodyOnly = OBJ_NONE != nIdent;
        bool bBodyOnly = 0xFFFF == nIdent;
        bool bAtPage = false;
        const SwFrm* pPage = 0;
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt );
        SwPosition aPos( GetDoc()->GetNodes() );
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState );

        //JP 22.01.99: nicht in ReadnOnly-Inhalt setzen
        if( aPos.nNode.GetNode().IsProtect() )
            // dann darf er nur seitengebunden sein. Oder sollte man
            // die naechste nicht READONLY Position suchen?
            bAtPage = true;

        pAnch = aPos.nNode.GetNode().GetCntntNode()->GetFrm( &aPoint, 0, FALSE );

        if( !bAtPage )
        {
            const SwFlyFrm *pTmp = pAnch->FindFlyFrm();
            if( pTmp )
            {
                const SwFrm* pTmpFrm = pAnch;
                SwRect aBound( rBound );
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( aBound ) )
                    {
                        if( !bBodyOnly || !pTmp->FindFooterOrHeader() )
                            pPage = pTmpFrm;
                        break;
                    }
                    pTmp = pTmp->GetAnchorFrm()
                                ? pTmp->GetAnchorFrm()->FindFlyFrm()
                                : 0;
                    pTmpFrm = pTmp;
                }
            }

            if( !pPage )
                pPage = pAnch->FindPageFrm();

            // immer ueber FindAnchor gehen, damit der Frame immer an den
            // davorgehen gebunden wird. Beim GetCrsOfst kann man auch zum
            // nachfolgenden kommen. DAS IST FALSCH
            pAnch = ::FindAnchor( pPage, aPt, bBodyOnly );
            aPos.nNode = *((SwCntntFrm*)pAnch)->GetNode();

            //JP 22.01.99: nicht in ReadnOnly-Inhalt setzen
            if( aPos.nNode.GetNode().IsProtect() )
                // dann darf er nur seitengebunden sein. Oder sollte man
                // die naechste nicht READONLY Position suchen?
                bAtPage = true;
            else
            {
                aAnch.SetType( FLY_AT_CNTNT );
                aAnch.SetAnchor( &aPos );
            }
        }

        if( bAtPage )
        {
            pPage = pAnch->FindPageFrm();

            aAnch.SetType( FLY_PAGE );
            aAnch.SetPageNum( pPage->GetPhyPageNum() );
            pAnch = pPage;      // die Page wird jetzt zum Anker
        }
    }

    SfxItemSet aSet( GetDoc()->GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_SURROUND, RES_ANCHOR, 0 );
    aSet.Put( aAnch );

    // OD 2004-03-30 #i26791# - determine relative object position
    SwTwips nXOffset;
    SwTwips nYOffset = rBound.Top() - pAnch->Frm().Top();
    {
        SWRECTFN( pAnch )
        if( bVert )
        {
            nXOffset = nYOffset;
            nYOffset = pAnch->Frm().Left()+pAnch->Frm().Width()-rBound.Right();
        }
        else if( pAnch->IsRightToLeft() )
            nXOffset = pAnch->Frm().Left()+pAnch->Frm().Width()-rBound.Right();
        else
            nXOffset = rBound.Left() - pAnch->Frm().Left();
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                ASSERT( pTmp, "Where's my Master?" );
                // OD 2004-03-30 #i26791# - correction: add frame area height
                // of master frames.
                nYOffset += pTmp->IsVertical() ?
                            pTmp->Frm().Width() : pTmp->Frm().Height();
            } while ( pTmp->IsFollow() );
        }
    }

    if( OBJ_NONE == nIdent )
    {
        //Bei OBJ_NONE wird ein Fly eingefuegt.
        const long nWidth = rBound.Right()  - rBound.Left();
        const long nHeight= rBound.Bottom() - rBound.Top();
        aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE, Max( nWidth,  long(MINFLY) ),
                                              Max( nHeight, long(MINFLY) )));

        SwFmtHoriOrient aHori( nXOffset, HORI_NONE, FRAME );
        SwFmtVertOrient aVert( nYOffset, VERT_NONE, FRAME );
        aSet.Put( SwFmtSurround( SURROUND_PARALLEL ) );
        aSet.Put( aHori );
        aSet.Put( aVert );

        //Schnell noch das Rechteck merken
        const SwRect aFlyRect( rBound );

        //Erzeugtes Object wegwerfen, so kann der Fly am elegentesten
        //ueber vorhandene SS erzeugt werden.
        GetDoc()->SetNoDrawUndoObj( TRUE );         // siehe oben
        SdrPage *pPg = GetDoc()->MakeDrawModel()->GetPage( 0 );
        if( !pPg )
        {
            pPg = GetDoc()->GetDrawModel()->AllocPage( FALSE );
            GetDoc()->GetDrawModel()->InsertPage( pPg );
        }
        pPg->RecalcObjOrdNums();
        delete pPg->RemoveObject( rSdrObj.GetOrdNumDirect() );
        GetDoc()->SetNoDrawUndoObj( FALSE );

        SwFlyFrm* pFlyFrm;
        if( NewFlyFrm( aSet, TRUE ) &&
            ::GetHtmlMode( GetDoc()->GetDocShell() ) &&
            0 != ( pFlyFrm = FindFlyFrm() ))
        {
            SfxItemSet aHtmlSet( GetDoc()->GetAttrPool(), RES_VERT_ORIENT, RES_HORI_ORIENT );
            //Horizontale Ausrichtung:
            const FASTBOOL bLeftFrm = aFlyRect.Left() <
                                      pAnch->Frm().Left() + pAnch->Prt().Left(),
                           bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                                      pAnch->Frm().Left() + pAnch->Prt().Width()/2;
            if( bLeftFrm || bLeftPrt )
            {
                aHori.SetHoriOrient( HORI_LEFT );
                aHori.SetRelationOrient( bLeftFrm ? FRAME : PRTAREA );
            }
            else
            {
                const FASTBOOL bRightFrm = aFlyRect.Left() >
                                           pAnch->Frm().Left() + pAnch->Prt().Width();
                aHori.SetHoriOrient( HORI_RIGHT );
                aHori.SetRelationOrient( bRightFrm ? FRAME : PRTAREA );
            }
            aHtmlSet.Put( aHori );
            aVert.SetVertOrient( VERT_TOP );
            aVert.SetRelationOrient( PRTAREA );
            aHtmlSet.Put( aVert );

            GetDoc()->SetAttr( aHtmlSet, *pFlyFrm->GetFmt() );
        }
    }
    else
    {
        Point aRelNullPt;
        if( OBJ_CAPTION == nIdent )
            aRelNullPt = ((SdrCaptionObj&)rSdrObj).GetTailPos();
        else
            aRelNullPt = rBound.TopLeft();

        aSet.Put( aAnch );
        aSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
        // OD 2004-03-30 #i26791# - set horizontal position
        SwFmtHoriOrient aHori( nXOffset, HORI_NONE, FRAME );
        aSet.Put( aHori );
        // OD 2004-03-30 #i26791# - set vertical position
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                ASSERT( pTmp, "Where's my Master?" );
                nYOffset += pTmp->IsVertical() ?
                            pTmp->Prt().Width() : pTmp->Prt().Height();
            } while ( pTmp->IsFollow() );
        }
        SwFmtVertOrient aVert( nYOffset, VERT_NONE, FRAME );
        aSet.Put( aVert );
        SwDrawFrmFmt* pFmt = (SwDrawFrmFmt*)GetDoc()->MakeLayoutFmt( RND_DRAW_OBJECT, 0, &aSet );

        SwDrawContact *pContact = new SwDrawContact( pFmt, &rSdrObj );
        if( bCharBound )
        {
            ASSERT( aAnch.GetAnchorId() == FLY_IN_CNTNT, "wrong AnchorType" );
            SwTxtNode *pNd = aAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();
            pNd->Insert( SwFmtFlyCnt( pFmt ),
                            aAnch.GetCntntAnchor()->nContent.GetIndex(), 0 );
            SwFmtVertOrient aVert( pFmt->GetVertOrient() );
            aVert.SetVertOrient( VERT_LINE_CENTER );
            pFmt->SetAttr( aVert );
        }
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                ASSERT( pTmp, "Where's my Master?" );
            } while( pTmp->IsFollow() );
            pAnch = pTmp;
        }

        pContact->ConnectToLayout();

        // OD 25.06.2003 #108784# - mark object at frame the object is inserted at.
        {
            SdrObject* pMarkObj = pContact->GetDrawObjectByAnchorFrm( *pAnch );
            if ( pMarkObj )
            {
                Imp()->GetDrawView()->MarkObj( pMarkObj, Imp()->GetPageView(),
                                                FALSE, FALSE );
            }
            else
            {
                Imp()->GetDrawView()->MarkObj( &rSdrObj, Imp()->GetPageView(),
                                                FALSE, FALSE );
            }
        }
    }

    GetDoc()->SetModified();

    KillPams();
    EndAllActionAndCall();
    return TRUE;
}


/*************************************************************************
|*
|*  SwFEShell::BreakCreate()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    MA 09. Jan. 95
|*
*************************************************************************/

void SwFEShell::BreakCreate()
{
    ASSERT( Imp()->HasDrawView(), "BreakCreate without DrawView?" );
    Imp()->GetDrawView()->BrkCreateObj();
    ::FrameNotify( this, FLY_DRAG_END );
}

/*************************************************************************
|*
|*  SwFEShell::IsDrawCreate()
|*
|*  Ersterstellung      OM 16. Mar. 95
|*  Letzte Aenderung    OM 16. Mar. 95
|*
*************************************************************************/

BOOL SwFEShell::IsDrawCreate() const
{
    return Imp()->HasDrawView() ? Imp()->GetDrawView()->IsCreateObj() : FALSE;
}

/*************************************************************************
|*
|*  SwFEShell::BeginMark()
|*
|*  Ersterstellung      OM 07. Feb. 95
|*  Letzte Aenderung    OM 07. Feb. 95
|*
*************************************************************************/

BOOL SwFEShell::BeginMark( const Point &rPos )
{
    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        SwDrawView* pDView = Imp()->GetDrawView();

        if (pDView->HasMarkablePoints())
            return pDView->BegMarkPoints( rPos, (OutputDevice*) NULL );
        else
            return pDView->BegMarkObj( rPos, (OutputDevice*) NULL );
    }
    else
        return FALSE;
}

/*************************************************************************
|*
|*  SwFEShell::MoveMark()
|*
|*  Ersterstellung      OM 07. Feb. 95
|*  Letzte Aenderung    OM 07. Feb. 95
|*
*************************************************************************/

void SwFEShell::MoveMark( const Point &rPos )
{
    ASSERT( Imp()->HasDrawView(), "MoveMark without DrawView?" );

    if ( GetPageNumber( rPos ) )
    {
        Scroll( rPos );
        SwDrawView* pDView = Imp()->GetDrawView();
//      Imp()->GetDrawView()->MovMarkObj( rPos );

        if (pDView->IsInsObjPoint())
            pDView->MovInsObjPoint( rPos );
        else if (pDView->IsMarkPoints())
            pDView->MovMarkPoints( rPos );
        else
            pDView->MovAction( rPos );
    }
}

/*************************************************************************
|*
|*  SwFEShell::EndMark()
|*
|*  Ersterstellung      OM 07. Feb. 95
|*  Letzte Aenderung    MA 08. Feb. 95
|*
*************************************************************************/

BOOL SwFEShell::EndMark()
{
    BOOL bRet = FALSE;
    ASSERT( Imp()->HasDrawView(), "EndMark without DrawView?" );

    if (Imp()->GetDrawView()->IsMarkObj())
    {
        bRet = Imp()->GetDrawView()->EndMarkObj();

        if ( bRet )
        {
            BOOL bShowHdl = FALSE;
            SwDrawView* pDView = Imp()->GetDrawView();
            //Rahmen werden auf diese Art nicht Selektiert, es sein denn es
            //ist nur ein Rahmen.
            SdrMarkList &rMrkList = (SdrMarkList&)pDView->GetMarkedObjectList();
            SwFlyFrm* pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );

            if ( rMrkList.GetMarkCount() > 1 )
                for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
                    if( pObj->ISA(SwVirtFlyDrawObj) )
                    {
                        if ( !bShowHdl )
                        {
                            pDView->HideMarkHdl( GetOut() );
                            bShowHdl = TRUE;
                        }
                        rMrkList.DeleteMark( i );
                        --i;    //keinen auslassen.
                    }
                }

            if( bShowHdl )
            {
                pDView->MarkListHasChanged();
                pDView->AdjustMarkHdl();
                pDView->ShowMarkHdl( GetOut() );
            }

            if ( rMrkList.GetMarkCount() )
                ::lcl_GrabCursor(this, pOldSelFly);
            else
                bRet = FALSE;
        }
        if ( bRet )
            ::FrameNotify( this, FLY_DRAG_START );
    }
    else
    {
        if (Imp()->GetDrawView()->IsMarkPoints())
            bRet = Imp()->GetDrawView()->EndMarkPoints();
    }

    SetChainMarker();
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::BreakSelect()
|*
|*  Ersterstellung      OM 07. Feb. 95
|*  Letzte Aenderung    OM 07. Feb. 95
|*
*************************************************************************/

void SwFEShell::BreakMark()
{
    ASSERT( Imp()->HasDrawView(), "BreakMark without DrawView?" );
    Imp()->GetDrawView()->BrkMarkObj();
}

/*************************************************************************
|*
|*  SwFEShell::GetAnchorId()
|*
|*  Ersterstellung      MA 30. Jan. 95
|*  Letzte Aenderung    MA 30. Jan. 95
|*
*************************************************************************/

short SwFEShell::GetAnchorId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            if ( pObj->ISA(SwVirtFlyDrawObj) )
            {
                nRet = -1;
                break;
            }
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
            short nId = pContact->GetFmt()->GetAnchor().GetAnchorId();
            if ( nRet == SHRT_MAX )
                nRet = nId;
            else if ( nRet != nId )
            {
                nRet = -1;
                break;
            }
        }
    }
    if ( nRet == SHRT_MAX )
        nRet = -1;
    return nRet;
}

/*************************************************************************
|*
|*  SwFEShell::ChgAnchor()
|*
|*  Ersterstellung      MA 10. Jan. 95
|*  Letzte Aenderung    MA 30. May. 96
|*
*************************************************************************/

void SwFEShell::ChgAnchor( int eAnchorId, BOOL bSameOnly, BOOL bPosCorr )
{
    ASSERT( Imp()->HasDrawView(), "ChgAnchor without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if( rMrkList.GetMarkCount() &&
        !rMrkList.GetMark( 0 )->GetObj()->GetUpGroup() )
    {
        StartAllAction();

        if( GetDoc()->ChgAnchor( rMrkList, (RndStdIds)eAnchorId, bSameOnly, bPosCorr ))
            Imp()->GetDrawView()->UnmarkAll();

        EndAllAction();

        ::FrameNotify( this, FLY_DRAG );
    }
}

/*************************************************************************
|*
|*  SwFEShell::DelSelectedObj()
|*
|*  Ersterstellung      MA 03. Nov. 92
|*  Letzte Aenderung    MA 14. Nov. 95
|*
*************************************************************************/

void SwFEShell::DelSelectedObj()
{
    ASSERT( Imp()->HasDrawView(), "DelSelectedObj(), no DrawView available" );
    if ( Imp()->HasDrawView() )
    {
        StartAllAction();
        Imp()->GetDrawView()->DeleteMarked();
        EndAllAction();
        ::FrameNotify( this, FLY_DRAG_END );
    }
}

/*************************************************************************
|*
|*  SwFEShell::GetObjSize(), GetAnchorObjDiff()
|*
|*  Beschreibung        Fuer die Statuszeile zum Erfragen der aktuellen
|*                      Verhaeltnisse
|*  Ersterstellung      MA 25. Apr. 95
|*  Letzte Aenderung    MA 25. Apr. 95
|*
*************************************************************************/

Size SwFEShell::GetObjSize() const
{
    Rectangle aRect;
    if ( Imp()->HasDrawView() )
    {
        if ( Imp()->GetDrawView()->IsAction() )
            Imp()->GetDrawView()->TakeActionRect( aRect );
        else
            aRect = Imp()->GetDrawView()->GetAllMarkedRect();
    }
    return aRect.GetSize();
}

Point SwFEShell::GetAnchorObjDiff() const
{
    const SdrView *pView = Imp()->GetDrawView();
    ASSERT( pView, "GetAnchorObjDiff without DrawView?" );

    Rectangle aRect;
    if ( Imp()->GetDrawView()->IsAction() )
        Imp()->GetDrawView()->TakeActionRect( aRect );
    else
        aRect = Imp()->GetDrawView()->GetAllMarkedRect();

    Point aRet( aRect.TopLeft() );

    if ( IsFrmSelected() )
    {
        SwFlyFrm *pFly = FindFlyFrm();
        aRet -= pFly->GetAnchorFrm()->Frm().Pos();
    }
    else
    {
        const SdrObject *pObj = pView->GetMarkedObjectList().GetMarkCount() == 1 ?
                                pView->GetMarkedObjectList().GetMark(0)->GetObj() : 0;
        if ( pObj )
            aRet -= pObj->GetAnchorPos();
    }

    return aRet;
}

Point SwFEShell::GetObjAbsPos() const
{
    ASSERT( Imp()->GetDrawView(), "GetObjAbsPos() without DrawView?" );
    return Imp()->GetDrawView()->GetDragStat().GetActionRect().TopLeft();
}



/*************************************************************************
|*
|*  SwFEShell::IsGroupSelected()
|*
|*  Ersterstellung      MA 30. Jan. 95
|*  Letzte Aenderung    MA 30. May. 96
|*
*************************************************************************/

BOOL SwFEShell::IsGroupSelected()
{
    if ( IsObjSelected() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
            // OD 30.06.2003 #108784# - consider 'virtual' drawing objects.
            // Thus, use corresponding method instead of checking type.
            if ( pObj->IsGroupObject() &&
                 FLY_IN_CNTNT != ((SwDrawContact*)GetUserCall(pObj))->
                                      GetFmt()->GetAnchor().GetAnchorId() )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// OD 27.06.2003 #108784# - change return type.
// OD 27.06.2003 #108784# - adjustments for drawing objects in header/footer:
//      allow group, only if all selected objects are in the same header/footer
//      or not in header/footer.
bool SwFEShell::IsGroupAllowed() const
{
    bool bIsGroupAllowed = false;
    if ( IsObjSelected() > 1 )
    {
        bIsGroupAllowed = true;
        const SdrObject* pUpGroup = 0L;
        const SwFrm* pHeaderFooterFrm = 0L;
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( USHORT i = 0; bIsGroupAllowed && i < rMrkList.GetMarkCount(); ++i )
        {
            const SdrObject* pObj = rMrkList.GetMark( i )->GetObj();
            if ( i )
                bIsGroupAllowed = pObj->GetUpGroup() == pUpGroup;
            else
                pUpGroup = pObj->GetUpGroup();

            if ( bIsGroupAllowed &&
                 FLY_IN_CNTNT == ::FindFrmFmt( (SdrObject*)pObj )->GetAnchor().GetAnchorId() )
            {
                bIsGroupAllowed = false;
            }

            // OD 27.06.2003 #108784# - check, if all selected objects are in the
            // same header/footer or not in header/footer.
            if ( bIsGroupAllowed )
            {
                const SwFrm* pAnchorFrm = 0L;
                if ( pObj->ISA(SwVirtFlyDrawObj) )
                {
                    const SwFlyFrm* pFlyFrm =
                            static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();
                    if ( pFlyFrm )
                    {
                        pAnchorFrm = pFlyFrm->GetAnchorFrm();
                    }
                }
                else
                {
                    SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
                    if ( pDrawContact )
                    {
                        pAnchorFrm = pDrawContact->GetAnchorFrm( pObj );
                    }
                }
                if ( pAnchorFrm )
                {
                    if ( i )
                    {
                        bIsGroupAllowed =
                            ( pAnchorFrm->FindFooterOrHeader() == pHeaderFooterFrm );
                    }
                    else
                    {
                        pHeaderFooterFrm = pAnchorFrm->FindFooterOrHeader();
                    }
                }
            }

        }
    }

    return bIsGroupAllowed;
}

/*************************************************************************
|*
|*  SwFEShell::GroupSelection()
|*
|*  Beschreibung        Die Gruppe bekommt den Anker und das Contactobjekt
|*                      des ersten in der Selektion
|*  Ersterstellung      MA 30. Jan. 95
|*  Letzte Aenderung    MA 23. Apr. 95
|*
*************************************************************************/

void SwFEShell::GroupSelection()
{
    if ( IsGroupAllowed() )
    {
        StartAllAction();
        StartUndo( UNDO_START );

        GetDoc()->GroupSelection( *Imp()->GetDrawView() );
        // OD 2004-04-01 #i26791# - no longer needed
        //ChgAnchor( 0, TRUE );

        EndUndo( UNDO_END );
        EndAllAction();
    }
}

/*************************************************************************
|*
|*  SwFEShell::UnGroupSelection()
|*
|*  Beschreibung        Die Einzelobjekte bekommen eine Kopie vom Anker und
|*                      Contactobjekt der Gruppe.
|*  Ersterstellung      MA 30. Jan. 95
|*  Letzte Aenderung    MA 01. Feb. 95
|*
*************************************************************************/

void SwFEShell::UnGroupSelection()
{
    if ( IsGroupSelected() )
    {
        StartAllAction();
        StartUndo( UNDO_START );

        GetDoc()->UnGroupSelection( *Imp()->GetDrawView() );
        // --> OD 2004-06-15 #i30010# - change of anchor no longer needed
        //ChgAnchor( 0, TRUE );

        EndUndo( UNDO_END );
        EndAllAction();
    }
}

/*************************************************************************
|*
|*  SwFEShell::MirrorSelection()
|*
|*  Ersterstellung      MA 06. Aug. 95
|*  Letzte Aenderung    MA 06. Aug. 95
|*
*************************************************************************/

void SwFEShell::MirrorSelection( BOOL bHorizontal )
{
    SdrView *pView = Imp()->GetDrawView();
    if ( IsObjSelected() && pView->IsMirrorAllowed() )
    {
        if ( bHorizontal )
            pView->MirrorAllMarkedHorizontal();
        else
            pView->MirrorAllMarkedVertical();
    }
}

// springe zum benannten Rahmen (Grafik/OLE)

BOOL SwFEShell::GotoFly( const String& rName, FlyCntType eType, BOOL bSelFrm )
{
    BOOL bRet = FALSE;
static BYTE __READONLY_DATA aChkArr[ 4 ] = {
             /* FLYCNTTYPE_ALL */   0,
             /* FLYCNTTYPE_FRM */   ND_TEXTNODE,
             /* FLYCNTTYPE_GRF */   ND_GRFNODE,
             /* FLYCNTTYPE_OLE */   ND_OLENODE
            };

    const SwFlyFrmFmt* pFlyFmt = pDoc->FindFlyByName( rName, aChkArr[ eType]);
    if( pFlyFmt )
    {
        SET_CURR_SHELL( this );

        SwClientIter aIter( *(SwModify*)pFlyFmt );
        SwFlyFrm* pFrm = (SwFlyFrm*)aIter.First( TYPE( SwFlyFrm ));
        if( pFrm )
        {
            ASSERT( pFrm->IsFlyFrm(), "Wrong FrmType" );
            if( bSelFrm )
            {
                SelectObj( pFrm->Frm().Pos(), 0, ((SwFlyFrm*)pFrm)->GetVirtDrawObj() );
                if( !ActionPend() )
                    MakeVisible( pFrm->Frm() );
            }
            else
            {
                // --> OD 2004-06-11 #i28701# - no format here
//                pFrm->GetAnchorFrm()->Calc();
                SwCntntFrm *pCFrm = pFrm->ContainsCntnt();
                if ( pCFrm )
                {
                    SwCntntNode *pCNode = pCFrm->GetNode();
                    ClearMark();
                    SwPaM* pCrsr = GetCrsr();

                    pCrsr->GetPoint()->nNode = *pCNode;
                    pCrsr->GetPoint()->nContent.Assign( pCNode, 0 );

                    SwRect& rChrRect = (SwRect&)GetCharRect();
                    rChrRect = pFrm->Prt();
                    rChrRect.Pos() += pFrm->Frm().Pos();
                    GetCrsrDocPos() = rChrRect.Pos();
                }
            }
            bRet = TRUE;
        }
    }
    return bRet;
}

USHORT SwFEShell::GetFlyCount(FlyCntType eType ) const
{
    return GetDoc()->GetFlyCount(eType);
}


const SwFrmFmt*  SwFEShell::GetFlyNum(USHORT nIdx, FlyCntType eType ) const
{
    return GetDoc()->GetFlyNum(nIdx, eType );
}

// zeige das akt. selektierte "Object" an
void SwFEShell::MakeSelVisible()
{
    if( Imp()->HasDrawView() &&
        Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )
    {
        MakeVisible( Imp()->GetDrawView()->GetAllMarkedRect() );
    }
    else
        SwCrsrShell::MakeSelVisible();
}


//Welcher Schutz ist am selektierten Objekt gesetzt?
BYTE SwFEShell::IsSelObjProtected( FlyProtectType eType ) const
{
    int nChk = 0;
    BOOL bParent = eType & FLYPROTECT_PARENT;
    if( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for( ULONG i = rMrkList.GetMarkCount(); i; )
        {
            SdrObject *pObj = rMrkList.GetMark( --i )->GetObj();
            if( !bParent )
            {
                nChk |= ( pObj->IsMoveProtect() ? FLYPROTECT_POS : 0 ) |
                        ( pObj->IsResizeProtect()? FLYPROTECT_SIZE : 0 );

                if( FLYPROTECT_CONTENT & eType && pObj->ISA(SwVirtFlyDrawObj) )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    if ( pFly->GetFmt()->GetProtect().IsCntntProtected() )
                        nChk |= FLYPROTECT_CONTENT;

                    if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                    {
                        SwOLENode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode();
                        if ( pNd )
                        {
                            SvInPlaceObjectRef aRef = pNd->GetOLEObj().GetOleRef();
                            if ( aRef.Is() &&
                                 SVOBJ_MISCSTATUS_NOTRESIZEABLE & aRef->GetMiscStatus() )
                            {
                                nChk |= FLYPROTECT_SIZE;
                                nChk |= FLYPROTECT_FIXED;
                            }
                        }
                    }
                }
                nChk &= eType;
                if( nChk == eType )
                    return eType;
            }
            const SwFrm* pAnch;
            if( pObj->ISA(SwVirtFlyDrawObj) )
                pAnch = ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm()->GetAnchorFrm();
            else
            {
                SwDrawContact* pTmp = (SwDrawContact*)GetUserCall(pObj);
                pAnch = pTmp ? pTmp->GetAnchorFrm( pObj ) : NULL;
            }
            if( pAnch && pAnch->IsProtected() )
                return eType;
        }
    }
    return nChk;
}

BOOL SwFEShell::GetObjAttr( SfxItemSet &rSet ) const
{
    if ( !IsObjSelected() )
        return FALSE;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        if ( i )
            rSet.MergeValues( pContact->GetFmt()->GetAttrSet() );
        else
            rSet.Put( pContact->GetFmt()->GetAttrSet() );
    }
    return TRUE;
}

BOOL SwFEShell::SetObjAttr( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );

    if ( !rSet.Count() )
    {   ASSERT( !this, "SetObjAttr, empty set." );
        return FALSE;
    }

    StartAllAction();
    StartUndo( UNDO_INSATTR );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        GetDoc()->SetAttr( rSet, *pContact->GetFmt() );
    }

    EndUndo( UNDO_INSATTR );
    EndAllActionAndCall();
    GetDoc()->SetModified();
    return TRUE;
}

BOOL SwFEShell::IsAlignPossible() const
{
    USHORT nCnt;
    if ( 0 < (nCnt = IsObjSelected()) )
    {
        BOOL bRet = TRUE;
        if ( nCnt == 1 )
        {
            SdrObject *pO = Imp()->GetDrawView()->GetMarkedObjectList().GetMark(0)->GetObj();
            SwDrawContact *pC = (SwDrawContact*)GetUserCall(pO);
            //only as character bound drawings can be aligned
            bRet = pC->GetFmt()->GetAnchor().GetAnchorId() == FLY_IN_CNTNT;
        }
        if ( bRet )
            return Imp()->GetDrawView()->IsAlignPossible();
    }
    return FALSE;
}


//Temporaerer Fix bis SS von JOE da ist
void SwFEShell::CheckUnboundObjects()
{
    SET_CURR_SHELL( this );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
        if ( !GetUserCall(pObj) )
        {
            const Rectangle &rBound = pObj->GetSnapRect();
            const Point aPt( rBound.TopLeft() );
            const SwFrm *pPage = GetLayout()->Lower();
            const SwFrm *pLast = pPage;
            while ( pPage && !pPage->Frm().IsInside( aPt ) )
            {
                if ( aPt.Y() > pPage->Frm().Bottom() )
                    pLast = pPage;
                pPage = pPage->GetNext();
            }
            if ( !pPage )
                pPage = pLast;
            ASSERT( pPage, "Page not found." );

            //Fremde Identifier sollen in den Default laufen.
            //Ueberschneidungen sind moeglich!!
            UINT16 nIdent =
                    Imp()->GetDrawView()->GetCurrentObjInventor() == SdrInventor ?
                            Imp()->GetDrawView()->GetCurrentObjIdentifier() : 0xFFFF;

            SwFmtAnchor aAnch;
            const SwFrm *pAnch = 0;
            {
            pAnch = ::FindAnchor( pPage, aPt, TRUE );
            SwPosition aPos( *((SwCntntFrm*)pAnch)->GetNode() );
            aAnch.SetType( FLY_AT_CNTNT );
            aAnch.SetAnchor( &aPos );
            ((SwRect&)GetCharRect()).Pos() = aPt;
            }

            //Erst hier die Action, damit das GetCharRect aktuelle Werte liefert.
            StartAllAction();

            SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_SURROUND, RES_ANCHOR, 0 );
            aSet.Put( aAnch );

            Point aRelNullPt;

            if( OBJ_CAPTION == nIdent )
                aRelNullPt = ((SdrCaptionObj*)pObj)->GetTailPos();
            else
                aRelNullPt = rBound.TopLeft();

            aSet.Put( aAnch );
            aSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
            SwFrmFmt* pFmt = GetDoc()->MakeLayoutFmt(
                                            RND_DRAW_OBJECT, 0, &aSet );

            SwDrawContact *pContact = new SwDrawContact(
                                            (SwDrawFrmFmt*)pFmt, pObj );

            pContact->ConnectToLayout();

            EndAllAction();
        }
    }
}

void SwFEShell::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    GetDoc()->SetCalcFieldValueHdl(pOutliner);
}



int SwFEShell::Chainable( SwRect &rRect, const SwFrmFmt &rSource,
                            const Point &rPt ) const
{
    rRect.Clear();

    //Die Source darf noch keinen Follow haben.
    const SwFmtChain &rChain = rSource.GetChain();
    if ( rChain.GetNext() )
        return SW_CHAIN_SOURCE_CHAINED;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();
        const USHORT nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        if( pDView->PickObj( rPt, pObj, pPView, SDRSEARCH_PICKMARKABLE ) &&
            pObj->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            rRect = pFly->Frm();

            //Ziel darf natuerlich nicht gleich Source sein und es
            //darf keine geschlossene Kette entstehen.
            SwFrmFmt *pFmt = pFly->GetFmt();
            return GetDoc()->Chainable(rSource, *pFmt);
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return SW_CHAIN_NOT_FOUND;
}
/* -----------------------------09.08.2002 07:40------------------------------

 ---------------------------------------------------------------------------*/
int SwFEShell::Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    return GetDoc()->Chain(rSource, rDest);
}

int SwFEShell::Chain( SwFrmFmt &rSource, const Point &rPt )
{
    SwRect aDummy;
    int nErr = Chainable( aDummy, rSource, rPt );
    if ( !nErr )
    {
        StartAllAction();
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();
        const USHORT nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        pDView->PickObj( rPt, pObj, pPView, SDRSEARCH_PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();

        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)pFly->GetFmt();
        GetDoc()->Chain(rSource, *pFmt);
        EndAllAction();
        SetChainMarker();
    }
    return nErr;
}

void SwFEShell::Unchain( SwFrmFmt &rFmt )
{
    StartAllAction();
    GetDoc()->Unchain(rFmt);
    EndAllAction();
}


void SwFEShell::HideChainMarker()
{
    if ( pChainFrom )
        pChainFrom->Hide();
    if ( pChainTo )
        pChainTo->Hide();
}

void SwFEShell::SetChainMarker()
{
    FASTBOOL bDelFrom = TRUE,
             bDelTo   = TRUE;
    if ( IsFrmSelected() )
    {
        SwFlyFrm *pFly = FindFlyFrm();

        XPolygon aPoly(3);
        if ( pFly->GetPrevLink() )
        {
            bDelFrom = FALSE;
            const SwFrm *pPre = pFly->GetPrevLink();
            aPoly[0] = Point( pPre->Frm().Right(), pPre->Frm().Bottom());
            aPoly[1] = pFly->Frm().Pos();
            if ( !pChainFrom )
                pChainFrom = new SdrViewUserMarker( GetDrawView() );
            pChainFrom->SetPolyLine( TRUE );
            pChainFrom->SetXPolygon( aPoly );
            pChainFrom->Show();
        }
        if ( pFly->GetNextLink() )
        {
            bDelTo = FALSE;
            const SwFlyFrm *pNxt = pFly->GetNextLink();
            aPoly[0] = Point( pFly->Frm().Right(), pFly->Frm().Bottom());
            aPoly[1] = pNxt->Frm().Pos();
            if ( !pChainTo )
                pChainTo = new SdrViewUserMarker( GetDrawView() );
            pChainTo->SetXPolygon( aPoly );
            pChainTo->SetPolyLine( TRUE );
            pChainTo->Show();
        }
    }
    if ( bDelFrom )
        delete pChainFrom, pChainFrom = 0;
    if ( bDelTo )
        delete pChainTo,   pChainTo = 0;
}

long SwFEShell::GetSectionWidth( SwFmt& rFmt ) const
{
    SwFrm *pFrm = GetCurrFrm();
    // Steht der Cursor z.Z. in einem SectionFrm?
    if( pFrm && pFrm->IsInSct() )
    {
        SwSectionFrm* pSect = pFrm->FindSctFrm();
        do
        {
            // Ist es der Gewuenschte?
            if( pSect->GetRegisteredIn() == &rFmt )
                return pSect->Frm().Width();
            // fuer geschachtelte Bereiche
            pSect = pSect->GetUpper()->FindSctFrm();
        }
        while( pSect );
    }
    SwClientIter aIter( rFmt );
    SwClient *pLast = aIter.GoStart();
    while ( pLast )
    {
        if ( pLast->IsA( TYPE(SwFrm) ) )
        {
            SwSectionFrm* pSct = (SwSectionFrm*)pLast;
            if( !pSct->IsFollow() )
                return pSct->Frm().Width();
        }
        pLast = aIter++;
    }
    return 0;
}
/* -----------------------------2002/06/24 15:07------------------------------

 ---------------------------------------------------------------------------*/
void SwFEShell::CreateDefaultShape(UINT16 eSdrObjectKind, const Rectangle& rRect,
                USHORT nSlotId)
{
    SdrView* pDrawView = GetDrawView();
    SdrModel* pDrawModel = pDrawView->GetModel();
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        SdrInventor, eSdrObjectKind,
        0L, pDrawModel);

    if(pObj)
    {
        Rectangle aRect(rRect);
        if(OBJ_CARC == eSdrObjectKind || OBJ_CCUT == eSdrObjectKind)
        {
            // force quadratic
            if(aRect.GetWidth() > aRect.GetHeight())
            {
                aRect = Rectangle(
                    Point(aRect.Left() + ((aRect.GetWidth() - aRect.GetHeight()) / 2), aRect.Top()),
                    Size(aRect.GetHeight(), aRect.GetHeight()));
            }
            else
            {
                aRect = Rectangle(
                    Point(aRect.Left(), aRect.Top() + ((aRect.GetHeight() - aRect.GetWidth()) / 2)),
                    Size(aRect.GetWidth(), aRect.GetWidth()));
            }
        }
        pObj->SetLogicRect(aRect);

        if(pObj->ISA(SdrCircObj))
        {
            SfxItemSet aAttr(pDrawModel->GetItemPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));
            pObj->SetMergedItemSet(aAttr);
        }
        else if(pObj->ISA(SdrPathObj))
        {
            XPolyPolygon aPoly;

            switch(eSdrObjectKind)
            {
                case OBJ_PATHLINE:
                {
                    XPolygon aInnerPoly;
                    aInnerPoly[0] = aRect.BottomLeft();
                    aInnerPoly[1] = aRect.BottomCenter();
                    aInnerPoly[2] = aRect.BottomCenter();
                    aInnerPoly[3] = aRect.Center();
                    aInnerPoly[4] = aRect.TopCenter();
                    aInnerPoly[5] = aRect.TopCenter();
                    aInnerPoly[6] = aRect.TopRight();

                    aInnerPoly.SetFlags(1, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(2, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(3, XPOLY_SYMMTR);
                    aInnerPoly.SetFlags(4, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(5, XPOLY_CONTROL);

                    aPoly.Insert(aInnerPoly);
                }
                break;
                case OBJ_FREELINE:
                {
                    XPolygon aInnerPoly;
                    aInnerPoly[0] = aRect.BottomLeft();
                    aInnerPoly[1] = aRect.TopLeft();
                    aInnerPoly[2] = aRect.TopCenter();
                    aInnerPoly[3] = aRect.Center();
                    aInnerPoly[4] = aRect.BottomCenter();
                    aInnerPoly[5] = aRect.BottomRight();
                    aInnerPoly[6] = aRect.TopRight();

                    aInnerPoly.SetFlags(1, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(2, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(3, XPOLY_SMOOTH);
                    aInnerPoly.SetFlags(4, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(5, XPOLY_CONTROL);

                    aInnerPoly[7] = aRect.BottomRight();

                    aPoly.Insert(aInnerPoly);
                }
                break;
                case OBJ_POLY:
                case OBJ_PLIN:
                {
                    XPolygon aInnerPoly;
                    sal_Int32 nWdt(aRect.GetWidth());
                    sal_Int32 nHgt(aRect.GetHeight());

                    aInnerPoly[0] = aRect.BottomLeft();
                    aInnerPoly[1] = aRect.TopLeft() + Point((nWdt * 30) / 100, (nHgt * 70) / 100);
                    aInnerPoly[2] = aRect.TopLeft() + Point(0, (nHgt * 15) / 100);
                    aInnerPoly[3] = aRect.TopLeft() + Point((nWdt * 65) / 100, 0);
                    aInnerPoly[4] = aRect.TopLeft() + Point(nWdt, (nHgt * 30) / 100);
                    aInnerPoly[5] = aRect.TopLeft() + Point((nWdt * 80) / 100, (nHgt * 50) / 100);
                    aInnerPoly[6] = aRect.TopLeft() + Point((nWdt * 80) / 100, (nHgt * 75) / 100);
                    aInnerPoly[7] = aRect.BottomRight();

                    if(OBJ_PLIN == eSdrObjectKind)
                    {
                        aInnerPoly[8] = aRect.BottomCenter();
                    }

                    aPoly.Insert(aInnerPoly);
                }
                break;
                case OBJ_LINE :
                {
                    aPoly.Insert(XPolygon(2));
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    aPoly[0][0] = Point(aRect.TopLeft().X(), nYMiddle);
                    aPoly[0][1] = Point(aRect.BottomRight().X(), nYMiddle);
                }
                break;
            }

            ((SdrPathObj*)pObj)->SetPathPoly(aPoly);
        }
        else if(pObj->ISA(SdrCaptionObj))
        {
            BOOL bVerticalText = ( SID_DRAW_TEXT_VERTICAL == nSlotId ||
                                            SID_DRAW_CAPTION_VERTICAL == nSlotId );
            ((SdrTextObj*)pObj)->SetVerticalWriting(bVerticalText);
            if(bVerticalText)
            {
                SfxItemSet aSet(pObj->GetMergedItemSet());
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pObj->SetMergedItemSet(aSet);
            }

            ((SdrCaptionObj*)pObj)->SetLogicRect(aRect);
            ((SdrCaptionObj*)pObj)->SetTailPos(
                aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
        }
        else if(pObj->ISA(SdrTextObj))
        {
            SdrTextObj* pText = (SdrTextObj*)pObj;
            pText->SetLogicRect(aRect);

            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotId);
            sal_Bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nSlotId);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrawModel->GetItemPool());
                aSet.Put(SdrTextAutoGrowWidthItem(TRUE));
                aSet.Put(SdrTextAutoGrowHeightItem(FALSE));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrawModel->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);
                aSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                aSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                aSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                aSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( (INT16)GetWin()->PixelToLogic(Size(2,1)).Width()) );
                pObj->SetMergedItemSetAndBroadcast(aSet);
            }
        }
        SdrPageView* pPageView = pDrawView->GetPageViewPvNum(0);
        pDrawView->InsertObject(pObj, *pPageView, pDrawView->IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);
    }
    ImpEndCreate();
}

/** SwFEShell::GetShapeBackgrd

    OD 02.09.2002 for #102450#:
    method determines background color of the page the selected drawing
    object is on and returns this color.
    If no color is found, because no drawing object is selected or ...,
    color COL_BLACK (default color on constructing object of class Color)
    is returned.

    @author OD

    @returns an object of class Color
*/
const Color SwFEShell::GetShapeBackgrd() const
{
    Color aRetColor;

    // check, if a draw view exists
    ASSERT( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackgrd - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        ASSERT( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackgrd - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetObj();
            // check, if selected object is a shape (drawing object)
            ASSERT( !pSdrObj->ISA(SwVirtFlyDrawObj), "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( !pSdrObj->ISA(SwVirtFlyDrawObj) )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrm* pAnchorFrm =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrm( pSdrObj );
                ASSERT( pAnchorFrm, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrm )
                {
                    const SwPageFrm* pPageFrm = pAnchorFrm->FindPageFrm();
                    ASSERT( pPageFrm, "inconsistent modell - no page!");
                    if ( pPageFrm )
                    {
                        aRetColor = pPageFrm->GetDrawBackgrdColor();
                    }
                }
            }
        }
    }

    return aRetColor;
}

/** Is default horizontal text direction for selected drawing object right-to-left

    OD 09.12.2002 #103045#
    Because drawing objects only painted for each page only, the default
    horizontal text direction of a drawing object is given by the corresponding
    page property.

    @author OD

    @returns boolean, indicating, if the horizontal text direction of the
    page, the selected drawing object is on, is right-to-left.
*/
const bool SwFEShell::IsShapeDefaultHoriTextDirR2L() const
{
    bool bRet = false;

    // check, if a draw view exists
    ASSERT( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackgrd - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        ASSERT( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackgrd - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetObj();
            // check, if selected object is a shape (drawing object)
            ASSERT( !pSdrObj->ISA(SwVirtFlyDrawObj), "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( !pSdrObj->ISA(SwVirtFlyDrawObj) )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrm* pAnchorFrm =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrm( pSdrObj );
                ASSERT( pAnchorFrm, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrm )
                {
                    const SwPageFrm* pPageFrm = pAnchorFrm->FindPageFrm();
                    ASSERT( pPageFrm, "inconsistent modell - no page!");
                    if ( pPageFrm )
                    {
                        bRet = pPageFrm->IsRightToLeft() ? true : false;
                    }
                }
            }
        }
    }

    return bRet;
}
/* -----------------20.03.2003 14:35-----------------

 --------------------------------------------------*/
Point SwFEShell::GetRelativePagePosition(const Point& rDocPos)
{
    Point aRet(-1, -1);
    const SwFrm *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->Frm().IsInside( rDocPos ) )
    {
        pPage = pPage->GetNext();
    }
    if(pPage)
    {
        aRet = rDocPos - pPage->Frm().TopLeft();
    }
    return aRet;
}

