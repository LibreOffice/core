/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaymanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:12:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayManager::ImpDrawMembers(const basegfx::B2DRange& rRange, OutputDevice& rDestinationDevice) const
        {
            ::sdr::overlay::OverlayObject* pCurrent = mpOverlayObjectStart;

            while(pCurrent)
            {
                if(pCurrent->isVisible())
                {
                    if(rRange.overlaps(pCurrent->getBaseRange()))
                    {
                        pCurrent->drawGeometry(rDestinationDevice);
                    }
                }

                pCurrent = pCurrent->mpNext;
            }
        }

        void OverlayManager::ImpCheckMapModeChange() const
        {
            sal_Bool bZoomHasChanged(sal_False);
            MapMode aOutputDeviceMapMode(getOutputDevice().GetMapMode());
            ::sdr::overlay::OverlayObject* pCurrent = mpOverlayObjectStart;

            if(maMapMode != aOutputDeviceMapMode)
            {
                bZoomHasChanged = (
                    maMapMode.GetScaleX() != aOutputDeviceMapMode.GetScaleX()
                    || maMapMode.GetScaleY() != aOutputDeviceMapMode.GetScaleY());

                // remember MapMode
                ((OverlayManager*)this)->maMapMode = aOutputDeviceMapMode;
            }

            if(bZoomHasChanged && pCurrent)
            {
                while(pCurrent)
                {
                    pCurrent->zoomHasChanged();
                    pCurrent = pCurrent->mpNext;
                }
            }
        }

        void OverlayManager::ImpStripeDefinitionChanged()
        {
            ::sdr::overlay::OverlayObject* pCurrent = mpOverlayObjectStart;

            while(pCurrent)
            {
                pCurrent->stripeDefinitionHasChanged();
                pCurrent = pCurrent->mpNext;
            }
        }

        OverlayManager::OverlayManager(OutputDevice& rOutputDevice)
        :   Scheduler(),
            rmOutputDevice(rOutputDevice),
            mpOverlayObjectStart(0L),
            mpOverlayObjectEnd(0L),
            maStripeColorA(Color(COL_BLACK)),
            maStripeColorB(Color(COL_WHITE)),
            mnStripeLengthPixel(5L)
        {
        }

        OverlayManager::~OverlayManager()
        {
            // the OverlayManager is not the owner of the OverlayObjects
            // and thus will not delete them, but remove them.
            while(mpOverlayObjectStart)
            {
                remove(*mpOverlayObjectStart);
            }
        }

        void OverlayManager::completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty() && mpOverlayObjectStart)
            {
                // check for changed MapModes. That may influence the
                // logical size of pixel based OverlayObjects (like BitmapHandles)
                ImpCheckMapModeChange();

                // paint members
                const Rectangle aRegionBoundRect(rRegion.GetBoundRect());
                const basegfx::B2DRange aRegionRange(
                    aRegionBoundRect.Left(), aRegionBoundRect.Top(),
                    aRegionBoundRect.Right(), aRegionBoundRect.Bottom());

                OutputDevice& rTarget = (pPreRenderDevice) ? *pPreRenderDevice : getOutputDevice();
                ImpDrawMembers(aRegionRange, rTarget);
            }
        }

        void OverlayManager::flush()
        {
            // default has nothing to do
        }

        void OverlayManager::restoreBackground(const Region& /*rRegion*/) const
        {
            // unbuffered versions do nothing here
        }

        void OverlayManager::add(OverlayObject& rOverlayObject)
        {
            // add to the end of chain to preserve display order in paint
            DBG_ASSERT(0L == rOverlayObject.mpOverlayManager,
                "OverlayManager::add: OverlayObject is added to an OverlayManager (!)");

            if(mpOverlayObjectEnd)
            {
                // new element, add to end
                rOverlayObject.mpNext = mpOverlayObjectEnd->mpNext;
                rOverlayObject.mpPrevious = mpOverlayObjectEnd;
                mpOverlayObjectEnd->mpNext = &rOverlayObject;
                mpOverlayObjectEnd = &rOverlayObject;
            }
            else
            {
                // first element
                rOverlayObject.mpNext = rOverlayObject.mpPrevious = 0L;
                mpOverlayObjectEnd = mpOverlayObjectStart = &rOverlayObject;
            }

            // set manager
            rOverlayObject.mpOverlayManager = this;

            // make visible
            invalidateRange(rOverlayObject.getBaseRange());

            // handle evtl. animation
            if(rOverlayObject.allowsAnimation())
            {
                InsertEvent(&rOverlayObject);
                Execute();
            }
        }

        void OverlayManager::remove(OverlayObject& rOverlayObject)
        {
            // handle evtl. animation
            if(rOverlayObject.allowsAnimation())
            {
                // remove from event chain
                RemoveEvent(&rOverlayObject);
            }

            // Remove from chain
            DBG_ASSERT(rOverlayObject.mpOverlayManager == this,
                "OverlayManager::remove: OverlayObject is removed from wrong OverlayManager (!)");

            if(rOverlayObject.mpPrevious)
            {
                rOverlayObject.mpPrevious->mpNext = rOverlayObject.mpNext;
            }

            if(rOverlayObject.mpNext)
            {
                rOverlayObject.mpNext->mpPrevious = rOverlayObject.mpPrevious;
            }

            if(&rOverlayObject == mpOverlayObjectStart)
            {
                mpOverlayObjectStart = rOverlayObject.mpNext;
            }

            if(&rOverlayObject == mpOverlayObjectEnd)
            {
                mpOverlayObjectEnd = rOverlayObject.mpPrevious;
            }

            // make invisible
            invalidateRange(rOverlayObject.getBaseRange());

            // clear manager
            rOverlayObject.mpOverlayManager = 0L;
        }

        void OverlayManager::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if(OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
            {
                // transform to rectangle
                const basegfx::B2DPoint aMinimum(rRange.getMinimum());
                const basegfx::B2DPoint aMaximum(rRange.getMaximum());
                const Rectangle aInvalidateRectangle(
                    FRound(aMinimum.getX()), FRound(aMinimum.getY()),
                    FRound(aMaximum.getX()), FRound(aMaximum.getY()));

                // simply invalidate
                ((Window&)getOutputDevice()).Invalidate(aInvalidateRectangle, INVALIDATE_NOERASE);
            }
        }

        // stripe support ColA
        void OverlayManager::setStripeColorA(Color aNew)
        {
            if(aNew != maStripeColorA)
            {
                maStripeColorA = aNew;
                ImpStripeDefinitionChanged();
            }
        }

        // stripe support ColB
        void OverlayManager::setStripeColorB(Color aNew)
        {
            if(aNew != maStripeColorB)
            {
                maStripeColorB = aNew;
                ImpStripeDefinitionChanged();
            }
        }

        // stripe support StripeLengthPixel
        void OverlayManager::setStripeLengthPixel(sal_uInt32 nNew)
        {
            if(nNew != mnStripeLengthPixel)
            {
                mnStripeLengthPixel = nNew;
                ImpStripeDefinitionChanged();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
