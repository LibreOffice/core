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
#include "spiralwipe.hxx"
#include "transitiontools.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


namespace slideshow {
namespace internal {

SpiralWipe::SpiralWipe( sal_Int32 nElements, bool flipOnYAxis )
    : m_elements(nElements),
      m_sqrtElements( static_cast<sal_Int32>(
                          sqrt( static_cast<double>(nElements) ) ) ),
      m_flipOnYAxis(flipOnYAxis)
{
}

::basegfx::B2DPolyPolygon SpiralWipe::calcNegSpiral( double t ) const
{
    const double area = (t * m_elements);
    const double e = (sqrt(area) / 2.0);
    const sal_Int32 edge = (static_cast<sal_Int32>(e) * 2);

    basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(-0.5, -0.5));
    const double edge_ = ::basegfx::pruneScaleValue(
        static_cast<double>(edge) / m_sqrtElements );
    aTransform.scale( edge_, edge_ );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( createUnitRect() );
    poly.transform( aTransform );
    ::basegfx::B2DPolyPolygon res(poly);

    if (! ::basegfx::fTools::equalZero( 1.0 - t )) {
        const sal_Int32 edge1 = (edge + 1);
        sal_Int32 len = static_cast<sal_Int32>( (e - (edge /2)) * edge1 * 4 );
        double w = M_PI_2;
        while (len > 0) {
            const sal_Int32 alen = (len > edge1 ? edge1 : len);
            len -= alen;
            poly = createUnitRect();
            aTransform = basegfx::tools::createScaleB2DHomMatrix(
                ::basegfx::pruneScaleValue( static_cast<double>(alen) / m_sqrtElements ),
                ::basegfx::pruneScaleValue( 1.0 / m_sqrtElements ) );
            aTransform.translate(
                - ::basegfx::pruneScaleValue(
                    static_cast<double>(edge / 2) / m_sqrtElements ),
                ::basegfx::pruneScaleValue(
                    static_cast<double>(edge / 2) / m_sqrtElements ) );
            aTransform.rotate( w );
            w -= M_PI_2;
            aTransform.translate( 0.5, 0.5 );
            poly.transform( aTransform );
            res.append(poly);
        }
    }

    return res;
}

::basegfx::B2DPolyPolygon SpiralWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolyPolygon innerSpiral( calcNegSpiral( 1.0 - t ) );
    innerSpiral.flip();
    res.append(innerSpiral);
    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

::basegfx::B2DPolyPolygon BoxSnakesWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolyPolygon innerSpiral( calcNegSpiral( 1.0 - t ) );
    innerSpiral.flip();

    if (m_fourBox) {
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( 0.5, 0.5 );
        innerSpiral.transform( aTransform );
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
        innerSpiral = flipOnYAxis(innerSpiral);
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
    }
    else {
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( 1.0, 0.5 );
        innerSpiral.transform( aTransform );
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
