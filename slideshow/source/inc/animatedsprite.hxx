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

#ifndef INCLUDED_SLIDESHOW_ANIMATEDSPRITE_HXX
#define INCLUDED_SLIDESHOW_ANIMATEDSPRITE_HXX

#include <cppcanvas/customsprite.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include "viewlayer.hxx"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


/* Definition of AnimatedSprite class */

namespace slideshow
{
    namespace internal
    {
        /** This class provides the sprite for animated shapes.

            Besides encapsulating the Canvas sprite for animated
            shapes, this class also handles dynamic sprite resizing
            and all the gory details of offset calculations and
            rounding prevention.
         */
        class AnimatedSprite : private boost::noncopyable
        {
        public:
            /** Create a new AnimatedSprite, for the given metafile
                shape.

                @param rViewLayer
                The destination view layer, on which the animation should appear

                @param rSpriteSizePixel
                The overall size of the sprite in device coordinate
                space, sufficient to display all transformations,
                shape changes and clips.

                @param nSpritePrio
                Priority of the sprite. Must remain static over the
                lifetime of this object
             */
            AnimatedSprite( const ViewLayerSharedPtr&   rViewLayer,
                            const ::basegfx::B2DSize&   rSpriteSizePixel,
                            double                      nSpritePrio );

            /** Resize the sprite.

                @param rSpriteSizePixel
                The new size in pixel

                @return true, if the resize was successful. If false
                is returned, the sprite might be invalid.
             */
            bool resize( const ::basegfx::B2DSize& rSpriteSizePixel );

            /** Set an offset for the content output in pixel

                This method offsets the output on the sprite content
                canvas by the specified amount of device pixel (for
                subsequent render operations).
             */
            void                setPixelOffset( const ::basegfx::B2DSize& rPixelOffset );

            /// Retrieve current pixel offset for content output.
            ::basegfx::B2DSize  getPixelOffset() const;

            /// Show the sprite
            void show();

            /// Hide the sprite
            void hide();

            /** Query the content canvas for the current sprite.

                Note that this method must be called
                <em>everytime</em> something is rendered to the
                sprite, because XCustomSprite does not guarantee the
                validity of the canvas after a render operation.

                Furthermore, the view transformation on the returned
                canvas is already correctly setup, matching the
                associated destination canvas.
             */
            ::cppcanvas::CanvasSharedPtr getContentCanvas() const;

            /** Move the sprite in device pixel space.

                If the sprite is not yet created, this method has no
                effect.
             */
            void movePixel( const ::basegfx::B2DPoint& rNewPos );

            /** Set the alpha value of the sprite.

                If the sprite is not yet created, this method has no
                effect.
             */
            void setAlpha( double rAlpha );

            /** Set a sprite clip in user coordinate space.

                If the sprite is not yet created, this method has no
                effect.
             */
            void clip( const ::basegfx::B2DPolyPolygon& rClip );

            /** Clears a sprite clip

                If the sprite is not yet created, this method has no
                effect.
             */
            void clip();

            /** Set a sprite transformation.

                If the sprite is not yet created, this method has no
                effect.
             */
            void transform( const ::basegfx::B2DHomMatrix& rTransform );

            /** Set the sprite priority.

                The sprite priority determines the ordering of the
                sprites on screen, i.e. which sprite lies before which.

                @param rPrio
                The new sprite prio. Must be in the range [0,1]
             */
            void setPriority( double rPrio );

        private:
            ViewLayerSharedPtr                                          mpViewLayer;

            ::cppcanvas::CustomSpriteSharedPtr                          mpSprite;
            ::basegfx::B2DSize                                          maEffectiveSpriteSizePixel;
            ::basegfx::B2DSize                                          maContentPixelOffset;

            double                                                      mnSpritePrio;
            double                                                      mnAlpha;
            ::boost::optional< ::basegfx::B2DPoint >                    maPosPixel;
            ::boost::optional< ::basegfx::B2DPolyPolygon >              maClip;
            ::boost::optional< ::basegfx::B2DHomMatrix >                maTransform;

            bool                                                        mbSpriteVisible;
        };

        typedef ::boost::shared_ptr< AnimatedSprite > AnimatedSpriteSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATEDSPRITE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
