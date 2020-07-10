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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_PAUSEEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_PAUSEEVENTHANDLER_HXX

#include <memory>


/* Definition of PauseHandler interface */

namespace slideshow::internal
    {

        /** Interface for handling pause events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle pause events.
         */
        class PauseEventHandler
        {
        public:
            /** Handle the event.

                @param bPauseShow
                When true, the show is paused. When false, the show is
                started again

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handlePause( bool bPauseShow ) = 0;

        protected:
            ~PauseEventHandler() {}
        };

        typedef ::std::shared_ptr< PauseEventHandler > PauseEventHandlerSharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_PAUSEEVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
