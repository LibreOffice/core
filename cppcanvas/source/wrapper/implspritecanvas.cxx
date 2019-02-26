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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/rendering/InterpolationMode.hpp>
#include <osl/diagnose.h>

#include "implspritecanvas.hxx"
#include "implcustomsprite.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplSpriteCanvas::TransformationArbiter::TransformationArbiter() :
            maTransformation()
        {
        }

        void ImplSpriteCanvas::TransformationArbiter::setTransformation( const ::basegfx::B2DHomMatrix& rViewTransform )
        {
            maTransformation = rViewTransform;
        }


        ImplSpriteCanvas::ImplSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& rCanvas ) :
            ImplCanvas( uno::Reference< rendering::XCanvas >(rCanvas,
                                                             uno::UNO_QUERY) ),
            mxSpriteCanvas( rCanvas ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas(): Invalid canvas" );
        }

        ImplSpriteCanvas::ImplSpriteCanvas(const ImplSpriteCanvas& rOrig) :
            Canvas(),
            SpriteCanvas(),
            ImplCanvas( rOrig ),
            mxSpriteCanvas( rOrig.getUNOSpriteCanvas() ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas( const ImplSpriteCanvas& ): Invalid canvas" );

            mpTransformArbiter->setTransformation( getTransformation() );
        }

        ImplSpriteCanvas::~ImplSpriteCanvas()
        {
        }

        void ImplSpriteCanvas::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            mpTransformArbiter->setTransformation( rMatrix );

            ImplCanvas::setTransformation( rMatrix );
        }

        bool ImplSpriteCanvas::updateScreen( bool bUpdateAll ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::updateScreen(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return false;

            return mxSpriteCanvas->updateScreen( bUpdateAll );
        }

        CustomSpriteSharedPtr ImplSpriteCanvas::createCustomSprite( const ::basegfx::B2DSize& rSize ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::createCustomSprite(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return CustomSpriteSharedPtr();

            return CustomSpriteSharedPtr(
                new ImplCustomSprite( mxSpriteCanvas,
                                      mxSpriteCanvas->createCustomSprite( ::basegfx::unotools::size2DFromB2DSize(rSize) ),
                                      mpTransformArbiter ) );
        }

        CanvasSharedPtr ImplSpriteCanvas::clone() const
        {
            return SpriteCanvasSharedPtr( new ImplSpriteCanvas( *this ) );
        }

        uno::Reference< rendering::XSpriteCanvas > ImplSpriteCanvas::getUNOSpriteCanvas() const
        {
            return mxSpriteCanvas;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
