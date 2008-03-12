/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sortedobjsimpl.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:22:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef _SORTEDOBJSIMPL_HXX
#include <sortedobjsimpl.hxx>
#endif

#include <algorithm>

#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif
#ifndef IDOCUMENTDRAWMODELACCESS_HXX_INCLUDED
#include <IDocumentDrawModelAccess.hxx>
#endif

using namespace ::com::sun::star;

typedef std::vector< SwAnchoredObject* >::iterator tIter;
typedef std::vector< SwAnchoredObject* >::const_iterator tConstIter;

using namespace ::com::sun::star;


SwSortedObjsImpl::SwSortedObjsImpl()
{
}

SwSortedObjsImpl::~SwSortedObjsImpl()
{
}

sal_uInt32 SwSortedObjsImpl::Count() const
{
    return maSortedObjLst.size();
}

SwAnchoredObject* SwSortedObjsImpl::operator[]( sal_uInt32 _nIndex )
{
    SwAnchoredObject* pAnchoredObj = 0L;

    if ( _nIndex >= Count() )
    {
        ASSERT( false, "<SwSortedObjsImpl::operator[]> - index out of range" );
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
        // get attributes of listed object
        const SwFrmFmt& rFmtListed = _pListedAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorListed = &(rFmtListed.GetAnchor());

        // get attributes of new object
        const SwFrmFmt& rFmtNew = _pNewAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorNew = &(rFmtNew.GetAnchor());

        // check for to-page anchored objects
        if ( pAnchorListed->GetAnchorId() == FLY_PAGE &&
             pAnchorNew->GetAnchorId() != FLY_PAGE )
        {
            return true;
        }
        else if ( pAnchorListed->GetAnchorId() != FLY_PAGE &&
                  pAnchorNew->GetAnchorId() == FLY_PAGE )
        {
            return false;
        }
        else if ( pAnchorListed->GetAnchorId() == FLY_PAGE &&
                  pAnchorNew->GetAnchorId() == FLY_PAGE )
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchored to page.
        // Thus, check for to-fly anchored objects
        if ( pAnchorListed->GetAnchorId() == FLY_AT_FLY &&
             pAnchorNew->GetAnchorId() != FLY_AT_FLY )
        {
            return true;
        }
        else if ( pAnchorListed->GetAnchorId() != FLY_AT_FLY &&
                  pAnchorNew->GetAnchorId() == FLY_AT_FLY )
        {
            return false;
        }
        else if ( pAnchorListed->GetAnchorId() == FLY_AT_FLY &&
                  pAnchorNew->GetAnchorId() == FLY_AT_FLY )
        {
            return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
        }

        // Both objects aren't anchor to page or to fly
        // Thus, compare content anchor nodes, if existing.
        const SwPosition* pCntntAnchorListed = pAnchorListed->GetCntntAnchor();
        const SwPosition* pCntntAnchorNew = pAnchorNew->GetCntntAnchor();
        if ( pCntntAnchorListed && pCntntAnchorNew &&
             pCntntAnchorListed->nNode != pCntntAnchorNew->nNode )
        {
            return pCntntAnchorListed->nNode < pCntntAnchorNew->nNode;
        }

        // objects anchored at the same content.
        // --> OD 2006-11-29 #???# - objects have to be ordered by anchor node position
        // Thus, compare content anchor node positions and anchor type,
        // if not anchored at-paragraph
        if ( pAnchorListed->GetAnchorId() != FLY_AT_CNTNT &&
             pAnchorNew->GetAnchorId() != FLY_AT_CNTNT &&
             pCntntAnchorListed && pCntntAnchorNew )
        {
            if ( pCntntAnchorListed->nContent != pCntntAnchorNew->nContent )
            {
                return pCntntAnchorListed->nContent < pCntntAnchorNew->nContent;
            }
            else if ( pAnchorListed->GetAnchorId() == FLY_AUTO_CNTNT &&
                      pAnchorNew->GetAnchorId() == FLY_IN_CNTNT )
            {
                return true;
            }
            else if ( pAnchorListed->GetAnchorId() == FLY_IN_CNTNT &&
                      pAnchorNew->GetAnchorId() == FLY_AUTO_CNTNT )
            {
                return false;
            }
        }
        // <--

        // objects anchored at the same content and at the same content anchor
        // node position with the same anchor type
        // Thus, compare its wrapping style including its layer
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

        // objects anchored at the same content with a set text wrapping
        // Thus, compare wrap influences on object position
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosListed =
                                        &(rFmtListed.GetWrapInfluenceOnObjPos());
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosNew =
                                        &(rFmtNew.GetWrapInfluenceOnObjPos());
        // --> OD 2004-10-18 #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
        if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true ) !=
                pWrapInfluenceOnObjPosNew->GetWrapInfluenceOnObjPos( true ) )
        // <--
        {
            // --> OD 2004-10-18 #i35017# - constant name has changed
            if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos( true )
                            == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE )
            // <--
                return true;
            else
                return false;
        }

        // objects anchored at the same content position/page/fly with same
        // wrap influence.
        // Thus, compare anchor order number
        return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
    }
};

