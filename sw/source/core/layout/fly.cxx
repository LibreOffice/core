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

#include <svl/itemiter.hxx>
#include <svtools/imap.hxx>
#include <tools/helpers.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtcnct.hxx>
#include <layhelp.hxx>
#include <ndgrf.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <ndole.hxx>
#include <swtable.hxx>
#include <svx/svdpage.hxx>
#include "layouter.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"
#include "frmtool.hxx"
#include "hints.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
#include <vcl/svapp.hxx>
#include "switerator.hxx"

using namespace ::com::sun::star;

TYPEINIT2(SwFlyFrm,SwLayoutFrm,SwAnchoredObject);

SwFlyFrm::SwFlyFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwLayoutFrm( pFmt, pSib ),
    SwAnchoredObject(), 
    pPrevLink( 0 ),
    pNextLink( 0 ),
    bInCnt( sal_False ),
    bAtCnt( sal_False ),
    bLayout( sal_False ),
    bAutoPosition( sal_False ),
    bNoShrink( sal_False ),
    bLockDeleteContent( sal_False )
{
    mnType = FRMC_FLY;

    bInvalid = bNotifyBack = sal_True;
    bLocked  = bMinHeight =
    bHeightClipped = bWidthClipped = bFormatHeightOnly = sal_False;

    
    const SwFmtFrmSize &rFrmSize = pFmt->GetFrmSize();
    sal_uInt16 nDir =
        ((SvxFrameDirectionItem&)pFmt->GetFmtAttr( RES_FRAMEDIR )).GetValue();
    if( FRMDIR_ENVIRONMENT == nDir )
    {
        mbDerivedVert = 1;
        mbDerivedR2L = 1;
    }
    else
    {
        mbInvalidVert = 0;
        mbDerivedVert = 0;
        mbDerivedR2L = 0;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            
            mbVertLR = 0;
            mbVertical = 0;
        }
        else
        {
            const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                
                mbVertLR = 0;
                mbVertical = 0;
            }
            else
            {
                mbVertical = 1;
                
                if ( FRMDIR_VERT_TOP_LEFT == nDir )
                    mbVertLR = 1;
                else
                    mbVertLR = 0;
            }
        }

        mbInvalidR2L = 0;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            mbRightToLeft = 1;
        else
            mbRightToLeft = 0;
    }

    Frm().Width( rFrmSize.GetWidth() );
    Frm().Height( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE ? MINFLY : rFrmSize.GetHeight() );

    
    if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        bMinHeight = sal_True;
    else if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        mbFixSize = sal_True;

    
    InsertColumns();

    
    
    
    InitDrawObj( sal_False );

    Chain( pAnch );

    InsertCnt();

    
    Frm().Pos().setX(FAR_AWAY);
    Frm().Pos().setY(FAR_AWAY);
}

void SwFlyFrm::Chain( SwFrm* _pAnch )
{
    
    
    
    const SwFmtChain& rChain = GetFmt()->GetChain();
    if ( rChain.GetPrev() || rChain.GetNext() )
    {
        if ( rChain.GetNext() )
        {
            SwFlyFrm* pFollow = FindChainNeighbour( *rChain.GetNext(), _pAnch );
            if ( pFollow )
            {
                OSL_ENSURE( !pFollow->GetPrevLink(), "wrong chain detected" );
                if ( !pFollow->GetPrevLink() )
                    SwFlyFrm::ChainFrames( this, pFollow );
            }
        }
        if ( rChain.GetPrev() )
        {
            SwFlyFrm *pMaster = FindChainNeighbour( *rChain.GetPrev(), _pAnch );
            if ( pMaster )
            {
                OSL_ENSURE( !pMaster->GetNextLink(), "wrong chain detected" );
                if ( !pMaster->GetNextLink() )
                    SwFlyFrm::ChainFrames( pMaster, this );
            }
        }
    }
}

void SwFlyFrm::InsertCnt()
{
    if ( !GetPrevLink() )
    {
        const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
        OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( no content prepared." );
        sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
        
        ::_InsertCnt( Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower() : (SwLayoutFrm*)this,
                      GetFmt()->GetDoc(), nIndex );

        
        if ( Lower() && Lower()->IsNoTxtFrm() )
        {
            mbFixSize = sal_True;
            bMinHeight = sal_False;
        }
    }
}

void SwFlyFrm::InsertColumns()
{
    
    
    
    const SwFmtCntnt& rCntnt = GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "<SwFlyFrm::InsertColumns()> - no content prepared." );
    SwNodeIndex aFirstCntnt( *(rCntnt.GetCntntIdx()), 1 );
    if ( aFirstCntnt.GetNode().IsNoTxtNode() )
    {
        return;
    }

    const SwFmtCol &rCol = GetFmt()->GetCol();
    if ( rCol.GetNumCols() > 1 )
    {
        
        
        Prt().Width( Frm().Width() );
        Prt().Height( Frm().Height() );
        const SwFmtCol aOld; 
        ChgColumns( aOld, rCol );
    }
}

SwFlyFrm::~SwFlyFrm()
{
    
    
    
    
    if( IsAccessibleFrm() && GetFmt() && (IsFlyInCntFrm() || !GetAnchorFrm()) )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            SwViewShell *pVSh = pRootFrm->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                
                
                pVSh->Imp()->DisposeAccessibleFrm( this, sal_True );
            }
        }
    }

    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        
        Unchain();

        
        DeleteCnt();

        if ( GetAnchorFrm() )
            AnchorFrm()->RemoveFly( this );
    }

    FinitDrawObj();
}

const IDocumentDrawModelAccess* SwFlyFrm::getIDocumentDrawModelAccess()
{
    return GetFmt()->getIDocumentDrawModelAccess();
}


void SwFlyFrm::Unchain()
{
    if ( GetPrevLink() )
        UnchainFrames( GetPrevLink(), this );
    if ( GetNextLink() )
        UnchainFrames( this, GetNextLink() );
}


void SwFlyFrm::DeleteCnt()
{
    
    if ( IsLockDeleteContent() )
        return;

    SwFrm* pFrm = pLower;
    while ( pFrm )
    {
        while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
        {
            SwAnchoredObject *pAnchoredObj = (*pFrm->GetDrawObjs())[0];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
                delete pAnchoredObj;
            else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
            {
                
                SdrObject* pObj = pAnchoredObj->DrawObj();
                if ( pObj->ISA(SwDrawVirtObj) )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
                    pDrawVirtObj->RemoveFromWriterLayout();
                    pDrawVirtObj->RemoveFromDrawingPage();
                }
                else
                {
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(::GetUserCall( pObj ));
                    if ( pContact )
                    {
                        pContact->DisconnectFromLayout();
                    }
                }
            }
        }

        pFrm->Remove();
        delete pFrm;
        pFrm = pLower;
    }

    InvalidatePage();
}

sal_uInt32 SwFlyFrm::_GetOrdNumForNewRef( const SwFlyDrawContact* pContact )
{
    sal_uInt32 nOrdNum( 0L );

    
    SwIterator<SwFlyFrm,SwFmt> aIter( *pContact->GetFmt() );
    const SwFlyFrm* pFlyFrm( 0L );
    for ( pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
    {
        if ( pFlyFrm != this )
        {
            break;
        }
    }

    if ( pFlyFrm )
    {
        
        nOrdNum = pFlyFrm->GetVirtDrawObj()->GetOrdNum();
    }
    else
    {
        
        
        
        
        nOrdNum = pContact->GetMaster()->GetOrdNumDirect();
    }

    return nOrdNum;
}

SwVirtFlyDrawObj* SwFlyFrm::CreateNewRef( SwFlyDrawContact *pContact )
{
    SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *pContact->GetMaster(), this );
    pDrawObj->SetModel( pContact->GetMaster()->GetModel() );
    pDrawObj->SetUserCall( pContact );

    
    
    
    
    SdrPage* pPg( 0L );
    if ( 0 != ( pPg = pContact->GetMaster()->GetPage() ) )
    {
        const sal_uInt32 nOrdNum = pContact->GetMaster()->GetOrdNum();
        pPg->ReplaceObject( pDrawObj, nOrdNum );
    }
    
    
    else
    {
        pContact->GetFmt()->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 )->
                        InsertObject( pDrawObj, _GetOrdNumForNewRef( pContact ) );
    }
    
    
    pContact->MoveObjToVisibleLayer( pDrawObj );
    return pDrawObj;
}

void SwFlyFrm::InitDrawObj( sal_Bool bNotify )
{
    
    

    IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
    SwFlyDrawContact *pContact = SwIterator<SwFlyDrawContact,SwFmt>::FirstElement( *GetFmt() );
    if ( !pContact )
    {
        
        pContact = new SwFlyDrawContact( (SwFlyFrmFmt*)GetFmt(),
                                          pIDDMA->GetOrCreateDrawModel() );
    }
    OSL_ENSURE( pContact, "InitDrawObj failed" );
    
    SetDrawObj( *(CreateNewRef( pContact )) );

    
    
    SdrLayerID nHeavenId = pIDDMA->GetHeavenId();
    SdrLayerID nHellId = pIDDMA->GetHellId();
    
    GetVirtDrawObj()->SetLayer( GetFmt()->GetOpaque().GetValue()
                                ? nHeavenId
                                : nHellId );
    if ( bNotify )
        NotifyDrawObj();
}

