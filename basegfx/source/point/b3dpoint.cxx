/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B3DPoint& B3DPoint::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        double fTempX(
            rMat.get(0, 0) * mfX +
            rMat.get(0, 1) * mfY +
            rMat.get(0, 2) * mfZ +
            rMat.get(0, 3));
        double fTempY(
            rMat.get(1, 0) * mfX +
            rMat.get(1, 1) * mfY +
            rMat.get(1, 2) * mfZ +
            rMat.get(1, 3));
        double fTempZ(
            rMat.get(2, 0) * mfX +
            rMat.get(2, 1) * mfY +
            rMat.get(2, 2) * mfZ +
            rMat.get(2, 3));

        if(!rMat.isLastLineDefault())
        {
            const double fOne(1.0);
            const double fTempM(
                rMat.get(3, 0) * mfX +
                rMat.get(3, 1) * mfY +
                rMat.get(3, 2) * mfZ +
                rMat.get(3, 3));

            if(!fTools::equalZero(fTempM) && !fTools::equal(fOne, fTempM))
            {
                fTempX /= fTempM;
                fTempY /= fTempM;
                fTempZ /= fTempM;
            }
        }

        mfX = fTempX;
        mfY = fTempY;
        mfZ = fTempZ;

        return *this;
    }

    B3DPoint operator*( const ::basegfx::B3DHomMatrix& rMat, const B3DPoint& rPoint )
    {
        B3DPoint aRes( rPoint );
        return aRes *= rMat;
    }
} // end of namespace basegfx

// eof
