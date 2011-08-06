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

#ifndef _SDR_ANIMATION_ANIMATIONSTATE_HXX
#define _SDR_ANIMATION_ANIMATIONSTATE_HXX

#include <sal/types.h>
#include <vector>
#include <svx/sdr/animation/scheduler.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr
{
    namespace contact
    {
        class ViewObjectContact;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

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
            drawinglayer::primitive2d::Primitive2DSequence      maAnimatedPrimitives;

            // local helpers
            double getSmallestNextTime(double fCurrentTime);
            void prepareNextEvent();

        public:
            // basic constructor.
            PrimitiveAnimation(sdr::contact::ViewObjectContact& rVOContact, const drawinglayer::primitive2d::Primitive2DSequence& rAnimatedPrimitives);

            // destructor
            virtual ~PrimitiveAnimation();

            // execute event, from base class Event
            virtual void Trigger(sal_uInt32 nTime);
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_ANIMATIONSTATE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
