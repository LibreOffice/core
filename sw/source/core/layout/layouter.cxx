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

#include "layouter.hxx"
#include "doc.hxx"
#include "sectfrm.hxx"
#include "pagefrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"

// #i28701#
#include <movedfwdfrmsbyobjpos.hxx>
// #i35911#
#include <objstmpconsiderwrapinfl.hxx>

#define LOOP_DETECT 250

class SwLooping
{
    sal_uInt16 nMinPage;
    sal_uInt16 nMaxPage;
    sal_uInt16 nCount;
    sal_uInt16 mnLoopControlStage;
public:
    SwLooping( SwPageFrm* pPage );
    void Control( SwPageFrm* pPage );
    void Drastic( SwFrm* pFrm );
    bool IsLoopingLouieLight() const { return nCount > LOOP_DETECT - 30; };
};

class SwEndnoter
{
    SwLayouter* pMaster;
    SwSectionFrm* pSect;
    SwFtnFrms*    pEndArr;
public:
    SwEndnoter( SwLayouter* pLay )
        : pMaster( pLay ), pSect( NULL ), pEndArr( NULL ) {}
    ~SwEndnoter() { delete pEndArr; }
    void CollectEndnotes( SwSectionFrm* pSct );
    void CollectEndnote( SwFtnFrm* pFtn );
    const SwSectionFrm* GetSect() const { return pSect; }
    void InsertEndnotes();
    bool HasEndnotes() const { return pEndArr && !pEndArr->empty(); }
};

void SwEndnoter::CollectEndnotes( SwSectionFrm* pSct )
{
    OSL_ENSURE( pSct, "CollectEndnotes: Which section?" );
    if( !pSect )
        pSect = pSct;
    else if( pSct != pSect )
        return;
    pSect->CollectEndnotes( pMaster );
}

void SwEndnoter::CollectEndnote( SwFtnFrm* pFtn )
{
    if( pEndArr && pEndArr->end() != std::find( pEndArr->begin(), pEndArr->end(), pFtn ) )
        return;

    if( pFtn->GetUpper() )
    {
        // pFtn is the master, he incorporates its follows
        SwFtnFrm *pNxt = pFtn->GetFollow();
        while ( pNxt )
        {
            SwFrm *pCnt = pNxt->ContainsAny();
            if ( pCnt )
            {
                do
                {   SwFrm *pNxtCnt = pCnt->GetNext();
                    pCnt->Cut();
                    pCnt->Paste( pFtn );
                    pCnt = pNxtCnt;
                } while ( pCnt );
            }
            else
            { OSL_ENSURE( pNxt->Lower() && pNxt->Lower()->IsSctFrm(),
                        "Endnote without content?" );
                pNxt->Cut();
                delete pNxt;
            }
            pNxt = pFtn->GetFollow();
        }
        if( pFtn->GetMaster() )
            return;
        pFtn->Cut();
    }
    else if( pEndArr )
    {
        for ( sal_uInt16 i = 0; i < pEndArr->size(); ++i )
        {
            SwFtnFrm *pEndFtn = (*pEndArr)[i];
            if( pEndFtn->GetAttr() == pFtn->GetAttr() )
            {
                delete pFtn;
                return;
            }
        }
    }
    if( !pEndArr )
        pEndArr = new SwFtnFrms;  // deleted from the SwLayouter
    pEndArr->push_back( pFtn );
}

void SwEndnoter::InsertEndnotes()
{
    if( !pSect )
        return;
    if( !pEndArr || pEndArr->empty() )
    {
        pSect = NULL;
        return;
    }
    OSL_ENSURE( pSect->Lower() && pSect->Lower()->IsFtnBossFrm(),
            "InsertEndnotes: Where's my column?" );
    SwFrm* pRef = pSect->FindLastCntnt( FINDMODE_MYLAST );
    SwFtnBossFrm *pBoss = pRef ? pRef->FindFtnBossFrm()
                               : (SwFtnBossFrm*)pSect->Lower();
    pBoss->_MoveFtns( *pEndArr );
    delete pEndArr;
    pEndArr = NULL;
    pSect = NULL;
}

SwLooping::SwLooping( SwPageFrm* pPage )
{
    OSL_ENSURE( pPage, "Where's my page?" );
    nMinPage = pPage->GetPhyPageNum();
    nMaxPage = nMinPage;
    nCount = 0;
    mnLoopControlStage = 0;
}

void SwLooping::Drastic( SwFrm* pFrm )
{
    while( pFrm )
    {
        pFrm->ValidateThisAndAllLowers( mnLoopControlStage );
        pFrm = pFrm->GetNext();
    }
}