void SwFlyFrm::FinitDrawObj()
{
    if ( !GetVirtDrawObj() )
        return;

    
    if ( !GetFmt()->GetDoc()->IsInDtor() )
    {
        SwViewShell *p1St = getRootFrm()->GetCurrShell();
        if ( p1St )
        {
            SwViewShell *pSh = p1St;
            do
            {   
                
                if( pSh->HasDrawView() )
                    pSh->Imp()->GetDrawView()->UnmarkAll();
                pSh = (SwViewShell*)pSh->GetNext();

            } while ( pSh != p1St );
        }
    }

    
    
    
    SwFlyDrawContact *pMyContact = 0;
    if ( GetFmt() )
    {
        bool bContinue = true;
        SwIterator<SwFrm,SwFmt> aFrmIter( *GetFmt() );
        for ( SwFrm* pFrm = aFrmIter.First(); pFrm; pFrm = aFrmIter.Next() )
            if ( pFrm != this )
            {
                
                bContinue = false;
                break;
            }

        if ( bContinue )
            
            pMyContact = SwIterator<SwFlyDrawContact,SwFmt>::FirstElement( *GetFmt() );
    }

    
    
    
    if ( pMyContact )
    {
        pMyContact->GetMaster()->SetUserCall( 0 );
    }
    GetVirtDrawObj()->SetUserCall( 0 ); 
    delete GetVirtDrawObj();            
    delete pMyContact;                  
}

void SwFlyFrm::ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    OSL_ENSURE( pMaster && pFollow, "uncomplete chain" );
    OSL_ENSURE( !pMaster->GetNextLink(), "link can not be changed" );
    OSL_ENSURE( !pFollow->GetPrevLink(), "link can not be changed" );

    pMaster->pNextLink = pFollow;
    pFollow->pPrevLink = pMaster;

    if ( pMaster->ContainsCntnt() )
    {
        
        SwFrm *pInva = pMaster->FindLastLower();
        SWRECTFN( pMaster )
        const long nBottom = (pMaster->*fnRect->fnGetPrtBottom)();
        while ( pInva )
        {
            if( (pInva->Frm().*fnRect->fnBottomDist)( nBottom ) <= 0 )
            {
                pInva->InvalidateSize();
                pInva->Prepare( PREP_CLEAR );
                pInva = pInva->FindPrev();
            }
            else
                pInva = 0;
        }
    }

    if ( pFollow->ContainsCntnt() )
    {
        
        
        SwFrm *pFrm = pFollow->ContainsCntnt();
        OSL_ENSURE( !pFrm->IsTabFrm() && !pFrm->FindNext(), "follow in chain contains content" );
        pFrm->Cut();
        delete pFrm;
    }

    
    SwViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

void SwFlyFrm::UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow )
{
    pMaster->pNextLink = 0;
    pFollow->pPrevLink = 0;

    if ( pFollow->ContainsCntnt() )
    {
        
        SwLayoutFrm *pUpper = pMaster;
        if ( pUpper->Lower()->IsColumnFrm() )
        {
            pUpper = static_cast<SwLayoutFrm*>(pUpper->GetLastLower());
            pUpper = static_cast<SwLayoutFrm*>(pUpper->Lower()); 
            OSL_ENSURE( pUpper && pUpper->IsColBodyFrm(), "Missing ColumnBody" );
        }
        SwFlyFrm *pFoll = pFollow;
        while ( pFoll )
        {
            SwFrm *pTmp = ::SaveCntnt( pFoll );
            if ( pTmp )
                ::RestoreCntnt( pTmp, pUpper, pMaster->FindLastLower(), true );
            pFoll->SetCompletePaint();
            pFoll->InvalidateSize();
            pFoll = pFoll->GetNextLink();
        }
    }

    
    const SwFmtCntnt &rCntnt = pFollow->GetFmt()->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), ":-( No content prepared." );
    sal_uLong nIndex = rCntnt.GetCntntIdx()->GetIndex();
    
    ::_InsertCnt( pFollow->Lower() ? (SwLayoutFrm*)((SwLayoutFrm*)pFollow->Lower())->Lower()
                                   : (SwLayoutFrm*)pFollow,
                  pFollow->GetFmt()->GetDoc(), ++nIndex );

    
    SwViewShell* pSh = pMaster->getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = pMaster->getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->InvalidateAccessibleRelationSet( pMaster, pFollow );
    }
}

SwFlyFrm *SwFlyFrm::FindChainNeighbour( SwFrmFmt &rChain, SwFrm *pAnch )
{
    
    

    if ( !pAnch ) 
        pAnch = AnchorFrm();

    SwLayoutFrm *pLay;
    if ( pAnch->IsInFly() )
        pLay = pAnch->FindFlyFrm();
    else
    {
        
        
        pLay = pAnch->GetUpper();
        while ( pLay && !(pLay->GetType() & (FRM_HEADER|FRM_FOOTER)) )
            pLay = pLay->GetUpper();
    }

    SwIterator<SwFlyFrm,SwFmt> aIter( rChain );
    SwFlyFrm *pFly = aIter.First();
    if ( pLay )
    {
        while ( pFly )
        {
            if ( pFly->GetAnchorFrm() )
            {
                if ( pFly->GetAnchorFrm()->IsInFly() )
                {
                    if ( pFly->AnchorFrm()->FindFlyFrm() == pLay )
                        break;
                }
                else if ( pLay == pFly->FindFooterOrHeader() )
                    break;
            }
            pFly = aIter.Next();
        }
    }
    else if ( pFly )
    {
        OSL_ENSURE( !aIter.Next(), "chain with more than one instance" );
    }
    return pFly;
}

SwFrm *SwFlyFrm::FindLastLower()
{
    SwFrm *pRet = ContainsAny();
    if ( pRet && pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    SwFrm *pNxt = pRet;
    while ( pNxt && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNext();
    }
    return pRet;
}

sal_Bool SwFlyFrm::FrmSizeChg( const SwFmtFrmSize &rFrmSize )
{
    sal_Bool bRet = sal_False;
    SwTwips nDiffHeight = Frm().Height();
    if ( rFrmSize.GetHeightSizeType() == ATT_VAR_SIZE )
        mbFixSize = bMinHeight = sal_False;
    else
    {
        if ( rFrmSize.GetHeightSizeType() == ATT_FIX_SIZE )
        {
            mbFixSize = sal_True;
            bMinHeight = sal_False;
        }
        else if ( rFrmSize.GetHeightSizeType() == ATT_MIN_SIZE )
        {
            mbFixSize = sal_False;
            bMinHeight = sal_True;
        }
        nDiffHeight -= rFrmSize.GetHeight();
    }
    
    
    if ( Lower() )
    {
        if ( Lower()->IsColumnFrm() )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            const Size   aOldSz( Prt().SSize() );
            const SwTwips nDiffWidth = Frm().Width() - rFrmSize.GetWidth();
            maFrm.Height( maFrm.Height() - nDiffHeight );
            maFrm.Width ( maFrm.Width()  - nDiffWidth  );
            
            InvalidateObjRectWithSpaces();
            maPrt.Height( maPrt.Height() - nDiffHeight );
            maPrt.Width ( maPrt.Width()  - nDiffWidth  );
            ChgLowersProp( aOldSz );
            ::Notify( this, FindPageFrm(), aOld );
            mbValidPos = sal_False;
            bRet = sal_True;
        }
        else if ( Lower()->IsNoTxtFrm() )
        {
            mbFixSize = sal_True;
            bMinHeight = sal_False;
        }
    }
    return bRet;
}

void SwFlyFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( true )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        _Invalidate();
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePos();
            
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x02 )
        {
            _InvalidateSize();
            
            InvalidateObjRectWithSpaces();
        }
        if ( nInvFlags & 0x04 )
            _InvalidatePrt();
        if ( nInvFlags & 0x08 )
            SetNotifyBack();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
        if ( ( nInvFlags & 0x40 ) && Lower() && Lower()->IsNoTxtFrm() )
            ClrContourCache( GetVirtDrawObj() );
        SwRootFrm *pRoot;
        if ( nInvFlags & 0x20 && 0 != (pRoot = getRootFrm()) )
            pRoot->InvalidateBrowseWidth();
        
        if ( nInvFlags & 0x80 )
        {
            
            UpdateObjInSortedList();
        }

        
        ResetLayoutProcessBools();
    }
}

void SwFlyFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    switch( nWhich )
    {
        case RES_VERT_ORIENT:
        case RES_HORI_ORIENT:
        
        case RES_FOLLOW_TEXT_FLOW:
        {
            
            rInvFlags |= 0x09;
        }
        break;
        
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
        {
            rInvFlags |= 0x89;
        }
        break;
        case RES_SURROUND:
        {
            
            
            
            rInvFlags |= 0x41;
            
            const SwRect aTmp( GetObjRectWithSpaces() );
            NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_ATTR_CHG );

            
            
            if( FLY_AT_FLY == GetFmt()->GetAnchor().GetAnchorId() )
                rInvFlags |= 0x09;

            
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 !GetFmt()->GetSurround().IsContour() )
            {
                SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
                if ( pNd->HasContour() )
                    pNd->SetContour( 0 );
            }
            
            
            rInvFlags |= 0x80;
        }
        break;

        case RES_PROTECT:
            {
            const SvxProtectItem *pP = (SvxProtectItem*)pNew;
            GetVirtDrawObj()->SetMoveProtect( pP->IsPosProtected()   );
            GetVirtDrawObj()->SetResizeProtect( pP->IsSizeProtected() );
            if( pSh )
            {
                SwRootFrm* pLayout = getRootFrm();
                if( pLayout && pLayout->IsAnyShellAccessible() )
                pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
            }
            break;
            }

        case RES_COL:
            {
                ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
                const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
                if ( FrmSizeChg( rNew ) )
                    NotifyDrawObj();
                rInvFlags |= 0x1A;
                break;
            }

        case RES_FRM_SIZE:
        case RES_FMT_CHG:
        {
            const SwFmtFrmSize &rNew = GetFmt()->GetFrmSize();
            if ( FrmSizeChg( rNew ) )
                NotifyDrawObj();
            rInvFlags |= 0x7F;
            if ( RES_FMT_CHG == nWhich )
            {
                SwRect aNew( GetObjRectWithSpaces() );
                SwRect aOld( maFrm );
                const SvxULSpaceItem &rUL = ((SwFmtChg*)pOld)->pChangedFmt->GetULSpace();
                aOld.Top( std::max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
                const SvxLRSpaceItem &rLR = ((SwFmtChg*)pOld)->pChangedFmt->GetLRSpace();
                aOld.Left  ( std::max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
                aNew.Union( aOld );
                NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );

                
                
                
                
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                ChgColumns( aCol, GetFmt()->GetCol() );
            }

            SwFmtURL aURL( GetFmt()->GetURL() );
            if ( aURL.GetMap() )
            {
                const SwFmtFrmSize &rOld = nWhich == RES_FRM_SIZE ?
                                *(SwFmtFrmSize*)pNew :
                                ((SwFmtChg*)pOld)->pChangedFmt->GetFrmSize();
                
                if ( rOld.GetWidth() && rOld.GetHeight() )
                {

                    Fraction aScaleX( rOld.GetWidth(), rNew.GetWidth() );
                    Fraction aScaleY( rOld.GetHeight(), rOld.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            const SvxProtectItem &rP = GetFmt()->GetProtect();
            GetVirtDrawObj()->SetMoveProtect( rP.IsPosProtected()    );
            GetVirtDrawObj()->SetResizeProtect( rP.IsSizeProtected() );

            if ( pSh )
                pSh->InvalidateWindows( Frm() );
            const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
            const sal_uInt8 nId = GetFmt()->GetOpaque().GetValue() ?
                             pIDDMA->GetHeavenId() :
                             pIDDMA->GetHellId();
            GetVirtDrawObj()->SetLayer( nId );

            if ( Lower() )
            {
                
                if( Lower()->IsNoTxtFrm() &&
                     !GetFmt()->GetSurround().IsContour() )
                {
                    SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
                    if ( pNd->HasContour() )
                        pNd->SetContour( 0 );
                }
                else if( !Lower()->IsColumnFrm() )
                {
                    SwFrm* pFrm = GetLastLower();
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        pFrm->Prepare( PREP_ADJUST_FRM );
                }
            }

            
            
            rInvFlags |= 0x80;

            break;
        }
        case RES_UL_SPACE:
        case RES_LR_SPACE:
        {
            rInvFlags |= 0x41;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                getRootFrm()->InvalidateBrowseWidth();
            SwRect aNew( GetObjRectWithSpaces() );
            SwRect aOld( maFrm );
            if ( RES_UL_SPACE == nWhich )
            {
                const SvxULSpaceItem &rUL = *(SvxULSpaceItem*)pNew;
                aOld.Top( std::max( aOld.Top() - long(rUL.GetUpper()), 0L ) );
                aOld.SSize().Height()+= rUL.GetLower();
            }
            else
            {
                const SvxLRSpaceItem &rLR = *(SvxLRSpaceItem*)pNew;
                aOld.Left  ( std::max( aOld.Left() - long(rLR.GetLeft()), 0L ) );
                aOld.SSize().Width() += rLR.GetRight();
            }
            aNew.Union( aOld );
            NotifyBackground( FindPageFrm(), aNew, PREP_CLEAR );
        }
        break;

        case RES_BOX:
        case RES_SHADOW:
            rInvFlags |= 0x17;
            break;

        case RES_FRAMEDIR :
            SetDerivedVert( sal_False );
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;

        case RES_OPAQUE:
            {
                if ( pSh )
                    pSh->InvalidateWindows( Frm() );

                const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();
                const sal_uInt8 nId = ((SvxOpaqueItem*)pNew)->GetValue() ?
                                    pIDDMA->GetHeavenId() :
                                    pIDDMA->GetHellId();
                GetVirtDrawObj()->SetLayer( nId );
                if( pSh )
                {
                    SwRootFrm* pLayout = getRootFrm();
                    if( pLayout && pLayout->IsAnyShellAccessible() )
                {
                    pSh->Imp()->DisposeAccessibleFrm( this );
                    pSh->Imp()->AddAccessibleFrm( this );
                }
                }
                
                
                rInvFlags |= 0x80;
            }
            break;

        case RES_URL:
            
            
            if ( (!Lower() || !Lower()->IsNoTxtFrm()) &&
                 ((SwFmtURL*)pNew)->GetMap() && ((SwFmtURL*)pOld)->GetMap() )
            {
                const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
                if ( rSz.GetHeight() != Frm().Height() ||
                     rSz.GetWidth()  != Frm().Width() )
                {
                    SwFmtURL aURL( GetFmt()->GetURL() );
                    Fraction aScaleX( Frm().Width(),  rSz.GetWidth() );
                    Fraction aScaleY( Frm().Height(), rSz.GetHeight() );
                    aURL.GetMap()->Scale( aScaleX, aScaleY );
                    SwFrmFmt *pFmt = GetFmt();
                    pFmt->LockModify();
                    pFmt->SetFmtAttr( aURL );
                    pFmt->UnlockModify();
                }
            }
            
            break;

        case RES_CHAIN:
            {
                SwFmtChain *pChain = (SwFmtChain*)pNew;
                if ( pChain->GetNext() )
                {
                    SwFlyFrm *pFollow = FindChainNeighbour( *pChain->GetNext() );
                    if ( GetNextLink() && pFollow != GetNextLink() )
                        SwFlyFrm::UnchainFrames( this, GetNextLink());
                    if ( pFollow )
                    {
                        if ( pFollow->GetPrevLink() &&
                             pFollow->GetPrevLink() != this )
                            SwFlyFrm::UnchainFrames( pFollow->GetPrevLink(),
                                                     pFollow );
                        if ( !GetNextLink() )
                            SwFlyFrm::ChainFrames( this, pFollow );
                    }
                }
                else if ( GetNextLink() )
                    SwFlyFrm::UnchainFrames( this, GetNextLink() );
                if ( pChain->GetPrev() )
                {
                    SwFlyFrm *pMaster = FindChainNeighbour( *pChain->GetPrev() );
                    if ( GetPrevLink() && pMaster != GetPrevLink() )
                        SwFlyFrm::UnchainFrames( GetPrevLink(), this );
                    if ( pMaster )
                    {
                        if ( pMaster->GetNextLink() &&
                             pMaster->GetNextLink() != this )
                            SwFlyFrm::UnchainFrames( pMaster,
                                                     pMaster->GetNextLink() );
                        if ( !GetPrevLink() )
                            SwFlyFrm::ChainFrames( pMaster, this );
                    }
                }
                else if ( GetPrevLink() )
                    SwFlyFrm::UnchainFrames( GetPrevLink(), this );
            }

        default:
            bClear = false;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}


bool SwFlyFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
        return false;   
    return true;        
}

void SwFlyFrm::_Invalidate( SwPageFrm *pPage )
{
    InvalidatePage( pPage );
    bNotifyBack = bInvalid = sal_True;

    SwFlyFrm *pFrm;
    if ( GetAnchorFrm() && 0 != (pFrm = AnchorFrm()->FindFlyFrm()) )
    {
        
        
        if ( !pFrm->IsLocked() && !pFrm->IsColLocked() &&
             pFrm->Lower() && pFrm->Lower()->IsColumnFrm() )
            pFrm->InvalidateSize();
    }

    
    
    
    
    if ( GetVertPosOrientFrm() && GetVertPosOrientFrm()->IsLayoutFrm() )
    {
        const SwSectionFrm* pSectFrm( GetVertPosOrientFrm()->FindSctFrm() );
        if ( pSectFrm && pSectFrm->GetSection() == 0 )
        {
            InvalidatePos();
            ClearVertPosOrientFrm();
        }
    }
}

/** Change the relative position
 *
 * The position will be Fix automatically and the attribute is changed accordingly.
 */
void SwFlyFrm::ChgRelPos( const Point &rNewPos )
{
    if ( GetCurrRelPos() != rNewPos )
    {
        SwFrmFmt *pFmt = GetFmt();
        const bool bVert = GetAnchorFrm()->IsVertical();
        const SwTwips nNewY = bVert ? rNewPos.X() : rNewPos.Y();
        SwTwips nTmpY = nNewY == LONG_MAX ? 0 : nNewY;
        if( bVert )
            nTmpY = -nTmpY;
        SfxItemSet aSet( pFmt->GetDoc()->GetAttrPool(),
                         RES_VERT_ORIENT, RES_HORI_ORIENT);

        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwTxtFrm *pAutoFrm = NULL;
        
        
        const RndStdIds eAnchorType = GetFrmFmt().GetAnchor().GetAnchorId();
        if ( eAnchorType == FLY_AT_PAGE )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
        }
        else if ( eAnchorType == FLY_AT_FLY )
        {
            aVert.SetVertOrient( text::VertOrientation::NONE );
            aVert.SetRelationOrient( text::RelOrientation::FRAME );
        }
        else if ( IsFlyAtCntFrm() || text::VertOrientation::NONE != aVert.GetVertOrient() )
        {
            if( text::RelOrientation::CHAR == aVert.GetRelationOrient() && IsAutoPos() )
            {
                if( LONG_MAX != nNewY )
                {
                    aVert.SetVertOrient( text::VertOrientation::NONE );
                    sal_Int32 nOfs =
                        pFmt->GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected" );
                    pAutoFrm = (SwTxtFrm*)GetAnchorFrm();
                    while( pAutoFrm->GetFollow() &&
                           pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                    {
                        if( pAutoFrm == GetAnchorFrm() )
                            nTmpY += pAutoFrm->GetRelPos().Y();
                        nTmpY -= pAutoFrm->GetUpper()->Prt().Height();
                        pAutoFrm = pAutoFrm->GetFollow();
                    }
                    nTmpY = ((SwFlyAtCntFrm*)this)->GetRelCharY(pAutoFrm)-nTmpY;
                }
                else
                    aVert.SetVertOrient( text::VertOrientation::CHAR_BOTTOM );
            }
            else
            {
                aVert.SetVertOrient( text::VertOrientation::NONE );
                aVert.SetRelationOrient( text::RelOrientation::FRAME );
            }
        }
        aVert.SetPos( nTmpY );
        aSet.Put( aVert );

        
        
        if ( !IsFlyInCntFrm() )
        {
            const SwTwips nNewX = bVert ? rNewPos.Y() : rNewPos.X();
            SwTwips nTmpX = nNewX == LONG_MAX ? 0 : nNewX;
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            
            
            if ( eAnchorType == FLY_AT_PAGE )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::PAGE_FRAME );
                aHori.SetPosToggle( sal_False );
            }
            else if ( eAnchorType == FLY_AT_FLY )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( sal_False );
            }
            else if ( IsFlyAtCntFrm() || text::HoriOrientation::NONE != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                if( text::RelOrientation::CHAR == aHori.GetRelationOrient() && IsAutoPos() )
                {
                    if( LONG_MAX != nNewX )
                    {
                        if( !pAutoFrm )
                        {
                            sal_Int32 nOfs = pFmt->GetAnchor().GetCntntAnchor()
                                          ->nContent.GetIndex();
                            OSL_ENSURE( GetAnchorFrm()->IsTxtFrm(), "TxtFrm expected");
                            pAutoFrm = (SwTxtFrm*)GetAnchorFrm();
                            while( pAutoFrm->GetFollow() &&
                                   pAutoFrm->GetFollow()->GetOfst() <= nOfs )
                                pAutoFrm = pAutoFrm->GetFollow();
                        }
                        nTmpX -= ((SwFlyAtCntFrm*)this)->GetRelCharX(pAutoFrm);
                    }
                }
                else
                    aHori.SetRelationOrient( text::RelOrientation::FRAME );
                aHori.SetPosToggle( sal_False );
            }
            aHori.SetPos( nTmpX );
            aSet.Put( aHori );
        }
        SetCurrRelPos( rNewPos );
        pFmt->GetDoc()->SetAttr( aSet, *pFmt );
    }
}

