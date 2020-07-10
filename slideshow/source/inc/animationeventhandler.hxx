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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONEVENTHANDLER_HXX

#include <memory>
#include "animationnode.hxx"


/* Definition of AnimationEventHandler interface */

namespace slideshow::internal
    {

        /** Interface for handling animation events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle animation state change events.
         */
        class AnimationEventHandler
        {
        public:
            virtual ~AnimationEventHandler() {}

            /** Handle the event.

                @param rNode
                Animation node which caused this event to fire

                @return true, if this handler has successfully
                processed the animation event. When this method
                returns false, possibly other, less prioritized
                handlers are called, too.
             */
            virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode ) = 0;
        };

        typedef ::std::shared_ptr< AnimationEventHandler > AnimationEventHandlerSharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONEVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
