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



#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#define _BGFX_POLYGON_B3DPOLYPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/basegfxdllapi.h>

// predeclarations
class ImplB3DPolyPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DHomMatrix;
    class B2DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B3DPolyPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolyPolygon > ImplType;

    private:
        ImplType                                        mpPolyPolygon;

    public:
        B3DPolyPolygon();
        B3DPolyPolygon(const B3DPolyPolygon& rPolyPolygon);
        explicit B3DPolyPolygon(const B3DPolygon& rPolygon);
        ~B3DPolyPolygon();

        // assignment operator
        B3DPolyPolygon& operator=(const B3DPolyPolygon& rPolyPolygon);

        /// unshare this poly-polygon (and all included polygons) with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B3DPolyPolygon& rPolyPolygon) const;
        bool operator!=(const B3DPolyPolygon& rPolyPolygon) const;

        // polygon interface
        sal_uInt32 count() const;

        // B3DPolygon interface
        B3DPolygon getB3DPolygon(sal_uInt32 nIndex) const;
        void setB3DPolygon(sal_uInt32 nIndex, const B3DPolygon& rPolygon);

        // BColor interface
        bool areBColorsUsed() const;
        void clearBColors();

        // Normals interface
        void transformNormals(const B3DHomMatrix& rMatrix);
        bool areNormalsUsed() const;
        void clearNormals();

        // TextureCoordinate interface
        void transformTextureCoordiantes(const B2DHomMatrix& rMatrix);
        bool areTextureCoordinatesUsed() const;
        void clearTextureCoordinates();

        // insert/append single polygon
        void insert(sal_uInt32 nIndex, const B3DPolygon& rPolygon, sal_uInt32 nCount = 1);
        void append(const B3DPolygon& rPolygon, sal_uInt32 nCount = 1);

        // insert/append multiple polygons
        void insert(sal_uInt32 nIndex, const B3DPolyPolygon& rPolyPolygon);
        void append(const B3DPolyPolygon& rPolyPolygon);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // reset to empty state
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if PolyPolygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const basegfx::B3DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

#endif /* _BGFX_POLYGON_B3DPOLYPOLYGON_HXX */
