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
    res.transform(basegfx::utils::createTranslateB2DHomMatrix((1.0 + m_zigEdge) * t, 0.0));
    return res;
}

::basegfx::B2DPolyPolygon BarnZigZagWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolygon poly( m_stdZigZag );
    poly.flip();
    basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(
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
