/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
