/*************************************************************************
 *
 *  $RCSfile: sortedobjsimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:13:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

typedef std::vector< SwAnchoredObject* >::iterator tIter;
typedef std::vector< SwAnchoredObject* >::const_iterator tConstIter;

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
        SdrLayerID nHellId = rFmtListed.GetDoc()->GetHellId();
        SdrLayerID nInvisibleHellId = rFmtListed.GetDoc()->GetInvisibleHellId();
        const SwFmtAnchor* pAnchorListed = &(rFmtListed.GetAnchor());
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosListed =
                                        &(rFmtListed.GetWrapInfluenceOnObjPos());
        const bool bWrapThroughOrHellListed =
                    rFmtListed.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pListedAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;

        // get attributes of new object
        const SwFrmFmt& rFmtNew = _pNewAnchoredObj->GetFrmFmt();
        const SwFmtAnchor* pAnchorNew = &(rFmtNew.GetAnchor());
        const SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPosNew =
                                        &(rFmtNew.GetWrapInfluenceOnObjPos());
        const bool bWrapThroughOrHellNew =
                    rFmtNew.GetSurround().GetSurround() == SURROUND_THROUGHT ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nHellId ||
                    _pNewAnchoredObj->GetDrawObj()->GetLayer() == nInvisibleHellId;

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
        // Thus, compare its wrapping style including its layer
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
        if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos() !=
                pWrapInfluenceOnObjPosNew->GetWrapInfluenceOnObjPos() )
        {
            if ( pWrapInfluenceOnObjPosListed->GetWrapInfluenceOnObjPos()
                    == text::WrapInfluenceOnPosition::NONE_SUCCESSIVE_POSITIONED )
                return true;
            else
                return false;
        }

        // objects anchored at the same content with same wrap influence.
        // Thus, compare anchor types
        if ( pAnchorListed->GetAnchorId() != pAnchorNew->GetAnchorId() )
        {
            // order of anchor types: FLY_PAGE = 1, FLY_AT_FLY = 2, FLY_AT_CNTNT = 3,
            // FLY_AUTO_CNTNT = 4 and FLY_IN_CNTNT = 5
            static int aAnchorTypeOrder[5] = { 3, 5, 1, 2, 4 };

            return aAnchorTypeOrder[pAnchorListed->GetAnchorId()] <
                                    aAnchorTypeOrder[pAnchorNew->GetAnchorId()];
        }

        // objects anchored at the same content with same wrap influence and
        // same anchor type.
        // Thus, compare content anchor node positions, if existing
        if ( pCntntAnchorListed && pCntntAnchorNew &&
             pCntntAnchorListed->nContent != pCntntAnchorNew->nContent )
        {
            return pCntntAnchorListed->nContent < pCntntAnchorNew->nContent;
        }

        // objects anchored at the same content position/page/fly with same
        // wrap influence.
        // Thus, compare anchor order number
        return pAnchorListed->GetOrder() < pAnchorNew->GetOrder();
    }
};

bool SwSortedObjsImpl::Insert( SwAnchoredObject& _rAnchoredObj )
{
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
        ASSERT( false,
                "<SwSortedObjsImpl::Remove()> - object not found" );
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
        nRetLstPos = aIter - maSortedObjLst.begin();
    }

    return nRetLstPos;
}

sal_uInt32 SwSortedObjsImpl::IndexOfObjAnchoredAfterFrm( const SwTxtFrm& _rTxtFrm ) const
{
    sal_uInt32 nRetIdx = Count();

    SwNodeIndex pTxtNodeIdx( *_rTxtFrm.GetTxtNode() );

    tConstIter aIter = maSortedObjLst.begin();
    for ( ; aIter != maSortedObjLst.end(); ++aIter )
    {
        SwAnchoredObject* pAnchoredObj = *(aIter);

        const SwFmtAnchor* pAnchor = &(pAnchoredObj->GetFrmFmt().GetAnchor());

        if ( ( pAnchor->GetAnchorId() == FLY_AT_CNTNT ||
               pAnchor->GetAnchorId() == FLY_AUTO_CNTNT ) &&
             pAnchor->GetCntntAnchor()->nNode > pTxtNodeIdx )
        {
            nRetIdx = aIter - maSortedObjLst.begin();
            break;
        }
    }

    return nRetIdx;
}

