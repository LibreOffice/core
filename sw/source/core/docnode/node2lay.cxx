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

#include <calbck.hxx>
#include <node.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <ftnfrm.hxx>
#include <sectfrm.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <frmtool.hxx>
#include <section.hxx>
#include <node2lay.hxx>

/**
 * The SwNode2LayImpl class does the actual work, the SwNode2Layout class is
 * just the public interface.
 */
class SwNode2LayImpl
{
    std::unique_ptr<SwIterator<SwFrame, SwModify, sw::IteratorMode::UnwrapMulti>> mpIter;
    SwModify* mpMod;
    std::vector<SwFrame*> mvUpperFrames; // To collect the Upper
    sal_uLong mnIndex;        // The Index of the to-be-inserted Nodes
    bool mbMaster    : 1; // true => only Master, false => only Frames without Follow
    bool mbInit      : 1; // Did we already call First() at SwClient?

    SwNode2LayImpl(const SwNode2LayImpl&) = delete;
    SwNode2LayImpl& operator=(const SwNode2LayImpl&) = delete;

public:
    SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, bool bSearch );
    SwFrame* NextFrame(); // Returns the next "useful" Frame
    SwLayoutFrame* UpperFrame( SwFrame* &rpFrame, const SwNode &rNode );
    void SaveUpperFrames(); // Saves (and locks if needed) the pUpper
    // Inserts a Frame under every pUpper of the array
    void RestoreUpperFrames( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd );

    SwFrame* GetFrame( const Point* pDocPos ) const;
};

static SwNode* GoNextWithFrame(const SwNodes& rNodes, SwNodeIndex *pIdx)
{
    if( pIdx->GetIndex() >= rNodes.Count() - 1 )
        return nullptr;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd = nullptr;
    while( aTmp < rNodes.Count()-1 )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsContentNode() )
            // sw_redlinehide: assume that it's OK to find a node with the same
            // frame as the caller's one
            bFound = SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*static_cast<SwContentNode*>(pNd)).First();
        else if ( pNd->IsTableNode() )
            bFound = SwIterator<SwFrame,SwFormat>(*static_cast<SwTableNode*>(pNd)->GetTable().GetFrameFormat()).First() ;
        else if( pNd->IsEndNode() && !pNd->StartOfSectionNode()->IsSectionNode() )
        {
            pNd = nullptr;
            break;
        }
        if ( bFound )
                break;
        ++aTmp;
    }

    if( aTmp == rNodes.Count()-1 )
        pNd = nullptr;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

static SwNode* GoPreviousWithFrame(SwNodeIndex *pIdx)
{
    if( !pIdx->GetIndex() )
        return nullptr;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNode* pNd(nullptr);
    while( aTmp.GetIndex() )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsContentNode() )
            // sw_redlinehide: assume that it's OK to find a node with the same
            // frame as the caller's one
            bFound = SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*static_cast<SwContentNode*>(pNd)).First();
        else if ( pNd->IsTableNode() )
            bFound = SwIterator<SwFrame,SwFormat>(*static_cast<SwTableNode*>(pNd)->GetTable().GetFrameFormat()).First();
        else if( pNd->IsStartNode() && !pNd->IsSectionNode() )
        {
            pNd = nullptr;
            break;
        }
        if ( bFound )
                break;
        --aTmp;
    }

    if( !aTmp.GetIndex() )
        pNd = nullptr;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

/**
 * The main purpose of this ctor is to find the right SwModify to iterate over.
 *
 * @param bSearch true:     find the next Content or TableNode which contains
 *                          Frames (to collect the pUpper).
 *                          Else we assume that rNode points already to such a
 *                          Content or TableNode.
 *                          We insert before or after it.
 */
