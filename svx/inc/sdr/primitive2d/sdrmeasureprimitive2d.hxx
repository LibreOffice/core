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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <sdr/attribute/sdrlineeffectstextattribute.hxx>


// predefines

namespace drawinglayer::primitive2d {
    enum MeasureTextPosition
    {
        MEASURETEXTPOSITION_AUTOMATIC,
        MEASURETEXTPOSITION_NEGATIVE,
        MEASURETEXTPOSITION_CENTERED,
        MEASURETEXTPOSITION_POSITIVE
    };
}

namespace drawinglayer::attribute {
    class SdrLineAttribute;
}


namespace drawinglayer::primitive2d
    {
        class SdrMeasurePrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            attribute::SdrLineEffectsTextAttribute       maSdrLSTAttribute;
            basegfx::B2DPoint                           maStart;
            basegfx::B2DPoint                           maEnd;
            MeasureTextPosition                         meHorizontal;
            MeasureTextPosition                         meVertical;
            double                                      mfDistance;
            double                                      mfUpper;
            double                                      mfLower;
            double                                      mfLeftDelta;
            double                                      mfRightDelta;

            bool                                        mbBelow : 1;
            bool                                        mbTextRotation : 1;
            bool                                        mbTextAutoAngle : 1;

            // internal decomposition helper
            Primitive2DReference impCreatePart(
                const attribute::SdrLineAttribute& rLineAttribute,
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                bool bLeftActive,
                bool bRightActive) const;

            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrMeasurePrimitive2D(
                const attribute::SdrLineEffectsTextAttribute& rSdrLSTAttribute,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                MeasureTextPosition eHorizontal,
                MeasureTextPosition eVertical,
                double fDistance,
                double fUpper,
                double fLower,
                double fLeftDelta,
                double fRightDelta,
                bool bBelow,
                bool bTextRotation,
                bool bTextAutoAngle);

            // data access
            const attribute::SdrLineEffectsTextAttribute& getSdrLSTAttribute() const { return maSdrLSTAttribute; }
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            MeasureTextPosition getHorizontal() const { return meHorizontal; }
            MeasureTextPosition getVertical() const { return meVertical; }
            double getDistance() const { return mfDistance; }
            double getUpper() const { return mfUpper; }
            double getLower() const { return mfLower; }
            double getLeftDelta() const { return mfLeftDelta; }
            double getRightDelta() const { return mfRightDelta; }
            bool getBelow() const { return mbBelow; }
            bool getTextRotation() const { return mbTextRotation; }
            bool getTextAutoAngle() const { return mbTextAutoAngle; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
