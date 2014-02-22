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

#include <objectformattertxtfrm.hxx>
#include <anchoredobject.hxx>
#include <sortedobjs.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <layouter.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fmtfollowtextflow.hxx>
#include <layact.hxx>

using namespace ::com::sun::star;


class SwForbidFollowFormat
{
private:
    SwTxtFrm& mrTxtFrm;
    const bool bOldFollowFormatAllowed;

public:
    SwForbidFollowFormat( SwTxtFrm& _rTxtFrm )
        : mrTxtFrm( _rTxtFrm ),
          bOldFollowFormatAllowed( _rTxtFrm.FollowFormatAllowed() )
    {
        mrTxtFrm.ForbidFollowFormat();
    }

    ~SwForbidFollowFormat()
    {
        if ( bOldFollowFormatAllowed )
        {
            mrTxtFrm.AllowFollowFormat();
        }
    }
};

SwObjectFormatterTxtFrm::SwObjectFormatterTxtFrm( SwTxtFrm& _rAnchorTxtFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwTxtFrm* _pMasterAnchorTxtFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction, true ),
      mrAnchorTxtFrm( _rAnchorTxtFrm ),
      mpMasterAnchorTxtFrm( _pMasterAnchorTxtFrm )
{
}

SwObjectFormatterTxtFrm::~SwObjectFormatterTxtFrm()
{
}

