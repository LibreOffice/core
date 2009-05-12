/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaybitmap.cxx,v $
 * $Revision: 1.5 $
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

#include <svx/sdr/overlay/overlayhatchrect.hxx>
#include <vcl/hatch.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        basegfx::B2DPolyPolygon OverlayHatchRect::getGeometry(OutputDevice& rOutputDevice)
        {
            const basegfx::B2DPoint aDiscreteTopLeft(rOutputDevice.GetViewTransformation() * getBasePosition());
            const basegfx::B2DPoint aDiscreteBottomRight(rOutputDevice.GetViewTransformation() * getSecondPosition());

            basegfx::B2DRange aInnerRange(
                floor(aDiscreteTopLeft.getX()), floor(aDiscreteTopLeft.getY()),
                floor(aDiscreteBottomRight.getX()), floor(aDiscreteBottomRight.getY()));
            basegfx::B2DRange aOuterRange(aInnerRange);
            basegfx::B2DPolyPolygon aRetval;

            aOuterRange.grow(getDiscreteWidth() * 0.5);
            aInnerRange.grow(getDiscreteWidth() * -0.5);

            aRetval.append(basegfx::tools::createPolygonFromRect(aOuterRange));
            aRetval.append(basegfx::tools::createPolygonFromRect(aInnerRange));

            if(!basegfx::fTools::equalZero(mfRotation))
            {
                basegfx::B2DHomMatrix aTransform;

                aTransform.translate(-aOuterRange.getMinX(), -aOuterRange.getMinY());
                aTransform.rotate(getRotation());
                aTransform.translate(aOuterRange.getMinX(), aOuterRange.getMinY());

                aRetval.transform(aTransform);
            }

            return aRetval;
        }

        void OverlayHatchRect::drawGeometry(OutputDevice& rOutputDevice)
        {
            const basegfx::B2DPolyPolygon aB2DGeometry(getGeometry(rOutputDevice));
            const bool bMapModeWasEnabled(rOutputDevice.IsMapModeEnabled());

            // use VCL polygon and methodology for paint
            double fFullRotation(getHatchRotation() - getRotation());

            while(fFullRotation < 0.0)
            {
                fFullRotation += F_2PI;
            }

            while(fFullRotation >= F_2PI)
            {
                fFullRotation -= F_2PI;
            }

            const Hatch aHatch(HATCH_SINGLE, getBaseColor(), 3, (sal_uInt16)basegfx::fround(fFullRotation * ( 10.0 / F_PI180)));
            rOutputDevice.EnableMapMode(false);
            rOutputDevice.DrawHatch(PolyPolygon(aB2DGeometry), aHatch);
            rOutputDevice.EnableMapMode(bMapModeWasEnabled);
        }

        void OverlayHatchRect::createBaseRange(OutputDevice& rOutputDevice)
        {
            // reset range and expand with fresh geometry
            maBaseRange = getGeometry(rOutputDevice).getB2DRange();

            // getGeometry data is in discrete coordinates (pixels), so transform back to
            // world coordinates (logic)
            maBaseRange.transform(rOutputDevice.GetInverseViewTransformation());
        }

        OverlayHatchRect::OverlayHatchRect(
            const basegfx::B2DPoint& rBasePosition,
            const basegfx::B2DPoint& rSecondPosition,
            const Color& rHatchColor,
            double fDiscreteWidth,
            double fHatchRotation,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePosition, rHatchColor),
            maSecondPosition(rSecondPosition),
            mfDiscreteWidth(fDiscreteWidth),
            mfHatchRotation(fHatchRotation),
            mfRotation(fRotation)
        {
        }

        void OverlayHatchRect::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayHatchRect::zoomHasChanged()
        {
            // reset validity of range in logical coor to force recalculation
            mbIsChanged = sal_True;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
