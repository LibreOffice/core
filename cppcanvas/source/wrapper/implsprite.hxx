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

#ifndef _CPPCANVAS_IMPLSPRITE_HXX
#define _CPPCANVAS_IMPLSPRITE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XSprite.hpp>

#include <boost/shared_ptr.hpp>
#include <cppcanvas/sprite.hxx>

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
            virtual ~ImplSprite();

            virtual void setAlpha( const double& rAlpha );
            virtual void movePixel( const ::basegfx::B2DPoint& rNewPos );
            virtual void move( const ::basegfx::B2DPoint& rNewPos );
            virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip();

            virtual void show();
            virtual void hide();

            virtual void setPriority( double fPriority );

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XSprite >  getUNOSprite() const;

        private:
            // default: disabled copy/assignment
            ImplSprite(const ImplSprite&);
            ImplSprite& operator=( const ImplSprite& );

            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >         mxGraphicDevice;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >          mxSprite;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite >  mxAnimatedSprite;
            ImplSpriteCanvas::TransformationArbiterSharedPtr                                        mpTransformArbiter;
        };
    }
}

#endif /* _CPPCANVAS_IMPLSPRITE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
