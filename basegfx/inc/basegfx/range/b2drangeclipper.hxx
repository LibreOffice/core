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



#ifndef _BGFX_RANGE_B2DRANGECLIPPER_HXX
#define _BGFX_RANGE_B2DRANGECLIPPER_HXX

#include <basegfx/range/b2dpolyrange.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    namespace tools
    {
        /** Extract poly-polygon w/o self-intersections from poly-range

            Similar to the solveCrossovers(const B2DPolyPolygon&)
            method, this one calculates a self-intersection-free
            poly-polygon with the same topology, and encoding
            inside/outsidedness via polygon orientation and layering.
         */
        BASEGFX_DLLPUBLIC B2DPolyPolygon solveCrossovers(const std::vector<B2DRange>& rRanges,
                                       const std::vector<B2VectorOrientation>& rOrientations);
    }
}

#endif /* _BGFX_RANGE_B2DRANGECLIPPER_HXX */
