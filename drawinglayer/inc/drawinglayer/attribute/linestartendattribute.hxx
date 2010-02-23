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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class LineStartEndAttribute
        {
        private:
            double                                  mfWidth;                // absolute line StartEndGeometry base width
            basegfx::B2DPolyPolygon                 maPolyPolygon;          // the StartEndGeometry PolyPolygon

            // bitfield
            unsigned                                mbCentered : 1;         // use centered to ineStart/End point?

        public:
            LineStartEndAttribute(
                double fWidth,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                bool bCentered)
            :   mfWidth(fWidth),
                maPolyPolygon(rPolyPolygon),
                mbCentered(bCentered)
            {
            }

            // compare operator
            bool operator==(const LineStartEndAttribute& rCandidate) const
            {
                return (basegfx::fTools::equal(mfWidth, rCandidate.mfWidth)
                    && maPolyPolygon == rCandidate.maPolyPolygon
                    && mbCentered == rCandidate.mbCentered);
            }

            // data access
            double getWidth() const { return mfWidth; }
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            bool isCentered() const { return mbCentered; }
            bool isActive() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX

// eof
