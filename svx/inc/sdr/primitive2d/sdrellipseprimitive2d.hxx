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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRELLIPSEPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRELLIPSEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/attribute/sdrlinefilleffectstextattribute.hxx>


// predefines


namespace drawinglayer::primitive2d
    {
        class SdrEllipsePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            ::basegfx::B2DHomMatrix                     maTransform;
            attribute::SdrLineFillEffectsTextAttribute   maSdrLFSTAttribute;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrEllipsePrimitive2D(
                const ::basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillEffectsTextAttribute& rSdrLFSTAttribute);

            // data access
            const ::basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillEffectsTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
    {
        class SdrEllipseSegmentPrimitive2D final : public SdrEllipsePrimitive2D
        {
        private:
            double                                      mfStartAngle;
            double                                      mfEndAngle;

            bool                                        mbCloseSegment : 1;
            bool                                        mbCloseUsingCenter : 1;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrEllipseSegmentPrimitive2D(
                const ::basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillEffectsTextAttribute& rSdrLFSTAttribute,
                double fStartAngle,
                double fEndAngle,
                bool bCloseSegment,
                bool bCloseUsingCenter);

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRELLIPSEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
