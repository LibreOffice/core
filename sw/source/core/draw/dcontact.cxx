/*************************************************************************
 *
 *  $RCSfile: dcontact.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:02:08 $
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
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDVITER_HXX
#include <svx/svdviter.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
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

// #108784#
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif

// OD 2004-02-11 #110582#-2
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif

// AW, OD 2004-04-30 #i28501#
#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif
#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

// OD 18.06.2003 #108784#
#include <algorithm>
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
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
    SwFrmFmt* pRetval = 0L;

    if ( pObj->ISA(SwVirtFlyDrawObj) )
    {
       pRetval = ((SwVirtFlyDrawObj*)pObj)->GetFmt();
    }
    else
    {
        SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
        if ( pContact )
        {
            pRetval = pContact->GetFmt();
        }
    }

#if OSL_DEBUG_LEVEL > 1
    ASSERT( pRetval,
            "<::FindFrmFmt(..)> - no frame format found for given object. Please inform OD." );
#endif

    return pRetval;
}

sal_Bool HasWrap( const SdrObject* pObj )
{
    if ( pObj )
    {
        const SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
        if ( pFmt )
        {
            return SURROUND_THROUGHT != pFmt->GetSurround().GetSurround();
        }
    }

    return sal_False;
}

/*****************************************************************************
 *
 * GetBoundRect liefert das BoundRect _inklusive_ Abstand des Objekts.
 *
 *****************************************************************************/

SwRect GetBoundRect( const SdrObject* pObj )
{
    SwRect aRet( pObj->GetCurrentBoundRect() );
    const SwFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
    const SvxULSpaceItem &rUL = pFmt->GetULSpace();
    const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
    {
        aRet.Top ( Max( aRet.Top() - long(rUL.GetUpper()), 0L ));
        aRet.Left( Max( aRet.Left()- long(rLR.GetLeft()),  0L ));
        aRet.SSize().Height() += rUL.GetLower();
        aRet.SSize().Width()  += rLR.GetRight();
    }
    return aRet;
}

//Liefert den UserCall ggf. vom Gruppenobjekt
// OD 2004-03-31 #i26791# - change return type
SwContact* GetUserCall( const SdrObject* pObj )
{
    SdrObject *pTmp;
    while ( !pObj->GetUserCall() && 0 != (pTmp = pObj->GetUpGroup()) )
        pObj = pTmp;
    ASSERT( !pObj->GetUserCall() || pObj->GetUserCall()->ISA(SwContact),
            "<::GetUserCall(..)> - wrong type of found object user call." );
    return static_cast<SwContact*>(pObj->GetUserCall());
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

SwContact::SwContact( SwFrmFmt *pToRegisterIn ) :
    SwClient( pToRegisterIn ),
    // OD 05.09.2003 #112039# - init member <mbInDTOR>
    mbInDTOR( false )
{}

SwContact::~SwContact()
{
    // OD 05.09.2003 #112039# - set <mbInDTOR>
    SetInDTOR();
}

// OD 05.09.2003 #112039# - accessor for member <mbInDTOR>
const bool SwContact::IsInDTOR() const
{
    return mbInDTOR;
}

// OD 05.09.2003 #112039# - accessor to set member <mbInDTOR>
void SwContact::SetInDTOR()
{
    mbInDTOR = true;
}

/** method to move drawing object to corresponding visible layer

    OD 21.08.2003 #i18447#

    @author OD
*/
void SwContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    _MoveObjToLayer( true, _pDrawObj );
}

/** method to move drawing object to corresponding invisible layer

    OD 21.08.2003 #i18447#

    @author OD
*/
void SwContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    _MoveObjToLayer( false, _pDrawObj );
}

/** method to move object to visible/invisible layer

    OD 21.08.2003 #i18447#
    implementation for the public method <MoveObjToVisibleLayer(..)>
    and <MoveObjToInvisibleLayer(..)>

    @author OD
*/
void SwContact::_MoveObjToLayer( const bool _bToVisible,
                                 SdrObject* _pDrawObj )
{
    if ( !_pDrawObj )
    {
        ASSERT( false, "SwDrawContact::_MoveObjToLayer(..) - no drawing object!" );
        return;
    }

    if ( !pRegisteredIn )
    {
        ASSERT( false, "SwDrawContact::_MoveObjToLayer(..) - no drawing frame format!" );
        return;
    }

    SwDoc* pWriterDoc = static_cast<SwFrmFmt*>(pRegisteredIn)->GetDoc();
    if ( !pWriterDoc )
    {
        ASSERT( false, "SwDrawContact::_MoveObjToLayer(..) - no writer document!" );
        return;
    }

    SdrLayerID nToHellLayerId =
        _bToVisible ? pWriterDoc->GetHellId() : pWriterDoc->GetInvisibleHellId();
    SdrLayerID nToHeavenLayerId =
        _bToVisible ? pWriterDoc->GetHeavenId() : pWriterDoc->GetInvisibleHeavenId();
    SdrLayerID nToControlLayerId =
        _bToVisible ? pWriterDoc->GetControlsId() : pWriterDoc->GetInvisibleControlsId();
    SdrLayerID nFromHellLayerId =
        _bToVisible ? pWriterDoc->GetInvisibleHellId() : pWriterDoc->GetHellId();
    SdrLayerID nFromHeavenLayerId =
        _bToVisible ? pWriterDoc->GetInvisibleHeavenId() : pWriterDoc->GetHeavenId();
    SdrLayerID nFromControlLayerId =
        _bToVisible ? pWriterDoc->GetInvisibleControlsId() : pWriterDoc->GetControlsId();

    if ( _pDrawObj->ISA( SdrObjGroup ) )
    {
        // determine layer for group object
        {
            // proposed layer of a group object is the hell layer
            SdrLayerID nNewLayerId = nToHellLayerId;
            if ( ::CheckControlLayer( _pDrawObj ) )
            {
                // it has to be the control layer, if one of the member
                // is a control
                nNewLayerId = nToControlLayerId;
            }
            else if ( _pDrawObj->GetLayer() == pWriterDoc->GetHeavenId() ||
                      _pDrawObj->GetLayer() == pWriterDoc->GetInvisibleHeavenId() )
            {
                // it has to be the heaven layer, if method <GetLayer()> reveals
                // a heaven layer
                nNewLayerId = nToHeavenLayerId;
            }
            // set layer at group object, but do *not* broadcast and
            // no propagation to the members.
            // Thus, call <NbcSetLayer(..)> at super class
            _pDrawObj->SdrObject::NbcSetLayer( nNewLayerId );
        }

        // call method recursively for group object members
        const SdrObjList* pLst =
                static_cast<SdrObjGroup*>(_pDrawObj)->GetSubList();
        if ( pLst )
        {
            for ( USHORT i = 0; i < pLst->GetObjCount(); ++i )
            {
                _MoveObjToLayer( _bToVisible, pLst->GetObj( i ) );
            }
        }
    }
    else
    {
        const SdrLayerID nLayerIdOfObj = _pDrawObj->GetLayer();
        if ( nLayerIdOfObj == nFromHellLayerId )
        {
            _pDrawObj->SetLayer( nToHellLayerId );
        }
        else if ( nLayerIdOfObj == nFromHeavenLayerId )
        {
            _pDrawObj->SetLayer( nToHeavenLayerId );
        }
        else if ( nLayerIdOfObj == nFromControlLayerId )
        {
            _pDrawObj->SetLayer( nToControlLayerId );
        }
    }
}

// -------------------------------------------------------------------------
// OD 2004-01-16 #110582# - some virtual helper methods for information
// about the object (Writer fly frame resp. drawing object)

const SwNodeIndex& SwContact::GetCntntAnchorNode() const
{
    return GetCntntAnchor().nNode;
}

const SwIndex& SwContact::GetCntntAnchorIndex() const
{
    return GetCntntAnchor().nContent;
}

// -------------------------------------------------------------------------

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
    // OD 2004-04-01 #i26791# - class <SwFlyDrawContact> contains the 'master'
    // drawing object of type <SwFlyDrawObj> on its own.
    mpMasterObj = new SwFlyDrawObj;
    mpMasterObj->SetOrdNum( 0xFFFFFFFE );
    mpMasterObj->SetUserCall( this );
}

