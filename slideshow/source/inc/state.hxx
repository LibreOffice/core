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
