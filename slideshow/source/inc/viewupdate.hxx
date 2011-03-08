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

#ifndef INCLUDED_SLIDESHOW_VIEWUPDATE_HXX
#define INCLUDED_SLIDESHOW_VIEWUPDATE_HXX

#include "disposable.hxx"
#include "view.hxx"
#include <boost/shared_ptr.hpp>

/* Definition of ViewUpdate interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface for something that can perform view updates.

            Use this interface for objects that perform view updates
            from time to time, e.g. slide content. Availability of
            updates can be queried, and subsequently performed.
         */
        class ViewUpdate : public Disposable
        {
        public:
            /** Perform the update action on all views

                @return true, if the update was performed
                successfully, false otherwise.
             */
            virtual bool update() = 0;

            /** Perform the update action on given view only

                @return true, if the update was performed
                successfully, false otherwise.
             */
            virtual bool update( ViewSharedPtr const& rView ) = 0;

            /** Query whether updates are pending

                @return true, if updates are pending. Calling update()
                subsequently will perform the pending update then.
             */
            virtual bool needsUpdate() const = 0;
        };

        typedef ::boost::shared_ptr< ViewUpdate > ViewUpdateSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWUPDATE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
