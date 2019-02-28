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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_CURSORMANAGER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_CURSORMANAGER_HXX

#include <sal/types.h>
#include <memory>


/* Definition of CursorManager interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling the view cursor.

            Classes implementing this interface interact with the
            View, arbitrating access to the mouse cursor shape.
         */
        class CursorManager
        {
        public:
            virtual ~CursorManager() {}

            /** Request different cursor shape.

                @param nCursorShape
                Shape ID of the new mouse cursor
             */
            virtual bool requestCursor( sal_Int16 nCursorShape ) = 0;

            /** Reset cursor to default one.

                This method resets the cursor to whatever default to
                manager deems appropriate.
             */
            virtual void resetCursor() = 0;
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_CURSORMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
