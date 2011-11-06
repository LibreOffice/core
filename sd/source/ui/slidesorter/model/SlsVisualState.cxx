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
