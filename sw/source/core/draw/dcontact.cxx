/*************************************************************************
 *
 *  $RCSfile: dcontact.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-30 17:57:05 $
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

#include "hintids.hxx"

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _VIEWIMP_HXX //autogen
#include <viewimp.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>  // Notify_Background
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

TYPEINIT1( SwContact, SwClient )
TYPEINIT1( SwFlyDrawContact, SwContact )
TYPEINIT1( SwDrawContact, SwContact )


//Der Umgekehrte Weg: Sucht das Format zum angegebenen Objekt.
//Wenn das Object ein SwVirtFlyDrawObj ist so wird das Format von
//selbigem besorgt.
//Anderfalls ist es eben ein einfaches Zeichenobjekt. Diese hat einen
//UserCall und der ist Client vom gesuchten Format.

SwFrmFmt *FindFrmFmt( SdrObject *pObj )
{
    if ( pObj->IsWriterFlyFrame() )
        return ((SwVirtFlyDrawObj*)pObj)->GetFmt();
    else
    {
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall( pObj );
        return pContact ? pContact->GetFmt() : NULL;
    }
}

/*****************************************************************************
 *
 * GetBoundRect liefert das BoundRect _inklusive_ Abstand des Objekts.
 *
 *****************************************************************************/

SwRect GetBoundRect( const SdrObject* pObj )
{
    SwRect aRet( pObj->GetBoundRect() );
    const SwFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
    const SvxULSpaceItem &rUL = pFmt->GetULSpace();
    const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
    aRet.Top ( Max( aRet.Top() - long(rUL.GetUpper()), 0L ));
    aRet.Left( Max( aRet.Left()- long(rLR.GetLeft()),  0L ));
    aRet.SSize().Height() += rUL.GetLower();
    aRet.SSize().Width()  += rLR.GetRight();
    return aRet;
}

//Liefert den UserCall ggf. vom Gruppenobjekt
SdrObjUserCall* GetUserCall( const SdrObject* pObj )
{
    SdrObject *pTmp;
    while ( !pObj->GetUserCall() && 0 != (pTmp = pObj->GetUpGroup()) )
        pObj = pTmp;
    return pObj->GetUserCall();
}

// liefert TRUE falls das SrdObject ein Marquee-Object (Lauftext) ist
FASTBOOL IsMarqueeTextObj( const SdrObject& rObj )
{
    SdrTextAniKind eTKind;
    return SdrInventor == rObj.GetObjInventor() &&
        OBJ_TEXT == rObj.GetObjIdentifier() &&
        ( SDRTEXTANI_SCROLL == ( eTKind = ((SdrTextObj&)rObj).GetTextAniKind())
         || SDRTEXTANI_ALTERNATE == eTKind || SDRTEXTANI_SLIDE == eTKind );
}

/*************************************************************************
|*
|*  SwContact, Ctor und Dtor
|*
|*  Ersterstellung      AMA 27.Sep.96 18:13
|*  Letzte Aenderung    AMA 27.Sep.96
|*
|*************************************************************************/

SwContact::SwContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj ) :
    SwClient( pToRegisterIn ),
    pMasterObj( pObj )
{
    pObj->SetUserCall( this );
}

SwContact::~SwContact()
{
    if ( pMasterObj )
    {
        pMasterObj->SetUserCall( 0 );   //Soll mir nicht in den Ruecken fallen.
        if ( pMasterObj->GetPage() )    //Der SdrPage die Verantwortung entziehen.
            pMasterObj->GetPage()->RemoveObject( pMasterObj->GetOrdNum() );
        delete pMasterObj;
    }
}

/*************************************************************************
|*
|*  SwFlyDrawContact, Ctor und Dtor
|*
|*  Ersterstellung      OK 23.11.94 18:13
|*  Letzte Aenderung    MA 06. Apr. 95
|*
|*************************************************************************/

SwFlyDrawContact::SwFlyDrawContact( SwFlyFrmFmt *pToRegisterIn, SdrModel *pMod ) :
    SwContact( pToRegisterIn )
{
    SetMaster( new SwFlyDrawObj() );
    GetMaster()->SetOrdNum( 0xFFFFFFFE );
    GetMaster()->SetUserCall( this );
}

/*************************************************************************
|*
|*  SwFlyDrawContact::CreateNewRef()
|*
|*  Ersterstellung      MA 14. Dec. 94
|*  Letzte Aenderung    MA 24. Apr. 95
|*
|*************************************************************************/

