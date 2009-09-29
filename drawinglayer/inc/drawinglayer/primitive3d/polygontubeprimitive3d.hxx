/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygontubeprimitive3d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:21 $
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

#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>

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
            // hold the last decompositon since it's expensive
            Primitive3DSequence                         maLast3DDecomposition;

            // visualisation parameters
            double                                      mfRadius;
            double                                      mfDegreeStepWidth;
            double                                      mfMiterMinimumAngle;
            basegfx::B2DLineJoin                        maLineJoin;

        protected:
            /** access methods to maLast3DDecomposition. The usage of this methods may allow
                later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
                implementations for buffering the last decomposition.
             */
            const Primitive3DSequence& getLast3DDecomposition() const { return maLast3DDecomposition; }
            void setLast3DDecomposition(const Primitive3DSequence& rNew) { maLast3DDecomposition = rNew; }

            // local decomposition.
            Primitive3DSequence impCreate3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

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

            /** local decomposition. Use own buffering since we are not derived from
                BufferedDecompositionPrimitive3D
             */
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
