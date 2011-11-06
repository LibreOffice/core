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
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B3DVector& B3DVector::normalize()
    {
        double fLen(scalar(*this));

        if(!::basegfx::fTools::equalZero(fLen))
        {
            const double fOne(1.0);

            if(!::basegfx::fTools::equal(fOne, fLen))
            {
                fLen = sqrt(fLen);

                if(!::basegfx::fTools::equalZero(fLen))
                {
                    mfX /= fLen;
                    mfY /= fLen;
                    mfZ /= fLen;
                }
            }
        }

        return *this;
    }

    B3DVector B3DVector::getPerpendicular(const B3DVector& rNormalizedVec) const
    {
        B3DVector aNew(*this);
        aNew = cross(aNew, rNormalizedVec);
        aNew.normalize();
        return aNew;
    }

    B3DVector B3DVector::getProjectionOnPlane(const B3DVector& rNormalizedPlane) const
    {
        B3DVector aNew(*this);
        aNew = cross(aNew, rNormalizedPlane);
        aNew = cross(aNew, rNormalizedPlane);

        aNew.mfX = mfX - aNew.mfX;
        aNew.mfY = mfY - aNew.mfY;
        aNew.mfZ = mfZ - aNew.mfZ;

        return aNew;
    }

    B3DVector& B3DVector::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*mfX + rMat.get(0,1)*mfY + rMat.get(0,2)*mfZ );
        const double fTempY( rMat.get(1,0)*mfX + rMat.get(1,1)*mfY + rMat.get(1,2)*mfZ );
        const double fTempZ( rMat.get(2,0)*mfX + rMat.get(2,1)*mfY + rMat.get(2,2)*mfZ );
        mfX = fTempX;
        mfY = fTempY;
        mfZ = fTempZ;

        return *this;
    }

    B3DVector operator*( const ::basegfx::B3DHomMatrix& rMat, const B3DVector& rVec )
    {
        B3DVector aRes( rVec );
        return aRes*=rMat;
    }

    bool areParallel( const B3DVector& rVecA, const B3DVector& rVecB )
    {
        // i think fastest is to compare relations, need no square or division
        if(!fTools::equal(rVecA.getX() * rVecB.getY(), rVecA.getY() * rVecB.getX()))
            return false;

        if(!fTools::equal(rVecA.getX() * rVecB.getZ(), rVecA.getZ() * rVecB.getX()))
            return false;

        return (fTools::equal(rVecA.getY() * rVecB.getZ(), rVecA.getZ() * rVecB.getY()));
    }

} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
