/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intrinsicanimationactivity.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:53:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX

#include "wakeupevent.hxx"
#include "activity.hxx"
#include "slideshowcontext.hxx"
#include "drawshape.hxx"
#include "tools.hxx"

/* Definition of IntrinsicAnimationActivity class */

namespace slideshow
{
    namespace internal
    {
        /** Create an IntrinsicAnimationActivity.

            This is an Activity interface implementation for intrinsic
            shape animations. Intrinsic shape animations are
            animations directly within a shape, e.g. drawing layer
            animations, or GIF animations.

            @param rContext
            Common slideshow objects

            @param rDrawShape
            Shape to control the intrinsic animation for

            @param rWakeupEvent
            Externally generated wakeup event, to set this
            activity to sleep during inter-frame intervals. Must
            come frome the outside, since wakeup event and this
            object have mutual references to each other.

            @param rTimeouts
            Vector of timeout values, to wait before the next
            frame is shown.
        */
        ActivitySharedPtr createIntrinsicAnimationActivity(
            const SlideShowContext&         rContext,
            const DrawShapeSharedPtr&       rDrawShape,
            const WakeupEventSharedPtr&     rWakeupEvent,
            const ::std::vector<double>&    rTimeouts,
            ::std::size_t                   nNumLoops,
            CycleMode                       eCycleMode );
    }
}

#endif /* INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX */
