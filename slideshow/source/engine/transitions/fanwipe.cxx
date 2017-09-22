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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "transitiontools.hxx"
#include "clockwipe.hxx"
#include "fanwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon FanWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    ::basegfx::B2DPolygon poly(
        ClockWipe::calcCenteredClock(
            t / ((m_center && m_single) ? 2.0 : 4.0) ) );

    res.append( poly );
    // flip on y-axis:
    poly.transform(basegfx::utils::createScaleB2DHomMatrix(-1.0, 1.0));
    poly.flip();
    res.append( poly );

    if (m_center)
    {
        res.transform(basegfx::utils::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

        if (! m_single)
            res.append( flipOnXAxis(res) );
    }
    else
    {
        SAL_WARN_IF( m_fanIn, "slideshow.opengl", "FanWipe: m_fanIn is true ?" );
        res.transform(basegfx::utils::createScaleTranslateB2DHomMatrix(0.5, 1.0, 0.5, 1.0));
    }
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
