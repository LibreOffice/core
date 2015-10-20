/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>


// predefines

namespace basegfx {
    class B3DPolyPolygon;
    class B3DHomMatrix;
}



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
            css::drawing::LineCap                       maLineCap;

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
                double fRadius,
                basegfx::B2DLineJoin aLineJoin,
                css::drawing::LineCap aLineCap,
                double fDegreeStepWidth = 10.0 * F_PI180,
                double fMiterMinimumAngle = 15.0 * F_PI180);

            /// data read access
            double getRadius() const { return mfRadius; }
            double getDegreeStepWidth() const { return mfDegreeStepWidth; }
            double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }
            basegfx::B2DLineJoin getLineJoin() const { return maLineJoin; }
            css::drawing::LineCap getLineCap() const { return maLineCap; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /** local decomposition. Use own buffering since we are not derived from
                BufferedDecompositionPrimitive3D
             */
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
