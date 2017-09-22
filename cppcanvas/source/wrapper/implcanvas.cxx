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


#include <rtl/ustring.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/utils/canvastools.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include <canvas/canvastools.hxx>
#include <cppcanvas/polypolygon.hxx>

#include "implfont.hxx"
#include "implcolor.hxx"
#include "implcanvas.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplCanvas::ImplCanvas( const uno::Reference< rendering::XCanvas >& xCanvas ) :
            maViewState(),
            maClipPolyPolygon(),
            mxCanvas( xCanvas )
        {
            OSL_ENSURE( mxCanvas.is(), "Canvas::Canvas() invalid XCanvas" );

            ::canvas::tools::initViewState( maViewState );
        }

        ImplCanvas::~ImplCanvas()
        {
        }

        void ImplCanvas::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            ::canvas::tools::setViewStateTransform( maViewState, rMatrix );
        }

        ::basegfx::B2DHomMatrix ImplCanvas::getTransformation() const
        {
            ::basegfx::B2DHomMatrix aMatrix;
            return ::canvas::tools::getViewStateTransform( aMatrix,
                                                           maViewState );
        }

        void ImplCanvas::setClip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            // TODO(T3): not thread-safe. B2DPolyPolygon employs copy-on-write
            maClipPolyPolygon.reset( rClipPoly );
            maViewState.Clip.clear();
        }

        void ImplCanvas::setClip()
        {
            maClipPolyPolygon.reset();
            maViewState.Clip.clear();
        }

        ::basegfx::B2DPolyPolygon const* ImplCanvas::getClip() const
        {
            return !maClipPolyPolygon ? nullptr : &(*maClipPolyPolygon);
        }

        ColorSharedPtr ImplCanvas::createColor() const
        {
            return ColorSharedPtr( new ImplColor( getUNOCanvas()->getDevice() ) );
        }

        CanvasSharedPtr ImplCanvas::clone() const
        {
            return CanvasSharedPtr( new ImplCanvas( *this ) );
        }

        void ImplCanvas::clear() const
        {
            OSL_ENSURE( mxCanvas.is(), "ImplCanvas::clear(): Invalid XCanvas" );
            mxCanvas->clear();
        }

        uno::Reference< rendering::XCanvas > ImplCanvas::getUNOCanvas() const
        {
            OSL_ENSURE( mxCanvas.is(), "ImplCanvas::getUNOCanvas(): Invalid XCanvas" );

            return mxCanvas;
        }

        rendering::ViewState ImplCanvas::getViewState() const
        {
            if( maClipPolyPolygon && !maViewState.Clip.is() )
            {
                if( !mxCanvas.is() )
                    return maViewState;

                maViewState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    mxCanvas->getDevice(),
                    *maClipPolyPolygon );
            }

            return maViewState;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
