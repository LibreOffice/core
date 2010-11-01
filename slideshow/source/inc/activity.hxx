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

#ifndef INCLUDED_SLIDESHOW_ACTIVITY_HXX
#define INCLUDED_SLIDESHOW_ACTIVITY_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>

#include "disposable.hxx"


/* Definition of Activity interface */

namespace slideshow
{
    namespace internal
    {

        class Activity : public Disposable
        {
        public:
            /** Perform the activity associated with this interface's
                implementation.

                @return true, if activity continues, or false, if activity has
                ended.
            */
            virtual bool perform() = 0;

            /** Calculates whether the activity lags time.

                If this method returns a time lag greater than 0.0,
                the ActivitiesQueue will adjust the global slideshow
                time, by subtracting the given amount of lag.

                @return time lag or 0.0. Value must be greater or
                equal than zero.
            */
            virtual double calcTimeLag() const = 0;

            /** Query whether this activity is still continuing

                @return true, if this activity still
                continues. Returns false, if activity has ended. It is
                required that operator() returns false, when
                isActive() returns false. Furthermore, it is required
                that the inactive state is persistent; an activity
                that has become inactive (i.e. isActive() once
                returned false) must stay in that state eternally.
            */
            virtual bool isActive() const = 0;

            /** Notifies the Activity that it has now left the
                ActivitiesQueue

                Use this method to react on the queue removal
                event. For animated shapes, this is e.g. used to
                switch back to the non-sprite presentation mode of the
                shape.
             */
            virtual void dequeued() = 0;

            /** Forces this activity deactivate and get to its end state
                (if possible), but does _not_ dispose.
            */
            virtual void end() = 0;
        };

        typedef ::boost::shared_ptr< Activity > ActivitySharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ACTIVITY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
