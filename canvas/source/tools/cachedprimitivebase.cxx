/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedprimitivebase.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/base/cachedprimitivebase.hxx>

#include <com/sun/star/rendering/RepaintResult.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>


using namespace ::com::sun::star;

#define IMPLEMENTATION_NAME "canvas::CachedPrimitiveBase"
#define SERVICE_NAME "com.sun.star.rendering.CachedBitmap"

namespace canvas
{
    CachedPrimitiveBase::CachedPrimitiveBase( const rendering::ViewState&                   rUsedViewState,
                                              const uno::Reference< rendering::XCanvas >&   rTarget,
                                              bool                                          bFailForChangedViewTransform ) :
        CachedPrimitiveBase_Base( m_aMutex ),
        maUsedViewState( rUsedViewState ),
        mxTarget( rTarget ),
        mbFailForChangedViewTransform( bFailForChangedViewTransform )
    {
    }

    CachedPrimitiveBase::~CachedPrimitiveBase()
    {
    }

    void SAL_CALL CachedPrimitiveBase::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maUsedViewState.Clip.clear();
        mxTarget.clear();
    }

    sal_Int8 SAL_CALL CachedPrimitiveBase::redraw( const rendering::ViewState& aState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::basegfx::B2DHomMatrix aUsedTransformation;
        ::basegfx::B2DHomMatrix aNewTransformation;

        ::basegfx::unotools::homMatrixFromAffineMatrix( aUsedTransformation,
                                                        maUsedViewState.AffineTransform );
        ::basegfx::unotools::homMatrixFromAffineMatrix( aNewTransformation,
                                                        aState.AffineTransform );

        const bool bSameViewTransforms( aUsedTransformation == aNewTransformation );

        if( mbFailForChangedViewTransform &&
            !bSameViewTransforms )
        {
            // differing transformations, don't try to draft the
            // output, just plain fail here.
            return rendering::RepaintResult::FAILED;
        }

        return doRedraw( aState,
                         maUsedViewState,
                         mxTarget,
                         bSameViewTransforms );
    }

    ::rtl::OUString SAL_CALL CachedPrimitiveBase::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CachedPrimitiveBase::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CachedPrimitiveBase::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}
