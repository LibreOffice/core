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
#include <svx/svdviter.hxx>
#include <svx/svdview.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <drawdoc.hxx>
#include <fmtornt.hxx>
#include <viewimp.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <node.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
#include <flyfrm.hxx>
#include <textboxhelper.hxx>
#include <frmfmt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <unodraw.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <doc.hxx>
#include <hints.hxx>
#include <txtfrm.hxx>
#include <editsh.hxx>
#include <docary.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <calbck.hxx>
#include <algorithm>
#include <txtfly.hxx>

using namespace ::com::sun::star;


void setContextWritingMode( SdrObject* pObj, SwFrame* pAnchor )
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

/** The Get reverse way: seeks the format to the specified object.
 * If the object is a SwVirtFlyDrawObj then the format of this
 * will be acquired.
 * Otherwise it is just a simple drawing object. This has a
 * UserCall and is the client of the searched format.
*/
SwFrameFormat *FindFrameFormat( SdrObject *pObj )
{
    SwFrameFormat* pRetval = nullptr;

    if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
    {
       pRetval = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFormat();
    }
    else
    {
        SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
        if ( pContact )
        {
            pRetval = pContact->GetFormat();
        }
    }
    return pRetval;
}

bool HasWrap( const SdrObject* pObj )
{
    if ( pObj )
    {
        const SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
        if ( pFormat )
        {
            return SURROUND_THROUGHT != pFormat->GetSurround().GetSurround();
        }
    }

    return false;
}

/// returns the BoundRect _inclusive_ distance of the object.
SwRect GetBoundRectOfAnchoredObj( const SdrObject* pObj )
{
    SwRect aRet( pObj->GetCurrentBoundRect() );
    // #i68520# - call cache of <SwAnchoredObject>
    SwContact* pContact( GetUserCall( pObj ) );
    if ( pContact )
    {
        const SwAnchoredObject* pAnchoredObj( pContact->GetAnchoredObj( pObj ) );
        if ( pAnchoredObj )
        {
            aRet = pAnchoredObj->GetObjRectWithSpaces();
        }
    }
    return aRet;
}

/// Returns the UserCall if applicable from the group object
SwContact* GetUserCall( const SdrObject* pObj )
{
    SdrObject *pTmp;
    while ( !pObj->GetUserCall() && nullptr != (pTmp = pObj->GetUpGroup()) )
        pObj = pTmp;
    OSL_ENSURE( !pObj->GetUserCall() || nullptr != dynamic_cast< const SwContact*> (pObj->GetUserCall()),
            "<::GetUserCall(..)> - wrong type of found object user call." );
    return static_cast<SwContact*>(pObj->GetUserCall());
}

/// Returns true if the SrdObject is a Marquee-Object (scrolling text)
bool IsMarqueeTextObj( const SdrObject& rObj )
{
    SdrTextAniKind eTKind;
    return SdrInventor == rObj.GetObjInventor() &&
        OBJ_TEXT == rObj.GetObjIdentifier() &&
        ( SDRTEXTANI_SCROLL == ( eTKind = static_cast<const SdrTextObj&>(rObj).GetTextAniKind())
         || SDRTEXTANI_ALTERNATE == eTKind || SDRTEXTANI_SLIDE == eTKind );
}

SwContact::SwContact( SwFrameFormat *pToRegisterIn ) :
    SwClient( pToRegisterIn ),
    mbInDTOR( false )
{}

SwContact::~SwContact()
{
    SetInDTOR();
}


void SwContact::SetInDTOR()
{
    mbInDTOR = true;
}

/// method to move drawing object to corresponding visible layer
void SwContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    // #i46297# - notify background about the arriving of
    // the object and invalidate its position.
    const bool bNotify( !GetFormat()->getIDocumentDrawModelAccess().IsVisibleLayerId( _pDrawObj->GetLayer() ) );

    _MoveObjToLayer( true, _pDrawObj );

    // #i46297#
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        OSL_ENSURE( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        if ( pAnchoredObj )
        {
            ::setContextWritingMode( _pDrawObj, pAnchoredObj->GetAnchorFrameContainingAnchPos() );
            // Note: as-character anchored objects aren't registered at a page frame and
            //       a notification of its background isn't needed.
            if ( pAnchoredObj->GetPageFrame() )
            {
                ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrame(),
                                     pAnchoredObj->GetObjRect(), PREP_FLY_ARRIVE, true );
            }

            pAnchoredObj->InvalidateObjPos();
        }
    }
}

/// method to move drawing object to corresponding invisible layer - #i18447#
void SwContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    // #i46297# - notify background about the leaving of the object.
    const bool bNotify( GetFormat()->getIDocumentDrawModelAccess().IsVisibleLayerId( _pDrawObj->GetLayer() ) );

    _MoveObjToLayer( false, _pDrawObj );

    // #i46297#
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        OSL_ENSURE( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        // Note: as-character anchored objects aren't registered at a page frame and
        //       a notification of its background isn't needed.
        if ( pAnchoredObj && pAnchoredObj->GetPageFrame() )
        {
            ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrame(),
                                 pAnchoredObj->GetObjRect(), PREP_FLY_LEAVE, true );
        }
    }
}

/** method to move object to visible/invisible layer -  #i18447#

    implementation for the public method <MoveObjToVisibleLayer(..)>
    and <MoveObjToInvisibleLayer(..)>
*/
void SwContact::_MoveObjToLayer( const bool _bToVisible,
                                 SdrObject* _pDrawObj )
{
    if ( !_pDrawObj )
    {
        OSL_FAIL( "SwDrawContact::_MoveObjToLayer(..) - no drawing object!" );
        return;
    }

    if ( !GetRegisteredIn() )
    {
        OSL_FAIL( "SwDrawContact::_MoveObjToLayer(..) - no drawing frame format!" );
        return;
    }

    const IDocumentDrawModelAccess& rIDDMA = static_cast<SwFrameFormat*>(GetRegisteredInNonConst())->getIDocumentDrawModelAccess();

    SdrLayerID nToHellLayerId =
        _bToVisible ? rIDDMA.GetHellId() : rIDDMA.GetInvisibleHellId();
    SdrLayerID nToHeavenLayerId =
        _bToVisible ? rIDDMA.GetHeavenId() : rIDDMA.GetInvisibleHeavenId();
    SdrLayerID nToControlLayerId =
        _bToVisible ? rIDDMA.GetControlsId() : rIDDMA.GetInvisibleControlsId();
    SdrLayerID nFromHellLayerId =
        _bToVisible ? rIDDMA.GetInvisibleHellId() : rIDDMA.GetHellId();
    SdrLayerID nFromHeavenLayerId =
        _bToVisible ? rIDDMA.GetInvisibleHeavenId() : rIDDMA.GetHeavenId();
    SdrLayerID nFromControlLayerId =
        _bToVisible ? rIDDMA.GetInvisibleControlsId() : rIDDMA.GetControlsId();

    if ( dynamic_cast<const SdrObjGroup*>( _pDrawObj) !=  nullptr )
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
            else if ( _pDrawObj->GetLayer() == rIDDMA.GetHeavenId() ||
                      _pDrawObj->GetLayer() == rIDDMA.GetInvisibleHeavenId() )
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
            for ( size_t i = 0; i < pLst->GetObjCount(); ++i )
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

// some virtual helper methods for information
// about the object (Writer fly frame resp. drawing object)

const SwIndex& SwContact::GetContentAnchorIndex() const
{
    return GetContentAnchor().nContent;
}

/// get minimum order number of anchored objects handled by with contact
sal_uInt32 SwContact::GetMinOrdNum() const
{
    sal_uInt32 nMinOrdNum( SAL_MAX_UINT32 );

    std::list< SwAnchoredObject* > aObjs;
    GetAnchoredObjs( aObjs );

    while ( !aObjs.empty() )
    {
        sal_uInt32 nTmpOrdNum = aObjs.back()->GetDrawObj()->GetOrdNum();

        if ( nTmpOrdNum < nMinOrdNum )
        {
            nMinOrdNum = nTmpOrdNum;
        }

        aObjs.pop_back();
    }

    OSL_ENSURE( nMinOrdNum != SAL_MAX_UINT32,
            "<SwContact::GetMinOrdNum()> - no order number found." );
    return nMinOrdNum;
}

/// get maximum order number of anchored objects handled by with contact
sal_uInt32 SwContact::GetMaxOrdNum() const
{
    sal_uInt32 nMaxOrdNum( 0L );

    std::list< SwAnchoredObject* > aObjs;
    GetAnchoredObjs( aObjs );

    while ( !aObjs.empty() )
    {
        sal_uInt32 nTmpOrdNum = aObjs.back()->GetDrawObj()->GetOrdNum();

        if ( nTmpOrdNum > nMaxOrdNum )
        {
            nMaxOrdNum = nTmpOrdNum;
        }

        aObjs.pop_back();
    }

    return nMaxOrdNum;
}

SwFlyDrawContact::SwFlyDrawContact( SwFlyFrameFormat *pToRegisterIn, SdrModel * ) :
    SwContact( pToRegisterIn )
{
    // #i26791# - class <SwFlyDrawContact> contains the 'master'
    // drawing object of type <SwFlyDrawObj> on its own.
    mpMasterObj = new SwFlyDrawObj;
    mpMasterObj->SetOrdNum( 0xFFFFFFFE );
    mpMasterObj->SetUserCall( this );
}

SwFlyDrawContact::~SwFlyDrawContact()
{
    if ( mpMasterObj )
    {
        mpMasterObj->SetUserCall( nullptr );
        if ( mpMasterObj->GetPage() )
            mpMasterObj->GetPage()->RemoveObject( mpMasterObj->GetOrdNum() );
        delete mpMasterObj;
    }
}

// #i26791#
const SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    OSL_ENSURE( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( _pSdrObj) !=  nullptr,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = nullptr;

    if ( _pSdrObj && dynamic_cast<const SwVirtFlyDrawObj*>( _pSdrObj) !=  nullptr )
    {
        pRetAnchoredObj = static_cast<const SwVirtFlyDrawObj*>(_pSdrObj)->GetFlyFrame();
    }

    return pRetAnchoredObj;
}

SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( SdrObject* _pSdrObj )
{
    OSL_ENSURE( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( _pSdrObj) !=  nullptr,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = nullptr;

    if ( _pSdrObj && dynamic_cast<const SwVirtFlyDrawObj*>( _pSdrObj) !=  nullptr )
    {
        pRetAnchoredObj = static_cast<SwVirtFlyDrawObj*>(_pSdrObj)->GetFlyFrame();
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
    OSL_ENSURE( dynamic_cast<const SwFlyDrawObj*>( _pNewMaster) !=  nullptr,
            "<SwFlyDrawContact::SetMaster(..)> - wrong type of new master object" );
    mpMasterObj = static_cast<SwFlyDrawObj *>(_pNewMaster);
}

void SwFlyDrawContact::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
}

/**
 * @note Overriding method to control Writer fly frames, which are linked, and
 *       to assure that all objects anchored at/inside the Writer fly frame are
 *       also made visible.
 */
void SwFlyDrawContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( _pDrawObj) !=  nullptr,
            "<SwFlyDrawContact::MoveObjToVisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( GetFormat()->getIDocumentDrawModelAccess().IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        // nothing to do
        return;
    }

    SwFlyFrame* pFlyFrame = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrame();

    // #i44464# - consider, that Writer fly frame content
    // already exists - (e.g. WW8 document is inserted into a existing document).
    if ( !pFlyFrame->Lower() )
    {
        pFlyFrame->InsertColumns();
        pFlyFrame->Chain( pFlyFrame->AnchorFrame() );
        pFlyFrame->InsertCnt();
    }
    if ( pFlyFrame->GetDrawObjs() )
    {
        for ( size_t i = 0; i < pFlyFrame->GetDrawObjs()->size(); ++i)
        {
            // #i28701# - consider type of objects in sorted object list.
            SdrObject* pObj = (*pFlyFrame->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToVisibleLayer( pObj );
        }
    }

    // make fly frame visible
    SwContact::MoveObjToVisibleLayer( _pDrawObj );
}

/**
 * @note Override method to control Writer fly frames, which are linked, and
 *       to assure that all objects anchored at/inside the Writer fly frame are
 *       also made invisible.
 */
void SwFlyDrawContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( _pDrawObj) !=  nullptr,
            "<SwFlyDrawContact::MoveObjToInvisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( !GetFormat()->getIDocumentDrawModelAccess().IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        // nothing to do
        return;
    }

    SwFlyFrame* pFlyFrame = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrame();

    pFlyFrame->Unchain();
    pFlyFrame->DeleteCnt();
    if ( pFlyFrame->GetDrawObjs() )
    {
        for ( size_t i = 0; i < pFlyFrame->GetDrawObjs()->size(); ++i)
        {
            // #i28701# - consider type of objects in sorted object list.
            SdrObject* pObj = (*pFlyFrame->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToInvisibleLayer( pObj );
        }
    }

    // make fly frame invisible
    SwContact::MoveObjToInvisibleLayer( _pDrawObj );
}

/// get data collection of anchored objects, handled by with contact
void SwFlyDrawContact::GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const
{
    const SwFrameFormat* pFormat = GetFormat();
    SwFlyFrame::GetAnchoredObjects( _roAnchoredObjs, *pFormat );
}

// SwDrawContact

bool CheckControlLayer( const SdrObject *pObj )
{
    if ( FmFormInventor == pObj->GetObjInventor() )
        return true;
    if ( dynamic_cast<const SdrObjGroup*>( pObj) !=  nullptr )
    {
        const SdrObjList *pLst = static_cast<const SdrObjGroup*>(pObj)->GetSubList();
        for ( size_t i = 0; i < pLst->GetObjCount(); ++i )
        {
            if ( ::CheckControlLayer( pLst->GetObj( i ) ) )
            {
                // #i18447# - return correct value ;-)
                return true;
            }
        }
    }
    return false;
}

SwDrawContact::SwDrawContact( SwFrameFormat* pToRegisterIn, SdrObject* pObj ) :
    SwContact( pToRegisterIn ),
    maAnchoredDrawObj(),
    mbMasterObjCleared( false ),
    mbDisconnectInProgress( false ),
    mbUserCallActive( false ),
    // Note: value of <meEventTypeOfCurrentUserCall> isn't of relevance, because
    //       <mbUserCallActive> is false.
    meEventTypeOfCurrentUserCall( SDRUSERCALL_MOVEONLY )
{
    // clear list containing 'virtual' drawing objects.
    maDrawVirtObjs.clear();

    // --> #i33909# - assure, that drawing object is inserted
    // in the drawing page.
    if ( !pObj->IsInserted() )
    {
        pToRegisterIn->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0)->
                                InsertObject( pObj, pObj->GetOrdNumDirect() );
    }

    // Controls have to be always in the Control-Layer. This is also true for
    // group objects, if they contain controls.
    if ( ::CheckControlLayer( pObj ) )
    {
        // set layer of object to corresponding invisible layer.
        pObj->SetLayer( pToRegisterIn->getIDocumentDrawModelAccess().GetInvisibleControlsId() );
    }

    // #i26791#
    pObj->SetUserCall( this );
    maAnchoredDrawObj.SetDrawObj( *pObj );

    // if there already exists an SwXShape for the object, ensure it knows about us, and the SdrObject
    // #i99056#
    SwXShape::AddExistingShapeToFormat( *pObj );
}

SwDrawContact::~SwDrawContact()
{
    SetInDTOR();

    DisconnectFromLayout();

    // remove 'master' from drawing page
    RemoveMasterFromDrawPage();

    // remove and destroy 'virtual' drawing objects.
    RemoveAllVirtObjs();

    if ( !mbMasterObjCleared )
    {
        SdrObject* pObject = const_cast< SdrObject* >( maAnchoredDrawObj.GetDrawObj() );
        SdrObject::Free( pObject );
    }
}

void SwDrawContact::GetTextObjectsFromFormat( std::list<SdrTextObj*>& rTextObjects, SwDoc* pDoc )
{
    for( sal_Int32 n=0; n<(sal_Int32)pDoc->GetSpzFrameFormats()->size(); n++ )
    {
        const SwFrameFormat* pFly = (*pDoc->GetSpzFrameFormats())[n];
        if( dynamic_cast<const SwDrawFrameFormat*>( pFly ) !=  nullptr )
        {
            SwDrawContact* pContact = SwIterator<SwDrawContact,SwFrameFormat>(*pFly).First();
            if( pContact )
            {
                SdrObject* pSdrO = pContact->GetMaster();
                if ( pSdrO )
                {
                    if ( dynamic_cast<const SdrObjGroup*>(pSdrO) !=  nullptr )
                    {
                        SdrObjListIter aListIter( *pSdrO, IM_DEEPNOGROUPS );
                        //iterate inside of a grouped object
                        while( aListIter.IsMore() )
                        {
                            SdrObject* pSdrOElement = aListIter.Next();
                            if( pSdrOElement && dynamic_cast<const SdrTextObj*>(pSdrOElement) !=  nullptr &&
                                static_cast<SdrTextObj*>( pSdrOElement)->HasText() )
                            {
                                rTextObjects.push_back(static_cast<SdrTextObj*>( pSdrOElement ));
                            }
                        }
                    }
                    else if( dynamic_cast<const SdrTextObj*>(pSdrO) !=  nullptr &&
                            static_cast<SdrTextObj*>( pSdrO )->HasText() )
                    {
                        rTextObjects.push_back(static_cast<SdrTextObj*>( pSdrO ));
                    }
                }
            }
        }
    }
}

// #i26791#
const SwAnchoredObject* SwDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    // handle default parameter value
    if ( !_pSdrObj )
    {
        _pSdrObj = GetMaster();
    }

    OSL_ENSURE( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) !=  nullptr ||
            ( dynamic_cast<const SdrVirtObj*>( _pSdrObj) == nullptr && dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) == nullptr ),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this ||
            _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    const SwAnchoredObject* pRetAnchoredObj = nullptr;

    if ( _pSdrObj )
    {
        if ( dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) !=  nullptr )
        {
            pRetAnchoredObj = &(static_cast<const SwDrawVirtObj*>(_pSdrObj)->GetAnchoredObj());
        }
        else if ( dynamic_cast<const SdrVirtObj*>( _pSdrObj) == nullptr && dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) == nullptr)
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

    OSL_ENSURE( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) !=  nullptr ||
            ( dynamic_cast<const SdrVirtObj*>( _pSdrObj) == nullptr && dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) == nullptr),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this || _pSdrObj == GetMaster(),
            "<SwDrawContact::GetAnchoredObj(..)> - provided object doesn't belongs to this contact" );

    SwAnchoredObject* pRetAnchoredObj = nullptr;

    if ( _pSdrObj )
    {
        if ( dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) !=  nullptr )
        {
            pRetAnchoredObj = &(static_cast<SwDrawVirtObj*>(_pSdrObj)->AnchoredObj());
        }
        else if ( dynamic_cast<const SdrVirtObj*>( _pSdrObj) == nullptr && dynamic_cast<const SwDrawVirtObj*>( _pSdrObj) == nullptr)
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
           : nullptr;
}

