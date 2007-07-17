/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperlinkhandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:08:31 $
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

#ifndef INCLUDED_SLIDESHOW_HYPERLINKHANDLER_HXX
#define INCLUDED_SLIDESHOW_HYPERLINKHANDLER_HXX

#include <boost/shared_ptr.hpp>


/* Definition of EventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling hyperlink clicks.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle hyperlink events.
         */
        class HyperlinkHandler
        {
        public:
            /** Handle the event.

                @param rLink
                The actual hyperlink URI

                @return true, if this handler has successfully
                processed the event. When this method returns false,
                possibly other, less prioritized handlers are called,
                too.
             */
            virtual bool handleHyperlink( ::rtl::OUString const& rLink ) = 0;
        };

        typedef ::boost::shared_ptr< HyperlinkHandler > HyperlinkHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_HYPERLINKHANDLER_HXX */
