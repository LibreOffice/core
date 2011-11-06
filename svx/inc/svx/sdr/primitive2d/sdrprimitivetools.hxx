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



#ifndef INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX
#define INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX

#include <vcl/bitmapex.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////
// helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        // create a 3x3 cross in given color as BitmapEx
        BitmapEx createDefaultCross_3x3(const basegfx::BColor& rBColor);

        // create a 7x7 gluepoint symbol in given colors as BitmapEx
        BitmapEx createDefaultGluepoint_7x7(const basegfx::BColor& rBColorA, const basegfx::BColor& rBColorB);
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX

// eof
