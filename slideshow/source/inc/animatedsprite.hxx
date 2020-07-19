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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATEDSPRITE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATEDSPRITE_HXX

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include "viewlayer.hxx"

#include <optional>
#include <memory>


/* Definition of AnimatedSprite class */

namespace slideshow::internal
    {
        /** This class provides the sprite for animated shapes.

            Besides encapsulating the Canvas sprite for animated
            shapes, this class also handles dynamic sprite resizing
            and all the gory details of offset calculations and
            rounding prevention.
         */
        class AnimatedSprite
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
            AnimatedSprite(const AnimatedSprite&) = delete;
            AnimatedSprite& operator=(const AnimatedSprite&) = delete;

            /** Resize the sprite.

                @param rSpriteSizePixel
                The new size in pixel
             */
            void resize( const ::basegfx::B2DSize& rSpriteSizePixel );

            /** Set an offset for the content output in pixel

                This method offsets the output on the sprite content
                canvas by the specified amount of device pixel (for
                subsequent render operations).
             */
            void                setPixelOffset( const ::basegfx::B2DSize& rPixelOffset );

            /// Show the sprite
            void show();

            /// Hide the sprite
            void hide();

            /** Query the content canvas for the current sprite.

                Note that this method must be called
                <em>every time</em> something is rendered to the
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

        private:
            ViewLayerSharedPtr                                          mpViewLayer;

            ::cppcanvas::CustomSpriteSharedPtr                          mpSprite;
            ::basegfx::B2DSize                                          maEffectiveSpriteSizePixel;
            ::basegfx::B2DSize                                          maContentPixelOffset;

            double                                                      mnSpritePrio;
            double                                                      mnAlpha;
            ::std::optional< ::basegfx::B2DPoint >                    maPosPixel;
            ::std::optional< ::basegfx::B2DPolyPolygon >              maClip;

            bool                                                        mbSpriteVisible;
        };

        typedef ::std::shared_ptr< AnimatedSprite > AnimatedSpriteSharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATEDSPRITE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
