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



#ifndef _SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX
#define _SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SfxItemSet;

namespace drawinglayer { namespace attribute {
    class Sdr3DObjectAttribute;
}}

namespace basegfx {
    class B2DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // SdrAttribute creators
        attribute::Sdr3DObjectAttribute* createNewSdr3DObjectAttribute(const SfxItemSet& rSet);

        // helpers

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PRIMITIVE3D_SDRATTRIBUTECREATOR3D_HXX

// eof
