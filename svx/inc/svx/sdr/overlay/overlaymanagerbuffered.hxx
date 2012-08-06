/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#define _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/range/b2irange.hxx>
#include <vcl/virdev.hxx>

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
            unsigned                                mbRefreshWithPreRendering : 1;

            // link for timer
            DECL_LINK(ImpBufferTimerHandler, AutoTimer*);

            // Internal methods for buffering
            void ImpPrepareBufferDevice();
            void ImpRestoreBackground() const ;
            void ImpRestoreBackground(const Region& rRegionPixel) const;
            void ImpSaveBackground(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L);

            // when handing over another OverlayManager at construction, the OverlayObjects
            // will be taken over from it. The new one will have added all OverlayObjects
            // while the handed over one will have none
            OverlayManagerBuffered(
                OutputDevice& rOutputDevice,
                OverlayManager* pOldOverlayManager,
                bool bRefreshWithPreRendering);
            virtual ~OverlayManagerBuffered();

        public:
            static rtl::Reference<OverlayManager> create(OutputDevice& rOutputDevice,
                OverlayManager* pOldOverlayManager = 0, bool bRefreshWithPreRendering = false);

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
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
