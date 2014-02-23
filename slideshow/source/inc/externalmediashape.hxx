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

#ifndef INCLUDED_SLIDESHOW_EXTERNALMEDIASHAPE_HXX
#define INCLUDED_SLIDESHOW_EXTERNALMEDIASHAPE_HXX

#include <boost/shared_ptr.hpp>

#include "shape.hxx"


namespace slideshow
{
    namespace internal
    {
        /** Represents a shape containing media (video, sound).

            This interface adds media handling methods to a shape. It
            allows starting/stopping and pausing playback.
         */
        class ExternalMediaShape : public Shape
        {
        public:
            // Animation methods


            /** Notify the Shape that it should start with playback

                This method enters playback mode on all registered
                views. It makes the media initially visible (for videos).
             */
            virtual void play() = 0;

            /** Notify the Shape that it should stop playback

                This method leaves playback mode on all registered
                views. The media is then rewound to the start, and
            removed from screen (for videos)
             */
            virtual void stop() = 0;

            /** Notify the Shape that it should pause playback

                This method stops playback on all registered
                views. The media stays visible (for videos)
             */
            virtual void pause() = 0;

            /** Query whether the media is currently playing.
             */
            virtual bool isPlaying() const = 0;

            /** Set media time in seconds.

            @param fTime
            Time in seconds of the media time line, that should now be
            presented
             */
            virtual void setMediaTime(double fTime) = 0;
        };

        typedef ::boost::shared_ptr< ExternalMediaShape > ExternalMediaShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_EXTERNALMEDIASHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
