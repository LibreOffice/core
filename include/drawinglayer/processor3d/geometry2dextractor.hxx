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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/color/bcolormodifier.hxx>



namespace drawinglayer
{
    namespace processor3d
    {
        /** Geometry2DExtractingProcessor class

            This processor extracts the 2D geometry (projected geometry) of all feeded primitives.
            It is e.g. used as sub-processor for contour extraction where 3D geometry is only
            useful as 2D projected geometry.
         */
        class DRAWINGLAYER_DLLPUBLIC Geometry2DExtractingProcessor : public BaseProcessor3D
        {
        private:
            /// result holding vector (2D)
            primitive2d::Primitive2DSequence                maPrimitive2DSequence;

            /// object transformation for scene for 2d definition
            basegfx::B2DHomMatrix                           maObjectTransformation;

            /// the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                    maBColorModifierStack;

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
             */
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate) override;

        public:
            Geometry2DExtractingProcessor(
                const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B2DHomMatrix& rObjectTransformation);

            // data read access
            const primitive2d::Primitive2DSequence& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
