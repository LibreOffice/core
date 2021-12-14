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

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/range/b2drange.hxx>
#include <comphelper/propertyvalue.hxx>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <osl/diagnose.h>
#include <memory>


using namespace com::sun::star;


namespace sdr::overlay
{
        void OverlayManager::ImpDrawMembers(const basegfx::B2DRange& rRange, OutputDevice& rDestinationDevice) const
        {
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(!nSize)
                return;

            const AntialiasingFlags nOriginalAA(rDestinationDevice.GetAntialiasing());
            const bool bIsAntiAliasing(SvtOptionsDrawinglayer::IsAntiAliasing());

            // create processor
            std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                rDestinationDevice,
                getCurrentViewInformation2D()));

            for(const auto& rpOverlayObject : maOverlayObjects)
            {
                OSL_ENSURE(rpOverlayObject, "Corrupted OverlayObject List (!)");
                const OverlayObject& rCandidate = *rpOverlayObject;

                if(rCandidate.isVisible())
                {
                    const drawinglayer::primitive2d::Primitive2DContainer& rSequence = rCandidate.getOverlayObjectPrimitive2DSequence();

                    if(!rSequence.empty())
                    {
                        if(rRange.overlaps(rCandidate.getBaseRange()))
                        {
                            if(bIsAntiAliasing && rCandidate.allowsAntiAliase())
                            {
                                rDestinationDevice.SetAntialiasing(nOriginalAA | AntialiasingFlags::Enable);
                            }
                            else
                            {
                                rDestinationDevice.SetAntialiasing(nOriginalAA & ~AntialiasingFlags::Enable);
                            }

                            pProcessor->process(rSequence);
                        }
                    }
                }
            }

            pProcessor.reset();

            // restore AA settings
            rDestinationDevice.SetAntialiasing(nOriginalAA);
        }

        void OverlayManager::ImpStripeDefinitionChanged()
        {
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(nSize)
            {
                for(const auto& rpOverlayObject : maOverlayObjects)
                {
                    OSL_ENSURE(rpOverlayObject, "Corrupted OverlayObject List (!)");
                    OverlayObject& rCandidate = *rpOverlayObject;
                    rCandidate.stripeDefinitionHasChanged();
                }
            }
        }

        double OverlayManager::getDiscreteOne() const
        {
            if(basegfx::fTools::equalZero(mfDiscreteOne))
            {
                const basegfx::B2DVector aDiscreteInLogic(getOutputDevice().GetInverseViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const_cast< OverlayManager* >(this)->mfDiscreteOne = aDiscreteInLogic.getLength();
            }

            return mfDiscreteOne;
        }

        OverlayManager::OverlayManager(OutputDevice& rOutputDevice)
        :   mrOutputDevice(rOutputDevice),
            maStripeColorA(COL_BLACK),
            maStripeColorB(COL_WHITE),
            mnStripeLengthPixel(5),
            mfDiscreteOne(0.0)
        {
            // set Property 'ReducedDisplayQuality' to true to allow simpler interaction
            // visualisations
            uno::Sequence< beans::PropertyValue > xProperties{
                comphelper::makePropertyValue("ReducedDisplayQuality", true)
            };
            maViewInformation2D = drawinglayer::geometry::createViewInformation2D(xProperties);
        }

        rtl::Reference<OverlayManager> OverlayManager::create(OutputDevice& rOutputDevice)
        {
            return rtl::Reference<OverlayManager>(new OverlayManager(rOutputDevice));
        }

        drawinglayer::geometry::ViewInformation2D const & OverlayManager::getCurrentViewInformation2D() const
        {
            if(getOutputDevice().GetViewTransformation() != maViewTransformation)
            {
                basegfx::B2DRange aViewRange(maViewInformation2D.getViewport());

                if(OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
                {
                    const Size aOutputSizePixel(getOutputDevice().GetOutputSizePixel());

                    // only set when we *have* an output size, else let aViewRange
                    // stay on empty
                    if(aOutputSizePixel.Width() && aOutputSizePixel.Height())
                    {
                        aViewRange = basegfx::B2DRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());
                        aViewRange.transform(getOutputDevice().GetInverseViewTransformation());
                    }
                }

                OverlayManager* pThis = const_cast< OverlayManager* >(this);

                pThis->maViewTransformation = getOutputDevice().GetViewTransformation();
                pThis->maViewInformation2D = drawinglayer::geometry::ViewInformation2D(
                    maViewInformation2D.getObjectTransformation(),
                    maViewTransformation,
                    aViewRange,
                    maViewInformation2D.getVisualizedPage(),
                    maViewInformation2D.getViewTime());
                pThis->mfDiscreteOne = 0.0;
            }

            return maViewInformation2D;
        }

        void OverlayManager::impApplyRemoveActions(OverlayObject& rTarget)
        {
            // handle evtl. animation
            if(rTarget.allowsAnimation())
            {
                // remove from event chain
                RemoveEvent(&rTarget);
            }

            // make invisible
            invalidateRange(rTarget.getBaseRange());

            // clear manager
            rTarget.mpOverlayManager = nullptr;
        }

        void OverlayManager::impApplyAddActions(OverlayObject& rTarget)
        {
            // set manager
            rTarget.mpOverlayManager = this;

            // make visible
            invalidateRange(rTarget.getBaseRange());

            // handle evtl. animation
            if(rTarget.allowsAnimation())
            {
                // Trigger at current time to get alive. This will do the
                // object-specific next time calculation and hand over adding
                // again to the scheduler to the animated object, too. This works for
                // a paused or non-paused animator.
                rTarget.Trigger(GetTime());
            }
        }

        OverlayManager::~OverlayManager()
        {
            // The OverlayManager is not the owner of the OverlayObjects
            // and thus will not delete them, but remove them. Profit here
            // from knowing that all will be removed
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(nSize)
            {
                for(const auto& rpOverlayObject : maOverlayObjects)
                {
                    OSL_ENSURE(rpOverlayObject, "Corrupted OverlayObject List (!)");
                    OverlayObject& rCandidate = *rpOverlayObject;
                    impApplyRemoveActions(rCandidate);
                }

                // erase vector
                maOverlayObjects.clear();
            }
        }

        void OverlayManager::completeRedraw(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(rRegion.IsEmpty() || maOverlayObjects.empty())
                return;

            // check for changed MapModes. That may influence the
            // logical size of pixel based OverlayObjects (like BitmapHandles)
            //ImpCheckMapModeChange();

            // paint members
            const tools::Rectangle aRegionBoundRect(rRegion.GetBoundRect());
            const basegfx::B2DRange aRegionRange = vcl::unotools::b2DRectangleFromRectangle(aRegionBoundRect);

            OutputDevice& rTarget = pPreRenderDevice ? *pPreRenderDevice : getOutputDevice();
            ImpDrawMembers(aRegionRange, rTarget);
        }

        void OverlayManager::flush()
        {
            // default has nothing to do
        }

        void OverlayManager::add(OverlayObject& rOverlayObject)
        {
            OSL_ENSURE(nullptr == rOverlayObject.mpOverlayManager, "OverlayObject is added twice to an OverlayManager (!)");

            // add to the end of chain to preserve display order in paint
            maOverlayObjects.push_back(&rOverlayObject);

            // execute add actions
            impApplyAddActions(rOverlayObject);
        }

        void OverlayManager::remove(OverlayObject& rOverlayObject)
        {
            OSL_ENSURE(rOverlayObject.mpOverlayManager == this, "OverlayObject is removed from wrong OverlayManager (!)");

            // execute remove actions
            impApplyRemoveActions(rOverlayObject);

            // remove from vector
            const OverlayObjectVector::iterator aFindResult = ::std::find(maOverlayObjects.begin(), maOverlayObjects.end(), &rOverlayObject);
            const bool bFound(aFindResult != maOverlayObjects.end());
            OSL_ENSURE(bFound, "OverlayObject NOT found at OverlayManager (!)");

            if(bFound)
            {
                maOverlayObjects.erase(aFindResult);
            }
        }

        tools::Rectangle OverlayManager::RangeToInvalidateRectangle(const basegfx::B2DRange& rRange) const
        {
            if (rRange.isEmpty()) {
                return {};
            }
            if (SvtOptionsDrawinglayer::IsAntiAliasing())
            {
                // assume AA needs one pixel more and invalidate one pixel more
                const double fDiscreteOne(getDiscreteOne());
                const tools::Rectangle aInvalidateRectangle(
                    static_cast<tools::Long>(floor(rRange.getMinX() - fDiscreteOne)),
                    static_cast<tools::Long>(floor(rRange.getMinY() - fDiscreteOne)),
                    static_cast<tools::Long>(ceil(rRange.getMaxX() + fDiscreteOne)),
                    static_cast<tools::Long>(ceil(rRange.getMaxY() + fDiscreteOne)));
                return aInvalidateRectangle;
            }
            else
            {
                // #i77674# transform to rectangle. Use floor/ceil to get all covered
                // discrete pixels, see #i75163# and OverlayManagerBuffered::invalidateRange
                const tools::Rectangle aInvalidateRectangle(
                    static_cast<sal_Int32>(floor(rRange.getMinX())), static_cast<sal_Int32>(floor(rRange.getMinY())),
                    static_cast<sal_Int32>(ceil(rRange.getMaxX())), static_cast<sal_Int32>(ceil(rRange.getMaxY())));
                return aInvalidateRectangle;
            }
        }

        void OverlayManager::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if (OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
            {
                tools::Rectangle aInvalidateRectangle(RangeToInvalidateRectangle(rRange));
                // simply invalidate
                getOutputDevice().GetOwnerWindow()->Invalidate(aInvalidateRectangle, InvalidateFlags::NoErase);
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

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