SwVirtFlyDrawObj *SwFlyDrawContact::CreateNewRef( SwFlyFrm *pFly )
{
    SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *GetMaster(), pFly );
    pDrawObj->SetModel( GetMaster()->GetModel() );
    pDrawObj->SetUserCall( this );

    //Der Reader erzeugt die Master und setzt diese, um die Z-Order zu
    //transportieren, in die Page ein. Beim erzeugen der ersten Referenz werden
    //die Master aus der Liste entfernt und fuehren von da an ein
    //Schattendasein.
    SdrPage *pPg;
    if ( 0 != ( pPg = GetMaster()->GetPage() ) )
    {
        const UINT32 nOrdNum = GetMaster()->GetOrdNum();
        pPg->ReplaceObject( pDrawObj, nOrdNum );
    }
    return pDrawObj;
}

/*************************************************************************
|*
|*  SwFlyDrawContact::Modify()
|*
|*  Ersterstellung      OK 08.11.94 10:21
|*  Letzte Aenderung    MA 06. Dec. 94
|*
|*************************************************************************/

void SwFlyDrawContact::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
}

/*************************************************************************
|*
|*  SwDrawContact, Ctor+Dtor
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 22. Jul. 98
|*
|*************************************************************************/
FASTBOOL lcl_CheckControlLayer( const SdrObject *pObj )
{
    if ( FmFormInventor == pObj->GetObjInventor() )
        return TRUE;
    if ( pObj->ISA( SdrObjGroup ) )
    {
        const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( USHORT i = 0; i < pLst->GetObjCount(); ++i )
            if ( ::lcl_CheckControlLayer( pLst->GetObj( i ) ) )
                return TRUE;
    }
    return FALSE;
}

SwDrawContact::SwDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj ) :
    SwContact( pToRegisterIn, pObj ),
    pAnchor( 0 ),
    pPage( 0 )
{
    //Controls muessen immer im Control-Layer liegen. Das gilt auch fuer
    //Gruppenobjekte, wenn diese Controls enthalten.
    if ( lcl_CheckControlLayer( pObj ) )
        pObj->SetLayer( pToRegisterIn->GetDoc()->GetControlsId() );
}


SwDrawContact::~SwDrawContact()
{
    DisconnectFromLayout();
}

/*************************************************************************
|*
|*  SwDrawContact::Changed
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 29. May. 96
|*
|*************************************************************************/

void lcl_Notify( SwDrawContact* pThis, const Rectangle* pOldBoundRect )
{
    SwFrm *pAnch = pThis->GetAnchor();
    if( pAnch )
    {
        SwPageFrm *pPage = pAnch->FindPageFrm();
        ASSERT( GetUserCall(pThis->GetMaster()) == pThis, "Wrong Master" );
        if( pOldBoundRect && pPage )
        {
            SwRect aOldRect( *pOldBoundRect );
            if( aOldRect.HasArea() )
                Notify_Background( pThis->GetMaster(),pPage, aOldRect,
                                    PREP_FLY_LEAVE,TRUE);
        }
        SwRect aRect( pThis->GetMaster()->GetBoundRect() );
        if( aRect.HasArea() )
        {
            SwPageFrm *pPg = pThis->FindPage( aRect );
            if( pPg )
                Notify_Background( pThis->GetMaster(), pPg, aRect,
                                    PREP_FLY_ARRIVE, TRUE );
        }
        ClrContourCache( pThis->GetMaster() );
    }
}


void SwDrawContact::Changed(const SdrObject& rObj, SdrUserCallType eType,
                                    const Rectangle& rOldBoundRect)
{
    //Action aufsetzen, aber nicht wenn gerade irgendwo eine Action laeuft.
    ViewShell *pSh = 0, *pOrg;
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetRootFrm() && pDoc->GetRootFrm()->IsCallbackActionEnabled() )
    {
        pDoc->GetEditShell( &pOrg );
        pSh = pOrg;
        if ( pSh )
            do
            {   if ( pSh->Imp()->IsAction() || pSh->Imp()->IsIdleAction() )
                    pSh = 0;
                else
                    pSh = (ViewShell*)pSh->GetNext();

            } while ( pSh && pSh != pOrg );

        if ( pSh )
            pDoc->GetRootFrm()->StartAllAction();
    }

    SdrObjUserCall::Changed( rObj, eType, rOldBoundRect );
    _Changed( rObj, eType, &rOldBoundRect );    //Achtung, ggf. Suizid!

    if ( pSh )
        pDoc->GetRootFrm()->EndAllAction();
}

