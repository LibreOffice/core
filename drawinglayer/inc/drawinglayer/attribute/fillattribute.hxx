/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillattribute.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:30:34 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

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
