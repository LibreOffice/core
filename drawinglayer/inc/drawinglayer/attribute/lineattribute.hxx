/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineattribute.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2007-11-07 14:27:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX

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
        class LineAttribute
        {
        private:
            basegfx::BColor                             maColor;                // color
            double                                      mfWidth;                // absolute line width
            basegfx::B2DLineJoin                        meLineJoin;             // type of LineJoin

        public:
            LineAttribute(
                const basegfx::BColor& rColor,
                double fWidth = 0.0,
                basegfx::B2DLineJoin aB2DLineJoin = basegfx::B2DLINEJOIN_ROUND)
            :   maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin)
            {
            }

            LineAttribute(const LineAttribute& rCandidate)
            {
                *this = rCandidate;
            }

            LineAttribute& operator=(const LineAttribute& rCandidate)
            {
                maColor = rCandidate.maColor;
                mfWidth = rCandidate.mfWidth;
                meLineJoin = rCandidate.meLineJoin;
                return *this;
            }

            ~LineAttribute()
            {
            }

            // compare operator
            bool operator==(const LineAttribute& rCandidate) const
            {
                return (maColor == rCandidate.maColor
                    && mfWidth == rCandidate.mfWidth
                    && meLineJoin == rCandidate.meLineJoin);
            }

            // data access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX

// eof
