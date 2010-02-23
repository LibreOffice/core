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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX

#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class XHatch;

namespace drawinglayer
{
    namespace attribute
    {
        enum GradientStyle
        {
            GRADIENTSTYLE_LINEAR,
            GRADIENTSTYLE_AXIAL,
            GRADIENTSTYLE_RADIAL,
            GRADIENTSTYLE_ELLIPTICAL,
            GRADIENTSTYLE_SQUARE,
            GRADIENTSTYLE_RECT
        };

        enum HatchStyle
        {
            HATCHSTYLE_SINGLE,
            HATCHSTYLE_DOUBLE,
            HATCHSTYLE_TRIPLE
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class FillGradientAttribute
        {
            basegfx::BColor                             maStartColor;
            basegfx::BColor                             maEndColor;
            double                                      mfBorder;
            double                                      mfOffsetX;
            double                                      mfOffsetY;
            double                                      mfAngle;
            GradientStyle                               meStyle;
            sal_uInt16                                  mnSteps;

        public:
            FillGradientAttribute(
                GradientStyle eStyle, double fBorder, double fOffsetX, double fOffsetY, double fAngle,
                const basegfx::BColor& rStartColor, const basegfx::BColor& rEndColor, sal_uInt16 nSteps);
            bool operator==(const FillGradientAttribute& rCandidate) const;

            // data access
            const basegfx::BColor& getStartColor() const { return maStartColor; }
            const basegfx::BColor& getEndColor() const { return maEndColor; }
            double getBorder() const { return mfBorder; }
            double getOffsetX() const { return mfOffsetX; }
            double getOffsetY() const { return mfOffsetY; }
            double getAngle() const { return mfAngle; }
            GradientStyle getStyle() const { return meStyle; }
            sal_uInt16 getSteps() const { return mnSteps; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class FillHatchAttribute
        {
            double                                      mfDistance;
            double                                      mfAngle;
            basegfx::BColor                             maColor;
            HatchStyle                                  meStyle;

            // bitfield
            unsigned                                    mbFillBackground : 1;

        public:
            FillHatchAttribute(HatchStyle eStyle, double fDistance, double fAngle, const basegfx::BColor& rColor, bool bFillBackground);
            bool operator==(const FillHatchAttribute& rCandidate) const;

            // data access
            double getDistance() const { return mfDistance; }
            double getAngle() const { return mfAngle; }
            const basegfx::BColor& getColor() const { return maColor; }
            HatchStyle getStyle() const { return meStyle; }
            bool isFillBackground() const { return mbFillBackground; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX

// eof
