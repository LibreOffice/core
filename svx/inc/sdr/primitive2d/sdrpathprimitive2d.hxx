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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/attribute/sdrlinefilleffectstextattribute.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


// predefines


namespace drawinglayer::primitive2d
    {
        class SdrPathPrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrLineFillEffectsTextAttribute   maSdrLFSTAttribute;
            basegfx::B2DPolyPolygon                     maUnitPolyPolygon;

            // OperationSmiley: Added to be able to define a FillGeometry different from local
            // geometry. It is ignored when empty and/or equal to UnitPolyPolygon.
            // If used and there is a fill, the object's geometry (maUnitPolyPolygon) will be filled,
            // but UnitDefinitionPolyPolygon will be used to define the FillStyle. Thus when
            // using the 'same' UnitDefinitionPolyPolygon for multiple definitions,
            // all filled stuff using it will fit seamlessly together.
            // 'same' is in quotes since it is a UnitPolygon, so being relative to the
            // unit polygon of the local geometry (UnitPolyPolygon). The definition is complete
            // when applying the also given transformation (maTransform)
            basegfx::B2DPolyPolygon                     maUnitDefinitionPolyPolygon;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            // OperationSmiley: Extended to UnitDefinitionPolyPolygon, but when needed
            // a 2nd version without can be defined that just does not set the
            // maUnitDefinitionPolyPolygon or set equal to UnitPolyPolygon
            SdrPathPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillEffectsTextAttribute& rSdrLFSTAttribute,
                const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
                const basegfx::B2DPolyPolygon& rUnitDefinitionPolyPolygon);

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillEffectsTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }
            const basegfx::B2DPolyPolygon& getUnitPolyPolygon() const { return maUnitPolyPolygon; }
            const basegfx::B2DPolyPolygon& getUnitDefinitionPolyPolygon() const { return maUnitDefinitionPolyPolygon; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
