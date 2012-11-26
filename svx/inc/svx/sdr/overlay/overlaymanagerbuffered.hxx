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



#ifndef _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#define _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/range/b2irange.hxx>
#include <vcl/virdev.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class VirtualDevice;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayManagerBuffered : public OverlayManager
        {
        protected:
            // The VirtualDevice for draw window content buffering, this
            // is the view content without overlay
            VirtualDevice                           maBufferDevice;

            // #i73602# The VirtualDevice for OverlayPaint buffering. This
            // is an extra device to avoid flickering of overlay paints
            VirtualDevice                           maOutputBufferDevice;

            // Timer for buffering
            Timer                                   maBufferTimer;

            // Range for buffering (in pixel to be independent from mapMode)
            basegfx::B2IRange                       maBufferRememberedRangePixel;

            // bitfield
            // Flag to decide if PreRendering shall be used for overlay refreshes.
            // Default is false.
            bool                                    mbRefreshWithPreRendering : 1;

            // link for timer
            DECL_LINK(ImpBufferTimerHandler, AutoTimer*);

            // Internal methods for buffering
            void ImpPrepareBufferDevice();
            void ImpRestoreBackground() const ;
            void ImpRestoreBackground(const Region& rRegionPixel) const;
            void ImpSaveBackground(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L);

        public:
            OverlayManagerBuffered(
                OutputDevice& rOutputDevice,
                bool bRefreshWithPreRendering = false);
            virtual ~OverlayManagerBuffered();

            // complete redraw
            virtual void completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L) const;

            // flush. Do buffered updates.
            virtual void flush();

            // #i68597# part of content gets copied, react on it
            virtual void copyArea(const Point& rDestPt, const Point& rSrcPt, const Size& rSrcSize);

            // restore part of background. Implemented form buffered versions only.
            virtual void restoreBackground(const Region& rRegion) const;

            // invalidate the given range at local OutputDevice
            virtual void invalidateRange(const basegfx::B2DRange& rRange);

            // access to RefreshWithPreRendering Flag
            bool DoRefreshWithPreRendering() const { return mbRefreshWithPreRendering; }
            void SetRefreshWithPreRendering(bool bNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

// eof
