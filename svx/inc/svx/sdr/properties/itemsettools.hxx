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



#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#define _SDR_PROPERTIES_ITEMSETTOOLS_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrObject;
class SfxItemSet;
class Rectangle;
class Fraction;

//////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions
namespace sdr
{
    namespace properties
    {
        class ItemChangeBroadcaster
        {
            sal_uInt32                                      mnCount;
            void*                                           mpData;

        public:
            ItemChangeBroadcaster(const SdrObject& rObj);
            ~ItemChangeBroadcaster();

            sal_uInt32 GetRectangleCount() const;
            const Rectangle& GetRectangle(sal_uInt32 nIndex) const;
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale);
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_ITEMSETTOOLS_HXX

// eof
