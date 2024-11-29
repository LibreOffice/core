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

#include <memory>
#include <layouter.hxx>
#include <doc.hxx>
#include <sectfrm.hxx>
#include <pagefrm.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <movedfwdfrmsbyobjpos.hxx>
#include "objstmpconsiderwrapinfl.hxx"
#include <osl/diagnose.h>

#define LOOP_DETECT 250

class SwLooping
{
    sal_uInt16 mnMinPage;
    sal_uInt16 mnMaxPage;
    sal_uInt16 mnCount;
    sal_uInt16 mnLoopControlStage;
public:
    explicit SwLooping( SwPageFrame const * pPage );
    void Control( SwPageFrame* pPage );
    void Drastic( SwFrame* pFrame );
    bool IsLoopingLouieLight() const { return mnCount > LOOP_DETECT - 30; };
};

class SwEndnoter
{
    SwLayouter*                        m_pMaster;
    SwSectionFrame*                    m_pSect;
    std::unique_ptr<SwFootnoteFrames>  m_pEndArr;
public:
    explicit SwEndnoter( SwLayouter* pLay )
        : m_pMaster( pLay ), m_pSect( nullptr ) {}
    void CollectEndnotes( SwSectionFrame* pSct );
    void CollectEndnote( SwFootnoteFrame* pFootnote );
    const SwSectionFrame* GetSect() const { return m_pSect; }
    void InsertEndnotes();
    bool HasEndnotes() const { return m_pEndArr && !m_pEndArr->empty(); }
};

void SwEndnoter::CollectEndnotes( SwSectionFrame* pSct )
{
    OSL_ENSURE( pSct, "CollectEndnotes: Which section?" );
    if( !m_pSect )
        m_pSect = pSct;
    else if( pSct != m_pSect )
        return;
    m_pSect->CollectEndnotes( m_pMaster );
}

void SwEndnoter::CollectEndnote( SwFootnoteFrame* pFootnote )
{
    if( m_pEndArr && m_pEndArr->end() != std::find( m_pEndArr->begin(), m_pEndArr->end(), pFootnote ) )
        return;

    if( pFootnote->GetUpper() )
    {
        // pFootnote is the master, he incorporates its follows
        SwFootnoteFrame *pNxt = pFootnote->GetFollow();
        while ( pNxt )
        {
            SwFrame *pCnt = pNxt->ContainsAny();
            if ( pCnt )
            {
                do
                {   SwFrame *pNxtCnt = pCnt->GetNext();
                    pCnt->Cut();
                    pCnt->Paste( pFootnote );
                    pCnt = pNxtCnt;
                } while ( pCnt );
            }
            else
            {
#ifdef DBG_UTIL
                SwFrame* pLower = pNxt->Lower();
                OSL_ENSURE( pLower && pLower->IsSctFrame(),
                        "Endnote without content?" );
#endif
                pNxt->Cut();
                SwFrame::DestroyFrame(pNxt);
            }
            pNxt = pFootnote->GetFollow();
        }
        if( pFootnote->GetMaster() )
            return;
        pFootnote->Cut();
    }
    else if( m_pEndArr )
    {
        for (SwFootnoteFrame* pEndFootnote : *m_pEndArr)
        {
            if( pEndFootnote->GetAttr() == pFootnote->GetAttr() )
            {
                SwFrame::DestroyFrame(pFootnote);
                return;
            }
        }
    }
    if( !m_pEndArr )
        m_pEndArr.reset( new SwFootnoteFrames );  // deleted from the SwLayouter
    m_pEndArr->push_back( pFootnote );
}

void SwEndnoter::InsertEndnotes()
{
    if( !m_pSect )
        return;
    if( !m_pEndArr || m_pEndArr->empty() )
    {
        m_pSect = nullptr;
        return;
    }
    SwFrame* pLower = m_pSect->Lower();
    OSL_ENSURE( pLower && pLower->IsFootnoteBossFrame(),
            "InsertEndnotes: Where's my column?" );
    SwFrame* pRef = m_pSect->FindLastContent( SwFindMode::MyLast );
    SwFootnoteBossFrame *pBoss = pRef ? pRef->FindFootnoteBossFrame()
                               : static_cast<SwFootnoteBossFrame*>(pLower);
    pBoss->MoveFootnotes_( *m_pEndArr );
    m_pEndArr.reset();
    m_pSect = nullptr;
}

SwLooping::SwLooping( SwPageFrame const * pPage )
{
    assert(pPage);
    mnMinPage = pPage->GetPhyPageNum();
    mnMaxPage = mnMinPage;
    mnCount = 0;
    mnLoopControlStage = 0;
}

