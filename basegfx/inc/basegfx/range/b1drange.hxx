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



#ifndef _BGFX_RANGE_B1DRANGE_HXX
#define _BGFX_RANGE_B1DRANGE_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    class B1IRange;

    class BASEGFX_DLLPUBLIC B1DRange
    {
        ::basegfx::BasicRange< double, DoubleTraits >   maRange;

    public:
        B1DRange()
        {
        }

        explicit B1DRange(double fStartValue)
        :   maRange(fStartValue)
        {
        }

        B1DRange(double fStartValue1, double fStartValue2)
        :   maRange(fStartValue1)
        {
            expand(fStartValue2);
        }

        B1DRange(const B1DRange& rRange)
        :   maRange(rRange.maRange)
        {
        }

        explicit B1DRange( const B1IRange& rRange );

        bool isEmpty() const
        {
            return maRange.isEmpty();
        }

        void reset()
        {
            maRange.reset();
        }

        bool operator==( const B1DRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1DRange& rRange ) const
        {
            return (maRange != rRange.maRange);
        }

        B1DRange& operator=(const B1DRange& rRange)
        {
            maRange = rRange.maRange;
            return *this;
        }

        bool equal(const B1DRange& rRange) const
        {
            return (maRange.equal(rRange.maRange));
        }

        double getMinimum() const
        {
            return maRange.getMinimum();
        }

        double getMaximum() const
        {
            return maRange.getMaximum();
        }

        double getRange() const
        {
            return maRange.getRange();
        }

        double getCenter() const
        {
            return maRange.getCenter();
        }

        bool isInside(double fValue) const
        {
            return maRange.isInside(fValue);
        }

        bool isInside(const B1DRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        bool overlaps(const B1DRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        bool overlapsMore(const B1DRange& rRange) const
        {
            return maRange.overlapsMore(rRange.maRange);
        }

        void expand(double fValue)
        {
            maRange.expand(fValue);
        }

        void expand(const B1DRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        void intersect(const B1DRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        void grow(double fValue)
        {
            maRange.grow(fValue);
        }
    };

    /** Round double to nearest integer for 1D range

        @return the nearest integer for this range
    */
    B1IRange fround(const B1DRange& rRange);
} // end of namespace basegfx


#endif /* _BGFX_RANGE_B1DRANGE_HXX */