void SwDrawContact::_Changed(const SdrObject& rObj, SdrUserCallType eType,
                             const Rectangle* pOldBoundRect)
{
    BOOL bInCntnt = FLY_IN_CNTNT == GetFmt()->GetAnchor().GetAnchorId();
    BOOL bNotify = !bInCntnt &&
        ( SURROUND_THROUGHT != GetFmt()->GetSurround().GetSurround() );
    switch( eType )
    {
        case SDRUSERCALL_DELETE:
            {
                if( bNotify )
                    lcl_Notify( this, pOldBoundRect );
                DisconnectFromLayout( FALSE );
                SetMaster( NULL );
//              GetFmt()->GetDoc()->DelLayoutFmt( GetFmt() );
                delete this;
                break;
            }
        case SDRUSERCALL_INSERTED:
            {
                ConnectToLayout();
                if( bNotify )
                    lcl_Notify( this, pOldBoundRect );
                break;
            }
        case SDRUSERCALL_REMOVED:
            {
                if( bNotify )
                    lcl_Notify( this, pOldBoundRect );
                DisconnectFromLayout( FALSE );
                break;
            }
        case SDRUSERCALL_MOVEONLY:
        case SDRUSERCALL_RESIZE:
        case SDRUSERCALL_CHILD_MOVEONLY :
        case SDRUSERCALL_CHILD_RESIZE :
        case SDRUSERCALL_CHILD_CHGATTR :
        case SDRUSERCALL_CHILD_DELETE :
        case SDRUSERCALL_CHILD_COPY :
        case SDRUSERCALL_CHILD_INSERTED :
        case SDRUSERCALL_CHILD_REMOVED :
            if( bInCntnt )
            {
                SwFrm *pAnch = GetAnchor();
                if( !pAnch )
                {
                    ConnectToLayout();
                    pAnch = GetAnchor();
                }
                if( pAnch && !((SwTxtFrm*)pAnch)->IsLocked() )
                {
                    SwFrmFmt *pFmt = GetFmt();
                    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
                    Point aRelPos = rObj.GetRelativePos();
                    if( rVert.GetPos() != aRelPos.Y() )
                    {
                        SwFmtVertOrient aVert( rVert );
                        aVert.SetVertOrient( VERT_NONE );
                        aVert.SetPos( aRelPos.Y() );
                        pFmt->SetAttr( aVert );
                    }
                    ((SwTxtFrm*)pAnch)->Prepare();
                }
            }
            else if( bNotify )
                lcl_Notify( this, pOldBoundRect );
            break;
        case SDRUSERCALL_CHGATTR:
            if( bNotify )
                lcl_Notify( this, pOldBoundRect );
            break;
    }
}

/*************************************************************************
|*
|*  SwDrawContact::Modify()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 03. Dec. 95
|*
|*************************************************************************/

void SwDrawContact::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    //Es kommen immer Sets herein.
    //MA 03. Dec. 95: Falsch es kommen nicht immer Sets herein
    //(siehe SwRootFrm::AssertPageFlys()
    USHORT nWhich = pNew ? pNew->Which() : 0;
    SwFmtAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich )
    {
        if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_ANCHOR, FALSE, (const SfxPoolItem**)&pAnch ))
        ;       // Beim GetItemState wird der AnkerPointer gesetzt !
        else if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
                                 GetItemState( RES_VERT_ORIENT, FALSE ))
        {
            SwFrm *pFrm = GetAnchor();
            if( !pFrm )
            {
                ConnectToLayout();
                pFrm = GetAnchor();
            }
            if( pFrm && pFrm->IsTxtFrm() )
                ((SwTxtFrm*)pFrm)->Prepare();
        }
    }
    else if( RES_ANCHOR == nWhich )
        pAnch = (SwFmtAnchor*)pNew;

    if ( pAnch )
    {
        // JP 10.04.95: nicht auf ein Reset Anchor reagieren !!!!!
        if( SFX_ITEM_SET == ((SwFrmFmt*)pRegisteredIn)->GetAttrSet().
            GetItemState( RES_ANCHOR, FALSE ) )
        {
            SwFrm *pOldAnch = GetAnchor();
            SwPageFrm *pPg = NULL;
            SwRect aOldRect;
            if( pOldAnch )
            {
                pPg = pOldAnch->FindPageFrm();
                aOldRect = SwRect( GetMaster()->GetBoundRect() );
            }
            ConnectToLayout( pAnch );
            if( pPg && aOldRect.HasArea() )
                Notify_Background(GetMaster(),pPg,aOldRect,PREP_FLY_LEAVE,TRUE);
            lcl_Notify( this, NULL );
        }
        else
            DisconnectFromLayout();
    }
    else if( RES_SURROUND == nWhich || RES_UL_SPACE == nWhich ||
             RES_LR_SPACE == nWhich ||
             ( RES_ATTRSET_CHG == nWhich &&
               ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_SURROUND, FALSE ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_LR_SPACE, FALSE ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_UL_SPACE, FALSE ) ) ) )
        lcl_Notify( this, NULL );
}


