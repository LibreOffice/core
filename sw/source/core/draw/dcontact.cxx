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
#include "hintids.hxx"
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <fmtornt.hxx>
#include <viewimp.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <node.hxx>
#include <fmtcntnt.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>  // Notify_Background
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <unodraw.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <doc.hxx>
#include <hints.hxx>
#include <txtfrm.hxx>
#include <editsh.hxx>
#include <docary.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <svx/svdograf.hxx>
#include <svx/svddrgv.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdetc.hxx>
#include <switerator.hxx>
#include <algorithm>
#include <drawdoc.hxx>

using namespace ::com::sun::star;

void setContextWritingMode( SdrObject* pObj, SwFrm* pAnchor )
{
    if( pObj && pAnchor )
    {
        short nWritingDirection = text::WritingMode2::LR_TB;
        if( pAnchor->IsVertical() )
        {
            nWritingDirection = text::WritingMode2::TB_RL;
        } else if( pAnchor->IsRightToLeft() )
        {
            nWritingDirection = text::WritingMode2::RL_TB;
        }
        pObj->SetContextWritingMode( nWritingDirection );
    }
}


//Der Umgekehrte Weg: Sucht das Format zum angegebenen Objekt.
//Wenn das Object ein SwVirtFlyDrawObj ist so wird das Format von
//selbigem besorgt.
//Anderfalls ist es eben ein einfaches Zeichenobjekt. Diese hat einen
//UserCall und der ist Client vom gesuchten Format.

