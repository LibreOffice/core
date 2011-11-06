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
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B2IVector& B2IVector::operator=( const ::basegfx::B2ITuple& rVec )
    {
        mnX = rVec.getX();
        mnY = rVec.getY();
        return *this;
    }


    double B2IVector::getLength() const
    {
        return hypot( mnX, mnY );
    }

    double B2IVector::scalar( const B2IVector& rVec ) const
    {
        return((mnX * rVec.mnX) + (mnY * rVec.mnY));
    }

    double B2IVector::cross( const B2IVector& rVec ) const
    {
        return(mnX * rVec.getY() - mnY * rVec.getX());
    }

    double B2IVector::angle( const B2IVector& rVec ) const
    {
        return atan2(double( mnX * rVec.getY() - mnY * rVec.getX()),
            double( mnX * rVec.getX() + mnY * rVec.getY()));
    }

    const B2IVector& B2IVector::getEmptyVector()
    {
        return (const B2IVector&) ::basegfx::B2ITuple::getEmptyTuple();
    }

    B2IVector& B2IVector::operator*=( const B2DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX +
                      rMat.get(0,1)*mnY );
        mnY = fround( rMat.get(1,0)*mnX +
                      rMat.get(1,1)*mnY );

        return *this;
    }

    B2IVector& B2IVector::setLength(double fLen)
    {
        double fLenNow(scalar(*this));

        if(!::basegfx::fTools::equalZero(fLenNow))
        {
            const double fOne(10.0);

            if(!::basegfx::fTools::equal(fOne, fLenNow))
            {
                fLen /= sqrt(fLenNow);
            }

            mnX = fround( mnX*fLen );
            mnY = fround( mnY*fLen );
        }

        return *this;
    }

    bool areParallel( const B2IVector& rVecA, const B2IVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
        return ::basegfx::fTools::equalZero(fVal);
    }

    B2VectorOrientation getOrientation( const B2IVector& rVecA, const B2IVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());

        if(fVal > 0.0)
        {
            return ORIENTATION_POSITIVE;
        }

        if(fVal < 0.0)
        {
            return ORIENTATION_NEGATIVE;
        }

        return ORIENTATION_NEUTRAL;
    }

    B2IVector getPerpendicular( const B2IVector& rNormalizedVec )
    {
        B2IVector aPerpendicular(-rNormalizedVec.getY(), rNormalizedVec.getX());
        return aPerpendicular;
    }

    B2IVector operator*( const B2DHomMatrix& rMat, const B2IVector& rVec )
    {
        B2IVector aRes( rVec );
        return aRes*=rMat;
    }

    B2VectorContinuity getContinuity(const B2IVector& rBackVector, const B2IVector& rForwardVector )
    {
        B2VectorContinuity eRetval(CONTINUITY_NONE);

        if(!rBackVector.equalZero() && !rForwardVector.equalZero())
        {
            const B2IVector aInverseForwardVector(-rForwardVector.getX(), -rForwardVector.getY());

            if(rBackVector == aInverseForwardVector)
            {
                // same direction and same length -> C2
                eRetval = CONTINUITY_C2;
            }
            else if(areParallel(rBackVector, aInverseForwardVector))
            {
                // same direction -> C1
                eRetval = CONTINUITY_C1;
            }
        }

        return eRetval;
    }
} // end of namespace basegfx

// eof
