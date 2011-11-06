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
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritecanvas.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D&   rSpriteSize,
                                            const SpriteCanvasRef&                          rRefDevice ) :
        mpSpriteCanvas( rRefDevice ),
        maSize( ::canvas::tools::roundUp( rSpriteSize.Width ),
                ::canvas::tools::roundUp( rSpriteSize.Height ) )
    {
        ENSURE_OR_THROW( rRefDevice.get(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        OSL_TRACE("sprite size: %d, %d",
                  ::canvas::tools::roundUp( rSpriteSize.Width ),
                  ::canvas::tools::roundUp( rSpriteSize.Height ));

        mpBufferSurface = mpSpriteCanvas->createSurface( maSize );

        maCanvasHelper.init( maSize,
                             *rRefDevice,
                             rRefDevice.get() );
        maCanvasHelper.setSurface( mpBufferSurface, true );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice );
        maSpriteHelper.setSurface( mpBufferSurface );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();
        mpBufferSurface.reset();

        // forward to parent
        CanvasCustomSpriteBaseT::disposing();
    }

    void CanvasCustomSprite::redraw( const CairoSharedPtr& pCairo,
                                     bool                  bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        redraw( pCairo, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( const CairoSharedPtr&      pCairo,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
                                     bool                       bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( pCairo,
                               rOrigOutputPos,
                               mbSurfaceDirty,
                               bBufferedUpdate );

        mbSurfaceDirty = false;
    }

    bool CanvasCustomSprite::repaint( const SurfaceSharedPtr&       pSurface,
                                      const rendering::ViewState&   viewState,
                                      const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    SurfaceSharedPtr CanvasCustomSprite::getSurface()
    {
        return mpBufferSurface;
    }

    SurfaceSharedPtr CanvasCustomSprite::createSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
        return mpSpriteCanvas->createSurface(rSize,aContent);
    }

    SurfaceSharedPtr CanvasCustomSprite::createSurface( ::Bitmap& rBitmap )
    {
        return mpSpriteCanvas->createSurface(rBitmap);
    }

    SurfaceSharedPtr CanvasCustomSprite::changeSurface( bool bHasAlpha, bool bCopyContent )
    {
        if( !bHasAlpha && !bCopyContent )
        {
            OSL_TRACE("replacing sprite background surface");

            mpBufferSurface = mpSpriteCanvas->createSurface( maSize, CAIRO_CONTENT_COLOR );
            maSpriteHelper.setSurface( mpBufferSurface );

            return mpBufferSurface;
        }

        return SurfaceSharedPtr();
    }

    OutputDevice* CanvasCustomSprite::getOutputDevice()
    {
        return mpSpriteCanvas->getOutputDevice();
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasCustomSprite"
#define SERVICE_NAME "com.sun.star.rendering.CanvasCustomSprite"

    ::rtl::OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}