bool SwSortedObjsImpl::Insert( SwAnchoredObject& _rAnchoredObj )
{
    // --> OD 2005-08-18 #i51941#
    if ( Contains( _rAnchoredObj ) )
    {
        // list already contains object
#if OSL_DEBUG_LEVEL > 1
        ASSERT( false,
                "<SwSortedObjsImpl::Insert()> - already contains object" );
#endif
        return true;
    }

    // find insert position
    tIter aInsPosIter = std::lower_bound( maSortedObjLst.begin(),
                                          maSortedObjLst.end(),
                                          &_rAnchoredObj, ObjAnchorOrder() );

    // insert object into list
    maSortedObjLst.insert( aInsPosIter, &_rAnchoredObj );

    return Contains( _rAnchoredObj );
}

bool SwSortedObjsImpl::Remove( SwAnchoredObject& _rAnchoredObj )
{
    bool bRet = true;

    tIter aDelPosIter = std::find( maSortedObjLst.begin(),
                                   maSortedObjLst.end(),
                                   &_rAnchoredObj );

    if ( aDelPosIter == maSortedObjLst.end() )
    {
        // object not found.
        bRet = false;
#if OSL_DEBUG_LEVEL > 1
        ASSERT( false,
                "<SwSortedObjsImpl::Remove()> - object not found" );
#endif
    }
    else
    {
        maSortedObjLst.erase( aDelPosIter );
    }

    return bRet;
}

bool SwSortedObjsImpl::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    tConstIter aIter = std::find( maSortedObjLst.begin(), maSortedObjLst.end(),
                                  &_rAnchoredObj );

    return aIter != maSortedObjLst.end();
}

bool SwSortedObjsImpl::Update( SwAnchoredObject& _rAnchoredObj )
{
    if ( !Contains( _rAnchoredObj ) )
    {
        // given anchored object not found in list
        ASSERT( false,
                "<SwSortedObjsImpl::Update(..) - sorted list doesn't contain given anchored object" );
        return false;
    }

    if ( Count() == 1 )
    {
        // given anchored object is the only one in the list.
        return true;
    }

    Remove( _rAnchoredObj );
    Insert( _rAnchoredObj );

    return Contains( _rAnchoredObj );
}

sal_uInt32 SwSortedObjsImpl::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    sal_uInt32 nRetLstPos = Count();

    tConstIter aIter = std::find( maSortedObjLst.begin(), maSortedObjLst.end(),
                                  &_rAnchoredObj );

    if ( aIter != maSortedObjLst.end() )
    {
        // --> OD 2005-08-18 #i51941#
//        nRetLstPos = aIter - maSortedObjLst.begin();
        std::vector< SwAnchoredObject* >::difference_type nPos =
                                                aIter - maSortedObjLst.begin();
        nRetLstPos = sal_uInt32( nPos );
        // <--
    }

    return nRetLstPos;
}

