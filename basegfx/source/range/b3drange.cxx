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
#include <basegfx/range/b3drange.hxx>
#include <basegfx/range/b3irange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
    B3DRange::B3DRange(const B3IRange& rRange) :
        maRangeX(),
        maRangeY(),
        maRangeZ()
    {
        if( !rRange.isEmpty() )
        {
            maRangeX = rRange.getMinX();
            maRangeY = rRange.getMinY();
            maRangeZ = rRange.getMinZ();

            maRangeX.expand( rRange.getMaxX() );
            maRangeY.expand( rRange.getMaxY() );
            maRangeZ.expand( rRange.getMaxZ() );
        }
    }

    void B3DRange::transform(const B3DHomMatrix& rMatrix)
    {
        if(!isEmpty() && !rMatrix.isIdentity())
        {
            const B3DRange aSource(*this);
            reset();
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMinY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMinY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMaxY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMaxY(), aSource.getMinZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMinY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMinY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMinX(), aSource.getMaxY(), aSource.getMaxZ()));
            expand(rMatrix * B3DPoint(aSource.getMaxX(), aSource.getMaxY(), aSource.getMaxZ()));
        }
    }

    B3DRange& B3DRange::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        transform(rMat);
        return *this;
    }

    const B3DRange& B3DRange::getUnitB3DRange()
    {
        static const B3DRange aUnitB3DRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

        return aUnitB3DRange;
    }

    B3IRange fround(const B3DRange& rRange )
    {
        return rRange.isEmpty() ?
            B3IRange() :
            B3IRange(fround(rRange.getMinX()),
                     fround(rRange.getMinY()),
                     fround(rRange.getMinZ()),
                     fround(rRange.getMaxX()),
                     fround(rRange.getMaxY()),
                     fround(rRange.getMaxZ()));
    }

    B3DRange operator*( const ::basegfx::B3DHomMatrix& rMat, const B3DRange& rB3DRange )
    {
        B3DRange aRes( rB3DRange );
        return aRes *= rMat;
    }
} // end of namespace basegfx

// eof
