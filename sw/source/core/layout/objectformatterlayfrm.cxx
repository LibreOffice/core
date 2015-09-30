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

#include <objectformatterlayfrm.hxx>
#include <anchoredobject.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>

#include <layact.hxx>

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
        OSL_FAIL( "<SwObjectFormatterLayFrm::CreateObjFormatter(..)> - unexpected type of anchor frame " );
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

// #i40147# - add parameter <_bCheckForMovedFwd>.
// Not relevant for objects anchored at layout frame.
bool SwObjectFormatterLayFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool )
{
    _FormatObj( _rAnchoredObj );

    // #124218# - consider that the layout action has to be
    // restarted due to a deleted page frame.
    return GetLayAction() == nullptr || !GetLayAction()->IsAgain();
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
*/
bool SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()
{
    if ( !GetAnchorFrm().IsPageFrm() )
    {
        OSL_FAIL( "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - mis-usage of method, call only for anchor frames of type page frame" );
        return true;
    }

    // #124218# - consider, if the layout action
    // has to be restarted due to a delete of a page frame.
    if ( GetLayAction() && GetLayAction()->IsAgain() )
    {
        return false;
    }

    SwPageFrm& rPageFrm = static_cast<SwPageFrm&>(GetAnchorFrm());

    if ( !rPageFrm.GetSortedObjs() )
    {
        // nothing to do, if no floating screen object is registered at the anchor frame.
        return true;
    }

    bool bSuccess( true );

    for ( size_t i = 0; i < rPageFrm.GetSortedObjs()->size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*rPageFrm.GetSortedObjs())[i];

        // #i51941# - do not format object, which are anchored
        // inside or at fly frame.
        if ( pAnchoredObj->GetAnchorFrm()->FindFlyFrm() )
        {
            continue;
        }
        // #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
        // is replaced by method <FindPageFrmOfAnchor()>. It's return value
        // have to be checked.
        SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
        // #i26945# - check, if the page frame of the
        // object's anchor frame isn't the given page frame
        OSL_ENSURE( pPageFrmOfAnchor,
                "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - missing page frame" );
        if ( pPageFrmOfAnchor &&
             // #i35911#
             pPageFrmOfAnchor->GetPhyPageNum() < rPageFrm.GetPhyPageNum() )
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
                 i > rPageFrm.GetSortedObjs()->size() )
            {
                break;
            }
            else
            {
                const size_t nActPosOfObj =
                    rPageFrm.GetSortedObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == rPageFrm.GetSortedObjs()->size() ||
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
