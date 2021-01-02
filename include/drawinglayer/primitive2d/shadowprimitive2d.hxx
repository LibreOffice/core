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

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>


namespace drawinglayer::primitive2d
{
        /** ShadowPrimitive2D class

            This primitive defines a generic shadow geometry construction
            for 2D objects. It decomposes to a TransformPrimitive2D embedded
            into a ModifiedColorPrimitive2D.

            It's for primitive usage convenience, so that not everyone has
            to implement the generic shadow construction by himself.

            The same geometry as sequence of primitives is used as geometry and
            as shadow. Since these are RefCounted Uno-Api objects, no extra objects
            are needed for the shadow itself; all the local decompositions of the
            original geometry can be reused from the renderer for shadow visualisation.
        */
        class DRAWINGLAYER_DLLPUBLIC ShadowPrimitive2D final : public GroupPrimitive2D
        {
        private:
            /// the shadow transformation, normally just an offset
            basegfx::B2DHomMatrix                   maShadowTransform;

            /// the shadow color to which all geometry is to be forced
            basegfx::BColor                         maShadowColor;

            /// the blur radius of the shadow
            double mfShadowBlur;


    public:
            /// constructor
            ShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                double fShadowBlur,
                const Primitive2DContainer& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }
            double getShadowBlur() const { return mfShadowBlur; }
            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            ///  create decomposition
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
