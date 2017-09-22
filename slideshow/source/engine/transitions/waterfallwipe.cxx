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
    poly.transform(basegfx::utils::createTranslateB2DHomMatrix(0.0, ::basegfx::pruneScaleValue(2.0 * t)));
    poly.setB2DPoint( 0, ::basegfx::B2DPoint( 0.0, -1.0 ) );
    poly.setB2DPoint( poly.count()-1, ::basegfx::B2DPoint( 1.0, -1.0 ) );

    return m_flipOnYAxis ? flipOnYAxis( ::basegfx::B2DPolyPolygon(poly) )
                         : ::basegfx::B2DPolyPolygon(poly);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
