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

#include <svx/sdr/animation/animationstate.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <comphelper/lok.hxx>


namespace sdr::animation
{
        double PrimitiveAnimation::getSmallestNextTime(double fCurrentTime)
        {
            double fRetval(0.0);

            if(!maAnimatedPrimitives.empty())
            {
                const sal_Int32 nCount(maAnimatedPrimitives.size());

                for(sal_Int32 a(0); a < nCount; a++)
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
            if(::basegfx::fTools::equalZero(fNextTime))
                return;

            // next time point exists, use it
            sal_uInt32 nNextTime;

            if(fNextTime >= double(0xffffff00))
            {
                // take care for very late points in time, e.g. when a text animation stops
                // in a defined AnimationEntryFixed with endless (0xffffffff) duration
                nNextTime = GetTime() + (1000 * 60 * 60); // one hour, works with vcl timers, 0xffffff00 was too much...
            }
            else
            {
                nNextTime = static_cast<sal_uInt32>(fNextTime);
            }

            // ensure step forward in integer timing, the floating step difference maybe smaller than 1.0. Use
            // at least 25ms for next step
            const sal_uInt32 nMinimumStepTime(static_cast<sal_uInt32>(fCurrentTime) + 25);

            if(nNextTime <= nMinimumStepTime)
            {
                nNextTime = nMinimumStepTime;
            }

            // set time and reactivate by re-adding to the scheduler
            SetTime(nNextTime);
            mrVOContact.GetObjectContact().getPrimitiveAnimator().InsertEvent(*this);
        }

        PrimitiveAnimation::PrimitiveAnimation(sdr::contact::ViewObjectContact& rVOContact, const drawinglayer::primitive2d::Primitive2DContainer& rAnimatedPrimitives)
        :   Event(),
            mrVOContact(rVOContact),
            maAnimatedPrimitives(rAnimatedPrimitives)
        {
            if (!comphelper::LibreOfficeKit::isActive())
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

            if (!comphelper::LibreOfficeKit::isActive())
                // re-setup
                prepareNextEvent();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