SwFrmFmt *FindFrmFmt( SdrObject *pObj )
{
    SwFrmFmt* pRetval = 0L;

    if ( dynamic_cast< SwVirtFlyDrawObj* >(pObj) )
    {
       pRetval = ((SwVirtFlyDrawObj*)pObj)->GetFmt();
    }
    else
    {
        SwDrawContact* pContact = static_cast<SwDrawContact*>(findConnectionToSdrObject( pObj ));
        if ( pContact )
        {
            pRetval = pContact->GetFmt();
        }
    }
/* SJ: after prior consultation with OD we decided to remove this Assertion
#if OSL_DEBUG_LEVEL > 1
    ASSERT( pRetval,
            "<::FindFrmFmt(..)> - no frame format found for given object. Please inform OD." );
#endif
*/
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

// --> OD 2006-08-15 #i68520# - change naming
SwRect GetBoundRectOfAnchoredObj( const SdrObject* pObj )
// <--
{
    SwRect aRet( sdr::legacy::GetBoundRect(*pObj) );
    // --> OD 2006-08-10 #i68520# - call cache of <SwAnchoredObject>
    SwContact* pContact( findConnectionToSdrObject( pObj ) );
    if ( pContact )
    {
        const SwAnchoredObject* pAnchoredObj( pContact->GetAnchoredObj( pObj ) );
        if ( pAnchoredObj )
        {
            aRet = pAnchoredObj->GetObjRectWithSpaces();
        }
    }
    // <--
    return aRet;
}

//////////////////////////////////////////////////////////////////////////////
// detect and hand back the connected SwContact for a SdrObject

SwContact* findConnectionToSdrObjectDirect(const SdrObject* pSdrObject)
{
    if(pSdrObject)
    {
        if(pSdrObject->HasListeners())
        {
            const sal_uInt16 nListenerCount(pSdrObject->GetListenerCount());

            for(sal_uInt16 a(0); a < nListenerCount; a++)
            {
                SfxListener* pCandidate = pSdrObject->GetListener(a);

                if(pCandidate) // not all slots in a broadcaster have to be used
                {
                    SwContact* pRetval = dynamic_cast< SwContact* >(pCandidate);

                    if(pRetval)
                    {
                        return pRetval;
                    }
                }
            }
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// same as findConnectionToSdrObjectDirect, but with traveling up the object hierarchy
// to get the connection for the outmost containing GroupObject

SwContact* findConnectionToSdrObject( const SdrObject* pObj )
{
    while(pObj)
    {
        SwContact* pRetval = findConnectionToSdrObjectDirect(pObj);

        if(pRetval)
        {
            return pRetval;
        }

        pObj = pObj->GetParentSdrObject();
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// connect to given SdrObject. All existing connections are deleted, a single
// new one gets established

void establishConnectionToSdrObject(SdrObject* pSdrObject, SwContact* pSwContact)
{
    if(pSdrObject)
    {
        // remove all SwContacts
        resetConnectionToSdrObject(pSdrObject);

        if(pSwContact)
        {
            pSwContact->StartListening(*pSdrObject);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// all connections (should be a single one) to the SdrObject are removed

void resetConnectionToSdrObject(SdrObject* pSdrObject)
{
    if(pSdrObject)
    {
        SwContact* pFound = findConnectionToSdrObjectDirect(pSdrObject);

        while(pFound)
        {
            pFound->EndListening(*pSdrObject);
            pFound = findConnectionToSdrObjectDirect(pSdrObject);
        }
    }
}

// liefert sal_True falls das SrdObject ein Marquee-Object (Lauftext) ist
sal_Bool IsMarqueeTextObj( const SdrObject& rObj )
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
bool SwContact::IsInDTOR() const
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
    // --> OD 2005-06-08 #i46297# - notify background about the arriving of
    // the object and invalidate its position.
    const bool bNotify( !GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) );
    // <--

    _MoveObjToLayer( true, _pDrawObj );

    // --> OD 2005-05-23 #i46297#
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        ASSERT( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        if ( pAnchoredObj )
        {
            ::setContextWritingMode( _pDrawObj, pAnchoredObj->GetAnchorFrmContainingAnchPos() );
            // Note: as-character anchored objects aren't registered at a page frame and
            //       a notification of its background isn't needed.
            if ( pAnchoredObj->GetPageFrm() )
            {
                ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrm(),
                                     pAnchoredObj->GetObjRect(), PREP_FLY_ARRIVE, sal_True );
            }

            pAnchoredObj->InvalidateObjPos();
        }
    }
    // <--
}

/** method to move drawing object to corresponding invisible layer

    OD 21.08.2003 #i18447#

    @author OD
*/
void SwContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    // --> OD 2005-06-08 #i46297# - notify background about the leaving of the object.
    const bool bNotify( GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) );
    // <--

    _MoveObjToLayer( false, _pDrawObj );

    // --> OD 2005-05-19 #i46297#
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        ASSERT( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        // Note: as-character anchored objects aren't registered at a page frame and
        //       a notification of its background isn't needed.
        if ( pAnchoredObj && pAnchoredObj->GetPageFrm() )
        {
            ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrm(),
                                 pAnchoredObj->GetObjRect(), PREP_FLY_LEAVE, sal_True );
        }
    }
    // <--
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

    if ( !GetRegisteredIn() )
    {
        ASSERT( false, "SwDrawContact::_MoveObjToLayer(..) - no drawing frame format!" );
        return;
    }

    const IDocumentDrawModelAccess* pIDDMA = static_cast<SwFrmFmt*>(GetRegisteredInNonConst())->getIDocumentDrawModelAccess();
    if ( !pIDDMA )
    {
        ASSERT( false, "SwDrawContact::_MoveObjToLayer(..) - no writer document!" );
        return;
    }

    SdrLayerID nToHellLayerId =
        _bToVisible ? pIDDMA->GetHellId() : pIDDMA->GetInvisibleHellId();
    SdrLayerID nToHeavenLayerId =
        _bToVisible ? pIDDMA->GetHeavenId() : pIDDMA->GetInvisibleHeavenId();
    SdrLayerID nToControlLayerId =
        _bToVisible ? pIDDMA->GetControlsId() : pIDDMA->GetInvisibleControlsId();
    SdrLayerID nFromHellLayerId =
        _bToVisible ? pIDDMA->GetInvisibleHellId() : pIDDMA->GetHellId();
    SdrLayerID nFromHeavenLayerId =
        _bToVisible ? pIDDMA->GetInvisibleHeavenId() : pIDDMA->GetHeavenId();
    SdrLayerID nFromControlLayerId =
        _bToVisible ? pIDDMA->GetInvisibleControlsId() : pIDDMA->GetControlsId();

    if ( dynamic_cast< SdrObjGroup* >(_pDrawObj) )
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
            else if ( _pDrawObj->GetLayer() == pIDDMA->GetHeavenId() ||
                      _pDrawObj->GetLayer() == pIDDMA->GetInvisibleHeavenId() )
            {
                // it has to be the heaven layer, if method <GetLayer()> reveals
                // a heaven layer
                nNewLayerId = nToHeavenLayerId;
            }
            // set layer at group object, but do *not* broadcast and
            // no propagation to the members.
            // Thus, call <SetLayer(..)> at super class
            _pDrawObj->SdrObject::SetLayer( nNewLayerId );
        }

        // call method recursively for group object members
        const SdrObjList* pLst = _pDrawObj->getChildrenOfSdrObject();
        if ( pLst )
        {
            for ( sal_uInt32 i = 0; i < pLst->GetObjCount(); ++i )
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

const SwIndex& SwContact::GetCntntAnchorIndex() const
{
    return GetCntntAnchor().nContent;
}

/** get minimum order number of anchored objects handled by with contact

    OD 2004-08-24 #110810#

    @author
*/
sal_uInt32 SwContact::GetMinOrdNum() const
{
    sal_uInt32 nMinOrdNum( SAL_MAX_UINT32 );

    std::list< SwAnchoredObject* > aObjs;
    GetAnchoredObjs( aObjs );

    while ( !aObjs.empty() )
    {
        sal_uInt32 nTmpOrdNum = aObjs.back()->GetDrawObj()->GetNavigationPosition();

        if ( nTmpOrdNum < nMinOrdNum )
        {
            nMinOrdNum = nTmpOrdNum;
        }

        aObjs.pop_back();
    }

    ASSERT( nMinOrdNum != SAL_MAX_UINT32,
            "<SwContact::GetMinOrdNum()> - no order number found." );
    return nMinOrdNum;
}

/** get maximum order number of anchored objects handled by with contact

    OD 2004-08-24 #110810#

    @author
*/
sal_uInt32 SwContact::GetMaxOrdNum() const
{
    sal_uInt32 nMaxOrdNum( 0L );

    std::list< SwAnchoredObject* > aObjs;
    GetAnchoredObjs( aObjs );

    while ( !aObjs.empty() )
    {
        sal_uInt32 nTmpOrdNum = aObjs.back()->GetDrawObj()->GetNavigationPosition();

        if ( nTmpOrdNum > nMaxOrdNum )
        {
            nMaxOrdNum = nTmpOrdNum;
        }

        aObjs.pop_back();
    }

    return nMaxOrdNum;
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

SwFlyDrawContact::SwFlyDrawContact( SwFlyFrmFmt *pToRegisterIn, SdrModel& rTargetModel ) :
    SwContact( pToRegisterIn )
{
    // OD 2004-04-01 #i26791# - class <SwFlyDrawContact> contains the 'master'
    // drawing object of type <SwFlyDrawObj> on its own.
    mpMasterObj = new SwFlyDrawObj(rTargetModel);
    // TTTT: makes no sense, object is not added to a SdrObjList: mpMasterObj->SetOrdNum( 0xFFFFFFFE );
    establishConnectionToSdrObject(mpMasterObj, this);
}

SwFlyDrawContact::~SwFlyDrawContact()
{
    if ( mpMasterObj )
    {
        resetConnectionToSdrObject(mpMasterObj);
        if ( mpMasterObj->getParentOfSdrObject() )
            mpMasterObj->getParentOfSdrObject()->RemoveObjectFromSdrObjList( mpMasterObj->GetNavigationPosition() );
        deleteSdrObjectSafeAndClearPointer(mpMasterObj);
    }
}

// OD 2004-03-29 #i26791#
const SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    ASSERT( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( dynamic_cast< const SwVirtFlyDrawObj* >(_pSdrObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    ASSERT( findConnectionToSdrObject( _pSdrObj ) == const_cast<SwFlyDrawContact*>(this),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj && dynamic_cast< const SwVirtFlyDrawObj* >(_pSdrObj) )
    {
        pRetAnchoredObj = static_cast<const SwVirtFlyDrawObj*>(_pSdrObj)->GetFlyFrm();
    }

    return pRetAnchoredObj;
}

SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( SdrObject* _pSdrObj )
{
    ASSERT( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    ASSERT( dynamic_cast< SwVirtFlyDrawObj* >(_pSdrObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type provided" );
    ASSERT( findConnectionToSdrObject( _pSdrObj ) == this,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj && dynamic_cast< SwVirtFlyDrawObj* >(_pSdrObj) )
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
    ASSERT( dynamic_cast< SwFlyDrawObj* >(_pNewMaster),
            "<SwFlyDrawContact::SetMaster(..)> - wrong type of new master object" );
    mpMasterObj = static_cast<SwFlyDrawObj *>(_pNewMaster);
}

/*************************************************************************
|*
|*  SwFlyDrawContact::Modify()
|*
|*  Ersterstellung      OK 08.11.94 10:21
|*  Letzte Aenderung    MA 06. Dec. 94
|*
|*************************************************************************/

void SwFlyDrawContact::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
}

// OD 2004-01-16 #110582# - override method to control Writer fly frames,
// which are linked, and to assure that all objects anchored at/inside the
// Writer fly frame are also made visible.
void SwFlyDrawContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    ASSERT( dynamic_cast< SwVirtFlyDrawObj* >(_pDrawObj),
            "<SwFlyDrawContact::MoveObjToVisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        // nothing to do
        return;
    }

    SwFlyFrm* pFlyFrm = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrm();

    // --> OD 2005-03-09 #i44464# - consider, that Writer fly frame content
    // already exists - (e.g. WW8 document is inserted into a existing document).
    if ( !pFlyFrm->Lower() )
    {
        pFlyFrm->InsertColumns();
        pFlyFrm->Chain( pFlyFrm->AnchorFrm() );
        pFlyFrm->InsertCnt();
    }
    if ( pFlyFrm->GetDrawObjs() )
    {
        for ( sal_uInt8 i = 0; i < pFlyFrm->GetDrawObjs()->Count(); ++i)
        {
            // --> OD 2004-07-01 #i28701# - consider type of objects in sorted object list.
            SdrObject* pObj = (*pFlyFrm->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(findConnectionToSdrObjectDirect(pObj));
            pContact->MoveObjToVisibleLayer( pObj );
        }
    }

    // make fly frame visible
    SwContact::MoveObjToVisibleLayer( _pDrawObj );
}

// OD 2004-01-16 #110582# - override method to control Writer fly frames,
// which are linked, and to assure that all objects anchored at/inside the
// Writer fly frame are also made invisible.
void SwFlyDrawContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    ASSERT( dynamic_cast< SwVirtFlyDrawObj* >(_pDrawObj),
            "<SwFlyDrawContact::MoveObjToInvisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( !GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
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
            SwContact* pContact = static_cast<SwContact*>(findConnectionToSdrObjectDirect(pObj));
            pContact->MoveObjToInvisibleLayer( pObj );
        }
    }

    // make fly frame invisible
    SwContact::MoveObjToInvisibleLayer( _pDrawObj );
}

/** get data collection of anchored objects, handled by with contact

    OD 2004-08-23 #110810#

    @author
*/
void SwFlyDrawContact::GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const
{
    const SwFrmFmt* pFmt = GetFmt();
    SwFlyFrm::GetAnchoredObjects( _roAnchoredObjs, *pFmt );
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
    if ( dynamic_cast< const SdrObjGroup* >(pObj) )
    {
        const SdrObjList *pLst = pObj->getChildrenOfSdrObject();
        for ( sal_uInt32 i = 0; i < pLst->GetObjCount(); ++i )
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
    mbDisconnectInProgress( false ),
    mbNotifyActive( false ),
    meEventTypeOfCurrentUserCall(HINT_OBJCHG_MOVE)
{
    // clear list containing 'virtual' drawing objects.
    maDrawVirtObjs.clear();

    // --> OD 2004-09-22 #i33909# - assure, that drawing object is inserted
    // in the drawing page.
    if ( !pObj->IsObjectInserted() )
    {
        pToRegisterIn->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
            InsertObjectToSdrObjList(*pObj, pObj->GetNavigationPosition() );
    }
    // <--

    //Controls muessen immer im Control-Layer liegen. Das gilt auch fuer
    //Gruppenobjekte, wenn diese Controls enthalten.
    if ( ::CheckControlLayer( pObj ) )
    {
        // OD 25.06.2003 #108784# - set layer of object to corresponding invisible layer.
        pObj->SetLayer( pToRegisterIn->getIDocumentDrawModelAccess()->GetInvisibleControlsId() );
    }

    // OD 2004-03-29 #i26791#
    establishConnectionToSdrObject(pObj, this);
    maAnchoredDrawObj.SetDrawObj( *pObj );

    // if there already exists an SwXShape for the object, ensure it knows about us, and the SdrObject
    // FS 2009-04-07 #i99056#
    SwXShape::AddExistingShapeToFmt( *pObj );
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
        SdrObject* pObject = const_cast< SdrObject* >( maAnchoredDrawObj.GetDrawObj() );
        deleteSdrObjectSafeAndClearPointer( pObject );
    }
}

void SwDrawContact::GetTextObjectsFromFmt( std::list<SdrTextObj*>& rTextObjects, SwDoc* pDoc )
{
    for( sal_Int32 n=0; n<pDoc->GetSpzFrmFmts()->Count(); n++ )
    {
        SwFrmFmt* pFly = (*pDoc->GetSpzFrmFmts())[n];
        if( dynamic_cast< SwDrawFrmFmt* >(pFly) )
        {
            SwDrawContact* pContact = SwIterator<SwDrawContact,SwFrmFmt>::FirstElement(*pFly);
            if( pContact )
            {
                SdrObject* pSdrO = pContact->GetMaster();

                if ( pSdrO )
                {
                    SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pSdrO);

                    if ( pSdrO->getChildrenOfSdrObject() )
                    {
                        SdrObjListIter aListIter( *pSdrO->getChildrenOfSdrObject(), IM_DEEPNOGROUPS );
                        //iterate inside of a grouped object
                        while( aListIter.IsMore() )
                        {
                            SdrTextObj* pSdrOElement = dynamic_cast< SdrTextObj* >(aListIter.Next());

                            if( pSdrOElement && pSdrOElement->HasText() )
                            {
                                rTextObjects.push_back(pSdrOElement);
                            }
                        }
                    }
                    else if( pSdrTextObj && pSdrTextObj->HasText() )
                    {
                        rTextObjects.push_back(pSdrTextObj);
                    }
                }
            }
        }
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
    ASSERT( findConnectionToSdrObject( _pSdrObj ) == const_cast<SwDrawContact*>(this) ||
            _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj )
    {
        if ( dynamic_cast< const SwDrawVirtObj* >(_pSdrObj) )
        {
            pRetAnchoredObj = static_cast<const SwDrawVirtObj*>(_pSdrObj)->GetAnchoredObj();
        }
        else
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
    ASSERT( findConnectionToSdrObject( _pSdrObj ) == this || _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    if ( _pSdrObj )
    {
        if ( dynamic_cast< SwDrawVirtObj* >(_pSdrObj) )
        {
            pRetAnchoredObj = static_cast<SwDrawVirtObj*>(_pSdrObj)->AnchoredObj();
        }
        else
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
         ( !findConnectionToSdrObjectDirect(_pDrawObj) &&
           findConnectionToSdrObject( _pDrawObj ) == static_cast<const SwContact* const>(this) ) )
    {
        pAnchorFrm = maAnchoredDrawObj.GetAnchorFrm();
    }
    else if ( dynamic_cast< const SwDrawVirtObj* >(_pDrawObj) )
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
         ( !findConnectionToSdrObjectDirect(_pDrawObj) &&
           findConnectionToSdrObject( _pDrawObj ) == this ) )
    {
        pAnchorFrm = maAnchoredDrawObj.AnchorFrm();
    }
    else
    {
        ASSERT( dynamic_cast< SwDrawVirtObj* >(_pDrawObj),
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
        deleteSdrObjectSafeAndClearPointer(_pVirtObj);
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
            // --> OD 2004-10-21 #i34640# - determine correct page frame
            SwPageFrm* pPage = pDrawVirtObj->AnchoredObj()->FindPageFrmOfAnchor();
            // <--
            if( pOldBoundRect && pPage )
            {
                SwRect aOldRect( *pOldBoundRect );
                const Point aPointOffset(basegfx::fround(pDrawVirtObj->GetOffset().getX()), basegfx::fround(pDrawVirtObj->GetOffset().getX()));
                aOldRect.Pos() += aPointOffset;
                if( aOldRect.HasArea() )
                    ::Notify_Background( pDrawVirtObj, pPage,
                                         aOldRect, PREP_FLY_LEAVE,sal_True);
            }
            // --> OD 2004-10-21 #i34640# - include spacing for wrapping
            SwRect aRect( pDrawVirtObj->GetAnchoredObj()->GetObjRectWithSpaces() );
            // <--
            if( aRect.HasArea() )
            {
                // --> OD 2004-10-21 #i34640# - simplify
                SwPageFrm* pPg = (SwPageFrm*)::FindPage( aRect, pPage );
                // <--
                if ( pPg )
                    ::Notify_Background( pDrawVirtObj, pPg, aRect,
                                         PREP_FLY_ARRIVE, sal_True );
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
    // --> OD 2004-10-21 #i34640#
    SwAnchoredObject* pAnchoredObj =
        const_cast<SwAnchoredObject*>(_rDrawContact.GetAnchoredObj( &_rObj ));
    if ( pAnchoredObj && pAnchoredObj->GetAnchorFrm() )
    // <--
    {
        // --> OD 2004-10-21 #i34640# - determine correct page frame
        SwPageFrm* pPageFrm = pAnchoredObj->FindPageFrmOfAnchor();
        // <--
        if( _pOldObjRect && pPageFrm )
        {
            SwRect aOldRect( *_pOldObjRect );
            if( aOldRect.HasArea() )
            {
                // --> OD 2004-10-21 #i34640# - determine correct page frame
                SwPageFrm* pOldPageFrm = (SwPageFrm*)::FindPage( aOldRect, pPageFrm );
                // <--
                ::Notify_Background( &_rObj, pOldPageFrm, aOldRect,
                                     PREP_FLY_LEAVE, sal_True);
            }
        }
        // --> OD 2004-10-21 #i34640# - include spacing for wrapping
        SwRect aNewRect( pAnchoredObj->GetObjRectWithSpaces() );
        // <--
        if( aNewRect.HasArea() && pPageFrm )
        {
            pPageFrm = (SwPageFrm*)::FindPage( aNewRect, pPageFrm );
            ::Notify_Background( &_rObj, pPageFrm, aNewRect,
                                 PREP_FLY_ARRIVE, sal_True );
        }
        ClrContourCache( &_rObj );
    }
}

void SwDrawContact::HandleChanged(
    const SdrObject& rObj,
    SdrHintKind eHint)
{
    // OD 2004-06-01 #i26791# - no event handling, if existing <ViewShell>
    // is in construction
    SwDoc* pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetCurrentViewShell() &&
         pDoc->GetCurrentViewShell()->IsInConstructor() )
    {
        return;
    }

    // --> OD 2005-03-08 #i44339#
    // no event handling, if document is in destruction.
    // Exception: It's the HINT_SDROBJECTDYING event
    if ( pDoc->IsInDtor() && eHint != HINT_SDROBJECTDYING )
    {
        return;
    }
    // <--

    //Action aufsetzen, aber nicht wenn gerade irgendwo eine Action laeuft.
    ViewShell *pSh = 0, *pOrg;
    SwRootFrm *pTmpRoot = pDoc->GetCurrentLayout();//swmod 080317
    if ( pTmpRoot && pTmpRoot->IsCallbackActionEnabled() )
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
            pTmpRoot->StartAllAction();
    }

    const SdrObjGroup* pGroup = 0;

    if(rObj.getParentOfSdrObject())
    {
        pGroup = dynamic_cast< const SdrObjGroup* >(rObj.getParentOfSdrObject()->getSdrObjectFromSdrObjList());
    }

    _Changed( rObj, eHint, false );    //Achtung, ggf. Suizid!

    while(pGroup)
    {
        if(findConnectionToSdrObjectDirect(pGroup) && findConnectionToSdrObjectDirect(pGroup) == this)
        {
            _Changed( *pGroup, eHint, true );
        }

        SdrObjGroup* pUpGroup = 0;

        if(pGroup->getParentOfSdrObject())
        {
            pUpGroup = dynamic_cast< SdrObjGroup* >(pGroup->getParentOfSdrObject()->getSdrObjectFromSdrObjList());

            if(pGroup == pUpGroup)
            {
                pUpGroup = 0;
            }
        }

        pGroup = pUpGroup;
    }

    if ( pSh )
        pTmpRoot->EndAllAction();
}

void SwDrawContact::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint)
    {
        const SdrObject* pObj = pSdrBaseHint->GetSdrHintObject();

        if(pObj)
        {
            HandleChanged(*pObj, pSdrBaseHint->GetSdrHintKind());
        }
    }
}

// --> OD 2006-01-18 #129959#
// helper class for method <SwDrawContact::_Changed(..)> for handling nested
// <SfxBroadcaster/Listener/Notify> events
class NestedUserCallHdl
{
    private:
        SwDrawContact* mpDrawContact;
        bool mbParentUserCallActive;
        SdrHintKind meParentUserCallEventType;

    public:
        NestedUserCallHdl( SwDrawContact* _pDrawContact,
                           SdrHintKind _eEventType )
            : mpDrawContact( _pDrawContact ),
              mbParentUserCallActive( _pDrawContact->mbNotifyActive ),
              meParentUserCallEventType( _pDrawContact->meEventTypeOfCurrentUserCall )
        {
            mpDrawContact->mbNotifyActive = true;
            mpDrawContact->meEventTypeOfCurrentUserCall = _eEventType;
        }

        ~NestedUserCallHdl()
        {
            if ( mpDrawContact )
            {
                mpDrawContact->mbNotifyActive = mbParentUserCallActive;
                mpDrawContact->meEventTypeOfCurrentUserCall = meParentUserCallEventType;
            }
        }

        void DrawContactDeleted()
        {
            mpDrawContact = 0;
        }

        bool IsNestedUserCall()
        {
            return mbParentUserCallActive;
        }

        void AssertNestedUserCall()
        {
            if ( IsNestedUserCall() )
            {
                bool bTmpAssert( true );
                // Currently its known, that a nested event HINT_OBJCHG_RESIZE
                // could occur during parent user call HINT_OBJINSERTED,
                // HINT_SDROBJECTDYING and HINT_OBJCHG_RESIZE for edge objects.
                // Thus, assert all other combinations
                if ( ( meParentUserCallEventType == HINT_OBJINSERTED ||
                       meParentUserCallEventType == HINT_SDROBJECTDYING ||
                       meParentUserCallEventType == HINT_OBJCHG_RESIZE) &&
                     mpDrawContact->meEventTypeOfCurrentUserCall == HINT_OBJCHG_RESIZE)
                {
                    bTmpAssert = false;
                }

                if ( bTmpAssert )
                {
                    ASSERT( false,
                            "<SwDrawContact::_Changed(..)> - unknown nested <UserCall> event. This is serious, please inform OD." );
                }
            }
        }
};

// <--
//
// !!!ACHTUNG!!! The object may commit suicide!!!
//
void SwDrawContact::_Changed(
    const SdrObject& rObj,
    SdrHintKind eHint,
    bool bGroupHierarchy)
{
    // --> OD 2006-01-18 #129959#
    // suppress handling of nested <SfxBroadcaster/Listener/Notify> events
    NestedUserCallHdl aNestedUserCallHdl( this, eHint );
    if ( aNestedUserCallHdl.IsNestedUserCall() )
    {
        aNestedUserCallHdl.AssertNestedUserCall();
        return;
    }
    // <--
    // OD 05.08.2002 #100843# - do *not* notify, if document is destructing
    // --> OD 2004-10-21 #i35912# - do *not* notify for as-character anchored
    // drawing objects.
    // --> OD 2004-11-11 #i35007#
    // improvement: determine as-character anchored object flag only once.
    const bool bAnchoredAsChar = ObjAnchoredAsChar();
    // <--
    const bool bNotify = !(GetFmt()->GetDoc()->IsInDtor()) &&
                         ( SURROUND_THROUGHT != GetFmt()->GetSurround().GetSurround() ) &&
                         !bAnchoredAsChar;
    const SwAnchoredDrawObject* pAnchoredDrawObj =
        static_cast<const SwAnchoredDrawObject*>( GetAnchoredObj( &rObj ) );
    const Rectangle aOldObjRect = pAnchoredDrawObj ? pAnchoredDrawObj->GetLastObjRect() : Rectangle();

    if(bGroupHierarchy
        || HINT_OBJCHG_MOVE == eHint
        || HINT_OBJCHG_RESIZE == eHint)
    {
        // OD 2004-04-06 #i26791# - adjust positioning and alignment attributes,
        // if positioning of drawing object isn't in progress.
        // --> OD 2005-08-15 #i53320# - no adjust of positioning attributes,
        // if drawing object isn't positioned.
        if ( !pAnchoredDrawObj->IsPositioningInProgress() &&
                !pAnchoredDrawObj->NotYetPositioned() )
        // <--
        {
            // --> OD 2008-02-18 #i79400#
            // always invalidate object rectangle inclusive spaces
            pAnchoredDrawObj->InvalidateObjRectWithSpaces();
            // <--
            // --> OD 2005-01-28 #i41324# - notify background before
            // adjusting position
            if ( bNotify )
            {
                // --> OD 2004-07-20 #i31573# - correction: Only invalidate
                // background of given drawing object.
                lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect );
            }
            // <--
            // --> OD 2004-08-04 #i31698# - determine layout direction
            // via draw frame format.
            SwFrmFmt::tLayoutDir eLayoutDir =
                            pAnchoredDrawObj->GetFrmFmt().GetLayoutDir();
            // <--
            // use geometry of drawing object
            Rectangle aObjRect(sdr::legacy::GetSnapRect(rObj));
            // If drawing object is a member of a group, the adjustment
            // of the positioning and the alignment attributes has to
            // be done for the top group object.
            if ( rObj.GetParentSdrObject() )
            {
                const SdrObject* pGroupObj = rObj.GetParentSdrObject();
                while ( pGroupObj->GetParentSdrObject() )
                {
                    pGroupObj = pGroupObj->GetParentSdrObject();
                }
                // use geometry of drawing object
                aObjRect = sdr::legacy::GetSnapRect(*pGroupObj);
            }
            SwTwips nXPosDiff(0L);
            SwTwips nYPosDiff(0L);
            switch ( eLayoutDir )
            {
                case SwFrmFmt::HORI_L2R:
                {
                    nXPosDiff = aObjRect.Left() - aOldObjRect.Left();
                    nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                }
                break;
                case SwFrmFmt::HORI_R2L:
                {
                    nXPosDiff = aOldObjRect.Right() - aObjRect.Right();
                    nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                }
                break;
                case SwFrmFmt::VERT_R2L:
                {
                    nXPosDiff = aObjRect.Top() - aOldObjRect.Top();
                    nYPosDiff = aOldObjRect.Right() - aObjRect.Right();
                }
                break;
                default:
                {
                    ASSERT( false,
                            "<SwDrawContact::_Changed(..)> - unsupported layout direction" );
                }
            }
            SfxItemSet aSet( GetFmt()->GetDoc()->GetAttrPool(),
                                RES_VERT_ORIENT, RES_HORI_ORIENT, 0 );
            const SwFmtVertOrient& rVert = GetFmt()->GetVertOrient();
            if ( nYPosDiff != 0 )
            {

                if ( rVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                        rVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                {
                    nYPosDiff = -nYPosDiff;
                }
                aSet.Put( SwFmtVertOrient( rVert.GetPos()+nYPosDiff,
                                            text::VertOrientation::NONE,
                                            rVert.GetRelationOrient() ) );
            }

            const SwFmtHoriOrient& rHori = GetFmt()->GetHoriOrient();
            if ( !bAnchoredAsChar && nXPosDiff != 0 )
            {
                aSet.Put( SwFmtHoriOrient( rHori.GetPos()+nXPosDiff,
                                            text::HoriOrientation::NONE,
                                            rHori.GetRelationOrient() ) );
            }

            if ( nYPosDiff ||
                    ( !bAnchoredAsChar && nXPosDiff != 0 ) )
            {
                GetFmt()->GetDoc()->SetFlyFrmAttr( *(GetFmt()), aSet );
                // keep new object rectangle, to avoid multiple
                // changes of the attributes by multiple event from
                // the drawing layer - e.g. group objects and its members
                // --> OD 2004-09-29 #i34748# - use new method
                // <SwAnchoredDrawObject::SetLastObjRect(..)>.
                const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)->SetLastObjRect(aObjRect);
            }
            else if(aObjRect.GetSize() != aOldObjRect.GetSize())
            {
                _InvalidateObjs();
                // --> OD 2004-11-11 #i35007# - notify anchor frame
                // of as-character anchored object
                {
                    //-->Modified for i119654,2012.6.8
                    SwFrm* pAnchorFrame = pAnchoredDrawObj
                                          ? const_cast<SwAnchoredDrawObject*>( pAnchoredDrawObj )->AnchorFrm()
                                          : NULL;
                    if ( pAnchorFrame )
                    {
                        pAnchorFrame->Prepare( PREP_FLY_ATTR_CHG, GetFmt() );
                    }
                    //<--
                }
            }
        }
    }
    else
    {
        switch(eHint)
        {
            case HINT_SDROBJECTDYING:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect );
                    // --> OD 2004-10-27 #i36181# - background of 'virtual'
                    // drawing objects have also been notified.
                    NotifyBackgrdOfAllVirtObjs( &aOldObjRect );
                    // <--
                }
                DisconnectFromLayout( false );
                SetMaster( NULL );
                delete this;
                // --> FME 2006-07-12 #i65784# Prevent memory corruption
                aNestedUserCallHdl.DrawContactDeleted();
                // <--
                break;
            }
            case HINT_OBJINSERTED:
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
                        lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect);
                    }
                }
                break;
            }
            case HINT_OBJREMOVED:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj(*this, rObj, &aOldObjRect);
                }
                DisconnectFromLayout( false );
                break;
            }
            case HINT_OBJCHG_ATTR:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj(*this, rObj, &aOldObjRect);
                }
                break;
            }
            default:
                break;
        }
    }
}

