/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spritesurface.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_CANVAS_SPRITESURFACE_HXX
#define INCLUDED_CANVAS_SPRITESURFACE_HXX

#include <canvas/base/sprite.hxx>

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
