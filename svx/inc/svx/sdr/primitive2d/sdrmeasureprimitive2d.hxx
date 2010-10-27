/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
