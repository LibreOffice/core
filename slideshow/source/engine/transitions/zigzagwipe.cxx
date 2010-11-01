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
#include "transitiontools.hxx"
#include "zigzagwipe.hxx"


namespace slideshow {
namespace internal {

ZigZagWipe::ZigZagWipe( sal_Int32 nZigs ) : m_zigEdge( 1.0 / nZigs )
{
    const double d = m_zigEdge;
    const double d2 = (d / 2.0);
    m_stdZigZag.append( ::basegfx::B2DPoint( -1.0 - d, -d ) );
    m_stdZigZag.append( ::basegfx::B2DPoint( -1.0 - d, 1.0 + d ) );
    m_stdZigZag.append( ::basegfx::B2DPoint( -d, 1.0 + d ) );
    for ( sal_Int32 pos = (nZigs + 2); pos--; ) {
        m_stdZigZag.append( ::basegfx::B2DPoint( 0.0, ((pos - 1) * d) + d2 ) );
        m_stdZigZag.append( ::basegfx::B2DPoint( -d, (pos - 1) * d ) );
    }
    m_stdZigZag.setClosed(true);
}

::basegfx::B2DPolyPolygon ZigZagWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res(m_stdZigZag);
    res.transform(basegfx::tools::createTranslateB2DHomMatrix((1.0 + m_zigEdge) * t, 0.0));
    return res;
}

::basegfx::B2DPolyPolygon BarnZigZagWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolygon poly( m_stdZigZag );
    poly.flip();
    basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
        (1.0 + m_zigEdge) * (1.0 - t) / 2.0, 0.0));
    poly.transform( aTransform );
    res.append( poly );
    aTransform.scale( -1.0, 1.0 );
    aTransform.translate( 1.0, m_zigEdge / 2.0 );
    poly = m_stdZigZag;
    poly.transform( aTransform );
    res.append( poly );
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
