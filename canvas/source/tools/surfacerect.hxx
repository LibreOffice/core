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

#pragma once

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2isize.hxx>

namespace canvas
{
    /**
     * This implements some equivalent to basegfx::B2IBox, but instead of two
     * BasicBox ranges, it uses a position and a size. maPos and maSize could
     * be replaced by:
     * - B2IPoint(getMinX(), getMinY()) and
     * - B2ISize(getMaxX()-getMinX(), getMaxY()-getMinY())
     *
     * The current allocation algorithm uses size and pos a lot. Not sure how
     * time-critical any of this code is and if that would be a problem.
     */
    struct SurfaceRect
    {
        ::basegfx::B2IPoint maPos;
        ::basegfx::B2ISize  maSize;

        explicit SurfaceRect( const ::basegfx::B2ISize &rSize ) :
            maPos(),
            maSize(rSize)
        {
        }

        bool pointInside( sal_Int32 px, sal_Int32 py ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(x1 + maSize.getX());
            const sal_Int32 y2(y1 + maSize.getY());
            if(px  < x1) return false;
            if(px >= x2) return false;
            if(py  < y1) return false;
            if(py >= y2) return false;
            return true;
        }

        /// returns true if the passed rect intersects this one.
        bool intersection( const SurfaceRect& r ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x1w(x1 + maSize.getX() - 1);
            const sal_Int32 y1h(y1 + maSize.getY() - 1);

            const sal_Int32 x2(r.maPos.getX());
            const sal_Int32 y2(r.maPos.getY());
            const sal_Int32 x2w(x2 + r.maSize.getX() - 1);
            const sal_Int32 y2h(y2 + r.maSize.getY() - 1);

            return !((x1w < x2) || (x2w < x1) || (y1h < y2) || (y2h < y1));
        }

        bool inside( const SurfaceRect& r ) const
        {
            const sal_Int32 x1(maPos.getX());
            const sal_Int32 y1(maPos.getY());
            const sal_Int32 x2(x1 + maSize.getX() - 1);
            const sal_Int32 y2(y1 + maSize.getY() - 1);
            if(!(r.pointInside(x1,y1))) return false;
            if(!(r.pointInside(x2,y2))) return false;
            return true;
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
