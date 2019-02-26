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


#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <osl/diagnose.h>

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
