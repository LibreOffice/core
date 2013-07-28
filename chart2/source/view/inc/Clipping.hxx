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

#ifndef _CHART2_CLIPPING_HXX
#define _CHART2_CLIPPING_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>

namespace chart
{

/**
*/

class Clipping
{
    /** This class uses the Liang-Biarsky parametric line-clipping algorithm as described in:
            Computer Graphics: principles and practice, 2nd ed.,
            James D. Foley et al.,
            Section 3.12.4 on page 117.
    */

public:
    /** @descr  The intersection between an open polygon and a rectangle is
            calculated and the resulting lines are placed into the poly-polygon aResult.
        @param  rPolygon    The polygon is required to be open, ie. it's start and end point
            have different coordinates and that it is continuous, ie. has no holes.
        @param  rRectangle  The clipping area.
        @param  aResult The resulting lines that are the parts of the given polygon lying inside
            the clipping area are stored into aResult whose prior content is deleted first.
     */
    static void clipPolygonAtRectangle(
                                const ::com::sun::star::drawing::PolyPolygonShape3D& rPolygon
                                , const ::basegfx::B2DRectangle& rRectangle
                                , ::com::sun::star::drawing::PolyPolygonShape3D& aResult
                                , bool bSplitPiecesToDifferentPolygons = true );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
