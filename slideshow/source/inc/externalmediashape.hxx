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
            //------------------------------------------------------------------

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
