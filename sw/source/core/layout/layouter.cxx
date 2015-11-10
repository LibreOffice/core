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
#include <IDocumentLayoutAccess.hxx>

#include <movedfwdfrmsbyobjpos.hxx>
#include <objstmpconsiderwrapinfl.hxx>

#define LOOP_DETECT 250

class SwLooping
{
    sal_uInt16 nMinPage;
    sal_uInt16 nMaxPage;
    sal_uInt16 nCount;
    sal_uInt16 mnLoopControlStage;
public:
    explicit SwLooping( SwPageFrm* pPage );
    void Control( SwPageFrm* pPage );
    void Drastic( SwFrm* pFrm );
    bool IsLoopingLouieLight() const { return nCount > LOOP_DETECT - 30; };
};

class SwEndnoter
{
    SwLayouter* pMaster;
    SwSectionFrm* pSect;
    SwFootnoteFrms*    pEndArr;
public:
    explicit SwEndnoter( SwLayouter* pLay )
        : pMaster( pLay ), pSect( nullptr ), pEndArr( nullptr ) {}
    ~SwEndnoter() { delete pEndArr; }
    void CollectEndnotes( SwSectionFrm* pSct );
    void CollectEndnote( SwFootnoteFrm* pFootnote );
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

void SwEndnoter::CollectEndnote( SwFootnoteFrm* pFootnote )
{
    if( pEndArr && pEndArr->end() != std::find( pEndArr->begin(), pEndArr->end(), pFootnote ) )
        return;

    if( pFootnote->GetUpper() )
    {
        // pFootnote is the master, he incorporates its follows
        SwFootnoteFrm *pNxt = pFootnote->GetFollow();
        while ( pNxt )
        {
            SwFrm *pCnt = pNxt->ContainsAny();
            if ( pCnt )
            {
                do
                {   SwFrm *pNxtCnt = pCnt->GetNext();
                    pCnt->Cut();
                    pCnt->Paste( pFootnote );
                    pCnt = pNxtCnt;
                } while ( pCnt );
            }
            else
            { OSL_ENSURE( pNxt->Lower() && pNxt->Lower()->IsSctFrm(),
                        "Endnote without content?" );
                pNxt->Cut();
                SwFrm::DestroyFrm(pNxt);
            }
            pNxt = pFootnote->GetFollow();
        }
        if( pFootnote->GetMaster() )
            return;
        pFootnote->Cut();
    }
    else if( pEndArr )
    {
        for ( size_t i = 0; i < pEndArr->size(); ++i )
        {
            SwFootnoteFrm *pEndFootnote = (*pEndArr)[i];
            if( pEndFootnote->GetAttr() == pFootnote->GetAttr() )
            {
                SwFrm::DestroyFrm(pFootnote);
                return;
            }
        }
    }
    if( !pEndArr )
        pEndArr = new SwFootnoteFrms;  // deleted from the SwLayouter
    pEndArr->push_back( pFootnote );
}

void SwEndnoter::InsertEndnotes()
{
    if( !pSect )
        return;
    if( !pEndArr || pEndArr->empty() )
    {
        pSect = nullptr;
        return;
    }
    OSL_ENSURE( pSect->Lower() && pSect->Lower()->IsFootnoteBossFrm(),
            "InsertEndnotes: Where's my column?" );
    SwFrm* pRef = pSect->FindLastContent( FINDMODE_MYLAST );
    SwFootnoteBossFrm *pBoss = pRef ? pRef->FindFootnoteBossFrm()
                               : static_cast<SwFootnoteBossFrm*>(pSect->Lower());
    pBoss->_MoveFootnotes( *pEndArr );
    delete pEndArr;
    pEndArr = nullptr;
    pSect = nullptr;
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
    const sal_uInt16 nNew = pPage->GetPhyPageNum();
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
            Drastic( static_cast<SwPageFrm*>(pPage->GetPrev())->Lower() );
        if( nNew < nMaxPage && pPage->GetNext() )
            Drastic( static_cast<SwPageFrm*>(pPage->GetNext())->Lower() );

        ++mnLoopControlStage;
        nCount = 0;
    }
}

