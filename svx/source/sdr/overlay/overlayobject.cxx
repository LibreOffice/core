/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayobject.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:36:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

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
                basegfx::B2DPolygon aPolygon;
                aPolygon.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMinY()));
                aPolygon.append(basegfx::B2DPoint(rRange.getMaxX(), rRange.getMinY()));
                aPolygon.append(basegfx::B2DPoint(rRange.getMaxX(), rRange.getMaxY()));
                aPolygon.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()));
                aPolygon.setClosed(true);

                ImpDrawPolygonStriped(rOutputDevice, aPolygon);
            }
        }

        void OverlayObject::ImpDrawLineStriped(OutputDevice& rOutputDevice, double x1, double y1, double x2, double y2)
        {
            if(getOverlayManager())
            {
                const basegfx::B2DPoint aStart(x1, y1);
                const basegfx::B2DPoint aEnd(x2, y2);
                basegfx::B2DPolygon aPolygon;
                aPolygon.append(aStart);
                aPolygon.append(aEnd);

                ImpDrawPolygonStriped(rOutputDevice, aPolygon);
            }
        }

        void OverlayObject::ImpDrawLineStriped(OutputDevice& rOutputDevice, const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd)
        {
            if(getOverlayManager())
            {
                basegfx::B2DPolygon aPolygon;
                aPolygon.append(rStart);
                aPolygon.append(rEnd);

                ImpDrawPolygonStriped(rOutputDevice, aPolygon);
            }
        }

        void OverlayObject::ImpDrawPolygonStriped(OutputDevice& rOutputDevice, const basegfx::B2DPolygon& rPolygon)
        {
            if(getOverlayManager())
            {
                const sal_uInt32 nLenPixel(getOverlayManager()->getStripeLengthPixel());
                const Size aDashSizePixel(nLenPixel, nLenPixel);
                const Size aDashSizeLogic(rOutputDevice.PixelToLogic(aDashSizePixel));
                const double fDashLength(aDashSizeLogic.Width());
                const double fFullDotDashLength(fDashLength + fDashLength);

                // fill DashDot vector A
                ::std::vector<double> aDashDotArrayA;
                aDashDotArrayA.push_back(fDashLength);
                aDashDotArrayA.push_back(fDashLength);

                // fill DashDot vector B
                ::std::vector<double> aDashDotArrayB;
                aDashDotArrayB.push_back(0.0);
                aDashDotArrayB.push_back(fDashLength);
                aDashDotArrayB.push_back(fDashLength);

                // get dash polygons
                basegfx::B2DPolyPolygon aStripesA = basegfx::tools::applyLineDashing(rPolygon, aDashDotArrayA, fFullDotDashLength);
                aStripesA = basegfx::tools::mergeDashedLines(aStripesA);
                basegfx::B2DPolyPolygon aStripesB = basegfx::tools::applyLineDashing(rPolygon, aDashDotArrayB, fFullDotDashLength);
                aStripesB = basegfx::tools::mergeDashedLines(aStripesB);

                // draw stripes A
                if(aStripesA.count())
                {
                    rOutputDevice.SetFillColor();
                    rOutputDevice.SetLineColor(getOverlayManager()->getStripeColorA());
                    ImpDrawStripes(rOutputDevice, aStripesA);
                }

                // draw stripes B
                if(aStripesB.count())
                {
                    rOutputDevice.SetFillColor();
                    rOutputDevice.SetLineColor(getOverlayManager()->getStripeColorB());
                    ImpDrawStripes(rOutputDevice, aStripesB);
                }
            }
        }

        void OverlayObject::ImpDrawStripes(OutputDevice& rOutputDevice, const basegfx::B2DPolyPolygon& rPolyPolygon)
        {
            for(sal_uInt32 a(0L); a < rPolyPolygon.count();a ++)
            {
                // #i82889# Do not just paint from start point to end point
                // assuming that each partial Polygon contains a single line. Instead,
                // paint the whole polygon
                rOutputDevice.DrawPolyLine(Polygon(rPolyPolygon.getB2DPolygon(a)));
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
