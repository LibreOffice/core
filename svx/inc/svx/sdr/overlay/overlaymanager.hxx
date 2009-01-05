/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaymanager.hxx,v $
 *
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

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#define _SDR_OVERLAY_OVERLAYMANAGER_HXX

#include <svx/sdr/animation/scheduler.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/mapmod.hxx>
#include <tools/color.hxx>
#include "svx/svxdllapi.h"
#include <svtools/optionsdrawinglayer.hxx>
#include <boost/shared_ptr.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class OutputDevice;
class Region;

namespace sdr { namespace overlay {
    class OverlayObject;
}}

namespace basegfx {
    class B2DRange;
}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayManager : public ::sdr::animation::Scheduler
        {
        protected:
            // the OutputDevice to work on, set on construction and not to be changed
            OutputDevice&                               rmOutputDevice;

            // start, end and number of the double linked list of OverlayObjects
            // managed by this manager
            OverlayObject*                              mpOverlayObjectStart;
            OverlayObject*                              mpOverlayObjectEnd;

            // MapMode for MapMode change watching
            MapMode                                     maMapMode;

            // Stripe support. All striped OverlayObjects use these stripe
            // values. Changes change all those objects.
            Color                                       maStripeColorA; // defaults to Color(COL_BLACK)
            Color                                       maStripeColorB; // defaults to Color(COL_WHITE)
            sal_uInt32                                  mnStripeLengthPixel; // defaults to 4L

            // hold an incarnation of Drawinglayer configuration options
            SvtOptionsDrawinglayer                      maDrawinglayerOpt;

            // hold buffered the logic length of discrete vector (1.0, 0.0) and the
            // view transformation belonging to it. Update happens in getDiscreteOne()
            basegfx::B2DHomMatrix                       maViewTransformation;
            double                                      mfDiscreteOne;

            // internal
            void ImpDrawMembers(const basegfx::B2DRange& rRange, OutputDevice& rDestinationDevice) const;
            void ImpCheckMapModeChange() const;
            void ImpStripeDefinitionChanged();

            // return mfDiscreteOne to derivations, but also check for buffered local
            // ViewTransformation and evtl. correct mfDiscreteOne
            const double getDiscreteOne() const;

        public:
            OverlayManager(OutputDevice& rOutputDevice);
            virtual ~OverlayManager();

            // complete redraw
            virtual void completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice = 0L) const;

            // flush. Do buffered updates.
            virtual void flush();

            // #i68597# part of content gets copied, react on it
            virtual void copyArea(const Point& rDestPt, const Point& rSrcPt, const Size& rSrcSize);

            // restore part of background. Implemented form buffered versions only.
            virtual void restoreBackground(const Region& rRegion) const;

            // get the OutputDevice
            OutputDevice& getOutputDevice() const { return rmOutputDevice; }

            // add and remove OverlayObjects
            void add(OverlayObject& rOverlayObject);
            void remove(OverlayObject& rOverlayObject);

            // invalidate the given range at local OutputDevice
            virtual void invalidateRange(const basegfx::B2DRange& rRange);

            // stripe support ColA
            Color getStripeColorA() const { return maStripeColorA; }
            void setStripeColorA(Color aNew= Color(COL_BLACK));

            // stripe support ColB
            Color getStripeColorB() const { return maStripeColorB; }
            void setStripeColorB(Color aNew = Color(COL_WHITE));

            // stripe support StripeLengthPixel
            sal_uInt32 getStripeLengthPixel() const { return mnStripeLengthPixel; }
            void setStripeLengthPixel(sal_uInt32 nNew = 5L);

            // access to maDrawinglayerOpt
            const SvtOptionsDrawinglayer& getDrawinglayerOpt() const { return maDrawinglayerOpt; }

            /** Return a list of all OverlayObjects that currently belong to
                the called OverlayManager.  Subsequent calls to add() or
                remove() will not alter the content of the returned list.
                Modifying the list will not change the list of
                OverlayObjects that belong to the called OverlayManager.
                @return
                    The returned pointer is never empty but the pointed-to
                    vector may be.
            */
            ::boost::shared_ptr<OverlayObjectVector> GetOverlayObjects (void) const;
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYMANAGER_HXX

// eof
