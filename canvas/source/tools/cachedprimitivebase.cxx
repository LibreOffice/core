/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <canvas/base/cachedprimitivebase.hxx>


using namespace ::com::sun::star;

namespace canvas
{
    CachedPrimitiveBase::CachedPrimitiveBase( const rendering::ViewState&                   rUsedViewState,
                                              const uno::Reference< rendering::XCanvas >&   rTarget ) :
        CachedPrimitiveBase_Base( m_aMutex ),
        maUsedViewState( rUsedViewState ),
        mxTarget( rTarget )
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

    sal_Int8 SAL_CALL CachedPrimitiveBase::redraw( const rendering::ViewState& aState )
    {
        ::basegfx::B2DHomMatrix aUsedTransformation;
        ::basegfx::B2DHomMatrix aNewTransformation;

        ::basegfx::unotools::homMatrixFromAffineMatrix( aUsedTransformation,
                                                        maUsedViewState.AffineTransform );
        ::basegfx::unotools::homMatrixFromAffineMatrix( aNewTransformation,
                                                        aState.AffineTransform );

        const bool bSameViewTransforms( aUsedTransformation == aNewTransformation );

        if( !bSameViewTransforms )
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

    OUString SAL_CALL CachedPrimitiveBase::getImplementationName(  )
    {
        return OUString("canvas::CachedPrimitiveBase");
    }

    sal_Bool SAL_CALL CachedPrimitiveBase::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL CachedPrimitiveBase::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.CachedBitmap" };
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
