/*************************************************************************
 *
 *  $RCSfile: canvasgraphichelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:57:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvasgraphichelper.hxx>

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP_
#include <drafts/com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
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
using namespace ::drafts::com::sun::star;

/* Implementation of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {
        CanvasGraphicHelper::CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas ) :
            mpClipPolyPolygon(),
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

        void CanvasGraphicHelper::setClip( const PolyPolygonSharedPtr& rClipPoly )
        {
            mpClipPolyPolygon = rClipPoly;

            if( rClipPoly.get() )
                maRenderState.Clip = rClipPoly->getUNOPolyPolygon();
            else
                maRenderState.Clip.clear();
        }

        PolyPolygonSharedPtr CanvasGraphicHelper::getClip() const
        {
            return mpClipPolyPolygon;
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
