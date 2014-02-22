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

#include <objectformatter.hxx>
#include <objectformattertxtfrm.hxx>
#include <objectformatterlayfrm.hxx>
#include <anchoredobject.hxx>
#include <anchoreddrawobject.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <layact.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>

#include <vector>



class SwPageNumAndTypeOfAnchors
{
    private:
        struct tEntry
        {
            SwAnchoredObject* mpAnchoredObj;
            sal_uInt32 mnPageNumOfAnchor;
            bool mbAnchoredAtMaster;
        };

        std::vector< tEntry* > maObjList;

    public:
        inline SwPageNumAndTypeOfAnchors()
        {
        }
        inline ~SwPageNumAndTypeOfAnchors()
        {
            for ( std::vector< tEntry* >::iterator aIter = maObjList.begin();
                  aIter != maObjList.end(); ++aIter )
            {
                delete (*aIter);
            }
            maObjList.clear();
        }

        inline void Collect( SwAnchoredObject& _rAnchoredObj )
        {
            tEntry* pNewEntry = new tEntry();
            pNewEntry->mpAnchoredObj = &_rAnchoredObj;
            
            
            
            SwPageFrm* pPageFrmOfAnchor = _rAnchoredObj.FindPageFrmOfAnchor();
            if ( pPageFrmOfAnchor )
            {
                pNewEntry->mnPageNumOfAnchor = pPageFrmOfAnchor->GetPhyPageNum();
            }
            else
            {
                pNewEntry->mnPageNumOfAnchor = 0;
            }
            
            SwTxtFrm* pAnchorCharFrm = _rAnchoredObj.FindAnchorCharFrm();
            if ( pAnchorCharFrm )
            {
                pNewEntry->mbAnchoredAtMaster = !pAnchorCharFrm->IsFollow();
            }
            else
            {
                pNewEntry->mbAnchoredAtMaster = true;
            }
            maObjList.push_back( pNewEntry );
        }

        inline SwAnchoredObject* operator[]( sal_uInt32 _nIndex )
        {
            SwAnchoredObject* bRetObj = 0L;

            if ( _nIndex < Count())
            {
                bRetObj = maObjList[_nIndex]->mpAnchoredObj;
            }

            return bRetObj;
        }

        inline sal_uInt32 GetPageNum( sal_uInt32 _nIndex ) const
        {
            sal_uInt32 nRetPgNum = 0L;

            if ( _nIndex < Count())
            {
                nRetPgNum = maObjList[_nIndex]->mnPageNumOfAnchor;
            }

            return nRetPgNum;
        }

        
        inline bool AnchoredAtMaster( sal_uInt32 _nIndex )
        {
            bool bAnchoredAtMaster( true );

            if ( _nIndex < Count())
            {
                bAnchoredAtMaster = maObjList[_nIndex]->mbAnchoredAtMaster;
            }

            return bAnchoredAtMaster;
        }

        inline sal_uInt32 Count() const
        {
            return maObjList.size();
        }
};

SwObjectFormatter::SwObjectFormatter( const SwPageFrm& _rPageFrm,
                                      SwLayAction* _pLayAction,
                                      const bool _bCollectPgNumOfAnchors )
    : mrPageFrm( _rPageFrm ),
      mbFormatOnlyAsCharAnchored( false ),
      mbConsiderWrapOnObjPos( _rPageFrm.GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) ),
      mpLayAction( _pLayAction ),
      
      mpPgNumAndTypeOfAnchors( _bCollectPgNumOfAnchors ? new SwPageNumAndTypeOfAnchors() : 0L )
{
}

SwObjectFormatter::~SwObjectFormatter()
{
    delete mpPgNumAndTypeOfAnchors;
}

SwObjectFormatter* SwObjectFormatter::CreateObjFormatter(
                                                      SwFrm& _rAnchorFrm,
                                                      const SwPageFrm& _rPageFrm,
                                                      SwLayAction* _pLayAction )
{
    SwObjectFormatter* pObjFormatter = 0L;
    if ( _rAnchorFrm.IsTxtFrm() )
    {
        pObjFormatter = SwObjectFormatterTxtFrm::CreateObjFormatter(
                                            static_cast<SwTxtFrm&>(_rAnchorFrm),
                                            _rPageFrm, _pLayAction );
    }
    else if ( _rAnchorFrm.IsLayoutFrm() )
    {
        pObjFormatter = SwObjectFormatterLayFrm::CreateObjFormatter(
                                        static_cast<SwLayoutFrm&>(_rAnchorFrm),
                                        _rPageFrm, _pLayAction );
    }
    else
    {
        OSL_FAIL( "<SwObjectFormatter::CreateObjFormatter(..)> - unexcepted type of anchor frame" );
    }

    return pObjFormatter;
}

