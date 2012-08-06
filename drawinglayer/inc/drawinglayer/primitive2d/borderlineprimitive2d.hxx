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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svtools/ctrlbox.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLinePrimitive2D class

            This is the basic primitive to build frames around objects, e.g. tables.
            It defines a single or double line from Start to nd using the LeftWidth,
            Distance and RightWidth definitions.
            The LineStart/End overlap is defined by the Extend(Inner|Outer)(Start|End)
            definitions.
         */
        class DRAWINGLAYER_DLLPUBLIC BorderLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line definition
            basegfx::B2DPoint                               maStart;
            basegfx::B2DPoint                               maEnd;

            /// the widths of single/double line
            double                                          mfLeftWidth;
            double                                          mfDistance;
            double                                          mfRightWidth;

            /// edge overlap sizes
            double                                          mfExtendLeftStart;
            double                                          mfExtendLeftEnd;
            double                                          mfExtendRightStart;
            double                                          mfExtendRightEnd;

            /// the line colors
            basegfx::BColor                                 maRGBColorRight;
            basegfx::BColor                                 maRGBColorLeft;
            basegfx::BColor                                 maRGBColorGap;
            bool                                            mbHasGapColor;

            short                                           mnStyle;

            /// bitfield
            /// flags to influence inside/outside creation
            unsigned                                        mbCreateInside : 1;
            unsigned                                        mbCreateOutside : 1;

            /// local helpers
            double getWidth(
                    const geometry::ViewInformation2D& rViewInformation) const;

            bool isSolidLine() const
            {
                return (mnStyle==STYLE_SOLID);
            }

            bool isInsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfLeftWidth);
            }

            bool isOutsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfRightWidth);
            }

        protected:
            virtual basegfx::B2DPolyPolygon getClipPolygon(
                    const geometry::ViewInformation2D& rViewInformation) const;

            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                double fLeftWidth,
                double fDistance,
                double fRightWidth,
                double fExtendLeftStart,
                double fExtendLeftEnd,
                double fExtendRightStart,
                double fExtendRightEnd,
                const basegfx::BColor& rRGBColorRight,
                const basegfx::BColor& rRGBColorLeft,
                const basegfx::BColor& rRGBColorGap,
                bool bHasGapColor,
                const short nStyle );

            /// data read access
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            double getLeftWidth() const { return mfLeftWidth; }
            double getDistance() const { return mfDistance; }
            double getRightWidth() const { return mfRightWidth; }
            double getExtendLeftStart() const { return mfExtendLeftStart; }
            double getExtendLeftEnd() const { return mfExtendLeftEnd; }
            double getExtendRightStart() const { return mfExtendRightStart; }
            double getExtendRightEnd() const { return mfExtendRightEnd; }
            const basegfx::BColor& getRGBColorRight () const { return maRGBColorRight; }
            const basegfx::BColor& getRGBColorLeft () const { return maRGBColorLeft; }
            const basegfx::BColor& getRGBColorGap () const { return maRGBColorGap; }
            bool hasGapColor( ) const { return mbHasGapColor; }
            short getStyle () const { return mnStyle; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