SwObjectFormatterTxtFrm* SwObjectFormatterTxtFrm::CreateObjFormatter(
                                                SwTxtFrm& _rAnchorTxtFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    SwObjectFormatterTxtFrm* pObjFormatter = 0L;

    
    SwTxtFrm* pMasterOfAnchorFrm = 0L;
    if ( _rAnchorTxtFrm.IsFollow() )
    {
        pMasterOfAnchorFrm = _rAnchorTxtFrm.FindMaster();
        while ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->IsFollow() )
        {
            pMasterOfAnchorFrm = pMasterOfAnchorFrm->FindMaster();
        }
    }

    
    
    if ( _rAnchorTxtFrm.GetDrawObjs() ||
         ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->GetDrawObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterTxtFrm( _rAnchorTxtFrm, _rPageFrm,
                                         pMasterOfAnchorFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterTxtFrm::GetAnchorFrm()
{
    return mrAnchorTxtFrm;
}


bool SwObjectFormatterTxtFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool _bCheckForMovedFwd )
{
    
    
    if ( FormatOnlyAsCharAnchored() &&
         !(_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
    {
        return true;
    }

    
    
    if ( GetLayAction() && GetLayAction()->IsAgain() )
    {
        return false;
    }

    bool bSuccess( true );

    if ( _rAnchoredObj.IsFormatPossible() )
    {
        _rAnchoredObj.SetRestartLayoutProcess( false );

        _FormatObj( _rAnchoredObj );
        
        
        if ( GetLayAction() && GetLayAction()->IsAgain() )
        {
            return false;
        }

        
        

        
        
        
        const bool bRestart =
                _rAnchoredObj.RestartLayoutProcess() &&
                !( _rAnchoredObj.PositionLocked() &&
                   _rAnchoredObj.GetAnchorFrm()->IsInFly() &&
                   _rAnchoredObj.GetFrmFmt().GetFollowTextFlow().GetValue() );
        if ( bRestart )
        {
            bSuccess = false;
            _InvalidatePrevObjs( _rAnchoredObj );
            _InvalidateFollowObjs( _rAnchoredObj, true );
        }

        
        
        
        
        
        
        
        
        
        if ( bSuccess &&
             _rAnchoredObj.ConsiderObjWrapInfluenceOnObjPos() &&
             ( _bCheckForMovedFwd ||
               _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                    
                    GetWrapInfluenceOnObjPos( true ) ==
                        
                        text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) )
        {
            
            
            
            const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

            
            
            _FormatAnchorFrmForCheckMoveFwd();

            
            if ( _rAnchoredObj.HasClearedEnvironment() )
            {
                _rAnchoredObj.SetClearedEnvironment( true );
                
                
                SwPageFrm* pAnchorPageFrm( mrAnchorTxtFrm.FindPageFrm() );
                if ( pAnchorPageFrm != _rAnchoredObj.GetPageFrm() )
                {
                    bool bInsert( true );
                    sal_uInt32 nToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTxtFrm, nToPageNum ) )
                    {
                        if ( nToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                       pAnchorPageFrm->GetPhyPageNum() );
                        mrAnchorTxtFrm.InvalidatePos();
                        bSuccess = false;
                        _InvalidatePrevObjs( _rAnchoredObj );
                        _InvalidateFollowObjs( _rAnchoredObj, true );
                    }
                    else
                    {
                        OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            else if ( !mrAnchorTxtFrm.IsFollow() && bDoesAnchorHadPrev )
            {
                
                
                sal_uInt32 nIdx( CountOfCollected() );
                OSL_ENSURE( nIdx > 0,
                        "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchored object not collected!?" );
                --nIdx;

                sal_uInt32 nToPageNum( 0L );
                
                bool bDummy( false );
                
                if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition( *GetCollectedObj( nIdx ),
                                              GetPgNumOfCollected( nIdx ),
                                              IsCollectedAnchoredAtMaster( nIdx ),
                                              nToPageNum, bDummy ) )
                {
                    
                    
                    bool bInsert( true );
                    sal_uInt32 nMovedFwdToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTxtFrm, nMovedFwdToPageNum ) )
                    {
                        if ( nMovedFwdToPageNum < nToPageNum )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        
                        
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                       nToPageNum );
                        mrAnchorTxtFrm.InvalidatePos();

                        
                        bSuccess = false;

                        
                        
                        _InvalidatePrevObjs( _rAnchoredObj );

                        
                        
                        _InvalidateFollowObjs( _rAnchoredObj, true );
                    }
                    else
                    {
                        OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            
            
            else if ( !mrAnchorTxtFrm.IsFollow() &&
                      mrAnchorTxtFrm.GetFollow() &&
                      mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
            {
                SwLayouter::RemoveMovedFwdFrm(
                                *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                                mrAnchorTxtFrm );
            }
        }
    }

    return bSuccess;
}

bool SwObjectFormatterTxtFrm::DoFormatObjs()
{
    if ( !mrAnchorTxtFrm.IsValid() )
    {
        if ( GetLayAction() &&
             mrAnchorTxtFrm.FindPageFrm() != &GetPageFrm() )
        {
            
            
            GetLayAction()->SetAgain();
        }
        else
        {
            
            OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs()> called for invalidate anchor text frame." );
        }

        return false;
    }

    bool bSuccess( true );

    if ( mrAnchorTxtFrm.IsFollow() )
    {
        
        
        
        
        
        OSL_ENSURE( mpMasterAnchorTxtFrm,
                "SwObjectFormatterTxtFrm::DoFormatObjs() - missing 'master' anchor text frame" );
        bSuccess = _FormatObjsAtFrm( mpMasterAnchorTxtFrm );

        if ( bSuccess )
        {
            
            
            bSuccess = _FormatObjsAtFrm();
        }
    }
    else
    {
        bSuccess = _FormatObjsAtFrm();
    }

    
    
    if ( bSuccess &&
         ( ConsiderWrapOnObjPos() ||
           ( !mrAnchorTxtFrm.IsFollow() &&
             _AtLeastOneObjIsTmpConsiderWrapInfluence() ) ) )
    {
        const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

        
        
        
        
        
        
        
        _FormatAnchorFrmForCheckMoveFwd();

        sal_uInt32 nToPageNum( 0L );
        
        bool bInFollow( false );
        SwAnchoredObject* pObj = 0L;
        if ( !mrAnchorTxtFrm.IsFollow() )
        {
            pObj = _GetFirstObjWithMovedFwdAnchor(
                    
                    text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
                    nToPageNum, bInFollow );
        }
        
        if ( pObj && pObj->HasClearedEnvironment() )
        {
            pObj->SetClearedEnvironment( true );
            
            
            SwPageFrm* pAnchorPageFrm( mrAnchorTxtFrm.FindPageFrm() );
            
            
            if ( pAnchorPageFrm != pObj->GetPageFrm() ||
                 bInFollow )
            {
                bool bInsert( true );
                sal_uInt32 nTmpToPageNum( 0L );
                const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                if ( SwLayouter::FrmMovedFwdByObjPos(
                                        rDoc, mrAnchorTxtFrm, nTmpToPageNum ) )
                {
                    if ( nTmpToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                        SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                    else
                        bInsert = false;
                }
                if ( bInsert )
                {
                    SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                   pAnchorPageFrm->GetPhyPageNum() );
                    mrAnchorTxtFrm.InvalidatePos();
                    bSuccess = false;
                    _InvalidatePrevObjs( *pObj );
                    _InvalidateFollowObjs( *pObj, true );
                }
                else
                {
                    OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
                }
            }
        }
        else if ( pObj && bDoesAnchorHadPrev )
        {
            

            
            
            bool bInsert( true );
            sal_uInt32 nMovedFwdToPageNum( 0L );
            const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
            if ( SwLayouter::FrmMovedFwdByObjPos(
                                    rDoc, mrAnchorTxtFrm, nMovedFwdToPageNum ) )
            {
                if ( nMovedFwdToPageNum < nToPageNum )
                    SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                else
                    bInsert = false;
            }
            if ( bInsert )
            {
                
                
                SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm, nToPageNum );
                mrAnchorTxtFrm.InvalidatePos();

                
                bSuccess = false;

                
                
                _InvalidatePrevObjs( *pObj );

                
                
                _InvalidateFollowObjs( *pObj, true );
            }
            else
            {
                OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
            }
        }
        
        
        else if ( !mrAnchorTxtFrm.IsFollow() &&
                  mrAnchorTxtFrm.GetFollow() &&
                  mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
        {
            SwLayouter::RemoveMovedFwdFrm(
                            *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                            mrAnchorTxtFrm );
        }
    }

    return bSuccess;
}

void SwObjectFormatterTxtFrm::_InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj )
{
    
    
    
    if ( _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                
                GetWrapInfluenceOnObjPos( true ) ==
                            
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
    {
        const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
        if ( pObjs )
        {
            
            sal_Int32 i = pObjs->ListPosOf( _rAnchoredObj ) - 1;
            for ( ; i >= 0; --i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                        
                        GetWrapInfluenceOnObjPos( true ) ==
                            
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
                {
                    pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                }
            }
        }
    }
}

void SwObjectFormatterTxtFrm::_InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
                                                     const bool _bInclObj )
{
    if ( _bInclObj )
    {
        _rAnchoredObj.InvalidateObjPosForConsiderWrapInfluence( true );
    }

    const SwSortedObjs* pObjs = GetPageFrm().GetSortedObjs();
    if ( pObjs )
    {
        
        sal_uInt32 i = pObjs->ListPosOf( _rAnchoredObj ) + 1;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
        }
    }
}

SwAnchoredObject* SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum,
                                    bool& _boInFollow )
{
    
    OSL_ENSURE( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
            _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
            "<SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(..)> - invalid value for parameter <_nWrapInfluenceOnPosition>" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    sal_uInt32 i = 0L;
    for ( ; i < CountOfCollected(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = GetCollectedObj(i);
        if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() &&
             pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                    
                    GetWrapInfluenceOnObjPos( true ) == _nWrapInfluenceOnPosition )
        {
            
            
            
            if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition( *GetCollectedObj( i ),
                                          GetPgNumOfCollected( i ),
                                          IsCollectedAnchoredAtMaster( i ),
                                          _noToPageNum, _boInFollow ) )
            {
                pRetAnchoredObj = pAnchoredObj;
                break;
            }
        }
    }

    return pRetAnchoredObj;
}



bool SwObjectFormatterTxtFrm::CheckMovedFwdCondition(
                                            SwAnchoredObject& _rAnchoredObj,
                                            const sal_uInt32 _nFromPageNum,
                                            const bool _bAnchoredAtMasterBeforeFormatAnchor,
                                            sal_uInt32& _noToPageNum,
                                            bool& _boInFollow )
{
    bool bAnchorIsMovedForward( false );

    SwPageFrm* pPageFrmOfAnchor = _rAnchoredObj.FindPageFrmOfAnchor();
    if ( pPageFrmOfAnchor )
    {
        const sal_uInt32 nPageNum = pPageFrmOfAnchor->GetPhyPageNum();
        if ( nPageNum > _nFromPageNum )
        {
            _noToPageNum = nPageNum;
            
            
            
            
            
            
            if ( _noToPageNum > (_nFromPageNum + 1) )
            {
                SwFrm* pAnchorFrm = _rAnchoredObj.GetAnchorFrmContainingAnchPos();
                if ( pAnchorFrm->IsInTab() &&
                     pAnchorFrm->IsInFollowFlowRow() )
                {
                    _noToPageNum = _nFromPageNum + 1;
                }
            }
            bAnchorIsMovedForward = true;
        }
    }
    
    
    
    
    
    if ( !bAnchorIsMovedForward &&
         _bAnchoredAtMasterBeforeFormatAnchor &&
        ((_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_CHAR) ||
         (_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_PARA)))
    {
        SwFrm* pAnchorFrm = _rAnchoredObj.GetAnchorFrmContainingAnchPos();
        OSL_ENSURE( pAnchorFrm->IsTxtFrm(),
                "<SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..) - wrong type of anchor frame>" );
        SwTxtFrm* pAnchorTxtFrm = static_cast<SwTxtFrm*>(pAnchorFrm);
        bool bCheck( false );
        if ( pAnchorTxtFrm->IsFollow() )
        {
            bCheck = true;
        }
        else if( pAnchorTxtFrm->IsInTab() )
        {
            const SwRowFrm* pMasterRow = pAnchorTxtFrm->IsInFollowFlowRow();
            if ( pMasterRow &&
                 pMasterRow->FindPageFrm() == pPageFrmOfAnchor )
            {
                bCheck = true;
            }
        }
        if ( bCheck )
        {
            
            
            SwFrm* pColFrm = pAnchorTxtFrm->FindColFrm();
            while ( pColFrm && !pColFrm->GetNext() )
            {
                pColFrm = pColFrm->FindColFrm();
            }
            if ( !pColFrm || !pColFrm->GetNext() )
            {
                _noToPageNum = _nFromPageNum + 1;
                bAnchorIsMovedForward = true;
                
                _boInFollow = true;
            }
        }
    }

    return bAnchorIsMovedForward;
}




static void lcl_FormatCntntOfLayoutFrm( SwLayoutFrm* pLayFrm,
                                 SwFrm* pLastLowerFrm = 0L )
{
    SwFrm* pLowerFrm = pLayFrm->GetLower();
    while ( pLowerFrm )
    {
        
        if ( pLastLowerFrm && pLowerFrm == pLastLowerFrm )
        {
            break;
        }
        if ( pLowerFrm->IsLayoutFrm() )
            lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pLowerFrm),
                                        pLastLowerFrm );
        else
            pLowerFrm->Calc();

        pLowerFrm = pLowerFrm->GetNext();
    }
}

