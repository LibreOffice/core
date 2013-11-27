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



#ifndef _SW_OVERLAY_OVERLAYRANGESOUTLINE_HXX
#define _SW_OVERLAY_OVERLAYRANGESOUTLINE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/range/b2drange.hxx>

#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace sw
{
    namespace overlay
    {
        class OverlayRangesOutline : public sdr::overlay::OverlayObject
        {
        protected:
            // geometry of overlay
            std::vector< basegfx::B2DRange > maRanges;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayRangesOutline(
                const Color& rColor,
                const std::vector< basegfx::B2DRange >& rRanges );

            virtual ~OverlayRangesOutline();

            // data read access
            inline const std::vector< basegfx::B2DRange >& getRanges() const
            {
                return maRanges;
            }

            // data write access
            void setRanges(const std::vector< basegfx::B2DRange >& rNew);
        };
    } // end of namespace overlay
} // end of namespace sw

//////////////////////////////////////////////////////////////////////////////

#endif //_SW_OVERLAY_OVERLAYRANGESOUTLINE_HXX

// eof
