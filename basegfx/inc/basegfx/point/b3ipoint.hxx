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



#ifndef _BGFX_POINT_B3IPOINT_HXX
#define _BGFX_POINT_B3IPOINT_HXX

#include <basegfx/tuple/b3ituple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three sal_Int32 values

        This class derives all operators and common handling for
        a 3D data class from B3ITuple. All necessary extensions
        which are special for points will be added here.

        @see B3ITuple
    */
    class BASEGFX_DLLPUBLIC B3IPoint : public ::basegfx::B3ITuple
    {
    public:
        /** Create a 3D Point

            The point is initialized to (0, 0, 0)
        */
        B3IPoint()
        :   B3ITuple()
        {}

        /** Create a 3D Point

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 3D Point.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 3D Point.

            @param nZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Point.
        */
        B3IPoint(sal_Int32 nX, sal_Int32 nY, sal_Int32 nZ)
        :   B3ITuple(nX, nY, nZ)
        {}

        /** Create a copy of a 3D Point

            @param rVec
            The 3D Point which will be copied.
        */
        B3IPoint(const B3IPoint& rVec)
        :   B3ITuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3ITuple-based classes
        */
        B3IPoint(const ::basegfx::B3ITuple& rTuple)
        :   B3ITuple(rTuple)
        {}

        ~B3IPoint()
        {}

        /** *=operator to allow usage from B3IPoint, too
        */
        B3IPoint& operator*=( const B3IPoint& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            mnZ *= rPnt.mnZ;
            return *this;
        }

        /** *=operator to allow usage from B3IPoint, too
        */
        B3IPoint& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            mnZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3ITuple calculations
        */
        B3IPoint& operator=( const ::basegfx::B3ITuple& rVec )
        {
            mnX = rVec.getX();
            mnY = rVec.getY();
            mnZ = rVec.getZ();
            return *this;
        }

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B3DVector, applied.
        */
        B3IPoint& operator*=( const ::basegfx::B3DHomMatrix& rMat );

        static const B3IPoint& getEmptyPoint()
        {
            return (const B3IPoint&) ::basegfx::B3ITuple::getEmptyTuple();
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_POINT_B3IPOINT_HXX */
