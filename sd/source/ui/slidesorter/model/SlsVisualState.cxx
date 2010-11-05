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

#include "precompiled_sd.hxx"

#include "model/SlsVisualState.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsAnimator.hxx"

namespace sd { namespace slidesorter { namespace model {

VisualState::VisualState (const sal_Int32 nPageId)
    : mnPageId(nPageId),
      meCurrentVisualState(VS_None),
      meOldVisualState(VS_None),
      mnVisualStateBlend(1.0),
      mnStateAnimationId(controller::Animator::NotAnAnimationId),
      maLocationOffset(0,0),
      mnLocationAnimationId(controller::Animator::NotAnAnimationId),
      mnButtonAlpha(1.0),
      mnButtonBarAlpha(1.0),
      mnButtonAlphaAnimationId(controller::Animator::NotAnAnimationId)
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




VisualState::State VisualState::GetCurrentVisualState (void) const
{
    return meCurrentVisualState;
}




VisualState::State VisualState::GetOldVisualState (void) const
{
    return meOldVisualState;
}




void VisualState::SetVisualState (const State eState)
{
    meOldVisualState = meCurrentVisualState;
    meCurrentVisualState = eState;
    mnVisualStateBlend = 1.0;
}




double VisualState::GetVisualStateBlend (void) const
{
    return mnVisualStateBlend;
}




void VisualState::SetVisualStateBlend (const double nBlend)
{
    mnVisualStateBlend = nBlend;
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




sal_Int32 VisualState::GetStateAnimationId (void) const
{
    return mnStateAnimationId;
}




void VisualState::SetStateAnimationId (const sal_Int32 nAnimationId)
{
    mnStateAnimationId = nAnimationId;
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




sal_Int32 VisualState::GetLocationAnimationId (void) const
{
    return mnLocationAnimationId;
}




void VisualState::SetLocationAnimationId (const sal_Int32 nAnimationId)
{
    mnLocationAnimationId = nAnimationId;
}




double VisualState::GetButtonAlpha (void) const
{
    return mnButtonAlpha;
}




void VisualState::SetButtonAlpha (const double nAlpha)
{
    mnButtonAlpha = nAlpha;
}




double VisualState::GetButtonBarAlpha (void) const
{
    return mnButtonBarAlpha;
}




void VisualState::SetButtonBarAlpha (const double nAlpha)
{
    mnButtonBarAlpha = nAlpha;
}




sal_Int32 VisualState::GetButtonAlphaAnimationId (void) const
{
    return mnButtonAlphaAnimationId;
}




void VisualState::SetButtonAlphaAnimationId (const sal_Int32 nAnimationId)
{
    mnButtonAlphaAnimationId = nAnimationId;
}


} } } // end of namespace ::sd::slidesorter::model