SwNode2LayImpl::SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, bool bSearch )
    : mnIndex( nIdx ), mbInit( false )
{
    const SwNode* pNd;
    if( bSearch || rNode.IsSectionNode() )
    {
        // Find the next Content/TableNode that contains a Frame, so that we can add
        // ourselves before/after it
        if( !bSearch && rNode.GetIndex() < mnIndex )
        {
            SwNodeIndex aTmp( *rNode.EndOfSectionNode(), +1 );
            pNd = GoPreviousWithFrame( &aTmp );
            if( pNd && rNode.GetIndex() > pNd->GetIndex() )
                pNd = nullptr; // Do not go over the limits
            mbMaster = false;
        }
        else
        {
            SwNodeIndex aTmp( rNode, -1 );
            pNd = GoNextWithFrame( rNode.GetNodes(), &aTmp );
            mbMaster = true;
            if( !bSearch && pNd && rNode.EndOfSectionIndex() < pNd->GetIndex() )
                pNd = nullptr; // Do not go over the limits
        }
    }
    else
    {
        pNd = &rNode;
        mbMaster = mnIndex < rNode.GetIndex();
    }
    if( pNd )
    {
        if( pNd->IsContentNode() )
            mpMod = const_cast<SwModify*>(static_cast<SwModify const *>(pNd->GetContentNode()));
        else
        {
            assert(pNd->IsTableNode());
            mpMod = pNd->GetTableNode()->GetTable().GetFrameFormat();
        }
        mpIter.reset(new SwIterator<SwFrame, SwModify, sw::IteratorMode::UnwrapMulti>(*mpMod));
    }
    else
    {
        mpIter = nullptr;
        mpMod = nullptr;
    }
}

/**
 * Returns the next "useful" Frame.
 *
 * When calling this method for the first time, a First is triggered at the
 * actual Iterator. The result is check for suitability: Follows are not
 * accepted, a Master is accepted when collecting the pUpper and when
 * inserting before it.
 * When inserting after it, we find and return the last Follow starting
 * from the Master.
 *
 * If the Frame is located in a SectionFrame, we check to see whether the
 * SectionFrame is the suitable return value (instead of the Frame itself).
 * This is the case if the to-be-inserted Node is outside of the Section.
 */
SwFrame* SwNode2LayImpl::NextFrame()
{
    SwFrame* pRet;
    if( !mpIter )
        return nullptr;
    if( !mbInit )
    {
         pRet = mpIter->First();
         mbInit = true;
    }
    else
        pRet = mpIter->Next();
    while( pRet )
    {
        SwFlowFrame* pFlow = SwFlowFrame::CastFlowFrame( pRet );
        assert(pFlow);
        // Follows are pretty volatile, thus we ignore them.
        // Even if we insert after the Frame, we start from the Master
        // and iterate through it until the last Follow
        if( !pFlow->IsFollow() )
        {
            if( !mbMaster )
            {
                while( pFlow->HasFollow() )
                    pFlow = pFlow->GetFollow();
                pRet = &(pFlow->GetFrame());
            }
            if( pRet->IsInSct() )
            {
                SwSectionFrame* pSct = pRet->FindSctFrame();
                // ATTENTION: If we are in a Footnote, from a Layout point of view
                // it could be located in a Section with columns, although it
                // should be outside of it when looking at the Nodes.
                // Thus, when dealing with Footnotes, we need to check whether the
                // SectionFrame is also located within the Footnote and not outside of it.
                if( !pRet->IsInFootnote() || pSct->IsInFootnote() )
                {
                    assert(pSct && pSct->GetSection());
                    SwSectionNode* pNd = pSct->GetSection()->GetFormat()->GetSectionNode();
                    assert(pNd);
                    // If the result Frame is located within a Section Frame
                    // whose Section does not contain the Node, we return with
                    // the SectionFrame, else we return with the Content/TabFrame
                    if( mbMaster )
                    {
                        if( pNd->GetIndex() >= mnIndex )
                            pRet = pSct;
                    }
                    else if( pNd->EndOfSectionIndex() < mnIndex )
                        pRet = pSct;
                }
            }
            return pRet;
        }
        pRet = mpIter->Next();
    }
    return nullptr;
}

void SwNode2LayImpl::SaveUpperFrames()
{
    SwFrame* pFrame;
    while( nullptr != (pFrame = NextFrame()) )
    {
        SwFrame* pPrv = pFrame->GetPrev();
        pFrame = pFrame->GetUpper();
        if( pFrame )
        {
            if( pFrame->IsFootnoteFrame() )
                static_cast<SwFootnoteFrame*>(pFrame)->ColLock();
            else if( pFrame->IsInSct() )
                pFrame->FindSctFrame()->ColLock();
            if( pPrv && pPrv->IsSctFrame() )
                static_cast<SwSectionFrame*>(pPrv)->LockJoin();
            mvUpperFrames.push_back( pPrv );
            mvUpperFrames.push_back( pFrame );
        }
    }
    mpIter.reset();
    mpMod = nullptr;
}

