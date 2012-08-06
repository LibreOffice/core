/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

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
        /** PolygonStrokePrimitive3D class

            This 3D primitive extends a 3D hairline to a 3D tube which is
            e.g. used for fat lines in 3D. It's decomposition will create all
            3D objects needed for the line tubes and the edge roundings
            in full 3D.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonTubePrimitive3D : public PolygonHairlinePrimitive3D
        {
        private:
            /// hold the last decompositon since it's expensive
            Primitive3DSequence                         maLast3DDecomposition;

            /// visualisation parameters
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

            /// local decomposition.
            Primitive3DSequence impCreate3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

        public:
            /// constructor
            PolygonTubePrimitive3D(
                const basegfx::B3DPolygon& rPolygon,
                const basegfx::BColor& rBColor,
                double fRadius, basegfx::B2DLineJoin aLineJoin,
                double fDegreeStepWidth = 10.0 * F_PI180,
                double fMiterMinimumAngle = 15.0 * F_PI180);

            /// data read access
            double getRadius() const { return mfRadius; }
            double getDegreeStepWidth() const { return mfDegreeStepWidth; }
            double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }
            basegfx::B2DLineJoin getLineJoin() const { return maLineJoin; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /** local decomposition. Use own buffering since we are not derived from
                BufferedDecompositionPrimitive3D
             */
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