/*************************************************************************
|*
|*  SwDrawContact::DisconnectFromLayout()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/

void SwDrawContact::DisconnectFromLayout( BOOL bRemoveFromPage )
{
    if ( pAnchor )
        pAnchor->RemoveDrawObj( this );

    if ( bRemoveFromPage && GetMaster() && GetMaster()->IsInserted() )
        ((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
                                    RemoveObject( GetMaster()->GetOrdNum() );
}

/*************************************************************************
|*
|*  SwDrawContact::ConnectToLayout()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/

void SwDrawContact::ConnectToLayout( const SwFmtAnchor *pAnch )
{
    SwFrmFmt *pFmt = (SwFrmFmt*)pRegisteredIn;

    SwRootFrm *pRoot = pFmt->GetDoc()->GetRootFrm();
    if ( !pRoot )
        return;

    if ( pAnchor )
        pAnchor->RemoveDrawObj( this );

    if ( !pAnch )
        pAnch = &pFmt->GetAnchor();

    BOOL bSetAnchorPos = TRUE;

    switch ( pAnch->GetAnchorId() )
    {
        case FLY_PAGE:
            {
                USHORT nPgNum = pAnch->GetPageNum();
                SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();

                for ( USHORT i = 1; i < nPgNum && pPage;
                            ++i, pPage = (SwPageFrm*)pPage->GetNext() )
                    /* do nothing */;

                if ( pPage )
                {
                    bSetAnchorPos = FALSE;
                    pPage->SwFrm::AppendDrawObj( this );
                }
                else
                    //Sieht doof aus, ist aber erlaubt (vlg. SwFEShell::SetPageObjsNewPage)
                    pRoot->SetAssertFlyPages();
            }
            break;

        case FLY_AUTO_CNTNT:
        case FLY_AT_CNTNT:
            {
                //Wenn die Zeichenobjekte in mehrfacher Darstellung vorkommen
                //(z.B. Kopf-/Fusszeilen) wird es komplizierter, dann bitte aus
                //SwFlyAtCntFrm::Modify() abkupfern.
                if ( pAnch->GetCntntAnchor() ) //#28154#
                {
                    SwCntntNode *pNode = pFmt->GetDoc()->
                            GetNodes()[pAnch->GetCntntAnchor()->nNode]->GetCntntNode();
                    SwCntntFrm *pCntnt = pNode->GetFrm( 0, 0, FALSE );
                    if ( pCntnt )
                    {
                        //Kann sein, dass der Anker noch nicht existiert
                        pCntnt->AppendDrawObj( this );
                        bSetAnchorPos = FALSE;
                    }
                }
            }
            break;
        case FLY_AT_FLY:
            {
                if( pAnch->GetCntntAnchor() ) // LAYER_IMPL
                {
                    SwFrm *pAnchor = 0;
                    //Erst einmal ueber den Inhalt suchen, weil konstant schnell. Kann
                    //Bei verketteten Rahmen aber auch schief gehen, weil dann evtl.
                    //niemals ein Frame zu dem Inhalt existiert. Dann muss leider noch
                    //die Suche vom StartNode zum FrameFormat sein.
                    SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
                    SwCntntNode *pCNd = pFmt->GetDoc()->GetNodes().GoNext( &aIdx );
                    if ( pCNd && 0 != (pAnchor = pCNd->GetFrm( 0, 0, FALSE ) ) )
                        pAnchor = pAnchor->FindFlyFrm();
                    else
                    {
                        const SwNodeIndex &rIdx = pAnch->GetCntntAnchor()->nNode;
                        SwSpzFrmFmts& rFmts = *pFmt->GetDoc()->GetSpzFrmFmts();
                        for( USHORT i = 0; i < rFmts.Count(); ++i )
                        {
                            SwFrmFmt *pFmt = rFmts[i];
                            SwFlyFrmFmt* pFlyFmt;
                            if( 0 != (pFlyFmt = PTR_CAST( SwFlyFrmFmt, pFmt )) &&
                                pFlyFmt->GetCntnt().GetCntntIdx() && //#57390#, Reader
                                rIdx == *pFlyFmt->GetCntnt().GetCntntIdx() )
                            {
                                pAnchor = pFlyFmt->GetFrm( 0, FALSE );
                                break;
                            }
                        }
                    }
                    if ( pAnchor )  //Kann sein, dass der Anker noch nicht existiert
                    {
                        pAnchor->FindFlyFrm()->AppendDrawObj( this );
                        bSetAnchorPos = FALSE;
                    }
                }
            }
            break;
        case FLY_IN_CNTNT:
            {
                ClrContourCache( GetMaster() );
                SwCntntNode *pNode = GetFmt()->GetDoc()->
                        GetNodes()[pAnch->GetCntntAnchor()->nNode]->GetCntntNode();
                SwCntntFrm *pCntnt = pNode->GetFrm( 0, 0, FALSE );
                if ( pCntnt )
                {
                    //Kann sein, dass der Anker noch nicht existiert
                    pCntnt->AppendDrawObj( this );
                    pCntnt->InvalidatePrt();
                }
                bSetAnchorPos = FALSE;
            }
            break;
