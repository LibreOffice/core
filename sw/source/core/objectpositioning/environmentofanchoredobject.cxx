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

    @author OD
*/
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

    OSL_ENSURE( pHoriEnvironmentLayFrm->ISA(SwLayoutFrm),
                "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pHoriEnvironmentLayFrm);
}

/** determine environment layout frame for possible vertical object positions
*/
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
                !pVertEnvironmentLayFrm->IsFtnFrm() &&
                !pVertEnvironmentLayFrm->IsPageBodyFrm() &&
                !pVertEnvironmentLayFrm->IsPageFrm() )
        {
            pVertEnvironmentLayFrm = pVertEnvironmentLayFrm->GetUpper();
            OSL_ENSURE( pVertEnvironmentLayFrm,
                    "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - proposed frame not found" );
        }
    }

    OSL_ENSURE( pVertEnvironmentLayFrm->ISA(SwLayoutFrm),
                "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrm(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrm&>(*pVertEnvironmentLayFrm);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
