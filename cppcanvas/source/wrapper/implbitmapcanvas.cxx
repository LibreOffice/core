/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_cppcanvas.hxx"

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include "implbitmapcanvas.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplBitmapCanvas::ImplBitmapCanvas( const uno::Reference< rendering::XBitmapCanvas >& rCanvas ) :
            ImplCanvas( uno::Reference< rendering::XCanvas >(rCanvas,
                                                             uno::UNO_QUERY) ),
            mxBitmapCanvas( rCanvas ),
            mxBitmap( rCanvas,
                      uno::UNO_QUERY )
        {
            OSL_ENSURE( mxBitmapCanvas.is(), "ImplBitmapCanvas::ImplBitmapCanvas(): Invalid canvas" );
            OSL_ENSURE( mxBitmap.is(), "ImplBitmapCanvas::ImplBitmapCanvas(): Invalid bitmap" );
        }

        ImplBitmapCanvas::~ImplBitmapCanvas()
        {
        }

        ::basegfx::B2ISize ImplBitmapCanvas::getSize() const
        {
            OSL_ENSURE( mxBitmap.is(), "ImplBitmapCanvas::getSize(): Invalid canvas" );
            return ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBitmap->getSize() );
        }

        CanvasSharedPtr ImplBitmapCanvas::clone() const
        {
            return BitmapCanvasSharedPtr( new ImplBitmapCanvas( *this ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
