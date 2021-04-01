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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRFRAMEBORDERPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRFRAMEBORDERPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/framelink.hxx>

#include <memory>

// predefines

namespace drawinglayer::primitive2d
    {
        class SVXCORE_DLLPUBLIC SdrFrameBorderData
        {
        private:
            basegfx::B2DPoint   maOrigin;       /// start point of borderline
            basegfx::B2DVector  maX;            /// X-Axis of borderline with length
            svx::frame::Style   maStyle;        /// Style of borderline
            Color               maColor;
            bool                mbForceColor;

            class SdrConnectStyleData
            {
            private:
                svx::frame::Style   maStyle;
                basegfx::B2DVector  maNormalizedPerpendicular;
                bool                mbStyleMirrored;

            public:
                SdrConnectStyleData(
                    const svx::frame::Style& rStyle,
                    const basegfx::B2DVector& rNormalizedPerpendicular,
                    bool bStyleMirrored);

                const svx::frame::Style& getStyle() const { return maStyle; }
                const basegfx::B2DVector& getNormalizedPerpendicular() const { return maNormalizedPerpendicular; }
                bool getStyleMirrored() const { return  mbStyleMirrored; }
                bool operator==(const SdrConnectStyleData& rCompare) const;
            };

            std::vector<SdrConnectStyleData>   maStart;
            std::vector<SdrConnectStyleData>   maEnd;

        public:
            SdrFrameBorderData(
                const basegfx::B2DPoint& rOrigin,
                const basegfx::B2DVector& rX,
                const svx::frame::Style& rStyle,
                const Color* pForceColor);

            void addSdrConnectStyleData(
                bool bStart,
                const svx::frame::Style& rStyle,
                const basegfx::B2DVector& rNormalizedPerpendicular,
                bool bStyleMirrored);

            void create2DDecomposition(
                Primitive2DContainer& rContainer,
                double fMinDiscreteUnit) const;

            double getMinimalNonZeroBorderWidth() const;
            bool operator==(const SdrFrameBorderData& rCompare) const;
        };

        typedef std::vector<SdrFrameBorderData> SdrFrameBorderDataVector;

} // end of namespace drawinglayer::primitive2d

namespace drawinglayer::primitive2d
    {
        class SVXCORE_DLLPUBLIC SdrFrameBorderPrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            std::shared_ptr<SdrFrameBorderDataVector>   maFrameBorders;
            double                                      mfMinimalNonZeroBorderWidth;
            double                                      mfMinimalNonZeroBorderWidthUsedForDecompose;
            bool                                        mbForceToSingleDiscreteUnit;

            // local decomposition.
            virtual void create2DDecomposition(
                Primitive2DContainer& rContainer,
                const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrFrameBorderPrimitive2D(
                std::shared_ptr<SdrFrameBorderDataVector>& rFrameBorders,
                bool bForceToSingleDiscreteUnit);

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // override to get view-dependent
            virtual void get2DDecomposition(
                Primitive2DDecompositionVisitor& rVisitor,
                const geometry::ViewInformation2D& rViewInformation) const override;

            // data access
            const std::shared_ptr<SdrFrameBorderDataVector>& getFrameBorders() const { return maFrameBorders; }
            bool doForceToSingleDiscreteUnit() const { return mbForceToSingleDiscreteUnit; }

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

} // end of namespace drawinglayer::primitive2d


#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRFRAMEBORDERPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
