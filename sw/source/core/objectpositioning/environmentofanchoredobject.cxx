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
#include <osl/diagnose.h>

namespace objectpositioning
{

SwEnvironmentOfAnchoredObject::SwEnvironmentOfAnchoredObject(
                                                const bool   _bFollowTextFlow )
    : mbFollowTextFlow( _bFollowTextFlow )
{}

/** determine environment layout frame for possible horizontal object positions */
const SwLayoutFrame& SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrame(
                                            const SwFrame& _rHoriOrientFrame ) const
{
    const SwFrame* pHoriEnvironmentLayFrame = &_rHoriOrientFrame;

    if ( !mbFollowTextFlow )
    {
        // No exception any more for page alignment.
        // the page frame determines the horizontal layout environment.
        pHoriEnvironmentLayFrame = _rHoriOrientFrame.FindPageFrame();
    }
    else
    {
        while ( !pHoriEnvironmentLayFrame->IsCellFrame() &&
                !pHoriEnvironmentLayFrame->IsFlyFrame() &&
                !pHoriEnvironmentLayFrame->IsPageFrame() )
        {
            pHoriEnvironmentLayFrame = pHoriEnvironmentLayFrame->GetUpper();
            assert(pHoriEnvironmentLayFrame &&
                    "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrame(..) - no page|fly|cell frame found");
        }
    }

    assert( dynamic_cast< const SwLayoutFrame *>( pHoriEnvironmentLayFrame ) &&
                "SwEnvironmentOfAnchoredObject::GetHoriEnvironmentLayoutFrame(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrame&>(*pHoriEnvironmentLayFrame);
}

/** determine environment layout frame for possible vertical object positions */
const SwLayoutFrame& SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame(
                                            const SwFrame& _rVertOrientFrame ) const
{
    const SwFrame* pVertEnvironmentLayFrame = &_rVertOrientFrame;

    if ( !mbFollowTextFlow )
    {
        // No exception any more for page alignment.
        // the page frame determines the vertical layout environment.
        pVertEnvironmentLayFrame = _rVertOrientFrame.FindPageFrame();
    }
    else
    {
        while ( !pVertEnvironmentLayFrame->IsCellFrame() &&
                !pVertEnvironmentLayFrame->IsFlyFrame() &&
                !pVertEnvironmentLayFrame->IsHeaderFrame() &&
                !pVertEnvironmentLayFrame->IsFooterFrame() &&
                !pVertEnvironmentLayFrame->IsFootnoteFrame() &&
                !pVertEnvironmentLayFrame->IsPageBodyFrame() &&
                !pVertEnvironmentLayFrame->IsPageFrame() )
        {
            pVertEnvironmentLayFrame = pVertEnvironmentLayFrame->GetUpper();
            assert(pVertEnvironmentLayFrame &&
                    "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame(..) - proposed frame not found");
        }
    }

    assert( dynamic_cast< const SwLayoutFrame *>( pVertEnvironmentLayFrame ) &&
                "SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame(..) - found frame isn't a layout frame" );

    return static_cast<const SwLayoutFrame&>(*pVertEnvironmentLayFrame);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
