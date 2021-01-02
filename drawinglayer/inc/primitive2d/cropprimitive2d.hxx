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

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>


namespace drawinglayer::primitive2d
    {
        /** CropPrimitive2D class

            Caution: Due to old constraints (old core definitions) the
            crop distances describe how the uncropped content is defined
            relative to the current object size. This means that maTransformation
            describes the current object size (the part of the object visible
            with the crop applied). To get the original size and orientation
            of the uncropped content it is necessary to calc back from the
            current situation (maTransformation) using the crop values
            to get to the uncropped original content.

            Thus a transformation has to be calculated which will be applied
            to the already existing content to get it to the uncropped state
            and then this is masked with the current state (mask polygon
            created from unit polygon and maTransformation).

            At least in this primitive the units of the crop values are
            already in the local coordinate system; in the core these distances
            are defined relative to the object content size (PrefMapMode
            and PrefSize of the content)...

            Of course this is a primitive, so feel free to just ignore all that
            stuff and use the automatically generated decomposition. Sigh.
        */
        class CropPrimitive2D final : public GroupPrimitive2D
        {
        private:
            // the transformation already applied to the child geometry
            basegfx::B2DHomMatrix           maTransformation;

            // the crop offsets relative to the range of the unrotated content
            double                          mfCropLeft;
            double                          mfCropTop;
            double                          mfCropRight;
            double                          mfCropBottom;

        public:
            /// constructor
            CropPrimitive2D(
                const Primitive2DContainer& rChildren,
                const basegfx::B2DHomMatrix& rTransformation,
                double fCropLeft,
                double fCropTop,
                double fCropRight,
                double fCropBottom);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            double getCropLeft() const { return mfCropLeft; }
            double getCropTop() const { return mfCropTop; }
            double getCropRight() const { return mfCropRight; }
            double getCropBottom() const { return mfCropBottom; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// local decomposition
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
