/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        ASSERT( false,
                "<SwObjectFormatterLayFrm::CreateObjFormatter(..)> - unexcepted type of anchor frame " );
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
        ASSERT( false,
                "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - mis-usage of method, call only for anchor frames of type page frame" );
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
        ASSERT( pPageFrmOfAnchor,
                "<SwObjectFormatterLayFrm::_AdditionalFormatObjsOnPage()> - missing page frame" )
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
