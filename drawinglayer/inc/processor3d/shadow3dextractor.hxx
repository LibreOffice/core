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

#pragma once

#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace basegfx { class B3DPolyPolygon; }
namespace basegfx { class B3DPolygon; }


namespace drawinglayer::processor3d
    {
        /** Shadow3DExtractingProcessor class

            This processor extracts the 2D shadow geometry (projected geometry) of all fed primitives.
            It is used to create the shadow of 3D objects which consists of 2D geometry. It needs quite
            some data to do so since we do not only offer flat projected 2D shadow, but also projections
            dependent on the light source
         */
        class Shadow3DExtractingProcessor final : public BaseProcessor3D
        {
        private:
            /// result holding vector (2D) and target vector for stacking (inited to &maPrimitive2DSequence)
            primitive2d::Primitive2DContainer                  maPrimitive2DSequence;
            primitive2d::Primitive2DContainer*                 mpPrimitive2DSequence;

            /// object transformation for scene for 2d definition
            basegfx::B2DHomMatrix                           maObjectTransformation;

            /// prepared data (transformations) for 2D/3D shadow calculations
            basegfx::B3DHomMatrix                           maWorldToEye;
            basegfx::B3DHomMatrix                           maEyeToView;
            basegfx::B3DVector                              maLightNormal;
            basegfx::B3DVector                              maShadowPlaneNormal;
            basegfx::B3DPoint                               maPlanePoint;
            double                                          mfLightPlaneScalar;

            /// flag if shadow plane projection preparation led to valid results
            bool                                            mbShadowProjectionIsValid : 1;

            /// flag if conversion is switched on
            bool                                            mbConvert : 1;

            /// flag if conversion shall use projection
            bool                                            mbUseProjection : 1;

            /// local helpers
            basegfx::B2DPolygon impDoShadowProjection(const basegfx::B3DPolygon& rSource);
            basegfx::B2DPolyPolygon impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource);

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
             */
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate) override;

        public:
            Shadow3DExtractingProcessor(
                const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const basegfx::B3DVector& rLightNormal,
                double fShadowSlant,
                const basegfx::B3DRange& rContained3DRange);
            virtual ~Shadow3DExtractingProcessor() override;

            /// data read access
            const primitive2d::Primitive2DContainer& getPrimitive2DSequence() const;
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const basegfx::B3DHomMatrix& getWorldToEye() const { return maWorldToEye; }
        };

} // end of namespace drawinglayer::processor3d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
