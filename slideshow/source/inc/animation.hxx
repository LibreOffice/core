/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:04:50 $
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

#ifndef _SLIDESHOW_ANIMATION_HXX
#define _SLIDESHOW_ANIMATION_HXX

#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


/* Definition of Animation interface */

namespace presentation
{
    namespace internal
    {
        /** Interface defining a generic animation.

            This interface is used by objects implementing the
            Activity interface to drive the animation effect. Objects
            implementing this interface will receive time-varying
            animation values, which they can forward to the
            appropriate attributes. The type of these animation values
            is given in derived interfaces.

            @see NumberAnimation
            @see ColorAnimation
            @see PairAnimation
         */
        class Animation
        {
        public:
            virtual ~Animation() {}

            /** Notify that the animation is about to begin.

                @param rShape
                Shape to apply this animation to.

                @param rAttrLayer
                Attribute layer to play the animation on.
             */
            virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                const ShapeAttributeLayerSharedPtr& rAttrLayer ) = 0;

            /** Notify that the animation is about to end.
             */
            virtual void end() = 0;
        };

        typedef ::boost::shared_ptr< Animation > AnimationSharedPtr;

    }
}

#endif /* _SLIDESHOW_ANIMATION_HXX */