namespace
{
    static const SwFmtAnchor* lcl_getAnchorFmt( const SfxPoolItem& _rItem )
    {
        sal_uInt16 nWhich = _rItem.Which();
        const SwFmtAnchor* pAnchorFmt = NULL;
        if ( RES_ATTRSET_CHG == nWhich )
        {
            static_cast<const SwAttrSetChg&>(_rItem).GetChgSet()->
                GetItemState( RES_ANCHOR, sal_False, (const SfxPoolItem**)&pAnchorFmt );
        }
        else if ( RES_ANCHOR == nWhich )
        {
            pAnchorFmt = &static_cast<const SwFmtAnchor&>(_rItem);
        }
        return pAnchorFmt;
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

void SwDrawContact::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    // OD 10.10.2003 #112299#
    ASSERT( !mbDisconnectInProgress,
            "<SwDrawContact::Modify(..)> called during disconnection.");

    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor* pNewAnchorFmt = pNew ? lcl_getAnchorFmt( *pNew ) : NULL;

    if ( pNewAnchorFmt )
    {
        // JP 10.04.95: nicht auf ein Reset Anchor reagieren !!!!!
        if ( SFX_ITEM_SET ==
                GetFmt()->GetAttrSet().GetItemState( RES_ANCHOR, sal_False ) )
        {
            // OD 10.10.2003 #112299# - no connect to layout during disconnection
            if ( !mbDisconnectInProgress )
            {
                // determine old object retangle of 'master' drawing object
                // for notification
                const Rectangle* pOldRect = 0L;
                Rectangle aOldRect;
                if ( GetAnchorFrm() )
                {
                    // --> OD 2004-10-27 #i36181# - include spacing in object
                    // rectangle for notification.
                    aOldRect = maAnchoredDrawObj.GetObjRectWithSpaces().SVRect();
                    pOldRect = &aOldRect;
                    // <--
                }
                // re-connect to layout due to anchor format change
                ConnectToLayout( pNewAnchorFmt );
                // notify background of drawing objects
                lcl_NotifyBackgroundOfObj( *this, *GetMaster(), pOldRect );
                NotifyBackgrdOfAllVirtObjs( pOldRect );

                const SwFmtAnchor* pOldAnchorFmt = pOld ? lcl_getAnchorFmt( *pOld ) : NULL;
                if ( !pOldAnchorFmt || ( pOldAnchorFmt->GetAnchorId() != pNewAnchorFmt->GetAnchorId() ) )
                {
                    ASSERT( maAnchoredDrawObj.DrawObj(), "SwDrawContact::Modify: no draw object here?" );
                    if ( maAnchoredDrawObj.DrawObj() )
                    {
                        // --> OD 2009-07-10 #i102752#
                        // assure that a ShapePropertyChangeNotifier exists
                        maAnchoredDrawObj.DrawObj()->notifyShapePropertyChange( ::svx::eTextShapeAnchorType );
                        // <--
                    }
                }
            }
        }
        else
            DisconnectFromLayout();
    }
    // --> OD 2006-03-17 #i62875# - revised fix for issue #124157#
    // no further notification, if not connected to Writer layout
    else if ( maAnchoredDrawObj.GetAnchorFrm() &&
              findConnectionToSdrObjectDirect(maAnchoredDrawObj.GetDrawObj()) )
    {
        // --> OD 2004-07-01 #i28701# - on change of wrapping style, hell|heaven layer,
        // or wrapping style influence an update of the <SwSortedObjs> list,
        // the drawing object is registered in, has to be performed. This is triggered
        // by the 1st parameter of method call <_InvalidateObjs(..)>.
        if ( RES_SURROUND == nWhich ||
             RES_OPAQUE == nWhich ||
             RES_WRAP_INFLUENCE_ON_OBJPOS == nWhich ||
             ( RES_ATTRSET_CHG == nWhich &&
               ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_SURROUND, sal_False ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_OPAQUE, sal_False ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_WRAP_INFLUENCE_ON_OBJPOS, sal_False ) ) ) )
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
                                RES_LR_SPACE, sal_False ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_UL_SPACE, sal_False ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_HORI_ORIENT, sal_False ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_VERT_ORIENT, sal_False ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_FOLLOW_TEXT_FLOW, sal_False ) ) ) )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
            NotifyBackgrdOfAllVirtObjs( 0L );
            _InvalidateObjs();
        }
        // --> OD 2004-10-26 #i35443#
        else if ( RES_ATTRSET_CHG == nWhich )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
            NotifyBackgrdOfAllVirtObjs( 0L );
            _InvalidateObjs();
        }
        // <--
        else if ( RES_REMOVE_UNO_OBJECT == nWhich )
        {
            // nothing to do
        }
