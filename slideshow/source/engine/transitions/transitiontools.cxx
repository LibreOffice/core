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


#include "transitiontools.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


namespace slideshow {
namespace internal {

// TODO(Q2): Move this to basegfx
::basegfx::B2DPolygon createUnitRect()
{
    return ::basegfx::utils::createPolygonFromRect(
        ::basegfx::B2DRectangle(0.0,0.0,
                                1.0,1.0 ) );
}

::basegfx::B2DPolyPolygon flipOnYAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    res.transform(basegfx::utils::createScaleTranslateB2DHomMatrix(-1.0, 1.0, 1.0, 0.0));
    res.flip();
    return res;
}

::basegfx::B2DPolyPolygon flipOnXAxis(
    ::basegfx::B2DPolyPolygon const & polypoly )
{
    ::basegfx::B2DPolyPolygon res(polypoly);
    res.transform(basegfx::utils::createScaleTranslateB2DHomMatrix(1.0, -1.0, 0.0, 1.0));
    res.flip();
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