SwFlyDrawContact::SwFlyDrawContact( SwFrmFmt* pToRegisterIn, SdrObject* pObj ):
    SwContact( pToRegisterIn )
{
#if OSL_DEBUG_LEVEL > 1
    ASSERT( pObj, "<SwFlyDrawContact::SwFlyDrawContact(..)> - no <SdrObject> provided. Please inform OD." );
#endif
    ASSERT( !pObj || pObj->ISA(SwFlyDrawObj),
            "<SwFlyDrawContact::SwFlyDrawContact(..)> - wrong type of <SdrObject>." );
    mpMasterObj = static_cast<SwFlyDrawObj*>(pObj);
    mpMasterObj->SetUserCall( this );
}

SwFlyDrawContact::~SwFlyDrawContact()
{
    if ( mpMasterObj )
    {
        mpMasterObj->SetUserCall( 0 );
        if ( mpMasterObj->GetPage() )
            mpMasterObj->GetPage()->RemoveObject( mpMasterObj->GetOrdNum() );
        delete mpMasterObj;
    }
}

// OD 2004-03-29 #i26791#
const SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    ASSERT( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( _pSdrObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    ASSERT( GetUserCall( _pSdrObj ) == const_cast<SwFlyDrawContact*>(this),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj && _pSdrObj->ISA(SwVirtFlyDrawObj) )
    {
        pRetAnchoredObj = static_cast<const SwVirtFlyDrawObj*>(_pSdrObj)->GetFlyFrm();
    }

    return pRetAnchoredObj;
}

SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( SdrObject* _pSdrObj )
{
    ASSERT( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( _pSdrObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type provided" );
    ASSERT( GetUserCall( _pSdrObj ) == this,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj && _pSdrObj->ISA(SwVirtFlyDrawObj) )
    {
        pRetAnchoredObj = static_cast<SwVirtFlyDrawObj*>(_pSdrObj)->GetFlyFrm();
    }

    return pRetAnchoredObj;
}

const SdrObject* SwFlyDrawContact::GetMaster() const
{
    return mpMasterObj;
}

SdrObject* SwFlyDrawContact::GetMaster()
{
    return mpMasterObj;
}

void SwFlyDrawContact::SetMaster( SdrObject* _pNewMaster )
{
    ASSERT( _pNewMaster->ISA(SwFlyDrawObj),
            "<SwFlyDrawContact::SetMaster(..)> - wrong type of new master object" );
    mpMasterObj = static_cast<SwFlyDrawObj *>(_pNewMaster);
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

// OD 2004-01-16 #110582# - override method to control Writer fly frames,
// which are linked, and to assure that all objects anchored at/inside the
// Writer fly frame are also made visible.
void SwFlyDrawContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    ASSERT( _pDrawObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::MoveObjToVisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( GetFmt()->GetDoc()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        // nothing to do
        return;
    }

    SwFlyFrm* pFlyFrm = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrm();

    // OD 2004-02-12 #110582#-2 - insert columns, if necessary
    pFlyFrm->InsertColumns();
    pFlyFrm->Chain( pFlyFrm->AnchorFrm() );
    pFlyFrm->InsertCnt();
    if ( pFlyFrm->GetDrawObjs() )
    {
        for ( sal_uInt8 i = 0; i < pFlyFrm->GetDrawObjs()->Count(); ++i)
        {
            // --> OD 2004-07-01 #i28701# - consider type of objects in sorted object list.
            SdrObject* pObj = (*pFlyFrm->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToVisibleLayer( pObj );
        }
    }

    // make fly frame visible
    SwContact::MoveObjToVisibleLayer( _pDrawObj );

    // OD 2004-02-11 #110582#-2
    if ( pFlyFrm->IsFlyInCntFrm() )
    {
        static_cast<SwFlyInCntFrm*>(pFlyFrm)->SetRefPoint( Point(), Point(), Point() );
    }
    else
    {
        pFlyFrm->Frm().Pos( Point() );
    }
    pFlyFrm->InvalidatePos();
}

// OD 2004-01-16 #110582# - override method to control Writer fly frames,
// which are linked, and to assure that all objects anchored at/inside the
// Writer fly frame are also made invisible.
void SwFlyDrawContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    ASSERT( _pDrawObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::MoveObjToInvisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        // nothing to do
        return;
    }

    SwFlyFrm* pFlyFrm = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrm();

    pFlyFrm->Unchain();
    pFlyFrm->DeleteCnt();
    if ( pFlyFrm->GetDrawObjs() )
    {
        for ( sal_uInt8 i = 0; i < pFlyFrm->GetDrawObjs()->Count(); ++i)
        {
            // --> OD 2004-07-01 #i28701# - consider type of objects in sorted object list.
            SdrObject* pObj = (*pFlyFrm->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToInvisibleLayer( pObj );
        }
    }

    // make fly frame invisible
    SwContact::MoveObjToInvisibleLayer( _pDrawObj );
}

/*************************************************************************
|*
|*  SwDrawContact, Ctor+Dtor
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 22. Jul. 98
|*
|*************************************************************************/
bool CheckControlLayer( const SdrObject *pObj )
{
    if ( FmFormInventor == pObj->GetObjInventor() )
        return true;
    if ( pObj->ISA( SdrObjGroup ) )
    {
        const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( USHORT i = 0; i < pLst->GetObjCount(); ++i )
        {
            if ( ::CheckControlLayer( pLst->GetObj( i ) ) )
            {
                // OD 21.08.2003 #i18447# - return correct value ;-)
                return true;
            }
        }
    }
    return false;
}

SwDrawContact::SwDrawContact( SwFrmFmt* pToRegisterIn, SdrObject* pObj ) :
    SwContact( pToRegisterIn ),
    maAnchoredDrawObj(),
    mbMasterObjCleared( false ),
    // OD 10.10.2003 #112299#
    mbDisconnectInProgress( false )
{
    // clear vector containing 'virtual' drawing objects.
    maDrawVirtObjs.clear();

    //Controls muessen immer im Control-Layer liegen. Das gilt auch fuer
    //Gruppenobjekte, wenn diese Controls enthalten.
    if ( ::CheckControlLayer( pObj ) )
    {
        // OD 25.06.2003 #108784# - set layer of object to corresponding invisible layer.
        pObj->SetLayer( pToRegisterIn->GetDoc()->GetInvisibleControlsId() );
    }

    // OD 2004-03-29 #i26791#
    pObj->SetUserCall( this );
    maAnchoredDrawObj.SetDrawObj( *pObj );
}

SwDrawContact::~SwDrawContact()
{
    // OD 05.09.2003 #112039# - set <mbInDTOR>
    SetInDTOR();

    DisconnectFromLayout();

    // OD 25.06.2003 #108784# - remove 'master' from drawing page
    RemoveMasterFromDrawPage();

    // remove and destroy 'virtual' drawing objects.
    RemoveAllVirtObjs();

    if ( !mbMasterObjCleared )
    {
        delete maAnchoredDrawObj.GetDrawObj();
    }
}

// OD 2004-03-29 #i26791#
const SwAnchoredObject* SwDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    // handle default parameter value
    if ( !_pSdrObj )
    {
        _pSdrObj = GetMaster();
    }

    ASSERT( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( _pSdrObj->ISA(SwDrawVirtObj) ||
            ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) ),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    ASSERT( GetUserCall( _pSdrObj ) == const_cast<SwDrawContact*>(this) ||
            _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj )
    {
        if ( _pSdrObj->ISA(SwDrawVirtObj) )
        {
            pRetAnchoredObj = static_cast<const SwDrawVirtObj*>(_pSdrObj)->GetAnchoredObj();
        }
        else if ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) )
        {
            pRetAnchoredObj = &maAnchoredDrawObj;
        }
    }

    return pRetAnchoredObj;
}

SwAnchoredObject* SwDrawContact::GetAnchoredObj( SdrObject* _pSdrObj )
{
    // handle default parameter value
    if ( !_pSdrObj )
    {
        _pSdrObj = GetMaster();
    }

    ASSERT( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( _pSdrObj->ISA(SwDrawVirtObj) ||
            ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) ),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    ASSERT( GetUserCall( _pSdrObj ) == this || _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj )
    {
        if ( _pSdrObj->ISA(SwDrawVirtObj) )
        {
            pRetAnchoredObj = static_cast<SwDrawVirtObj*>(_pSdrObj)->AnchoredObj();
        }
        else if ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) )
        {
            pRetAnchoredObj = &maAnchoredDrawObj;
        }
    }

    return pRetAnchoredObj;
}