/** "Formats" the Frame; Frm and PrtArea.
 *
 * The FixSize is not inserted here.
 */
void SwFlyFrm::Format( const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "FlyFrm::Format, pAttrs is 0." );

    ColLock();

    if ( !mbValidSize )
    {
        if ( Frm().Top() == FAR_AWAY && Frm().Left() == FAR_AWAY )
        {
            
            Frm().Pos().setX(0);
            Frm().Pos().setY(0);
            
            InvalidateObjRectWithSpaces();
        }

        
        if ( Lower() && Lower()->IsColumnFrm() )
            AdjustColumns( 0, sal_False );

        mbValidSize = sal_True;

        const SwTwips nUL = pAttrs->CalcTopLine()  + pAttrs->CalcBottomLine();
        const SwTwips nLR = pAttrs->CalcLeftLine() + pAttrs->CalcRightLine();
        const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
              Size aRelSize( CalcRel( rFrmSz ) );

        OSL_ENSURE( pAttrs->GetSize().Height() != 0 || rFrmSz.GetHeightPercent(), "FrameAttr height is 0." );
        OSL_ENSURE( pAttrs->GetSize().Width()  != 0 || rFrmSz.GetWidthPercent(), "FrameAttr width is 0." );

        SWRECTFN( this )
        if( !HasFixSize() )
        {
            SwTwips nRemaining = 0;

            long nMinHeight = 0;
            if( IsMinHeight() )
                nMinHeight = bVert ? aRelSize.Width() : aRelSize.Height();

            if ( Lower() )
            {
                if ( Lower()->IsColumnFrm() )
                {
                    FormatWidthCols( *pAttrs, nUL, nMinHeight );
                    nRemaining = (Lower()->Frm().*fnRect->fnGetHeight)();
                }
                else
                {
                    SwFrm *pFrm = Lower();
                    while ( pFrm )
                    {
                        nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                            
                            nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                                    - (pFrm->Prt().*fnRect->fnGetHeight)();
                        else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
                            nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                        pFrm = pFrm->GetNext();
                    }
                }
                if ( GetDrawObjs() )
                {
                    sal_uInt32 nCnt = GetDrawObjs()->Count();
                    SwTwips nTop = (Frm().*fnRect->fnGetTop)();
                    SwTwips nBorder = (Frm().*fnRect->fnGetHeight)() -
                                      (Prt().*fnRect->fnGetHeight)();
                    for ( sal_uInt16 i = 0; i < nCnt; ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                            
                            
                            if ( pFly->IsFlyLayFrm() &&
                                 pFly->Frm().Top() != FAR_AWAY &&
                                 pFly->GetFmt()->GetFollowTextFlow().GetValue() )
                            {
                                SwTwips nDist = -(pFly->Frm().*fnRect->
                                    fnBottomDist)( nTop );
                                if( nDist > nBorder + nRemaining )
                                    nRemaining = nDist - nBorder;
                            }
                        }
                    }
                }
            }

            if( IsMinHeight() && (nRemaining + nUL) < nMinHeight )
                nRemaining = nMinHeight - nUL;
            
            
            
            
            
            
            if ( nRemaining < MINFLY )
                nRemaining = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nRemaining );
            nRemaining -= (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nRemaining + nUL );
            
            if ( nRemaining + nUL != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
            mbValidSize = sal_True;
        }
        else
        {
            mbValidSize = sal_True;  
            
            SwTwips nNewSize = bVert ? aRelSize.Width() : aRelSize.Height();
            nNewSize -= nUL;
            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetHeight)( nNewSize );
            nNewSize += nUL - (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnAddBottom)( nNewSize );
            
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }

        if ( !bFormatHeightOnly )
        {
            OSL_ENSURE( aRelSize == CalcRel( rFrmSz ), "SwFlyFrm::Format CalcRel problem" );
            SwTwips nNewSize = bVert ? aRelSize.Height() : aRelSize.Width();

            if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
            {
                
                const SwTwips nAutoWidth = CalcAutoWidth();
                if ( nAutoWidth )
                {
                    if( ATT_MIN_SIZE == rFrmSz.GetWidthSizeType() )
                        nNewSize = std::max( nNewSize - nLR, nAutoWidth );
                    else
                        nNewSize = nAutoWidth;
                }
            }
            else
                nNewSize -= nLR;

            if( nNewSize < MINFLY )
                nNewSize = MINFLY;
            (Prt().*fnRect->fnSetWidth)( nNewSize );
            nNewSize += nLR - (Frm().*fnRect->fnGetWidth)();
            (Frm().*fnRect->fnAddRight)( nNewSize );
            
            if ( nNewSize != 0 )
            {
                InvalidateObjRectWithSpaces();
            }
        }
    }
    ColUnlock();
}











