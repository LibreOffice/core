/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:52:24 $
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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include "dx_canvascustomsprite.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_impltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D&   rSpriteSize,
                                            const SpriteCanvasRef&                          rRefDevice,
                                            const IDXRenderModuleSharedPtr&                 rRenderModule,
                                            const ::canvas::ISurfaceProxyManagerSharedPtr&  rSurfaceProxy,
                                            bool                                            bShowSpriteBounds ) :
        mpSpriteCanvas( rRefDevice )
    {
        ENSURE_AND_THROW( rRefDevice.get(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        DXBitmapSharedPtr pBitmap(
            new DXBitmap(
                ::basegfx::B2IVector(
                    ::canvas::tools::roundUp( rSpriteSize.Width ),
                    ::canvas::tools::roundUp( rSpriteSize.Height )),
                rSurfaceProxy,
                rRenderModule,
                true));

        maCanvasHelper.setDevice( *rRefDevice.get() );
        maCanvasHelper.setTarget( pBitmap );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice,
                             rRenderModule,
                             maCanvasHelper.getTarget(),
                             bShowSpriteBounds );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteBaseT::disposing();
    }

#define IMPLEMENTATION_NAME "DXCanvas.CanvasCustomSprite"
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

    void CanvasCustomSprite::redraw() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( mbSurfaceDirty );
    }
}
