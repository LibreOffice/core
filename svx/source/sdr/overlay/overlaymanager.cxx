/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaymanager.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>

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

        // #i68597# part of content gets copied, react on it
        void OverlayManager::copyArea(const Point& /*rDestPt*/, const Point& /*rSrcPt*/, const Size& /*rSrcSize*/)
        {
            // unbuffered versions do nothing here
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
                // #i77674# transform to rectangle. Use floor/ceil to get all covered
                // discrete pixels, see #i75163# and OverlayManagerBuffered::invalidateRange
                const Rectangle aInvalidateRectangle(
                    (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
                    (sal_Int32)ceil(rRange.getMaxX()), (sal_Int32)ceil(rRange.getMaxY()));

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