void CalcCntnt( SwLayoutFrm *pLay,
                bool bNoColl,
                bool bNoCalcFollow )
{
    SwSectionFrm* pSect;
    bool bCollect = false;
    if( pLay->IsSctFrm() )
    {
        pSect = (SwSectionFrm*)pLay;
        if( pSect->IsEndnAtEnd() && !bNoColl )
        {
            bCollect = true;
            SwLayouter::CollectEndnotes( pLay->GetFmt()->GetDoc(), pSect );
        }
        pSect->CalcFtnCntnt();
    }
    else
        pSect = NULL;
    SwFrm *pFrm = pLay->ContainsAny();
    if ( !pFrm )
    {
        if( pSect )
        {
            if( pSect->HasFollow() )
                pFrm = pSect->GetFollow()->ContainsAny();
            if( !pFrm )
            {
                if( pSect->IsEndnAtEnd() )
                {
                    if( bCollect )
                        pLay->GetFmt()->GetDoc()->GetLayouter()->
                            InsertEndnotes( pSect );
                    sal_Bool bLock = pSect->IsFtnLock();
                    pSect->SetFtnLock( true );
                    pSect->CalcFtnCntnt();
                    pSect->CalcFtnCntnt();
                    pSect->SetFtnLock( bLock );
                }
                return;
            }
            pFrm->_InvalidatePos();
        }
        else
            return;
    }
    pFrm->InvalidatePage();

    do
    {
        
        SwAnchoredObject* pAgainObj1 = 0;
        SwAnchoredObject* pAgainObj2 = 0;

        
        sal_uInt16 nLoopControlRuns = 0;
        const sal_uInt16 nLoopControlMax = 20;
        const SwFrm* pLoopControlCond = 0;

        SwFrm* pLast;
        do
        {
            pLast = pFrm;
            if( pFrm->IsVertical() ?
                ( pFrm->GetUpper()->Prt().Height() != pFrm->Frm().Height() )
                : ( pFrm->GetUpper()->Prt().Width() != pFrm->Frm().Width() ) )
            {
                pFrm->Prepare( PREP_FIXSIZE_CHG );
                pFrm->_InvalidateSize();
            }

            if ( pFrm->IsTabFrm() )
            {
                ((SwTabFrm*)pFrm)->bCalcLowers = sal_True;
                
                
                
                if ( ((SwTabFrm*)pFrm)->IsFollow() &&
                     ( !pSect || pSect == pFrm->FindSctFrm() ) )
                {
                    ((SwTabFrm*)pFrm)->bLockBackMove = sal_True;
                }
            }

            
            if ( bNoCalcFollow && pFrm->IsTxtFrm() )
                static_cast<SwTxtFrm*>(pFrm)->ForbidFollowFormat();

            pFrm->Calc();

            
            if ( pFrm->IsTxtFrm() )
            {
                static_cast<SwTxtFrm*>(pFrm)->AllowFollowFormat();
            }

            
            
            
            
            
            
            
            
            SwFrm* pTmpPrev = pFrm->FindPrev();
            SwFlowFrm* pTmpPrevFlowFrm = pTmpPrev && pTmpPrev->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pTmpPrev) : 0;
            SwFlowFrm* pTmpFlowFrm     = pFrm->IsFlowFrm() ? SwFlowFrm::CastFlowFrm(pFrm) : 0;

            bool bPrevInvalid = pTmpPrevFlowFrm && pTmpFlowFrm &&
                               !pTmpFlowFrm->IsFollow() &&
                               !StackHack::IsLocked() && 
                               !pTmpFlowFrm->IsJoinLocked() &&
                               !pTmpPrev->GetValidPosFlag() &&
                                pLay->IsAnLower( pTmpPrev ) &&
                                pTmpPrevFlowFrm->IsKeep( *pTmpPrev->GetAttrSet() ) &&
                                pTmpPrevFlowFrm->IsKeepFwdMoveAllowed();

            
            bool bRestartLayoutProcess = false;
            if ( !bPrevInvalid && pFrm->GetDrawObjs() && pLay->IsAnLower( pFrm ) )
            {
                bool bAgain = false;
                SwPageFrm* pPageFrm = pFrm->FindPageFrm();
                sal_uInt32 nCnt = pFrm->GetDrawObjs()->Count();
                for ( sal_uInt16 i = 0; i < nCnt; ++i )
                {
                    
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    
                    if ( pAnchoredObj->PositionLocked() )
                    {
                        continue;
                    }

                    
                    if ( pAnchoredObj->IsFormatPossible() )
                    {
                        
                        
                        
                        
                        SwRect aRect( pAnchoredObj->GetObjRect() );
                        if ( !SwObjectFormatter::FormatObj( *pAnchoredObj, pFrm, pPageFrm ) )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }
                        
                        
                        if ( pAnchoredObj->PositionLocked() )
                        {
                            bRestartLayoutProcess = true;
                            break;
                        }

                        if ( aRect != pAnchoredObj->GetObjRect() )
                        {
                            bAgain = true;
                            if ( pAgainObj2 == pAnchoredObj )
                            {
                                OSL_FAIL( "::CalcCntnt(..) - loop detected, perform attribute changes to avoid the loop" );
                                
                                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                                SwFmtSurround aAttr( rFmt.GetSurround() );
                                if( SURROUND_THROUGHT != aAttr.GetSurround() )
                                {
                                    
                                    
                                    if ((rFmt.GetAnchor().GetAnchorId() ==
                                            FLY_AT_CHAR) &&
                                        (SURROUND_PARALLEL ==
                                            aAttr.GetSurround()))
                                    {
                                        aAttr.SetSurround( SURROUND_THROUGHT );
                                    }
                                    else
                                    {
                                        aAttr.SetSurround( SURROUND_PARALLEL );
                                    }
                                    rFmt.LockModify();
                                    rFmt.SetFmtAttr( aAttr );
                                    rFmt.UnlockModify();
                                }
                            }
                            else
                            {
                                if ( pAgainObj1 == pAnchoredObj )
                                    pAgainObj2 = pAnchoredObj;
                                pAgainObj1 = pAnchoredObj;
                            }
                        }

                        if ( !pFrm->GetDrawObjs() )
                            break;
                        if ( pFrm->GetDrawObjs()->Count() < nCnt )
                        {
                            --i;
                            --nCnt;
                        }
                    }
                }

                
                
                if ( bRestartLayoutProcess )
                {
                    pFrm = pLay->ContainsAny();
                    pAgainObj1 = 0L;
                    pAgainObj2 = 0L;
                    continue;
                }

                
                
                if ( pLay->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    pFrm->Calc();
                }

                if ( bAgain )
                {
                    pFrm = pLay->ContainsCntnt();
                    if ( pFrm && pFrm->IsInTab() )
                        pFrm = pFrm->FindTabFrm();
                    if( pFrm && pFrm->IsInSct() )
                    {
                        SwSectionFrm* pTmp = pFrm->FindSctFrm();
                        if( pTmp != pLay && pLay->IsAnLower( pTmp ) )
                            pFrm = pTmp;
                    }

                    if ( pFrm == pLoopControlCond )
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pFrm;
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                        continue;

                    OSL_FAIL( "LoopControl in CalcCntnt" );
                }
            }
            if ( pFrm->IsTabFrm() )
            {
                if ( ((SwTabFrm*)pFrm)->IsFollow() )
                    ((SwTabFrm*)pFrm)->bLockBackMove = sal_False;
            }

            pFrm = bPrevInvalid ? pTmpPrev : pFrm->FindNext();
            if( !bPrevInvalid && pFrm && pFrm->IsSctFrm() && pSect )
            {
                
                while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                    pFrm = pFrm->FindNext();

                
                
                if( pFrm && pFrm->IsSctFrm() && ( pFrm == pSect->GetFollow() ||
                    ((SwSectionFrm*)pFrm)->IsAnFollow( pSect ) ) )
                {
                    pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pFrm )
                        pFrm->_InvalidatePos();
                }
            }
          
          
          
          
        } while ( pFrm &&
                  ( pLay->IsAnLower( pFrm ) ||
                    ( pSect &&
                      ( ( pSect->HasFollow() &&
                          ( pLay->IsAnLower( pLast ) ||
                            ( pLast->IsInSct() &&
                              pLast->FindSctFrm()->IsAnFollow(pSect) ) ) &&
                          pSect->GetFollow()->IsAnLower( pFrm )  ) ||
                        ( pFrm->IsInSct() &&
                          pFrm->FindSctFrm()->IsAnFollow( pSect ) ) ) ) ) );
        if( pSect )
        {
            if( bCollect )
            {
                pLay->GetFmt()->GetDoc()->GetLayouter()->InsertEndnotes(pSect);
                pSect->CalcFtnCntnt();
            }
            if( pSect->HasFollow() )
            {
                SwSectionFrm* pNxt = pSect->GetFollow();
                while( pNxt && !pNxt->ContainsCntnt() )
                    pNxt = pNxt->GetFollow();
                if( pNxt )
                    pNxt->CalcFtnCntnt();
            }
            if( bCollect )
            {
                pFrm = pLay->ContainsAny();
                bCollect = false;
                if( pFrm )
                    continue;
            }
        }
        break;
    }
    while( true );
}


