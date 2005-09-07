/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedbitmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:16:39 $
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

#include <canvas/debug.hxx>

#include "cachedbitmap.hxx"
#include "repainttarget.hxx"

#ifndef _COM_SUN_STAR_RENDERING_REPAINTRESULT_HPP_
#include <com/sun/star/rendering/RepaintResult.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif


using namespace ::com::sun::star;


#define IMPLEMENTATION_NAME "VCLCanvas::CachedBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CachedBitmap"

namespace vclcanvas
{
    CachedBitmap::CachedBitmap( const GraphicObjectSharedPtr&               rGraphicObject,
                                const ::Point&                              rPoint,
                                const ::Size&                               rSize,
                                const GraphicAttr&                          rAttr,
                                const rendering::ViewState&                 rUsedViewState,
                                const uno::Reference< rendering::XCanvas >& rTarget ) :
        CachedBitmap_Base( m_aMutex ),
        mpGraphicObject( rGraphicObject ),
        maPoint( rPoint ),
        maSize( rSize ),
        maAttributes( rAttr ),
        maUsedViewState( rUsedViewState ),
        mxTarget( rTarget )
    {
    }

    CachedBitmap::~CachedBitmap()
    {
    }

    void SAL_CALL CachedBitmap::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maUsedViewState.Clip.clear();
        mpGraphicObject.reset();
        mxTarget.clear();
    }

    sal_Int8 SAL_CALL CachedBitmap::redraw( const rendering::ViewState& aState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::basegfx::B2DHomMatrix aUsedTransformation;
        ::basegfx::B2DHomMatrix aNewTransformation;

        ::basegfx::unotools::homMatrixFromAffineMatrix( aUsedTransformation,
                                                        maUsedViewState.AffineTransform );
        ::basegfx::unotools::homMatrixFromAffineMatrix( aNewTransformation,
                                                        aState.AffineTransform );

        if( aUsedTransformation != aNewTransformation )
        {
            // differing transformations, don't try to draft the
            // output, just plain fail here.
            return rendering::RepaintResult::FAILED;
        }

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(mxTarget.get());

        ENSURE_AND_THROW( pTarget,
                          "CachedBitmap::redraw(): cannot cast target to RepaintTarget" );


        if( !pTarget->repaint( mpGraphicObject,
                               maPoint,
                               maSize,
                               maAttributes ) )
        {
            // target failed to repaint
            return rendering::RepaintResult::FAILED;
        }

        return rendering::RepaintResult::REDRAWN;
    }

    ::rtl::OUString SAL_CALL CachedBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CachedBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CachedBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}
