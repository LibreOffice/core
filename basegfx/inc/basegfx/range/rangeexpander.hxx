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



#ifndef _BGFX_RANGE_RANGEEXPANDER_HXX
#define _BGFX_RANGE_RANGEEXPANDER_HXX

#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b1irange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/range/b3irange.hxx>

namespace basegfx
{
    /** Generic functor for expanding a range with a number of other
        ranges.

        Since *Range::expand() is overloaded, straight-forward
        application of ::boost::bind and friends fails (because of
        ambiguities). Thus, this functor template can be used, to
        expand the given range with a number of other ranges, passed
        in at the function operator.

        @tpl RangeType
        Range type to operate with. Preferably, one of B1*Range,
        B2*Range, or B3*Range.
    */
    template< typename RangeType > class RangeExpander
    {
    public:
        typedef RangeType   ValueType;
        typedef void        result_type;

        explicit RangeExpander( ValueType& rBounds ) :
            mrBounds( rBounds )
        {
        }

        void operator()( const ValueType& rBounds )
        {
            mrBounds.expand( rBounds );
        }

    private:
        ValueType& mrBounds;
    };

    typedef RangeExpander< B1DRange > B1DRangeExpander;
    typedef RangeExpander< B1IRange > B1IRangeExpander;
    typedef RangeExpander< B2DRange > B2DRangeExpander;
    typedef RangeExpander< B2IRange > B2IRangeExpander;
    typedef RangeExpander< B3DRange > B3DRangeExpander;
    typedef RangeExpander< B3IRange > B3IRangeExpander;

} // end of namespace basegfx


#endif /* _BGFX_RANGE_RANGEEXPANDER_HXX */
