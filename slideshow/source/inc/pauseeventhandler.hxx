/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pauseeventhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:00:50 $
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

#ifndef _SLIDESHOW_PAUSEEVENTHANDLER_HXX
#define _SLIDESHOW_PAUSEEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>


/* Definition of PauseHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling pause events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle pause events.
         */
        class PauseEventHandler
        {
        public:
            /** Handle the event.

                @param bPauseShow
                When true, the show is paused. When false, the show is
                started again

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handlePause( bool bPauseShow ) = 0;
        };

        typedef ::boost::shared_ptr< PauseEventHandler > PauseEventHandlerSharedPtr;
    }
}

#endif /* _SLIDESHOW_PAUSEEVENTHANDLER_HXX */