void SwLooping::Drastic( SwFrame* pFrame )
{
    while( pFrame )
    {
        pFrame->ValidateThisAndAllLowers( mnLoopControlStage );
        pFrame = pFrame->GetNext();
    }
}

void SwLooping::Control( SwPageFrame* pPage )
{
    if( !pPage )
        return;
    const sal_uInt16 nNew = pPage->GetPhyPageNum();
    if (nNew > mnMaxPage)
        mnMaxPage = nNew;
    if (nNew < mnMinPage)
    {
        mnMinPage = nNew;
        mnMaxPage = nNew;
        mnCount = 0;
        mnLoopControlStage = 0;
    }
    else if (nNew > mnMinPage + 2)
    {
        mnMinPage = nNew - 2;
        mnMaxPage = nNew;
        mnCount = 0;
        mnLoopControlStage = 0;
    }
    else if (++mnCount > LOOP_DETECT)
    {
#if OSL_DEBUG_LEVEL > 1
        static bool bNoLouie = false;
        if( bNoLouie )
            return;
#endif

        // FME 2007-08-30 #i81146# new loop control
        SAL_WARN("sw.layout", "Looping Louie: Stage " << (mnLoopControlStage + 1) << "!");

        Drastic( pPage->Lower() );
        if (nNew > mnMinPage && pPage->GetPrev())
            Drastic( static_cast<SwPageFrame*>(pPage->GetPrev())->Lower() );
        if (nNew < mnMaxPage && pPage->GetNext())
            Drastic( static_cast<SwPageFrame*>(pPage->GetNext())->Lower() );

        ++mnLoopControlStage;
        mnCount = 0;
    }
}

SwLayouter::SwLayouter()
{
}

SwLayouter::~SwLayouter()
{
}

void SwLayouter::CollectEndnotes_( SwSectionFrame* pSect )
{
    if( !mpEndnoter )
        mpEndnoter.reset(new SwEndnoter( this ));
    mpEndnoter->CollectEndnotes( pSect );
}

bool SwLayouter::HasEndnotes() const
{
    return mpEndnoter->HasEndnotes();
}

void SwLayouter::CollectEndnote( SwFootnoteFrame* pFootnote )
{
    mpEndnoter->CollectEndnote( pFootnote );
}

void SwLayouter::InsertEndnotes( SwSectionFrame const * pSect )
{
    if( !mpEndnoter || mpEndnoter->GetSect() != pSect )
        return;
    mpEndnoter->InsertEndnotes();
}

void SwLayouter::LoopControl( SwPageFrame* pPage )
{
    assert(mpLooping);
    mpLooping->Control( pPage );
}

void SwLayouter::LoopingLouieLight( const SwDoc& rDoc, const SwTextFrame& rFrame )
{
    if ( mpLooping && mpLooping->IsLoopingLouieLight() )
    {
        SAL_WARN("sw.layout", "Looping Louie (Light): Fixating fractious frame");
        SwLayouter::InsertMovedFwdFrame( rDoc, rFrame, rFrame.FindPageFrame()->GetPhyPageNum() );
    }
}

bool SwLayouter::StartLooping( SwPageFrame const * pPage )
{
    if( mpLooping )
        return false;
    mpLooping.reset(new SwLooping( pPage ));
    return true;
}

void SwLayouter::EndLoopControl()
{
    mpLooping.reset();
}

void SwLayouter::CollectEndnotes( SwDoc* pDoc, SwSectionFrame* pSect )
{
    assert(pDoc && "No doc, no fun");
    if( !pDoc->getIDocumentLayoutAccess().GetLayouter() )
        pDoc->getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    pDoc->getIDocumentLayoutAccess().GetLayouter()->CollectEndnotes_( pSect );
}

bool SwLayouter::Collecting( SwDoc* pDoc, SwSectionFrame const * pSect, SwFootnoteFrame* pFootnote )
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

bool SwLayouter::StartLoopControl( SwDoc* pDoc, SwPageFrame const *pPage )
{
    assert(pDoc);
    if( !pDoc->getIDocumentLayoutAccess().GetLayouter() )
        pDoc->getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    return !pDoc->getIDocumentLayoutAccess().GetLayouter()->mpLooping &&
            pDoc->getIDocumentLayoutAccess().GetLayouter()->StartLooping( pPage );
}

// #i28701#
// methods to manage text frames, which are moved forward by the positioning
// of its anchored objects
void SwLayouter::ClearMovedFwdFrames( const SwDoc& _rDoc )
{
    if ( _rDoc.getIDocumentLayoutAccess().GetLayouter() &&
         _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames )
    {
        _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames->Clear();
    }
}

void SwLayouter::InsertMovedFwdFrame( const SwDoc& _rDoc,
                                    const SwTextFrame& _rMovedFwdFrameByObjPos,
                                    const sal_uInt32 _nToPageNum )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames )
    {
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames.reset(
                                                new SwMovedFwdFramesByObjPos());
    }

    _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames->Insert( _rMovedFwdFrameByObjPos,
                                                 _nToPageNum );
}

// #i40155#
void SwLayouter::RemoveMovedFwdFrame( const SwDoc& _rDoc,
                                    const SwTextFrame& _rTextFrame )
{
    sal_uInt32 nDummy;
    if ( SwLayouter::FrameMovedFwdByObjPos( _rDoc, _rTextFrame, nDummy ) )
    {
        _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames->Remove( _rTextFrame );
    }
}

bool SwLayouter::FrameMovedFwdByObjPos( const SwDoc& _rDoc,
                                      const SwTextFrame& _rTextFrame,
                                      sal_uInt32& _ornToPageNum )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        _ornToPageNum = 0;
        return false;
    }
    else if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames )
    {
        _ornToPageNum = 0;
        return false;
    }
    else
    {
        return _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames->
                                FrameMovedFwdByObjPos( _rTextFrame, _ornToPageNum );
    }
}

// #i26945#
bool SwLayouter::DoesRowContainMovedFwdFrame( const SwDoc& _rDoc,
                                            const SwRowFrame& _rRowFrame )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        return false;
    }
    else if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpMovedFwdFrames )
    {
        return false;
    }
    else
    {
        return _rDoc.getIDocumentLayoutAccess().GetLayouter()->
                        mpMovedFwdFrames->DoesRowContainMovedFwdFrame( _rRowFrame );
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
        const_cast<SwDoc&>(_rDoc).getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl.reset(
                                new SwObjsMarkedAsTmpConsiderWrapInfluence());
    }

    _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl->Insert( _rAnchoredObj );
}

void SwLayouter::RemoveObjForTmpConsiderWrapInfluence(
                                            const SwDoc& _rDoc,
                                            SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter() )
        return;

    if ( !_rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl )
        return;

    _rDoc.getIDocumentLayoutAccess().GetLayouter()->mpObjsTmpConsiderWrapInfl->Remove( _rAnchoredObj );
}


void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTextFrame& rTextFrame )
{
    if ( bCondition )
    {
        const SwDoc& rDoc = *rTextFrame.GetAttrSet()->GetDoc();
        if ( rDoc.getIDocumentLayoutAccess().GetLayouter() )
        {
            const_cast<SwDoc&>(rDoc).getIDocumentLayoutAccess().GetLayouter()->LoopingLouieLight( rDoc, rTextFrame );
        }
    }
}

// #i65250#
bool SwLayouter::MoveBwdSuppressed( const SwDoc& p_rDoc,
                                    const SwFlowFrame& p_rFlowFrame,
                                    const SwLayoutFrame& p_rNewUpperFrame )
{
    bool bMoveBwdSuppressed( false );

    if ( !p_rDoc.getIDocumentLayoutAccess().GetLayouter() )
    {
        const_cast<SwDoc&>(p_rDoc).getIDocumentLayoutAccess().SetLayouter( new SwLayouter() );
    }

    // create hash map key
    tMoveBwdLayoutInfoKey aMoveBwdLayoutInfo;
    aMoveBwdLayoutInfo.mnFrameId = p_rFlowFrame.GetFrame().GetFrameId();
    aMoveBwdLayoutInfo.mnNewUpperPosX = p_rNewUpperFrame.getFrameArea().Pos().X();
    aMoveBwdLayoutInfo.mnNewUpperPosY = p_rNewUpperFrame.getFrameArea().Pos().Y();
    aMoveBwdLayoutInfo.mnNewUpperWidth = p_rNewUpperFrame.getFrameArea().Width();
    aMoveBwdLayoutInfo.mnNewUpperHeight =  p_rNewUpperFrame.getFrameArea().Height();
    SwRectFnSet aRectFnSet(&p_rNewUpperFrame);
    const SwFrame* pLastLower( p_rNewUpperFrame.Lower() );
    while ( pLastLower && pLastLower->GetNext() )
    {
        pLastLower = pLastLower->GetNext();
    }
    aMoveBwdLayoutInfo.mnFreeSpaceInNewUpper =
            pLastLower
            ? aRectFnSet.BottomDist( pLastLower->getFrameArea(), aRectFnSet.GetPrtBottom(p_rNewUpperFrame) )
            : aRectFnSet.GetHeight(p_rNewUpperFrame.getFrameArea());

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
