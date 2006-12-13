/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumanimation.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:55:37 $
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

#ifndef _SLIDESHOW_ENUMANIMATION_HXX
#define _SLIDESHOW_ENUMANIMATION_HXX

#include <animation.hxx>


/* Definition of EnumAnimation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining an enum animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes representable
            by a set of constant values, such as UNO constantsm, or enums.
         */
        class EnumAnimation : public Animation
        {
        public:
            typedef sal_Int16 ValueType;

            /** Set the animation to value k

                @param k
                Current animation value (must be in an
                attribute-specific permissible range). Overflowing
                values will be clipped to the permissible range
                internally.
             */
            virtual bool operator()( ValueType k ) = 0;

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

        typedef ::boost::shared_ptr< EnumAnimation > EnumAnimationSharedPtr;

    }
}

#endif /* _SLIDESHOW_ENUMANIMATION_HXX */
