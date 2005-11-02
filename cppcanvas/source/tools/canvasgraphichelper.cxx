/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasgraphichelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:42:36 $
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

#include <canvasgraphichelper.hxx>

#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP_
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif

#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#include <cppcanvas/polypolygon.hxx>
#include <tools.hxx>


using namespace ::com::sun::star;

/* Implementation of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {
        CanvasGraphicHelper::CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas ) :
            maClipPolyPolygon(),
            mpCanvas( rParentCanvas ),
            mxGraphicDevice()
        {
            OSL_ENSURE( mpCanvas.get() != NULL &&
                        mpCanvas->getUNOCanvas().is(),
                        "CanvasGraphicHelper::CanvasGraphicHelper: no valid canvas" );

            if( mpCanvas.get() != NULL &&
                mpCanvas->getUNOCanvas().is() )
            {
                mxGraphicDevice = mpCanvas->getUNOCanvas()->getDevice();
            }

            ::canvas::tools::initRenderState( maRenderState );
        }

        void CanvasGraphicHelper::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            ::canvas::tools::setRenderStateTransform( maRenderState, rMatrix );
        }

        ::basegfx::B2DHomMatrix CanvasGraphicHelper::getTransformation() const
        {
            ::basegfx::B2DHomMatrix aMatrix;
            return ::canvas::tools::getRenderStateTransform( aMatrix,
                                                             maRenderState );
        }

        void CanvasGraphicHelper::setClip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            // TODO(T3): not thread-safe. B2DPolyPolygon employs copy-on-write
            maClipPolyPolygon = rClipPoly;
            maRenderState.Clip.clear();
        }

        ::basegfx::B2DPolyPolygon CanvasGraphicHelper::getClip() const
        {
            return maClipPolyPolygon;
        }

        const rendering::RenderState& CanvasGraphicHelper::getRenderState() const
        {
            if( maClipPolyPolygon.count() && !maRenderState.Clip.is() )
            {
                uno::Reference< rendering::XCanvas > xCanvas( mpCanvas->getUNOCanvas() );
                if( !xCanvas.is() )
                    return maRenderState;

                maRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    xCanvas->getDevice(),
                    maClipPolyPolygon );
            }

            return maRenderState;
        }

        void CanvasGraphicHelper::setRGBAColor( Color::IntSRGBA aColor )
        {
            maRenderState.DeviceColor = tools::intSRGBAToDoubleSequence( mxGraphicDevice,
                                                                         aColor );
        }

        Color::IntSRGBA CanvasGraphicHelper::getRGBAColor() const
        {
            return tools::doubleSequenceToIntSRGBA( mxGraphicDevice,
                                                    maRenderState.DeviceColor );
        }

        void CanvasGraphicHelper::setCompositeOp( CompositeOp aOp )
        {
            maRenderState.CompositeOperation = aOp;
        }

        CanvasGraphic::CompositeOp CanvasGraphicHelper::getCompositeOp() const
        {
            return static_cast<CompositeOp>(maRenderState.CompositeOperation);
        }

        CanvasSharedPtr CanvasGraphicHelper::getCanvas() const
        {
            return mpCanvas;
        }

        uno::Reference< rendering::XGraphicDevice > CanvasGraphicHelper::getGraphicDevice() const
        {
            return mxGraphicDevice;
        }

    }
}
