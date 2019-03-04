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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SLIDESHOWCONTEXT_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SLIDESHOWCONTEXT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <memory>

namespace com{ namespace sun{ namespace star{ namespace uno
{
    class XComponentContext;
}}}}


namespace slideshow
{
    namespace internal
    {
        class ShapeManager;
        class EventQueue;
        class ActivitiesQueue;
        class UserEventQueue;
        class EventMultiplexer;
        class ScreenUpdater;
        class UnoViewContainer;
        class CursorManager;
        class MediaFileManager;
        class SubsettableShapeManager;
        typedef ::std::shared_ptr< SubsettableShapeManager > SubsettableShapeManagerSharedPtr;

        /** Common arguments for slideshow objects.

            This struct combines a number of object references
            ubiquitously needed throughout the slideshow.
         */
        struct SlideShowContext
        {
            /** Common context for node creation

                @param rShapeManager
                ShapeManager, which handles all shapes

                @param rEventQueue
                Event queue, where time-based events are to be
                scheduled. A node must not schedule events there
                before it's not resolved.

                @param rEventMultiplexer
                Event multiplexer. Clients can register there for
                about any event that happens in the slideshow

                @param rScreenUpdater
                Screen updater. Gets notified of necessary screen
                updates.

                @param rActivitiesQueue
                Activities queue, where repeating activities are
                to be scheduled.

                @param rMediaFileManager
                To handle media file with package urls.

                @param rUserEventQueue
                User event queue

                @param rViewContainer
                Holds all views added to slideshow

                @param rComponentContext
                To create UNO services from
            */
            SlideShowContext( SubsettableShapeManagerSharedPtr&                 rSubsettableShapeManager,
                              EventQueue&                                       rEventQueue,
                              EventMultiplexer&                                 rEventMultiplexer,
                              ScreenUpdater&                                    rScreenUpdater,
                              ActivitiesQueue&                                  rActivitiesQueue,
                              UserEventQueue&                                   rUserEventQueue,
                              CursorManager&                                    rCursorManager,
                              MediaFileManager&                                 rMediaFileManager,
                              const UnoViewContainer&                           rViewContainer,
                              const css::uno::Reference< css::uno::XComponentContext>&    rComponentContext );
            void dispose();

            std::shared_ptr<SubsettableShapeManager>&     mpSubsettableShapeManager;
            EventQueue&                                     mrEventQueue;
            EventMultiplexer&                               mrEventMultiplexer;
            ScreenUpdater&                                  mrScreenUpdater;
            ActivitiesQueue&                                mrActivitiesQueue;
            UserEventQueue&                                 mrUserEventQueue;
            CursorManager&                                  mrCursorManager;
            MediaFileManager&                               mrMediaFileManager;
            const UnoViewContainer&                         mrViewContainer;
            css::uno::Reference< css::uno::XComponentContext>   mxComponentContext;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SLIDESHOWCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
