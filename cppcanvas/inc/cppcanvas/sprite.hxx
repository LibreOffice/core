/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sprite.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:38:52 $
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

#ifndef _CPPCANVAS_SPRITE_HXX
#define _CPPCANVAS_SPRITE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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
