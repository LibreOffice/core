/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vieweventhandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:19:37 $
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

#ifndef INCLUDED_SLIDESHOW_VIEWEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_VIEWEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "unoview.hxx"


/* Definition of ViewEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling view events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle view events.
         */
        class ViewEventHandler
        {
        public:
            virtual ~ViewEventHandler() {}

            /** Notify new view.

                @param rView
                The newly added view
             */
            virtual void viewAdded( const UnoViewSharedPtr& rView ) = 0;

            /** Notify removed view.

                @param rView
                The removed view
             */
            virtual void viewRemoved( const UnoViewSharedPtr& rView ) = 0;

            /** Notify changed view.

                Reasons for a viewChanged notification can be
                different view size, transformation, or other device
                properties (color resolution or profile, etc.)

                @param rView
                The changed view
             */
            virtual void viewChanged( const UnoViewSharedPtr& rView ) = 0;

            /** Notify that all views changed.

                Reasons for a viewChanged notification can be
                different view size, transformation, or other device
                properties (color resolution or profile, etc.)

                Note that this method avoids hidden inefficiencies
                (O(n^2) behaviour when viewChanged() needs to perform
                linear searches)
             */
            virtual void viewsChanged() = 0;
        };

        typedef ::boost::shared_ptr< ViewEventHandler > ViewEventHandlerSharedPtr;
        typedef ::boost::weak_ptr< ViewEventHandler >   ViewEventHandlerWeakPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWEVENTHANDLER_HXX */
