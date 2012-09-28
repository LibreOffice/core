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

    SetMouseOverState(rDescriptor.HasState(PageDescriptor::ST_MouseOver));
}


void VisualState::SetMouseOverState (const bool bIsMouseOver)
{
    mbOldMouseOverState = mbCurrentMouseOverState;
    mbCurrentMouseOverState = bIsMouseOver;
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
