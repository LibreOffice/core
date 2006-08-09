/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillattribute.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:47:33 $
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

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        fillGradientAttribute::fillGradientAttribute(GradientStyle eStyle, double fBorder, double fOffsetX, double fOffsetY, double fAngle,
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

        bool fillGradientAttribute::operator==(const fillGradientAttribute& rCandidate) const
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
        fillHatchAttribute::fillHatchAttribute(HatchStyle eStyle, double fDistance, double fAngle, const basegfx::BColor& rColor, bool bFillBackground)
        :   mfDistance(fDistance),
            mfAngle(fAngle),
            maColor(rColor),
            meStyle(eStyle),
            mbFillBackground(bFillBackground)
        {
        }

        bool fillHatchAttribute::operator==(const fillHatchAttribute& rCandidate) const
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