#if OSL_DEBUG_LEVEL > 1
        else
        {
            ASSERT( false,
                    "<SwDrawContact::Modify(..)> - unhandled attribute? - please inform od@openoffice.org" );
        }
#endif
    }

    // --> OD 2005-07-18 #i51474#
    GetAnchoredObj( 0L )->ResetLayoutProcessBools();
    // <--
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
        // --> OD 2004-08-23 #i33313# - invalidation only for connected
        // 'virtual' drawing objects
        if ( pDrawVirtObj->IsConnected() )
        {
            pDrawVirtObj->AnchoredObj()->InvalidateObjPos();
            // --> OD 2004-07-01 #i28701#
            if ( _bUpdateSortedObjsList )
            {
                pDrawVirtObj->AnchoredObj()->UpdateObjInSortedList();
            }
            // <--
        }
        // <--
    }

    // invalidate position of 'master' drawing object
    SwAnchoredObject* pAnchoredObj = GetAnchoredObj( 0L );
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

    // --> OD 2004-10-27 #i36181# - notify background of drawing object
    if ( _bMoveMasterToInvisibleLayer &&
         !(GetFmt()->GetDoc()->IsInDtor()) &&
         GetAnchorFrm() )
    {
        const Rectangle aOldRect( maAnchoredDrawObj.GetObjRectWithSpaces().SVRect() );
        lcl_NotifyBackgroundOfObj( *this, *GetMaster(), &aOldRect );
        NotifyBackgrdOfAllVirtObjs( &aOldRect );
    }
    // <--

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

    if ( _bMoveMasterToInvisibleLayer && GetMaster() && GetMaster()->IsObjectInserted() )
    {
        const ::std::set< SdrView* > aAllSdrViews(GetMaster()->getSdrModelFromSdrObject().getSdrViews());

        for(::std::set< SdrView* >::const_iterator aLoopViews(aAllSdrViews.begin());
            aLoopViews != aAllSdrViews.end(); aLoopViews++)
        {
            (*aLoopViews)->MarkObj(*GetMaster(), true);
        }

        // OD 25.06.2003 #108784# - Instead of removing 'master' object from
        // drawing page, move the 'master' drawing object into the corresponding
        // invisible layer.
        {
            //((SwFrmFmt*)GetRegisteredIn())->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
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
        resetConnectionToSdrObject(GetMaster());

        if ( GetMaster()->IsObjectInserted() )
        {
            ((SwFrmFmt*)GetRegisteredIn())->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
                RemoveObjectFromSdrObjList( GetMaster()->GetNavigationPosition() );
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
    if ( dynamic_cast< SwDrawVirtObj* >(_pDrawObj) )
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

    // --> OD 2004-09-22 #i33909# - *no* connect to layout, if 'master' drawing
    // object isn't inserted in the drawing page
    if ( !GetMaster()->IsObjectInserted() )
    {
        ASSERT( false, "<SwDrawContact::ConnectToLayout(..)> - master drawing object not inserted -> no connect to layout. Please inform od@openoffice.org" );
        return;
    }
    // <--

    SwFrmFmt* pDrawFrmFmt = (SwFrmFmt*)GetRegisteredIn();

    if( !pDrawFrmFmt->getIDocumentLayoutAccess()->GetCurrentViewShell() )
        return;

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
        case FLY_AT_PAGE:
                {
                sal_uInt16 nPgNum = pAnch->GetPageNum();
                ViewShell *pShell = pDrawFrmFmt->getIDocumentLayoutAccess()->GetCurrentViewShell();
                if( !pShell )
                    break;
                SwRootFrm* pRoot = pShell->GetLayout();
                SwPageFrm *pPage = static_cast<SwPageFrm*>(pRoot->Lower());

                for ( sal_uInt16 i = 1; i < nPgNum && pPage; ++i )
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

        case FLY_AT_CHAR:
        case FLY_AT_PARA:
        case FLY_AT_FLY:
        case FLY_AS_CHAR:
            {
                if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
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
                        if ( SwIterator<SwFrm,SwCntntNode>::FirstElement( *pCNd ) )
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
                SwIterator<SwFrm,SwModify> aIter( *pModify );
                SwFrm* pAnchorFrmOfMaster = 0;
                for( SwFrm *pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
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
                        if ((pAnch->GetAnchorId() == FLY_AS_CHAR) &&
                             pFrm->IsTxtFrm() )
                        {
                            pFrm = lcl_GetFlyInCntntAnchor(
                                        static_cast<SwTxtFrm*>(pFrm),
                                        pAnch->GetCntntAnchor()->nContent.GetIndex() );
                        }

                        if ( !pAnchorFrmOfMaster )
                        {
                            // append 'master' drawing object
                            pAnchorFrmOfMaster = pFrm;
                            pFrm->AppendDrawObj( maAnchoredDrawObj );
                        }
                        else
                        {
                            // append 'virtual' drawing object
                            SwDrawVirtObj* pDrawVirtObj = AddVirtObj();
                            if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
                            {
                                ClrContourCache( pDrawVirtObj );
                            }
                            pFrm->AppendDrawObj( *(pDrawVirtObj->AnchoredObj()) );

                            // for repaint, use new ActionChanged()
                            // pDrawVirtObj->SendRepaintBroadcast();
                            pDrawVirtObj->ActionChanged();
                        }

                        if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
                        {
                            pFrm->InvalidatePrt();
                        }
                    }
                }
            }
            break;
        default:
            ASSERT( sal_False, "Unknown Anchor." )
            break;
    }
    if ( GetAnchorFrm() )
    {
        ::setContextWritingMode( maAnchoredDrawObj.DrawObj(), GetAnchorFrm() );
        // OD 2004-04-01 #i26791# - invalidate objects instead of direct positioning
        _InvalidateObjs();
    }
}

// OD 27.06.2003 #108784# - insert 'master' drawing object into drawing page
void SwDrawContact::InsertMasterIntoDrawPage()
{
    if ( !GetMaster()->IsObjectInserted() )
    {
        GetFmt()->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)
            ->InsertObjectToSdrObjList(*GetMaster(), GetMaster()->GetNavigationPosition() );
    }

    establishConnectionToSdrObject(GetMaster(), this);
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
                     : FindPage( sdr::legacy::GetBoundRect(*GetMaster()) );
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
// method is called by method <SwDPage::ReplaceObjectInSdrObjList(..)>, which called its
// corresponding superclass method <FmFormPage::ReplaceObjectInSdrObjList(..)>.
// Note: 'master' drawing object *has* to be connected to layout triggered
//       by the caller of this, if method is called.
void SwDrawContact::ChangeMasterObject( SdrObject *pNewMaster )
{
    DisconnectFromLayout( false );
    // OD 10.07.2003 #110742# - consider 'virtual' drawing objects
    RemoveAllVirtObjs();

    resetConnectionToSdrObject(GetMaster());
    SetMaster( pNewMaster );
    establishConnectionToSdrObject(GetMaster(), this);

    _InvalidateObjs();
}

/** get data collection of anchored objects, handled by with contact

    OD 2004-08-23 #110810#

    @author
*/
void SwDrawContact::GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const
{
    _roAnchoredObjs.push_back( const_cast<SwAnchoredDrawObject*>(&maAnchoredDrawObj) );

    for ( std::list<SwDrawVirtObj*>::const_iterator aDrawVirtObjsIter = maDrawVirtObjs.begin();
          aDrawVirtObjsIter != maDrawVirtObjs.end();
          ++aDrawVirtObjsIter )
    {
        _roAnchoredObjs.push_back( (*aDrawVirtObjsIter)->AnchoredObj() );
    }
}

//////////////////////////////////////////////////////////////////////////////////////
// AW: own sdr::contact::ViewContact (VC) sdr::contact::ViewObjectContact (VOC) needed
// since offset is defined different

namespace sdr
{
    namespace contact
    {
        class VOCOfDrawVirtObj : public ViewObjectContactOfSdrObj
        {
        protected:
            // This method is responsible for creating the graphical visualisation data which is
            // stored/cached in the local primitive. Default gets view-independent Primitive
            // from the ViewContact using ViewContact::getViewIndependentPrimitive2DSequence(), takes care of
            // visibility, handles glue and ghosted.
            // This method will not handle included hierarchies and not check geometric visibility.
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        public:
            VOCOfDrawVirtObj(ObjectContact& rObjectContact, ViewContact& rViewContact)
            :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
            {
            }

            virtual ~VOCOfDrawVirtObj();
        };

        class VCOfDrawVirtObj : public ViewContactOfSdrObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something. Default is to create
            // a standard ViewObjectContact containing the given ObjectContact and *this
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // basic constructor, used from SdrObject.
            VCOfDrawVirtObj(SwDrawVirtObj& rObj)
            :   ViewContactOfSdrObj(rObj)
            {
            }
            virtual ~VCOfDrawVirtObj();

            // access to SwDrawVirtObj
            SwDrawVirtObj& GetSwDrawVirtObj() const
            {
                return (SwDrawVirtObj&)mrObject;
            }
        };
    } // end of namespace contact
} // end of namespace sdr

