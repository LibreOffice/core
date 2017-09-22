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

#ifndef INCLUDED_BASEGFX_RANGE_B2DRANGECLIPPER_HXX
#define INCLUDED_BASEGFX_RANGE_B2DRANGECLIPPER_HXX

#include <basegfx/range/b2dpolyrange.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    namespace utils
    {
        /** Extract poly-polygon w/o self-intersections from poly-range

            Similar to the solveCrossovers(const B2DPolyPolygon&)
            method, this one calculates a self-intersection-free
            poly-polygon with the same topology, and encoding
            inside/outsidedness via polygon orientation and layering.
         */
        BASEGFX_DLLPUBLIC B2DPolyPolygon solveCrossovers(const std::vector<B2DRange>& rRanges,
                                       const std::vector<B2VectorOrientation>& rOrientations);
    }
}

#endif // INCLUDED_BASEGFX_RANGE_B2DRANGECLIPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
