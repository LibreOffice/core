/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <fmtornt.hxx>
#include <viewimp.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <node.hxx>
#include <fmtcntnt.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
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
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <com/sun/star/text/WritingMode2.hpp>
#include <switerator.hxx>
#include <algorithm>

using namespace ::com::sun::star;

TYPEINIT1( SwContact, SwClient )
TYPEINIT1( SwFlyDrawContact, SwContact )
TYPEINIT1( SwDrawContact, SwContact )

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

/** The Get reverse way: seeks the format to the specified object.
 * If the object is a SwVirtFlyDrawObj then the format of this
 * will be acquired.
 * Otherwise it is just a simple drawing object. This has a
 * UserCall and is the client of the searched format.
*/
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


SwRect GetBoundRectOfAnchoredObj( const SdrObject* pObj )
{
    SwRect aRet( pObj->GetCurrentBoundRect() );
    
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


SwContact* GetUserCall( const SdrObject* pObj )
{
    SdrObject *pTmp;
    while ( !pObj->GetUserCall() && 0 != (pTmp = pObj->GetUpGroup()) )
        pObj = pTmp;
    OSL_ENSURE( !pObj->GetUserCall() || pObj->GetUserCall()->ISA(SwContact),
            "<::GetUserCall(..)> - wrong type of found object user call." );
    return static_cast<SwContact*>(pObj->GetUserCall());
}


sal_Bool IsMarqueeTextObj( const SdrObject& rObj )
{
    SdrTextAniKind eTKind;
    return SdrInventor == rObj.GetObjInventor() &&
        OBJ_TEXT == rObj.GetObjIdentifier() &&
        ( SDRTEXTANI_SCROLL == ( eTKind = ((SdrTextObj&)rObj).GetTextAniKind())
         || SDRTEXTANI_ALTERNATE == eTKind || SDRTEXTANI_SLIDE == eTKind );
}



SwContact::SwContact( SwFrmFmt *pToRegisterIn ) :
    SwClient( pToRegisterIn ),
    mbInDTOR( false )
{}

SwContact::~SwContact()
{
    SetInDTOR();
}

bool SwContact::IsInDTOR() const
{
    return mbInDTOR;
}

void SwContact::SetInDTOR()
{
    mbInDTOR = true;
}


void SwContact::MoveObjToVisibleLayer( SdrObject* _pDrawObj )
{
    
    
    const bool bNotify( !GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) );

    _MoveObjToLayer( true, _pDrawObj );

    
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        OSL_ENSURE( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        if ( pAnchoredObj )
        {
            ::setContextWritingMode( _pDrawObj, pAnchoredObj->GetAnchorFrmContainingAnchPos() );
            
            
            if ( pAnchoredObj->GetPageFrm() )
            {
                ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrm(),
                                     pAnchoredObj->GetObjRect(), PREP_FLY_ARRIVE, sal_True );
            }

            pAnchoredObj->InvalidateObjPos();
        }
    }
}


void SwContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    
    const bool bNotify( GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) );

    _MoveObjToLayer( false, _pDrawObj );

    
    if ( bNotify )
    {
        SwAnchoredObject* pAnchoredObj = GetAnchoredObj( _pDrawObj );
        OSL_ENSURE( pAnchoredObj,
                "<SwContact::MoveObjToInvisibleLayer(..)> - missing anchored object" );
        
        
        if ( pAnchoredObj && pAnchoredObj->GetPageFrm() )
        {
            ::Notify_Background( _pDrawObj, pAnchoredObj->GetPageFrm(),
                                 pAnchoredObj->GetObjRect(), PREP_FLY_LEAVE, sal_True );
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

    const IDocumentDrawModelAccess* pIDDMA = static_cast<SwFrmFmt*>(GetRegisteredInNonConst())->getIDocumentDrawModelAccess();
    if ( !pIDDMA )
    {
        OSL_FAIL( "SwDrawContact::_MoveObjToLayer(..) - no writer document!" );
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

    if ( _pDrawObj->ISA( SdrObjGroup ) )
    {
        
        {
            
            SdrLayerID nNewLayerId = nToHellLayerId;
            if ( ::CheckControlLayer( _pDrawObj ) )
            {
                
                
                nNewLayerId = nToControlLayerId;
            }
            else if ( _pDrawObj->GetLayer() == pIDDMA->GetHeavenId() ||
                      _pDrawObj->GetLayer() == pIDDMA->GetInvisibleHeavenId() )
            {
                
                
                nNewLayerId = nToHeavenLayerId;
            }
            
            
            
            _pDrawObj->SdrObject::NbcSetLayer( nNewLayerId );
        }

        
        const SdrObjList* pLst =
                static_cast<SdrObjGroup*>(_pDrawObj)->GetSubList();
        if ( pLst )
        {
            for ( sal_uInt16 i = 0; i < pLst->GetObjCount(); ++i )
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




const SwIndex& SwContact::GetCntntAnchorIndex() const
{
    return GetCntntAnchor().nContent;
}


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



SwFlyDrawContact::SwFlyDrawContact( SwFlyFrmFmt *pToRegisterIn, SdrModel * ) :
    SwContact( pToRegisterIn )
{
    
    
    mpMasterObj = new SwFlyDrawObj;
    mpMasterObj->SetOrdNum( 0xFFFFFFFE );
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


const SwAnchoredObject* SwFlyDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    OSL_ENSURE( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( _pSdrObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == const_cast<SwFlyDrawContact*>(this),
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
    OSL_ENSURE( _pSdrObj,
            "<SwFlyDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( _pSdrObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::GetAnchoredObj(..)> - wrong object type provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this,
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
    OSL_ENSURE( _pNewMaster->ISA(SwFlyDrawObj),
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
    OSL_ENSURE( _pDrawObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::MoveObjToVisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        
        return;
    }

    SwFlyFrm* pFlyFrm = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrm();

    
    
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
            
            SdrObject* pObj = (*pFlyFrm->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToVisibleLayer( pObj );
        }
    }

    
    SwContact::MoveObjToVisibleLayer( _pDrawObj );
}

/**
 * @note Override method to control Writer fly frames, which are linked, and
 *       to assure that all objects anchored at/inside the Writer fly frame are
 *       also made invisible.
 */
void SwFlyDrawContact::MoveObjToInvisibleLayer( SdrObject* _pDrawObj )
{
    OSL_ENSURE( _pDrawObj->ISA(SwVirtFlyDrawObj),
            "<SwFlyDrawContact::MoveObjToInvisibleLayer(..)> - wrong SdrObject type -> crash" );

    if ( !GetFmt()->getIDocumentDrawModelAccess()->IsVisibleLayerId( _pDrawObj->GetLayer() ) )
    {
        
        return;
    }

    SwFlyFrm* pFlyFrm = static_cast<SwVirtFlyDrawObj*>(_pDrawObj)->GetFlyFrm();

    pFlyFrm->Unchain();
    pFlyFrm->DeleteCnt();
    if ( pFlyFrm->GetDrawObjs() )
    {
        for ( sal_uInt8 i = 0; i < pFlyFrm->GetDrawObjs()->Count(); ++i)
        {
            
            SdrObject* pObj = (*pFlyFrm->GetDrawObjs())[i]->DrawObj();
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            pContact->MoveObjToInvisibleLayer( pObj );
        }
    }

    
    SwContact::MoveObjToInvisibleLayer( _pDrawObj );
}


void SwFlyDrawContact::GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const
{
    const SwFrmFmt* pFmt = GetFmt();
    SwFlyFrm::GetAnchoredObjects( _roAnchoredObjs, *pFmt );
}



bool CheckControlLayer( const SdrObject *pObj )
{
    if ( FmFormInventor == pObj->GetObjInventor() )
        return true;
    if ( pObj->ISA( SdrObjGroup ) )
    {
        const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( sal_uInt16 i = 0; i < pLst->GetObjCount(); ++i )
        {
            if ( ::CheckControlLayer( pLst->GetObj( i ) ) )
            {
                
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
    mbUserCallActive( false ),
    
    
    meEventTypeOfCurrentUserCall( SDRUSERCALL_MOVEONLY )
{
    
    maDrawVirtObjs.clear();

    
    
    if ( !pObj->IsInserted() )
    {
        pToRegisterIn->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
                                InsertObject( pObj, pObj->GetOrdNumDirect() );
    }

    
    
    if ( ::CheckControlLayer( pObj ) )
    {
        
        pObj->SetLayer( pToRegisterIn->getIDocumentDrawModelAccess()->GetInvisibleControlsId() );
    }

    
    pObj->SetUserCall( this );
    maAnchoredDrawObj.SetDrawObj( *pObj );

    
    
    SwXShape::AddExistingShapeToFmt( *pObj );
}

SwDrawContact::~SwDrawContact()
{
    SetInDTOR();

    DisconnectFromLayout();

    
    RemoveMasterFromDrawPage();

    
    RemoveAllVirtObjs();

    if ( !mbMasterObjCleared )
    {
        SdrObject* pObject = const_cast< SdrObject* >( maAnchoredDrawObj.GetDrawObj() );
        SdrObject::Free( pObject );
    }
}

void SwDrawContact::GetTextObjectsFromFmt( std::list<SdrTextObj*>& rTextObjects, SwDoc* pDoc )
{
    for( sal_Int32 n=0; n<(sal_Int32)pDoc->GetSpzFrmFmts()->size(); n++ )
    {
        SwFrmFmt* pFly = (*pDoc->GetSpzFrmFmts())[n];
        if( pFly->IsA( TYPE(SwDrawFrmFmt) ) )
        {
            SwDrawContact* pContact = SwIterator<SwDrawContact,SwFrmFmt>::FirstElement(*pFly);
            if( pContact )
            {
                SdrObject* pSdrO = pContact->GetMaster();
                if ( pSdrO )
                {
                    if ( pSdrO->IsA( TYPE(SdrObjGroup) ) )
                    {
                        SdrObjListIter aListIter( *pSdrO, IM_DEEPNOGROUPS );
                        
                        while( aListIter.IsMore() )
                        {
                            SdrObject* pSdrOElement = aListIter.Next();
                            if( pSdrOElement && pSdrOElement->IsA( TYPE(SdrTextObj) ) &&
                                static_cast<SdrTextObj*>( pSdrOElement)->HasText() )
                            {
                                rTextObjects.push_back((SdrTextObj*) pSdrOElement);
                            }
                        }
                    }
                    else if( pSdrO->IsA( TYPE(SdrTextObj) ) &&
                            static_cast<SdrTextObj*>( pSdrO )->HasText() )
                    {
                        rTextObjects.push_back((SdrTextObj*) pSdrO);
                    }
                }
            }
        }
    }
}


const SwAnchoredObject* SwDrawContact::GetAnchoredObj( const SdrObject* _pSdrObj ) const
{
    
    if ( !_pSdrObj )
    {
        _pSdrObj = GetMaster();
    }

    OSL_ENSURE( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( _pSdrObj->ISA(SwDrawVirtObj) ||
            ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) ),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == const_cast<SwDrawContact*>(this) ||
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
    
    if ( !_pSdrObj )
    {
        _pSdrObj = GetMaster();
    }

    OSL_ENSURE( _pSdrObj,
            "<SwDrawContact::GetAnchoredObj(..)> - no object provided" );
    OSL_ENSURE( _pSdrObj->ISA(SwDrawVirtObj) ||
            ( !_pSdrObj->ISA(SdrVirtObj) && !_pSdrObj->ISA(SwDrawVirtObj) ),
            "<SwDrawContact::GetAnchoredObj(..)> - wrong object type object provided" );
    OSL_ENSURE( GetUserCall( _pSdrObj ) == this || _pSdrObj == GetMaster(),
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

/**
 * @note Overloading <SwContact::SetMaster(..)> in order to assert, if the
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
        OSL_FAIL( "<SwDrawContact::GetAnchorFrm(..)> - unknown drawing object." );
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
        OSL_ENSURE( _pDrawObj->ISA(SwDrawVirtObj),
                "<SwDrawContact::GetAnchorFrm(..)> - unknown drawing object." );
        pAnchorFrm = static_cast<SwDrawVirtObj*>(_pDrawObj)->AnchorFrm();
    }

    return pAnchorFrm;
}


SwDrawVirtObj* SwDrawContact::CreateVirtObj()
{
    
    SdrObject* pOrgMasterSdrObj = GetMaster();

    
    SwDrawVirtObj* pNewDrawVirtObj = new SwDrawVirtObj ( *(pOrgMasterSdrObj), *(this) );

    
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
        _pVirtObj = 0;
    }
}

/** add a 'virtual' drawing object to drawing page.
 *
 * Use an already created one, which isn't used, or create a new one.
 */
SwDrawVirtObj* SwDrawContact::AddVirtObj()
{
    SwDrawVirtObj* pAddedDrawVirtObj = 0L;

    
    std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
            std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
                          UsedOrUnusedVirtObjPred( false ) );

    if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
    {
        
        pAddedDrawVirtObj = (*aFoundVirtObjIter);
    }
    else
    {
        
        pAddedDrawVirtObj = CreateVirtObj();
    }
    pAddedDrawVirtObj->AddToDrawingPage();

    return pAddedDrawVirtObj;
}


void SwDrawContact::RemoveAllVirtObjs()
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjsIter = maDrawVirtObjs.begin();
          aDrawVirtObjsIter != maDrawVirtObjs.end();
          ++aDrawVirtObjsIter )
    {
        
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


SdrObject* SwDrawContact::GetDrawObjectByAnchorFrm( const SwFrm& _rAnchorFrm )
{
    SdrObject* pRetDrawObj = 0L;

    
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

void SwDrawContact::NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect )
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        if ( pDrawVirtObj->GetAnchorFrm() )
        {
            
            SwPageFrm* pPage = pDrawVirtObj->AnchoredObj()->FindPageFrmOfAnchor();
            if( pOldBoundRect && pPage )
            {
                SwRect aOldRect( *pOldBoundRect );
                aOldRect.Pos() += pDrawVirtObj->GetOffset();
                if( aOldRect.HasArea() )
                    ::Notify_Background( pDrawVirtObj, pPage,
                                         aOldRect, PREP_FLY_LEAVE,sal_True);
            }
            
            SwRect aRect( pDrawVirtObj->GetAnchoredObj()->GetObjRectWithSpaces() );
            if (aRect.HasArea() && pPage)
            {
                SwPageFrm* pPg = (SwPageFrm*)::FindPage( aRect, pPage );
                if ( pPg )
                    ::Notify_Background( pDrawVirtObj, pPg, aRect,
                                         PREP_FLY_ARRIVE, sal_True );
            }
            ::ClrContourCache( pDrawVirtObj );
        }
    }
}


static void lcl_NotifyBackgroundOfObj( SwDrawContact& _rDrawContact,
                                const SdrObject& _rObj,
                                const Rectangle* _pOldObjRect )
{
    
    SwAnchoredObject* pAnchoredObj =
        const_cast<SwAnchoredObject*>(_rDrawContact.GetAnchoredObj( &_rObj ));
    if ( pAnchoredObj && pAnchoredObj->GetAnchorFrm() )
    {
        
        SwPageFrm* pPageFrm = pAnchoredObj->FindPageFrmOfAnchor();
        if( _pOldObjRect && pPageFrm )
        {
            SwRect aOldRect( *_pOldObjRect );
            if( aOldRect.HasArea() )
            {
                
                SwPageFrm* pOldPageFrm = (SwPageFrm*)::FindPage( aOldRect, pPageFrm );
                ::Notify_Background( &_rObj, pOldPageFrm, aOldRect,
                                     PREP_FLY_LEAVE, sal_True);
            }
        }
        
        SwRect aNewRect( pAnchoredObj->GetObjRectWithSpaces() );
        if( aNewRect.HasArea() && pPageFrm )
        {
            pPageFrm = (SwPageFrm*)::FindPage( aNewRect, pPageFrm );
            ::Notify_Background( &_rObj, pPageFrm, aNewRect,
                                 PREP_FLY_ARRIVE, sal_True );
        }
        ClrContourCache( &_rObj );
    }
}

void SwDrawContact::Changed( const SdrObject& rObj,
                             SdrUserCallType eType,
                             const Rectangle& rOldBoundRect )
{
    
    
    SwDoc* pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetCurrentViewShell() &&
         pDoc->GetCurrentViewShell()->IsInConstructor() )
    {
        return;
    }

    
    
    
    if ( pDoc->IsInDtor() && eType != SDRUSERCALL_DELETE )
    {
        return;
    }

    
    SwViewShell *pSh = 0, *pOrg;
    SwRootFrm *pTmpRoot = pDoc->GetCurrentLayout();
    if ( pTmpRoot && pTmpRoot->IsCallbackActionEnabled() )
    {
        pDoc->GetEditShell( &pOrg );
        pSh = pOrg;
        if ( pSh )
            do
            {   if ( pSh->Imp()->IsAction() || pSh->Imp()->IsIdleAction() )
                    pSh = 0;
                else
                    pSh = (SwViewShell*)pSh->GetNext();

            } while ( pSh && pSh != pOrg );

        if ( pSh )
            pTmpRoot->StartAllAction();
    }
    SdrObjUserCall::Changed( rObj, eType, rOldBoundRect );
    _Changed( rObj, eType, &rOldBoundRect );    

    if ( pSh )
        pTmpRoot->EndAllAction();
}



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
            mpDrawContact = 0;
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
                    OSL_FAIL( "<SwDrawContact::_Changed(..)> - unknown nested <UserCall> event. This is serious, please inform OD." );
                }
            }
        }
};

//

//
void SwDrawContact::_Changed( const SdrObject& rObj,
                              SdrUserCallType eType,
                              const Rectangle* pOldBoundRect )
{
    
    NestedUserCallHdl aNestedUserCallHdl( this, eType );
    if ( aNestedUserCallHdl.IsNestedUserCall() )
    {
        aNestedUserCallHdl.AssertNestedUserCall();
        return;
    }
    
    
    
    
    
    const bool bAnchoredAsChar = ObjAnchoredAsChar();
    const bool bNotify = !(GetFmt()->GetDoc()->IsInDtor()) &&
                         ( SURROUND_THROUGHT != GetFmt()->GetSurround().GetSurround() ) &&
                         !bAnchoredAsChar;
    switch( eType )
    {
        case SDRUSERCALL_DELETE:
            {
                if ( bNotify )
                {
                    lcl_NotifyBackgroundOfObj( *this, rObj, pOldBoundRect );
                    
                    
                    NotifyBackgrdOfAllVirtObjs( pOldBoundRect );
                }
                DisconnectFromLayout( false );
                SetMaster( NULL );
                delete this;
                
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
            
            
            if(dynamic_cast< SdrObjGroup* >(maAnchoredDrawObj.DrawObj()))
            {
                if(::CheckControlLayer(maAnchoredDrawObj.DrawObj()))
                {
                    const IDocumentDrawModelAccess* pIDDMA = static_cast<SwFrmFmt*>(GetRegisteredInNonConst())->getIDocumentDrawModelAccess();
                    const SdrLayerID aCurrentLayer(maAnchoredDrawObj.DrawObj()->GetLayer());
                    const SdrLayerID aControlLayerID(pIDDMA->GetControlsId());
                    const SdrLayerID aInvisibleControlLayerID(pIDDMA->GetInvisibleControlsId());

                    if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
                    {
                        if ( aCurrentLayer == pIDDMA->GetInvisibleHellId() ||
                             aCurrentLayer == pIDDMA->GetInvisibleHeavenId() )
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
            
        }
        case SDRUSERCALL_MOVEONLY:
        case SDRUSERCALL_RESIZE:
        case SDRUSERCALL_CHILD_MOVEONLY :
        case SDRUSERCALL_CHILD_RESIZE :
        case SDRUSERCALL_CHILD_CHGATTR :
        case SDRUSERCALL_CHILD_DELETE :
        case SDRUSERCALL_CHILD_COPY :
        {
            
            
            const SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<const SwAnchoredDrawObject*>( GetAnchoredObj( &rObj ) );

            /* protect against NULL pointer dereferencing */
            if(!pAnchoredDrawObj)
            {
                break;
            }

            
            
            
            
            if ( !pAnchoredDrawObj->IsPositioningInProgress() &&
                 !pAnchoredDrawObj->NotYetPositioned() )
            {
                
                
                const Rectangle& aOldObjRect = pAnchoredDrawObj->GetLastObjRect()
                                               ? *(pAnchoredDrawObj->GetLastObjRect())
                                               : *(pOldBoundRect);
                
                
                pAnchoredDrawObj->InvalidateObjRectWithSpaces();
                
                
                if ( bNotify )
                {
                    
                    
                    lcl_NotifyBackgroundOfObj( *this, rObj, &aOldObjRect );
                }
                
                
                SwFrmFmt::tLayoutDir eLayoutDir =
                                pAnchoredDrawObj->GetFrmFmt().GetLayoutDir();
                
                SwRect aObjRect( rObj.GetSnapRect() );
                
                
                
                if ( rObj.GetUpGroup() )
                {
                    const SdrObject* pGroupObj = rObj.GetUpGroup();
                    while ( pGroupObj->GetUpGroup() )
                    {
                        pGroupObj = pGroupObj->GetUpGroup();
                    }
                    
                    aObjRect = pGroupObj->GetSnapRect();
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
                        OSL_FAIL( "<SwDrawContact::_Changed(..)> - unsupported layout direction" );
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
                    
                    
                    
                    
                    
                    const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)
                                    ->SetLastObjRect( aObjRect.SVRect() );
                }
                else if ( aObjRect.SSize() != aOldObjRect.GetSize() )
                {
                    _InvalidateObjs();
                    
                    
                    if ( bAnchoredAsChar )
                    {
                        SwFrm* pAnchorFrm = const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)->AnchorFrm();
                        if(pAnchorFrm)
                        {
                            pAnchorFrm->Prepare( PREP_FLY_ATTR_CHG, GetFmt() );
                        }
                    }
                }
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
    static const SwFmtAnchor* lcl_getAnchorFmt( const SfxPoolItem& _rItem )
    {
        sal_uInt16 nWhich = _rItem.Which();
        const SwFmtAnchor* pAnchorFmt = NULL;
        if ( RES_ATTRSET_CHG == nWhich )
        {
            static_cast<const SwAttrSetChg&>(_rItem).GetChgSet()->
                GetItemState( RES_ANCHOR, false, (const SfxPoolItem**)&pAnchorFmt );
        }
        else if ( RES_ANCHOR == nWhich )
        {
            pAnchorFmt = &static_cast<const SwFmtAnchor&>(_rItem);
        }
        return pAnchorFmt;
    }
}

void SwDrawContact::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    OSL_ENSURE( !mbDisconnectInProgress,
            "<SwDrawContact::Modify(..)> called during disconnection.");

    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor* pNewAnchorFmt = pNew ? lcl_getAnchorFmt( *pNew ) : NULL;

    if ( pNewAnchorFmt )
    {
        
        if ( SFX_ITEM_SET ==
                GetFmt()->GetAttrSet().GetItemState( RES_ANCHOR, false ) )
        {
            
            if ( !mbDisconnectInProgress )
            {
                
                
                const Rectangle* pOldRect = 0L;
                Rectangle aOldRect;
                if ( GetAnchorFrm() )
                {
                    
                    
                    aOldRect = maAnchoredDrawObj.GetObjRectWithSpaces().SVRect();
                    pOldRect = &aOldRect;
                }
                
                ConnectToLayout( pNewAnchorFmt );
                
                lcl_NotifyBackgroundOfObj( *this, *GetMaster(), pOldRect );
                NotifyBackgrdOfAllVirtObjs( pOldRect );

                const SwFmtAnchor* pOldAnchorFmt = pOld ? lcl_getAnchorFmt( *pOld ) : NULL;
                if ( !pOldAnchorFmt || ( pOldAnchorFmt->GetAnchorId() != pNewAnchorFmt->GetAnchorId() ) )
                {
                    OSL_ENSURE( maAnchoredDrawObj.DrawObj(), "SwDrawContact::Modify: no draw object here?" );
                    if ( maAnchoredDrawObj.DrawObj() )
                    {
                        
                        
                        maAnchoredDrawObj.DrawObj()->notifyShapePropertyChange( ::svx::eTextShapeAnchorType );
                    }
                }
            }
        }
        else
            DisconnectFromLayout();
    }
    
    else if ( maAnchoredDrawObj.GetAnchorFrm() &&
              maAnchoredDrawObj.GetDrawObj()->GetUserCall() )
    {
        
        
        
        
        if ( RES_SURROUND == nWhich ||
             RES_OPAQUE == nWhich ||
             RES_WRAP_INFLUENCE_ON_OBJPOS == nWhich ||
             ( RES_ATTRSET_CHG == nWhich &&
               ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_SURROUND, false ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_OPAQUE, false ) ||
                 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                           RES_WRAP_INFLUENCE_ON_OBJPOS, false ) ) ) )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
            NotifyBackgrdOfAllVirtObjs( 0L );
            _InvalidateObjs( true );
        }
        else if ( RES_UL_SPACE == nWhich || RES_LR_SPACE == nWhich ||
                  RES_HORI_ORIENT == nWhich || RES_VERT_ORIENT == nWhich ||
                  
                  RES_FOLLOW_TEXT_FLOW == nWhich ||
                  ( RES_ATTRSET_CHG == nWhich &&
                    ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_LR_SPACE, false ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_UL_SPACE, false ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_HORI_ORIENT, false ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_VERT_ORIENT, false ) ||
                      SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
                                RES_FOLLOW_TEXT_FLOW, false ) ) ) )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
            NotifyBackgrdOfAllVirtObjs( 0L );
            _InvalidateObjs();
        }
        
        else if ( RES_ATTRSET_CHG == nWhich )
        {
            lcl_NotifyBackgroundOfObj( *this, *GetMaster(), 0L );
            NotifyBackgrdOfAllVirtObjs( 0L );
            _InvalidateObjs();
        }
        else if ( RES_REMOVE_UNO_OBJECT == nWhich )
        {
            
        }
#if OSL_DEBUG_LEVEL > 0
        else
        {
            OSL_FAIL( "<SwDrawContact::Modify(..)> - unhandled attribute? - please inform od@openoffice.org" );
        }
#endif
    }

    
    GetAnchoredObj( 0L )->ResetLayoutProcessBools();
}



void SwDrawContact::_InvalidateObjs( const bool _bUpdateSortedObjsList )
{
    
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        
        
        if ( pDrawVirtObj->IsConnected() )
        {
            pDrawVirtObj->AnchoredObj()->InvalidateObjPos();
            
            if ( _bUpdateSortedObjsList )
            {
                pDrawVirtObj->AnchoredObj()->UpdateObjInSortedList();
            }
        }
    }

    
    SwAnchoredObject* pAnchoredObj = GetAnchoredObj( 0L );
    pAnchoredObj->InvalidateObjPos();
    
    if ( _bUpdateSortedObjsList )
    {
        pAnchoredObj->UpdateObjInSortedList();
    }
}