namespace sdr
{
    namespace contact
    {
        // recursively collect primitive data from given VOC with given offset
        void impAddPrimitivesFromGroup(
            const ViewObjectContact& rVOC,
            const basegfx::B2DHomMatrix& rOriginalToVirtual,
            const DisplayInfo& rDisplayInfo,
            drawinglayer::primitive2d::Primitive2DSequence& rxTarget)
        {
            const sal_uInt32 nSubHierarchyCount(rVOC.GetViewContact().GetObjectCount());

            for(sal_uInt32 a(0L); a < nSubHierarchyCount; a++)
            {
                const ViewObjectContact& rCandidate(rVOC.GetViewContact().GetViewContact(a).GetViewObjectContact(rVOC.GetObjectContact()));

                if(rCandidate.GetViewContact().GetObjectCount())
                {
                    // is a group object itself, call resursively
                    impAddPrimitivesFromGroup(rCandidate, rOriginalToVirtual, rDisplayInfo, rxTarget);
                }
                else
                {
                    // single object, add primitives; check model-view visibility
                    if(rCandidate.isPrimitiveVisible(rDisplayInfo))
                    {
                        drawinglayer::primitive2d::Primitive2DSequence aNewSequence(rCandidate.getPrimitive2DSequence(rDisplayInfo));

                        if(aNewSequence.hasElements())
                        {
                            // get ranges
                            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(rCandidate.GetObjectContact().getViewInformation2D());
                            const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());
                            basegfx::B2DRange aObjectRange(rCandidate.getViewDependentRange());

                            // correct with virtual object's offset
                            aObjectRange.transform(rOriginalToVirtual);

                            // check geometrical visibility (with offset)
                            if(!aViewRange.overlaps(aObjectRange))
                            {
                                // not visible, release
                                aNewSequence.realloc(0);
                            }
                        }

                        if(aNewSequence.hasElements())
                        {
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rxTarget, aNewSequence);
                        }
                    }
                }
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence VOCOfDrawVirtObj::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            const VCOfDrawVirtObj& rVC = static_cast< const VCOfDrawVirtObj& >(GetViewContact());
            const SdrObject& rReferencedObject = rVC.GetSwDrawVirtObj().GetReferencedObj();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // create transformation to map original object to virtual transformation. To do
            // so, apply inverse original and virtual
            basegfx::B2DHomMatrix aOriginalToVirtual(rReferencedObject.getSdrObjectTransformation());
            aOriginalToVirtual.invert();
            aOriginalToVirtual = rVC.GetSwDrawVirtObj().getSdrObjectTransformation() * aOriginalToVirtual;

