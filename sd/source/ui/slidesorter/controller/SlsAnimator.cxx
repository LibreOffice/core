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
        const Animator::AnimationFunction& rAnimation,
        const double nDelta);
    ~Animation (void);
    bool Run (void);
    bool IsExpired (void);
    Animator::AnimationFunction maAnimation;
    double mnValue;
    double mnDelta;
};




class Animator::DrawLock
{
public:
    DrawLock (View& rView);
    ~DrawLock (void);

private:
    View& mrView;
};




Animator::Animator (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maTimer(),
      maAnimations(),
      mpDrawLock()
{
    maTimer.SetTimeout(gnResolution);
    maTimer.SetTimeoutHdl(LINK(this,Animator,TimeoutHandler));
}




Animator::~Animator (void)
{
    maTimer.Stop();
    mpDrawLock.reset();
}




void Animator::AddAnimation (
    const AnimationFunction& rAnimation,
    const sal_Int32 nDuration)
{
    const double nDelta = double(gnResolution) / double(nDuration);
    maAnimations.push_back(boost::shared_ptr<Animation>(new Animation(rAnimation, nDelta)));

    // Prevent redraws except for the ones in TimeoutHandler.
    // While the Animator is active it will schedule repaints regularly.
    // Repaints in between would only lead to visual artifacts.
    mpDrawLock.reset(new DrawLock(mrSlideSorter.GetView()));
    maTimer.Start();
}




bool Animator::ServeAnimations (void)
{
    bool bExpired (false);

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
    if (ServeAnimations())
        CleanUpAnimationList();

    // Unlock the draw lock.  This should lead to a repaint.
    mpDrawLock.reset();

    if (maAnimations.size() > 0)
    {
        mpDrawLock.reset(new DrawLock(mrSlideSorter.GetView()));
        maTimer.Start();
    }

    return 0;
}




//===== Animator::Animation ===================================================

Animator::Animation::Animation (
    const Animator::AnimationFunction& rAnimation,
    const double nDelta)
    : maAnimation(rAnimation),
      mnValue(0),
      mnDelta(nDelta)
{

    maAnimation(mnValue);
    mnValue = mnDelta;

}




Animator::Animation::~Animation (void)
{
}




bool Animator::Animation::Run (void)
{
    if (mnValue < 1.0)
    {
        maAnimation(mnValue);
        mnValue += mnDelta;
        return false;
    }
    else
    {
        maAnimation(1.0);
        return true;
    }
}




bool Animator::Animation::IsExpired (void)
{
    return mnValue >= 1.0;
}




//===== Animator::DrawLock ====================================================

Animator::DrawLock::DrawLock (View& rView)
    : mrView(rView)
{
    mrView.LockRedraw(TRUE);
}




Animator::DrawLock::~DrawLock (void)
{
    mrView.LockRedraw(FALSE);
}


} } } // end of namespace ::sd::slidesorter::controller
