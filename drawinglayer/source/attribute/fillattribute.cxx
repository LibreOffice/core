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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/fillattribute.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        FillGradientAttribute::FillGradientAttribute(GradientStyle eStyle, double fBorder, double fOffsetX, double fOffsetY, double fAngle,
            const basegfx::BColor& rStartColor, const basegfx::BColor& rEndColor, sal_uInt16 nSteps)
        :   maStartColor(rStartColor),
            maEndColor(rEndColor),
            mfBorder(fBorder),
            mfOffsetX(fOffsetX),
            mfOffsetY(fOffsetY),
            mfAngle(fAngle),
            meStyle(eStyle),
            mnSteps(nSteps)
        {
        }

        bool FillGradientAttribute::operator==(const FillGradientAttribute& rCandidate) const
        {
            return (meStyle == rCandidate.meStyle
                && maStartColor == rCandidate.maStartColor
                && maEndColor == rCandidate.maEndColor
                && mfBorder == rCandidate.mfBorder
                && mfOffsetX == rCandidate.mfOffsetX
                && mfOffsetY == rCandidate.mfOffsetY
                && mfAngle == rCandidate.mfAngle
                && mnSteps == rCandidate.mnSteps);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        FillHatchAttribute::FillHatchAttribute(HatchStyle eStyle, double fDistance, double fAngle, const basegfx::BColor& rColor, bool bFillBackground)
        :   mfDistance(fDistance),
            mfAngle(fAngle),
            maColor(rColor),
            meStyle(eStyle),
            mbFillBackground(bFillBackground)
        {
        }

        bool FillHatchAttribute::operator==(const FillHatchAttribute& rCandidate) const
        {
            return (meStyle == rCandidate.meStyle
                && mfDistance == rCandidate.mfDistance
                && mfAngle == rCandidate.mfAngle
                && maColor == rCandidate.maColor
                && mbFillBackground  == rCandidate.mbFillBackground);
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
