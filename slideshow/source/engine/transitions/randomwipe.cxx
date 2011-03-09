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
    for (sal_Int32 nIndex=0; nIndex<nElements; ++nIndex)
    {
        const sal_Int32 nOtherIndex (getRandomOrdinal(nElements));
        OSL_ASSERT(nOtherIndex>=0 && nOtherIndex<nElements);
        ::std::swap(m_positions[nIndex], m_positions[nOtherIndex]);
    }
}

::basegfx::B2DPolyPolygon RandomWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 pos = static_cast<sal_Int32>(t * m_nElements); pos--; )
    {
        ::basegfx::B2DPoint const & point = m_positions[ pos ];
        ::basegfx::B2DPolygon poly( m_rect );
        poly.transform(basegfx::tools::createTranslateB2DHomMatrix(point.getX(), point.getY()));
        res.append( poly );
    }
    return res;
}

}
}
