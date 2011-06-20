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

#ifndef INCLUDED_SLIDESHOW_STATE_HXX
#define INCLUDED_SLIDESHOW_STATE_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


/* Definition of State interface */

namespace slideshow
{
    namespace internal
    {

        /** This interface represents a stateful object.

            The state ID returned by the getStateId() method
            abstractly encodes the object's state. When this ID
            changes, clients can assume that the object's state has
            changed.
         */
        class State
        {
        public:
            virtual ~State() {}

            /// Abstract, numerically encoded state ID
            typedef ::std::size_t StateId;

            /** This method returns a numerical state identifier.

                The state ID returned by this method abstractly
                encodes the object's state. When this ID changes,
                clients can assume that the object's state has
                changed.

                @return an abstract, numerical state ID.
             */
            virtual StateId getStateId() const = 0;
        };

        typedef ::boost::shared_ptr< State > StateSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_STATE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
