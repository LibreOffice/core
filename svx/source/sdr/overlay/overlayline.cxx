/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayline.cxx,v $
 * $Revision: 1.4 $
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
#include <svx/sdr/overlay/overlayline.hxx>
#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayLineStriped::drawGeometry(OutputDevice& rOutputDevice)
        {
            ImpDrawLineStriped(rOutputDevice, getBasePosition(), getSecondPosition());
        }

        void OverlayLineStriped::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            // reset range and expand it
            maBaseRange.reset();
            maBaseRange.expand(getBasePosition());
            maBaseRange.expand(getSecondPosition());
        }

        OverlayLineStriped::OverlayLineStriped(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            maSecondPosition(rSecondPos)
        {
        }

        OverlayLineStriped::~OverlayLineStriped()
        {
        }

        void OverlayLineStriped::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        sal_Bool OverlayLineStriped::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                return basegfx::tools::isInEpsilonRange(getBasePosition(), getSecondPosition(), rPos, fTol);
            }

            return sal_False;
        }

        void OverlayLineStriped::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            if(!rMatrix.isIdentity())
            {
                // transform base position
                OverlayObjectWithBasePosition::transform(rMatrix);

                // transform maSecondPosition
                const basegfx::B2DPoint aNewSecondPosition = rMatrix * getSecondPosition();
                setSecondPosition(aNewSecondPosition);
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayLine::drawGeometry(OutputDevice& rOutputDevice)
        {
            const Point aStart(FRound(getBasePosition().getX()), FRound(getBasePosition().getY()));
            const Point aEnd(FRound(getSecondPosition().getX()), FRound(getSecondPosition().getY()));

            rOutputDevice.SetLineColor(getBaseColor());
            rOutputDevice.SetFillColor();

            rOutputDevice.DrawLine(aStart, aEnd);
        }

        OverlayLine::OverlayLine(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            Color aLineColor)
        :   OverlayLineStriped(rBasePos, rSecondPos)
        {
            // set base color here, OverlayCrosshairStriped constructor has set
            // it to it's own default.
            maBaseColor = aLineColor;
        }

        OverlayLine::~OverlayLine()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