void SwDrawContact::DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer )
{
    mbDisconnectInProgress = true;

    
    if ( _bMoveMasterToInvisibleLayer &&
         !(GetFmt()->GetDoc()->IsInDtor()) &&
         GetAnchorFrm() )
    {
        const Rectangle aOldRect( maAnchoredDrawObj.GetObjRectWithSpaces().SVRect() );
        lcl_NotifyBackgroundOfObj( *this, *GetMaster(), &aOldRect );
        NotifyBackgrdOfAllVirtObjs( &aOldRect );
    }

    
    
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
            pView->MarkObj( GetMaster(), pView->GetSdrPageView(), sal_True );
        }

        
        
        {
            
            
            
            
            MoveObjToInvisibleLayer( GetMaster() );
        }
    }

    mbDisconnectInProgress = false;
}


void SwDrawContact::RemoveMasterFromDrawPage()
{
    if ( GetMaster() )
    {
        GetMaster()->SetUserCall( 0 );
        if ( GetMaster()->IsInserted() )
        {
            ((SwFrmFmt*)GetRegisteredIn())->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)->
                                        RemoveObject( GetMaster()->GetOrdNum() );
        }
    }
}







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
            
            
            SwDrawVirtObj* pDrawVirtObj = (*aFoundVirtObjIter);
            SwFrm* pNewAnchorFrmOfMaster = pDrawVirtObj->AnchorFrm();
            
            pDrawVirtObj->RemoveFromWriterLayout();
            pDrawVirtObj->RemoveFromDrawingPage();
            
            GetAnchorFrm()->RemoveDrawObj( maAnchoredDrawObj );
            
            
            pNewAnchorFrmOfMaster->AppendDrawObj( maAnchoredDrawObj );
        }
        else
        {
            
            
            DisconnectFromLayout();
        }
    }
}

