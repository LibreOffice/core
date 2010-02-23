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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

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
        class StrokeAttribute
        {
        private:
            ::std::vector< double >                     maDotDashArray;         // array of double which defines the dot-dash pattern
            double                                      mfFullDotDashLen;       // sum of maDotDashArray (for convenience)

        public:
            StrokeAttribute(
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen = 0.0)
            :   maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            StrokeAttribute()
            :   maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            StrokeAttribute(const StrokeAttribute& rCandidate)
            {
                *this = rCandidate;
            }

            StrokeAttribute& operator=(const StrokeAttribute& rCandidate)
            {
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
                return (mfFullDotDashLen == rCandidate.mfFullDotDashLen
                    && maDotDashArray == rCandidate.maDotDashArray);
            }

            // data access
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX

// eof
