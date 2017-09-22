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
        aTransform = basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5);
        aTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(d, d, -0.25, -0.25)
            * aTransform;
    }
    else
    {
        aTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(d, d, -0.5, -0.5);
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

    aTransform = basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5);
    res.transform( aTransform );
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
