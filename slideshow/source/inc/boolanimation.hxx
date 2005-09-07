/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: boolanimation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:08:06 $
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

#ifndef _SLIDESHOW_BOOLANIMATION_HXX
#define _SLIDESHOW_BOOLANIMATION_HXX

#include <animation.hxx>


/* Definition of BoolAnimation interface */

namespace presentation
{
    namespace internal
    {
        /** Interface defining a bool animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes that have
            only two discrete values (on and off, or true and false,
            for example).
         */
        class BoolAnimation : public Animation
        {
        public:
            typedef bool ValueType;

            /** Set the animation to the given value

                @param bValue
                Current animation value.
             */
            virtual bool operator()( ValueType bValue ) = 0;

            /** Request the underlying value for this animation.

                This is necessary for pure To or By animations, as the
                Activity cannot determine a sensible start value
                otherwise.

                @attention Note that you are only permitted to query
                for the underlying value, if the animation has actually
                been started (via start() call).
             */
            virtual ValueType getUnderlyingValue() const = 0;
        };

        typedef ::boost::shared_ptr< BoolAnimation > BoolAnimationSharedPtr;

    }
}

#endif /* _SLIDESHOW_BOOLANIMATION_HXX */