            if(dynamic_cast< const SdrObjGroup* >(&rReferencedObject))
            {
                // group object. Since the VOC/OC/VC hierarchy does not represent the
                // hierarchy virtual objects when they have group objects
                // to avoid multiple usages of VOCs (which would not work), the primitives
                // for the sub-hierarchy need to be collected here

                // Get the VOC of the referenced object (the Group) and fetch primitives from it
                const ViewObjectContact& rVOCOfRefObj = rReferencedObject.GetViewContact().GetViewObjectContact(GetObjectContact());
                impAddPrimitivesFromGroup(rVOCOfRefObj, aOriginalToVirtual, rDisplayInfo, xRetval);
            }
            else
            {
                // single object, use method from referenced object to get the Primitive2DSequence
                xRetval = rReferencedObject.GetViewContact().getViewIndependentPrimitive2DSequence();
            }

            if(xRetval.hasElements())
            {
                // create transform primitive
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aOriginalToVirtual,
                        xRetval));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            return xRetval;
        }

        VOCOfDrawVirtObj::~VOCOfDrawVirtObj()
        {
        }

        ViewObjectContact& VCOfDrawVirtObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            return *(new VOCOfDrawVirtObj(rObjectContact, *this));
        }

        VCOfDrawVirtObj::~VCOfDrawVirtObj()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////////////

