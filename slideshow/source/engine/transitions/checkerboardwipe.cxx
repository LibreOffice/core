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
#include "checkerboardwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon CheckerBoardWipe::operator () ( double t )
{
    const double d = (1.0 / m_unitsPerEdge);
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( ::basegfx::pruneScaleValue( d * 2.0 * t ),
                      ::basegfx::pruneScaleValue( d ) );

    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 i = m_unitsPerEdge; i--; )
    {
        ::basegfx::B2DHomMatrix transform( aTransform );
        if ((i % 2) == 1) // odd line
            transform.translate( -d, 0.0 );
        for ( sal_Int32 j = (m_unitsPerEdge / 2) + 1; j--; )
        {
            ::basegfx::B2DPolyPolygon poly( m_unitRect );
            poly.transform( transform );
            res.append( poly );
            transform.translate( d * 2.0, 0.0 );
        }
        aTransform.translate( 0.0, d ); // next line
    }
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
