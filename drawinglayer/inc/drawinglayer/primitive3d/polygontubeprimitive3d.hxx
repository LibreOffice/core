/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygontubeprimitive3d.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2007-11-07 14:27:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B3DPolyPolygon;
    class B3DHomMatrix;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class PolygonTubePrimitive3D : public PolygonHairlinePrimitive3D
        {
        private:
            double                                      mfRadius;
            double                                      mfDegreeStepWidth;
            double                                      mfMiterMinimumAngle;
            basegfx::B2DLineJoin                        maLineJoin;

        protected:
            // local decomposition.
            virtual Primitive3DSequence createLocalDecomposition(double fTime) const;

        public:
            PolygonTubePrimitive3D(
                const basegfx::B3DPolygon& rPolygon,
                const basegfx::BColor& rBColor,
                double fRadius, basegfx::B2DLineJoin aLineJoin,
                double fDegreeStepWidth = 10.0 * F_PI180,
                double fMiterMinimumAngle = 15.0 * F_PI180);

            // get data
            double getRadius() const { return mfRadius; }
            double getDegreeStepWidth() const { return mfDegreeStepWidth; }
            double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }
            basegfx::B2DLineJoin getLineJoin() const { return maLineJoin; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
