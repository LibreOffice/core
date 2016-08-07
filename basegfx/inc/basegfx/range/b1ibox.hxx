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



#ifndef _BGFX_RANGE_B1IBOX_HXX
#define _BGFX_RANGE_B1IBOX_HXX

#include <basegfx/range/basicbox.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    class BASEGFX_DLLPUBLIC B1IBox
    {
        ::basegfx::BasicBox maRange;

    public:
        B1IBox()
        {
        }

        explicit B1IBox(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        B1IBox(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        B1IBox(const B1IBox& rBox)
        :   maRange(rBox.maRange)
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

        bool operator==( const B1IBox& rBox ) const
        {
            return (maRange == rBox.maRange);
        }

        bool operator!=( const B1IBox& rBox ) const
        {
            return (maRange != rBox.maRange);
        }

        void operator=(const B1IBox& rBox)
        {
            maRange = rBox.maRange;
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

        bool isInside(const B1IBox& rBox) const
        {
            return maRange.isInside(rBox.maRange);
        }

        bool overlaps(const B1IBox& rBox) const
        {
            return maRange.overlaps(rBox.maRange);
        }

        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        void expand(const B1IBox& rBox)
        {
            maRange.expand(rBox.maRange);
        }

        void intersect(const B1IBox& rBox)
        {
            maRange.intersect(rBox.maRange);
        }

        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B1IBOX_HXX */