SwLayouter::SwLayouter()
        : mpEndnoter( nullptr ),
          mpLooping( nullptr ),
          // #i28701#
          mpMovedFwdFrms( nullptr ),
          // #i35911#
          mpObjsTmpConsiderWrapInfl( nullptr )
{
}

SwLayouter::~SwLayouter()
{
    delete mpEndnoter;
    delete mpLooping;
    // #i28701#
    delete mpMovedFwdFrms;
    mpMovedFwdFrms = nullptr;
    // #i35911#
    delete mpObjsTmpConsiderWrapInfl;
    mpObjsTmpConsiderWrapInfl = nullptr;
}

void SwLayouter::_CollectEndnotes( SwSectionFrm* pSect )
{
    if( !mpEndnoter )
        mpEndnoter = new SwEndnoter( this );
    mpEndnoter->CollectEndnotes( pSect );
}

bool SwLayouter::HasEndnotes() const
{
    return mpEndnoter->HasEndnotes();
}

void SwLayouter::CollectEndnote( SwFootnoteFrm* pFootnote )
{
    mpEndnoter->CollectEndnote( pFootnote );
}

void SwLayouter::InsertEndnotes( SwSectionFrm* pSect )
{
    if( !mpEndnoter || mpEndnoter->GetSect() != pSect )
        return;
    mpEndnoter->InsertEndnotes();
}

void SwLayouter::LoopControl( SwPageFrm* pPage, sal_uInt8 )
{
    OSL_ENSURE( mpLooping, "Looping: Lost control" );
    mpLooping->Control( pPage );
}

void SwLayouter::LoopingLouieLight( const SwDoc& rDoc, const SwTextFrm& rFrm )
{
    if ( mpLooping && mpLooping->IsLoopingLouieLight() )
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_FAIL( "Looping Louie (Light): Fixating fractious frame" );
#endif
        SwLayouter::InsertMovedFwdFrm( rDoc, rFrm, rFrm.FindPageFrm()->GetPhyPageNum() );
    }
}

bool SwLayouter::StartLooping( SwPageFrm* pPage )
{
    if( mpLooping )
        return false;
    mpLooping = new SwLooping( pPage );
    return true;
}

void SwLayouter::EndLoopControl()
{
    delete mpLooping;
    mpLooping = nullptr;
}

void SwLayouter::CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect )
{
    assert(pDoc && "No doc, no fun");
    if( !pDoc->getIDocumentLayoutAccess().GetLayouter() )
        pDoc->getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    pDoc->getIDocumentLayoutAccess().GetLayouter()->_CollectEndnotes( pSect );
}

bool SwLayouter::Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFootnoteFrm* pFootnote )
{
    if( !pDoc->getIDocumentLayoutAccess().GetLayouter() )
        return false;
    SwLayouter *pLayouter = pDoc->getIDocumentLayoutAccess().GetLayouter();
    if( pLayouter->mpEndnoter && pLayouter->mpEndnoter->GetSect() && pSect &&
        ( pLayouter->mpEndnoter->GetSect()->IsAnFollow( pSect ) ||
          pSect->IsAnFollow( pLayouter->mpEndnoter->GetSect() ) ) )
    {
        if( pFootnote )
            pLayouter->CollectEndnote( pFootnote );
        return true;
    }
    return false;
}

bool SwLayouter::StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage )
{
    OSL_ENSURE( pDoc, "No doc, no fun" );
    if( !pDoc->getIDocumentLayoutAccess().GetLayouter() )
        pDoc->getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    return !pDoc->getIDocumentLayoutAccess().GetLayouter()->mpLooping &&
            pDoc->getIDocumentLayoutAccess().GetLayouter()->StartLooping( pPage );
}

