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



#ifndef _BGFX_RANGE_B1IRANGE_HXX
#define _BGFX_RANGE_B1IRANGE_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    class BASEGFX_DLLPUBLIC B1IRange
    {
        ::basegfx::BasicRange< sal_Int32, Int32Traits > maRange;

    public:
        B1IRange()
        {
        }

        explicit B1IRange(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        B1IRange(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        B1IRange(const B1IRange& rRange)
        :   maRange(rRange.maRange)
        {
        }

        bool isEmpty() const
        {
            return maRange.isEmpty();
        }

        void reset()
        {
            maRange.reset();
        }

        bool operator==( const B1IRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1IRange& rRange ) const
        {
            return (maRange != rRange.maRange);
        }

        B1IRange& operator=(const B1IRange& rRange)
        {
            maRange = rRange.maRange;
            return *this;
        }

        sal_Int32 getMinimum() const
        {
            return maRange.getMinimum();
        }

        sal_Int32 getMaximum() const
        {
            return maRange.getMaximum();
        }

        Int32Traits::DifferenceType getRange() const
        {
            return maRange.getRange();
        }

        double getCenter() const
        {
            return maRange.getCenter();
        }

        bool isInside(sal_Int32 nValue) const
        {
            return maRange.isInside(nValue);
        }

        bool isInside(const B1IRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        bool overlaps(const B1IRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        void expand(const B1IRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        void intersect(const B1IRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B1IRANGE_HXX */
