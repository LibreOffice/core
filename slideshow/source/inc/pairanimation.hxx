/*************************************************************************
 *
 *  $RCSfile: pairanimation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:18:44 $
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

#ifndef _SLIDESHOW_PAIRANIMATION_HXX
#define _SLIDESHOW_PAIRANIMATION_HXX

#include <animation.hxx>

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif


/* Definition of PairAnimation interface */

namespace presentation
{
    namespace internal
    {
        /** Interface defining a pair of numbers animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes representable
            by a pair of floating point values (e.g. a position or a
            size).
         */
        class PairAnimation : public Animation
        {
        public:
            typedef ::basegfx::B2DTuple ValueType;

            /** Set the animation to the given value

                @param rValue
                Current animation value.
             */
            virtual bool operator()( const ValueType& rValue ) = 0;

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

        typedef ::boost::shared_ptr< PairAnimation > PairAnimationSharedPtr;

    }
}

#endif /* _SLIDESHOW_PAIRANIMATION_HXX */
