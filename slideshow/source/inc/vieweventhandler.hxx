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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