void SwFlyFrm::MakeObjPos()
{
    if ( !mbValidPos )
    {
        mbValidPos = sal_True;

        
        GetAnchorFrm()->Calc();
        objectpositioning::SwToLayoutAnchoredObjectPosition
                aObjPositioning( *GetVirtDrawObj() );
        aObjPositioning.CalcPosition();

        
        
        SetCurrRelPos( aObjPositioning.GetRelPos() );

        SWRECTFN( GetAnchorFrm() );
        maFrm.Pos( aObjPositioning.GetRelPos() );
        maFrm.Pos() += (GetAnchorFrm()->Frm().*fnRect->fnGetPos)();
        
        InvalidateObjRectWithSpaces();
    }
}

void SwFlyFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{

    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = sal_True;

        
        SWRECTFN( this )
        (this->*fnRect->fnSetXMargins)( rAttrs.CalcLeftLine(),
                                        rAttrs.CalcRightLine() );
        (this->*fnRect->fnSetYMargins)( rAttrs.CalcTopLine(),
                                        rAttrs.CalcBottomLine() );
    }
}

SwTwips SwFlyFrm::_Grow( SwTwips nDist, sal_Bool bTst )
{
    SWRECTFN( this )
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        SwTwips nSize = (Frm().*fnRect->fnGetHeight)();
        if( nSize > 0 && nDist > ( LONG_MAX - nSize ) )
            nDist = LONG_MAX - nSize;

        if ( nDist <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   
            
            if ( !bTst )
            {
                
                UnlockPosition();
                _InvalidatePos();
                InvalidateSize();
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const sal_Bool bOldLock = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                
                
                
                
                
                
                mbValidPos = sal_True;
                
                
                
                
                const sal_Bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = sal_True;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLock )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
                ::Notify( this, FindPageFrm(), aOld );
            return (aNew.*fnRect->fnGetHeight)()-(aOld.*fnRect->fnGetHeight)();
        }
        return nDist;
    }
    return 0L;
}

SwTwips SwFlyFrm::_Shrink( SwTwips nDist, sal_Bool bTst )
{
    if( Lower() && !IsColLocked() && !HasFixSize() && !IsNoShrink() )
    {
        SWRECTFN( this )
        SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
        if ( nDist > nHeight )
            nDist = nHeight;

        SwTwips nVal = nDist;
        if ( IsMinHeight() )
        {
            const SwFmtFrmSize& rFmtSize = GetFmt()->GetFrmSize();
            SwTwips nFmtHeight = bVert ? rFmtSize.GetWidth() : rFmtSize.GetHeight();

            nVal = std::min( nDist, nHeight - nFmtHeight );
        }

        if ( nVal <= 0L )
            return 0L;

        if ( Lower()->IsColumnFrm() )
        {   
            
            if ( !bTst )
            {
                SwRect aOld( GetObjRectWithSpaces() );
                (Frm().*fnRect->fnSetHeight)( nHeight - nVal );
                
                if ( nHeight - nVal != 0 )
                {
                    InvalidateObjRectWithSpaces();
                }
                nHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nHeight - nVal );
                _InvalidatePos();
                InvalidateSize();
                ::Notify( this, FindPageFrm(), aOld );
                NotifyDrawObj();
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return 0L;
        }

        if ( !bTst )
        {
            const SwRect aOld( GetObjRectWithSpaces() );
            _InvalidateSize();
            const sal_Bool bOldLocked = bLocked;
            Unlock();
            if ( IsFlyFreeFrm() )
            {
                
                
                
                
                
                
                mbValidPos = sal_True;
                
                
                
                
                const sal_Bool bOldFormatHeightOnly = bFormatHeightOnly;
                const SwFmtFrmSize& rFrmSz = GetFmt()->GetFrmSize();
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = sal_True;
                }
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( true );
                ((SwFlyFreeFrm*)this)->SwFlyFreeFrm::MakeAll();
                static_cast<SwFlyFreeFrm*>(this)->SetNoMoveOnCheckClip( false );
                
                if ( rFrmSz.GetWidthSizeType() != ATT_FIX_SIZE )
                {
                    bFormatHeightOnly = bOldFormatHeightOnly;
                }
            }
            else
                MakeAll();
            _InvalidateSize();
            InvalidatePos();
            if ( bOldLocked )
                Lock();
            const SwRect aNew( GetObjRectWithSpaces() );
            if ( aOld != aNew )
            {
                ::Notify( this, FindPageFrm(), aOld );
                if ( GetAnchorFrm()->IsInFly() )
                    AnchorFrm()->FindFlyFrm()->Shrink( nDist, bTst );
            }
            return (aOld.*fnRect->fnGetHeight)() -
                   (aNew.*fnRect->fnGetHeight)();
        }
        return nVal;
    }
    return 0L;
}

Size SwFlyFrm::ChgSize( const Size& aNewSize )
{
    
    
    
    
    Size aAdjustedNewSize( aNewSize );
    {
        if ( dynamic_cast<SwFlyAtCntFrm*>(this) &&
             Lower() && dynamic_cast<SwNoTxtFrm*>(Lower()) &&
             static_cast<SwNoTxtFrm*>(Lower())->GetNode()->GetOLENode() )
        {
            SwRect aClipRect;
            ::CalcClipRect( GetVirtDrawObj(), aClipRect, sal_False );
            if ( aAdjustedNewSize.Width() > aClipRect.Width() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Width() );
            }
            if ( aAdjustedNewSize.Height() > aClipRect.Height() )
            {
                aAdjustedNewSize.setWidth( aClipRect.Height() );
            }
        }
    }
    if ( aAdjustedNewSize != Frm().SSize() )
    {
        SwFrmFmt *pFmt = GetFmt();
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        aSz.SetWidth( aAdjustedNewSize.Width() );
        aSz.SetHeight( aAdjustedNewSize.Height() );
        
        pFmt->GetDoc()->SetAttr( aSz, *pFmt );
        return aSz.GetSize();
    }
    else
        return Frm().SSize();
}

sal_Bool SwFlyFrm::IsLowerOf( const SwLayoutFrm* pUpperFrm ) const
{
    OSL_ENSURE( GetAnchorFrm(), "8-( Fly is lost in Space." );
    const SwFrm* pFrm = GetAnchorFrm();
    do
    {
        if ( pFrm == pUpperFrm )
            return sal_True;
        pFrm = pFrm->IsFlyFrm()
               ? ((const SwFlyFrm*)pFrm)->GetAnchorFrm()
               : pFrm->GetUpper();
    } while ( pFrm );
    return sal_False;
}

void SwFlyFrm::Cut()
{
    
}

void SwFrm::AppendFly( SwFlyFrm *pNew )
{
    if ( !mpDrawObjs )
        mpDrawObjs = new SwSortedObjs();
    mpDrawObjs->Insert( *pNew );
    pNew->ChgAnchorFrm( this );

    
    
    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        if ( pNew->IsFlyAtCntFrm() && pNew->Frm().Top() == FAR_AWAY )
        {
            
            
            
            
            
            SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            if( !SwLayHelper::CheckPageFlyCache( pPage, pNew ) )
            {
                SwPageFrm *pTmp = pRoot->GetLastPage();
                if ( pTmp->GetPhyPageNum() > 30 )
                {
                    for ( sal_uInt16 i = 0; i < 10; ++i )
                    {
                        pTmp = (SwPageFrm*)pTmp->GetPrev();
                        if( pTmp->GetPhyPageNum() <= pPage->GetPhyPageNum() )
                            break; 
                    }
                    if ( pTmp->IsEmptyPage() )
                        pTmp = (SwPageFrm*)pTmp->GetPrev();
                    pPage = pTmp;
                }
            }
            pPage->AppendFlyToPage( pNew );
        }
        else
            pPage->AppendFlyToPage( pNew );
    }
}

