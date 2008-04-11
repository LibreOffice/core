/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasgraphichelper.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CPPCANVAS_CANVASGRAPHICHELPER_HXX
#define _CPPCANVAS_CANVASGRAPHICHELPER_HXX

#include <com/sun/star/rendering/RenderState.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <cppcanvas/canvasgraphic.hxx>

#include <boost/optional.hpp>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XGraphicDevice;
} } } }


/* Definition of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {

        class CanvasGraphicHelper : public virtual CanvasGraphic
        {
        public:
            CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas );

            // CanvasGraphic implementation
            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual ::basegfx::B2DHomMatrix          getTransformation() const;
            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void                             setClip();
            virtual ::basegfx::B2DPolyPolygon const* getClip() const;
            virtual void                             setRGBAColor( Color::IntSRGBA );
            virtual Color::IntSRGBA                  getRGBAColor() const;
            virtual void                             setCompositeOp( CompositeOp aOp );
            virtual CompositeOp                      getCompositeOp() const;

        protected:
            // for our clients
            // ===============
            CanvasSharedPtr                                                                 getCanvas() const;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > getGraphicDevice() const;
            const ::com::sun::star::rendering::RenderState&                                 getRenderState() const;

        private:
            mutable ::com::sun::star::rendering::RenderState                                maRenderState;

            boost::optional<basegfx::B2DPolyPolygon>                                        maClipPolyPolygon;
            CanvasSharedPtr                                                                 mpCanvas;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > mxGraphicDevice;
        };

    }
}

#endif /* _CPPCANVAS_CANVASGRAPHICHELPER_HXX */
