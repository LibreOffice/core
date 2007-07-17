/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewlayer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:19:50 $
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

#ifndef INCLUDED_SLIDESHOW_VIEWLAYER_HXX
#define INCLUDED_SLIDESHOW_VIEWLAYER_HXX

#include <sal/config.h>
#include <boost/shared_ptr.hpp>

namespace basegfx
{
    class B1DRange;
    class B2DRange;
    class B2DVector;
    class B2DHomMatrix;
    class B2DPolyPolygon;
}
namespace cppcanvas
{
    class Canvas;
    class CustomSprite;
}


/* Definition of ViewLayer interface */

namespace slideshow
{
    namespace internal
    {
        class View;

        class ViewLayer
        {
        public:
            virtual ~ViewLayer() {}

            /** Query whether layer displays on given view.

                @return true, if this layer displays on the given
                view.
            */
            virtual bool isOnView(boost::shared_ptr<View> const& rView) const = 0;

            /** Get the associated canvas of this layer.

                The canvas returned by this method must not change, as
                long as this object is alive.
            */
            virtual boost::shared_ptr< cppcanvas::Canvas > getCanvas() const = 0;

            /** Clear the clipped view layer area

                This method clears the area inside the clip polygon,
                if none is set, the transformed unit rectangle of the
                view.
             */
            virtual void clear() const = 0;

            /** Clear the complete view

                This method clears the full view area (not only the
                transformed unit rectangle, or within the clip). If
                this ViewLayer represents the background layer, the
                whole XSlideShowView is cleared. If this ViewLayer is
                implemented using sprites (i.e. one of the upper
                layers), the sprite is cleared to fully transparent.
             */
            virtual void clearAll() const = 0;

            /** Create a sprite for this layer

                @param rSpriteSizePixel
                Sprite size in device pixel

                @param nPriority
                Sprite priority. This value determines the priority of
                this sprite, relative to all other sprites of this
                ViewLayer. The higher the priority, the closer to the
                foreground the sprite will be.

                @return the sprite, or NULL on failure (or if this
                canvas does not support sprites).
            */
            virtual boost::shared_ptr< cppcanvas::CustomSprite >
            createSprite( const basegfx::B2DVector& rSpriteSizePixel,
                          double                    nPriority ) const = 0;

            /** Set the layer priority range

                This method influences the relative priority of this
                layer, i.e. the z position in relation to other layers
                on the parent view. The higher the priority range, the
                further in front the layer resides.

                @param rRange
                Priority range, must be in the range [0,1]
            */
            virtual void setPriority( const basegfx::B1DRange& rRange ) = 0;

            /** Get the overall view transformation.

                This method should <em>not</em> simply return the
                underlying canvas' transformation, but rather provide
                a layer above that. This enables clients of the
                slideshow to set their own user space transformation
                at the canvas, whilst the slideshow adds their
                transformation on top of that. Concretely, this method
                returns the user transform (implicitely calculated
                from the setViewSize() method), combined with the view
                transformation.
            */
            virtual basegfx::B2DHomMatrix getTransformation() const = 0;

            /** Get the overall view transformation.

                Same transformation as with getTransformation(), only
                that you can safely use this one to position sprites
                on screen (no ViewLayer offsets included whatsoever).
            */
            virtual basegfx::B2DHomMatrix getSpriteTransformation() const = 0;

            /** Set clipping on this view layer.

                @param rClip
                Clip poly-polygon to set. The polygon is interpreted
                in the user coordinate system, i.e. the view layer has
                the size as given by setViewSize() on its
                corresponding View.
             */
            virtual void setClip( const basegfx::B2DPolyPolygon& rClip ) = 0;

            /** Resize this view layer.

                @param rArea
                New area to cover. The area is interpreted in the user
                coordinate system, i.e. relative to the size as given
                by setViewSize() on the corresponding View.

                @return true, if layer was actually resized (which
                invalidates its content)
             */
            virtual bool resize( const basegfx::B2DRange& rArea ) = 0;

        };

        typedef boost::shared_ptr< ViewLayer > ViewLayerSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWLAYER_HXX */
