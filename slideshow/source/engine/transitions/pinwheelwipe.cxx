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
