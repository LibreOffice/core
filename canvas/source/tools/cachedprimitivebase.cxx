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
