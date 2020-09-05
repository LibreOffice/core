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
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>

#include <drawinglayer/processor2d/hittestprocessor2d.hxx>
#include <comphelper/lok.hxx>

#define DEFAULT_VALUE_FOR_HITTEST_PIXEL         (2)
#define DEFAULT_VALUE_FOR_HITTEST_TWIP          (30)

namespace sdr::overlay
{
        OverlayObjectList::~OverlayObjectList()
        {
            clear();
        }

        void OverlayObjectList::clear()
        {
            for(auto & pCandidate : maVector)
            {
                if(pCandidate->getOverlayManager())
                    pCandidate->getOverlayManager()->remove(*pCandidate);
            }
            maVector.clear();
        }

        void OverlayObjectList::append(std::unique_ptr<OverlayObject> pOverlayObject)
        {
            assert(pOverlayObject && "tried to add invalid OverlayObject to OverlayObjectList");
            maVector.push_back(std::move(pOverlayObject));
        }

        bool OverlayObjectList::isHitLogic(const basegfx::B2DPoint& rLogicPosition, double fLogicTolerance) const
        {
            if(!maVector.empty())
            {
                OverlayObject* pFirst = maVector.front().get();
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
                            if (pManager->getOutputDevice().GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                                aSizeLogic = OutputDevice::LogicToLogic(aSizeLogic, MapMode(MapUnit::MapTwip), MapMode(MapUnit::Map100thMM));
                        }

                        fLogicTolerance = aSizeLogic.Width();
                    }

                    const drawinglayer::geometry::ViewInformation2D& aViewInformation2D(pManager->getCurrentViewInformation2D());
                    drawinglayer::processor2d::HitTestProcessor2D aHitTestProcessor2D(
                        aViewInformation2D,
                        rLogicPosition,
                        fLogicTolerance,
                        false);

                    for(auto & pCandidate : maVector)
                    {
                        if(pCandidate->isHittable())
                        {
                            const drawinglayer::primitive2d::Primitive2DContainer& rSequence = pCandidate->getOverlayObjectPrimitive2DSequence();

                            if(!rSequence.empty())
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

        bool OverlayObjectList::isHitPixel(const Point& rDiscretePosition) const
        {
            constexpr sal_uInt32 nDiscreteTolerance = DEFAULT_VALUE_FOR_HITTEST_PIXEL;
            if(!maVector.empty())
            {
                OverlayObject* pCandidate = maVector.front().get();
                OverlayManager* pManager = pCandidate->getOverlayManager();

                if(pManager)
                {
                    const Point aPosLogic(pManager->getOutputDevice().PixelToLogic(rDiscretePosition));
                    const basegfx::B2DPoint aPosition(aPosLogic.X(), aPosLogic.Y());

                    const Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(Size(nDiscreteTolerance, nDiscreteTolerance)));
                    return isHitLogic(aPosition, static_cast<double>(aSizeLogic.Width()));
                }
            }

            return false;
        }

        basegfx::B2DRange OverlayObjectList::getBaseRange() const
        {
            basegfx::B2DRange aRetval;

            for(auto & pCandidate : maVector)
            {
                aRetval.expand(pCandidate->getBaseRange());
            }

            return aRetval;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
