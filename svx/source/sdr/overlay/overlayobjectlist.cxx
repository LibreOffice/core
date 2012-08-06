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

#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

// for SOLARIS compiler include of algorithm part of _STL is necesary to
// get access to basic algos like ::std::find
#include <algorithm>

#include <drawinglayer/processor2d/hittestprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////

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
                ::sdr::overlay::OverlayObject* pCandidate = *aStart;

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
                        const Size aSizeLogic(pManager->getOutputDevice().PixelToLogic(
                            Size(DEFAULT_VALUE_FOR_HITTEST_PIXEL, DEFAULT_VALUE_FOR_HITTEST_PIXEL)));
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
                    ::sdr::overlay::OverlayObject* pCandidate = *aStart;
                    aRetval.expand(pCandidate->getBaseRange());
                }
            }

            return aRetval;
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