SdrObject* SwDrawContact::GetMaster()
{
    return !mbMasterObjCleared
           ? maAnchoredDrawObj.DrawObj()
           : nullptr;
}

/**
 * @note Override <SwContact::SetMaster(..)> in order to assert, if the
 *       'master' drawing object is replaced. The latter is correctly handled,
 *       if handled by method <SwDrawContact::ChangeMasterObject(..)>. Thus,
 *       assert only, if a debug level is given.
 */
void SwDrawContact::SetMaster( SdrObject* _pNewMaster )
{
    if ( _pNewMaster )
    {
        OSL_FAIL( "debug notification - master replaced!" );
        maAnchoredDrawObj.SetDrawObj( *_pNewMaster );
    }
    else
    {
        mbMasterObjCleared = true;
    }
}

const SwFrame* SwDrawContact::GetAnchorFrame( const SdrObject* _pDrawObj ) const
{
    const SwFrame* pAnchorFrame = nullptr;
    if ( !_pDrawObj ||
         _pDrawObj == GetMaster() ||
         ( !_pDrawObj->GetUserCall() &&
           GetUserCall( _pDrawObj ) == static_cast<const SwContact* const>(this) ) )
    {
        pAnchorFrame = maAnchoredDrawObj.GetAnchorFrame();
    }
    else if ( dynamic_cast<const SwDrawVirtObj*>( _pDrawObj) !=  nullptr )
    {
        pAnchorFrame = static_cast<const SwDrawVirtObj*>(_pDrawObj)->GetAnchorFrame();
    }
    else
    {
        OSL_FAIL( "<SwDrawContact::GetAnchorFrame(..)> - unknown drawing object." );
    }

    return pAnchorFrame;
}
SwFrame* SwDrawContact::GetAnchorFrame( SdrObject* _pDrawObj )
{
    SwFrame* pAnchorFrame = nullptr;
    if ( !_pDrawObj ||
         _pDrawObj == GetMaster() ||
         ( !_pDrawObj->GetUserCall() &&
           GetUserCall( _pDrawObj ) == this ) )
    {
        pAnchorFrame = maAnchoredDrawObj.AnchorFrame();
    }
    else
    {
        OSL_ENSURE( dynamic_cast<const SwDrawVirtObj*>( _pDrawObj) !=  nullptr,
                "<SwDrawContact::GetAnchorFrame(..)> - unknown drawing object." );
        pAnchorFrame = static_cast<SwDrawVirtObj*>(_pDrawObj)->AnchorFrame();
    }

    return pAnchorFrame;
}

/// create a new 'virtual' drawing object.
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

/** destroys a given 'virtual' drawing object.
 *
 * side effect: 'virtual' drawing object is removed from data structure
 *              <maDrawVirtObjs>.
 */
void SwDrawContact::DestroyVirtObj( SwDrawVirtObj* _pVirtObj )
{
    if ( _pVirtObj )
    {
        delete _pVirtObj;
        _pVirtObj = nullptr;
    }
}

/** add a 'virtual' drawing object to drawing page.
 *
 * Use an already created one, which isn't used, or create a new one.
 */
SwDrawVirtObj* SwDrawContact::AddVirtObj()
{
    SwDrawVirtObj* pAddedDrawVirtObj = nullptr;

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

/// remove 'virtual' drawing objects and destroy them.
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

SwDrawContact::VirtObjAnchoredAtFramePred::VirtObjAnchoredAtFramePred(
                                                const SwFrame& _rAnchorFrame )
    : mpAnchorFrame( &_rAnchorFrame )
{
    if ( mpAnchorFrame->IsContentFrame() )
    {
        const SwContentFrame* pTmpFrame =
                            static_cast<const SwContentFrame*>( mpAnchorFrame );
        while ( pTmpFrame->IsFollow() )
        {
            pTmpFrame = pTmpFrame->FindMaster();
        }
        mpAnchorFrame = pTmpFrame;
    }
}

// #i26791# - compare with master frame
bool SwDrawContact::VirtObjAnchoredAtFramePred::operator() ( const SwDrawVirtObj* _pDrawVirtObj )
{
    const SwFrame* pObjAnchorFrame = _pDrawVirtObj->GetAnchorFrame();
    if ( pObjAnchorFrame && pObjAnchorFrame->IsContentFrame() )
    {
        const SwContentFrame* pTmpFrame =
                            static_cast<const SwContentFrame*>( pObjAnchorFrame );
        while ( pTmpFrame->IsFollow() )
        {
            pTmpFrame = pTmpFrame->FindMaster();
        }
        pObjAnchorFrame = pTmpFrame;
    }

    return ( pObjAnchorFrame == mpAnchorFrame );
}

/// get drawing object ('master' or 'virtual') by frame.
SdrObject* SwDrawContact::GetDrawObjectByAnchorFrame( const SwFrame& _rAnchorFrame )
{
    SdrObject* pRetDrawObj = nullptr;

    // #i26791# - compare master frames instead of direct frames
    const SwFrame* pProposedAnchorFrame = &_rAnchorFrame;
    if ( pProposedAnchorFrame->IsContentFrame() )
    {
        const SwContentFrame* pTmpFrame =
                            static_cast<const SwContentFrame*>( pProposedAnchorFrame );
        while ( pTmpFrame->IsFollow() )
        {
            pTmpFrame = pTmpFrame->FindMaster();
        }
        pProposedAnchorFrame = pTmpFrame;
    }

    const SwFrame* pMasterObjAnchorFrame = GetAnchorFrame();
    if ( pMasterObjAnchorFrame && pMasterObjAnchorFrame->IsContentFrame() )
    {
        const SwContentFrame* pTmpFrame =
                            static_cast<const SwContentFrame*>( pMasterObjAnchorFrame );
        while ( pTmpFrame->IsFollow() )
        {
            pTmpFrame = pTmpFrame->FindMaster();
        }
        pMasterObjAnchorFrame = pTmpFrame;
    }

    if ( pMasterObjAnchorFrame && pMasterObjAnchorFrame == pProposedAnchorFrame )
    {
        pRetDrawObj = GetMaster();
    }
    else
    {
        std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
                std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
                              VirtObjAnchoredAtFramePred( *pProposedAnchorFrame ) );

        if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
        {
            pRetDrawObj = (*aFoundVirtObjIter);
        }
    }

    return pRetDrawObj;
}

void SwDrawContact::NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect )
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        if ( pDrawVirtObj->GetAnchorFrame() )
        {
            // #i34640# - determine correct page frame
            SwPageFrame* pPage = pDrawVirtObj->AnchoredObj().FindPageFrameOfAnchor();
            if( pOldBoundRect && pPage )
            {
                SwRect aOldRect( *pOldBoundRect );
                aOldRect.Pos() += pDrawVirtObj->GetOffset();
                if( aOldRect.HasArea() )
                    ::Notify_Background( pDrawVirtObj, pPage,
                                         aOldRect, PREP_FLY_LEAVE,true);
            }
            // #i34640# - include spacing for wrapping
            SwRect aRect( pDrawVirtObj->GetAnchoredObj().GetObjRectWithSpaces() );
            if (aRect.HasArea() && pPage)
            {
                SwPageFrame* pPg = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(::FindPage( aRect, pPage )));
                if ( pPg )
                    ::Notify_Background( pDrawVirtObj, pPg, aRect,
                                         PREP_FLY_ARRIVE, true );
            }
            ::ClrContourCache( pDrawVirtObj );
        }
    }
}

/// local method to notify the background for a drawing object - #i26791#
static void lcl_NotifyBackgroundOfObj( SwDrawContact& _rDrawContact,
                                const SdrObject& _rObj,
                                const Rectangle* _pOldObjRect )
{
    // #i34640#
    SwAnchoredObject* pAnchoredObj =
        const_cast<SwAnchoredObject*>(_rDrawContact.GetAnchoredObj( &_rObj ));
    if ( pAnchoredObj && pAnchoredObj->GetAnchorFrame() )
    {
        // #i34640# - determine correct page frame
        SwPageFrame* pPageFrame = pAnchoredObj->FindPageFrameOfAnchor();
        if( _pOldObjRect && pPageFrame )
        {
            SwRect aOldRect( *_pOldObjRect );
            if( aOldRect.HasArea() )
            {
                // #i34640# - determine correct page frame
                SwPageFrame* pOldPageFrame = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(::FindPage( aOldRect, pPageFrame )));
                ::Notify_Background( &_rObj, pOldPageFrame, aOldRect,
                                     PREP_FLY_LEAVE, true);
            }
        }
        // #i34640# - include spacing for wrapping
        SwRect aNewRect( pAnchoredObj->GetObjRectWithSpaces() );
        if( aNewRect.HasArea() && pPageFrame )
        {
            pPageFrame = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(::FindPage( aNewRect, pPageFrame )));
            ::Notify_Background( &_rObj, pPageFrame, aNewRect,
                                 PREP_FLY_ARRIVE, true );
        }
        ClrContourCache( &_rObj );
    }
}

