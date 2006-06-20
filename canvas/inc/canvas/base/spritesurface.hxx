/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritesurface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:14:24 $
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

#ifndef INCLUDED_CANVAS_SPRITESURFACE_HXX
#define INCLUDED_CANVAS_SPRITESURFACE_HXX

#ifndef INCLUDED_CANVAS_SPRITE_HXX
#include <canvas/base/sprite.hxx>
#endif

namespace canvas
{
    /* Definition of the SpriteSurface interface */

    /** Canvas surface containing sprites

        Every canvas surface that contains sprites must implement this
        interface, when employing the canvas base framework. The
        methods provided here are used from the individual sprites to
        notify the canvas about necessary screen updates.
     */
    class SpriteSurface : public ::com::sun::star::uno::XInterface
    {
    public:
        typedef ::rtl::Reference< SpriteSurface > Reference;

        /// Sprites should call this from XSprite::show()
        virtual void showSprite( const Sprite::Reference& rSprite ) = 0;

        /// Sprites should call this from XSprite::hide()
        virtual void hideSprite( const Sprite::Reference& rSprite ) = 0;

        /// Sprites should call this from XSprite::move()
        virtual void moveSprite( const Sprite::Reference&       rSprite,
                                 const ::basegfx::B2DPoint&     rOldPos,
                                 const ::basegfx::B2DPoint&     rNewPos,
                                 const ::basegfx::B2DVector&    rSpriteSize ) = 0;

        /** Sprites should call this when some part of the content has
            changed.

            That includes show/hide, i.e. for show, both showSprite()
            and updateSprite() must be called.
        */
        virtual void updateSprite( const Sprite::Reference&     rSprite,
                                   const ::basegfx::B2DPoint&   rPos,
                                   const ::basegfx::B2DRange&   rUpdateArea ) = 0;
    };
}

#endif /* INCLUDED_CANVAS_SPRITESURFACE_HXX */
