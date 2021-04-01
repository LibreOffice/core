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

#include <controller/SlsAnimator.hxx>
#include <view/SlideSorterView.hxx>
#include <osl/diagnose.h>

namespace sd::slidesorter::controller {

/** Handle one animation function by using a timer for frequent calls to
    the animations operator().
*/
class Animator::Animation
{
public:
    Animation (
        const Animator::AnimationFunctor& rAnimation,
        const double nStartOffset,
        const double nDuration,
        const double nGlobalTime,
        const Animator::AnimationId nAnimationId,
        const Animator::FinishFunctor& rFinishFunctor);
    /** Run next animation step.  If animation has reached its end it is
        expired.
    */
    bool Run (const double nGlobalTime);

    /** Typically called when an animation has finished, but also from
        Animator::Disposed().  The finish functor is called and the
        animation is marked as expired to prevent another run.
    */
    void Expire();
    bool IsExpired() const { return mbIsExpired;}

    Animator::AnimationFunctor maAnimation;
    Animator::FinishFunctor maFinishFunctor;
    const Animator::AnimationId mnAnimationId;
    const double mnDuration;
    const double mnEnd;
    const double mnGlobalTimeAtStart;
    bool mbIsExpired;
};

Animator::Animator (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maIdle("sd slidesorter controller Animator"),
      mbIsDisposed(false),
      maAnimations(),
      maElapsedTime(),
      mpDrawLock(),
      mnNextAnimationId(0)
{
    maIdle.SetPriority(TaskPriority::REPAINT);
    maIdle.SetInvokeHandler(LINK(this,Animator,TimeoutHandler));
}

Animator::~Animator()
{
    if ( ! mbIsDisposed)
    {
        OSL_ASSERT(mbIsDisposed);
        Dispose();
    }
}

void Animator::Dispose()
{
    mbIsDisposed = true;

    AnimationList aCopy (maAnimations);
    for (const auto& rxAnimation : aCopy)
        rxAnimation->Expire();

    maIdle.Stop();
    if (mpDrawLock)
    {
        mpDrawLock->Dispose();
        mpDrawLock.reset();
    }
}

Animator::AnimationId Animator::AddAnimation (
    const AnimationFunctor& rAnimation,
    const FinishFunctor& rFinishFunctor)
{
    // When the animator is already disposed then ignore this call
    // silently (well, we show an assertion, but do not throw an exception.)
    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return -1;

    std::shared_ptr<Animation> pAnimation =
        std::make_shared<Animation>(
            rAnimation,
            0,
            300 / 1000.0,
            maElapsedTime.getElapsedTime(),
            ++mnNextAnimationId,
            rFinishFunctor);
    maAnimations.push_back(pAnimation);

    RequestNextFrame();

    return pAnimation->mnAnimationId;
}

void Animator::RemoveAnimation (const Animator::AnimationId nId)
{
    OSL_ASSERT( ! mbIsDisposed);

    const AnimationList::iterator iAnimation (::std::find_if(
        maAnimations.begin(),
        maAnimations.end(),
        [nId] (std::shared_ptr<Animation> const& pAnim)
            { return nId == pAnim->mnAnimationId; }));
    if (iAnimation != maAnimations.end())
    {
        OSL_ASSERT((*iAnimation)->mnAnimationId == nId);
        (*iAnimation)->Expire();
        maAnimations.erase(iAnimation);
    }

    if (maAnimations.empty())
    {
        // Reset the animation id when we can.
        mnNextAnimationId = 0;

        // No more animations => we do not have to suppress painting
        // anymore.
        mpDrawLock.reset();
    }
}

void Animator::RemoveAllAnimations()
{
    for (auto const& it : maAnimations)
    {
        it->Expire();
    }
    maAnimations.clear();
    mnNextAnimationId = 0;

    // No more animations => we do not have to suppress painting
    // anymore.
    mpDrawLock.reset();
}

bool Animator::ProcessAnimations (const double nTime)
{
    bool bExpired (false);

    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return bExpired;

    AnimationList aCopy (maAnimations);
    for (const auto& rxAnimation : aCopy)
    {
        bExpired |= rxAnimation->Run(nTime);
    }

    return bExpired;
}

void Animator::CleanUpAnimationList()
{
    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return;

    AnimationList aActiveAnimations;

    for (const auto& rxAnimation : maAnimations)
    {
        if ( ! rxAnimation->IsExpired())
            aActiveAnimations.push_back(rxAnimation);
    }

    maAnimations.swap(aActiveAnimations);
}

void Animator::RequestNextFrame ()
{
    if ( ! maIdle.IsActive())
    {
        // Prevent redraws except for the ones in TimeoutHandler.  While the
        // Animator is active it will schedule repaints regularly.  Repaints
        // in between would only lead to visual artifacts.
        mpDrawLock.reset(new view::SlideSorterView::DrawLock(mrSlideSorter));
        maIdle.Start();
    }
}

IMPL_LINK_NOARG(Animator, TimeoutHandler, Timer *, void)
{
    if (mbIsDisposed)
        return;

    if (ProcessAnimations(maElapsedTime.getElapsedTime()))
        CleanUpAnimationList();

    // Unlock the draw lock.  This should lead to a repaint.
    mpDrawLock.reset();

    if (!maAnimations.empty())
        RequestNextFrame();
}

//===== Animator::Animation ===================================================

Animator::Animation::Animation (
    const Animator::AnimationFunctor& rAnimation,
    const double nStartOffset,
    const double nDuration,
    const double nGlobalTime,
    const Animator::AnimationId nId,
    const Animator::FinishFunctor& rFinishFunctor)
    : maAnimation(rAnimation),
      maFinishFunctor(rFinishFunctor),
      mnAnimationId(nId),
      mnDuration(nDuration),
      mnEnd(nGlobalTime + nDuration + nStartOffset),
      mnGlobalTimeAtStart(nGlobalTime + nStartOffset),
      mbIsExpired(false)
{
    Run(nGlobalTime);
}

bool Animator::Animation::Run (const double nGlobalTime)
{
    if ( ! mbIsExpired)
    {
        if (mnDuration > 0)
        {
            if (nGlobalTime >= mnEnd)
            {
                maAnimation(1.0);
                Expire();
            }
            else if (nGlobalTime >= mnGlobalTimeAtStart)
            {
                maAnimation((nGlobalTime - mnGlobalTimeAtStart) / mnDuration);
            }
        }
        else if (mnDuration < 0)
        {
            // Animations without end have to be expired by their owner.
            maAnimation(nGlobalTime);
        }
    }

    return mbIsExpired;
}

void Animator::Animation::Expire()
{
    if ( ! mbIsExpired)
    {
        mbIsExpired = true;
        if (maFinishFunctor)
            maFinishFunctor();
    }
}

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