void SwDrawContact::Changed( const SdrObject& rObj,
                             SdrUserCallType eType,
                             const Rectangle& rOldBoundRect )
{
    // #i26791# - no event handling, if existing <SwViewShell>
    // is in construction
    SwDoc* pDoc = GetFormat()->GetDoc();
    if ( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() &&
         pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->IsInConstructor() )
    {
        return;
    }

    // #i44339#
    // no event handling, if document is in destruction.
    // Exception: It's the SDRUSERCALL_DELETE event
    if ( pDoc->IsInDtor() && eType != SDRUSERCALL_DELETE )
    {
        return;
    }

    //Put on Action, but not if presently anywhere an action runs.
    bool bHasActions(true);
    SwRootFrame *pTmpRoot = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if ( pTmpRoot && pTmpRoot->IsCallbackActionEnabled() )
    {
        SwViewShell* const pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
        if ( pSh )
        {
            for(SwViewShell& rShell : pSh->GetRingContainer() )
            {
                if ( rShell.Imp()->IsAction() || rShell.Imp()->IsIdleAction() )
                {
                    bHasActions = true;
                    break;
                }
                bHasActions = false;
            }
        }
        if(!bHasActions)
            pTmpRoot->StartAllAction();
    }
    SdrObjUserCall::Changed( rObj, eType, rOldBoundRect );
    _Changed( rObj, eType, &rOldBoundRect );    //Attention, possibly suicidal!

    if(!bHasActions)
        pTmpRoot->EndAllAction();
}

/// helper class for method <SwDrawContact::_Changed(..)> for handling nested
/// <SdrObjUserCall> events
class NestedUserCallHdl
{
    private:
        SwDrawContact* mpDrawContact;
        bool mbParentUserCallActive;
        SdrUserCallType meParentUserCallEventType;

    public:
        NestedUserCallHdl( SwDrawContact* _pDrawContact,
                           SdrUserCallType _eEventType )
            : mpDrawContact( _pDrawContact ),
              mbParentUserCallActive( _pDrawContact->mbUserCallActive ),
              meParentUserCallEventType( _pDrawContact->meEventTypeOfCurrentUserCall )
        {
            mpDrawContact->mbUserCallActive = true;
            mpDrawContact->meEventTypeOfCurrentUserCall = _eEventType;
        }

        ~NestedUserCallHdl()
        {
            if ( mpDrawContact )
            {
                mpDrawContact->mbUserCallActive = mbParentUserCallActive;
                mpDrawContact->meEventTypeOfCurrentUserCall = meParentUserCallEventType;
            }
        }

        void DrawContactDeleted()
        {
            mpDrawContact = nullptr;
        }

        bool IsNestedUserCall() const
        {
            return mbParentUserCallActive;
        }

        void AssertNestedUserCall()
        {
            if ( IsNestedUserCall() )
            {
                bool bTmpAssert( true );
                // Currently its known, that a nested event SDRUSERCALL_RESIZE
                // could occur during parent user call SDRUSERCALL_INSERTED,
                // SDRUSERCALL_DELETE and SDRUSERCALL_RESIZE for edge objects.
                // Also possible are nested SDRUSERCALL_CHILD_RESIZE events for
                // edge objects
                // Thus, assert all other combinations
                if ( ( meParentUserCallEventType == SDRUSERCALL_INSERTED ||
                       meParentUserCallEventType == SDRUSERCALL_DELETE ||
                       meParentUserCallEventType == SDRUSERCALL_RESIZE ) &&
                     mpDrawContact->meEventTypeOfCurrentUserCall == SDRUSERCALL_RESIZE )
                {
                    bTmpAssert = false;
                }
                else if ( meParentUserCallEventType == SDRUSERCALL_CHILD_RESIZE &&
                          mpDrawContact->meEventTypeOfCurrentUserCall == SDRUSERCALL_CHILD_RESIZE )
                {
                    bTmpAssert = false;
                }

                if ( bTmpAssert )
                {
                    OSL_FAIL( "<SwDrawContact::_Changed(..)> - unknown nested <UserCall> event. This is serious." );
                }
            }
        }
};

/// Notify the format's textbox that it should reconsider its position / size.
void lcl_textBoxSizeNotify(SwFrameFormat* pFormat)
{
    if (SwTextBoxHelper::findTextBox(pFormat))
    {
        // Just notify the textbox that the size has changed, the actual object size is not interesting.
        SfxItemSet aResizeSet(pFormat->GetDoc()->GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE, 0);
        SwFormatFrameSize aSize;
        aResizeSet.Put(aSize);
        SwTextBoxHelper::syncFlyFrameAttr(*pFormat, aResizeSet);
    }
}

// !!!ATTENTION!!! The object may commit suicide!!!

void SwDrawContact::_Changed( const SdrObject& rObj,
                              SdrUserCallType eType,
                              const Rectangle* pOldBoundRect )
{
    // suppress handling of nested <SdrObjUserCall> events
    NestedUserCallHdl aNestedUserCallHdl( this, eType );
    if ( aNestedUserCallHdl.IsNestedUserCall() )
    {
        aNestedUserCallHdl.AssertNestedUserCall();
        return;
    }
    // do *not* notify, if document is destructing
    // #i35912# - do *not* notify for as-character anchored
    // drawing objects.
    // #i35007#
    // improvement: determine as-character anchored object flag only once.
    const bool bAnchoredAsChar = ObjAnchoredAsChar();
    const bool bNotify = !(GetFormat()->GetDoc()->IsInDtor()) &&
                         ( SURROUND_THROUGHT != GetFormat()->GetSurround().GetSurround() ) &&
                         !bAnchoredAsChar;
    switch( eType )
    {
        case SDRUSERCALL_DELETE:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
                    // --> #i36181# - background of 'virtual'
                    // drawing objects have also been notified.
                    NotifyBackgrdOfAllVirtObjs( pOldBoundRect );
                }
                DisconnectFromLayout( false );
                SetMaster( nullptr );
                delete this;
                // --> #i65784# Prevent memory corruption
                aNestedUserCallHdl.DrawContactDeleted();
                break;
            }
        case SDRUSERCALL_INSERTED:
            {
                if ( mbDisconnectInProgress )
                {
                    OSL_FAIL( "<SwDrawContact::_Changed(..)> - Insert event during disconnection from layout is invalid." );
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
        case SDRUSERCALL_CHILD_INSERTED :
        case SDRUSERCALL_CHILD_REMOVED :
        {
            // --> #i113730#
            // force layer of controls for group objects containing control objects
            if(dynamic_cast< SdrObjGroup* >(maAnchoredDrawObj.DrawObj()))
            {
                if(::CheckControlLayer(maAnchoredDrawObj.DrawObj()))
                {
                    const IDocumentDrawModelAccess& rIDDMA = static_cast<SwFrameFormat*>(GetRegisteredInNonConst())->getIDocumentDrawModelAccess();
                    const SdrLayerID aCurrentLayer(maAnchoredDrawObj.DrawObj()->GetLayer());
                    const SdrLayerID aControlLayerID(rIDDMA.GetControlsId());
                    const SdrLayerID aInvisibleControlLayerID(rIDDMA.GetInvisibleControlsId());

                    if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
                    {
                        if ( aCurrentLayer == rIDDMA.GetInvisibleHellId() ||
                             aCurrentLayer == rIDDMA.GetInvisibleHeavenId() )
                        {
                            maAnchoredDrawObj.DrawObj()->SetLayer(aInvisibleControlLayerID);
                        }
                        else
                        {
                            maAnchoredDrawObj.DrawObj()->SetLayer(aControlLayerID);
                        }
                    }
                }
            }
            // fallthrough intended here
        }
        case SDRUSERCALL_MOVEONLY:
        case SDRUSERCALL_RESIZE:
        case SDRUSERCALL_CHILD_MOVEONLY :
        case SDRUSERCALL_CHILD_RESIZE :
        case SDRUSERCALL_CHILD_CHGATTR :
        case SDRUSERCALL_CHILD_DELETE :
        case SDRUSERCALL_CHILD_COPY :
        {
            // #i31698# - improvement
            // get instance <SwAnchoredDrawObject> only once
            const SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<const SwAnchoredDrawObject*>( GetAnchoredObj( &rObj ) );

            /* protect against NULL pointer dereferencing */
            if(!pAnchoredDrawObj)
            {
                break;
            }

            // #i26791# - adjust positioning and alignment attributes,
            // if positioning of drawing object isn't in progress.
            // #i53320# - no adjust of positioning attributes,
            // if drawing object isn't positioned.
            if ( !pAnchoredDrawObj->IsPositioningInProgress() &&
                 !pAnchoredDrawObj->NotYetPositioned() )
            {
                // #i34748# - If no last object rectangle is
                // provided by the anchored object, use parameter <pOldBoundRect>.
                const Rectangle& aOldObjRect = pAnchoredDrawObj->GetLastObjRect()
                                               ? *(pAnchoredDrawObj->GetLastObjRect())
                                               : *(pOldBoundRect);
                // #i79400#
                // always invalidate object rectangle inclusive spaces
                pAnchoredDrawObj->InvalidateObjRectWithSpaces();
                // #i41324# - notify background before
                // adjusting position
                if ( bNotify )
                {
                    // #i31573# - correction
                    // background of given drawing object.
                    lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect );
                }
                // #i31698# - determine layout direction
                // via draw frame format.
                SwFrameFormat::tLayoutDir eLayoutDir =
                                pAnchoredDrawObj->GetFrameFormat().GetLayoutDir();
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
                switch ( eLayoutDir )
                {
                    case SwFrameFormat::HORI_L2R:
                    {
                        nXPosDiff = aObjRect.Left() - aOldObjRect.Left();
                        nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                    }
                    break;
                    case SwFrameFormat::HORI_R2L:
                    {
                        nXPosDiff = aOldObjRect.Right() - aObjRect.Right();
                        nYPosDiff = aObjRect.Top() - aOldObjRect.Top();
                    }
                    break;
                    case SwFrameFormat::VERT_R2L:
                    {
                        nXPosDiff = aObjRect.Top() - aOldObjRect.Top();
                        nYPosDiff = aOldObjRect.Right() - aObjRect.Right();
                    }
                    break;
                    default:
                    {
                        OSL_FAIL( "<SwDrawContact::_Changed(..)> - unsupported layout direction" );
                    }
                }
                SfxItemSet aSet( GetFormat()->GetDoc()->GetAttrPool(),
                                 RES_VERT_ORIENT, RES_HORI_ORIENT, 0 );
                const SwFormatVertOrient& rVert = GetFormat()->GetVertOrient();
                if ( nYPosDiff != 0 )
                {

                    if ( rVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                         rVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nYPosDiff = -nYPosDiff;
                    }
                    aSet.Put( SwFormatVertOrient( rVert.GetPos()+nYPosDiff,
                                               text::VertOrientation::NONE,
                                               rVert.GetRelationOrient() ) );
                }

                const SwFormatHoriOrient& rHori = GetFormat()->GetHoriOrient();
                if ( !bAnchoredAsChar && nXPosDiff != 0 )
                {
                    aSet.Put( SwFormatHoriOrient( rHori.GetPos()+nXPosDiff,
                                               text::HoriOrientation::NONE,
                                               rHori.GetRelationOrient() ) );
                }

                if ( nYPosDiff ||
                     ( !bAnchoredAsChar && nXPosDiff != 0 ) )
                {
                    GetFormat()->GetDoc()->SetFlyFrameAttr( *(GetFormat()), aSet );
                    // keep new object rectangle, to avoid multiple
                    // changes of the attributes by multiple event from
                    // the drawing layer - e.g. group objects and its members
                    // #i34748# - use new method
                    // <SwAnchoredDrawObject::SetLastObjRect(..)>.
                    const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)
                                    ->SetLastObjRect( aObjRect.SVRect() );
                }
                else if ( aObjRect.SSize() != aOldObjRect.GetSize() )
                {
                    _InvalidateObjs();
                    // #i35007# - notify anchor frame
                    // of as-character anchored object
                    if ( bAnchoredAsChar )
                    {
                        SwFrame* pAnchorFrame = const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)->AnchorFrame();
                        if(pAnchorFrame)
                        {
                            pAnchorFrame->Prepare( PREP_FLY_ATTR_CHG, GetFormat() );
                        }
                    }

                    lcl_textBoxSizeNotify(GetFormat());
                }
                else if (eType == SDRUSERCALL_RESIZE)
                    // Even if the bounding box of the shape didn't change,
                    // notify about the size change, as an adjustment change
                    // may affect the size of the underlying textbox.
                    lcl_textBoxSizeNotify(GetFormat());
            }
        }
        break;
        case SDRUSERCALL_CHGATTR:
            if ( bNotify )
            {
                lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
            }
            break;
        default:
            break;
    }
}

