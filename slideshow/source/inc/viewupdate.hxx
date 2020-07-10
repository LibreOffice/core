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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_VIEWUPDATE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_VIEWUPDATE_HXX

#include "disposable.hxx"
#include <memory>

/* Definition of ViewUpdate interface */

namespace slideshow::internal
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

            /** Query whether updates are pending

                @return true, if updates are pending. Calling update()
                subsequently will perform the pending update then.
             */
            virtual bool needsUpdate() const = 0;
        };

        typedef ::std::shared_ptr< ViewUpdate > ViewUpdateSharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_VIEWUPDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
