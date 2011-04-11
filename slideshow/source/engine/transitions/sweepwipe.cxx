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
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "clockwipe.hxx"
#include "sweepwipe.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon SweepWipe::operator () ( double t )
{
    t /= 2.0;
    if (! m_center)
        t /= 2.0;
    if (!m_single && !m_oppositeVertical)
        t /= 2.0;

    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock( 0.25 + t ) );
    ::basegfx::B2DHomMatrix aTransform;

    if (m_center)
    {
        aTransform = basegfx::tools::createTranslateB2DHomMatrix(0.5, 0.0);
        poly.transform( aTransform );
    }
    ::basegfx::B2DPolyPolygon res(poly);

    if (! m_single)
    {
        if (m_oppositeVertical)
        {
            aTransform = basegfx::tools::createScaleB2DHomMatrix(1.0, -1.0);
            aTransform.translate( 0.0, 1.0 );
            poly.transform( aTransform );
            poly.flip();
        }
        else
        {
            aTransform = basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5);
            aTransform.rotate( M_PI );
            aTransform.translate( 0.5, 0.5 );
            poly.transform( aTransform );
        }
        res.append(poly);
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
