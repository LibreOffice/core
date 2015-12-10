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

#ifndef INCLUDED_SVX_SDR_ANIMATION_ANIMATIONSTATE_HXX
#define INCLUDED_SVX_SDR_ANIMATION_ANIMATIONSTATE_HXX

#include <sal/types.h>
#include <vector>
#include <svx/sdr/animation/scheduler.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

namespace sdr
{
    namespace contact
    {
        class ViewObjectContact;
    } // end of namespace contact
}

namespace sdr
{
    namespace animation
    {
        class PrimitiveAnimation : public Event
        {
        protected:
            // the animated VOC
            sdr::contact::ViewObjectContact&                    mrVOContact;

            // the vetor of animated primitives
            drawinglayer::primitive2d::Primitive2DContainer      maAnimatedPrimitives;

            // local helpers
            double getSmallestNextTime(double fCurrentTime);
            void prepareNextEvent();

        public:
            // basic constructor.
            PrimitiveAnimation(sdr::contact::ViewObjectContact& rVOContact, const drawinglayer::primitive2d::Primitive2DContainer& rAnimatedPrimitives);

            // destructor
            virtual ~PrimitiveAnimation();

            // execute event, from base class Event
            virtual void Trigger(sal_uInt32 nTime) override;
        };
    } // end of namespace animation
} // end of namespace sdr

#endif // INCLUDED_SVX_SDR_ANIMATION_ANIMATIONSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
