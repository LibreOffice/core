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

#include <switerator.hxx>
#include <calbck.hxx>
#include <node.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <ftnfrm.hxx>
#include <sectfrm.hxx>
#include "frmfmt.hxx"
#include "cntfrm.hxx"
#include "tabfrm.hxx"
#include "frmtool.hxx"
#include "section.hxx"
#include "node2lay.hxx"

/**
 * The SwNode2LayImpl class does the actual work, the SwNode2Layout class is
 * just the public interface.
 */
class SwNode2LayImpl
{
    SwIterator<SwFrm,SwModify>* pIter;
    SwModify* pMod;
    std::vector<SwFrm*>* pUpperFrms; // To collect the Upper
    sal_uLong nIndex;        // The Index of the to-be-inserted Nodes
    bool bMaster    : 1; // true => only Master, false => only Frames without Follow
    bool bInit      : 1; // Did we already call First() at SwClient?
public:
    SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, bool bSearch );
    ~SwNode2LayImpl() { delete pIter; delete pUpperFrms; }
    SwFrm* NextFrm(); // Returns the next "useful" Frame
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode &rNode );
    void SaveUpperFrms(); // Saves (and locks if needed) the pUpper
    // Inserts a Frame under every pUpper of the array
    void RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd );

    SwFrm* GetFrm( const Point* pDocPos = 0,
                    const SwPosition *pPos = 0,
                    const bool bCalcFrm = true ) const;
};

SwNode* GoNextWithFrm(const SwNodes& rNodes, SwNodeIndex *pIdx)
{
    if( pIdx->GetIndex() >= rNodes.Count() - 1 )
        return 0;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd = 0;
    while( aTmp < rNodes.Count()-1 )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsCntntNode() )
            bFound = ( SwIterator<SwFrm,SwCntntNode>::FirstElement(*(SwCntntNode*)pNd) != 0);
        else if ( pNd->IsTableNode() )
            bFound = ( SwIterator<SwFrm,SwFmt>::FirstElement(*((SwTableNode*)pNd)->GetTable().GetFrmFmt()) != 0 );
        else if( pNd->IsEndNode() && !pNd->StartOfSectionNode()->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( bFound )
                break;
        ++aTmp;
    }

    if( aTmp == rNodes.Count()-1 )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

SwNode* GoPreviousWithFrm(SwNodeIndex *pIdx)
{
    if( !pIdx->GetIndex() )
        return 0;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNode* pNd(0);
    while( aTmp.GetIndex() )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsCntntNode() )
            bFound = ( SwIterator<SwFrm,SwCntntNode>::FirstElement(*(SwCntntNode*)pNd) != 0);
        else if ( pNd->IsTableNode() )
            bFound = ( SwIterator<SwFrm,SwFmt>::FirstElement(*((SwTableNode*)pNd)->GetTable().GetFrmFmt()) != 0 );
        else if( pNd->IsStartNode() && !pNd->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( bFound )
                break;
        aTmp--;
    }

    if( !aTmp.GetIndex() )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

/**
 * The main purpose of this ctor is to find the right SwModify to iterate over.
 *
 * @param bSearch sal_True: find the next Content or TableNode which contains
 *                          Frames (to collect the pUpper).
 *                          Else we assume that rNode points already to such a
 *                          Content or TableNode.
 *                          We insert before or after it.
 */