void SwLooping::Control( SwPageFrm* pPage )
{
    if( !pPage )
        return;
    sal_uInt16 nNew = pPage->GetPhyPageNum();
    if( nNew > nMaxPage )
        nMaxPage = nNew;
    if( nNew < nMinPage )
    {
        nMinPage = nNew;
        nMaxPage = nNew;
        nCount = 0;
        mnLoopControlStage = 0;
    }
    else if( nNew > nMinPage + 2 )
    {
        nMinPage = nNew - 2;
        nMaxPage = nNew;
        nCount = 0;
        mnLoopControlStage = 0;
    }
    else if( ++nCount > LOOP_DETECT )
    {
#if OSL_DEBUG_LEVEL > 1
        static bool bNoLouie = false;
        if( bNoLouie )
            return;

        // FME 2007-08-30 #i81146# new loop control
        OSL_ENSURE( 0 != mnLoopControlStage, "Looping Louie: Stage 1!" );
        OSL_ENSURE( 1 != mnLoopControlStage, "Looping Louie: Stage 2!!" );
        OSL_ENSURE( 2 >  mnLoopControlStage, "Looping Louie: Stage 3!!!" );
#endif

        Drastic( pPage->Lower() );
        if( nNew > nMinPage && pPage->GetPrev() )
            Drastic( ((SwPageFrm*)pPage->GetPrev())->Lower() );
        if( nNew < nMaxPage && pPage->GetNext() )
            Drastic( ((SwPageFrm*)pPage->GetNext())->Lower() );

        ++mnLoopControlStage;
        nCount = 0;
    }
}

SwLayouter::SwLayouter()
        : pEndnoter( NULL ),
          pLooping( NULL ),
          // #i28701#
          mpMovedFwdFrms( 0L ),
          // #i35911#
          mpObjsTmpConsiderWrapInfl( 0L )
{
}

SwLayouter::~SwLayouter()
{
    delete pEndnoter;
    delete pLooping;
    // #i28701#
    delete mpMovedFwdFrms;
    mpMovedFwdFrms = 0L;
    // #i35911#
    delete mpObjsTmpConsiderWrapInfl;
    mpObjsTmpConsiderWrapInfl = 0L;
}

void SwLayouter::_CollectEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter )
        pEndnoter = new SwEndnoter( this );
    pEndnoter->CollectEndnotes( pSect );
}

bool SwLayouter::HasEndnotes() const
{
    return pEndnoter->HasEndnotes();
}

void SwLayouter::CollectEndnote( SwFtnFrm* pFtn )
{
    pEndnoter->CollectEndnote( pFtn );
}

void SwLayouter::InsertEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter || pEndnoter->GetSect() != pSect )
        return;
    pEndnoter->InsertEndnotes();
}

void SwLayouter::LoopControl( SwPageFrm* pPage, sal_uInt8 )
{
    OSL_ENSURE( pLooping, "Looping: Lost control" );
    pLooping->Control( pPage );
}

void SwLayouter::LoopingLouieLight( const SwDoc& rDoc, const SwTxtFrm& rFrm )
{
    if ( pLooping && pLooping->IsLoopingLouieLight() )
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_FAIL( "Looping Louie (Light): Fixating fractious frame" );
#endif
        SwLayouter::InsertMovedFwdFrm( rDoc, rFrm, rFrm.FindPageFrm()->GetPhyPageNum() );
    }
}

sal_Bool SwLayouter::StartLooping( SwPageFrm* pPage )
{
    if( pLooping )
        return sal_False;
    pLooping = new SwLooping( pPage );
    return sal_True;
}

void SwLayouter::EndLoopControl()
{
    delete pLooping;
    pLooping = NULL;
}

void SwLayouter::CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect )
{
    OSL_ENSURE( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    pDoc->GetLayouter()->_CollectEndnotes( pSect );
}

sal_Bool SwLayouter::Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFtnFrm* pFtn )
{
    if( !pDoc->GetLayouter() )
        return sal_False;
    SwLayouter *pLayouter = pDoc->GetLayouter();
    if( pLayouter->pEndnoter && pLayouter->pEndnoter->GetSect() && pSect &&
        ( pLayouter->pEndnoter->GetSect()->IsAnFollow( pSect ) ||
          pSect->IsAnFollow( pLayouter->pEndnoter->GetSect() ) ) )
    {
        if( pFtn )
            pLayouter->CollectEndnote( pFtn );
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwLayouter::StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage )
{
    OSL_ENSURE( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    return !pDoc->GetLayouter()->pLooping &&
            pDoc->GetLayouter()->StartLooping( pPage );
}

// #i28701#
// methods to manage text frames, which are moved forward by the positioning
// of its anchored objects
void SwLayouter::ClearMovedFwdFrms( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() &&
         _rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        _rDoc.GetLayouter()->mpMovedFwdFrms->Clear();
    }
}

void SwLayouter::InsertMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                    const sal_uInt32 _nToPageNum )
{
    if ( !_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->mpMovedFwdFrms =
                                                new SwMovedFwdFrmsByObjPos();
    }

    _rDoc.GetLayouter()->mpMovedFwdFrms->Insert( _rMovedFwdFrmByObjPos,
                                                 _nToPageNum );
}

