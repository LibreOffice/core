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
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "fourboxwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon FourBoxWipe::operator () ( double t )
{
    ::basegfx::B2DHomMatrix aTransform;
    const double d = ::basegfx::pruneScaleValue( t / 2.0 );
    if (m_cornersOut)
    {
        aTransform = basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5);
        aTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(d, d, -0.25, -0.25)
            * aTransform;
    }
    else
    {
        aTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(d, d, -0.5, -0.5);
    }

    // top left:
    ::basegfx::B2DPolygon square( m_unitRect );
    square.transform( aTransform );
    ::basegfx::B2DPolyPolygon res( square );
    // bottom left, flip on x-axis:
    aTransform.scale( -1.0, 1.0 );
    ::basegfx::B2DPolygon square2( m_unitRect );
    square2.transform( aTransform );
    square2.flip(); // flip direction
    res.append( square2 );
    // bottom right, flip on y-axis:
    aTransform.scale( 1.0, -1.0 );
    ::basegfx::B2DPolygon square3( m_unitRect );
    square3.transform( aTransform );
    res.append( square3 );
    // top right, flip on x-axis:
    aTransform.scale( -1.0, 1.0 );
    ::basegfx::B2DPolygon square4( m_unitRect );
    square4.transform( aTransform );
    square4.flip(); // flip direction
    res.append( square4 );

    aTransform = basegfx::tools::createTranslateB2DHomMatrix(0.5, 0.5);
    res.transform( aTransform );
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
