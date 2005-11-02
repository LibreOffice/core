/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implsprite.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:44:14 $
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

#ifndef _CPPCANVAS_IMPLSPRITE_HXX
#define _CPPCANVAS_IMPLSPRITE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XSPRITE_HPP_
#include <com.sun.star.rendering.XSprites.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_SPRITE_HXX
#include <cppcanvas/sprite.hxx>
#endif

#include <implspritecanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        class ImplSprite : public virtual Sprite
        {
        public:
            ImplSprite( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XSpriteCanvas >&     rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XSprite >&       rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&         rTransformArbiter );
            ImplSprite( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XSpriteCanvas >&     rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XAnimatedSprite >& rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&         rTransformArbiter );
            virtual ~ImplSprite();

            virtual void setAlpha( const double& rAlpha );
            virtual void movePixel( const ::basegfx::B2DPoint& rNewPos );
            virtual void move( const ::basegfx::B2DPoint& rNewPos );
            virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );

            virtual void show();
            virtual void hide();

            virtual void setPriority( double fPriority );

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XSprite >  getUNOSprite() const;

            ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XGraphicDevice >
                                                                getGraphicDevice() const;

        private:
            // default: disabled copy/assignment
            ImplSprite(const ImplSprite&);
            ImplSprite& operator=( const ImplSprite& );

            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >         mxGraphicDevice;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >          mxSprite;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite >  mxAnimatedSprite;
            ImplSpriteCanvas::TransformationArbiterSharedPtr                                                mpTransformArbiter;
        };
    }
}

#endif /* _CPPCANVAS_IMPLSPRITE_HXX */
