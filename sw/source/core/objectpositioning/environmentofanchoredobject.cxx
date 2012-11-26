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
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif
#include <frame.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>

using namespace objectpositioning;

SwEnvironmentOfAnchoredObject::SwEnvironmentOfAnchoredObject(
                                                const bool   _bFollowTextFlow )
    : mbFollowTextFlow( _bFollowTextFlow )
{}

SwEnvironmentOfAnchoredObject::~SwEnvironmentOfAnchoredObject()
{}

/** determine environment layout frame for possible horizontal object positions

    OD 05.11.2003

    @author OD
*/
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(
                                            const SwFrm& _rHoriOrientFrm ) const
{
    const SwFrm* pHoriEnvironmentLayFrm = &_rHoriOrientFrm;

    if ( !mbFollowTextFlow )
    {
        // --> OD 2005-01-20 #118546# - no exception any more for page alignment.
        // the page frame determines the horizontal layout environment.
        pHoriEnvironmentLayFrm = _rHoriOrientFrm.FindPageFrm();
        // <--
    }
    else
    {
        while ( !pHoriEnvironmentLayFrm->IsCellFrm() &&
                !pHoriEnvironmentLayFrm->IsFlyFrm() &&
                !pHoriEnvironmentLayFrm->IsPageFrm() )
        {
            pHoriEnvironmentLayFrm = pHoriEnvironmentLayFrm->GetUpper();
            ASSERT( pHoriEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - no page|fly|cell frame found" );
        }
    }

    ASSERT( dynamic_cast< const SwLayoutFrm* >(pHoriEnvironmentLayFrm),
                "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pHoriEnvironmentLayFrm);
}

/** determine environment layout frame for possible vertical object positions

    OD 05.11.2003

    @author OD
*/
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(
                                            const SwFrm& _rVertOrientFrm ) const
{
    const SwFrm* pVertEnvironmentLayFrm = &_rVertOrientFrm;

    if ( !mbFollowTextFlow )
    {
        // --> OD 2005-01-20 #118546# - no exception any more for page alignment.
        // the page frame determines the vertical layout environment.
        pVertEnvironmentLayFrm = _rVertOrientFrm.FindPageFrm();
        // <--
    }
    else
    {
        while ( !pVertEnvironmentLayFrm->IsCellFrm() &&
                !pVertEnvironmentLayFrm->IsFlyFrm() &&
                !pVertEnvironmentLayFrm->IsHeaderFrm() &&
                !pVertEnvironmentLayFrm->IsFooterFrm() &&
                !pVertEnvironmentLayFrm->IsFtnFrm() &&
                !pVertEnvironmentLayFrm->IsPageBodyFrm() &&
                !pVertEnvironmentLayFrm->IsPageFrm() )
        {
            pVertEnvironmentLayFrm = pVertEnvironmentLayFrm->GetUpper();
            ASSERT( pVertEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - proposed frame not found" );
        }
    }

    ASSERT( dynamic_cast< const SwLayoutFrm* >(pVertEnvironmentLayFrm),
                "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pVertEnvironmentLayFrm);
}
