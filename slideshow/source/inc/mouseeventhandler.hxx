/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_MOUSEEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_MOUSEEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star { namespace awt
{
    struct MouseEvent;
} } } }


/* Definition of MouseEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling mouse events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle mouse events.
         */
        class MouseEventHandler
        {
        public:
            virtual ~MouseEventHandler() {}

            /** Handle a mouse button pressed event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the mouse event. When this method returns
                false, possibly other, less prioritized handlers can be
                called, too.
             */
            virtual bool handleMousePressed( const ::com::sun::star::awt::MouseEvent& e ) = 0;

            /** Handle a mouse button released event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handleMouseReleased( const ::com::sun::star::awt::MouseEvent& e ) = 0;

            /** Handle a mouse entered the view event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handleMouseEntered( const ::com::sun::star::awt::MouseEvent& e ) = 0;

            /** Handle a mouse exited the view event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handleMouseExited( const ::com::sun::star::awt::MouseEvent& e ) = 0;

            /** Handle a mouse was moved with a pressed button event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handleMouseDragged( const ::com::sun::star::awt::MouseEvent& e ) = 0;

            /** Handle a mouse was moved event.

                @param e
                The mouse event that occurred. The x,y coordinates of
                the event are already transformed back to user
                coordinate space, taking the inverse transform of the
                view in which the event occurred.

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handleMouseMoved( const ::com::sun::star::awt::MouseEvent& e ) = 0;
        };

        typedef ::boost::shared_ptr< MouseEventHandler > MouseEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_MOUSEEVENTHANDLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
