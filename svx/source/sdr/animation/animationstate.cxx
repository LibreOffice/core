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
#include "precompiled_svx.hxx"
#include <svx/sdr/animation/animationstate.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        double PrimitiveAnimation::getSmallestNextTime(double fCurrentTime)
        {
            double fRetval(0.0);

            if(maAnimatedPrimitives.hasElements())
            {
                const sal_Int32 nCount(maAnimatedPrimitives.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    const drawinglayer::primitive2d::Primitive2DReference xRef(maAnimatedPrimitives[a]);
                    const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D* pCandidate = dynamic_cast< const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D* >(xRef.get());
                    OSL_ENSURE(pCandidate, "PrimitiveAnimation::getSmallestNextTime: wrong primitive in animated list (!)");

                    if(pCandidate)
                    {
                        const drawinglayer::animation::AnimationEntry& rAnimEntry = pCandidate->getAnimationEntry();
                        const double fNextTime(rAnimEntry.getNextEventTime(fCurrentTime));

                        if(!::basegfx::fTools::equalZero(fNextTime))
                        {
                            if(::basegfx::fTools::equalZero(fRetval))
                            {
                                fRetval = fNextTime;
                            }
                            else if(::basegfx::fTools::less(fNextTime, fRetval))
                            {
                                fRetval = fNextTime;
                            }
                        }
                    }
                }
            }

            return fRetval;
        }

        void PrimitiveAnimation::prepareNextEvent()
        {
            const double fCurrentTime(mrVOContact.GetObjectContact().getPrimitiveAnimator().GetTime());
            const double fNextTime(getSmallestNextTime(fCurrentTime));

            // getSmallestNextTime will be zero when animation ended. If not zero, a next step
            // exists
            if(!::basegfx::fTools::equalZero(fNextTime))
            {
                // next time point exists, use it
                sal_uInt32 nNextTime;

                if(fNextTime >= (double)0xffffff00)
                {
                    // take care for very late points in time, e.g. when a text animation stops
                    // in a defined AnimationEntryFixed with endless (0xffffffff) duration
                    nNextTime = GetTime() + (1000 * 60 * 60); // one hour, works with vcl timers, 0xffffff00 was too much...
                }
                else
                {
                    nNextTime = (sal_uInt32)fNextTime;
                }

                // ensure step forward in integer timing, the floating step difference maybe smaller than 1.0. Use
                // at least 25ms for next step
                const sal_uInt32 nMinimumStepTime((sal_uInt32)fCurrentTime + 25L);

                if(nNextTime <= nMinimumStepTime)
                {
                    nNextTime = nMinimumStepTime;
                }

                // set time and reactivate by re-adding to the scheduler
                SetTime(nNextTime);
                mrVOContact.GetObjectContact().getPrimitiveAnimator().InsertEvent(this);
            }
        }

        PrimitiveAnimation::PrimitiveAnimation(sdr::contact::ViewObjectContact& rVOContact, const drawinglayer::primitive2d::Primitive2DSequence& rAnimatedPrimitives)
        :   Event(0L),
            mrVOContact(rVOContact),
            maAnimatedPrimitives(rAnimatedPrimitives)
        {
            // setup initially
            prepareNextEvent();
        }

        PrimitiveAnimation::~PrimitiveAnimation()
        {
            // ensure that Event member is removed from PrimitiveAnimator
            mrVOContact.GetObjectContact().getPrimitiveAnimator().RemoveEvent(this);
        }

        // execute event, from base class Event
        void PrimitiveAnimation::Trigger(sal_uInt32 /*nTime*/)
        {
            // schedule a repaint of associated object
            mrVOContact.ActionChanged();

            // re-setup
            prepareNextEvent();
        }
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
