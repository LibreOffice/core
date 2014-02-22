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

using namespace ::com::sun::star;

SwSortedObjs::SwSortedObjs()
{
}

SwSortedObjs::~SwSortedObjs()
{
}

sal_uInt32 SwSortedObjs::Count() const
{
    return maSortedObjLst.size();
}

SwAnchoredObject* SwSortedObjs::operator[]( sal_uInt32 _nIndex ) const
{
    SwAnchoredObject* pAnchoredObj = 0L;

    if ( _nIndex >= Count() )
    {
        OSL_FAIL( "<SwSortedObjs::operator[]> - index out of range" );
    }
    else
    {
        pAnchoredObj = maSortedObjLst[ _nIndex ];
    }

    return pAnchoredObj;
}

struct ObjAnchorOrder
{
    bool operator()( const SwAnchoredObject* _pListedAnchoredObj,
                     const SwAnchoredObject* _pNewAnchoredObj )
    {
        
        const SwFrmFmt& rFmtListed = _pListedAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorListed = &(rFmtListed.GetAnchor());

        
        const SwFrmFmt& rFmtNew = _pNewAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorNew = &(rFmtNew.GetAnchor());

        
        if ((pAnchorListed->GetAnchorId() == FLY_AT_PAGE) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_PAGE))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_PAGE))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == FLY_AT_PAGE) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_PAGE))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        
        
        if ((pAnchorListed->GetAnchorId() == FLY_AT_FLY) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_FLY))
        {
            return true;
        }
        else if ((pAnchorListed->GetAnchorId() != FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_FLY))
        {
            return false;
        }
        else if ((pAnchorListed->GetAnchorId() == FLY_AT_FLY) &&
                 (pAnchorNew   ->GetAnchorId() == FLY_AT_FLY))
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        
        
        const SwPosition* pCntntAnchorListed = pAnchorListed->GetCntntAnchor();
        const SwPosition* pCntntAnchorNew = pAnchorNew->GetCntntAnchor();
        if ( pCntntAnchorListed && pCntntAnchorNew &&
             pCntntAnchorListed->nNode != pCntntAnchorNew->nNode )
        {
            return pCntntAnchorListed->nNode < pCntntAnchorNew->nNode;
        }

        
        
        
        
        if ((pAnchorListed->GetAnchorId() != FLY_AT_PARA) &&
            (pAnchorNew   ->GetAnchorId() != FLY_AT_PARA) &&
             pCntntAnchorListed && pCntntAnchorNew )
        {
            if ( pCntntAnchorListed->nContent != pCntntAnchorNew->nContent )
            {
                return pCntntAnchorListed->nContent < pCntntAnchorNew->nContent;
            }
            else if ((pAnchorListed->GetAnchorId() == FLY_AT_CHAR) &&
                     (pAnchorNew   ->GetAnchorId() == FLY_AS_CHAR))
            {
                return true;
            }
            else if ((pAnchorListed->GetAnchorId() == FLY_AS_CHAR) &&
                     (pAnchorNew   ->GetAnchorId() == FLY_AT_CHAR))
            {
                return false;
            }
        }

        
        
        
        const IDocumentDrawModelAccess* pIDDMA = rFmtListed.getIDocumentDrawModelAccess();
        const SdrLayerID nHellId = pIDDMA->GetHellId();
        const SdrLayerID nInvisibleHellId = pIDDMA->GetInvisibleHellId();
        const bool bWrapThroughOrHellListed =
                    rFmtListed.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        const bool bWrapThroughOrHellNew =
                    rFmtNew.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;
        if ( bWrapThroughOrHellListed != bWrapThroughOrHellNew )
        {
            if ( bWrapThroughOrHellListed )
                return false;
            else
                return true;
        }
        else if ( bWrapThroughOrHellListed && bWrapThroughOrHellNew )
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        
        
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosListed =
                                        &(rFmtListed.GetWrapInfluenceOnObjPos());
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosNew =
                                        &(rFmtNew.GetWrapInfluenceOnObjPos());
        
        if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true ) !=
                pWrapInfluenceOnObjPosNew->GetWrapInfluenceOnObjPos( true ) )
        {
            
            if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true )
                            == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE )
                return true;
            else
                return false;
        }

        
        
        
        return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
    }
};

bool SwSortedObjs::Insert( SwAnchoredObject& _rAnchoredObj )
{
    
    if ( Contains( _rAnchoredObj ) )
    {
        
        OSL_FAIL( "<SwSortedObjs::Insert()> - already contains object" );
        return true;
    }

    
    std::vector< SwAnchoredObject* >::iterator aInsPosIter =
        std::lower_bound( maSortedObjLst.begin(), maSortedObjLst.end(),
                          &_rAnchoredObj, ObjAnchorOrder() );

    
    maSortedObjLst.insert( aInsPosIter, &_rAnchoredObj );

    return Contains( _rAnchoredObj );
}

bool SwSortedObjs::Remove( SwAnchoredObject& _rAnchoredObj )
{
    bool bRet = true;

    std::vector< SwAnchoredObject* >::iterator aDelPosIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aDelPosIter == maSortedObjLst.end() )
    {
        
        bRet = false;
        OSL_FAIL( "<SwSortedObjs::Remove()> - object not found" );
    }
    else
    {
        maSortedObjLst.erase( aDelPosIter );
    }

    return bRet;
}

bool SwSortedObjs::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    return aIter != maSortedObjLst.end();
}

bool SwSortedObjs::Update( SwAnchoredObject& _rAnchoredObj )
{
    if ( !Contains( _rAnchoredObj ) )
    {
        
        OSL_FAIL( "<SwSortedObjs::Update(..) - sorted list doesn't contain given anchored object" );
        return false;
    }

    if ( Count() == 1 )
    {
        
        return true;
    }

    Remove( _rAnchoredObj );
    Insert( _rAnchoredObj );

    return Contains( _rAnchoredObj );
}

sal_uInt32 SwSortedObjs::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    sal_uInt32 nRetLstPos = Count();

    std::vector< SwAnchoredObject* >::const_iterator aIter =
        std::find( maSortedObjLst.begin(), maSortedObjLst.end(), &_rAnchoredObj );

    if ( aIter != maSortedObjLst.end() )
    {
        
        std::vector< SwAnchoredObject* >::difference_type nPos =
                                                aIter - maSortedObjLst.begin();
        nRetLstPos = sal_uInt32( nPos );
    }

    return nRetLstPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
