/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linestartendattribute.hxx,v $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

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
