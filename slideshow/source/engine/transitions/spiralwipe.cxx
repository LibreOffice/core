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

    basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5));
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
            aTransform = basegfx::utils::createScaleB2DHomMatrix(
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
