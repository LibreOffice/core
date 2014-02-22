/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>



using namespace com::sun::star;



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

                
                drawinglayer::processor2d::BaseProcessor2D* pProcessor = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
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

        OverlayManager::OverlayManager(OutputDevice& rOutputDevice)
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
            
            
            static bool bUseReducedDisplayQualityForDrag(true);

            if(bUseReducedDisplayQualityForDrag)
            {
                uno::Sequence< beans::PropertyValue > xProperties(1);
                xProperties[0].Name = "ReducedDisplayQuality";
                xProperties[0].Value <<= true;
                maViewInformation2D = drawinglayer::geometry::ViewInformation2D(xProperties);
            }
        }

        rtl::Reference<OverlayManager> OverlayManager::create(OutputDevice& rOutputDevice)
        {
            return rtl::Reference<OverlayManager>(new OverlayManager(rOutputDevice));
        }

        const drawinglayer::geometry::ViewInformation2D OverlayManager::getCurrentViewInformation2D() const
        {
            if(getOutputDevice().GetViewTransformation() != maViewTransformation)
            {
                basegfx::B2DRange aViewRange(maViewInformation2D.getViewport());

                if(OUTDEV_WINDOW == getOutputDevice().GetOutDevType())
                {
                    const Size aOutputSizePixel(getOutputDevice().GetOutputSizePixel());

                    
                    
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
                    maViewInformation2D.getViewTime(),
                    maViewInformation2D.getExtendedInformationSequence());
                pThis->mfDiscreteOne = 0.0;
            }

            return maViewInformation2D;
        }

        void OverlayManager::impApplyRemoveActions(OverlayObject& rTarget)
        {
            
            if(rTarget.allowsAnimation())
            {
                
                RemoveEvent(&rTarget);
            }

            
            invalidateRange(rTarget.getBaseRange());

            
            rTarget.mpOverlayManager = 0;
        }

        void OverlayManager::impApplyAddActions(OverlayObject& rTarget)
        {
            
            rTarget.mpOverlayManager = this;

            
            invalidateRange(rTarget.getBaseRange());

            
            if(rTarget.allowsAnimation())
            {
                
                
                
                
                rTarget.Trigger(GetTime());
            }
        }

        OverlayManager::~OverlayManager()
        {
            
            
            
            const sal_uInt32 nSize(maOverlayObjects.size());

            if(nSize)
            {
                for(OverlayObjectVector::iterator aIter(maOverlayObjects.begin()); aIter != maOverlayObjects.end(); ++aIter)
                {
                    OSL_ENSURE(*aIter, "Corrupted OverlayObject List (!)");
                    OverlayObject& rCandidate = **aIter;
                    impApplyRemoveActions(rCandidate);
                }

                
                maOverlayObjects.clear();
            }
        }

        void OverlayManager::completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty() && maOverlayObjects.size())
            {
                
                
                

                
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
            
        }

        
        void OverlayManager::copyArea(const Point& /*rDestPt*/, const Point& /*rSrcPt*/, const Size& /*rSrcSize*/)
        {
            
        }

        void OverlayManager::restoreBackground(const Region& /*rRegion*/) const
        {
            
        }

        void OverlayManager::add(OverlayObject& rOverlayObject)
        {
            OSL_ENSURE(0 == rOverlayObject.mpOverlayManager, "OverlayObject is added twice to an OverlayManager (!)");

            
            maOverlayObjects.push_back(&rOverlayObject);

            
            impApplyAddActions(rOverlayObject);
        }

        void OverlayManager::remove(OverlayObject& rOverlayObject)
        {
            OSL_ENSURE(rOverlayObject.mpOverlayManager == this, "OverlayObject is removed from wrong OverlayManager (!)");

            
            impApplyRemoveActions(rOverlayObject);

            
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
                    
                    const double fDiscreteOne(getDiscreteOne());
                    const Rectangle aInvalidateRectangle(
                        (sal_Int32)floor(rRange.getMinX() - fDiscreteOne),
                        (sal_Int32)floor(rRange.getMinY() - fDiscreteOne),
                        (sal_Int32)ceil(rRange.getMaxX() + fDiscreteOne),
                        (sal_Int32)ceil(rRange.getMaxY() + fDiscreteOne));

                    
                    ((Window&)getOutputDevice()).Invalidate(aInvalidateRectangle, INVALIDATE_NOERASE);
                }
                else
                {
                    
                    
                    const Rectangle aInvalidateRectangle(
                        (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
                        (sal_Int32)ceil(rRange.getMaxX()), (sal_Int32)ceil(rRange.getMaxY()));

                    
                    ((Window&)getOutputDevice()).Invalidate(aInvalidateRectangle, INVALIDATE_NOERASE);
                }
            }
        }

        
        void OverlayManager::setStripeColorA(Color aNew)
        {
            if(aNew != maStripeColorA)
            {
                maStripeColorA = aNew;
                ImpStripeDefinitionChanged();
            }
        }

        
        void OverlayManager::setStripeColorB(Color aNew)
        {
            if(aNew != maStripeColorB)
            {
                maStripeColorB = aNew;
                ImpStripeDefinitionChanged();
            }
        }

        
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

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
