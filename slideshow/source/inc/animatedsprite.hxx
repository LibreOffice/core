/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatedsprite.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:04:32 $
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

#ifndef _SLIDESHOW_ANIMATEDSPRITE_HXX
#define _SLIDESHOW_ANIMATEDSPRITE_HXX

#ifndef _CPPCANVAS_CUSTOMSPRITE_HXX
#include <cppcanvas/customsprite.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#include <comphelper/optionalvalue.hxx>
#endif

#include <viewlayer.hxx>


/* Definition of AnimatedSprite class */

namespace presentation
{
    namespace internal
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
             */
            AnimatedSprite( const ViewLayerSharedPtr&   rViewLayer,
                            const ::basegfx::B2DSize&   rSpriteSizePixel );

            ~AnimatedSprite();

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

            /** Move the sprite in user coordinate space.

                If the sprite is not yet created, this method has no
                effect.
             */
            void move( const ::basegfx::B2DPoint& rNewPos );

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
            // default: disabled copy/assignment
            AnimatedSprite(const AnimatedSprite&);
            AnimatedSprite& operator=( const AnimatedSprite& );

            ViewLayerSharedPtr                                          mpViewLayer;

            ::cppcanvas::CustomSpriteSharedPtr                          mpSprite;
            ::basegfx::B2DSize                                          maEffectiveSpriteSizePixel;
            ::basegfx::B2DSize                                          maContentPixelOffset;

            double                                                      mnAlpha;
            ::comphelper::OptionalValue< ::basegfx::B2DPoint >          maPosPixel;
            ::comphelper::OptionalValue< ::basegfx::B2DPoint >          maPos;
            ::comphelper::OptionalValue< ::basegfx::B2DPolyPolygon >    maClip;
            ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >      maTransform;

            bool                                                        mbSpriteVisible;
        };

        typedef ::boost::shared_ptr< AnimatedSprite > AnimatedSpriteSharedPtr;

    }
}

#endif /* _SLIDESHOW_ANIMATEDSPRITE_HXX */
