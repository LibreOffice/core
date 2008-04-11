/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_spritesurface.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _VCLCANVAS_SPRITESURFACE_HXX
#define _VCLCANVAS_SPRITESURFACE_HXX

#include <sal/config.h>

#include "cairo_sprite.hxx"

class Point;
class Size;
class Rectangle;

namespace cairocanvas
{
    /* Definition of SpriteSurface interface */

    class SpriteSurface
    {
    public:
        virtual ~SpriteSurface() {}

        // call this when XSprite::show() is called
        virtual void showSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::hide() is called
        virtual void hideSprite( const Sprite::ImplRef& sprite ) = 0;

        // call this when XSprite::move() is called
        virtual void moveSprite( const Sprite::ImplRef& sprite,
                                 const Point&           rOldPos,
                                 const Point&           rNewPos,
                                 const Size&            rSpriteSize ) = 0;

        // call this when some part of your sprite has changed. That
        // includes show/hide´, i.e. for show, both showSprite and
        // updateSprite must be called.
        virtual void updateSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rPos,
                                   const Rectangle&         rUpdateArea ) = 0;

    };
}

#endif /* _VCLCANVAS_SPRITESURFACE_HXX */
