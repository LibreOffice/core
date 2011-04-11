/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <objectformatterlayfrm.hxx>
#include <anchoredobject.hxx>
#include <sortedobjs.hxx>
#include <layfrm.hxx>
#include <pagefrm.hxx>

// --> OD 2005-07-13 #124218#
#include <layact.hxx>
// <--

// =============================================================================
// implementation of class <SwObjectFormatterLayFrm>
// =============================================================================
SwObjectFormatterLayFrm::SwObjectFormatterLayFrm( SwLayoutFrm& _rAnchorLayFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction ),
      mrAnchorLayFrm( _rAnchorLayFrm )
{
}

SwObjectFormatterLayFrm::~SwObjectFormatterLayFrm()
{
}

SwObjectFormatterLayFrm* SwObjectFormatterLayFrm::CreateObjFormatter(
                                                SwLayoutFrm& _rAnchorLayFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    if ( !_rAnchorLayFrm.IsPageFrm() &&
         !_rAnchorLayFrm.IsFlyFrm() )
    {
        OSL_FAIL( "<SwObjectFormatterLayFrm::CreateObjFormatter(..)> - unexcepted type of anchor frame " );
        return 0L;
    }

    SwObjectFormatterLayFrm* pObjFormatter = 0L;

    // create object formatter, if floating screen objects are registered at
    // given anchor layout frame.
    if ( _rAnchorLayFrm.GetDrawObjs() ||
         ( _rAnchorLayFrm.IsPageFrm() &&
            static_cast<SwPageFrm&>(_rAnchorLayFrm).GetSortedObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterLayFrm( _rAnchorLayFrm, _rPageFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterLayFrm::GetAnchorFrm()
{
    return mrAnchorLayFrm;
}

// --> OD 2005-01-10 #i40147# - add parameter <_bCheckForMovedFwd>.
// Not relevant for objects anchored at layout frame.
bool SwObjectFormatterLayFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool )
// <--
{
    _FormatObj( _rAnchoredObj );

    // --> OD 2005-07-13 #124218# - consider that the layout action has to be
    // restarted due to a deleted page frame.
    return GetLayAction() ? !GetLayAction()->IsAgain() : true;
    // <--
}

bool SwObjectFormatterLayFrm::DoFormatObjs()
{
    bool bSuccess( true );

    bSuccess = _FormatObjsAtFrm();

    if ( bSuccess && GetAnchorFrm().IsPageFrm() )
    {
        // anchor layout frame is a page frame.
        // Thus, format also all anchored objects, which are registered at
        // this page frame, whose 'anchor' isn't on this page frame and whose
        // anchor frame is valid.
        bSuccess = _AdditionalFormatObjsOnPage();
    }

    return bSuccess;
}

/** method to format all anchored objects, which are registered at
    the page frame, whose 'anchor' isn't on this page frame and whose
    anchor frame is valid.

    OD 2004-07-02 #i28701#

    @author
*/
bool SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()
{
    if ( !GetAnchorFrm().IsPageFrm() )
    {
        OSL_FAIL( "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - mis-usage of method, call only for anchor frames of type page frame" );
        return true;
    }

    // --> OD 2005-07-13 #124218# - consider, if the layout action
    // has to be restarted due to a delete of a page frame.
    if ( GetLayAction() && GetLayAction()->IsAgain() )
    {
        return false;
    }
    // <--


    SwPageFrm& rPageFrm = static_cast<SwPageFrm&>(GetAnchorFrm());

    if ( !rPageFrm.GetSortedObjs() )
    {
        // nothing to do, if no floating screen object is registered at the anchor frame.
        return true;
    }

    bool bSuccess( true );

    sal_uInt32 i = 0;
    for ( ; i < rPageFrm.GetSortedObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*rPageFrm.GetSortedObjs())[i];

        // --> OD 2005-08-18 #i51941# - do not format object, which are anchored
        // inside or at fly frame.
        if ( pAnchoredObj->GetAnchorFrm()->FindFlyFrm() )
        {
            continue;
        }
        // <--
        // --> OD 2004-09-23 #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
        // is replaced by method <FindPageFrmOfAnchor()>. It's return value
        // have to be checked.
        SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
        // --> OD 2004-10-08 #i26945# - check, if the page frame of the
        // object's anchor frame isn't the given page frame
        OSL_ENSURE( pPageFrmOfAnchor,
                "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - missing page frame" );
        if ( pPageFrmOfAnchor &&
             // --> OD 2004-10-22 #i35911#
             pPageFrmOfAnchor->GetPhyPageNum() < rPageFrm.GetPhyPageNum() )
             // <--
        // <--
        {
            // if format of object fails, stop formatting and pass fail to
            // calling method via the return value.
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            // considering changes at <GetAnchorFrm().GetDrawObjs()> during
            // format of the object.
            if ( !rPageFrm.GetSortedObjs() ||
                 i > rPageFrm.GetSortedObjs()->Count() )
            {
                break;
            }
            else
            {
                sal_uInt32 nActPosOfObj =
                    rPageFrm.GetSortedObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == rPageFrm.GetSortedObjs()->Count() ||
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
    } // end of loop on <rPageFrm.GetSortedObjs()>

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
