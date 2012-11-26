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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace basegfx
{
    B2DRange::B2DRange( const B2IRange& rRange ) :
        maRangeX(),
        maRangeY()
    {
        if( !rRange.isEmpty() )
        {
            maRangeX = rRange.getMinX();
            maRangeY = rRange.getMinY();

            maRangeX.expand(rRange.getMaxX());
            maRangeY.expand(rRange.getMaxY());
        }
    }

    void B2DRange::transform(const B2DHomMatrix& rMatrix)
    {
        if(!isEmpty() && !rMatrix.isIdentity())
        {
            const B2DRange aSource(*this);
            reset();
            expand(rMatrix * B2DPoint(aSource.getMinX(), aSource.getMinY()));
            expand(rMatrix * B2DPoint(aSource.getMaxX(), aSource.getMinY()));
            expand(rMatrix * B2DPoint(aSource.getMinX(), aSource.getMaxY()));
            expand(rMatrix * B2DPoint(aSource.getMaxX(), aSource.getMaxY()));
        }
    }

    B2DRange& B2DRange::operator*=( const ::basegfx::B2DHomMatrix& rMat )
    {
        transform(rMat);
        return *this;
    }

    const B2DRange& B2DRange::getUnitB2DRange()
    {
        static const B2DRange aUnitB2DRange(0.0, 0.0, 1.0, 1.0);

        return aUnitB2DRange;
    }

    B2IRange fround(const B2DRange& rRange)
    {
        return rRange.isEmpty() ?
            B2IRange() :
            B2IRange(fround(rRange.getMinimum()),
                     fround(rRange.getMaximum()));
    }

    B2DRange operator*( const ::basegfx::B2DHomMatrix& rMat, const B2DRange& rB2DRange )
    {
        B2DRange aRes( rB2DRange );
        return aRes *= rMat;
    }
} // end of namespace basegfx

// eof
