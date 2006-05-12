/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygontubeprimitive3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-05-12 11:45:13 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
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
    namespace primitive
    {
        class polygonTubePrimitive3D : public polygonHairlinePrimitive3D
        {
        protected:
            double                                      mfRadius;
            double                                      mfDegreeStepWidth;
            double                                      mfMiterMinimumAngle;
            ::basegfx::tools::B2DLineJoin               maLineJoin;

            //  create decomposition
            virtual void decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation);

        public:
            polygonTubePrimitive3D(
                const ::basegfx::B3DPolygon& rPolygon,
                const ::basegfx::BColor& rBColor,
                double fRadius, ::basegfx::tools::B2DLineJoin aLineJoin,
                double fDegreeStepWidth = 10.0 * F_PI180,
                double fMiterMinimumAngle = 15.0 * F_PI180);
            virtual ~polygonTubePrimitive3D();

            // get data
            double getRadius() const { return mfRadius; }
            double getDegreeStepWidth() const { return mfDegreeStepWidth; }
            double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }
            ::basegfx::tools::B2DLineJoin getLineJoin() const { return maLineJoin; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // clone operator
            virtual basePrimitive* createNewClone() const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

// eof