// #i40155#
void SwLayouter::RemoveMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTxtFrm& _rTxtFrm )
{
    sal_uInt32 nDummy;
    if ( SwLayouter::FrmMovedFwdByObjPos( _rDoc, _rTxtFrm, nDummy ) )
    {
        _rDoc.GetLayouter()->mpMovedFwdFrms->Remove( _rTxtFrm );
    }
}

bool SwLayouter::FrmMovedFwdByObjPos( const SwDoc& _rDoc,
                                      const SwTxtFrm& _rTxtFrm,
                                      sal_uInt32& _ornToPageNum )
{
    if ( !_rDoc.GetLayouter() )
    {
        _ornToPageNum = 0;
        return false;
    }
    else if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        _ornToPageNum = 0;
        return false;
    }
    else
    {
        return _rDoc.GetLayouter()->mpMovedFwdFrms->
                                FrmMovedFwdByObjPos( _rTxtFrm, _ornToPageNum );
    }
}

// #i26945#
bool SwLayouter::DoesRowContainMovedFwdFrm( const SwDoc& _rDoc,
                                            const SwRowFrm& _rRowFrm )
{
    if ( !_rDoc.GetLayouter() )
    {
        return false;
    }
    else if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        return false;
    }
    else
    {
        return _rDoc.GetLayouter()->
                        mpMovedFwdFrms->DoesRowContainMovedFwdFrm( _rRowFrm );
    }
}

// #i35911#
void SwLayouter::ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() &&
         _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl->Clear();
    }
}
void SwLayouter::InsertObjForTmpConsiderWrapInfluence(
                                            const SwDoc& _rDoc,
                                            SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->mpObjsTmpConsiderWrapInfl =
                                new SwObjsMarkedAsTmpConsiderWrapInfluence();
    }

    _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl->Insert( _rAnchoredObj );
}

void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTxtFrm& rTxtFrm )
{
    if ( bCondition )
    {
        const SwDoc& rDoc = *rTxtFrm.GetAttrSet()->GetDoc();
        if ( rDoc.GetLayouter() )
        {
            const_cast<SwDoc&>(rDoc).GetLayouter()->LoopingLouieLight( rDoc, rTxtFrm );
        }
    }
}

// #i65250#
bool SwLayouter::MoveBwdSuppressed( const SwDoc& p_rDoc,
                                    const SwFlowFrm& p_rFlowFrm,
                                    const SwLayoutFrm& p_rNewUpperFrm )
{
    bool bMoveBwdSuppressed( false );

    if ( !p_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(p_rDoc).SetLayouter( new SwLayouter() );
    }

    // create hash map key
    tMoveBwdLayoutInfoKey aMoveBwdLayoutInfo;
    aMoveBwdLayoutInfo.mnFrmId = p_rFlowFrm.GetFrm()->GetFrmId();
    aMoveBwdLayoutInfo.mnNewUpperPosX = p_rNewUpperFrm.Frm().Pos().X();
    aMoveBwdLayoutInfo.mnNewUpperPosY = p_rNewUpperFrm.Frm().Pos().Y();
    aMoveBwdLayoutInfo.mnNewUpperWidth = p_rNewUpperFrm.Frm().Width();
    aMoveBwdLayoutInfo.mnNewUpperHeight =  p_rNewUpperFrm.Frm().Height();
    SWRECTFN( (&p_rNewUpperFrm) )
    const SwFrm* pLastLower( p_rNewUpperFrm.Lower() );
    while ( pLastLower && pLastLower->GetNext() )
    {
        pLastLower = pLastLower->GetNext();
    }
    aMoveBwdLayoutInfo.mnFreeSpaceInNewUpper =
            pLastLower
            ? (pLastLower->Frm().*fnRect->fnBottomDist)( (p_rNewUpperFrm.*fnRect->fnGetPrtBottom)() )
            : (p_rNewUpperFrm.Frm().*fnRect->fnGetHeight)();

    // check for moving backward suppress threshold
    const sal_uInt16 cMoveBwdCountSuppressThreshold = 20;
    if ( ++const_cast<SwDoc&>(p_rDoc).GetLayouter()->maMoveBwdLayoutInfo[ aMoveBwdLayoutInfo ] >
                                                cMoveBwdCountSuppressThreshold )
    {
        bMoveBwdSuppressed = true;
    }

    return bMoveBwdSuppressed;
}

void SwLayouter::ClearMoveBwdLayoutInfo( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() )
        const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
