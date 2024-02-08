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

#include <sortedobjs.hxx>

#include <algorithm>
#include <anchoredobject.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <svx/svdobj.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

SwSortedObjs::SwSortedObjs()
{
}

SwSortedObjs::~SwSortedObjs()
{
}

size_t SwSortedObjs::size() const
{
    return maSortedObjLst.size();
}

SwAnchoredObject* SwSortedObjs::operator[]( size_t _nIndex ) const
{
    SwAnchoredObject* pAnchoredObj = nullptr;

    if ( _nIndex >= size() )
    {
        OSL_FAIL( "<SwSortedObjs::operator[]> - index out of range" );
    }
    else
    {
        pAnchoredObj = maSortedObjLst[ _nIndex ];
    }

    return pAnchoredObj;
}

namespace
{
    int GetAnchorWeight(RndStdIds eAnchor)
    {
        if (eAnchor == RndStdIds::FLY_AT_CHAR)
            return 0;
        if (eAnchor == RndStdIds::FLY_AS_CHAR)
            return 1;
        return 2;
    }

struct ObjAnchorOrder
{
    bool operator()( const SwAnchoredObject* _pListedAnchoredObj,
                     const SwAnchoredObject* _pNewAnchoredObj )
    {
        // get attributes of listed object
        const SwFrameFormat* pFormatListed = _pListedAnchoredObj->GetFrameFormat();
        if (!pFormatListed)
            return false;
        const SwFormatAnchor* pAnchorListed = &(pFormatListed->GetAnchor());

        // get attributes of new object
        const SwFrameFormat* pFormatNew = _pNewAnchoredObj->GetFrameFormat();
        if (!pFormatNew)
            return false;
        const SwFormatAnchor* pAnchorNew = &(pFormatNew->GetAnchor());

        // check for to-page anchored objects
        if ((pAnchorListed->GetAnchorId() == RndStdIds::FLY_AT_PAGE) &&
            (pAnchorNew   ->GetAnchorId() != RndStdIds::FLY_AT_PAGE))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != RndStdIds::FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == RndStdIds::FLY_AT_PAGE))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == RndStdIds::FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == RndStdIds::FLY_AT_PAGE))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchored to page.
        // Thus, check for to-fly anchored objects
        if ((pAnchorListed->GetAnchorId() == RndStdIds::FLY_AT_FLY) &&
            (pAnchorNew   ->GetAnchorId() != RndStdIds::FLY_AT_FLY))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != RndStdIds::FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == RndStdIds::FLY_AT_FLY))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == RndStdIds::FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == RndStdIds::FLY_AT_FLY))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchor to page or to fly
        // Thus, compare content anchor nodes, if existing.
        const SwNode* pContentAnchorListed = pAnchorListed->GetAnchorNode();
        const SwNode* pContentAnchorNew = pAnchorNew->GetAnchorNode();
        if ( pContentAnchorListed && pContentAnchorNew &&
             *pContentAnchorListed != *pContentAnchorNew )
        {
            return *pContentAnchorListed < *pContentAnchorNew;
        }

        // objects anchored at the same content.
        // --> OD 2006-11-29 #???# - objects have to be ordered by anchor node position
        // Thus, compare content anchor node positions and anchor type,
        // if not anchored at-paragraph
        if (pContentAnchorListed && pContentAnchorNew)
        {
            sal_Int32 nListedIndex = pAnchorListed->GetAnchorId() != RndStdIds::FLY_AT_PARA ?
                pAnchorListed->GetAnchorContentOffset() : 0;
            sal_Int32 nNewIndex = pAnchorNew->GetAnchorId() != RndStdIds::FLY_AT_PARA ?
                pAnchorNew->GetAnchorContentOffset() : 0;
            if (nListedIndex != nNewIndex)
            {
                return nListedIndex < nNewIndex;
            }
        }

        int nAnchorListedWeight = GetAnchorWeight(pAnchorListed->GetAnchorId());
        int nAnchorNewWeight = GetAnchorWeight(pAnchorNew->GetAnchorId());
        if (nAnchorListedWeight != nAnchorNewWeight)
        {
            return nAnchorListedWeight < nAnchorNewWeight;
        }

        // objects anchored at the same content and at the same content anchor
        // node position with the same anchor type
        // Thus, compare its wrapping style including its layer
        const IDocumentDrawModelAccess& rIDDMA = pFormatListed->getIDocumentDrawModelAccess();
        const SdrLayerID nHellId = rIDDMA.GetHellId();
        const SdrLayerID nInvisibleHellId = rIDDMA.GetInvisibleHellId();
        const bool bWrapThroughOrHellListed =
                    pFormatListed->GetSurround().GetSurround() == css::text::WrapTextMode_THROUGH ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        const bool bWrapThroughOrHellNew =
                    pFormatNew->GetSurround().GetSurround() == css::text::WrapTextMode_THROUGH ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        if ( bWrapThroughOrHellListed != bWrapThroughOrHellNew )
        {
            return !bWrapThroughOrHellListed;
        }
        else if ( bWrapThroughOrHellListed && bWrapThroughOrHellNew )
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // objects anchored at the same content with a set text wrapping
        // Thus, compare wrap influences on object position
        const SwFormatWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosListed =
                                        &(pFormatListed->GetWrapInfluenceOnObjPos());
        const SwFormatWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosNew =
                                        &(pFormatNew->GetWrapInfluenceOnObjPos());
        // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
        if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true ) !=
                pWrapInfluenceOnObjPosNew->GetWrapInfluenceOnObjPos( true ) )
        {
            // #i35017# - constant name has changed
            return pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true )
                            == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE;
        }

        // objects anchored at the same content position/page/fly with same
        // wrap influence.
        // Thus, compare anchor order number
        return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
    }
};

}

