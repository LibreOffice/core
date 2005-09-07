/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationactivity.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:05:08 $
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

#ifndef _SLIDESHOW_ANIMATIONACTIVITY_HXX
#define _SLIDESHOW_ANIMATIONACTIVITY_HXX

#include <activity.hxx>
#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


/* Definition of AnimationActivity interface */

namespace presentation
{
    namespace internal
    {

        /** Extends the Activity interface with animation-specific functions
         */
        class AnimationActivity : public Activity
        {
        public:
            /** Sets targets (shape and attributeLayer)

                Since attribute layers can only be generated when the
                animation starts, the Activity owner must be able to
                pass it into the Activity after initial creation. The
                same applies to the actual shape the animation must
                run for, since e.g. subsetted shapes are generated
                close before the animation starts, too (this is not
                necessary in and out of itself, but for performance
                reasons. Otherwise, character iterations produce tons
                of subset shapes).

                @param rShape
                Shape to play the animation on.

                @param rAttrLayer
                Attribute layer to change the animated values on.
             */
            virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                                     const ShapeAttributeLayerSharedPtr&    rAttrLayer ) = 0;
        };

        typedef ::boost::shared_ptr< AnimationActivity > AnimationActivitySharedPtr;

    }
}

#endif /* _SLIDESHOW_ANIMATIONACTIVITY_HXX */
