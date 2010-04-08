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

#ifndef SD_SLIDESORTER_VISIBLE_AREA_MANAGER_HXX
#define SD_SLIDESORTER_VISIBLE_AREA_MANAGER_HXX

#include "controller/SlsAnimator.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace sd { namespace slidesorter { namespace controller {


/** Manage requests for scrolling page objects into view.
*/
class VisibleAreaManager
    : public ::boost::noncopyable
{
public:
    VisibleAreaManager (SlideSorter& rSlideSorter);
    ~VisibleAreaManager (void);

    /** Request to make the specified page object visible.
        @param eRequestedAnimationMode
            This flag specifies wether to smoothly scroll the page object into
            view (AM_Animated) or do this in one step (AM_Immediate).
    */
    void RequestVisible (
        const model::SharedPageDescriptor& rpDescriptor,
        const Animator::AnimationMode eRequestedAnimationMode = Animator::AM_Immediate);

private:
    SlideSorter& mrSlideSorter;

    /** List of rectangle that someone wants to be moved into the visible
        area.
        Cleared on every call to ForgetVisibleRequests() and MakeVisible().
    */
    ::std::vector<Rectangle> maVisibleRequests;

    /** Animation id for a scroll animation that sets the top
        and left of the visible area to maRequestedVisibleTopLeft.
    */
    Animator::AnimationId mnScrollAnimationId;
    Point maRequestedVisibleTopLeft;

    Animator::AnimationMode meRequestedAnimationMode;

    void MakeVisible (void);
    ::boost::optional<Point> GetRequestedTopLeft (void) const;
};


} } } // end of namespace ::sd::slidesorter::view

#endif
