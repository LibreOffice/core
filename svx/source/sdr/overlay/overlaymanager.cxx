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

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayManager::ImpDrawMembers(const basegfx::B2DRange& rRange, OutputDevice& rDestinationDevice) const
        {
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(nSize)
            {
                const sal_uInt16 nOriginalAA(rDestinationDevice.GetAntialiasing());
                const bool bIsAntiAliasing(getDrawinglayerOpt().IsAntiAliasing());

                // create processor
                drawinglayer::processor2d::BaseProcessor2D* pProcessor =
                    ::drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                        rDestinationDevice,
                        getCurrentViewInformation2D());

                if(pProcessor)
                {
                    for(OverlayObjectVector::const_iterator aIter(maOverlayObjects.begin()); aIter != maOverlayObjects.end(); ++aIter)
                    {
                        OSL_ENSURE(*aIter, "Corrupted OverlayObject List (!)");
                        const OverlayObject& rCandidate = **aIter;

                        if(rCandidate.isVisible())
                        {
                            const drawinglayer::primitive2d::Primitive2DSequence& rSequence = rCandidate.getOverlayObjectPrimitive2DSequence();

                            if(rSequence.hasElements())
                            {
                                if(rRange.overlaps(rCandidate.getBaseRange()))
                                {
                                    if(bIsAntiAliasing && rCandidate.allowsAntiAliase())
                                    {
                                        rDestinationDevice.SetAntialiasing(nOriginalAA | ANTIALIASING_ENABLE_B2DDRAW);
                                    }
                                    else
                                    {
                                        rDestinationDevice.SetAntialiasing(nOriginalAA & ~ANTIALIASING_ENABLE_B2DDRAW);
                                    }

                                    pProcessor->process(rSequence);
                                }
                            }
                        }
                    }

                    delete pProcessor;
                }

                // restore AA settings
                rDestinationDevice.SetAntialiasing(nOriginalAA);
            }
        }

        void OverlayManager::ImpStripeDefinitionChanged()
        {
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(nSize)
            {
                for(OverlayObjectVector::iterator aIter(maOverlayObjects.begin()); aIter != maOverlayObjects.end(); ++aIter)
                {
                    OSL_ENSURE(*aIter, "Corrupted OverlayObject List (!)");
                    OverlayObject& rCandidate = **aIter;
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

        OverlayManager::OverlayManager(
            OutputDevice& rOutputDevice,
            OverlayManager* pOldOverlayManager)
        :   Scheduler(),
            mnRefCount(0),
            rmOutputDevice(rOutputDevice),
            maOverlayObjects(),
            maStripeColorA(Color(COL_BLACK)),
            maStripeColorB(Color(COL_WHITE)),
            mnStripeLengthPixel(5),
            maDrawinglayerOpt(),
            maViewTransformation(),
            maViewInformation2D(),
            mfDiscreteOne(0.0)
        {
            // set Property 'ReducedDisplayQuality' to true to allow simpler interaction
            // visualisations
            static bool bUseReducedDisplayQualityForDrag(true);

            if(bUseReducedDisplayQualityForDrag)
            {
                uno::Sequence< beans::PropertyValue > xProperties(1);
                xProperties[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedDisplayQuality"));
                xProperties[0].Value <<= true;
                maViewInformation2D = drawinglayer::geometry::ViewInformation2D(xProperties);
            }

            if(pOldOverlayManager)
            {
                // take over OverlayObjects from given OverlayManager. Copy
                // the vector of pointers
                maOverlayObjects = pOldOverlayManager->maOverlayObjects;
                const sal_uInt32 nSize(maOverlayObjects.size());

                if(nSize)
                {
                    for(OverlayObjectVector::iterator aIter(maOverlayObjects.begin()); aIter != maOverlayObjects.end(); ++aIter)
                    {
                        OSL_ENSURE(*aIter, "Corrupted OverlayObject List (!)");
                        OverlayObject& rCandidate = **aIter;

                        // remove from old and add to new OverlayManager
                        pOldOverlayManager->impApplyRemoveActions(rCandidate);
                        impApplyAddActions(rCandidate);
                    }

                    pOldOverlayManager->maOverlayObjects.clear();
                }
            }
        }

        rtl::Reference<OverlayManager> OverlayManager::create(
            OutputDevice& rOutputDevice,
            OverlayManager* pOldOverlayManager)
        {
            return rtl::Reference<OverlayManager>(new OverlayManager(rOutputDevice,
                pOldOverlayManager));
        }

        const drawinglayer::geometry::ViewInformation2D OverlayManager::getCurrentViewInformation2D() const
        {
            if(getOutputDevice().GetViewTransformation() != maViewTransformation)
            {
                basegfx::B2DRange aViewRange(maViewInformation2D.getViewport());

                if(OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
                {
                    const Size aOutputSizePixel(getOutputDevice().GetOutputSizePixel());
                    aViewRange = basegfx::B2DRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());
                    aViewRange.transform(getOutputDevice().GetInverseViewTransformation());
                }

                OverlayManager* pThis = const_cast< OverlayManager* >(this);

                pThis->maViewTransformation = getOutputDevice().GetViewTransformation();
                pThis->maViewInformation2D = drawinglayer::geometry::ViewInformation2D(
                    maViewInformation2D.getObjectTransformation(),
                    maViewTransformation,
                    aViewRange,
                    maViewInformation2D.getVisualizedPage(),
                    maViewInformation2D.getViewTime(),
                    maViewInformation2D.getExtendedInformationSequence());
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
            rTarget.mpOverlayManager = 0;
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
                for(OverlayObjectVector::iterator aIter(maOverlayObjects.begin()); aIter != maOverlayObjects.end(); ++aIter)
                {
                    OSL_ENSURE(*aIter, "Corrupted OverlayObject List (!)");
                    OverlayObject& rCandidate = **aIter;
                    impApplyRemoveActions(rCandidate);
                }

                // erase vector
                maOverlayObjects.clear();
            }
        }

        void OverlayManager::completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty() && maOverlayObjects.size())
            {
                // check for changed MapModes. That may influence the
                // logical size of pixel based OverlayObjects (like BitmapHandles)
                //ImpCheckMapModeChange();

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
            OSL_ENSURE(0 == rOverlayObject.mpOverlayManager, "OverlayObject is added twice to an OverlayManager (!)");

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

        void OverlayManager::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if(OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
            {
                if(getDrawinglayerOpt().IsAntiAliasing())
                {
                    // assume AA needs one pixel more and invalidate one pixel more
                    const double fDiscreteOne(getDiscreteOne());
                    const Rectangle aInvalidateRectangle(
                        (sal_Int32)floor(rRange.getMinX() - fDiscreteOne),
                        (sal_Int32)floor(rRange.getMinY() - fDiscreteOne),
                        (sal_Int32)ceil(rRange.getMaxX() + fDiscreteOne),
                        (sal_Int32)ceil(rRange.getMaxY() + fDiscreteOne));

                    // simply invalidate
                    ((Window&)getOutputDevice()).Invalidate(aInvalidateRectangle, INVALIDATE_NOERASE);
                }
                else
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

        oslInterlockedCount OverlayManager::acquire()
        {
            return osl_atomic_increment( &mnRefCount );
        }

        oslInterlockedCount OverlayManager::release()
        {
            oslInterlockedCount nCount( osl_atomic_decrement( &mnRefCount ) );
            if ( nCount == 0 )
                delete this;
            return nCount;
        }

    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
