/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#define INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/range/b2irange.hxx>
#include <vcl/virdev.hxx>
#include <vcl/idle.hxx>


namespace sdr::overlay
    {
        class OverlayManagerBuffered final : public OverlayManager
        {
            // The VirtualDevice for draw window content buffering, this
            // is the view content without overlay
            ScopedVclPtr<VirtualDevice>             mpBufferDevice;

            // #i73602# The VirtualDevice for OverlayPaint buffering. This
            // is an extra device to avoid flickering of overlay paints
            ScopedVclPtr<VirtualDevice>             mpOutputBufferDevice;

            // Idle for buffering
            Idle                                    maBufferIdle;

            // Range for buffering (in pixel to be independent from mapMode)
            basegfx::B2IRange                       maBufferRememberedRangePixel;

            // link for timer
            DECL_LINK(ImpBufferTimerHandler, Timer*, void);

            // Internal methods for buffering
            void ImpPrepareBufferDevice();
            void ImpRestoreBackground() const ;
            void ImpRestoreBackground(const vcl::Region& rRegionPixel) const;
            void ImpSaveBackground(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice);

            OverlayManagerBuffered(OutputDevice& rOutputDevice);
            virtual ~OverlayManagerBuffered() override;

        public:
            static rtl::Reference<OverlayManager> create(OutputDevice& rOutputDevice);

            // complete redraw
            virtual void completeRedraw(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice = nullptr) const override;

            // invalidate the given range at local OutputDevice
            virtual void invalidateRange(const basegfx::B2DRange& rRange) override;
        };
} // end of namespace sdr::overlay


#endif // INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
