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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYMANAGER_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYMANAGER_HXX

#include <rtl/ref.hxx>
#include <svx/sdr/animation/scheduler.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <salhelper/simplereferenceobject.hxx>

class OutputDevice;
namespace vcl { class Region; }

namespace basegfx {
    class B2DRange;
}

namespace sdr::overlay
    {
        class SVXCORE_DLLPUBLIC OverlayManager
            : protected sdr::animation::Scheduler
            , public salhelper::SimpleReferenceObject
        {
        private:
            OverlayManager(const OverlayManager&) = delete;
            OverlayManager& operator=(const OverlayManager&) = delete;

        protected:
            // the OutputDevice to work on, set on construction and not to be changed
            OutputDevice&                               mrOutputDevice;

            // the vector of registered OverlayObjects
            OverlayObjectVector                         maOverlayObjects;

            // Stripe support. All striped OverlayObjects use these stripe
            // values. Changes change all those objects.
            Color                                       maStripeColorA; // defaults to COL_BLACK
            Color                                       maStripeColorB; // defaults to COL_WHITE
            sal_uInt32                                  mnStripeLengthPixel; // defaults to 4L

            // hold buffered the logic length of discrete vector (1.0, 0.0) and the
            // view transformation belonging to it. Update happens in getDiscreteOne()
            basegfx::B2DHomMatrix                       maViewTransformation;
            drawinglayer::geometry::ViewInformation2D   maViewInformation2D;
            double                                      mfDiscreteOne;

            // internal
            void ImpDrawMembers(const basegfx::B2DRange& rRange, OutputDevice& rDestinationDevice) const;
            void ImpStripeDefinitionChanged();
            void impApplyRemoveActions(OverlayObject& rTarget);
            void impApplyAddActions(OverlayObject& rTarget);

            // return mfDiscreteOne to derivations, but also check for buffered local
            // ViewTransformation and evtl. correct mfDiscreteOne
            double getDiscreteOne() const;

            tools::Rectangle RangeToInvalidateRectangle(const basegfx::B2DRange& rRange) const;

            OverlayManager(OutputDevice& rOutputDevice);
            virtual ~OverlayManager() override;

        public:
            static rtl::Reference<OverlayManager> create(OutputDevice& rOutputDevice);

            // access to current ViewInformation2D; this call checks and evtl. updates ViewInformation2D
            drawinglayer::geometry::ViewInformation2D const & getCurrentViewInformation2D() const;

            // complete redraw
            virtual void completeRedraw(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice = nullptr) const;

            // flush. Do buffered updates.
            virtual void flush();

            // get the OutputDevice
            OutputDevice& getOutputDevice() const { return mrOutputDevice; }

            // add and remove OverlayObjects
            void add(OverlayObject& rOverlayObject);
            void remove(OverlayObject& rOverlayObject);

            // invalidate the given range at local OutputDevice
            virtual void invalidateRange(const basegfx::B2DRange& rRange);

            // stripe support ColA
            const Color& getStripeColorA() const { return maStripeColorA; }
            void setStripeColorA(Color aNew);

            // stripe support ColB
            const Color& getStripeColorB() const { return maStripeColorB; }
            void setStripeColorB(Color aNew);

            // stripe support StripeLengthPixel
            sal_uInt32 getStripeLengthPixel() const { return mnStripeLengthPixel; }
            void setStripeLengthPixel(sal_uInt32 nNew);

            void InsertEvent(sdr::animation::Event& rNew) { Scheduler::InsertEvent(rNew); }
        };

} // end of namespace sdr::overlay

#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