static SwTxtFrm* lcl_GetFlyInCntntAnchor( SwTxtFrm* _pProposedAnchorFrm,
                                   const sal_Int32 _nTxtOfs )
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
    
    if ( mbDisconnectInProgress )
    {
        OSL_FAIL( "<SwDrawContact::ConnectToLayout(..)> called during disconnection.");
        return;
    }

    
    
    if ( !GetMaster()->IsInserted() )
    {
        OSL_FAIL( "<SwDrawContact::ConnectToLayout(..)> - master drawing object not inserted -> no connect to layout. Please inform od@openoffice.org" );
        return;
    }

    SwFrmFmt* pDrawFrmFmt = (SwFrmFmt*)GetRegisteredIn();

    if( !pDrawFrmFmt->getIDocumentLayoutAccess()->GetCurrentViewShell() )
        return;

    
    
    
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
                SwViewShell *pShell = pDrawFrmFmt->getIDocumentLayoutAccess()->GetCurrentViewShell();
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
                            SwFrmFmts& rFmts = *(pDrawFrmFmt->GetDoc()->GetSpzFrmFmts());
                            for( sal_uInt16 i = 0; i < rFmts.size(); ++i )
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
                        
                        
                        
                        
                        if ( !pModify )
                        {
                            
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
                    
                    
                    
                    
                    const bool bAdd = ( !pFrm->IsCntntFrm() ||
                                        !((SwCntntFrm*)pFrm)->IsFollow() ) &&
                                      ( !::CheckControlLayer( GetMaster() ) ||
                                        !pFrm->FindFooterOrHeader() );

                    if( bAdd )
                    {
                        if ( FLY_AT_FLY == pAnch->GetAnchorId() && !pFrm->IsFlyFrm() )
                        {
                            pFrm = pFrm->FindFlyFrm();
                            OSL_ENSURE( pFrm,
                                    "<SwDrawContact::ConnectToLayout(..)> - missing fly frame -> crash." );
                        }

                        
                        if ((pAnch->GetAnchorId() == FLY_AS_CHAR) &&
                             pFrm->IsTxtFrm() )
                        {
                            pFrm = lcl_GetFlyInCntntAnchor(
                                        static_cast<SwTxtFrm*>(pFrm),
                                        pAnch->GetCntntAnchor()->nContent.GetIndex() );
                        }

                        if ( !pAnchorFrmOfMaster )
                        {
                            
                            pAnchorFrmOfMaster = pFrm;
                            pFrm->AppendDrawObj( maAnchoredDrawObj );
                        }
                        else
                        {
                            
                            SwDrawVirtObj* pDrawVirtObj = AddVirtObj();
                            if ( pAnch->GetAnchorId() == FLY_AS_CHAR )
                            {
                                ClrContourCache( pDrawVirtObj );
                            }
                            pFrm->AppendDrawObj( *(pDrawVirtObj->AnchoredObj()) );

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
            OSL_FAIL( "Unknown Anchor." );
            break;
    }
    if ( GetAnchorFrm() )
    {
        ::setContextWritingMode( maAnchoredDrawObj.DrawObj(), GetAnchorFrm() );
        
        _InvalidateObjs();
    }
}


void SwDrawContact::InsertMasterIntoDrawPage()
{
    if ( !GetMaster()->IsInserted() )
    {
        GetFmt()->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage(0)
                ->InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
    }
    GetMaster()->SetUserCall( this );
}

SwPageFrm* SwDrawContact::FindPage( const SwRect &rRect )
{
    
    SwPageFrm* pPg = GetPageFrm();
    if ( !pPg && GetAnchorFrm() )
        pPg = GetAnchorFrm()->FindPageFrm();
    if ( pPg )
        pPg = (SwPageFrm*)::FindPage( rRect, pPg );
    return pPg;
}

void SwDrawContact::ChkPage()
{
    if ( mbDisconnectInProgress )
    {
        OSL_FAIL( "<SwDrawContact::ChkPage()> called during disconnection." );
        return;
    }

    
    SwPageFrm* pPg = ( maAnchoredDrawObj.GetAnchorFrm() &&
                       maAnchoredDrawObj.GetAnchorFrm()->IsPageFrm() )
                     ? GetPageFrm()
                     : FindPage( GetMaster()->GetCurrentBoundRect() );
    if ( GetPageFrm() != pPg )
    {
        
        
        if ( maAnchoredDrawObj.GetAnchorFrm() &&
             maAnchoredDrawObj.GetAnchorFrm()->FindFooterOrHeader() )
        {
            ConnectToLayout();
        }
        else
        {
            
            if ( GetPageFrm() )
                GetPageFrm()->RemoveDrawObjFromPage( maAnchoredDrawObj );
            pPg->AppendDrawObjToPage( maAnchoredDrawObj );
            SetPageFrm( pPg );
        }
    }
}






void SwDrawContact::ChangeMasterObject( SdrObject *pNewMaster )
{
    DisconnectFromLayout( false );
    
    RemoveAllVirtObjs();

    GetMaster()->SetUserCall( 0 );
    SetMaster( pNewMaster );
    GetMaster()->SetUserCall( this );

    _InvalidateObjs();
}


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
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

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
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            
            VCOfDrawVirtObj(SwDrawVirtObj& rObj)
            :   ViewContactOfVirtObj(rObj)
            {
            }
            virtual ~VCOfDrawVirtObj();

            
            SwDrawVirtObj& GetSwDrawVirtObj() const
            {
                return (SwDrawVirtObj&)mrObject;
            }
        };
    } 
} 

