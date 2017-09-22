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


#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "barndoorwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon BarnDoorWipe::operator () ( double t )
{
    if (m_doubled)
        t /= 2.0;

    basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5));
    aTransform.scale( ::basegfx::pruneScaleValue(t), 1.0 );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( m_unitRect );
    poly.transform( aTransform );
    ::basegfx::B2DPolyPolygon res(poly);

    if (m_doubled) {
        aTransform = basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5);
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        poly.transform( aTransform );
        res.append(poly);
    }

    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
