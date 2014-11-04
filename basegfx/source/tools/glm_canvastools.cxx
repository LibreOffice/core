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

#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/AffineMatrix3D.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/tools/glm_canvastools.hxx>
#include <limits>

using namespace ::com::sun::star;

namespace basegfx
{

    namespace unotools
    {
        namespace
        {


        glm::mat4 glmMatrixFromAffineMatrix( const ::com::sun::star::geometry::AffineMatrix2D& input )
        {
            // ensure last row is [0,0,1] (and optimized away)
            glm::mat4 output;
            output[0][0] = input.m00;
            output[1][0] = input.m01;
            output[2][0] = input.m02;
            output[0][1] = input.m10;
            output[1][1] = input.m11;
            output[2][1] = input.m12;

            return output;
        }

        glm::mat4 glmMatrixFromAffineMatrix3D( const ::com::sun::star::geometry::AffineMatrix3D& input )
        {
            glm::mat4 output;
            output[0][0] = input.m00;
            output[1][0] = input.m01;
            output[2][0] = input.m02;
            output[3][0] = input.m03;

            output[0][1] = input.m10;
            output[1][1] = input.m11;
            output[2][1] = input.m12;
            output[3][1] = input.m13;

            output[0][2] = input.m20;
            output[1][2] = input.m21;
            output[2][2] = input.m22;
            output[3][2] = input.m23;

            return output;
        }


        glm::mat4 glmMatFromHomMatrix( const ::basegfx::B2DHomMatrix& input)
        {
            glm::mat4 output;
            output[0][0] = input.get(0,0);
            output[1][0] = input.get(0,1);
            output[2][0] = input.get(0,2);
            output[0][1] = input.get(1,0);
            output[1][1] = input.get(1,1);
            output[2][1] = input.get(1,2);

            return output;
        }

        glm::mat4 glmMatFromHomMatrix3d( const ::basegfx::B3DHomMatrix& input)
        {
            glm::mat4 output;
            output[0][0] = input.get(0,0);
            output[1][0] = input.get(0,1);
            output[2][0] = input.get(0,2);
            output[3][0] = input.get(0,3);

            output[0][1] = input.get(1,0);
            output[1][1] = input.get(1,1);
            output[2][1] = input.get(1,2);
            output[3][1] = input.get(1,3);

            output[0][2] = input.get(2,0);
            output[1][2] = input.get(2,1);
            output[2][2] = input.get(2,2);
            output[3][2] = input.get(2,3);

            return output;
        }
    } // namespace bgfxtools

} // namespace canvas
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
