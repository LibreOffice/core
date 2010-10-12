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
#include "pinwheelwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon PinWheelWipe::operator () ( double t )
{
    ::basegfx::B2DPolygon poly( ClockWipe::calcCenteredClock(
                                    t / m_blades,
                                    2.0 /* max edge when rotating */ ) );
    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 i = m_blades; i--; )
    {
        ::basegfx::B2DPolygon p(poly);
        p.transform(basegfx::tools::createRotateB2DHomMatrix((i * 2.0 * M_PI) / m_blades));
        res.append( p );
    }
    res.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
