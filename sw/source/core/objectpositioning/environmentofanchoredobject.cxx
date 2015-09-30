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

#include <environmentofanchoredobject.hxx>
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

/** determine environment layout frame for possible horizontal object positions */
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(
                                            const SwFrm& _rHoriOrientFrm ) const
{
    const SwFrm* pHoriEnvironmentLayFrm = &_rHoriOrientFrm;

    if ( !mbFollowTextFlow )
    {
        // No exception any more for page alignment.
        // the page frame determines the horizontal layout environment.
        pHoriEnvironmentLayFrm = _rHoriOrientFrm.FindPageFrm();
    }
    else
    {
        while ( !pHoriEnvironmentLayFrm->IsCellFrm() &&
                !pHoriEnvironmentLayFrm->IsFlyFrm() &&
                !pHoriEnvironmentLayFrm->IsPageFrm() )
        {
            pHoriEnvironmentLayFrm = pHoriEnvironmentLayFrm->GetUpper();
            OSL_ENSURE( pHoriEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - no page|fly|cell frame found" );
        }
    }

    OSL_ENSURE( dynamic_cast< const SwLayoutFrm *>( pHoriEnvironmentLayFrm ) !=  nullptr,
                "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pHoriEnvironmentLayFrm);
}

/** determine environment layout frame for possible vertical object positions */
const SwLayoutFrm& SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(
                                            const SwFrm& _rVertOrientFrm ) const
{
    const SwFrm* pVertEnvironmentLayFrm = &_rVertOrientFrm;

    if ( !mbFollowTextFlow )
    {
        // No exception any more for page alignment.
        // the page frame determines the vertical layout environment.
        pVertEnvironmentLayFrm = _rVertOrientFrm.FindPageFrm();
    }
    else
    {
        while ( !pVertEnvironmentLayFrm->IsCellFrm() &&
                !pVertEnvironmentLayFrm->IsFlyFrm() &&
                !pVertEnvironmentLayFrm->IsHeaderFrm() &&
                !pVertEnvironmentLayFrm->IsFooterFrm() &&
                !pVertEnvironmentLayFrm->IsFootnoteFrm() &&
                !pVertEnvironmentLayFrm->IsPageBodyFrm() &&
                !pVertEnvironmentLayFrm->IsPageFrm() )
        {
            pVertEnvironmentLayFrm = pVertEnvironmentLayFrm->GetUpper();
            OSL_ENSURE( pVertEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - proposed frame not found" );
        }
    }

    OSL_ENSURE( dynamic_cast< const SwLayoutFrm *>( pVertEnvironmentLayFrm ) !=  nullptr,
                "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pVertEnvironmentLayFrm);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
