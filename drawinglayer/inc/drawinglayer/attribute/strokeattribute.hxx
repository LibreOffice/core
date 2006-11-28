/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strokeattribute.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-28 11:03:55 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#include <basegfx/polygon/b2dlinegeometry.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class StrokeAttribute
        {
            basegfx::BColor                             maColor;                // color
            double                                      mfWidth;                // absolute line width
            basegfx::tools::B2DLineJoin                 meLineJoin;             // type of LineJoin
            ::std::vector< double >                     maDotDashArray;         // array of double which defines the dot-dash pattern
            double                                      mfFullDotDashLen;       // sum of maDotDashArray (for convenience)

        public:
            StrokeAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::tools::B2DLineJoin aB2DLineJoin,
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin),
                maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            StrokeAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::tools::B2DLineJoin aB2DLineJoin = basegfx::tools::B2DLINEJOIN_ROUND)
            :   maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin),
                maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            StrokeAttribute(const StrokeAttribute& rCandidate)
            {
                *this = rCandidate;
            }

            StrokeAttribute& operator=(const StrokeAttribute& rCandidate)
            {
                maColor = rCandidate.maColor;
                mfWidth = rCandidate.mfWidth;
                meLineJoin = rCandidate.meLineJoin;
                maDotDashArray = rCandidate.maDotDashArray;
                mfFullDotDashLen = rCandidate.mfFullDotDashLen;

                return *this;
            }

            ~StrokeAttribute()
            {
            }

            // compare operator
            bool operator==(const StrokeAttribute& rCandidate) const
            {
                return (maColor == rCandidate.maColor
                    && mfWidth == rCandidate.mfWidth
                    && meLineJoin == rCandidate.meLineJoin
                    && mfFullDotDashLen == rCandidate.mfFullDotDashLen
                    && maDotDashArray == rCandidate.maDotDashArray);
            }

            // data access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::tools::B2DLineJoin getLineJoin() const { return meLineJoin; }
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

// eof