const SdrObject* SwDrawContact::GetMaster() const
{
    return !mbMasterObjCleared
           ? maAnchoredDrawObj.GetDrawObj()
           : 0L;
}

SdrObject* SwDrawContact::GetMaster()
{
    return !mbMasterObjCleared
           ? maAnchoredDrawObj.DrawObj()
           : 0L;
}

// OD 16.05.2003 #108784# - overload <SwContact::SetMaster(..)> in order to
// assert, if the 'master' drawing object is replaced.
// OD 10.07.2003 #110742# - replace of master object correctly handled, if
// handled by method <SwDrawContact::ChangeMasterObject(..)>. Thus, assert
// only, if a debug level is given.
void SwDrawContact::SetMaster( SdrObject* _pNewMaster )
{
    if ( _pNewMaster )
    {
#if OSL_DEBUG_LEVEL > 1
        ASSERT( false, "debug notification - master replaced!" );
#endif
        maAnchoredDrawObj.SetDrawObj( *_pNewMaster );
    }
    else
    {
        mbMasterObjCleared = true;
    }
}

const SwFrm* SwDrawContact::GetAnchorFrm( const SdrObject* _pDrawObj ) const
{
    const SwFrm* pAnchorFrm = 0L;
    if ( !_pDrawObj ||
         _pDrawObj == GetMaster() ||
         ( !_pDrawObj->GetUserCall() &&
           GetUserCall( _pDrawObj ) == static_cast<const SwContact* const>(this) ) )
    {
        pAnchorFrm = maAnchoredDrawObj.GetAnchorFrm();
    }
    else if ( _pDrawObj->ISA(SwDrawVirtObj) )
    {
        pAnchorFrm = static_cast<const SwDrawVirtObj*>(_pDrawObj)->GetAnchorFrm();
    }
    else
    {
        ASSERT( false,
                "<SwDrawContact::GetAnchorFrm(..)> - unknown drawing object." )
    }

    return pAnchorFrm;
}
SwFrm* SwDrawContact::GetAnchorFrm( SdrObject* _pDrawObj )
{
    SwFrm* pAnchorFrm = 0L;
    if ( !_pDrawObj ||
         _pDrawObj == GetMaster() ||
         ( !_pDrawObj->GetUserCall() &&
           GetUserCall( _pDrawObj ) == this ) )
    {
        pAnchorFrm = maAnchoredDrawObj.AnchorFrm();
    }
    else
    {
        ASSERT( _pDrawObj->ISA(SwDrawVirtObj),
                "<SwDrawContact::GetAnchorFrm(..)> - unknown drawing object." )
        pAnchorFrm = static_cast<SwDrawVirtObj*>(_pDrawObj)->AnchorFrm();
    }

    return pAnchorFrm;
}

// OD 23.06.2003 #108784# - method to create a new 'virtual' drawing object.
SwDrawVirtObj* SwDrawContact::CreateVirtObj()
{
    // determine 'master'
    SdrObject* pOrgMasterSdrObj = GetMaster();

    // create 'virtual' drawing object
    SwDrawVirtObj* pNewDrawVirtObj = new SwDrawVirtObj ( *(pOrgMasterSdrObj), *(this) );

    // add new 'virtual' drawing object managing data structure
    maDrawVirtObjs.push_back( pNewDrawVirtObj );

    return pNewDrawVirtObj;
}

// OD 23.06.2003 #108784# - destroys a given 'virtual' drawing object.
// side effect: 'virtual' drawing object is removed from data structure
//              <maDrawVirtObjs>.
void SwDrawContact::DestroyVirtObj( SwDrawVirtObj* _pVirtObj )
{
    if ( _pVirtObj )
    {
        delete _pVirtObj;
        _pVirtObj = 0;
    }
}

// OD 16.05.2003 #108784# - add a 'virtual' drawing object to drawing page.
// Use an already created one, which isn't used, or create a new one.
SwDrawVirtObj* SwDrawContact::AddVirtObj()
{
    SwDrawVirtObj* pAddedDrawVirtObj = 0L;

    // check, if a disconnected 'virtual' drawing object exist and use it
    std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
            std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
                          UsedOrUnusedVirtObjPred( false ) );

    if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
    {
        // use already created, disconnected 'virtual' drawing object
        pAddedDrawVirtObj = (*aFoundVirtObjIter);
    }
    else
    {
        // create new 'virtual' drawing object.
        pAddedDrawVirtObj = CreateVirtObj();
    }
    pAddedDrawVirtObj->AddToDrawingPage();

    return pAddedDrawVirtObj;
}

// OD 16.05.2003 #108784# - remove 'virtual' drawing objects and destroy them.
void SwDrawContact::RemoveAllVirtObjs()
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjsIter = maDrawVirtObjs.begin();
          aDrawVirtObjsIter != maDrawVirtObjs.end();
          ++aDrawVirtObjsIter )
    {
        // remove and destroy 'virtual object'
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjsIter);
        pDrawVirtObj->RemoveFromWriterLayout();
        pDrawVirtObj->RemoveFromDrawingPage();
        DestroyVirtObj( pDrawVirtObj );
    }
    maDrawVirtObjs.clear();
}

SwDrawContact::VirtObjAnchoredAtFrmPred::VirtObjAnchoredAtFrmPred(
                                                const SwFrm& _rAnchorFrm )
    : mpAnchorFrm( &_rAnchorFrm )
{
    if ( mpAnchorFrm->IsCntntFrm() )
    {
        const SwCntntFrm* pTmpFrm =
                            static_cast<const SwCntntFrm*>( mpAnchorFrm );
        while ( pTmpFrm->IsFollow() )
        {
            pTmpFrm = pTmpFrm->FindMaster();
        }
        mpAnchorFrm = pTmpFrm;
    }
}

// OD 2004-04-14 #i26791# - compare with master frame
bool SwDrawContact::VirtObjAnchoredAtFrmPred::operator() ( const SwDrawVirtObj* _pDrawVirtObj )
{
    const SwFrm* pObjAnchorFrm = _pDrawVirtObj->GetAnchorFrm();
    if ( pObjAnchorFrm && pObjAnchorFrm->IsCntntFrm() )
    {
        const SwCntntFrm* pTmpFrm =
                            static_cast<const SwCntntFrm*>( pObjAnchorFrm );
        while ( pTmpFrm->IsFollow() )
        {
            pTmpFrm = pTmpFrm->FindMaster();
        }
        pObjAnchorFrm = pTmpFrm;
    }

    return ( pObjAnchorFrm == mpAnchorFrm );
}

// OD 19.06.2003 #108784# - get drawing object ('master' or 'virtual') by frame.
SdrObject* SwDrawContact::GetDrawObjectByAnchorFrm( const SwFrm& _rAnchorFrm )
{
    SdrObject* pRetDrawObj = 0L;

    // OD 2004-04-14 #i26791# - compare master frames instead of direct frames
    const SwFrm* pProposedAnchorFrm = &_rAnchorFrm;
    if ( pProposedAnchorFrm->IsCntntFrm() )
    {
        const SwCntntFrm* pTmpFrm =
                            static_cast<const SwCntntFrm*>( pProposedAnchorFrm );
        while ( pTmpFrm->IsFollow() )
        {
            pTmpFrm = pTmpFrm->FindMaster();
        }
        pProposedAnchorFrm = pTmpFrm;
    }

    const SwFrm* pMasterObjAnchorFrm = GetAnchorFrm();
    if ( pMasterObjAnchorFrm && pMasterObjAnchorFrm->IsCntntFrm() )
    {
        const SwCntntFrm* pTmpFrm =
                            static_cast<const SwCntntFrm*>( pMasterObjAnchorFrm );
        while ( pTmpFrm->IsFollow() )
        {
            pTmpFrm = pTmpFrm->FindMaster();
        }
        pMasterObjAnchorFrm = pTmpFrm;
    }

    if ( pMasterObjAnchorFrm && pMasterObjAnchorFrm == pProposedAnchorFrm )
    {
        pRetDrawObj = GetMaster();
    }
    else
    {
        std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
                std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
                              VirtObjAnchoredAtFrmPred( *pProposedAnchorFrm ) );

        if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
        {
            pRetDrawObj = (*aFoundVirtObjIter);
        }
    }

    return pRetDrawObj;
}

