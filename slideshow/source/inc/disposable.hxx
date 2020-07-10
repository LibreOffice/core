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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_DISPOSABLE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_DISPOSABLE_HXX

#include <memory>


/* Definition of Disposable interface */

namespace slideshow::internal
    {
        /**
         * Base class for being a shared pointer, since quite a few of the downstream classes
         * want to be stored using std::shared_ptr.
         */
        class SharedPtrAble : public std::enable_shared_from_this<SharedPtrAble>
        {
        public:
            virtual ~SharedPtrAble() {}
        };

        /** Disposable interface

            With ref-counted objects, deleting object networks
            containing cycles requires a dispose() call, to enforce
            every object to call dispose on and release local
            references.
         */
        class Disposable
        {
        public:
            virtual ~Disposable() {}

            /** Dispose all object references.

                An implementor of this method must first call
                dispose() on any of its external references, and
                release them after that.
             */
            virtual void dispose() = 0;
        };

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_DISPOSABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
