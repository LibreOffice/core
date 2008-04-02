/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_spritesurface.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:44:25 $
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
