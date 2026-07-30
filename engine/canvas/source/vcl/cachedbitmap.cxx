/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

#include <cachedbitmap.hxx>
#include <repainttarget.hxx>


using namespace ::com::sun::star;

namespace vclcanvas
{
    CachedBitmap::CachedBitmap( GraphicObjectSharedPtr                      xGraphicObject,
                                const ::Point&                              rPoint,
                                const ::Size&                               rSize,
                                const GraphicAttr&                          rAttr,
                                const vclcanvas::ViewState&                 rUsedViewState,
                                rendering::RenderState                      aUsedRenderState,
                                const uno::Reference< vclcanvas::XCanvas >& rTarget ) :
        CachedBitmap_Base(),
        maUsedViewState( rUsedViewState ),
        mxTarget( rTarget ),
        mpGraphicObject(std::move( xGraphicObject )),
        maRenderState(std::move(aUsedRenderState)),
        maPoint( rPoint ),
        maSize( rSize ),
        maAttributes( rAttr )
    {
    }

    void CachedBitmap::disposing(std::unique_lock<std::mutex>& )
    {
        mpGraphicObject.reset();
        mxTarget.clear();
    }

    sal_Int8 CachedBitmap::redraw( const vclcanvas::ViewState& aState )
    {
        ::basegfx::B2DHomMatrix aUsedTransformation;
        ::basegfx::B2DHomMatrix aNewTransformation;

        ::basegfx::unotools::homMatrixFromAffineMatrix( aUsedTransformation,
                                                        maUsedViewState.AffineTransform );
        ::basegfx::unotools::homMatrixFromAffineMatrix( aNewTransformation,
                                                        aState.AffineTransform );

        const bool bSameViewTransform( aUsedTransformation == aNewTransformation );

        if( !bSameViewTransform )
        {
            // differing transformations, don't try to draft the
            // output, just plain fail here.
            return rendering::RepaintResult::FAILED;
        }

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(mxTarget.get());

        ENSURE_OR_THROW( pTarget,
                          "CachedBitmap::redraw(): cannot cast target to RepaintTarget" );

        if( !pTarget->repaint( mpGraphicObject,
                               aState,
                               maRenderState,
                               maPoint,
                               maSize,
                               maAttributes ) )
        {
            // target failed to repaint
            return rendering::RepaintResult::FAILED;
        }

        return rendering::RepaintResult::REDRAWN;
    }

    OUString CachedBitmap::getImplementationName(  )
    {
        return u"canvas::CachedPrimitiveBase"_ustr;
    }

    bool CachedBitmap::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    cpo::uno::Sequence< OUString > CachedBitmap::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.rendering.CachedBitmap"_ustr };
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
