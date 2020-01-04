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
        aTransform = basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.0);
        poly.transform( aTransform );
    }
    ::basegfx::B2DPolyPolygon res(poly);

    if (! m_single)
    {
        if (m_oppositeVertical)
        {
            aTransform = basegfx::utils::createScaleB2DHomMatrix(1.0, -1.0);
            aTransform.translate( 0.0, 1.0 );
            poly.transform( aTransform );
            poly.flip();
        }
        else
        {
            aTransform = basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5);
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
