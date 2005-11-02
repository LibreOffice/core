/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasgraphichelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:39:11 $
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

#ifndef _CPPCANVAS_CANVASGRAPHICHELPER_HXX
#define _CPPCANVAS_CANVASGRAPHICHELPER_HXX

#ifndef _COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#include <cppcanvas/canvasgraphic.hxx>

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
            virtual void                        setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual ::basegfx::B2DHomMatrix     getTransformation() const;
            virtual void                        setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual ::basegfx::B2DPolyPolygon   getClip() const;
            virtual void                        setRGBAColor( Color::IntSRGBA );
            virtual Color::IntSRGBA             getRGBAColor() const;
            virtual void                        setCompositeOp( CompositeOp aOp );
            virtual CompositeOp                 getCompositeOp() const;

        protected:
            // for our clients
            // ===============
            CanvasSharedPtr                                                                 getCanvas() const;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > getGraphicDevice() const;
            const ::com::sun::star::rendering::RenderState&                                 getRenderState() const;

        private:
            mutable ::com::sun::star::rendering::RenderState                                maRenderState;

            ::basegfx::B2DPolyPolygon                                                       maClipPolyPolygon;
            CanvasSharedPtr                                                                 mpCanvas;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > mxGraphicDevice;
        };

    }
}

#endif /* _CPPCANVAS_CANVASGRAPHICHELPER_HXX */