/*************************************************************************
|*
|*  SwDrawContact::Changed
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 29. May. 96
|*
|*************************************************************************/

// OD 03.07.2003 #108784#
void SwDrawContact::NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect )
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        if ( pDrawVirtObj->GetAnchorFrm() )
        {
            SwPageFrm* pPage = pDrawVirtObj->AnchorFrm()->FindPageFrm();
            if( pOldBoundRect && pPage )
            {
                SwRect aOldRect( *pOldBoundRect );
                aOldRect.Pos() += pDrawVirtObj->GetOffset();
                if( aOldRect.HasArea() )
                    ::Notify_Background( pDrawVirtObj ,pPage,
                                       aOldRect, PREP_FLY_LEAVE,TRUE);
            }
            SwRect aRect( pDrawVirtObj->GetCurrentBoundRect() );
            if( aRect.HasArea() )
            {
                SwPageFrm *pPg = pDrawVirtObj->GetPageFrm();
                if ( !pPg )
                    pPg = pDrawVirtObj->AnchorFrm()->FindPageFrm();
                if ( pPg )
                    pPg = (SwPageFrm*)::FindPage( aRect, pPg );
                if( pPg )
                    ::Notify_Background( pDrawVirtObj, pPg, aRect,
                                        PREP_FLY_ARRIVE, TRUE );
            }
            ::ClrContourCache( pDrawVirtObj );
        }
    }
}

// OD 2004-04-08 #i26791# - local method to notify the background for a drawing object
void lcl_NotifyBackgroundOfObj( SwDrawContact& _rDrawContact,
                                const SdrObject& _rObj,
                                const Rectangle* _pOldObjRect )
{
    if ( _rDrawContact.GetAnchorFrm( &_rObj ) )
    {
        SwPageFrm* pPageFrm = const_cast<SwPageFrm*>(
                            _rDrawContact.GetAnchorFrm( &_rObj )->FindPageFrm() );
        if( _pOldObjRect && pPageFrm )
        {
            SwRect aOldRect( *_pOldObjRect );
            if( aOldRect.HasArea() )
            {
                ::Notify_Background( &_rObj, pPageFrm, aOldRect,
                                     PREP_FLY_LEAVE, TRUE);
            }
        }
        SwRect aNewRect( _rObj.GetCurrentBoundRect() );
        if( aNewRect.HasArea() && pPageFrm )
        {
            pPageFrm = (SwPageFrm*)::FindPage( aNewRect, pPageFrm );
            ::Notify_Background( &_rObj, pPageFrm, aNewRect,
                                 PREP_FLY_ARRIVE, TRUE );
        }
        ClrContourCache( &_rObj );
    }
}

void SwDrawContact::Changed( const SdrObject& rObj,
                             SdrUserCallType eType,
                             const Rectangle& rOldBoundRect )
{
    // OD 2004-06-01 #i26791# - no event handling, if existing <ViewShell>
    // is in contruction
    SwDoc* pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetRootFrm() &&
         pDoc->GetRootFrm()->GetCurrShell() &&
         pDoc->GetRootFrm()->GetCurrShell()->IsInConstructor() )
    {
        return;
    }

    //Action aufsetzen, aber nicht wenn gerade irgendwo eine Action laeuft.
    ViewShell *pSh = 0, *pOrg;
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

void SwDrawContact::_Changed( const SdrObject& rObj,
                              SdrUserCallType eType,
                              const Rectangle* pOldBoundRect )
{
    /// OD 05.08.2002 #100843# - do *not* notify, if document is destructing
    const bool bNotify = !(GetFmt()->GetDoc()->IsInDtor()) &&
                         ( SURROUND_THROUGHT != GetFmt()->GetSurround().GetSurround() ||
                           ObjAnchoredAsChar() );
    switch( eType )
    {
        case SDRUSERCALL_DELETE:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
                }
                DisconnectFromLayout( false );
                SetMaster( NULL );
                delete this;
                break;
            }
        case SDRUSERCALL_INSERTED:
            {
                // OD 10.10.2003 #112299#
                if ( mbDisconnectInProgress )
                {
                    ASSERT( false,
                            "<SwDrawContact::_Changed(..)> - Insert event during disconnection from layout is invalid." );
                }
                else
                {
                    ConnectToLayout();
                    if ( bNotify )
                    {
                        lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
                    }
                }
                break;
            }
        case SDRUSERCALL_REMOVED:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
                }
                DisconnectFromLayout( false );
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
        {
            const Rectangle& aOldObjRect =
                        static_cast<const SwAnchoredDrawObject*>(
                                    GetAnchoredObj( &rObj ))->GetLastObjRect();
            // OD 2004-04-06 #i26791# - adjust positioning and alignment attributes,
            // if positioning of drawing object isn't in progress.
            if ( !GetAnchoredObj( &rObj )->IsPositioningInProgress() )
            {
                const SwFrm* pAnchorFrm = GetAnchorFrm( &rObj );
                if ( pAnchorFrm )
                {
                    const bool bVert = pAnchorFrm->IsVertical();
                    const bool bR2L = pAnchorFrm->IsRightToLeft();
                    // use geometry of drawing object
                    SwRect aObjRect( rObj.GetSnapRect() );
                    // If drawing object is a member of a group, the adjustment
                    // of the positioning and the alignment attributes has to
                    // be done for the top group object.
                    if ( rObj.GetUpGroup() )
                    {
                        const SdrObject* pGroupObj = rObj.GetUpGroup();
                        while ( pGroupObj->GetUpGroup() )
                        {
                            pGroupObj = pGroupObj->GetUpGroup();
                        }
                        // use geometry of drawing object
                        aObjRect = pGroupObj->GetSnapRect();
                    }
                    SwTwips nXPosDiff(0L);
                    SwTwips nYPosDiff(0L);
                    if ( bVert )
                    {
                        nXPosDiff = aObjRect.Top() - aOldObjRect.Top();
                        nYPosDiff = aOldObjRect.Right() - aObjRect.Right();
                    }
                    else if ( bR2L )
                    {
                        nXPosDiff = aOldObjRect.Right() - aObjRect.Right();
                        nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                    }
                    else
                    {
                        nXPosDiff = aObjRect.Left() - aOldObjRect.Left();
                        nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                    }
                    SfxItemSet aSet( GetFmt()->GetDoc()->GetAttrPool(),
                                     RES_VERT_ORIENT, RES_HORI_ORIENT, 0 );
                    const SwFmtVertOrient& rVert = GetFmt()->GetVertOrient();
                    if ( nYPosDiff != 0 )
                    {

                        if ( rVert.GetRelationOrient() == REL_CHAR ||
                             rVert.GetRelationOrient() == REL_VERT_LINE )
                        {
                            nYPosDiff = -nYPosDiff;
                        }
                        aSet.Put( SwFmtVertOrient( rVert.GetPos()+nYPosDiff,
                                                   VERT_NONE,
                                                   rVert.GetRelationOrient() ) );
                    }

                    const SwFmtHoriOrient& rHori = GetFmt()->GetHoriOrient();
                    if ( !ObjAnchoredAsChar() && nXPosDiff != 0 )
                    {
                        aSet.Put( SwFmtHoriOrient( rHori.GetPos()+nXPosDiff,
                                                   HORI_NONE,
                                                   rHori.GetRelationOrient() ) );
                    }

                    if ( nYPosDiff ||
                         ( !ObjAnchoredAsChar() && nXPosDiff != 0 ) )
                    {
                        GetFmt()->GetDoc()->SetFlyFrmAttr( *(GetFmt()), aSet );
                        // keep new object rectangle, to avoid multiple
                        // changes of the attributes by multiple event from
                        // the drawing layer - e.g. group objects and its members
                        static_cast<SwAnchoredDrawObject*>(
                            GetAnchoredObj( &(const_cast<SdrObject&>(rObj) )))
                                ->LastObjRect() = aObjRect.SVRect();
                    }
                    else if ( aObjRect.SSize() != aOldObjRect.GetSize() )
                    {
                        _InvalidateObjs();
                    }
                }
                else
                {
                    static_cast<SwAnchoredDrawObject*>(
                            GetAnchoredObj( GetMaster() ))->SetPositioningAttr();
                }
            }
            if ( bNotify )
            {
                // --> OD 2004-07-20 #i31573# - correction: Only invalidate
                // background of given drawing object.
                lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect );
            }
        }
        break;
        case SDRUSERCALL_CHGATTR:
            if ( bNotify )
            {
                lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
            }
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
    // OD 10.10.2003 #112299#
    ASSERT( !mbDisconnectInProgress,
            "<SwDrawContact::Modify(..)> called during disconnection.");

    USHORT nWhich = pNew ? pNew->Which() : 0;
    SwFmtAnchor* pAnchorFmt = 0L;
    if ( RES_ATTRSET_CHG == nWhich )
    {
        if ( SFX_ITEM_SET == static_cast<SwAttrSetChg*>(pNew)->GetChgSet()->
                GetItemState( RES_ANCHOR, FALSE, (const SfxPoolItem**)&pAnchorFmt ) )
        {
            // <pAnchorFmt> is set and will be handled below
        }
    }
    else if ( RES_ANCHOR == nWhich )
    {
        pAnchorFmt = static_cast<SwFmtAnchor*>(pNew);
    }

    if ( pAnchorFmt )
    {
        // JP 10.04.95: nicht auf ein Reset Anchor reagieren !!!!!
        if ( SFX_ITEM_SET ==
                GetFmt()->GetAttrSet().GetItemState( RES_ANCHOR, FALSE ) )
        {
            // OD 10.10.2003 #112299# - no connect to layout during disconnection
            if ( !mbDisconnectInProgress )
            {
                // determine old object retangle of 'master' drawing object
                // for notification
                const Rectangle* pOldRect = 0L;
                if ( GetAnchorFrm() )
                {
                    pOldRect = &GetMaster()->GetCurrentBoundRect();
                }
                // re-connect to layout due to anchor format change
                ConnectToLayout( pAnchorFmt );
                // notify background of drawing objects
                lcl_NotifyBackgroundOfObj( *this, *GetMaster(), pOldRect );
                NotifyBackgrdOfAllVirtObjs( pOldRect );
            }
        }
        else
            DisconnectFromLayout();
    }
    // --> OD 2004-07-01 #i28701# - on change of wrapping style, hell|heaven layer,
    // or wrapping style influence an update of the <SwSortedObjs> list,
    // the drawing object is registered in, has to be performed. This is triggered
    // by the 1st parameter of method call <_InvalidateObjs(..)>.
    else if ( RES_SURROUND == nWhich ||
              RES_OPAQUE == nWhich ||
              RES_WRAP_INFLUENCE_ON_OBJPOS == nWhich ||
              ( RES_ATTRSET_CHG == nWhich &&
                ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_SURROUND, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_OPAQUE, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_WRAP_INFLUENCE_ON_OBJPOS, FALSE ) ) ) )
    {
        lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
        NotifyBackgrdOfAllVirtObjs( 0L );
        _InvalidateObjs( true );
    }
    else if ( RES_UL_SPACE == nWhich || RES_LR_SPACE == nWhich ||
              RES_HORI_ORIENT == nWhich || RES_VERT_ORIENT == nWhich ||
              // --> OD 2004-07-01 #i28701# - add attribute 'Follow text flow'
              RES_FOLLOW_TEXT_FLOW == nWhich ||
              ( RES_ATTRSET_CHG == nWhich &&
                ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_LR_SPACE, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_UL_SPACE, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_HORI_ORIENT, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_VERT_ORIENT, FALSE ) ||
                  SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                            RES_FOLLOW_TEXT_FLOW, FALSE ) ) ) )
    {
        lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
        NotifyBackgrdOfAllVirtObjs( 0L );
        _InvalidateObjs();
    }
    else
    {
        ASSERT( false,
                "<SwDrawContact::Modify(..)> - unhandled attribute? - please inform od@openoffice.org" );
    }
}

