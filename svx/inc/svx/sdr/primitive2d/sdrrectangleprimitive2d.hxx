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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrRectanglePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrLineFillShadowTextAttribute   maSdrLFSTAttribute;
            double                                      mfCornerRadiusX;    // [0.0..1.0] relative to 1/2 width
            double                                      mfCornerRadiusY;    // [0.0..1.0] relative to 1/2 height

            // bitfield
            // flag which decides if the HitArea should be the filled geometry
            bool                                        mbForceFillForHitTest : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrRectanglePrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
                double fCornerRadiusX,
                double fCornerRadiusY,
                bool bForceFillForHitTest);

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillShadowTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }
            double getCornerRadiusX() const { return mfCornerRadiusX; }
            double getCornerRadiusY() const { return mfCornerRadiusY; }
            bool isCornerRadiusUsed() const { return (0.0 != mfCornerRadiusX || 0.0 != mfCornerRadiusY); }
            bool getForceFillForHitTest() const { return mbForceFillForHitTest; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
