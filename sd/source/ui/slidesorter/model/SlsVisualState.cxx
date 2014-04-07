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

#include "model/SlsVisualState.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsAnimator.hxx"

namespace sd { namespace slidesorter { namespace model {

VisualState::VisualState (const sal_Int32 nPageId)
    : mnPageId(nPageId),
      meCurrentVisualState(VS_None),
      meOldVisualState(VS_None),
      mnStateAnimationId(controller::Animator::NotAnAnimationId),
      maLocationOffset(0,0),
      mnLocationAnimationId(controller::Animator::NotAnAnimationId)
{
}

VisualState::~VisualState (void)
{
    if (mnStateAnimationId != controller::Animator::NotAnAnimationId
         || mnLocationAnimationId != controller::Animator::NotAnAnimationId)
    {
        OSL_ASSERT(mnStateAnimationId == controller::Animator::NotAnAnimationId);
        OSL_ASSERT(mnLocationAnimationId == controller::Animator::NotAnAnimationId);
    }
}

void VisualState::SetVisualState (const State eState)
{
    meOldVisualState = meCurrentVisualState;
    meCurrentVisualState = eState;
}


void VisualState::UpdateVisualState (const PageDescriptor& rDescriptor)
{
    if (rDescriptor.HasState(PageDescriptor::ST_Excluded))
        SetVisualState(VS_Excluded);
    else if (rDescriptor.HasState(PageDescriptor::ST_Current))
        SetVisualState(VS_Current);
    else if (rDescriptor.HasState(PageDescriptor::ST_Focused))
        SetVisualState(VS_Focused);
    else if (rDescriptor.HasState(PageDescriptor::ST_Selected))
        SetVisualState(VS_Selected);
    else
        SetVisualState(VS_None);
}

Point VisualState::GetLocationOffset (void) const
{
    return maLocationOffset;
}


bool VisualState::SetLocationOffset (const Point& rOffset)
{
    if (maLocationOffset != rOffset)
    {
        maLocationOffset = rOffset;
        return true;
    }
    else
        return false;
}


} } } // end of namespace ::sd::slidesorter::model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
