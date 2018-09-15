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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DPOLYGONTRIANGULATOR_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DPOLYGONTRIANGULATOR_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    namespace triangulator
    {
        // Simple B2D-based triangle. Main reason is to
        // keep the data types separated (before a B2DPolygon
        // was used with the convention that three points in
        // a row define a triangle)
        class BASEGFX_DLLPUBLIC B2DTriangle
        {
            // positions
            basegfx::B2DPoint                       maA;
            basegfx::B2DPoint                       maB;
            basegfx::B2DPoint                       maC;

        public:
            B2DTriangle(
                const basegfx::B2DPoint& rA,
                const basegfx::B2DPoint& rB,
                const basegfx::B2DPoint& rC)
            :   maA(rA),
                maB(rB),
                maC(rC)
            {
            }

            // get positions
            const basegfx::B2DPoint& getA() const { return maA; }
            const basegfx::B2DPoint& getB() const { return maB; }
            const basegfx::B2DPoint& getC() const { return maC; }
        };

        // typedef for a vector of B2DTriangle
        typedef ::std::vector< B2DTriangle > B2DTriangleVector;

        // triangulate given polygon
        BASEGFX_DLLPUBLIC B2DTriangleVector triangulate(const ::basegfx::B2DPolygon& rCandidate);

        // triangulate given PolyPolygon
        BASEGFX_DLLPUBLIC B2DTriangleVector triangulate(const ::basegfx::B2DPolyPolygon& rCandidate);

    } // end of namespace triangulator
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POLYGON_B2DPOLYGONTRIANGULATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
