/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: userpaintoverlay.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:58:25 $
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

#ifndef INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX
#define INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>

#include "unoview.hxx"
#include "rgbcolor.hxx"

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>


/* Definition of UserPaintOverlay class */

namespace slideshow
{
    namespace internal
    {
        class EventMultiplexer;
        struct SlideShowContext;

        class PaintOverlayHandler;
        typedef ::boost::shared_ptr< class UserPaintOverlay > UserPaintOverlaySharedPtr;

        /** Slide overlay, which can be painted into by the user.

            This class registers itself at the EventMultiplexer,
            listening for mouse clicks and moves. When the mouse is
            dragged, a hand sketching in the selected color is shown.
        */
        class UserPaintOverlay : private boost::noncopyable
        {
        public:
            /** Create a UserPaintOverlay

                @param rStrokeColor
                Color to use for drawing

                @param nStrokeWidth
                Width of the stroked path
             */
            static UserPaintOverlaySharedPtr create( const RGBColor&         rStrokeColor,
                                                     double                  nStrokeWidth,
                                                     const SlideShowContext& rContext );
            ~UserPaintOverlay();

        private:
            UserPaintOverlay( const RGBColor&         rStrokeColor,
                              double                  nStrokeWidth,
                              const SlideShowContext& rContext );

            ::boost::shared_ptr<PaintOverlayHandler>    mpHandler;
            EventMultiplexer&                           mrMultiplexer;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX */
