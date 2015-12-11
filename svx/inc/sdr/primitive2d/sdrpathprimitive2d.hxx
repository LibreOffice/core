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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRPATHPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRPATHPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


// predefines



namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrPathPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrLineFillShadowTextAttribute   maSdrLFSTAttribute;
            basegfx::B2DPolyPolygon                     maUnitPolyPolygon;

        protected:
            // local decomposition.
            virtual Primitive2DContainer create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrPathPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
                const basegfx::B2DPolyPolygon& rUnitPolyPolygon);

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillShadowTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }
            const basegfx::B2DPolyPolygon& getUnitPolyPolygon() const { return maUnitPolyPolygon; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRPATHPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
