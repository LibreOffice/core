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

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/basegfxdllapi.h>

namespace com::sun::star::drawing { struct PolyPolygonShape3D; }

namespace basegfx
{
    class B3DPolyPolygon;
    class B3DRange;
}

namespace basegfx::utils
{
        // B3DPolyPolygon tools

        // get size of PolyPolygon. Control vectors are included in that ranges.
        BASEGFX_DLLPUBLIC B3DRange getRange(const B3DPolyPolygon& rCandidate);

        /** Create a unit 3D line polyPolygon which defines a cube.
         */
        B3DPolyPolygon const & createUnitCubePolyPolygon();

        /** Create a unit 3D fill polyPolygon which defines a cube.
         */
        B3DPolyPolygon const & createUnitCubeFillPolyPolygon();

        /** Create a 3D line polyPolygon from a B3DRange which defines a cube.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon createCubePolyPolygonFromB3DRange( const B3DRange& rRange);

        /** Create a 3D fill polyPolygon from a B3DRange which defines a cube.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon createCubeFillPolyPolygonFromB3DRange( const B3DRange& rRange);

        /** Create a unit 3D line polyPolygon which defines a sphere with the given count of hor and ver segments.
            Result will be centered at (0.0, 0.0, 0.0) and sized [-1.0 .. 1.0] in all dimensions.
            If nHorSeg == 0 and/or nVerSeg == 0, a default will be calculated to have a step at least each 15 degrees.
            With VerStart, VerStop and hor range in cartesian may be specified to create a partial sphere only.
         */
        B3DPolyPolygon createUnitSpherePolyPolygon(
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            double fVerStart = M_PI_2, double fVerStop = -M_PI_2,
            double fHorStart = 0.0, double fHorStop = 2 * M_PI);

        /** Create a 3D line polyPolygon from a B3DRange which defines a sphere with the given count of hor and ver segments.
            If nHorSeg == 0 and/or nVerSeg == 0, a default will be calculated to have a step at least each 15 degrees.
            With VerStart, VerStop and hor range in cartesian may be specified to create a partial sphere only.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon createSpherePolyPolygonFromB3DRange(
            const B3DRange& rRange,
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            double fVerStart = M_PI_2, double fVerStop = -M_PI_2,
            double fHorStart = 0.0, double fHorStop = 2 * M_PI);

        /** same as createUnitSpherePolyPolygon, but creates filled polygons (closed and oriented)
            There is one extra, the bool bNormals defines if normals will be set, default is false
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon createUnitSphereFillPolyPolygon(
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            bool bNormals = false,
            double fVerStart = M_PI_2, double fVerStop = -M_PI_2,
            double fHorStart = 0.0, double fHorStop = 2 * M_PI);

        /** same as createSpherePolyPolygonFromB3DRange, but creates filled polygons (closed and oriented)
            There is one extra, the bool bNormals defines if normals will be set, default is false
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon createSphereFillPolyPolygonFromB3DRange(
            const B3DRange& rRange,
            sal_uInt32 nHorSeg, sal_uInt32 nVerSeg,
            bool bNormals = false,
            double fVerStart = M_PI_2, double fVerStop = -M_PI_2,
            double fHorStart = 0.0, double fHorStop = 2 * M_PI);

        /** Create/replace normals for given 3d geometry with default normals from given center to outside.
            rCandidate: the 3d geometry to change
            rCenter:    the center of the 3d geometry
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon applyDefaultNormalsSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter);

        /** invert normals for given 3d geometry.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon invertNormals( const B3DPolyPolygon& rCandidate);

        /** Create/replace texture coordinates for given 3d geometry with parallel projected one
            rRange: the full range of the 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon applyDefaultTextureCoordinatesParallel( const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bChangeX = true, bool bChangeY = true);

        /** Create/replace texture coordinates for given 3d geometry with spherical one
            rCenter: the centre of the used 3d geometry
            If bChangeX, x texture coordinate will be recalculated.
            If bChangeY, y texture coordinate will be recalculated.
         */
        BASEGFX_DLLPUBLIC B3DPolyPolygon applyDefaultTextureCoordinatesSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX = true, bool bChangeY = true);

        // isInside test for B3DPoint. On border is not inside.
        // It is assumed that the orientations of the given polygon are correct.
        BASEGFX_DLLPUBLIC bool isInside(const B3DPolyPolygon& rCandidate, const B3DPoint& rPoint);

        /// converters for css::drawing::PolyPolygonShape3D
        BASEGFX_DLLPUBLIC B3DPolyPolygon UnoPolyPolygonShape3DToB3DPolyPolygon(
            const css::drawing::PolyPolygonShape3D& rPolyPolygonShape3DSource);
        BASEGFX_DLLPUBLIC void B3DPolyPolygonToUnoPolyPolygonShape3D(
            const B3DPolyPolygon& rPolyPolygonSource,
            css::drawing::PolyPolygonShape3D& rPolyPolygonShape3DRetval);

} // end of namespace basegfx::utils

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