SwLayoutFrame* SwNode2LayImpl::UpperFrame( SwFrame* &rpFrame, const SwNode &rNode )
{
    rpFrame = NextFrame();
    if( !rpFrame )
        return nullptr;
    SwLayoutFrame* pUpper = rpFrame->GetUpper();
    if( rpFrame->IsSctFrame() )
    {
        const SwNode* pNode = rNode.StartOfSectionNode();
        if( pNode->IsSectionNode() )
        {
            SwFrame* pFrame = mbMaster ? rpFrame->FindPrev() : rpFrame->FindNext();
            if( pFrame && pFrame->IsSctFrame() )
            {
                // pFrame could be a "dummy"-section
                if( static_cast<SwSectionFrame*>(pFrame)->GetSection() &&
                    (&static_cast<const SwSectionNode*>(pNode)->GetSection() ==
                     static_cast<SwSectionFrame*>(pFrame)->GetSection()) )
                {
                    // #i22922# - consider columned sections
                    // 'Go down' the section frame as long as the layout frame
                    // is found, which would contain content.
                    while ( pFrame->IsLayoutFrame() &&
                            static_cast<SwLayoutFrame*>(pFrame)->Lower() &&
                            !static_cast<SwLayoutFrame*>(pFrame)->Lower()->IsFlowFrame() &&
                            static_cast<SwLayoutFrame*>(pFrame)->Lower()->IsLayoutFrame() )
                    {
                        pFrame = static_cast<SwLayoutFrame*>(pFrame)->Lower();
                    }
                    assert(pFrame->IsLayoutFrame());
                    rpFrame = mbMaster ? nullptr
                                    : static_cast<SwLayoutFrame*>(pFrame)->Lower();
                    assert((!rpFrame || rpFrame->IsFlowFrame()) &&
                            "<SwNode2LayImpl::UpperFrame(..)> - expected sibling isn't a flow frame." );
                    return static_cast<SwLayoutFrame*>(pFrame);
                }

                pUpper = new SwSectionFrame(const_cast<SwSectionNode*>(static_cast<const SwSectionNode*>(pNode))->GetSection(), rpFrame);
                pUpper->Paste( rpFrame->GetUpper(),
                               mbMaster ? rpFrame : rpFrame->GetNext() );
                static_cast<SwSectionFrame*>(pUpper)->Init();
                rpFrame = nullptr;
                // 'Go down' the section frame as long as the layout frame
                // is found, which would contain content.
                while ( pUpper->Lower() &&
                        !pUpper->Lower()->IsFlowFrame() &&
                        pUpper->Lower()->IsLayoutFrame() )
                {
                    pUpper = static_cast<SwLayoutFrame*>(pUpper->Lower());
                }
                return pUpper;
            }
        }
    }
    if( !mbMaster )
        rpFrame = rpFrame->GetNext();
    return pUpper;
}