// #i28701#
// methods to manage text frames, which are moved forward by the positioning
// of its anchored objects
void SwLayouter::ClearMovedFwdFrms( const SwDoc& _rDoc )
{
    if ( _rDoc.getIDocumentLayoutAccess().GetLayouter() &&
         _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms )
    {
        _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms->Clear();
    }
}

void SwLayouter::InsertMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTextFrm& _rMovedFwdFrmByObjPos,
                                    const sal_uInt32 _nToPageNum )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms =
                                                new SwMovedFwdFrmsByObjPos();
    }

    _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms->Insert( _rMovedFwdFrmByObjPos,
                                                 _nToPageNum );
}

// #i40155#
void SwLayouter::RemoveMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTextFrm& _rTextFrm )
{
    sal_uInt32 nDummy;
    if ( SwLayouter::FrmMovedFwdByObjPos( _rDoc, _rTextFrm, nDummy ) )
    {
        _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms->Remove( _rTextFrm );
    }
}

bool SwLayouter::FrmMovedFwdByObjPos( const SwDoc& _rDoc,
                                      const SwTextFrm& _rTextFrm,
                                      sal_uInt32& _ornToPageNum )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        _ornToPageNum = 0;
        return false;
    }
    else if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms )
    {
        _ornToPageNum = 0;
        return false;
    }
    else
    {
        return _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms->
                                FrmMovedFwdByObjPos( _rTextFrm, _ornToPageNum );
    }
}

// #i26945#
bool SwLayouter::DoesRowContainMovedFwdFrm( const SwDoc& _rDoc,
                                            const SwRowFrm& _rRowFrm )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        return false;
    }
    else if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrms )
    {
        return false;
    }
    else
    {
        return _rDoc.getIDocumentLayoutAccess().GetLayouter()->
                        mpMovedFwdFrms->DoesRowContainMovedFwdFrm( _rRowFrm );
    }
}

// #i35911#
void SwLayouter::ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc )
{
    if ( _rDoc.getIDocumentLayoutAccess().GetLayouter() &&
         _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl->Clear();
    }
}
void SwLayouter::InsertObjForTmpConsiderWrapInfluence(
                                            const SwDoc& _rDoc,
                                            SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl =
                                new SwObjsMarkedAsTmpConsiderWrapInfluence();
    }

    _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl->Insert( _rAnchoredObj );
}

void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTextFrm& rTextFrm )
{
    if ( bCondition )
    {
        const SwDoc& rDoc = *rTextFrm.GetAttrSet()->GetDoc();
        if ( rDoc.getIDocumentLayoutAccess().GetLayouter() )
        {
            const_cast<SwDoc&>(rDoc).getIDocumentLayoutAccess().GetLayouter()->LoopingLouieLight( rDoc, rTextFrm );
        }
    }
}

// #i65250#
bool SwLayouter::MoveBwdSuppressed( const SwDoc& p_rDoc,
                                    const SwFlowFrm& p_rFlowFrm,
                                    const SwLayoutFrm& p_rNewUpperFrm )
{
    bool bMoveBwdSuppressed( false );

    if ( !p_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        const_cast<SwDoc&>(p_rDoc).getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    }

    // create hash map key
    tMoveBwdLayoutInfoKey aMoveBwdLayoutInfo;
    aMoveBwdLayoutInfo.mnFrmId = p_rFlowFrm.GetFrm().GetFrmId();
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
    if ( ++const_cast<SwDoc&>(p_rDoc).getIDocumentLayoutAccess().GetLayouter()->maMoveBwdLayoutInfo[ aMoveBwdLayoutInfo ] >
                                                cMoveBwdCountSuppressThreshold )
    {
        bMoveBwdSuppressed = true;
    }

    return bMoveBwdSuppressed;
}

void SwLayouter::ClearMoveBwdLayoutInfo( const SwDoc& _rDoc )
{
    if ( _rDoc.getIDocumentLayoutAccess().GetLayouter() )
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().GetLayouter()->maMoveBwdLayoutInfo.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