namespace sdr
{
    namespace contact
    {
        
        void impAddPrimitivesFromGroup(const ViewObjectContact& rVOC, const basegfx::B2DHomMatrix& rOffsetMatrix, const DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DSequence& rxTarget)
        {
            const sal_uInt32 nSubHierarchyCount(rVOC.GetViewContact().GetObjectCount());

            for(sal_uInt32 a(0L); a < nSubHierarchyCount; a++)
            {
                const ViewObjectContact& rCandidate(rVOC.GetViewContact().GetViewContact(a).GetViewObjectContact(rVOC.GetObjectContact()));

                if(rCandidate.GetViewContact().GetObjectCount())
                {
                    
                    impAddPrimitivesFromGroup(rCandidate, rOffsetMatrix, rDisplayInfo, rxTarget);
                }
                else
                {
                    
                    if(rCandidate.isPrimitiveVisible(rDisplayInfo))
                    {
                        drawinglayer::primitive2d::Primitive2DSequence aNewSequence(rCandidate.getPrimitive2DSequence(rDisplayInfo));

                        if(aNewSequence.hasElements())
                        {
                            
                            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(rCandidate.GetObjectContact().getViewInformation2D());
                            const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());
                            basegfx::B2DRange aObjectRange(rCandidate.getObjectRange());

                            
                            aObjectRange.transform(rOffsetMatrix);

                            
                            if(!aViewRange.overlaps(aObjectRange))
                            {
                                
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

            
            basegfx::B2DHomMatrix aOffsetMatrix;
            const Point aLocalOffset(rVC.GetSwDrawVirtObj().GetOffset());

            if(aLocalOffset.X() || aLocalOffset.Y())
            {
                aOffsetMatrix.set(0, 2, aLocalOffset.X());
                aOffsetMatrix.set(1, 2, aLocalOffset.Y());
            }

            if(rReferencedObject.ISA(SdrObjGroup))
            {
                
                
                
                
                

                
                const ViewObjectContact& rVOCOfRefObj = rReferencedObject.GetViewContact().GetViewObjectContact(GetObjectContact());
                impAddPrimitivesFromGroup(rVOCOfRefObj, aOffsetMatrix, rDisplayInfo, xRetval);
            }
            else
            {
                
                xRetval = rReferencedObject.GetViewContact().getViewIndependentPrimitive2DSequence();
            }

            if(xRetval.hasElements())
            {
                
                const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::TransformPrimitive2D(aOffsetMatrix, xRetval));
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
    } 
} 


TYPEINIT1(SwDrawVirtObj,SdrVirtObj);

sdr::contact::ViewContact* SwDrawVirtObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::VCOfDrawVirtObj(*this);
}



SwDrawVirtObj::SwDrawVirtObj( SdrObject&        _rNewObj,
                              SwDrawContact&    _rDrawContact )
    : SdrVirtObj( _rNewObj ),
      
      maAnchoredDrawObj(),
      mrDrawContact( _rDrawContact )
{
    
    maAnchoredDrawObj.SetDrawObj( *this );
    
    NbcMove( Size( -RECT_EMPTY, -RECT_EMPTY ) );
}

SwDrawVirtObj::~SwDrawVirtObj()
{}

SwDrawVirtObj& SwDrawVirtObj::operator=( const SwDrawVirtObj& rObj )
{
    SdrVirtObj::operator=(rObj);
    
    
    return *this;
}

SwDrawVirtObj* SwDrawVirtObj::Clone() const
{
    SwDrawVirtObj* pObj = new SwDrawVirtObj( rRefObj, mrDrawContact );

    if ( pObj )
    {
        pObj->operator=( *this );
        
    }

    return pObj;
}



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
    
    return maAnchoredDrawObj.GetAnchorFrm();
}

SwFrm* SwDrawVirtObj::AnchorFrm()
{
    
    return maAnchoredDrawObj.AnchorFrm();
}

void SwDrawVirtObj::RemoveFromWriterLayout()
{
    
    
    if ( maAnchoredDrawObj.GetAnchorFrm() )
    {
        maAnchoredDrawObj.AnchorFrm()->RemoveDrawObj( maAnchoredDrawObj );
    }
}

void SwDrawVirtObj::AddToDrawingPage()
{
    
    SdrObject* pOrgMasterSdrObj = mrDrawContact.GetMaster();

    
    SdrPage* pDrawPg;
    
    if ( 0 != ( pDrawPg = pOrgMasterSdrObj->GetPage() ) )
    {
        
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
    SetUserCall( 0 );
    if ( GetPage() )
    {
        GetPage()->RemoveObject( GetOrdNum() );
    }
}


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
    
}

void SwDrawVirtObj::RecalcBoundRect()
{
    
    
    

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

SdrHdl* SwDrawVirtObj::GetPlusHdl(const SdrHdl& rHdl, sal_uInt16 nPlNum) const
{
    SdrHdl* pHdl = rRefObj.GetPlusHdl(rHdl, nPlNum);

    if(pHdl)
    {
        pHdl->SetPos(pHdl->GetPos() + GetOffset());
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

void SwDrawVirtObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    rRefObj.NbcShear(rRef - GetOffset(), nWink, tn, bVShear);
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

void SwDrawVirtObj::Shear(const Point& rRef, long nWink, double tn, bool bVShear)
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
    
    return SdrVirtObj::supportsFullDrag();
}

SdrObject* SwDrawVirtObj::getFullDragClone() const
{
    
    return SdrVirtObj::getFullDragClone();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
