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



#ifndef _BGFX_RANGE_B2DPOLYRANGE_HXX
#define _BGFX_RANGE_B2DPOLYRANGE_HXX

#include <o3tl/cow_wrapper.hxx>
#include <boost/tuple/tuple.hpp>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DTuple;
    class B2DRange;
    class B2DPolyPolygon;
    class ImplB2DPolyRange;

    /** Multiple ranges in one object.

        This class combines multiple ranges in one object, providing a
        total, enclosing range for it.

        You can use this class e.g. when updating views containing
        rectangular objects. Add each modified object to a
        B2DMultiRange, then test each viewable object against
        intersection with the multi range.

        Similar in spirit to the poly-polygon vs. polygon relationship.

        Note that comparable to polygons, a poly-range can also
        contain 'holes' - this is encoded via polygon orientation at
        the poly-polygon, and via explicit flags for the poly-range.
     */
    class BASEGFX_DLLPUBLIC B2DPolyRange
    {
    public:
        typedef boost::tuple<B2DRange,B2VectorOrientation> ElementType ;

        B2DPolyRange();
        ~B2DPolyRange();

        /** Create a multi range with exactly one containing range
         */
        explicit B2DPolyRange( const ElementType& rElement );
        B2DPolyRange( const B2DRange& rRange, B2VectorOrientation eOrient );
        B2DPolyRange( const B2DPolyRange& );
        B2DPolyRange& operator=( const B2DPolyRange& );

        /// unshare this poly-range with all internally shared instances
        void makeUnique();

        bool operator==(const B2DPolyRange&) const;
        bool operator!=(const B2DPolyRange&) const;

        /// Number of included ranges
        sal_uInt32 count() const;

        ElementType getElement(sal_uInt32 nIndex) const;
        void        setElement(sal_uInt32 nIndex, const ElementType& rElement );
        void        setElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient );

        // insert/append a single range
        void insertElement(sal_uInt32 nIndex, const ElementType& rElement, sal_uInt32 nCount = 1);
        void insertElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount = 1);
        void appendElement(const ElementType& rElement, sal_uInt32 nCount = 1);
        void appendElement(const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount = 1);

        // insert/append multiple ranges
        void insertPolyRange(sal_uInt32 nIndex, const B2DPolyRange&);
        void appendPolyRange(const B2DPolyRange&);

        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);
        void clear();

        // flip range orientations - converts holes to solids, and vice versa
        void flip();

        /** Get overall range

            @return
            The union range of all contained ranges
        */
        B2DRange getBounds() const;

        /** Test whether given tuple is inside one or more of the
            included ranges. Does *not* use overall range, but checks
            individually.
         */
        bool isInside( const B2DTuple& rTuple ) const;

        /** Test whether given range is inside one or more of the
            included ranges. Does *not* use overall range, but checks
            individually.
         */
        bool isInside( const B2DRange& rRange ) const;

        /** Test whether given range overlaps one or more of the
            included ranges. Does *not* use overall range, but checks
            individually.
         */
        bool overlaps( const B2DRange& rRange ) const;

        /** Request a poly-polygon with solved cross-overs
         */
        B2DPolyPolygon solveCrossovers() const;

        // element iterators (same iterator validity conditions as for vector)
        const B2DRange* begin() const;
        const B2DRange* end() const;
        B2DRange* begin();
        B2DRange* end();

    private:
        o3tl::cow_wrapper< ImplB2DPolyRange > mpImpl;
    };
}

#endif /* _BGFX_RANGE_B2DPOLYRANGE_HXX */
