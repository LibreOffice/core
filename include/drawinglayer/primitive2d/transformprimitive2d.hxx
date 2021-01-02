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


namespace drawinglayer::primitive2d
{
        /** TransformPrimitive2D class

            This is one of the basic grouping primitives and it provides
            embedding a sequence of primitives (a geometry) into a
            transformation. All renderers have to handle this, usually by
            building a current transformation stack (linear combination)
            and applying this to all to-be-rendered geometry. If not handling
            this, the output will be mostly wrong since this primitive is
            widely used.

            It does transform by embedding an existing geometry into a
            transformation as Child-content. This allows re-usage of the
            refcounted Uno-Api primitives and their existing, buffered
            decompositions.

            It could e.g. be used to show a single object geometry in 1000
            different, transformed states without the need to create those
            thousand primitive contents.
         */
        class DRAWINGLAYER_DLLPUBLIC TransformPrimitive2D final : public GroupPrimitive2D
        {
        private:
            // the transformation to apply to the child geometry
            basegfx::B2DHomMatrix                   maTransformation;

        public:
            /// constructor
            TransformPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                const Primitive2DContainer& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