#ifndef PRODUCT
        default:    ASSERT( FALSE, "Unknown Anchor." );
#endif
    }
    if ( GetAnchor() )
    {
        if( bSetAnchorPos )
            GetMaster()->SetAnchorPos( GetAnchor()->Frm().Pos() );

        //verankerte Objekte gehoeren immer auch in die Page,
        if ( !GetMaster()->IsInserted() )
            pFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
    }
}

/*************************************************************************
|*
|*  SwDrawContact::FindPage(), ChkPage()
|*
|*  Ersterstellung      MA 21. Mar. 95
|*  Letzte Aenderung    MA 19. Jul. 96
|*
|*************************************************************************/

SwPageFrm *SwDrawContact::FindPage( const SwRect &rRect )
{
    SwPageFrm *pPg = pPage;
    if ( !pPg && pAnchor )
        pPg = pAnchor->FindPageFrm();
    if ( pPg )
        pPg = (SwPageFrm*)::FindPage( rRect, pPg );
    return pPg;
}

void SwDrawContact::ChkPage()
{
    SwPageFrm *pPg = pAnchor && pAnchor->IsPageFrm() ?
        pPage : FindPage( GetMaster()->GetBoundRect() );
    if ( pPage != pPg )
    {
        if ( pPage )
            pPage->SwPageFrm::RemoveDrawObj( this );
        pPg->SwPageFrm::AppendDrawObj( this );
        ChgPage( pPg );
    }
}

/*************************************************************************
|*
|*  SwDrawContact::ChangeMasterObject()
|*
|*  Ersterstellung      MA 07. Aug. 95
|*  Letzte Aenderung    MA 20. Apr. 99
|*
|*************************************************************************/

void SwDrawContact::ChangeMasterObject( SdrObject *pNewMaster )
{
    SwFrm *pAnch = GetAnchor();
    DisconnectFromLayout( FALSE );
    GetMaster()->SetUserCall( 0 );
    SetMaster( pNewMaster );
    GetMaster()->SetUserCall( this );
    GetMaster()->NbcSetRelativePos( GetMaster()->GetSnapRect().TopLeft() -
                                   pAnch->Frm().Pos() );
    GetMaster()->NbcSetAnchorPos( pAnch->Frm().Pos() );

    //Hier wird der neue Master ggf. in die Page eingefuegt, was das Drawing
    //aber gar nicht gut haben kann. Deshalb nehmen wir das Objekt hinterher
    //gleich wieder aus der Seite heraus.
    FASTBOOL bInserted = pNewMaster->IsInserted();
    ConnectToLayout();
    if ( !bInserted && pNewMaster->IsInserted() )
        ((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
                                    RemoveObject( GetMaster()->GetOrdNum() );
}


