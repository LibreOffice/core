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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/color/bcolor.hxx>
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
