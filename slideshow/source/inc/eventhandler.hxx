/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:56:15 $
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

#ifndef _SLIDESHOW_EVENTHANDLER_HXX
#define _SLIDESHOW_EVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>

#include <disposable.hxx>


/* Definition of EventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for event handling objects.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle events.
         */
        class EventHandler : public Disposable
        {
        public:
            /** Handle the event.

                @return true, if this handler has successfully
                processed the event. When this method returns false,
                possibly other, less prioritized handlers are called,
                too.
             */
            virtual bool handleEvent() = 0;
        };

        typedef ::boost::shared_ptr< EventHandler > EventHandlerSharedPtr;

    }
}

#endif /* _SLIDESHOW_EVENTHANDLER_HXX */