namespace
{
    static const SwFormatAnchor* lcl_getAnchorFormat( const SfxPoolItem& _rItem )
    {
        sal_uInt16 nWhich = _rItem.Which();
        const SwFormatAnchor* pAnchorFormat = nullptr;
        if ( RES_ATTRSET_CHG == nWhich )
        {
            static_cast<const SwAttrSetChg&>(_rItem).GetChgSet()->
                GetItemState( RES_ANCHOR, false, reinterpret_cast<const SfxPoolItem**>(&pAnchorFormat) );
        }
        else if ( RES_ANCHOR == nWhich )
        {
            pAnchorFormat = &static_cast<const SwFormatAnchor&>(_rItem);
        }
        return pAnchorFormat;
    }
}

void SwDrawContact::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    OSL_ENSURE( !mbDisconnectInProgress,
            "<SwDrawContact::Modify(..)> called during disconnection.");

    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFormatAnchor* pNewAnchorFormat = pNew ? lcl_getAnchorFormat( *pNew ) : nullptr;

    if ( pNewAnchorFormat )
    {
        // Do not respond to a Reset Anchor !!!!!
        if ( SfxItemState::SET ==
                GetFormat()->GetAttrSet().GetItemState( RES_ANCHOR, false ) )
        {
            // no connect to layout during disconnection
            if ( !mbDisconnectInProgress )
            {
                // determine old object rectangle of 'master' drawing object
                // for notification
                const Rectangle* pOldRect = nullptr;
                Rectangle aOldRect;
                if ( GetAnchorFrame() )
                {
                    // --> #i36181# - include spacing in object
                    // rectangle for notification.
                    aOldRect = maAnchoredDrawObj.GetObjRectWithSpaces().SVRect();
                    pOldRect = &aOldRect;
                }
                // re-connect to layout due to anchor format change
                ConnectToLayout( pNewAnchorFormat );
                // notify background of drawing objects
                lcl_NotifyBackgroundOfObj( *this, *GetMaster(), pOldRect );
                NotifyBackgrdOfAllVirtObjs( pOldRect );

                const SwFormatAnchor* pOldAnchorFormat = pOld ? lcl_getAnchorFormat( *pOld ) : nullptr;
                if ( !pOldAnchorFormat || ( pOldAnchorFormat->GetAnchorId() != pNewAnchorFormat->GetAnchorId() ) )
                {
                    OSL_ENSURE( maAnchoredDrawObj.DrawObj(), "SwDrawContact::Modify: no draw object here?" );
                    if ( maAnchoredDrawObj.DrawObj() )
                    {
                        // --> #i102752#
                        // assure that a ShapePropertyChangeNotifier exists
                        maAnchoredDrawObj.DrawObj()->notifyShapePropertyChange( svx::eTextShapeAnchorType );
                    }
                }
            }
        }
        else
            DisconnectFromLayout();
    }
    // --> #i62875# - no further notification, if not connected to Writer layout
    else if ( maAnchoredDrawObj.GetAnchorFrame() &&
              maAnchoredDrawObj.GetDrawObj()->GetUserCall() )
    {
        // --> #i28701# - on change of wrapping style, hell|heaven layer,
        // or wrapping style influence an update of the <SwSortedObjs> list,
        // the drawing object is registered in, has to be performed. This is triggered
        // by the 1st parameter of method call <_InvalidateObjs(..)>.
        if ( RES_SURROUND == nWhich ||
             RES_OPAQUE == nWhich ||
             RES_WRAP_INFLUENCE_ON_OBJPOS == nWhich ||
             ( RES_ATTRSET_CHG == nWhich &&
               ( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                           RES_SURROUND, false ) ||
                 SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                           RES_OPAQUE, false ) ||
                 SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                           RES_WRAP_INFLUENCE_ON_OBJPOS, false ) ) ) )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), nullptr );
            NotifyBackgrdOfAllVirtObjs( nullptr );
            _InvalidateObjs( true );
        }
        else if ( RES_UL_SPACE == nWhich || RES_LR_SPACE == nWhich ||
                  RES_HORI_ORIENT == nWhich || RES_VERT_ORIENT == nWhich ||
                  // #i28701# - add attribute 'Follow text flow'
                  RES_FOLLOW_TEXT_FLOW == nWhich ||
                  ( RES_ATTRSET_CHG == nWhich &&
                    ( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                                RES_LR_SPACE, false ) ||
                      SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                                RES_UL_SPACE, false ) ||
                      SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                                RES_HORI_ORIENT, false ) ||
                      SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                                RES_VERT_ORIENT, false ) ||
                      SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
                                RES_FOLLOW_TEXT_FLOW, false ) ) ) )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), nullptr );
            NotifyBackgrdOfAllVirtObjs( nullptr );
            _InvalidateObjs();
        }
        // #i35443#
        else if ( RES_ATTRSET_CHG == nWhich )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), nullptr );
            NotifyBackgrdOfAllVirtObjs( nullptr );
            _InvalidateObjs();
        }
        else if ( RES_REMOVE_UNO_OBJECT == nWhich )
        {
            // nothing to do
        }
#if OSL_DEBUG_LEVEL > 0
        else
        {
            OSL_FAIL( "<SwDrawContact::Modify(..)> - unhandled attribute? - please inform od@openoffice.org" );
        }
#endif
    }

    // #i51474#
    GetAnchoredObj( nullptr )->ResetLayoutProcessBools();
}

// #i26791#
// #i28701# - added parameter <_bUpdateSortedObjsList>
void SwDrawContact::_InvalidateObjs( const bool _bUpdateSortedObjsList )
{
    // invalidate position of existing 'virtual' drawing objects
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        // #i33313# - invalidation only for connected
        // 'virtual' drawing objects
        if ( pDrawVirtObj->IsConnected() )
        {
            pDrawVirtObj->AnchoredObj().InvalidateObjPos();
            // #i28701#
            if ( _bUpdateSortedObjsList )
            {
                pDrawVirtObj->AnchoredObj().UpdateObjInSortedList();
            }
        }
    }

    // invalidate position of 'master' drawing object
    SwAnchoredObject* pAnchoredObj = GetAnchoredObj( nullptr );
    pAnchoredObj->InvalidateObjPos();
    // #i28701#
    if ( _bUpdateSortedObjsList )
    {
        pAnchoredObj->UpdateObjInSortedList();
    }
}

void SwDrawContact::DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer )
{
    mbDisconnectInProgress = true;

    // --> #i36181# - notify background of drawing object
    if ( _bMoveMasterToInvisibleLayer &&
         !(GetFormat()->GetDoc()->IsInDtor()) &&
         GetAnchorFrame() && !GetAnchorFrame()->IsInDtor() )
    {
        const Rectangle aOldRect( maAnchoredDrawObj.GetObjRectWithSpaces().SVRect() );
        lcl_NotifyBackgroundOfObj( *this, *GetMaster(), &aOldRect );
        NotifyBackgrdOfAllVirtObjs( &aOldRect );
    }

    // remove 'virtual' drawing objects from writer
    // layout and from drawing page
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        pDrawVirtObj->RemoveFromWriterLayout();
        pDrawVirtObj->RemoveFromDrawingPage();
    }

    if ( maAnchoredDrawObj.GetAnchorFrame() )
    {
        maAnchoredDrawObj.AnchorFrame()->RemoveDrawObj( maAnchoredDrawObj );
    }

    if ( _bMoveMasterToInvisibleLayer && GetMaster() && GetMaster()->IsInserted() )
    {
        SdrViewIter aIter( GetMaster() );
        for( SdrView* pView = aIter.FirstView(); pView;
                    pView = aIter.NextView() )
        {
            pView->MarkObj( GetMaster(), pView->GetSdrPageView(), true );
        }

        // Instead of removing 'master' object from drawing page, move the
        // 'master' drawing object into the corresponding invisible layer.
        {
            //static_cast<SwFrameFormat*>(GetRegisteredIn())->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
            //                            RemoveObject( GetMaster()->GetOrdNum() );
            // #i18447# - in order to consider group object correct
            // use new method <SwDrawContact::MoveObjToInvisibleLayer(..)>
            MoveObjToInvisibleLayer( GetMaster() );
        }
    }

    mbDisconnectInProgress = false;
}

/// method to remove 'master' drawing object from drawing page.
void SwDrawContact::RemoveMasterFromDrawPage()
{
    if ( GetMaster() )
    {
        GetMaster()->SetUserCall( nullptr );
        if ( GetMaster()->IsInserted() )
        {
            static_cast<SwFrameFormat*>(GetRegisteredIn())->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0)->
                                        RemoveObject( GetMaster()->GetOrdNum() );
        }
    }
}

// disconnect for a dedicated drawing object - could be 'master' or 'virtual'.
// a 'master' drawing object will disconnect a 'virtual' drawing object
// in order to take its place.
// #i19919# - no special case, if drawing object isn't in
// page header/footer, in order to get drawing objects in repeating table headers
// also working.
void SwDrawContact::DisconnectObjFromLayout( SdrObject* _pDrawObj )
{
    if ( dynamic_cast<const SwDrawVirtObj*>( _pDrawObj) !=  nullptr )
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
            SwFrame* pNewAnchorFrameOfMaster = pDrawVirtObj->AnchorFrame();
            // disconnect 'virtual' drawing object
            pDrawVirtObj->RemoveFromWriterLayout();
            pDrawVirtObj->RemoveFromDrawingPage();
            // disconnect 'master' drawing object from current frame
            GetAnchorFrame()->RemoveDrawObj( maAnchoredDrawObj );
            // re-connect 'master' drawing object to frame of found 'virtual'
            // drawing object.
            pNewAnchorFrameOfMaster->AppendDrawObj( maAnchoredDrawObj );
        }
        else
        {
            // no connected 'virtual' drawing object found. Thus, disconnect
            // completely from layout.
            DisconnectFromLayout();
        }
    }
}

static SwTextFrame* lcl_GetFlyInContentAnchor( SwTextFrame* _pProposedAnchorFrame,
                                   const sal_Int32 _nTextOfs )
{
    SwTextFrame* pAct = _pProposedAnchorFrame;
    SwTextFrame* pTmp;
    do
    {
        pTmp = pAct;
        pAct = pTmp->GetFollow();
    }
    while( pAct && _nTextOfs >= pAct->GetOfst() );
    return pTmp;
}