void SwFrm::RemoveFly( SwFlyFrm *pToRemove )
{
    
    
    SwPageFrm *pPage = pToRemove->FindPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
    {
        pPage->RemoveFlyFromPage( pToRemove );
    }
    
    else
    {
        if ( pToRemove->IsAccessibleFrm() &&
             pToRemove->GetFmt() &&
             !pToRemove->IsFlyInCntFrm() )
        {
            SwRootFrm *pRootFrm = getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
            {
                SwViewShell *pVSh = pRootFrm->GetCurrShell();
                if( pVSh && pVSh->Imp() )
                {
                    pVSh->Imp()->DisposeAccessibleFrm( pToRemove );
                }
            }
        }
    }

    mpDrawObjs->Remove( *pToRemove );
    if ( !mpDrawObjs->Count() )
        DELETEZ( mpDrawObjs );

    pToRemove->ChgAnchorFrm( 0 );

    if ( !pToRemove->IsFlyInCntFrm() && GetUpper() && IsInTab() )
        GetUpper()->InvalidateSize();
}

void SwFrm::AppendDrawObj( SwAnchoredObject& _rNewObj )
{
    if ( !_rNewObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwFrm::AppendDrawObj(..) - anchored object of unexcepted type -> object not appended" );
        return;
    }

    if ( !_rNewObj.GetDrawObj()->ISA(SwDrawVirtObj) &&
         _rNewObj.GetAnchorFrm() && _rNewObj.GetAnchorFrm() != this )
    {
        
        
        static_cast<SwDrawContact*>(::GetUserCall( _rNewObj.GetDrawObj() ))->
                                                DisconnectFromLayout( false );
    }

    if ( _rNewObj.GetAnchorFrm() != this )
    {
        if ( !mpDrawObjs )
            mpDrawObjs = new SwSortedObjs();
        mpDrawObjs->Insert( _rNewObj );
        _rNewObj.ChgAnchorFrm( this );
    }

    
    
    if ( ::CheckControlLayer( _rNewObj.DrawObj() ) )
    {
        const IDocumentDrawModelAccess* pIDDMA = getIDocumentDrawModelAccess();
        const SdrLayerID aCurrentLayer(_rNewObj.DrawObj()->GetLayer());
        const SdrLayerID aControlLayerID(pIDDMA->GetControlsId());
        const SdrLayerID aInvisibleControlLayerID(pIDDMA->GetInvisibleControlsId());

        if(aCurrentLayer != aControlLayerID && aCurrentLayer != aInvisibleControlLayerID)
        {
            if ( aCurrentLayer == pIDDMA->GetInvisibleHellId() ||
                 aCurrentLayer == pIDDMA->GetInvisibleHeavenId() )
            {
                _rNewObj.DrawObj()->SetLayer(aInvisibleControlLayerID);
            }
            else
            {
                _rNewObj.DrawObj()->SetLayer(aControlLayerID);
            }
        }
    }

    
    _rNewObj.InvalidateObjPos();

    
    SwPageFrm* pPage = FindPageFrm();
    if ( pPage )
    {
        pPage->AppendDrawObjToPage( _rNewObj );
    }

    
    SwViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->AddAccessibleObj( _rNewObj.GetDrawObj() );
    }
}

void SwFrm::RemoveDrawObj( SwAnchoredObject& _rToRemoveObj )
{
    
    SwViewShell* pSh = getRootFrm()->GetCurrShell();
    if( pSh )
    {
        SwRootFrm* pLayout = getRootFrm();
        if( pLayout && pLayout->IsAnyShellAccessible() )
        pSh->Imp()->DisposeAccessibleObj( _rToRemoveObj.GetDrawObj() );
    }

    
    SwPageFrm* pPage = _rToRemoveObj.GetPageFrm();
    if ( pPage && pPage->GetSortedObjs() )
        pPage->RemoveDrawObjFromPage( _rToRemoveObj );

    mpDrawObjs->Remove( _rToRemoveObj );
    if ( !mpDrawObjs->Count() )
        DELETEZ( mpDrawObjs );

    _rToRemoveObj.ChgAnchorFrm( 0 );
}

void SwFrm::InvalidateObjs( const bool _bInvaPosOnly,
                            const bool _bNoInvaOfAsCharAnchoredObjs )
{
    if ( GetDrawObjs() )
    {
        
        
        
        const SwPageFrm* pPageFrm = FindPageFrm();
        
        sal_uInt32 i = 0;
        for ( ; i < GetDrawObjs()->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
            if ( _bNoInvaOfAsCharAnchoredObjs &&
                 (pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                    == FLY_AS_CHAR) )
            {
                continue;
            }
            
            
            
            if ( pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() != pPageFrm )
            {
                SwTxtFrm* pAnchorCharFrm = pAnchoredObj->FindAnchorCharFrm();
                if ( pAnchorCharFrm &&
                     pAnchoredObj->GetPageFrm() == pAnchorCharFrm->FindPageFrm() )
                {
                    continue;
                }
                
                
                
                
                else
                {
                    pAnchoredObj->UnlockPosition();
                }
            }
            
            
            
            if ( pAnchoredObj->ClearedEnvironment() &&
                 pAnchoredObj->GetPageFrm() &&
                 pAnchoredObj->GetPageFrm() == pPageFrm )
            {
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->SetClearedEnvironment( false );
            }
            
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                pFly->_Invalidate();
                pFly->_InvalidatePos();
                if ( !_bInvaPosOnly )
                    pFly->_InvalidateSize();
            }
            else
            {
                pAnchoredObj->InvalidateObjPos();
            }
        } 
    }
}






void SwLayoutFrm::NotifyLowerObjs( const bool _bUnlockPosOfObjs )
{
    
    SwPageFrm* pPageFrm = FindPageFrm();
    if ( pPageFrm && pPageFrm->GetSortedObjs() )
    {
        SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
        for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            
            
            
            
            
            
            const SwFrm* pAnchorFrm = pObj->GetAnchorFrmContainingAnchPos();
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);

                if ( pFly->Frm().Left() == FAR_AWAY )
                    continue;

                if ( pFly->IsAnLower( this ) )
                    continue;

                
                
                
                const bool bLow = IsAnLower( pAnchorFrm );
                if ( bLow || pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    pFly->_Invalidate( pPageFrm );
                    if ( !bLow || pFly->IsFlyAtCntFrm() )
                    {
                        
                        if ( _bUnlockPosOfObjs )
                        {
                            pFly->UnlockPosition();
                        }
                        pFly->_InvalidatePos();
                    }
                    else
                        pFly->_InvalidatePrt();
                }
            }
            else
            {
                OSL_ENSURE( pObj->ISA(SwAnchoredDrawObject),
                        "<SwLayoutFrm::NotifyFlys() - anchored object of unexcepted type" );
                
                
                
                if ( IsAnLower( pAnchorFrm ) ||
                     pAnchorFrm->FindPageFrm() != pPageFrm )
                {
                    
                    if ( _bUnlockPosOfObjs )
                    {
                        pObj->UnlockPosition();
                    }
                    pObj->InvalidateObjPos();
                }
            }
        }
    }
}

void SwFlyFrm::NotifyDrawObj()
{
    SwVirtFlyDrawObj* pObj = GetVirtDrawObj();
    pObj->SetRect();
    pObj->SetRectsDirty();
    pObj->SetChanged();
    pObj->BroadcastObjectChange();
    if ( GetFmt()->GetSurround().IsContour() )
        ClrContourCache( pObj );
}

