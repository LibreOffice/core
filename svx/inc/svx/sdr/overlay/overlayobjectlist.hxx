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



#ifndef _SDR_OVERLAY_OVERLAYOBJECTLIST_HXX
#define _SDR_OVERLAY_OVERLAYOBJECTLIST_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////

#define DEFAULT_VALUE_FOR_HITTEST_PIXEL         (2L)

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayObjectList
        {
        protected:
            // the vector of OverlayObjects
            OverlayObjectVector                     maVector;

            // default value for HiTestPixel
            sal_uInt32 getDefaultValueForHitTestPixel() const { return 4L; }

        public:
            OverlayObjectList() {}
            ~OverlayObjectList();

            // clear list, this includes deletion of all contained objects
            void clear();

            // append/remove objects
            void append(OverlayObject& rOverlayObject) { maVector.push_back(&rOverlayObject); }
            void remove(OverlayObject& rOverlayObject);

            // access to objects
            sal_uInt32 count() const { return maVector.size(); }
            OverlayObject& getOverlayObject(sal_uInt32 nIndex) const { return *(maVector[nIndex]); }

            // Hittest with logical coordinates
            bool isHitLogic(const basegfx::B2DPoint& rLogicPosition, double fLogicTolerance = 0.0) const;

            // Hittest with pixel coordinates and pixel tolerance
            bool isHitPixel(const Point& rDiscretePosition, sal_uInt32 fDiscreteTolerance = DEFAULT_VALUE_FOR_HITTEST_PIXEL) const;

            // calculate BaseRange of all included OverlayObjects and return
            basegfx::B2DRange getBaseRange() const;
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYOBJECTLIST_HXX

// eof