void SwDrawContact::ConnectToLayout( const SwFormatAnchor* pAnch )
{
    // *no* connect to layout during disconnection from layout.
    if ( mbDisconnectInProgress )
    {
        OSL_FAIL( "<SwDrawContact::ConnectToLayout(..)> called during disconnection.");
        return;
    }

    // --> #i33909# - *no* connect to layout, if 'master' drawing
    // object isn't inserted in the drawing page
    if ( !GetMaster()->IsInserted() )
    {
        OSL_FAIL( "<SwDrawContact::ConnectToLayout(..)> - master drawing object not inserted -> no connect to layout. Please inform od@openoffice.org" );
        return;
    }

    SwFrameFormat* pDrawFrameFormat = static_cast<SwFrameFormat*>(GetRegisteredIn());

    if( !pDrawFrameFormat->getIDocumentLayoutAccess().GetCurrentViewShell() )
        return;

    // remove 'virtual' drawing objects from writer
    // layout and from drawing page, and remove 'master' drawing object from
    // writer layout - 'master' object will remain in drawing page.
    DisconnectFromLayout( false );

    if ( !pAnch )
    {
        pAnch = &(pDrawFrameFormat->GetAnchor());
    }

    switch ( pAnch->GetAnchorId() )
    {
        case FLY_AT_PAGE:
                {
                sal_uInt16 nPgNum = pAnch->GetPageNum();
                SwViewShell *pShell = pDrawFrameFormat->getIDocumentLayoutAccess().GetCurrentViewShell();
                if( !pShell )
                    break;
                SwRootFrame* pRoot = pShell->GetLayout();
                SwPageFrame *pPage = static_cast<SwPageFrame*>(pRoot->Lower());

                for ( sal_uInt16 i = 1; i < nPgNum && pPage; ++i )
                {
                    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                }

                if ( pPage )
                {
                    pPage->AppendDrawObj( maAnchoredDrawObj );
                }
                else
                    //Looks stupid but is allowed (compare SwFEShell::SetPageObjsNewPage)
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
                // support drawing objects in header/footer,
                // but not control objects:
                // anchor at first found frame the 'master' object and
                // at the following frames 'virtual' drawing objects.
                // Note: method is similar to <SwFlyFrameFormat::MakeFrames(..)>
                SwModify *pModify = nullptr;
                if( pAnch->GetContentAnchor() )
                {
                    if ( pAnch->GetAnchorId() == FLY_AT_FLY )
                    {
                        SwNodeIndex aIdx( pAnch->GetContentAnchor()->nNode );
                        SwContentNode* pCNd = pDrawFrameFormat->GetDoc()->GetNodes().GoNext( &aIdx );
                        if ( SwIterator<SwFrame,SwContentNode>( *pCNd ).First() )
                            pModify = pCNd;
                        else
                        {
                            const SwNodeIndex& rIdx = pAnch->GetContentAnchor()->nNode;
                            SwFrameFormats& rFormats = *(pDrawFrameFormat->GetDoc()->GetSpzFrameFormats());
                            for( auto pFlyFormat : rFormats )
                            {
                                if( pFlyFormat->GetContent().GetContentIdx() &&
                                    rIdx == *(pFlyFormat->GetContent().GetContentIdx()) )
                                {
                                    pModify = pFlyFormat;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        pModify = pAnch->GetContentAnchor()->nNode.GetNode().GetContentNode();
                    }
                }

                // #i29199# - It is possible, that
                // the anchor doesn't exist - E.g., reordering the
                // sub-documents in a master document.
                // Note: The anchor will be inserted later.
                if ( !pModify )
                {
                    // break to end of the current switch case.
                    break;
                }

                SwIterator<SwFrame,SwModify> aIter( *pModify );
                SwFrame* pAnchorFrameOfMaster = nullptr;
                for( SwFrame *pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
                {
                    // append drawing object, if
                    // (1) proposed anchor frame isn't a follow and
                    // (2) drawing object isn't a control object to be anchored
                    //     in header/footer.
                    const bool bAdd = ( !pFrame->IsContentFrame() ||
                                        !static_cast<SwContentFrame*>(pFrame)->IsFollow() ) &&
                                      ( !::CheckControlLayer( GetMaster() ) ||
                                        !pFrame->FindFooterOrHeader() );

                    if( bAdd )
                    {
                        if ( FLY_AT_FLY == pAnch->GetAnchorId() && !pFrame->IsFlyFrame() )
                        {
                            pFrame = pFrame->FindFlyFrame();
                            OSL_ENSURE( pFrame,
                                    "<SwDrawContact::ConnectToLayout(..)> - missing fly frame -> crash." );
                        }

                        // find correct follow for as character anchored objects
                        if ((pAnch->GetAnchorId() == FLY_AS_CHAR) &&
                             pFrame->IsTextFrame() )
                        {
                            pFrame = lcl_GetFlyInContentAnchor(
                                        static_cast<SwTextFrame*>(pFrame),
                                        pAnch->GetContentAnchor()->nContent.GetIndex() );
                        }

                        if ( !pAnchorFrameOfMaster )
                        {
                            // append 'master' drawing object
                            pAnchorFrameOfMaster = pFrame;
                            pFrame->AppendDrawObj( maAnchoredDrawObj );
                        }
                        else
                        {
                            // append 'virtual' drawing object
                            SwDrawVirtObj* pDrawVirtObj = AddVirtObj();
                            if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
                            {
                                ClrContourCache( pDrawVirtObj );
                            }
                            pFrame->AppendDrawObj( pDrawVirtObj->AnchoredObj() );

                            pDrawVirtObj->ActionChanged();
                        }

                        if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
                        {
                            pFrame->InvalidatePrt();
                        }
                    }
                }
            }
            break;
        default:
            OSL_FAIL( "Unknown Anchor." );
            break;
    }
    if ( GetAnchorFrame() )
    {
        ::setContextWritingMode( maAnchoredDrawObj.DrawObj(), GetAnchorFrame() );
        // #i26791# - invalidate objects instead of direct positioning
        _InvalidateObjs();
    }
}

/// insert 'master' drawing object into drawing page
void SwDrawContact::InsertMasterIntoDrawPage()
{
    if ( !GetMaster()->IsInserted() )
    {
        GetFormat()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0)
                ->InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
    }
    GetMaster()->SetUserCall( this );
}

SwPageFrame* SwDrawContact::FindPage( const SwRect &rRect )
{
    // --> #i28701# - use method <GetPageFrame()>
    SwPageFrame* pPg = GetPageFrame();
    if ( !pPg && GetAnchorFrame() )
        pPg = GetAnchorFrame()->FindPageFrame();
    if ( pPg )
        pPg = const_cast<SwPageFrame*>(static_cast<const SwPageFrame*>(::FindPage( rRect, pPg )));
    return pPg;
}

void SwDrawContact::ChkPage()
{
    if ( mbDisconnectInProgress )
    {
        OSL_FAIL( "<SwDrawContact::ChkPage()> called during disconnection." );
        return;
    }

    // --> #i28701#
    SwPageFrame* pPg = ( maAnchoredDrawObj.GetAnchorFrame() &&
                       maAnchoredDrawObj.GetAnchorFrame()->IsPageFrame() )
                     ? GetPageFrame()
                     : FindPage( GetMaster()->GetCurrentBoundRect() );
    if ( GetPageFrame() != pPg )
    {
        // if drawing object is anchor in header/footer a change of the page
        // is a dramatic change. Thus, completely re-connect to the layout
        if ( maAnchoredDrawObj.GetAnchorFrame() &&
             maAnchoredDrawObj.GetAnchorFrame()->FindFooterOrHeader() )
        {
            ConnectToLayout();
        }
        else
        {
            // --> #i28701# - use methods <GetPageFrame()> and <SetPageFrame>
            if ( GetPageFrame() )
                GetPageFrame()->RemoveDrawObjFromPage( maAnchoredDrawObj );
            pPg->AppendDrawObjToPage( maAnchoredDrawObj );
            SetPageFrame( pPg );
        }
    }
}

// Important note:
// method is called by method <SwDPage::ReplaceObject(..)>, which called its
// corresponding superclass method <FmFormPage::ReplaceObject(..)>.
// Note: 'master' drawing object *has* to be connected to layout triggered
//       by the caller of this, if method is called.
void SwDrawContact::ChangeMasterObject( SdrObject *pNewMaster )
{
    DisconnectFromLayout( false );
    // consider 'virtual' drawing objects
    RemoveAllVirtObjs();

    GetMaster()->SetUserCall( nullptr );
    SetMaster( pNewMaster );
    GetMaster()->SetUserCall( this );

    _InvalidateObjs();
}

/// get data collection of anchored objects, handled by with contact
void SwDrawContact::GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const
{
    _roAnchoredObjs.push_back( const_cast<SwAnchoredDrawObject*>(&maAnchoredDrawObj) );

    for ( std::list<SwDrawVirtObj*>::const_iterator aDrawVirtObjsIter = maDrawVirtObjs.begin();
          aDrawVirtObjsIter != maDrawVirtObjs.end();
          ++aDrawVirtObjsIter )
    {
        _roAnchoredObjs.push_back( &(*aDrawVirtObjsIter)->AnchoredObj() );
    }
}

// AW: own sdr::contact::ViewContact (VC) sdr::contact::ViewObjectContact (VOC) needed
// since offset is defined different from SdrVirtObj's sdr::contact::ViewContactOfVirtObj.
// For paint, that offset is used by setting at the OutputDevice; for primitives this is
// not possible since we have no OutputDevice, but define the geometry itself.

namespace sdr
{
    namespace contact
    {
        class VOCOfDrawVirtObj : public ViewObjectContactOfSdrObj
        {
        protected:
            /**
             * This method is responsible for creating the graphical visualisation data which is
             * stored/cached in the local primitive. Default gets view-independent Primitive from
             * the ViewContact using ViewContact::getViewIndependentPrimitive2DSequence(), takes
             * care of visibility, handles glue and ghosted.
             *
             * This method will not handle included hierarchies and not check geometric visibility.
             */
            virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        public:
            VOCOfDrawVirtObj(ObjectContact& rObjectContact, ViewContact& rViewContact)
            :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
            {
            }

            virtual ~VOCOfDrawVirtObj();
        };

        class VCOfDrawVirtObj : public ViewContactOfVirtObj
        {
        protected:
            /** Create a Object-Specific ViewObjectContact, set ViewContact and ObjectContact.
             *
             * Always needs to return something. Default is to create a standard ViewObjectContact
             * containing the given ObjectContact and *this.
             */
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

        public:
            /// basic constructor, used from SdrObject.
            explicit VCOfDrawVirtObj(SwDrawVirtObj& rObj)
            :   ViewContactOfVirtObj(rObj)
            {
            }
            virtual ~VCOfDrawVirtObj();

            /// access to SwDrawVirtObj
            SwDrawVirtObj& GetSwDrawVirtObj() const
            {
                return static_cast<SwDrawVirtObj&>(mrObject);
            }
        };
    } // end of namespace contact
} // end of namespace sdr

namespace sdr
{
    namespace contact
    {
        /// recursively collect primitive data from given VOC with given offset
        void impAddPrimitivesFromGroup(const ViewObjectContact& rVOC, const basegfx::B2DHomMatrix& rOffsetMatrix, const DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DContainer& rxTarget)
        {
            const sal_uInt32 nSubHierarchyCount(rVOC.GetViewContact().GetObjectCount());

            for(sal_uInt32 a(0L); a < nSubHierarchyCount; a++)
            {
                const ViewObjectContact& rCandidate(rVOC.GetViewContact().GetViewContact(a).GetViewObjectContact(rVOC.GetObjectContact()));

                if(rCandidate.GetViewContact().GetObjectCount())
                {
                    // is a group object itself, call recursively
                    impAddPrimitivesFromGroup(rCandidate, rOffsetMatrix, rDisplayInfo, rxTarget);
                }
                else
                {
                    // single object, add primitives; check model-view visibility
                    if(rCandidate.isPrimitiveVisible(rDisplayInfo))
                    {
                        drawinglayer::primitive2d::Primitive2DContainer aNewSequence(rCandidate.getPrimitive2DSequence(rDisplayInfo));

                        if(!aNewSequence.empty())
                        {
                            // get ranges
                            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(rCandidate.GetObjectContact().getViewInformation2D());
                            const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());
                            basegfx::B2DRange aObjectRange(rCandidate.getObjectRange());

                            // correct with virtual object's offset
                            aObjectRange.transform(rOffsetMatrix);

                            // check geometrical visibility (with offset)
                            if(!aViewRange.overlaps(aObjectRange))
                            {
                                // not visible, release
                                aNewSequence.clear();
                            }
                        }

                        if(!aNewSequence.empty())
                        {
                            rxTarget.append(aNewSequence);
                        }
                    }
                }
            }
        }

        drawinglayer::primitive2d::Primitive2DContainer VOCOfDrawVirtObj::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            const VCOfDrawVirtObj& rVC = static_cast< const VCOfDrawVirtObj& >(GetViewContact());
            const SdrObject& rReferencedObject = rVC.GetSwDrawVirtObj().GetReferencedObj();
            drawinglayer::primitive2d::Primitive2DContainer xRetval;

            // create offset transformation
            basegfx::B2DHomMatrix aOffsetMatrix;
            const Point aLocalOffset(rVC.GetSwDrawVirtObj().GetOffset());

            if(aLocalOffset.X() || aLocalOffset.Y())
            {
                aOffsetMatrix.set(0, 2, aLocalOffset.X());
                aOffsetMatrix.set(1, 2, aLocalOffset.Y());
            }

            if(dynamic_cast<const SdrObjGroup*>( &rReferencedObject) !=  nullptr)
            {
                // group object. Since the VOC/OC/VC hierarchy does not represent the
                // hierarchy virtual objects when they have group objects
                // (ViewContactOfVirtObj::GetObjectCount() returns null for that purpose)
                // to avoid multiple usages of VOCs (which would not work), the primitives
                // for the sub-hierarchy need to be collected here

                // Get the VOC of the referenced object (the Group) and fetch primitives from it
                const ViewObjectContact& rVOCOfRefObj = rReferencedObject.GetViewContact().GetViewObjectContact(GetObjectContact());
                impAddPrimitivesFromGroup(rVOCOfRefObj, aOffsetMatrix, rDisplayInfo, xRetval);
            }
            else
            {
                // single object, use method from referenced object to get the Primitive2DSequence
                xRetval = rReferencedObject.GetViewContact().getViewIndependentPrimitive2DSequence();
            }

            if(!xRetval.empty())
            {
                // create transform primitive
                const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::TransformPrimitive2D(aOffsetMatrix, xRetval));
                xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };
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

/// implementation of class <SwDrawVirtObj>

sdr::contact::ViewContact* SwDrawVirtObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::VCOfDrawVirtObj(*this);
}

SwDrawVirtObj::SwDrawVirtObj( SdrObject&        _rNewObj,
                              SwDrawContact&    _rDrawContact )
    : SdrVirtObj( _rNewObj ),
      // #i26791# - init new member <maAnchoredDrawObj>
      maAnchoredDrawObj(),
      mrDrawContact( _rDrawContact )
{
    // #i26791#
    maAnchoredDrawObj.SetDrawObj( *this );
    // #i35635# - set initial position out of sight
    NbcMove( Size( -16000, -16000 ) );
}

SwDrawVirtObj::~SwDrawVirtObj()
{}

SwDrawVirtObj& SwDrawVirtObj::operator=( const SwDrawVirtObj& rObj )
{
    SdrVirtObj::operator=(rObj);
    // Note: Members <maAnchoredDrawObj> and <mrDrawContact>
    //       haven't to be considered.
    return *this;
}

SwDrawVirtObj* SwDrawVirtObj::Clone() const
{
    SwDrawVirtObj* pObj = new SwDrawVirtObj( rRefObj, mrDrawContact );

    pObj->operator=( *this );
    // Note: Member <maAnchoredDrawObj> hasn't to be considered.

    return pObj;
}

const SwFrame* SwDrawVirtObj::GetAnchorFrame() const
{
    // #i26791# - use new member <maAnchoredDrawObj>
    return maAnchoredDrawObj.GetAnchorFrame();
}

SwFrame* SwDrawVirtObj::AnchorFrame()
{
    // #i26791# - use new member <maAnchoredDrawObj>
    return maAnchoredDrawObj.AnchorFrame();
}

void SwDrawVirtObj::RemoveFromWriterLayout()
{
    // remove contact object from frame for 'virtual' drawing object
    // #i26791# - use new member <maAnchoredDrawObj>
    if ( maAnchoredDrawObj.GetAnchorFrame() )
    {
        maAnchoredDrawObj.AnchorFrame()->RemoveDrawObj( maAnchoredDrawObj );
    }
}

void SwDrawVirtObj::AddToDrawingPage()
{
    // determine 'master'
    SdrObject* pOrgMasterSdrObj = mrDrawContact.GetMaster();

    // insert 'virtual' drawing object into page, set layer and user call.
    SdrPage* pDrawPg;
    // #i27030# - apply order number of referenced object
    if ( nullptr != ( pDrawPg = pOrgMasterSdrObj->GetPage() ) )
    {
        // #i27030# - apply order number of referenced object
        pDrawPg->InsertObject( this, GetReferencedObj().GetOrdNum() );
    }
    else
    {
        pDrawPg = GetPage();
        if ( pDrawPg )
        {
            pDrawPg->SetObjectOrdNum( GetOrdNumDirect(),
                                      GetReferencedObj().GetOrdNum() );
        }
        else
        {
            SetOrdNum( GetReferencedObj().GetOrdNum() );
        }
    }
    SetUserCall( &mrDrawContact );
}

void SwDrawVirtObj::RemoveFromDrawingPage()
{
    SetUserCall( nullptr );
    if ( GetPage() )
    {
        GetPage()->RemoveObject( GetOrdNum() );
    }
}

/// Is 'virtual' drawing object connected to writer layout and to drawing layer?
bool SwDrawVirtObj::IsConnected() const
{
    bool bRetVal = GetAnchorFrame() &&
                   ( GetPage() && GetUserCall() );

    return bRetVal;
}

void SwDrawVirtObj::NbcSetAnchorPos(const Point& rPnt)
{
    SdrObject::NbcSetAnchorPos( rPnt );
}

// #i97197#
// the methods relevant for positioning

const Rectangle& SwDrawVirtObj::GetCurrentBoundRect() const
{
    if(aOutRect.IsEmpty())
    {
        const_cast<SwDrawVirtObj*>(this)->RecalcBoundRect();
    }

    return aOutRect;
}

const Rectangle& SwDrawVirtObj::GetLastBoundRect() const
{
    return aOutRect;
}

const Point SwDrawVirtObj::GetOffset() const
{
    // do NOT use IsEmpty() here, there is already a useful offset
    // in the position
    if(aOutRect == Rectangle())
    {
        return Point();
    }
    else
    {
        return aOutRect.TopLeft() - GetReferencedObj().GetCurrentBoundRect().TopLeft();
    }
}

void SwDrawVirtObj::SetBoundRectDirty()
{
    // do nothing to not lose model information in aOutRect
}

void SwDrawVirtObj::RecalcBoundRect()
{
    // #i26791# - switch order of calling <GetOffset()> and
    // <ReferencedObj().GetCurrentBoundRect()>, because <GetOffset()> calculates
    // its value by the 'BoundRect' of the referenced object.

    const Point aOffset(GetOffset());
    aOutRect = ReferencedObj().GetCurrentBoundRect() + aOffset;
}

basegfx::B2DPolyPolygon SwDrawVirtObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval(rRefObj.TakeXorPoly());
    aRetval.transform(basegfx::tools::createTranslateB2DHomMatrix(GetOffset().X(), GetOffset().Y()));

    return aRetval;
}

basegfx::B2DPolyPolygon SwDrawVirtObj::TakeContour() const
{
    basegfx::B2DPolyPolygon aRetval(rRefObj.TakeContour());
    aRetval.transform(basegfx::tools::createTranslateB2DHomMatrix(GetOffset().X(), GetOffset().Y()));

    return aRetval;
}

SdrHdl* SwDrawVirtObj::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pHdl = rRefObj.GetHdl(nHdlNum);

    if(pHdl)
    {
        Point aP(pHdl->GetPos() + GetOffset());
        pHdl->SetPos(aP);
    }
    else
    {
        OSL_ENSURE(false, "Got no SdrHdl(!)");
    }

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

void SwDrawVirtObj::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    rRefObj.NbcRotate(rRef - GetOffset(), nAngle, sn, cs);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    rRefObj.NbcMirror(rRef1 - GetOffset(), rRef2 - GetOffset());
    SetRectsDirty();
}