/** method to format given anchor text frame and its previous frames

    #i56300#
    Usage: Needed to check, if the anchor text frame is moved forward
    due to the positioning and wrapping of its anchored objects, and
    to format the frames, which have become invalid due to the anchored
    object formatting in the iterative object positioning algorithm
*/
void SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( SwTxtFrm& _rAnchorTxtFrm )
{
    
    
    if ( !_rAnchorTxtFrm.IsFollow() )
    {
        
        
        
        if ( _rAnchorTxtFrm.IsInSct() )
        {
            SwFrm* pSectFrm = _rAnchorTxtFrm.GetUpper();
            while ( pSectFrm )
            {
                if ( pSectFrm->IsSctFrm() || pSectFrm->IsCellFrm() )
                {
                    break;
                }
                pSectFrm = pSectFrm->GetUpper();
            }
            if ( pSectFrm && pSectFrm->IsSctFrm() )
            {
                
                _rAnchorTxtFrm.LockJoin();
                SwFrm* pFrm = pSectFrm->GetUpper()->GetLower();
                
                
                
                while ( pFrm && pFrm != pSectFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc();

                    pFrm = pFrm->GetNext();
                }
                lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pSectFrm),
                                            &_rAnchorTxtFrm );
                
                _rAnchorTxtFrm.UnlockJoin();
            }
        }

        
        
        
        SwFrm* pColFrmOfAnchor = _rAnchorTxtFrm.FindColFrm();
        if ( pColFrmOfAnchor )
        {
            
            _rAnchorTxtFrm.LockJoin();
            SwFrm* pColFrm = pColFrmOfAnchor->GetUpper()->GetLower();
            while ( pColFrm != pColFrmOfAnchor )
            {
                SwFrm* pFrm = pColFrm->GetLower();
                while ( pFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc();

                    pFrm = pFrm->GetNext();
                }

                pColFrm = pColFrm->GetNext();
            }
            
            _rAnchorTxtFrm.UnlockJoin();
        }
    }

    
    
    
    if ( _rAnchorTxtFrm.IsInTab() )
    {
        SwForbidFollowFormat aForbidFollowFormat( _rAnchorTxtFrm );
        _rAnchorTxtFrm.Calc();
    }
    else
    {
        _rAnchorTxtFrm.Calc();
    }
}

/** method to format the anchor frame for checking of the move forward condition

    #i40141#
*/
void SwObjectFormatterTxtFrm::_FormatAnchorFrmForCheckMoveFwd()
{
    SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( mrAnchorTxtFrm );
}

/** method to determine if at least one anchored object has state
    <temporarly consider wrapping style influence> set.
*/
bool SwObjectFormatterTxtFrm::_AtLeastOneObjIsTmpConsiderWrapInfluence()
{
    bool bRet( false );

    const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
    if ( pObjs && pObjs->Count() > 1 )
    {
        sal_uInt32 i = 0;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
            {
                bRet = true;
                break;
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
