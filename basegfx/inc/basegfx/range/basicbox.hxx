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



#ifndef _BGFX_RANGE_BASICBOX_HXX
#define _BGFX_RANGE_BASICBOX_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** Specialization of BasicRange, handling the inside predicates
        differently.

        This template considers the rightmost and bottommost border as
        <em>outside</em> of the range, in contrast to BasicRange,
        which considers them inside.
     */
    class BASEGFX_DLLPUBLIC BasicBox : public BasicRange< sal_Int32, Int32Traits >
    {
        typedef BasicRange< sal_Int32, Int32Traits > Base;
    public:
        BasicBox() :
            Base()
        {
        }

        BasicBox( sal_Int32 nValue ) :
            Base( nValue )
        {
        }

        BasicBox(const BasicBox& rBox) :
            Base( rBox )
        {
        }

        double getCenter() const
        {
            if(isEmpty())
            {
                return 0.0;
            }
            else
            {
                return ((mnMaximum + mnMinimum - 1.0) / 2.0);
            }
        }

        using Base::isInside;

        bool isInside(sal_Int32 nValue) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                return (nValue >= mnMinimum) && (nValue < mnMaximum);
            }
        }

        using Base::overlaps;

        bool overlaps(const BasicBox& rBox) const
        {
            if(isEmpty())
            {
                return false;
            }
            else
            {
                if(rBox.isEmpty())
                {
                    return false;
                }
                else
                {
                    return !((rBox.mnMaximum <= mnMinimum) || (rBox.mnMinimum >= mnMaximum));
                }
            }
        }

        void grow(sal_Int32 nValue)
        {
            if(!isEmpty())
            {
                bool bLessThanZero(nValue < 0);

                if(nValue > 0 || bLessThanZero)
                {
                    mnMinimum -= nValue;
                    mnMaximum += nValue;

                    if(bLessThanZero)
                    {
                        // test if range did collapse
                        if(mnMinimum > mnMaximum)
                        {
                            // if yes, collapse to center
                            mnMinimum = mnMaximum = ((mnMaximum + mnMinimum - 1) / 2);
                        }
                    }
                }
            }
        }
    };

} // end of namespace basegfx

#endif /* _BGFX_RANGE_BASICBOX_HXX */
