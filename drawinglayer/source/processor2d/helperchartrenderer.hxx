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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;

namespace drawinglayer { namespace primitive2d { class ChartPrimitive2D; }}
namespace drawinglayer { namespace geometry { class ViewInformation2D; }}

//////////////////////////////////////////////////////////////////////////////
// support chart PrettyPrinter usage from primitives

namespace drawinglayer
{
    // #i101811#
    // Added current ViewInformation2D to take evtl. changed
    // ObjectTransformation into account
    bool renderChartPrimitive2D(
        const primitive2d::ChartPrimitive2D& rChartCandidate,
        OutputDevice& rOutputDevice,
        const geometry::ViewInformation2D& rViewInformation2D);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX

// eof
