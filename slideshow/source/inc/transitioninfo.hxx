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

#if ! defined INCLUDED_SLIDESHOW_TRANSITIONINFO_HXX
#define INCLUDED_SLIDESHOW_TRANSITIONINFO_HXX

#include <sal/types.h>


namespace slideshow {
namespace internal {

struct TransitionInfo
{
    // the following two member serve as the search key
    // for an incoming XTransitionFilter node
    //
    // {

    sal_Int16       mnTransitionType;
    sal_Int16       mnTransitionSubType;

    // }
    //

    /** This enum classifies a transition type
     */
    enum TransitionClass
    {
        /// Invalid type
        TRANSITION_INVALID,

        /// Transition expressed by parametric clip polygon
        TRANSITION_CLIP_POLYPOLYGON,

        /// Transition expressed by hand-crafted function
        TRANSITION_SPECIAL
    };

    /// class of effect handling
    TransitionClass meTransitionClass;

    /// Rotation angle of clip polygon
    double          mnRotationAngle;

    /// X scaling of clip polygon (negative values mirror)
    double          mnScaleX;

    /// Y scaling of clip polygon (negative values mirror)
    double          mnScaleY;

    /** This enum determines the method how to reverse
        a parametric clip polygon transition.

        A reversed transition runs in the geometrically
        opposite direction. For a left-to-right bar wipe, the
        reversed transition is a right-to-left wipe, whereas
        for an iris transition, the reversed mode will show
        the target in the outer area (instead of in the inner
        area, as in normal mode).
    */
    enum ReverseMethod
    {
        /** Ignore direction attribute altogether
            (if it has no sensible meaning for this transition)
        */
        REVERSEMETHOD_IGNORE,

        /** Revert by changing the direction of the parameter sweep
            (from 1->0 instead of 0->1)
        */
        REVERSEMETHOD_INVERT_SWEEP,

        /** Revert by subtracting the generated polygon from the
            target bound rect
        */
        REVERSEMETHOD_SUBTRACT_POLYGON,

        /** Combination of REVERSEMETHOD_INVERT_SWEEP and
            REVERSEMETHOD_SUBTRACT_POLYGON.
        */
        REVERSEMETHOD_SUBTRACT_AND_INVERT,

        /// Reverse by rotating polygon 180 degrees
        REVERSEMETHOD_ROTATE_180,

        /// Reverse by flipping polygon at the y (!) axis
        REVERSEMETHOD_FLIP_X,

        /// Reverse by flipping polygon at the x (!) axis
        REVERSEMETHOD_FLIP_Y
    };

    /** Indicating the method to use when transition
        should be 'reversed'.

        @see ReverseMethod
    */
    ReverseMethod   meReverseMethod;

    /** When true, transition 'out' effects are realized
        by inverting the parameter sweep direction (1->0
        instead of 0->1). Otherwise, 'out' effects are
        realized by changing inside and outside areas of
        the parametric poly-polygon.
    */
    bool            mbOutInvertsSweep;

    /** when true, scale clip polygon isotrophically to
        target size.  when false, scale is
        anisotrophically.
    */
    bool            mbScaleIsotrophically;


    /// Compare against type and subtype
    class Comparator
    {
        sal_Int16 mnTransitionType;
        sal_Int16 mnTransitionSubType;
    public:
        Comparator( sal_Int16 nTransitionType,
                    sal_Int16 nTransitionSubType )
            : mnTransitionType( nTransitionType ),
              mnTransitionSubType( nTransitionSubType ) {}
        bool operator()( const TransitionInfo& rEntry ) const {
            return rEntry.mnTransitionType == mnTransitionType &&
                rEntry.mnTransitionSubType == mnTransitionSubType;
        }
    };
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_TRANSITIONINFO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
