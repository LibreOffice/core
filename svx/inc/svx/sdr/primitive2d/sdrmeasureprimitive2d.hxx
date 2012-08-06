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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/sdr/attribute/sdrlineshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace primitive2d {
    enum MeasureTextPosition
    {
        MEASURETEXTPOSITION_AUTOMATIC,
        MEASURETEXTPOSITION_NEGATIVE,
        MEASURETEXTPOSITION_CENTERED,
        MEASURETEXTPOSITION_POSITIVE
    };
}}

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrMeasurePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            attribute::SdrLineShadowTextAttribute       maSdrLSTAttribute;
            basegfx::B2DPoint                           maStart;
            basegfx::B2DPoint                           maEnd;
            MeasureTextPosition                         meHorizontal;
            MeasureTextPosition                         meVertical;
            double                                      mfDistance;
            double                                      mfUpper;
            double                                      mfLower;
            double                                      mfLeftDelta;
            double                                      mfRightDelta;

            // bitfield
            unsigned                                    mbBelow : 1;
            unsigned                                    mbTextRotation : 1;
            unsigned                                    mbTextAutoAngle : 1;

            // internal decomposition helper
            Primitive2DReference impCreatePart(
                const attribute::SdrLineAttribute& rLineAttribute,
                const basegfx::B2DHomMatrix& rObjectMatrix,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                bool bLeftActive,
                bool bRightActive) const;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrMeasurePrimitive2D(
                const attribute::SdrLineShadowTextAttribute& rSdrLSTAttribute,
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
            const attribute::SdrLineShadowTextAttribute& getSdrLSTAttribute() const { return maSdrLSTAttribute; }
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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRMEASUREPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