bool SwSortedObjs::is_sorted() const
{
    return std::is_sorted(maSortedObjLst.begin(), maSortedObjLst.end(), ObjAnchorOrder());
}

bool SwSortedObjs::Insert( SwAnchoredObject& _rAnchoredObj )
{
    if (!is_sorted())
    {
        SAL_WARN("sw.core", "SwSortedObjs::Insert: object list is not sorted");
        UpdateAll();
    }

    // #i51941#
    if ( Contains( _rAnchoredObj ) )
    {
        // list already contains object
        OSL_FAIL( "<SwSortedObjs::Insert()> - already contains object" );
        return true;
    }

    // find insert position
    std::vector< SwAnchoredObject* >::iterator aInsPosIter =
        std::lower_bound( maSortedObjLst.begin(), maSortedObjLst.end(),
                          &_rAnchoredObj, ObjAnchorOrder() );

    // insert object into list
    maSortedObjLst.insert( aInsPosIter, &_rAnchoredObj );

    return Contains( _rAnchoredObj );
}

void SwSortedObjs::Remove( SwAnchoredObject& _rAnchoredObj )
{
    std::vector< SwAnchoredObject* >::iterator aDelPosIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aDelPosIter == maSortedObjLst.end() )
    {
        // object not found.
        OSL_FAIL( "<SwSortedObjs::Remove()> - object not found" );
    }
    else
    {
        maSortedObjLst.erase( aDelPosIter );
    }
}

bool SwSortedObjs::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    return aIter != maSortedObjLst.end();
}

void SwSortedObjs::Update( SwAnchoredObject& _rAnchoredObj )
{
    if ( !Contains( _rAnchoredObj ) )
    {
        // given anchored object not found in list
        OSL_FAIL( "<SwSortedObjs::Update(..) - sorted list doesn't contain given anchored object" );
        return;
    }

    if ( size() == 1 )
    {
        // given anchored object is the only one in the list.
        return;
    }

    Remove( _rAnchoredObj );
    Insert( _rAnchoredObj );
}

void SwSortedObjs::UpdateAll()
{
    std::stable_sort(maSortedObjLst.begin(), maSortedObjLst.end(), ObjAnchorOrder());
}

size_t SwSortedObjs::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aIter != maSortedObjLst.end() )
    {
        // #i51941#
        std::vector< SwAnchoredObject* >::difference_type nPos =
                                                aIter - maSortedObjLst.begin();
        return static_cast<size_t>( nPos );
    }

    return size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
