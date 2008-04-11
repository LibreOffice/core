/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sprite.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _CPPCANVAS_SPRITE_HXX
#define _CPPCANVAS_SPRITE_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
    class B2DPoint;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XSprite;
} } } }


/* Definition of Sprite class */

namespace cppcanvas
{

    class Sprite
    {
    public:
        virtual ~Sprite() {}

        virtual void setAlpha( const double& rAlpha ) = 0;

        /** Set the sprite position on screen

            This method differs from the XSprite::move() insofar, as
            no viewstate/renderstate transformations are applied to
            the specified position. The given position is interpreted
            in device coordinates (i.e. screen pixel)
         */
        virtual void movePixel( const ::basegfx::B2DPoint& rNewPos ) = 0;

        /** Set the sprite position on screen

            This method sets the sprite position in the view
            coordinate system of the parent canvas
         */
        virtual void move( const ::basegfx::B2DPoint& rNewPos ) = 0;

        virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;

        /** Set output clipping

            This method differs from the XSprite::clip() insofar, as
            no viewstate/renderstate transformations are applied to
            the specified clip polygon. The given polygon is
            interpreted in device coordinates (i.e. screen pixel)
         */
        virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;

        /** Set output clipping

            This method applies the clip poly-polygon interpreted in
            the view coordinate system of the parent canvas.
         */
        virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;

        virtual void setClip() = 0;

        virtual void show() = 0;
        virtual void hide() = 0;

        /** Change the sprite priority

            @param fPriority
            New sprite priority. The higher the priority, the further
            towards the viewer the sprite appears. That is, sprites
            with higher priority appear before ones with lower
            priority.
         */
        virtual void setPriority( double fPriority ) = 0;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XSprite > getUNOSprite() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Sprite > SpriteSharedPtr;
}

#endif /* _CPPCANVAS_SPRITE_HXX */