void SwDrawVirtObj::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    rRefObj.NbcShear(rRef - GetOffset(), nAngle, tn, bVShear);
    SetRectsDirty();
}

void SwDrawVirtObj::Move(const Size& rSiz)
{
    SdrObject::Move( rSiz );
}

void SwDrawVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if(xFact.GetNumerator() != xFact.GetDenominator() || yFact.GetNumerator() != yFact.GetDenominator())
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Resize(rRef - GetOffset(), xFact, yFact, bUnsetRelative);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::Rotate(const Point& rRef, long nAngle, double sn, double cs)
{
    if(nAngle)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Rotate(rRef - GetOffset(), nAngle, sn, cs);
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

void SwDrawVirtObj::Shear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    if(nAngle)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetLastBoundRect();
        rRefObj.Shear(rRef - GetOffset(), nAngle, tn, bVShear);
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
    const_cast<SwDrawVirtObj*>(this)->aSnapRect = rRefObj.GetSnapRect();
    const_cast<SwDrawVirtObj*>(this)->aSnapRect += GetOffset();

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
    const_cast<SwDrawVirtObj*>(this)->aSnapRect = rRefObj.GetLogicRect();
    const_cast<SwDrawVirtObj*>(this)->aSnapRect += GetOffset();

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

Point SwDrawVirtObj::GetSnapPoint(sal_uInt32 i) const
{
    Point aP(rRefObj.GetSnapPoint(i));
    aP += GetOffset();

    return aP;
}

Point SwDrawVirtObj::GetPoint(sal_uInt32 i) const
{
    return Point(rRefObj.GetPoint(i) + GetOffset());
}

void SwDrawVirtObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    Point aP(rPnt);
    aP -= GetOffset();
    rRefObj.SetPoint(aP, i);
    SetRectsDirty();
}

bool SwDrawVirtObj::HasTextEdit() const
{
    return rRefObj.HasTextEdit();
}

// override 'layer' methods for 'virtual' drawing object to assure
// that layer of 'virtual' object is the layer of the referenced object.
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

bool SwDrawVirtObj::supportsFullDrag() const
{
    // call parent
    return SdrVirtObj::supportsFullDrag();
}

SdrObject* SwDrawVirtObj::getFullDragClone() const
{
    // call parent
    return SdrVirtObj::getFullDragClone();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
