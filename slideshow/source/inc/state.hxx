/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: state.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:04:32 $
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

#ifndef _SLIDESHOW_STATE_HXX
#define _SLIDESHOW_STATE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


/* Definition of State interface */

namespace slideshow
{
    namespace internal
    {

        /** This interface represents a stateful object.

            The state ID returned by the getStateId() method
            abstractly encodes the object's state. When this ID
            changes, clients can assume that the object's state has
            changed.
         */
        class State
        {
        public:
            virtual ~State() {}

            /// Abstract, numerically encoded state ID
            typedef ::std::size_t StateId;

            /** This method returns a numerical state identifier.

                The state ID returned by this method abstractly
                encodes the object's state. When this ID changes,
                clients can assume that the object's state has
                changed.

                @return an abstract, numerical state ID.
             */
            virtual StateId getStateId() const = 0;
        };

        typedef ::boost::shared_ptr< State > StateSharedPtr;
    }
}

#endif /* _SLIDESHOW_STATE_HXX */
