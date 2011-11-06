/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>

#include <rtl/math.hxx>

#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <tools/poly.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <utility>

#include <comphelper/sequence.hxx>
#include <canvas/canvastools.hxx>

#include "cairo_textlayout.hxx"
#include "cairo_parametricpolypolygon.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_canvasbitmap.hxx"
#include "cairo_impltools.hxx"
#include "cairo_canvasfont.hxx"

using namespace ::com::sun::star;

namespace cairocanvas
{
    namespace
    {
        bool textureFill( OutputDevice&         rOutDev,
                          GraphicObject&        rGraphic,
                          const ::Point&        rPosPixel,
                          const ::Size&         rNextTileX,
                          const ::Size&         rNextTileY,
                          sal_Int32             nTilesX,
                          sal_Int32             nTilesY,
                          const ::Size&         rTileSize,
                          const GraphicAttr&    rAttr)
        {
            bool bRet( false );
            Point   aCurrPos;
            int     nX, nY;

            for( nY=0; nY < nTilesY; ++nY )
            {
                aCurrPos.X() = rPosPixel.X() + nY*rNextTileY.Width();
                aCurrPos.Y() = rPosPixel.Y() + nY*rNextTileY.Height();

                for( nX=0; nX < nTilesX; ++nX )
                {
                    // update return value. This method should return true, if
                    // at least one of the looped Draws succeeded.
                    bRet |= rGraphic.Draw( &rOutDev,
                                           aCurrPos,
                                           rTileSize,
                                           &rAttr );

                    aCurrPos.X() += rNextTileX.Width();
                    aCurrPos.Y() += rNextTileX.Height();
                }
            }

            return bRet;
        }

        inline sal_Int32 roundDown( const double& rVal )
        {
            return static_cast< sal_Int32 >( floor( rVal ) );
        }

        inline sal_Int32 roundUp( const double& rVal )
        {
            return static_cast< sal_Int32 >( ceil( rVal ) );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas&                          rCanvas,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "CanvasHelper::fillPolyPolygon(): polygon is NULL");
        ENSURE_ARG_OR_THROW( textures.getLength(),
                         "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

    cairo_save( mpCairo );

    useStates( viewState, renderState, true );
    mpTextures = &textures;
    drawPolyPolygonPath( xPolyPolygon, Fill );
    mpTextures = NULL;

    cairo_restore( mpCairo );

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }
}
