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

#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/gen.hxx>

// for SOLARIS compiler include of algorithm part of _STL is necessary to
// get access to basic algos like ::std::find
#include <algorithm>

#include <drawinglayer/processor2d/hittestprocessor2d.hxx>
#include <comphelper/lok.hxx>

namespace sdr
{
    namespace overlay
    {
        OverlayObjectList::~OverlayObjectList()
        {
            clear();
        }

        void OverlayObjectList::clear()
        {
            OverlayObjectVector::iterator aStart(maVector.begin());

            for(; aStart != maVector.end(); ++aStart)
            {
                sdr::overlay::OverlayObject* pCandidate = *aStart;

                if(pCandidate->getOverlayManager())
                {
                    pCandidate->getOverlayManager()->remove(*pCandidate);
                }

                delete pCandidate;
            }

            maVector.clear();
        }

        bool OverlayObjectList::isHitLogic(const basegfx::B2DPoint& rLogicPosition, double fLogicTolerance) const
        {
            if(!maVector.empty())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());
                sdr::overlay::OverlayObject* pFirst = *aStart;
                OSL_ENSURE(pFirst, "Corrupt OverlayObjectList (!)");
                OverlayManager* pManager = pFirst->getOverlayManager();

                if(pManager)
                {
                    if(0.0 == fLogicTolerance)
                    {
                        Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(
                            Size(DEFAULT_VALUE_FOR_HITTEST_PIXEL, DEFAULT_VALUE_FOR_HITTEST_PIXEL)));

                        // When tiled rendering, we always work in logic units, use the non-pixel default.
                        if (comphelper::LibreOfficeKit::isActive())
                        {
                            aSizeLogic = Size(DEFAULT_VALUE_FOR_HITTEST_TWIP, DEFAULT_VALUE_FOR_HITTEST_TWIP);
                            if (pManager->getOutputDevice().GetMapMode().GetMapUnit() == MAP_100TH_MM)
                                aSizeLogic = OutputDevice::LogicToLogic(aSizeLogic, MAP_TWIP, MAP_100TH_MM);
                        }

                        fLogicTolerance = aSizeLogic.Width();
                    }

                    const drawinglayer::geometry::ViewInformation2D aViewInformation2D(pManager->getCurrentViewInformation2D());
                    drawinglayer::processor2d::HitTestProcessor2D aHitTestProcessor2D(
                        aViewInformation2D,
                        rLogicPosition,
                        fLogicTolerance,
                        false);

                    for(; aStart != maVector.end(); ++aStart)
                    {
                        sdr::overlay::OverlayObject* pCandidate = *aStart;
                        OSL_ENSURE(pCandidate, "Corrupt OverlayObjectList (!)");

                        if(pCandidate->isHittable())
                        {
                            const drawinglayer::primitive2d::Primitive2DSequence& rSequence = pCandidate->getOverlayObjectPrimitive2DSequence();

                            if(rSequence.hasElements())
                            {
                                aHitTestProcessor2D.process(rSequence);

                                if(aHitTestProcessor2D.getHit())
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }

        bool OverlayObjectList::isHitPixel(const Point& rDiscretePosition, sal_uInt32 nDiscreteTolerance) const
        {
            if(!maVector.empty())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());
                sdr::overlay::OverlayObject* pCandidate = *aStart;
                OverlayManager* pManager = pCandidate->getOverlayManager();

                if(pManager)
                {
                    const Point aPosLogic(pManager->getOutputDevice().PixelToLogic(rDiscretePosition));
                    const basegfx::B2DPoint aPosition(aPosLogic.X(), aPosLogic.Y());

                    if(nDiscreteTolerance)
                    {
                        const Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(Size(nDiscreteTolerance, nDiscreteTolerance)));
                        return isHitLogic(aPosition, (double)aSizeLogic.Width());
                    }
                    else
                    {
                        return isHitLogic(aPosition);
                    }
                }
            }

            return false;
        }

        basegfx::B2DRange OverlayObjectList::getBaseRange() const
        {
            basegfx::B2DRange aRetval;

            if(!maVector.empty())
            {
                OverlayObjectVector::const_iterator aStart(maVector.begin());

                for(; aStart != maVector.end(); ++aStart)
                {
                    sdr::overlay::OverlayObject* pCandidate = *aStart;
                    aRetval.expand(pCandidate->getBaseRange());
                }
            }

            return aRetval;
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
