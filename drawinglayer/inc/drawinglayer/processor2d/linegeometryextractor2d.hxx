/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linegeometryextractor2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:44:27 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class LineGeometryExtractor2D : public BaseProcessor2D
        {
        private:
            std::vector< basegfx::B2DPolygon >      maExtractedHairlines;
            std::vector< basegfx::B2DPolyPolygon >  maExtractedLineFills;

            // bitfield
            unsigned                                mbInLineGeometry : 1;

            // tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            LineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~LineGeometryExtractor2D();

            const std::vector< basegfx::B2DPolygon >& getExtractedHairlines() const { return maExtractedHairlines; }
            const std::vector< basegfx::B2DPolyPolygon >& getExtractedLineFills() const { return maExtractedLineFills; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

// eof
