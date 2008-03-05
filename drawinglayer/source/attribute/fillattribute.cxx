/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillattribute.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:41 $
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
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
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
