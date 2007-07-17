/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intrinsicanimationeventhandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:09:20 $
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

#ifndef INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>

/* Definition of IntrinsicAnimationEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling intrinsic animation display modes.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle intrinsic animation events.
         */
        class IntrinsicAnimationEventHandler
        {
        public:
            virtual ~IntrinsicAnimationEventHandler() {}

            virtual bool enableAnimations() = 0;
            virtual bool disableAnimations() = 0;
        };

        typedef ::boost::shared_ptr< IntrinsicAnimationEventHandler > IntrinsicAnimationEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX */
