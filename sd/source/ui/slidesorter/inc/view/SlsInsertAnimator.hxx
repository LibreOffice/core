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

#ifndef SD_SLIDESORTER_VIEW_INSERT_ANIMATOR_HXX
#define SD_SLIDESORTER_VIEW_INSERT_ANIMATOR_HXX

#include "controller/SlsAnimator.hxx"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace sd { namespace slidesorter { namespace view {

class InsertPosition;


/** Animate the positions of page objects to make room at the insert
    position while a move or copy operation takes place.
*/
class InsertAnimator
    : private ::boost::noncopyable
{
public:
    InsertAnimator (SlideSorter& rSlideSorter);

    /** Set the position at which we have to make room for the display of an
        icon.
    */
    void SetInsertPosition (const InsertPosition& rInsertPosition);

    enum ResetMode { RM_Normal, RM_AbortAnimations };
    /** Restore the normal position of all page objects.
        @param eMode
            This flag controls wether to start an animation that ends in the
            normal positions of all slides (AM_Animated) or to restore the
            normal positions immediately (AM_Immediate).
    */
    void Reset (const controller::Animator::AnimationMode eMode);

private:
    class Implementation;
    ::boost::shared_ptr<Implementation> mpImplementation;
};


} } } // end of namespace ::sd::slidesorter::view

#endif