/** method to format all floating screen objects at the given anchor frame
*/
bool SwObjectFormatter::FormatObjsAtFrm( SwFrm& _rAnchorFrm,
                                         const SwPageFrm& _rPageFrm,
                                         SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( _rAnchorFrm, _rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        
        bSuccess = pObjFormatter->DoFormatObjs();
    }
    delete pObjFormatter;

    return bSuccess;
}

/** method to format a given floating screen object
*/
bool SwObjectFormatter::FormatObj( SwAnchoredObject& _rAnchoredObj,
                                   SwFrm* _pAnchorFrm,
                                   const SwPageFrm* _pPageFrm,
                                   SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    OSL_ENSURE( _pAnchorFrm || _rAnchoredObj.GetAnchorFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing anchor frame" );
    SwFrm& rAnchorFrm = _pAnchorFrm ? *(_pAnchorFrm) : *(_rAnchoredObj.AnchorFrm());

    OSL_ENSURE( _pPageFrm || rAnchorFrm.FindPageFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing page frame" );
    const SwPageFrm& rPageFrm = _pPageFrm ? *(_pPageFrm) : *(rAnchorFrm.FindPageFrm());

    
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( rAnchorFrm, rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        
        
        bSuccess = pObjFormatter->DoFormatObj( _rAnchoredObj, true );
    }
    delete pObjFormatter;

    return bSuccess;
}

/** helper method for method <_FormatObj(..)> - performs the intrinsic format
    of the layout of the given layout frame and all its lower layout frames.

    #i28701#
    IMPORTANT NOTE:
    Method corresponds to methods <SwLayAction::FormatLayoutFly(..)> and
    <SwLayAction::FormatLayout(..)>. Thus, its code for the formatting have
    to be synchronised.
*/
void SwObjectFormatter::_FormatLayout( SwLayoutFrm& _rLayoutFrm )
{
    _rLayoutFrm.Calc();

    SwFrm* pLowerFrm = _rLayoutFrm.Lower();
    while ( pLowerFrm )
    {
        if ( pLowerFrm->IsLayoutFrm() )
        {
            _FormatLayout( *(static_cast<SwLayoutFrm*>(pLowerFrm)) );
        }
        pLowerFrm = pLowerFrm->GetNext();
    }
}

/** helper method for method <_FormatObj(..)> - performs the intrinsic
    format of the content of the given floating screen object.

    #i28701#
*/
void SwObjectFormatter::_FormatObjCntnt( SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rAnchoredObj.ISA(SwFlyFrm) )
    {
        
        return;
    }

    SwFlyFrm& rFlyFrm = static_cast<SwFlyFrm&>(_rAnchoredObj);
    SwCntntFrm* pCntnt = rFlyFrm.ContainsCntnt();

    while ( pCntnt )
    {
        
        pCntnt->OptCalc();

        
        
        
        if ( pCntnt->IsTxtFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm( *pCntnt,
                                                  *(pCntnt->FindPageFrm()),
                                                  GetLayAction() ) )
        {
            
            pCntnt = rFlyFrm.ContainsCntnt();
            continue;
        }

        
        pCntnt = pCntnt->GetNextCntntFrm();
    }
}

/** performs the intrinsic format of a given floating screen object and its content.

    #i28701#
*/
void SwObjectFormatter::_FormatObj( SwAnchoredObject& _rAnchoredObj )
{
    
    
    if ( FormatOnlyAsCharAnchored() &&
         !(_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
    {
        return;
    }

    
    if ( mpPgNumAndTypeOfAnchors )
    {
        mpPgNumAndTypeOfAnchors->Collect( _rAnchoredObj );
    }

    if ( _rAnchoredObj.ISA(SwFlyFrm) )
    {
        SwFlyFrm& rFlyFrm = static_cast<SwFlyFrm&>(_rAnchoredObj);
        
        if ( rFlyFrm.IsFlyLayFrm() )
        {
            static_cast<SwFlyLayFrm&>(rFlyFrm).SetNoMakePos( false );
        }

        
        sal_uInt16 nLoopControlRuns = 0;
        const sal_uInt16 nLoopControlMax = 15;

        do {
            if ( mpLayAction )
            {
                mpLayAction->FormatLayoutFly( &rFlyFrm );
                
                
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
            }
            else
            {
                _FormatLayout( rFlyFrm );
            }
            
            
            if ( rFlyFrm.IsFlyLayFrm() && rFlyFrm.IsClipped() )
            {
                static_cast<SwFlyLayFrm&>(rFlyFrm).SetNoMakePos( true );
            }
            
            
            SwObjectFormatter::FormatObjsAtFrm( rFlyFrm,
                                                *(rFlyFrm.FindPageFrm()),
                                                mpLayAction );
            if ( mpLayAction )
            {
                mpLayAction->_FormatFlyCntnt( &rFlyFrm );
                
                
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
            }
            else
            {
                _FormatObjCntnt( rFlyFrm );
            }

            if ( ++nLoopControlRuns >= nLoopControlMax )
            {
                OSL_FAIL( "LoopControl in SwObjectFormatter::_FormatObj: Stage 3!!!" );
                rFlyFrm.ValidateThisAndAllLowers( 2 );
                nLoopControlRuns = 0;
            }

        
        
        } while ( !rFlyFrm.IsValid() &&
                  !_rAnchoredObj.RestartLayoutProcess() &&
                  rFlyFrm.GetAnchorFrm() == &GetAnchorFrm() );
    }
    else if ( _rAnchoredObj.ISA(SwAnchoredDrawObject) )
    {
        _rAnchoredObj.MakeObjPos();
    }
}

/** invokes the intrinsic format method for all floating screen objects,
    anchored at anchor frame on the given page frame

    #i28701#
    #i26945# - for format of floating screen objects for
    follow text frames, the 'master' text frame is passed to the method.
    Thus, the objects, whose anchor character is inside the follow text
    frame can be formatted.
*/
bool SwObjectFormatter::_FormatObjsAtFrm( SwTxtFrm* _pMasterTxtFrm )
{
    
    SwFrm* pAnchorFrm( 0L );
    if ( GetAnchorFrm().IsTxtFrm() &&
         static_cast<SwTxtFrm&>(GetAnchorFrm()).IsFollow() &&
         _pMasterTxtFrm )
    {
        pAnchorFrm = _pMasterTxtFrm;
    }
    else
    {
        pAnchorFrm = &GetAnchorFrm();
    }
    if ( !pAnchorFrm->GetDrawObjs() )
    {
        
        return true;
    }

    bool bSuccess( true );

    sal_uInt32 i = 0;
    for ( ; i < pAnchorFrm->GetDrawObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pAnchorFrm->GetDrawObjs())[i];

        
        
        
        
        
        
        
        
        
        SwTxtFrm* pAnchorCharFrm = pAnchoredObj->FindAnchorCharFrm();
        const bool bAnchoredAtFollowInSameBodyAsMaster =
                pAnchorCharFrm && pAnchorCharFrm->IsFollow() &&
                pAnchorCharFrm != pAnchoredObj->GetAnchorFrm() &&
                pAnchorCharFrm->FindBodyFrm() ==
                    static_cast<SwTxtFrm*>(pAnchoredObj->AnchorFrm())->FindBodyFrm();
        if ( bAnchoredAtFollowInSameBodyAsMaster )
        {
            continue;
        }
        
        
        
        SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
        OSL_ENSURE( pPageFrmOfAnchor,
                "<SwObjectFormatter::_FormatObjsAtFrm()> - missing page frame." );
        
        if ( pPageFrmOfAnchor && pPageFrmOfAnchor == &mrPageFrm )
        {
            
            
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            
            
            if ( !pAnchorFrm->GetDrawObjs() ||
                 i > pAnchorFrm->GetDrawObjs()->Count() )
            {
                break;
            }
            else
            {
                sal_uInt32 nActPosOfObj =
                    pAnchorFrm->GetDrawObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == pAnchorFrm->GetDrawObjs()->Count() ||
                     nActPosOfObj > i )
                {
                    --i;
                }
                else if ( nActPosOfObj < i )
                {
                    i = nActPosOfObj;
                }
            }
        }
    } 

    return bSuccess;
}

/** accessor to collected anchored object

    #i28701#
*/
SwAnchoredObject* SwObjectFormatter::GetCollectedObj( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? (*mpPgNumAndTypeOfAnchors)[_nIndex] : 0L;
}

/** accessor to 'anchor' page number of collected anchored object

    #i28701#
*/
sal_uInt32 SwObjectFormatter::GetPgNumOfCollected( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->GetPageNum(_nIndex) : 0L;
}

/** accessor to 'anchor' type of collected anchored object

    #i26945#
*/
bool SwObjectFormatter::IsCollectedAnchoredAtMaster( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors
           ? mpPgNumAndTypeOfAnchors->AnchoredAtMaster(_nIndex)
           : true;
}

/** accessor to total number of collected anchored objects

    #i28701#
*/
sal_uInt32 SwObjectFormatter::CountOfCollected()
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->Count() : 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