Size SwFlyFrm::CalcRel( const SwFmtFrmSize &rSz ) const
{
    Size aRet( rSz.GetSize() );

    const SwFrm *pRel = IsFlyLayFrm() ? GetAnchorFrm() : GetAnchorFrm()->GetUpper();
    if( pRel ) 
    {
        long nRelWidth = LONG_MAX, nRelHeight = LONG_MAX;
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( ( pRel->IsBodyFrm() || pRel->IsPageFrm() ) &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             pSh->VisArea().HasArea() )
        {
            nRelWidth  = pSh->GetBrowseWidth();
            nRelHeight = pSh->VisArea().Height();
            Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            long nDiff = nRelWidth - pRel->Prt().Width();
            if ( nDiff > 0 )
                nRelWidth -= nDiff;
            nRelHeight -= 2*aBorder.Height();
            nDiff = nRelHeight - pRel->Prt().Height();
            if ( nDiff > 0 )
                nRelHeight -= nDiff;
        }

        
        
        if (rSz.GetWidthPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelWidth  = std::min( nRelWidth,  pRel->Prt().Width() );
        if (rSz.GetHeightPercentRelation() != text::RelOrientation::PAGE_FRAME)
            nRelHeight = std::min( nRelHeight, pRel->Prt().Height() );
        if( !pRel->IsPageFrm() )
        {
            const SwPageFrm* pPage = FindPageFrm();
            if( pPage )
            {
                if (rSz.GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    
                    nRelWidth  = std::min( nRelWidth,  pPage->Frm().Width() );
                else
                    nRelWidth  = std::min( nRelWidth,  pPage->Prt().Width() );
                if (rSz.GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
                    
                    nRelHeight = std::min( nRelHeight, pPage->Frm().Height() );
                else
                    nRelHeight = std::min( nRelHeight, pPage->Prt().Height() );
            }
        }

        if ( rSz.GetWidthPercent() && rSz.GetWidthPercent() != 0xFF )
            aRet.Width() = nRelWidth * rSz.GetWidthPercent() / 100;
        if ( rSz.GetHeightPercent() && rSz.GetHeightPercent() != 0xFF )
            aRet.Height() = nRelHeight * rSz.GetHeightPercent() / 100;

        if ( rSz.GetWidthPercent() == 0xFF )
        {
            aRet.Width() *= aRet.Height();
            aRet.Width() /= rSz.GetHeight();
        }
        else if ( rSz.GetHeightPercent() == 0xFF )
        {
            aRet.Height() *= aRet.Width();
            aRet.Height() /= rSz.GetWidth();
        }
    }
    return aRet;
}

static SwTwips lcl_CalcAutoWidth( const SwLayoutFrm& rFrm )
{
    SwTwips nRet = 0;
    SwTwips nMin = 0;
    const SwFrm* pFrm = rFrm.Lower();

    
    if ( !pFrm || pFrm->IsColumnFrm() )
        return nRet;

    while ( pFrm )
    {
        if ( pFrm->IsSctFrm() )
        {
            nMin = lcl_CalcAutoWidth( *(SwSectionFrm*)pFrm );
        }
        if ( pFrm->IsTxtFrm() )
        {
            nMin = ((SwTxtFrm*)pFrm)->CalcFitToContent();
            const SvxLRSpaceItem &rSpace =
                ((SwTxtFrm*)pFrm)->GetTxtNode()->GetSwAttrSet().GetLRSpace();
            if (!((SwTxtFrm*)pFrm)->IsLocked())
                nMin += rSpace.GetRight() + rSpace.GetTxtLeft() + rSpace.GetTxtFirstLineOfst();
        }
        else if ( pFrm->IsTabFrm() )
        {
            const SwFmtFrmSize& rTblFmtSz = ((SwTabFrm*)pFrm)->GetTable()->GetFrmFmt()->GetFrmSize();
            if ( USHRT_MAX == rTblFmtSz.GetSize().Width() ||
                 text::HoriOrientation::NONE == ((SwTabFrm*)pFrm)->GetFmt()->GetHoriOrient().GetHoriOrient() )
            {
                const SwPageFrm* pPage = rFrm.FindPageFrm();
                
                nMin = pFrm->GetUpper()->IsVertical() ?
                    pPage->Prt().Height() :
                    pPage->Prt().Width();
            }
            else
            {
                nMin = rTblFmtSz.GetSize().Width();
            }
        }

        if ( nMin > nRet )
            nRet = nMin;

        pFrm = pFrm->GetNext();
    }

    return nRet;
}

SwTwips SwFlyFrm::CalcAutoWidth() const
{
    return lcl_CalcAutoWidth( *this );
}



sal_Bool SwFlyFrm::GetContour( PolyPolygon&   rContour,
                           const sal_Bool _bForPaint ) const
{
    sal_Bool bRet = sal_False;
    if( GetFmt()->GetSurround().IsContour() && Lower() &&
        Lower()->IsNoTxtFrm() )
    {
        SwNoTxtNode *pNd = (SwNoTxtNode*)((SwCntntFrm*)Lower())->GetNode();
        
        
        
        const GraphicObject* pGrfObj = NULL;
        bool bGrfObjCreated = false;
        const SwGrfNode* pGrfNd = pNd->GetGrfNode();
        if ( pGrfNd && _bForPaint )
        {
            pGrfObj = &(pGrfNd->GetGrfObj());
        }
        else
        {
            pGrfObj = new GraphicObject( pNd->GetGraphic() );
            bGrfObjCreated = true;
        }
        OSL_ENSURE( pGrfObj, "SwFlyFrm::GetContour() - No Graphic/GraphicObject found at <SwNoTxtNode>." );
        if ( pGrfObj && pGrfObj->GetType() != GRAPHIC_NONE )
        {
            if( !pNd->HasContour() )
            {
                
                
                if ( pGrfNd && _bForPaint )
                {
                    OSL_FAIL( "SwFlyFrm::GetContour() - No Contour found at <SwNoTxtNode> during paint." );
                    return bRet;
                }
                pNd->CreateContour();
            }
            pNd->GetContour( rContour );
            
            
            SwRect aClip;
            SwRect aOrig;
            Lower()->Calc();
            ((SwNoTxtFrm*)Lower())->GetGrfArea( aClip, &aOrig, false );
            
            
            
            {
                OutputDevice*   pOutDev = Application::GetDefaultDevice();
                const MapMode   aDispMap( MAP_TWIP );
                const MapMode   aGrfMap( pGrfObj->GetPrefMapMode() );
                const Size      aGrfSize( pGrfObj->GetPrefSize() );
                Size            aOrgSize;
                Point           aNewPoint;
                bool            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

                if ( bPixelMap )
                    aOrgSize = pOutDev->PixelToLogic( aGrfSize, aDispMap );
                else
                    aOrgSize = pOutDev->LogicToLogic( aGrfSize, aGrfMap, aDispMap );

                if ( aOrgSize.Width() && aOrgSize.Height() )
                {
                    double fScaleX = (double) aOrig.Width() / aOrgSize.Width();
                    double fScaleY = (double) aOrig.Height() / aOrgSize.Height();

                    for ( sal_uInt16 j = 0, nPolyCount = rContour.Count(); j < nPolyCount; j++ )
                    {
                        Polygon& rPoly = rContour[ j ];

                        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
                        {
                            if ( bPixelMap )
                                aNewPoint = pOutDev->PixelToLogic( rPoly[ i ], aDispMap  );
                            else
                                aNewPoint = pOutDev->LogicToLogic( rPoly[ i ], aGrfMap, aDispMap  );

                            rPoly[ i ] = Point( FRound( aNewPoint.getX() * fScaleX ), FRound( aNewPoint.getY() * fScaleY ) );
                        }
                    }
                }
            }
            
            if ( bGrfObjCreated )
            {
                delete pGrfObj;
            }
            rContour.Move( aOrig.Left(), aOrig.Top() );
            if( !aClip.Width() )
                aClip.Width( 1 );
            if( !aClip.Height() )
                aClip.Height( 1 );
            rContour.Clip( aClip.SVRect() );
            rContour.Optimize(POLY_OPTIMIZE_CLOSE);
            bRet = sal_True;
        }
    }
    return bRet;
}


const SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj() const
{
    return static_cast<const SwVirtFlyDrawObj*>(GetDrawObj());
}
SwVirtFlyDrawObj* SwFlyFrm::GetVirtDrawObj()
{
    return static_cast<SwVirtFlyDrawObj*>(DrawObj());
}




void SwFlyFrm::InvalidateObjPos()
{
    InvalidatePos();
    
    InvalidateObjRectWithSpaces();
}

SwFrmFmt& SwFlyFrm::GetFrmFmt()
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}
const SwFrmFmt& SwFlyFrm::GetFrmFmt() const
{
    OSL_ENSURE( GetFmt(),
            "<SwFlyFrm::GetFrmFmt()> - missing frame format -> crash." );
    return *GetFmt();
}

const SwRect SwFlyFrm::GetObjRect() const
{
    return Frm();
}



const SwRect SwFlyFrm::GetObjBoundRect() const
{
    return GetObjRect();
}


bool SwFlyFrm::_SetObjTop( const SwTwips _nTop )
{
    const bool bChanged( Frm().Pos().getY() != _nTop );

    Frm().Pos().setY(_nTop);

    return bChanged;
}
bool SwFlyFrm::_SetObjLeft( const SwTwips _nLeft )
{
    const bool bChanged( Frm().Pos().getX() != _nLeft );

    Frm().Pos().setX(_nLeft);

    return bChanged;
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#
*/
void SwFlyFrm::RegisterAtCorrectPage()
{
    
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    OD 2004-05-11 #i28701#
*/
bool SwFlyFrm::IsFormatPossible() const
{
    return SwAnchoredObject::IsFormatPossible() &&
           !IsLocked() && !IsColLocked();
}

void SwFlyFrm::GetAnchoredObjects( std::list<SwAnchoredObject*>& aList, const SwFmt& rFmt )
{
    SwIterator<SwFlyFrm,SwFmt> aIter( rFmt );
    for( SwFlyFrm* pFlyFrm = aIter.First(); pFlyFrm; pFlyFrm = aIter.Next() )
        aList.push_back( pFlyFrm );
}

const SwFlyFrmFmt * SwFlyFrm::GetFmt() const
{
    return static_cast< const SwFlyFrmFmt * >( GetDep() );
}

SwFlyFrmFmt * SwFlyFrm::GetFmt()
{
    return static_cast< SwFlyFrmFmt * >( GetDep() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
