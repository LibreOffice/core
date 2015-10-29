/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_MOUSEEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_MOUSEEVENTHANDLER_HXX

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
            virtual bool handleMousePressed( const css::awt::MouseEvent& e ) = 0;

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
            virtual bool handleMouseReleased( const css::awt::MouseEvent& e ) = 0;

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
            virtual bool handleMouseDragged( const css::awt::MouseEvent& e ) = 0;

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
            virtual bool handleMouseMoved( const css::awt::MouseEvent& e ) = 0;
        };

        typedef ::boost::shared_ptr< MouseEventHandler > MouseEventHandlerSharedPtr;

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_MOUSEEVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
