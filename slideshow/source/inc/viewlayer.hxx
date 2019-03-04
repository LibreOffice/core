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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_VIEWLAYER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_VIEWLAYER_HXX

#include <sal/config.h>
#include <memory>
#include <com/sun/star/geometry/IntegerSize2D.hpp>

namespace basegfx
{
    class B1DRange;
    class B2DRange;
    class B2DVector;
    class B2DHomMatrix;
    class B2DPolyPolygon;
    typedef B2DVector B2DSize;
}
namespace cppcanvas
{
    class Canvas;
    class CustomSprite;
    typedef std::shared_ptr< Canvas > CanvasSharedPtr;
    typedef std::shared_ptr< ::cppcanvas::CustomSprite > CustomSpriteSharedPtr;
}


/* Definition of ViewLayer interface */

namespace slideshow
{
    namespace internal
    {
        class View;
        typedef std::shared_ptr< View > ViewSharedPtr;

        class ViewLayer
        {
        public:
            virtual ~ViewLayer() {}

            /** Query whether layer displays on given view.

                @return true, if this layer displays on the given
                view.
            */
            virtual bool isOnView(ViewSharedPtr const& rView) const = 0;

            /** Get the associated canvas of this layer.

                The canvas returned by this method must not change, as
                long as this object is alive.
            */
            virtual cppcanvas::CanvasSharedPtr getCanvas() const = 0;

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
            virtual cppcanvas::CustomSpriteSharedPtr
            createSprite( const basegfx::B2DSize& rSpriteSizePixel,
                          double                  nPriority ) const = 0;

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
                returns the user transform (implicitly calculated
                from the setViewSize() method), combined with the view
                transformation.
            */
            virtual basegfx::B2DHomMatrix getTransformation() const = 0;

            virtual css::geometry::IntegerSize2D getTranslationOffset() const = 0;

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

        typedef std::shared_ptr< ViewLayer > ViewLayerSharedPtr;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_VIEWLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
