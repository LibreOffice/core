/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayobject.cxx,v $
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
#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <tools/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <vcl/outdev.hxx>
#include <vcl/salbtype.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayObject::objectChange()
        {
            if(mpOverlayManager)
            {
                basegfx::B2DRange aPreviousRange(maBaseRange);

                if(!aPreviousRange.isEmpty())
                {
                    mpOverlayManager->invalidateRange(aPreviousRange);
                }

                mbIsChanged = sal_True;
                const basegfx::B2DRange& rCurrentRange = getBaseRange();

                if(rCurrentRange != aPreviousRange && !rCurrentRange.isEmpty())
                {
                    mpOverlayManager->invalidateRange(rCurrentRange);
                }
            }
        }

        // support method to draw striped geometries
        void OverlayObject::ImpDrawRangeStriped(OutputDevice& rOutputDevice, const basegfx::B2DRange& rRange)
        {
            if(getOverlayManager())
            {
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(rRange));

                if(aPolygon.count())
                {
                    ImpDrawPolygonStriped(rOutputDevice, aPolygon);
                }
            }
        }

        void OverlayObject::ImpDrawLineStriped(OutputDevice& rOutputDevice, double x1, double y1, double x2, double y2)
        {
            if(getOverlayManager())
            {
                const basegfx::B2DPoint aStart(x1, y1);
                const basegfx::B2DPoint aEnd(x2, y2);

                if(!aStart.equal(aEnd))
                {
                    basegfx::B2DPolygon aPolygon;
                    aPolygon.append(aStart);
                    aPolygon.append(aEnd);

                    ImpDrawPolygonStriped(rOutputDevice, aPolygon);
                }
            }
        }

        void OverlayObject::ImpDrawLineStriped(OutputDevice& rOutputDevice, const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd)
        {
            if(getOverlayManager() && !rStart.equal(rEnd))
            {
                basegfx::B2DPolygon aPolygon;
                aPolygon.append(rStart);
                aPolygon.append(rEnd);

                ImpDrawPolygonStriped(rOutputDevice, aPolygon);
            }
        }

        void OverlayObject::ImpDrawPolygonStriped(OutputDevice& rOutputDevice, const basegfx::B2DPolygon& rPolygon)
        {
            if(getOverlayManager() && rPolygon.count())
            {
                if(getOverlayManager() && getOverlayManager()->getDrawinglayerOpt().IsAntiAliasing())
                {
                    // prepare ViewInformation2D
                    const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                        basegfx::B2DHomMatrix(),
                        rOutputDevice.GetViewTransformation(),
                        basegfx::B2DRange(),
                        0,
                        0.0,
                        0);

                    // create processor
                    drawinglayer::processor2d::BaseProcessor2D* pProcessor = ::sdr::contact::createBaseProcessor2DFromOutputDevice(
                        rOutputDevice,
                        aViewInformation2D);

                    if(pProcessor)
                    {
                        // prepare primitives
                        const drawinglayer::primitive2d::Primitive2DReference aPolygonMarkerPrimitive2D(
                            new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                                rPolygon,
                                getOverlayManager()->getStripeColorA().getBColor(),
                                getOverlayManager()->getStripeColorB().getBColor(),
                                getOverlayManager()->getStripeLengthPixel()));
                        const drawinglayer::primitive2d::Primitive2DSequence aSequence(&aPolygonMarkerPrimitive2D, 1);

                        pProcessor->process(aSequence);

                        delete pProcessor;
                    }
                }
                else
                {
                    const sal_uInt32 nLenPixel(getOverlayManager()->getStripeLengthPixel());
                    const Size aDashSizePixel(nLenPixel, nLenPixel);
                    const Size aDashSizeLogic(rOutputDevice.PixelToLogic(aDashSizePixel));
                    const double fDashLength(aDashSizeLogic.Width());
                    const double fFullDotDashLength(fDashLength + fDashLength);

                    // fill DashDot vector
                    ::std::vector<double> aDotDashArray;
                    aDotDashArray.push_back(fDashLength);
                    aDotDashArray.push_back(fDashLength);

                    // get dash polygons
                    basegfx::B2DPolyPolygon aStripesA;
                    basegfx::B2DPolyPolygon aStripesB;
                    basegfx::tools::applyLineDashing(rPolygon, aDotDashArray, &aStripesA, &aStripesB, fFullDotDashLength);

                    // draw stripes A
                    if(aStripesA.count())
                    {
                        rOutputDevice.SetFillColor();
                        rOutputDevice.SetLineColor(getOverlayManager()->getStripeColorA());

                        for(sal_uInt32 a(0L); a < aStripesA.count();a ++)
                        {
                            rOutputDevice.DrawPolyLine(aStripesA.getB2DPolygon(a));
                        }
                    }

                    // draw stripes B
                    if(aStripesB.count())
                    {
                        rOutputDevice.SetFillColor();
                        rOutputDevice.SetLineColor(getOverlayManager()->getStripeColorB());

                        for(sal_uInt32 a(0L); a < aStripesB.count();a ++)
                        {
                            rOutputDevice.DrawPolyLine(aStripesB.getB2DPolygon(a));
                        }
                    }
                }
            }
        }

        OverlayObject::OverlayObject(Color aBaseColor)
        :   Event(0L),
            mpOverlayManager(0L),
            mpNext(0L),
            mpPrevious(0L),
            maBaseColor(aBaseColor),
            mbIsVisible(sal_True),
            mbIsChanged(sal_True),
            mbIsHittable(sal_True),
            mbAllowsAnimation(sal_False)
        {
        }

        OverlayObject::~OverlayObject()
        {
            DBG_ASSERT(0L == mpOverlayManager,
                "OverlayObject is destructed which is still registered at OverlayManager (!)");
        }

        sal_Bool OverlayObject::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                if(0.0 != fTol)
                {
                    basegfx::B2DRange aRange(getBaseRange());
                    aRange.grow(fTol);
                    return aRange.isInside(rPos);
                }
                else
                {
                    return getBaseRange().isInside(rPos);
                }
            }

            return sal_False;
        }

        const basegfx::B2DRange& OverlayObject::getBaseRange() const
        {
            if(mbIsChanged)
            {
                if(mpOverlayManager)
                {
                    ((::sdr::overlay::OverlayObject*)this)->createBaseRange(mpOverlayManager->getOutputDevice());
                }

                ((::sdr::overlay::OverlayObject*)this)->mbIsChanged = sal_False;
            }

            return maBaseRange;
        }

        void OverlayObject::setVisible(sal_Bool bNew)
        {
            if(bNew != mbIsVisible)
            {
                // remember new value
                mbIsVisible = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::setHittable(sal_Bool bNew)
        {
            if(bNew != mbIsHittable)
            {
                // remember new value
                mbIsHittable = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::setBaseColor(Color aNew)
        {
            if(aNew != maBaseColor)
            {
                // remember new value
                maBaseColor = aNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::Trigger(sal_uInt32 /*nTime*/)
        {
            // default does not register again
        }

        void OverlayObject::zoomHasChanged()
        {
            // default does not need to do anything
        }

        void OverlayObject::stripeDefinitionHasChanged()
        {
            // default does not need to do anything
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        OverlayObjectWithBasePosition::OverlayObjectWithBasePosition(const basegfx::B2DPoint& rBasePos, Color aBaseColor)
        :   OverlayObject(aBaseColor),
            maBasePosition(rBasePos)
        {
        }

        OverlayObjectWithBasePosition::~OverlayObjectWithBasePosition()
        {
        }

        void OverlayObjectWithBasePosition::setBasePosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maBasePosition)
            {
                // remember new value
                maBasePosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObjectWithBasePosition::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                basegfx::B2DPoint aNewBasePosition = rMatrix * getBasePosition();
                setBasePosition(aNewBasePosition);
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