// OD 2004-03-31 #i26791#
// --> OD 2004-07-01 #i28701# - added parameter <_bUpdateSortedObjsList>
void SwDrawContact::_InvalidateObjs( const bool _bUpdateSortedObjsList )
{
    // invalidate position of existing 'virtual' drawing objects
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        pDrawVirtObj->AnchoredObj()->InvalidateObjPos();
        // --> OD 2004-07-01 #i28701#
        if ( _bUpdateSortedObjsList )
        {
            pDrawVirtObj->AnchoredObj()->UpdateObjInSortedList();
        }
        // <--
    }

    // invalidate position of 'master' drawing object
    SwAnchoredObject* pAnchoredObj = GetAnchoredObj();
    pAnchoredObj->InvalidateObjPos();
    // --> OD 2004-07-01 #i28701#
    if ( _bUpdateSortedObjsList )
    {
        pAnchoredObj->UpdateObjInSortedList();
    }
    // <--
}

/*************************************************************************
|*
|*  SwDrawContact::DisconnectFromLayout()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/

void SwDrawContact::DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer )
{
    // OD 10.10.2003 #112299#
    mbDisconnectInProgress = true;

    // OD 16.05.2003 #108784# - remove 'virtual' drawing objects from writer
    // layout and from drawing page
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        pDrawVirtObj->RemoveFromWriterLayout();
        pDrawVirtObj->RemoveFromDrawingPage();
    }

    if ( maAnchoredDrawObj.GetAnchorFrm() )
    {
        maAnchoredDrawObj.AnchorFrm()->RemoveDrawObj( maAnchoredDrawObj );
    }

    if ( _bMoveMasterToInvisibleLayer && GetMaster() && GetMaster()->IsInserted() )
    {
        SdrViewIter aIter( GetMaster() );
        for( SdrView* pView = aIter.FirstView(); pView;
                    pView = aIter.NextView() )
        {
            pView->MarkObj( GetMaster(), pView->GetPageViewPvNum(0), TRUE );
        }

        // OD 25.06.2003 #108784# - Instead of removing 'master' object from
        // drawing page, move the 'master' drawing object into the corresponding
        // invisible layer.
        {
            //((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
            //                            RemoveObject( GetMaster()->GetOrdNum() );
            // OD 21.08.2003 #i18447# - in order to consider group object correct
            // use new method <SwDrawContact::MoveObjToInvisibleLayer(..)>
            MoveObjToInvisibleLayer( GetMaster() );
        }
    }

    // OD 10.10.2003 #112299#
    mbDisconnectInProgress = false;
}

// OD 26.06.2003 #108784# - method to remove 'master' drawing object
// from drawing page.
void SwDrawContact::RemoveMasterFromDrawPage()
{
    if ( GetMaster() )
    {
        GetMaster()->SetUserCall( 0 );
        if ( GetMaster()->IsInserted() )
        {
            ((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
                                        RemoveObject( GetMaster()->GetOrdNum() );
        }
    }
}

// OD 19.06.2003 #108784# - disconnect for a dedicated drawing object -
// could be 'master' or 'virtual'.
// a 'master' drawing object will disconnect a 'virtual' drawing object
// in order to take its place.
// OD 13.10.2003 #i19919# - no special case, if drawing object isn't in
// page header/footer, in order to get drawing objects in repeating table headers
// also working.
void SwDrawContact::DisconnectObjFromLayout( SdrObject* _pDrawObj )
{
    if ( _pDrawObj->ISA(SwDrawVirtObj) )
    {
        SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(_pDrawObj);
        pDrawVirtObj->RemoveFromWriterLayout();
        pDrawVirtObj->RemoveFromDrawingPage();
    }
    else
    {
        std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
                std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
                              UsedOrUnusedVirtObjPred( true ) );
        if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
        {
            // replace found 'virtual' drawing object by 'master' drawing
            // object and disconnect the 'virtual' one
            SwDrawVirtObj* pDrawVirtObj = (*aFoundVirtObjIter);
            SwFrm* pNewAnchorFrmOfMaster = pDrawVirtObj->AnchorFrm();
            // disconnect 'virtual' drawing object
            pDrawVirtObj->RemoveFromWriterLayout();
            pDrawVirtObj->RemoveFromDrawingPage();
            // disconnect 'master' drawing object from current frame
            GetAnchorFrm()->RemoveDrawObj( maAnchoredDrawObj );
            // re-connect 'master' drawing object to frame of found 'virtual'
            // drawing object.
            pNewAnchorFrmOfMaster->AppendDrawObj( maAnchoredDrawObj );
        }
        else
        {
            // no connected 'virtual' drawing object found. Thus, disconnect
            // completely from layout.
            DisconnectFromLayout();
        }
    }
}

/*************************************************************************
|*
|*  SwDrawContact::ConnectToLayout()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/
SwTxtFrm* lcl_GetFlyInCntntAnchor( SwTxtFrm* _pProposedAnchorFrm,
                                   const xub_StrLen _nTxtOfs )
{
    SwTxtFrm* pAct = _pProposedAnchorFrm;
    SwTxtFrm* pTmp;
    do
    {
        pTmp = pAct;
        pAct = pTmp->GetFollow();
    }
    while( pAct && _nTxtOfs >= pAct->GetOfst() );
    return pTmp;
}

void SwDrawContact::ConnectToLayout( const SwFmtAnchor* pAnch )
{
    // OD 10.10.2003 #112299# - *no* connect to layout during disconnection from
    // layout.
    if ( mbDisconnectInProgress )
    {
        ASSERT( false,
                "<SwDrawContact::ConnectToLayout(..)> called during disconnection.");
        return;
    }

    SwFrmFmt* pDrawFrmFmt = (SwFrmFmt*)pRegisteredIn;

    SwRootFrm* pRoot = pDrawFrmFmt->GetDoc()->GetRootFrm();
    if ( !pRoot )
    {
        return;
    }

    // OD 16.05.2003 #108784# - remove 'virtual' drawing objects from writer
    // layout and from drawing page, and remove 'master' drawing object from
    // writer layout - 'master' object will remain in drawing page.
    DisconnectFromLayout( false );

    if ( !pAnch )
    {
        pAnch = &(pDrawFrmFmt->GetAnchor());
    }

    switch ( pAnch->GetAnchorId() )
    {
        case FLY_PAGE:
                {
                USHORT nPgNum = pAnch->GetPageNum();
                SwPageFrm *pPage = static_cast<SwPageFrm*>(pRoot->Lower());

                for ( USHORT i = 1; i < nPgNum && pPage; ++i )
                {
                    pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                }

                if ( pPage )
                {
                    pPage->AppendDrawObj( maAnchoredDrawObj );
                }
                else
                    //Sieht doof aus, ist aber erlaubt (vlg. SwFEShell::SetPageObjsNewPage)
                    pRoot->SetAssertFlyPages();
                }
                break;

        case FLY_AUTO_CNTNT:
        case FLY_AT_CNTNT:
        case FLY_AT_FLY:
        case FLY_IN_CNTNT:
            {
                if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                {
                    ClrContourCache( GetMaster() );
                }
                // OD 16.05.2003 #108784# - support drawing objects in header/footer,
                // but not control objects:
                // anchor at first found frame the 'master' object and
                // at the following frames 'virtual' drawing objects.
                // Note: method is similar to <SwFlyFrmFmt::MakeFrms(..)>
                SwModify *pModify = 0;
                if( pAnch->GetCntntAnchor() )
                {
                    if ( pAnch->GetAnchorId() == FLY_AT_FLY )
                    {
                        SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
                        SwCntntNode* pCNd = pDrawFrmFmt->GetDoc()->GetNodes().GoNext( &aIdx );
                        SwClientIter aIter( *pCNd );
                        if ( aIter.First( TYPE(SwFrm) ) )
                            pModify = pCNd;
                        else
                        {
                            const SwNodeIndex& rIdx = pAnch->GetCntntAnchor()->nNode;
                            SwSpzFrmFmts& rFmts = *(pDrawFrmFmt->GetDoc()->GetSpzFrmFmts());
                            for( sal_uInt16 i = 0; i < rFmts.Count(); ++i )
                            {
                                SwFrmFmt* pFlyFmt = rFmts[i];
                                if( pFlyFmt->GetCntnt().GetCntntIdx() &&
                                    rIdx == *(pFlyFmt->GetCntnt().GetCntntIdx()) )
                                {
                                    pModify = pFlyFmt;
                                    break;
                                }
                            }
                        }
                        // --> OD 2004-06-15 #i29199# - It is possible, that
                        // the anchor doesn't exist - E.g., reordering the
                        // sub-documents in a master document.
                        // Note: The anchor will be inserted later.
                        if ( !pModify )
                        {
                            // break to end of the current switch case.
                            break;
                        }
                    }
                    else
                    {
                        pModify = pAnch->GetCntntAnchor()->nNode.GetNode().GetCntntNode();
                    }
                }
                SwClientIter aIter( *pModify );
                SwFrm* pAnchorFrmOfMaster = 0;
                for( SwFrm *pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                     pFrm;
                     pFrm = (SwFrm*)aIter.Next() )
                {
                    // append drawing object, if
                    // (1) proposed anchor frame isn't a follow and
                    // (2) drawing object isn't a control object to be anchored
                    //     in header/footer.
                    const bool bAdd = ( !pFrm->IsCntntFrm() ||
                                        !((SwCntntFrm*)pFrm)->IsFollow() ) &&
                                      ( !::CheckControlLayer( GetMaster() ) ||
                                        !pFrm->FindFooterOrHeader() );

                    if( bAdd )
                    {
                        if ( FLY_AT_FLY == pAnch->GetAnchorId() && !pFrm->IsFlyFrm() )
                        {
                            pFrm = pFrm->FindFlyFrm();
                            ASSERT( pFrm,
                                    "<SwDrawContact::ConnectToLayout(..)> - missing fly frame -> crash." );
                        }

                        // OD 2004-01-20 #110582# - find correct follow for
                        // as character anchored objects.
                        if ( pAnch->GetAnchorId() == FLY_IN_CNTNT &&
                             pFrm->IsTxtFrm() )
                        {
                            pFrm = lcl_GetFlyInCntntAnchor(
                                        static_cast<SwTxtFrm*>(pFrm),
                                        pAnch->GetCntntAnchor()->nContent.GetIndex() );
                        }

                        if ( !pAnchorFrmOfMaster )
                        {
                            // OD 02.07.2003 #108784# - check, if 'master' drawing
                            // objects is inserted into drawing page. If not,
                            // assert and insert 'master' drawing object.
                            if ( !GetMaster()->IsInserted() )
                            {
                                // OD 25.06.2003 #108784# - debug assert
                                ASSERT( false, "<SwDrawContact::ConnectToLayout(..)> - master drawing object not inserted!?" );

                                pDrawFrmFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                                    InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
                            }
                            // append 'master' drawing object
                            pAnchorFrmOfMaster = pFrm;
                            pFrm->AppendDrawObj( maAnchoredDrawObj );
                        }
                        else
                        {
                            // append 'virtual' drawing object
                            SwDrawVirtObj* pDrawVirtObj = AddVirtObj();
                            if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                            {
                                ClrContourCache( pDrawVirtObj );
                            }
                            pFrm->AppendDrawObj( *(pDrawVirtObj->AnchoredObj()) );

                            // for repaint, use new ActionChanged()
                            // pDrawVirtObj->SendRepaintBroadcast();
                            pDrawVirtObj->ActionChanged();
                        }

                        if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                        {
                            pFrm->InvalidatePrt();
                        }
                    }
                }
            }
            break;
#ifndef PRODUCT
        default:    ASSERT( FALSE, "Unknown Anchor." );
#endif
    }
    if ( GetAnchorFrm() )
    {
        // OD 2004-04-01 #i26791# - invalidate objects instead of direct positioning
        _InvalidateObjs();
        // OD 21.08.2003 #i18447# - in order to consider group object correct
        // use new method <SwDrawContact::MoveObjToVisibleLayer(..)>
        MoveObjToVisibleLayer( GetMaster() );
    }
}

// OD 27.06.2003 #108784# - insert 'master' drawing object into drawing page
void SwDrawContact::InsertMasterIntoDrawPage()
{
    if ( !GetMaster()->IsInserted() )
    {
        GetFmt()->GetDoc()->GetDrawModel()->GetPage(0)
                ->InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
    }
    GetMaster()->SetUserCall( this );
}

/*************************************************************************
|*
|*  SwDrawContact::FindPage(), ChkPage()
|*
|*  Ersterstellung      MA 21. Mar. 95
|*  Letzte Aenderung    MA 19. Jul. 96
|*
|*************************************************************************/

