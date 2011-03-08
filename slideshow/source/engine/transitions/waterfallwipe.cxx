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
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "waterfallwipe.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

WaterfallWipe::WaterfallWipe( sal_Int32 nElements, bool flipOnYAxis )
    : m_flipOnYAxis( flipOnYAxis )
{
    const sal_Int32 sqrtElements = static_cast<sal_Int32>(
        sqrt( static_cast<double>(nElements) ) );
    const double elementEdge = (1.0 / sqrtElements);
    m_waterfall.append( ::basegfx::B2DPoint( 0.0, -1.0 ) );
    for ( sal_Int32 pos = sqrtElements; pos--; )
    {
        const sal_Int32 xPos = (sqrtElements - pos - 1);
        const double yPos = ::basegfx::pruneScaleValue( ((pos + 1) * elementEdge) - 1.0 );
        m_waterfall.append( ::basegfx::B2DPoint(
                                ::basegfx::pruneScaleValue( xPos * elementEdge ),
                                yPos ) );
        m_waterfall.append( ::basegfx::B2DPoint(
                                ::basegfx::pruneScaleValue( (xPos + 1) * elementEdge ),
                                yPos ) );
    }
    m_waterfall.append( ::basegfx::B2DPoint( 1.0, -1.0 ) );
    m_waterfall.setClosed(true);
}

::basegfx::B2DPolyPolygon WaterfallWipe::operator () ( double t )
{
    ::basegfx::B2DPolygon poly( m_waterfall );
    poly.transform(basegfx::tools::createTranslateB2DHomMatrix(0.0, ::basegfx::pruneScaleValue(2.0 * t)));
    poly.setB2DPoint( 0, ::basegfx::B2DPoint( 0.0, -1.0 ) );
    poly.setB2DPoint( poly.count()-1, ::basegfx::B2DPoint( 1.0, -1.0 ) );

    return m_flipOnYAxis ? flipOnYAxis( ::basegfx::B2DPolyPolygon(poly) )
                         : ::basegfx::B2DPolyPolygon(poly);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
