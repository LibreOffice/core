/*************************************************************************
 *
 *  $RCSfile: transitioninfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:22:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_SLIDESHOW_TRANSITIONINFO_HXX
#define INCLUDED_SLIDESHOW_TRANSITIONINFO_HXX

#include "sal/types.h"


namespace presentation {
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
