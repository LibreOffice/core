/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externalmediashape.hxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: thb $ $Date: 2008/07/02 19:51:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