SwNode2LayImpl::SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, bool bSearch )
    : pUpperFrms( NULL ), nIndex( nIdx ), bInit( false )
{
    const SwNode* pNd;
    if( bSearch || rNode.IsSectionNode() )
    {
        // Find the next Cntnt/TableNode that contains a Frame, so that we can add
        // ourselves before/after it
        if( !bSearch && rNode.GetIndex() < nIndex )
        {
            SwNodeIndex aTmp( *rNode.EndOfSectionNode(), +1 );
            pNd = GoPreviousWithFrm( &aTmp );
            if( !bSearch && pNd && rNode.GetIndex() > pNd->GetIndex() )
                pNd = NULL; // Do not go over the limits
            bMaster = false;
        }
        else
        {
            SwNodeIndex aTmp( rNode, -1 );
            pNd = GoNextWithFrm( rNode.GetNodes(), &aTmp );
            bMaster = true;
            if( !bSearch && pNd && rNode.EndOfSectionIndex() < pNd->GetIndex() )
                pNd = NULL; // Do not go over the limits
        }
    }
    else
    {
        pNd = &rNode;
        bMaster = nIndex < rNode.GetIndex();
    }
    if( pNd )
    {
        if( pNd->IsCntntNode() )
            pMod = (SwModify*)pNd->GetCntntNode();
        else
        {
            OSL_ENSURE( pNd->IsTableNode(), "For Tablenodes only" );
            pMod = pNd->GetTableNode()->GetTable().GetFrmFmt();
        }
        pIter = new SwIterator<SwFrm,SwModify>( *pMod );
    }
    else
    {
        pIter = NULL;
        pMod = 0;
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
 * If the Frame is located in a SectionFrm, we check to see whether the
 * SectionFrame is the suitable return value (instead of the Frame itself).
 * This is the case if the to-be-inserted Node is outside of the Section.
 */
SwFrm* SwNode2LayImpl::NextFrm()
{
    SwFrm* pRet;
    if( !pIter )
        return NULL;
    if( !bInit )
    {
         pRet = pIter->First();
         bInit = true;
    }
    else
        pRet = pIter->Next();
    while( pRet )
    {
        SwFlowFrm* pFlow = SwFlowFrm::CastFlowFrm( pRet );
        OSL_ENSURE( pFlow, "Cntnt or Table expected?!" );
        // Follows are pretty volatile, thus we ignore them.
        // Even if we insert after the Frame, we start from the Master
        // and iterate through it until the last Follow
        if( !pFlow->IsFollow() )
        {
            if( !bMaster )
            {
                while( pFlow->HasFollow() )
                    pFlow = pFlow->GetFollow();
                pRet = pFlow->GetFrm();
            }
            if( pRet->IsInSct() )
            {
                SwSectionFrm* pSct = pRet->FindSctFrm();
                // ATTENTION: If we are in a Footnote, from a Layout point of view
                // it could be located in a Section with columns, although it
                // should be outside of it when looking at the Nodes.
                // Thus, when dealing with Footnotes, we need to check whether the
                // SectionFrm is also located within the Footnote and not outside of it.
                if( !pRet->IsInFtn() || pSct->IsInFtn() )
                {
                    OSL_ENSURE( pSct && pSct->GetSection(), "Where's my section?" );
                    SwSectionNode* pNd = pSct->GetSection()->GetFmt()->GetSectionNode();
                    OSL_ENSURE( pNd, "Lost SectionNode" );
                    // If the result Frame is located within a Section Frame
                    // whose Section does not contain the Node, we return with
                    // the SectionFrm, else we return with the Cntnt/TabFrm
                    if( bMaster )
                    {
                        if( pNd->GetIndex() >= nIndex )
                            pRet = pSct;
                    }
                    else if( pNd->EndOfSectionIndex() < nIndex )
                        pRet = pSct;
                }
            }
            return pRet;
        }
        pRet = pIter->Next();
    }
    return NULL;
}

void SwNode2LayImpl::SaveUpperFrms()
{
    pUpperFrms = new std::vector<SwFrm*>;
    SwFrm* pFrm;
    while( 0 != (pFrm = NextFrm()) )
    {
        SwFrm* pPrv = pFrm->GetPrev();
        pFrm = pFrm->GetUpper();
        if( pFrm )
        {
            if( pFrm->IsFtnFrm() )
                ((SwFtnFrm*)pFrm)->ColLock();
            else if( pFrm->IsInSct() )
                pFrm->FindSctFrm()->ColLock();
            if( pPrv && pPrv->IsSctFrm() )
                ((SwSectionFrm*)pPrv)->LockJoin();
            pUpperFrms->push_back( pPrv );
            pUpperFrms->push_back( pFrm );
        }
    }
    delete pIter;
    pIter = NULL;
    pMod = 0;
}

SwLayoutFrm* SwNode2LayImpl::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
{
    rpFrm = NextFrm();
    if( !rpFrm )
        return NULL;
    SwLayoutFrm* pUpper = rpFrm->GetUpper();
    if( rpFrm->IsSctFrm() )
    {
        const SwNode* pNode = rNode.StartOfSectionNode();
        if( pNode->IsSectionNode() )
        {
            SwFrm* pFrm = bMaster ? rpFrm->FindPrev() : rpFrm->FindNext();
            if( pFrm && pFrm->IsSctFrm() )
            {
                // pFrm could be a "dummy"-section
                if( ((SwSectionFrm*)pFrm)->GetSection() &&
                    (&((SwSectionNode*)pNode)->GetSection() ==
                     ((SwSectionFrm*)pFrm)->GetSection()) )
                {
                    // #i22922# - consider columned sections
                    // 'Go down' the section frame as long as the layout frame
                    // is found, which would contain content.
                    while ( pFrm->IsLayoutFrm() &&
                            static_cast<SwLayoutFrm*>(pFrm)->Lower() &&
                            !static_cast<SwLayoutFrm*>(pFrm)->Lower()->IsFlowFrm() &&
                            static_cast<SwLayoutFrm*>(pFrm)->Lower()->IsLayoutFrm() )
                    {
                        pFrm = static_cast<SwLayoutFrm*>(pFrm)->Lower();
                    }
                    OSL_ENSURE( pFrm->IsLayoutFrm(),
                            "<SwNode2LayImpl::UpperFrm(..)> - expected upper frame isn't a layout frame." );
                    rpFrm = bMaster ? NULL
                                    : static_cast<SwLayoutFrm*>(pFrm)->Lower();
                    OSL_ENSURE( !rpFrm || rpFrm->IsFlowFrm(),
                            "<SwNode2LayImpl::UpperFrm(..)> - expected sibling isn't a flow frame." );
                    return static_cast<SwLayoutFrm*>(pFrm);
                }

                pUpper = new SwSectionFrm(((SwSectionNode*)pNode)->GetSection(), rpFrm);
                pUpper->Paste( rpFrm->GetUpper(),
                               bMaster ? rpFrm : rpFrm->GetNext() );
                static_cast<SwSectionFrm*>(pUpper)->Init();
                rpFrm = NULL;
                // 'Go down' the section frame as long as the layout frame
                // is found, which would contain content.
                while ( pUpper->Lower() &&
                        !pUpper->Lower()->IsFlowFrm() &&
                        pUpper->Lower()->IsLayoutFrm() )
                {
                    pUpper = static_cast<SwLayoutFrm*>(pUpper->Lower());
                }
                return pUpper;
            }
        }
    }
    if( !bMaster )
        rpFrm = rpFrm->GetNext();
    return pUpper;
}

void SwNode2LayImpl::RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd )
{
    OSL_ENSURE( pUpperFrms, "RestoreUpper without SaveUpper?" );
    SwNode* pNd;
    SwDoc *pDoc = rNds.GetDoc();
    bool bFirst = true;
    for( ; nStt < nEnd; ++nStt )
    {
        SwFrm* pNew = 0;
        SwFrm* pNxt;
        SwLayoutFrm* pUp;
        if( (pNd = rNds[nStt])->IsCntntNode() )
            for( sal_uInt16 n = 0; n < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[n++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[n++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwCntntNode*)pNd)->MakeFrm( pUp );
                pNew->Paste( pUp, pNxt );
                (*pUpperFrms)[n-2] = pNew;
            }
        else if( pNd->IsTableNode() )
            for( sal_uInt16 x = 0; x < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwTableNode*)pNd)->MakeFrm( pUp );
                OSL_ENSURE( pNew->IsTabFrm(), "Table expected" );
                pNew->Paste( pUp, pNxt );
                ((SwTabFrm*)pNew)->RegistFlys();
                (*pUpperFrms)[x-2] = pNew;
            }
        else if( pNd->IsSectionNode() )
        {
            nStt = pNd->EndOfSectionIndex();
            for( sal_uInt16 x = 0; x < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                OSL_ENSURE( pUp->GetUpper() || pUp->IsFlyFrm(), "Lost Upper" );
                ::_InsertCnt( pUp, pDoc, pNd->GetIndex(), false, nStt+1, pNxt );
                pNxt = pUp->GetLastLower();
                (*pUpperFrms)[x-2] = pNxt;
            }
        }
        bFirst = false;
    }
    for( sal_uInt16 x = 0; x < pUpperFrms->size(); ++x )
    {
        SwFrm* pTmp = (*pUpperFrms)[++x];
        if( pTmp->IsFtnFrm() )
            ((SwFtnFrm*)pTmp)->ColUnlock();
        else if ( pTmp->IsInSct() )
        {
            SwSectionFrm* pSctFrm = pTmp->FindSctFrm();
            pSctFrm->ColUnlock();
            // #i18103# - invalidate size of section in order to
            // assure, that the section is formatted, unless it was 'Collocked'
            // from its 'collection' until its 'restoration'.
            pSctFrm->_InvalidateSize();
        }
    }
}

