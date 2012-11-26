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



#ifndef _SDR_OVERLAY_OVERLAYSELECTION_HXX
#define _SDR_OVERLAY_OVERLAYSELECTION_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/region.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        // overlay type definition
        enum OverlayType
        {
            OVERLAY_INVERT,
            OVERLAY_SOLID,
            OVERLAY_TRANSPARENT
        };

        class SVX_DLLPUBLIC OverlaySelection : public OverlayObject
        {
        protected:
            // type of overlay
            OverlayType                         meOverlayType;

            // geometry of overlay
            std::vector< basegfx::B2DRange >    maRanges;

            // Values of last primitive creation. These are checked in getOverlayObjectPrimitive2DSequence
            // to evtl. get rid of last Primitive2DSequence. This ensures that these values are up-to-date
            // and are usable when creating primitives
            OverlayType                         maLastOverlayType;
            sal_uInt16                          mnLastTransparence;

            // bitfield
            bool                                mbBorder : 1;

            // geometry creation for OverlayObject, can use local *Last* values
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlaySelection(
                OverlayType eType,
                const Color& rColor,
                const std::vector< basegfx::B2DRange >& rRanges,
                bool bBorder);
            virtual ~OverlaySelection();

            // data read access
            OverlayType getOverlayType() const { return meOverlayType; }
            const std::vector< basegfx::B2DRange >& getRanges() const { return maRanges; }
            bool getBorder() const { return mbBorder; }

            // overloaded to check conditions for last createOverlayObjectPrimitive2DSequence
            virtual drawinglayer::primitive2d::Primitive2DSequence getOverlayObjectPrimitive2DSequence() const;

            // data write access
            void setRanges(const std::vector< basegfx::B2DRange >& rNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYSELECTION_HXX

// eof