void SwNode2LayImpl::RestoreUpperFrames( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd )
{
    SwNode* pNd;
    SwDoc& rDoc = rNds.GetDoc();
    bool bFirst = true;
    for( ; nStt < nEnd; ++nStt )
    {
        SwFrame* pNew = nullptr;
        SwFrame* pNxt;
        SwLayoutFrame* pUp;
        if( (pNd = rNds[nStt])->IsContentNode() )
            for( std::vector<SwFrame*>::size_type n = 0; n < mvUpperFrames.size(); )
            {
                pNxt = mvUpperFrames[n++];
                if( bFirst && pNxt && pNxt->IsSctFrame() )
                    static_cast<SwSectionFrame*>(pNxt)->UnlockJoin();
                pUp = static_cast<SwLayoutFrame*>(mvUpperFrames[n++]);
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = static_cast<SwContentNode*>(pNd)->MakeFrame( pUp );
                pNew->Paste( pUp, pNxt );
                mvUpperFrames[n-2] = pNew;
            }
        else if( pNd->IsTableNode() )
            for( std::vector<SwFrame*>::size_type x = 0; x < mvUpperFrames.size(); )
            {
                pNxt = mvUpperFrames[x++];
                if( bFirst && pNxt && pNxt->IsSctFrame() )
                    static_cast<SwSectionFrame*>(pNxt)->UnlockJoin();
                pUp = static_cast<SwLayoutFrame*>(mvUpperFrames[x++]);
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = static_cast<SwTableNode*>(pNd)->MakeFrame( pUp );
                assert(pNew->IsTabFrame());
                pNew->Paste( pUp, pNxt );
                static_cast<SwTabFrame*>(pNew)->RegistFlys();
                mvUpperFrames[x-2] = pNew;
            }
        else if( pNd->IsSectionNode() )
        {
            nStt = pNd->EndOfSectionIndex();
            for( std::vector<SwFrame*>::size_type x = 0; x < mvUpperFrames.size(); )
            {
                pNxt = mvUpperFrames[x++];
                if( bFirst && pNxt && pNxt->IsSctFrame() )
                    static_cast<SwSectionFrame*>(pNxt)->UnlockJoin();
                pUp = static_cast<SwLayoutFrame*>(mvUpperFrames[x++]);
                OSL_ENSURE( pUp->GetUpper() || pUp->IsFlyFrame(), "Lost Upper" );
                ::InsertCnt_( pUp, &rDoc, pNd->GetIndex(), false, nStt+1, pNxt );
                pNxt = pUp->GetLastLower();
                mvUpperFrames[x-2] = pNxt;
            }
        }
        bFirst = false;
    }
    for( std::vector<SwFrame*>::size_type x = 0; x < mvUpperFrames.size(); ++x )
    {
        SwFrame* pTmp = mvUpperFrames[++x];
        if( pTmp->IsFootnoteFrame() )
            static_cast<SwFootnoteFrame*>(pTmp)->ColUnlock();
        else if ( pTmp->IsInSct() )
        {
            SwSectionFrame* pSctFrame = pTmp->FindSctFrame();
            pSctFrame->ColUnlock();
            // #i18103# - invalidate size of section in order to
            // assure, that the section is formatted, unless it was 'Collocked'
            // from its 'collection' until its 'restoration'.
            pSctFrame->InvalidateSize_();
        }
    }
}

SwFrame* SwNode2LayImpl::GetFrame( const Point* pDocPos ) const
{
    // test if change of member pIter -> pMod broke anything
    std::pair<Point, bool> tmp;
    if (pDocPos)
    {
        tmp.first = *pDocPos;
        tmp.second = false;
    }
    return mpMod ? ::GetFrameOfModify(nullptr, *mpMod, FRM_ALL, nullptr, pDocPos ? &tmp : nullptr) : nullptr;
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd, sal_uLong nIdx )
    : m_pImpl( new SwNode2LayImpl( rNd, nIdx, false ) )
{
}

SwNode2LayoutSaveUpperFrames::SwNode2LayoutSaveUpperFrames(const SwNode& rNd)
    : m_pImpl( new SwNode2LayImpl( rNd, rNd.GetIndex(), true ) )
{
    m_pImpl->SaveUpperFrames();
}

void SwNode2LayoutSaveUpperFrames::RestoreUpperFrames(
        SwNodes& rNds, sal_uLong const nStt, sal_uLong const nEnd)
{
    m_pImpl->RestoreUpperFrames( rNds, nStt, nEnd );
}

SwFrame* SwNode2Layout::NextFrame()
{
    return m_pImpl->NextFrame();
}

SwLayoutFrame* SwNode2Layout::UpperFrame( SwFrame* &rpFrame, const SwNode &rNode )
{
    return m_pImpl->UpperFrame( rpFrame, rNode );
}

SwNode2Layout::~SwNode2Layout()
{
}

SwNode2LayoutSaveUpperFrames::~SwNode2LayoutSaveUpperFrames()
{
}

SwFrame* SwNode2Layout::GetFrame( const Point* pDocPos ) const
{
    return m_pImpl->GetFrame( pDocPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
