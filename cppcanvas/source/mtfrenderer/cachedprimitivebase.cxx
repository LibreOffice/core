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



#include <com/sun/star/rendering/RepaintResult.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <canvas/canvastools.hxx>
#include <cppcanvas/canvas.hxx>

#include "cachedprimitivebase.hxx"

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        CachedPrimitiveBase::CachedPrimitiveBase( const CanvasSharedPtr& rCanvas,
                                                  bool                   bOnlyRedrawWithSameTransform ) :
            mpCanvas( rCanvas ),
            mxCachedPrimitive(),
            maLastTransformation(),
            mbOnlyRedrawWithSameTransform( bOnlyRedrawWithSameTransform )
        {
            // TODO(F2): also store last view transform, and refuse to
            // redraw if changed.
        }

        bool CachedPrimitiveBase::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::CachedPrimitiveBase::render()" );
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::CachedPrimitiveBase: 0x" << std::hex << this );

            const rendering::ViewState& rViewState( mpCanvas->getViewState() );
            ::basegfx::B2DHomMatrix     aTotalTransform;

            ::canvas::tools::getViewStateTransform( aTotalTransform,
                                                    rViewState );
            aTotalTransform *= rTransformation;

            // can we use the cached primitive? For that, it must be
            // present in the first place, and, if
            // mbOnlyRedrawWithSameTransform is true, the overall
            // transformation must be the same.
            if( mxCachedPrimitive.is() &&
                (!mbOnlyRedrawWithSameTransform ||
                 maLastTransformation == aTotalTransform) )
            {
                if( mxCachedPrimitive->redraw( rViewState ) ==
                    rendering::RepaintResult::REDRAWN )
                {
                    // cached repaint succeeded, done.
                    return true;
                }
            }

            maLastTransformation = aTotalTransform;

            // delegate rendering to derived classes
            return renderPrimitive( mxCachedPrimitive,
                                    rTransformation );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
