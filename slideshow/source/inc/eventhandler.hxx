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

#ifndef INCLUDED_SLIDESHOW_EVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_EVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>


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
        class EventHandler
        {
        public:
            virtual ~EventHandler() {}

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

#endif /* INCLUDED_SLIDESHOW_EVENTHANDLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