SwFrm* SwNode2LayImpl::GetFrm( const Point* pDocPos,
                                const SwPosition *pPos,
                                const bool bCalcFrm ) const
{
    // test if change of member pIter -> pMod broke anything
    return pMod ? ::GetFrmOfModify( 0, *pMod, USHRT_MAX, pDocPos, pPos, bCalcFrm ) : 0;
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd, sal_uLong nIdx )
{
    pImpl = new SwNode2LayImpl( rNd, nIdx, false );
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd )
{
    pImpl = new SwNode2LayImpl( rNd, rNd.GetIndex(), true );
    pImpl->SaveUpperFrms();
}

void SwNode2Layout::RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd )
{
    OSL_ENSURE( pImpl, "RestoreUpperFrms without SaveUpperFrms" );
    pImpl->RestoreUpperFrms( rNds, nStt, nEnd );
}

SwFrm* SwNode2Layout::NextFrm()
{
    return pImpl->NextFrm();
}

SwLayoutFrm* SwNode2Layout::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
{
    return pImpl->UpperFrm( rpFrm, rNode );
}

SwNode2Layout::~SwNode2Layout()
{
    delete pImpl;
}

SwFrm* SwNode2Layout::GetFrm( const Point* pDocPos,
                              const SwPosition *pPos,
                              const bool bCalcFrm ) const
{
    return pImpl->GetFrm( pDocPos, pPos, bCalcFrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
