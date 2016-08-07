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



#ifndef _BGFX_CURVE_B2DBEZIERTOOLS_HXX
#define _BGFX_CURVE_B2DBEZIERTOOLS_HXX

#include <sal/types.h>
#include <vector>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx
{
    class B2DCubicBezier;
}

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B2DCubicBezierHelper
    {
    private:
        ::std::vector< double >         maLengthArray;
        sal_uInt32                      mnEdgeCount;

    public:
        B2DCubicBezierHelper(const B2DCubicBezier& rBase, sal_uInt32 nDivisions = 9);

        double getLength() const { if(maLengthArray.size()) return maLengthArray[maLengthArray.size() - 1]; else return 0.0; }
        double distanceToRelative(double fDistance) const;
        double relativeToDistance(double fRelative) const;
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_CURVE_B2DBEZIERTOOLS_HXX */
