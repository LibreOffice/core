/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaymanagerbuffered.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:08:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#define _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _BGFX_RANGE_B2IRANGE_HXX
#include <basegfx/range/b2irange.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

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
            // The VirtualDevice for buffering
            VirtualDevice                           maBufferDevice;

            // Timer for buffering
            Timer                                   maBufferTimer;

            // Range for buffering (in pixel to be independent from mapMode)
            basegfx::B2IRange                       maBufferRememberedRangePixel;

            // bitfield
            // Flag to decide if PreRendering shall be used for overlay refreshes.
            // Default is sal_False.
            unsigned                                mbRefreshWithPreRendering : 1;

            // link for timer
            DECL_LINK(ImpBufferTimerHandler, AutoTimer*);

            // Internal methods for buffering
            void ImpPrepareBufferDevice();
            void ImpRestoreBackground() const ;
            void ImpRestoreBackground(const Rectangle& rRegionRectanglePixel) const;
            void ImpSaveBackground(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L);

        public:
            OverlayManagerBuffered(OutputDevice& rOutputDevice, sal_Bool bRefreshWithPreRendering = sal_False);
            virtual ~OverlayManagerBuffered();

            // complete redraw
            virtual void completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L) const;

            // flush. Do buffered updates.
            virtual void flush();

            // restore part of background. Implemented form buffered versions only.
            virtual void restoreBackground(const Region& rRegion) const;

            // invalidate the given range at local OutputDevice
            virtual void invalidateRange(const basegfx::B2DRange& rRange);

            // access to RefreshWithPreRendering Flag
            sal_Bool DoRefreshWithPreRendering() const { return mbRefreshWithPreRendering; }
            void SetRefreshWithPreRendering(sal_Bool bNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX

// eof