// =============================================================================
/** implementation of class <SwDrawVirtObj>

    OD 14.05.2003 #108784#

    @author OD
*/

sdr::contact::ViewContact* SwDrawVirtObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::VCOfDrawVirtObj(*this);
}

void SwDrawVirtObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
//  mbClosedObject = rRefObj.IsClosedObj();
    ActionChanged();
}

// #108784#
// implemetation of SwDrawVirtObj
SwDrawVirtObj::SwDrawVirtObj(
    SdrObject& _rNewObj,
                              SwDrawContact&    _rDrawContact )
:   SdrObject(_rNewObj.getSdrModelFromSdrObject()),
      maAnchoredDrawObj(),
    mrDrawContact( _rDrawContact ),
    rRefObj(_rNewObj),
    aSnapRect()
{
    maAnchoredDrawObj.SetDrawObj( *this );
    sdr::legacy::transformSdrObject(*this, basegfx::tools::createTranslateB2DHomMatrix(-RECT_EMPTY, -RECT_EMPTY));
    StartListening(rRefObj);
}

SwDrawVirtObj::~SwDrawVirtObj()
{
    EndListening(rRefObj);
}

void SwDrawVirtObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
{
        const SwDrawVirtObj* pSource = dynamic_cast< const SwDrawVirtObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObject::copyDataFromSdrObject(rSource);

            // copy AnchorPos
            SetAnchorPos(pSource->GetAnchorPos());
        }
        else
    {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
    }

SdrObject* SwDrawVirtObj::CloneSdrObject(SdrModel* /*pTargetModel*/) const
{
    SwDrawVirtObj* pClone = new SwDrawVirtObj(rRefObj, mrDrawContact);
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SwDrawVirtObj::IsClosedObj() const
{
    return rRefObj.IsClosedObj();
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
    SdrPage* pDrawPg;
    // --> OD 2004-08-16 #i27030# - apply order number of referenced object
    if ( 0 != ( pDrawPg = pOrgMasterSdrObj->getSdrPageFromSdrObject() ) )
    {
        // --> OD 2004-08-16 #i27030# - apply order number of referenced object
        pDrawPg->InsertObjectToSdrObjList(*this, GetReferencedObj().GetNavigationPosition() );
    }
    else
    {
        if ( getParentOfSdrObject() )
        {
            getParentOfSdrObject()->SetNavigationPosition( GetNavigationPosition(),
                GetReferencedObj().GetNavigationPosition() );
        }
        else
        {
            // TTTT: object should always be inserted into an ObjectList, thus this should not
            // happen. Even if it happens, there is no use in setting an OrdNum since an object
            // which is not member of an SdrObjLsit by definition has an OrdNum of 0 since it
            // is not member of a Z-Order (see SdrObject::GetNavigationPosition())
            OSL_ENSURE(false, "SwDrawVirtObj::AddToDrawingPage try to set OrdNum for non-inserted SdrObject (!)");
            // former code: SetOrdNum( GetReferencedObj().GetNavigationPosition() );
        }
    }
    // <--

    establishConnectionToSdrObject(this, &mrDrawContact);
}

void SwDrawVirtObj::RemoveFromDrawingPage()
{
    resetConnectionToSdrObject(this);

    if ( getParentOfSdrObject() )
    {
        getParentOfSdrObject()->RemoveObjectFromSdrObjList( GetNavigationPosition() );
    }
}

// is 'virtual' drawing object connected to writer layout and to drawing layer.
bool SwDrawVirtObj::IsConnected() const
{
    bool bRetVal = GetAnchorFrm() &&
                   ( getSdrPageFromSdrObject() && findConnectionToSdrObjectDirect(this) );

    return bRetVal;
}

//void SwDrawVirtObj::NbcSetAnchorPos(const Point& rPnt)
//{
//    SdrObject::NbcSetAnchorPos( rPnt );
//}

//////////////////////////////////////////////////////////////////////////////
// #i97197#
// the methods relevant for positioning

const basegfx::B2DPoint SwDrawVirtObj::GetOffset() const
{
    const basegfx::B2DPoint aLocalTranslate(getSdrObjectTranslate());
    const basegfx::B2DPoint aRefTranslate(GetReferencedObj().getSdrObjectTranslate());

    return aLocalTranslate - aRefTranslate;
}

basegfx::B2DPolyPolygon SwDrawVirtObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval(rRefObj.TakeXorPoly());
    aRetval.transform(basegfx::tools::createTranslateB2DHomMatrix(GetOffset()));

    return aRetval;
}

