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


#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "randomwipe.hxx"
#include "tools.hxx"


namespace slideshow {
namespace internal {

RandomWipe::RandomWipe( sal_Int32 nElements, bool randomBars )
    : m_positions( new ::basegfx::B2DPoint[ nElements ] ),
      m_nElements( nElements ),
      m_rect( createUnitRect() )
{
    ::basegfx::B2DHomMatrix aTransform;
    if (randomBars)
    {
        double edge = (1.0 / nElements);
        for ( sal_Int32 pos = nElements; pos--; )
            m_positions[ pos ].setY( ::basegfx::pruneScaleValue( pos * edge ) );
        aTransform.scale( 1.0, ::basegfx::pruneScaleValue(edge) );
    }
    else // dissolve effect
    {
        sal_Int32 sqrtElements = static_cast<sal_Int32>(
            sqrt( static_cast<double>(nElements) ) );
        double edge = (1.0 / sqrtElements);
        for ( sal_Int32 pos = nElements; pos--; ) {
            m_positions[ pos ] = ::basegfx::B2DPoint(
                ::basegfx::pruneScaleValue( (pos % sqrtElements) * edge ),
                ::basegfx::pruneScaleValue( (pos / sqrtElements) * edge ) );
        }
        const double pedge = ::basegfx::pruneScaleValue(edge);
        aTransform.scale( pedge, pedge );
    }
    m_rect.transform( aTransform );

    // mix up:
    for ( sal_Int32 pos1 = nElements ; pos1-- ; )
    {
        const sal_Int32 pos2 = getRandomOrdinal(pos1+1);
        ::std::swap(m_positions[ pos1], m_positions[ pos2 ]);
    }
}

::basegfx::B2DPolyPolygon RandomWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 pos = static_cast<sal_Int32>(t * m_nElements); pos--; )
    {
        ::basegfx::B2DPoint const & point = m_positions[ pos ];
        ::basegfx::B2DPolygon poly( m_rect );
        poly.transform(basegfx::utils::createTranslateB2DHomMatrix(point.getX(), point.getY()));
        res.append( poly );
    }
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
