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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDROLE2PRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDROLE2PRIMITIVE2D_HXX

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
        class SdrOle2Primitive2D : public BasePrimitive2D
        {
        private:
            Primitive2DSequence                         maOLEContent;
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrLineFillShadowTextAttribute   maSdrLFSTAttribute;

        public:
            SdrOle2Primitive2D(
                const Primitive2DSequence& rOLEContent,
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute);

            // data access
            const Primitive2DSequence& getOLEContent() const { return maOLEContent; }
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillShadowTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // local decomposition.
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDROLE2PRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
