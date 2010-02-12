/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsAnimator.cxx,v $
 *
 * $Revision: 1.3 $
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
#include "controller/SlsAnimator.hxx"
#include "view/SlideSorterView.hxx"
#include "View.hxx"
#include <boost/bind.hpp>

namespace sd { namespace slidesorter { namespace controller {

namespace {
    static const sal_Int32 gnResolution = 25;
}
/** Handle one animation function by using a timer for frequent calls to
    the animations operator().
*/
class Animator::Animation
{
public:
    Animation (
        const Animator::AnimationFunctor& rAnimation,
        const double nDelta,
        const double nEnd,
        const Animator::AnimationId nAnimationId,
        const Animator::FinishFunctor& rFinishFunctor);
    ~Animation (void);
    /** Run next animation step.  If animation has reached its end it is
        expired.
    */
    bool Run (void);
    /** Typically called when an animation has finished, but also from
        Animator::Disposed().  The finish functor is called and the
        animation is marked as expired to prevent another run.
    */
    void Expire (void);
    bool IsExpired (void);

    Animator::AnimationFunctor maAnimation;
    Animator::FinishFunctor maFinishFunctor;
    const Animator::AnimationId mnAnimationId;
    double mnValue;
    const double mnEnd;
    const double mnDelta;
    bool mbIsExpired;
};




Animator::Animator (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maTimer(),
      mbIsDisposed(false),
      maAnimations(),
      mpDrawLock(),
      mnNextAnimationId(0)
{
    maTimer.SetTimeout(gnResolution);
    maTimer.SetTimeoutHdl(LINK(this,Animator,TimeoutHandler));
}




Animator::~Animator (void)
{
    if ( ! mbIsDisposed)
    {
        OSL_ASSERT(mbIsDisposed);
        Dispose();
    }
}




void Animator::Dispose (void)
{
    mbIsDisposed = true;

    AnimationList aCopy (maAnimations);
    AnimationList::const_iterator iAnimation;
    for (iAnimation=aCopy.begin(); iAnimation!=aCopy.end(); ++iAnimation)
        (*iAnimation)->Expire();

    maTimer.Stop();
    mpDrawLock.reset();
}




Animator::AnimationId Animator::AddAnimation (
    const AnimationFunctor& rAnimation,
    const sal_Int32 nDuration,
    const FinishFunctor& rFinishFunctor)
{
    // When the animator is already disposed then ignore this call
    // silently (well, we show an assertion, but do not throw an exception.)
    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return -1;

    const double nDelta = double(gnResolution) / double(nDuration);
    boost::shared_ptr<Animation> pAnimation (
        new Animation(rAnimation, nDelta, 1.0,  ++mnNextAnimationId, rFinishFunctor));
    maAnimations.push_back(pAnimation);

    // Prevent redraws except for the ones in TimeoutHandler.
    // While the Animator is active it will schedule repaints regularly.
    // Repaints in between would only lead to visual artifacts.
    mpDrawLock.reset(new view::SlideSorterView::DrawLock(mrSlideSorter));
    maTimer.Start();

    return pAnimation->mnAnimationId;
}




Animator::AnimationId Animator::AddInfiniteAnimation (
    const AnimationFunctor& rAnimation,
    const double nDelta)
{
    // When the animator is already disposed then ignore this call
    // silently (well, we show an assertion, but do not throw an exception.)
    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return -1;

    boost::shared_ptr<Animation> pAnimation (
        new Animation(rAnimation, nDelta, -1.0, mnNextAnimationId++, FinishFunctor()));
    maAnimations.push_back(pAnimation);

    // Prevent redraws except for the ones in TimeoutHandler.
    // While the Animator is active it will schedule repaints regularly.
    // Repaints in between would only lead to visual artifacts.
    mpDrawLock.reset(new view::SlideSorterView::DrawLock(mrSlideSorter));
    maTimer.Start();

    return pAnimation->mnAnimationId;
}




void Animator::RemoveAnimation (const Animator::AnimationId nId)
{
    OSL_ASSERT( ! mbIsDisposed);

    const AnimationList::iterator iAnimation (::std::find_if(
        maAnimations.begin(),
        maAnimations.end(),
        ::boost::bind(
            ::std::equal_to<Animator::AnimationId>(),
            nId,
            ::boost::bind(&Animation::mnAnimationId, _1))));
    if (iAnimation != maAnimations.end())
    {
        OSL_ASSERT((*iAnimation)->mnAnimationId == nId);
        (*iAnimation)->Expire();
        maAnimations.erase(iAnimation);
    }

    // Reset the animation id when we can.
    if (maAnimations.empty())
        mnNextAnimationId = 0;
}




bool Animator::ProcessAnimations (void)
{
    bool bExpired (false);

    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return bExpired;


    AnimationList aCopy (maAnimations);
    AnimationList::const_iterator iAnimation;
    for (iAnimation=aCopy.begin(); iAnimation!=aCopy.end(); ++iAnimation)
    {
        bExpired |= (*iAnimation)->Run();
    }

    return bExpired;
}




void Animator::CleanUpAnimationList (void)
{
    OSL_ASSERT( ! mbIsDisposed);
    if (mbIsDisposed)
        return;

    AnimationList aActiveAnimations;

    AnimationList::const_iterator iAnimation;
    for (iAnimation=maAnimations.begin(); iAnimation!=maAnimations.end(); ++iAnimation)
    {
        if ( ! (*iAnimation)->IsExpired())
            aActiveAnimations.push_back(*iAnimation);
    }

    maAnimations.swap(aActiveAnimations);
}




IMPL_LINK(Animator, TimeoutHandler, Timer*, EMPTYARG)
{
    if (mbIsDisposed)
        return 0;

    OSL_TRACE("Animator timeout start");

    if (ProcessAnimations())
        CleanUpAnimationList();

    // Unlock the draw lock.  This should lead to a repaint.
    mpDrawLock.reset();

    if (maAnimations.size() > 0)
    {
        mpDrawLock.reset(new view::SlideSorterView::DrawLock(mrSlideSorter));
        maTimer.Start();
    }

    OSL_TRACE("Animator timeout end");

    return 0;
}




//===== Animator::Animation ===================================================

Animator::Animation::Animation (
    const Animator::AnimationFunctor& rAnimation,
    const double nDelta,
    const double nEnd,
    const Animator::AnimationId nId,
    const Animator::FinishFunctor& rFinishFunctor)
    : maAnimation(rAnimation),
      maFinishFunctor(rFinishFunctor),
      mnAnimationId(nId),
      mnValue(0),
      mnEnd(nEnd),
      mnDelta(nDelta),
      mbIsExpired(false)
{
    maAnimation(mnValue);
    mnValue = mnDelta;
}




Animator::Animation::~Animation (void)
{
}




bool Animator::Animation::Run (void)
{
    if ( ! mbIsExpired)
    {
        if (mnEnd>=0 && mnValue>=mnEnd)
        {
            maAnimation(mnEnd);
            Expire();
            return true;
        }
        else
        {
            maAnimation(mnValue);
            mnValue += mnDelta;
            return false;
        }
    }
    else
        return true;
}




void Animator::Animation::Expire (void)
{
    if ( ! mbIsExpired)
    {
        mbIsExpired = true;
        if (maFinishFunctor)
            maFinishFunctor();
    }
}




bool Animator::Animation::IsExpired (void)
{
    return mbIsExpired;
}




} } } // end of namespace ::sd::slidesorter::controller