SwPageFrm* SwDrawContact::FindPage( const SwRect &rRect )
{
    // --> OD 2004-07-01 #i28701# - use method <GetPageFrm()>
    SwPageFrm* pPg = GetPageFrm();
    if ( !pPg && GetAnchorFrm() )
        pPg = GetAnchorFrm()->FindPageFrm();
    if ( pPg )
        pPg = (SwPageFrm*)::FindPage( rRect, pPg );
    return pPg;
}

void SwDrawContact::ChkPage()
{
    // OD 10.10.2003 #112299#
    if ( mbDisconnectInProgress )
    {
        ASSERT( false,
                "<SwDrawContact::ChkPage()> called during disconnection." );
        return;
    }

    // --> OD 2004-07-01 #i28701#
    SwPageFrm* pPg = ( maAnchoredDrawObj.GetAnchorFrm() &&
                       maAnchoredDrawObj.GetAnchorFrm()->IsPageFrm() )
                     ? GetPageFrm()
                     : FindPage( GetMaster()->GetCurrentBoundRect() );
    if ( GetPageFrm() != pPg )
    {
        // OD 27.06.2003 #108784# - if drawing object is anchor in header/footer
        // a change of the page is a dramatic change. Thus, completely re-connect
        // to the layout
        if ( maAnchoredDrawObj.GetAnchorFrm() &&
             maAnchoredDrawObj.GetAnchorFrm()->FindFooterOrHeader() )
        {
            ConnectToLayout();
        }
        else
        {
            // --> OD 2004-07-01 #i28701# - use methods <GetPageFrm()> and <SetPageFrm>
            if ( GetPageFrm() )
                GetPageFrm()->RemoveDrawObjFromPage( maAnchoredDrawObj );
            pPg->AppendDrawObjToPage( maAnchoredDrawObj );
            SetPageFrm( pPg );
        }
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
// OD 10.07.2003 #110742# - Important note:
// method is called by method <SwDPage::ReplaceObject(..)>, which called its
// corresponding superclass method <FmFormPage::ReplaceObject(..)>.
// Note: 'master' drawing object *has* to be connected to layout triggered
//       by the caller of this, if method is called.
void SwDrawContact::ChangeMasterObject( SdrObject *pNewMaster )
{
    DisconnectFromLayout( false );
    // OD 10.07.2003 #110742# - consider 'virtual' drawing objects
    RemoveAllVirtObjs();

    GetMaster()->SetUserCall( 0 );
    SetMaster( pNewMaster );
    GetMaster()->SetUserCall( this );

    _InvalidateObjs();
}

// =============================================================================
/** implementation of class <SwDrawVirtObj>

    OD 14.05.2003 #108784#

    @author OD
*/

TYPEINIT1(SwDrawVirtObj,SdrVirtObj);

// #108784#
// implemetation of SwDrawVirtObj
SwDrawVirtObj::SwDrawVirtObj( SdrObject&        _rNewObj,
                              SwDrawContact&    _rDrawContact )
    : SdrVirtObj( _rNewObj ),
      // OD 2004-03-29 #i26791# - init new member <maAnchoredDrawObj>
      maAnchoredDrawObj(),
      mrDrawContact( _rDrawContact )
{
    // OD 2004-03-29 #i26791#
    maAnchoredDrawObj.SetDrawObj( *this );
}

SwDrawVirtObj::SwDrawVirtObj( SdrObject&        _rNewObj,
                              const Point&      _rAnchorPos,
                              SwDrawContact&    _rDrawContact )
    : SdrVirtObj( _rNewObj, _rAnchorPos ),
      // OD 2004-03-29 #i26791# - init new member <maAnchoredDrawObj>
      maAnchoredDrawObj(),
      mrDrawContact( _rDrawContact )
{
    // OD 2004-03-29 #i26791#
    maAnchoredDrawObj.SetDrawObj( *this );
}

SwDrawVirtObj::~SwDrawVirtObj()
{}

const Point SwDrawVirtObj::GetOffset() const
{
    return GetLastBoundRect().TopLeft() -
           GetReferencedObj().GetLastBoundRect().TopLeft();
}

void SwDrawVirtObj::operator=( const SdrObject& rObj )
{
    SdrVirtObj::operator=(rObj);
    // Note: Members <maAnchoredDrawObj> and <mrDrawContact>
    //       haven't to be considered.
}

SdrObject* SwDrawVirtObj::Clone() const
{
    SwDrawVirtObj* pObj = new SwDrawVirtObj( rRefObj, mrDrawContact );

    if ( pObj )
    {
        pObj->operator=(static_cast<const SdrObject&>(*this));
        // Note: Member <maAnchoredDrawObj> hasn't to be considered.
    }

    return pObj;
}

// --------------------------------------------------------------------
// connection to writer layout: <GetAnchoredObj()>, <SetAnchorFrm(..)>,
// <GetAnchorFrm()>, <SetPageFrm(..)>, <GetPageFrm()> and <RemoveFromWriterLayout()>
// --------------------------------------------------------------------
const SwAnchoredObject* SwDrawVirtObj::GetAnchoredObj() const
{
    return &maAnchoredDrawObj;
}

SwAnchoredObject* SwDrawVirtObj::AnchoredObj()
{
    return &maAnchoredDrawObj;
}

void SwDrawVirtObj::SetAnchorFrm( SwFrm* _pNewAnchorFrm )
{
    // OD 2004-03-29 #i26791# - use new member <maAnchoredDrawObj>
    maAnchoredDrawObj.ChgAnchorFrm( _pNewAnchorFrm );
}

const SwFrm* SwDrawVirtObj::GetAnchorFrm() const
{
    // OD 2004-03-29 #i26791# - use new member <maAnchoredDrawObj>
    return maAnchoredDrawObj.GetAnchorFrm();
}

SwFrm* SwDrawVirtObj::AnchorFrm()
{
    // OD 2004-03-29 #i26791# - use new member <maAnchoredDrawObj>
    return maAnchoredDrawObj.AnchorFrm();
}

void SwDrawVirtObj::SetPageFrm( SwPageFrm* _pNewPageFrm )
{
    // --> OD 2004-06-30 #i28701# - page frame is now stored at member <maAnchoredDrawObj>
    maAnchoredDrawObj.SetPageFrm( _pNewPageFrm );
}

SwPageFrm* SwDrawVirtObj::GetPageFrm()
{
    // --> OD 2004-06-30 #i28701# - page frame is now stored at member <maAnchoredDrawObj>
    return maAnchoredDrawObj.GetPageFrm();
}

void SwDrawVirtObj::RemoveFromWriterLayout()
{
    // remove contact object from frame for 'virtual' drawing object
    // OD 2004-03-29 #i26791# - use new member <maAnchoredDrawObj>
    if ( maAnchoredDrawObj.GetAnchorFrm() )
    {
        maAnchoredDrawObj.AnchorFrm()->RemoveDrawObj( maAnchoredDrawObj );
    }
}

// --------------------------------------------------------------------
// connection to writer layout: <AddToDrawingPage()>, <RemoveFromDrawingPage()>
// --------------------------------------------------------------------
void SwDrawVirtObj::AddToDrawingPage()
{
    // determine 'master'
    SdrObject* pOrgMasterSdrObj = mrDrawContact.GetMaster();

    // insert 'virtual' drawing object into page, set layer and user call.
    SdrPage *pPg;
    if ( 0 != ( pPg = pOrgMasterSdrObj->GetPage() ) )
    {
        pPg->InsertObject( this );
    }
    SetUserCall( &mrDrawContact );
}

void SwDrawVirtObj::RemoveFromDrawingPage()
{
    SetUserCall( 0 );
    if ( GetPage() )
    {
        GetPage()->RemoveObject( GetOrdNum() );
    }
}

// is 'virtual' drawing object connected to writer layout and  to drawing layer.
bool SwDrawVirtObj::IsConnected() const
{
    bool bRetVal = GetAnchorFrm() &&
                   ( GetPage() && GetUserCall() );

    return bRetVal;
}

void SwDrawVirtObj::NbcSetAnchorPos(const Point& rPnt)
{
    SdrObject::NbcSetAnchorPos( rPnt );
}

const Rectangle& SwDrawVirtObj::GetCurrentBoundRect() const
{
    if (bBoundRectDirty)
    {
        const_cast<SwDrawVirtObj*>(this)->RecalcBoundRect();
        const_cast<SwDrawVirtObj*>(this)->bBoundRectDirty = FALSE;
    }
    return aOutRect;
}

const Rectangle& SwDrawVirtObj::GetLastBoundRect() const
{
    return aOutRect;
}

void SwDrawVirtObj::RecalcBoundRect()
{
    // OD 2004-04-05 #i26791# - switch order of calling <GetOffset()> and
    // <ReferencedObj().GetCurrentBoundRect()>, because <GetOffset()> calculates
    // its value by the 'BoundRect' of the referenced object.
    //aOutRect = rRefObj.GetCurrentBoundRect();
    //aOutRect += GetOffset();
    Point aOffset = GetOffset();
    aOutRect = ReferencedObj().GetCurrentBoundRect() + aOffset;
}

sal_Bool SwDrawVirtObj::DoPaintObject( ExtOutputDevice& rOut,
                                       const SdrPaintInfoRec& rInfoRec ) const

{
    sal_Bool bRetval;
    const SdrObject& rReferencedObject = GetReferencedObj();

    // rescue offset and set new one
    const Point aOfs(rOut.GetOffset());
    const Point aLocalOffset(GetOffset());
    rOut.SetOffset(aOfs + aLocalOffset);
    if ( rReferencedObject.ISA(SdrObjGroup) )
    {

        // If it's a group object, paint the whole group content
        // using a temporary ObjectContactOfObjListPainter
        sdr::contact::SdrObjectVector aObjectVector;
        aObjectVector.push_back(&((SdrObject&)rReferencedObject));

        sdr::contact::ObjectContactOfObjListPainter aPainter(aObjectVector);
        sdr::contact::DisplayInfo aDisplayInfo;
        SdrPaintInfoRec aCopyInfoRec(rInfoRec);
        aCopyInfoRec.aCheckRect.Move( -aLocalOffset.X(), -aLocalOffset.Y() );
        aCopyInfoRec.aDirtyRect.Move( -aLocalOffset.X(), -aLocalOffset.Y() );
        aDisplayInfo.SetExtendedOutputDevice(&rOut);
        aDisplayInfo.SetPaintInfoRec(&aCopyInfoRec);
        aDisplayInfo.SetOutputDevice(rOut.GetOutDev());

        // keep draw hierarchy up-to-date
        aPainter.PreProcessDisplay(aDisplayInfo);

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);

        // prepare delete
        aPainter.PrepareDelete();
    }
    else
    {
        // If it's a single object, paint it
        bRetval = rRefObj.DoPaintObject(rOut, rInfoRec);
    }

    // restore offset
    rOut.SetOffset(aOfs);

    return bRetval;
}

SdrObject* SwDrawVirtObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    Point aPnt(rPnt - GetOffset());
    FASTBOOL bRet = rRefObj.CheckHit(aPnt, nTol, pVisiLayer) != NULL;

    return bRet ? (SdrObject*)this : NULL;
}

void SwDrawVirtObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    rRefObj.TakeXorPoly(rPoly, bDetail);
    rPoly.Move(GetOffset().X(), GetOffset().Y());
}

void SwDrawVirtObj::TakeContour(XPolyPolygon& rPoly) const
{
    rRefObj.TakeContour(rPoly);
    rPoly.Move(GetOffset().X(), GetOffset().Y());
}

SdrHdl* SwDrawVirtObj::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pHdl = rRefObj.GetHdl(nHdlNum);
    Point aP(pHdl->GetPos() + GetOffset());
    pHdl->SetPos(aP);

    return pHdl;
}

SdrHdl* SwDrawVirtObj::GetPlusHdl(const SdrHdl& rHdl, USHORT nPlNum) const
{
    SdrHdl* pHdl = rRefObj.GetPlusHdl(rHdl, nPlNum);
    pHdl->SetPos(pHdl->GetPos() + GetOffset());

    return pHdl;
}

void SwDrawVirtObj::NbcMove(const Size& rSiz)
{
    SdrObject::NbcMove( rSiz );
}

void SwDrawVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    rRefObj.NbcResize(rRef - GetOffset(), xFact, yFact);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    rRefObj.NbcRotate(rRef - GetOffset(), nWink, sn, cs);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    rRefObj.NbcMirror(rRef1 - GetOffset(), rRef2 - GetOffset());
    SetRectsDirty();
}

void SwDrawVirtObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    rRefObj.NbcShear(rRef - GetOffset(), nWink, tn, bVShear);
    SetRectsDirty();
}

void SwDrawVirtObj::Move(const Size& rSiz)
{
    SdrObject::Move( rSiz );
//    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
//    rRefObj.Move( rSiz );
//    SetRectsDirty();
//    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if(xFact.GetNumerator() != xFact.GetDenominator() || yFact.GetNumerator() != yFact.GetDenominator())
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Resize(rRef - GetOffset(), xFact, yFact);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if(nWink)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Rotate(rRef - GetOffset(), nWink, sn, cs);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::Mirror(const Point& rRef1, const Point& rRef2)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
    rRefObj.Mirror(rRef1 - GetOffset(), rRef2 - GetOffset());
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    if(nWink)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Shear(rRef - GetOffset(), nWink, tn, bVShear);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::RecalcSnapRect()
{
    aSnapRect = rRefObj.GetSnapRect();
    aSnapRect += GetOffset();
}

const Rectangle& SwDrawVirtObj::GetSnapRect() const
{
    ((SwDrawVirtObj*)this)->aSnapRect = rRefObj.GetSnapRect();
    ((SwDrawVirtObj*)this)->aSnapRect += GetOffset();

    return aSnapRect;
}

void SwDrawVirtObj::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
    Rectangle aR(rRect);
    aR -= GetOffset();
    rRefObj.SetSnapRect(aR);
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aR(rRect);
    aR -= GetOffset();
    SetRectsDirty();
    rRefObj.NbcSetSnapRect(aR);
}

const Rectangle& SwDrawVirtObj::GetLogicRect() const
{
    ((SwDrawVirtObj*)this)->aSnapRect = rRefObj.GetLogicRect();
    ((SwDrawVirtObj*)this)->aSnapRect += GetOffset();

    return aSnapRect;
}

void SwDrawVirtObj::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
    Rectangle aR(rRect);
    aR -= GetOffset();
    rRefObj.SetLogicRect(aR);
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::NbcSetLogicRect(const Rectangle& rRect)
{
    Rectangle aR(rRect);
    aR -= GetOffset();
    rRefObj.NbcSetLogicRect(aR);
    SetRectsDirty();
}

Point SwDrawVirtObj::GetSnapPoint(USHORT i) const
{
    Point aP(rRefObj.GetSnapPoint(i));
    aP += GetOffset();

    return aP;
}

const Point& SwDrawVirtObj::GetPoint(USHORT i) const
{
    ((SwDrawVirtObj*)this)->aHack = rRefObj.GetPoint(i);
    ((SwDrawVirtObj*)this)->aHack += GetOffset();

    return aHack;
}

void SwDrawVirtObj::NbcSetPoint(const Point& rPnt, USHORT i)
{
    Point aP(rPnt);
    aP -= GetOffset();
    rRefObj.SetPoint(aP, i);
    SetRectsDirty();
}

// #108784#
FASTBOOL SwDrawVirtObj::HasTextEdit() const
{
    return rRefObj.HasTextEdit();
}

// OD 18.06.2003 #108784# - overloaded 'layer' methods for 'virtual' drawing
// object to assure, that layer of 'virtual' object is the layer of the referenced
// object.
SdrLayerID SwDrawVirtObj::GetLayer() const
{
    return GetReferencedObj().GetLayer();
}

void SwDrawVirtObj::NbcSetLayer(SdrLayerID nLayer)
{
    ReferencedObj().NbcSetLayer( nLayer );
    SdrVirtObj::NbcSetLayer( ReferencedObj().GetLayer() );
}

void SwDrawVirtObj::SetLayer(SdrLayerID nLayer)
{
    ReferencedObj().SetLayer( nLayer );
    SdrVirtObj::NbcSetLayer( ReferencedObj().GetLayer() );
}
// eof