void SwDrawVirtObj::GetPlusHdl(SdrHdlList& rHdlList, const SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlNum) const
    {
    rRefObj.GetPlusHdl(rHdlList, rSdrObject, rHdl, nPlNum);
    OSL_ENSURE(rHdlList.GetHdlCount(), "No PlusHdl added (!)");
    SdrHdl* pHdl = rHdlList.GetHdlByIndex(rHdlList.GetHdlCount() - 1);
    pHdl->setPosition(pHdl->getPosition() + GetOffset());
    }

basegfx::B2DPoint SwDrawVirtObj::GetSnapPoint(sal_uInt32 i) const
    {
    return rRefObj.GetSnapPoint(i) + GetOffset();
}

basegfx::B2DPoint SwDrawVirtObj::GetObjectPoint(sal_uInt32 i) const
{
    return rRefObj.GetObjectPoint(i) + GetOffset();
}

void SwDrawVirtObj::SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i)
{
    const basegfx::B2DPoint aP(rPnt - GetOffset());
    rRefObj.SetObjectPoint(aP, i);
    ActionChanged();
}

// #108784#
bool SwDrawVirtObj::HasTextEdit() const
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

//void SwDrawVirtObj::NbcSetLayer(SdrLayerID nLayer)
//{
//    ReferencedObj().NbcSetLayer( nLayer );
//    SdrObject::NbcSetLayer( ReferencedObj().GetLayer() );
//}

void SwDrawVirtObj::SetLayer(SdrLayerID nLayer)
{
    ReferencedObj().SetLayer(nLayer);
    SdrObject::SetLayer(ReferencedObj().GetLayer());
}

bool SwDrawVirtObj::supportsFullDrag() const
{
    return false;
}

SdrObject* SwDrawVirtObj::getFullDragClone() const
{
    SdrObject& rReferencedObject = const_cast< SdrObject& >(GetReferencedObj());

    return new SdrGrafObj(
        getSdrModelFromSdrObject(),
        GetObjGraphic(rReferencedObject),
        getSdrObjectTransformation());
}

//////////////////////////////////////////////////////////////////////////
// methods which were missing from SdrVirtObj which do something

SdrObject& SwDrawVirtObj::ReferencedObj()
{
    return rRefObj;
}

const SdrObject& SwDrawVirtObj::GetReferencedObj() const
{
    return rRefObj;
}

sdr::properties::BaseProperties& SwDrawVirtObj::GetProperties() const
{
    return rRefObj.GetProperties();
}

//void SwDrawVirtObj::SetModel(SdrModel* pNewModel)
//{
//  SdrObject::SetModel(pNewModel);
//  rRefObj.SetModel(pNewModel);
//}

void SwDrawVirtObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rRefObj.TakeObjInfo(rInfo);
}

sal_uInt32 SwDrawVirtObj::GetObjInventor() const
{
    return rRefObj.GetObjInventor();
}

sal_uInt16 SwDrawVirtObj::GetObjIdentifier() const
{
    return rRefObj.GetObjIdentifier();
}

SdrObjList* SwDrawVirtObj::getChildrenOfSdrObject() const
{
    return rRefObj.getChildrenOfSdrObject();
}

void SwDrawVirtObj::TakeObjNameSingul(XubString& rName) const
{
    rRefObj.TakeObjNameSingul(rName);
    rName.Insert(sal_Unicode('['), 0);
    rName += sal_Unicode(']');

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SwDrawVirtObj::TakeObjNamePlural(XubString& rName) const
    {
    rRefObj.TakeObjNamePlural(rName);
    rName.Insert(sal_Unicode('['), 0);
    rName += sal_Unicode(']');
    }

sal_uInt32 SwDrawVirtObj::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    return rRefObj.GetPlusHdlCount(rHdl);
}

void SwDrawVirtObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // #i73248#
    // SdrObject::AddToHdlList(rHdlList) is not a good thing to call
    // since at SdrPathObj, only AddToHdlList may be used and the call
    // will instead use the standard implementation which uses GetHdlCount()
    // and GetHdl instead. This is not wrong, but may be much less effective
    // and may not be prepared to GetHdl returning NULL

    // add handles from refObj to list and transform them
    const sal_uInt32 nStart(rHdlList.GetHdlCount());
    rRefObj.AddToHdlList(rHdlList);
    const sal_uInt32 nEnd(rHdlList.GetHdlCount());

    if(nEnd > nStart)
{
        const basegfx::B2DPoint aOffset(GetOffset());

        for(sal_uInt32 a(nStart); a < nEnd; a++)
    {
            SdrHdl* pCandidate = rHdlList.GetHdlByIndex(a);

            pCandidate->setPosition(pCandidate->getPosition() + aOffset);
        }
    }
}

bool SwDrawVirtObj::hasSpecialDrag() const
{
    return rRefObj.hasSpecialDrag();
}

bool SwDrawVirtObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    return rRefObj.beginSpecialDrag(rDrag);
}

bool SwDrawVirtObj::applySpecialDrag(SdrDragStat& rDrag)
    {
    return rRefObj.applySpecialDrag(rDrag);
    }

String SwDrawVirtObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragComment(rDrag);
}

basegfx::B2DPolyPolygon SwDrawVirtObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragPoly(rDrag);
    // Offset handlen !!!!!! fehlt noch !!!!!!!
}

bool SwDrawVirtObj::BegCreate(SdrDragStat& rStat)
{
    return rRefObj.BegCreate(rStat);
}

bool SwDrawVirtObj::MovCreate(SdrDragStat& rStat)
{
    return rRefObj.MovCreate(rStat);
}

bool SwDrawVirtObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    return rRefObj.EndCreate(rStat,eCmd);
}

bool SwDrawVirtObj::BckCreate(SdrDragStat& rStat)
{
    return rRefObj.BckCreate(rStat);
}

void SwDrawVirtObj::BrkCreate(SdrDragStat& rStat)
{
    rRefObj.BrkCreate(rStat);
}

basegfx::B2DPolyPolygon SwDrawVirtObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    return rRefObj.TakeCreatePoly(rDrag);
    // Offset handlen !!!!!! fehlt noch !!!!!!!
}

sal_uInt32 SwDrawVirtObj::GetSnapPointCount() const
{
    return rRefObj.GetSnapPointCount();
}

bool SwDrawVirtObj::IsPolygonObject() const
{
    return rRefObj.IsPolygonObject();
}

sal_uInt32 SwDrawVirtObj::GetObjectPointCount() const
{
    return rRefObj.GetObjectPointCount();
}

SdrObjGeoData* SwDrawVirtObj::GetGeoData() const
{
    return rRefObj.GetGeoData();
}

void SwDrawVirtObj::SetGeoData(const SdrObjGeoData& rGeo)
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    rRefObj.SetGeoData(rGeo);
    ActionChanged();
}

//void SwDrawVirtObj::NbcReformatText()
//{
//  rRefObj.NbcReformatText();
//}

void SwDrawVirtObj::ReformatText()
{
    rRefObj.ReformatText();
}

bool SwDrawVirtObj::HasMacro() const
{
    return rRefObj.HasMacro();
}

SdrObject* SwDrawVirtObj::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.CheckMacroHit(rRec); // Todo: Positionsversatz
}

Pointer SwDrawVirtObj::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.GetMacroPointer(rRec); // Todo: Positionsversatz
}

void SwDrawVirtObj::PaintMacro(OutputDevice& rOut, const SdrObjMacroHitRec& rRec) const
{
    rRefObj.PaintMacro(rOut,rRec); // Todo: Positionsversatz
}

bool SwDrawVirtObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return rRefObj.DoMacro(rRec); // Todo: Positionsversatz
}

XubString SwDrawVirtObj::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.GetMacroPopupComment(rRec); // Todo: Positionsversatz
}

//////////////////////////////////////////////////////////////////////////
// eof

