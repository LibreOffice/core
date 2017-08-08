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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

enum class SvxBorderLineStyle : sal_Int16;

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLine class
        Helper class holding the style definition for a single part of a full BNorderLine definition
        */
        class DRAWINGLAYER_DLLPUBLIC BorderLine
        {
        private:
            // line width
            double              mfWidth;

            // line color
            basegfx::BColor     maRGBColor;

            // line extends
            double              mfExtendStart;
            double              mfExtendEnd;

        public:
            BorderLine(
                double fWidth,
                const basegfx::BColor& rRGBColor,
                double fExtendStart = 0.0,
                double fExtendEnd = 0.0);
            ~BorderLine();

            double getWidth() const { return mfWidth; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }
            double getExtendStart() const { return mfExtendStart; }
            double getExtendEnd() const { return mfExtendEnd; }

            /// compare operator
            bool operator==(const BorderLine& rBorderLine) const;
        };

        /** BorderLinePrimitive2D class

        This is the basic primitive to build frames around objects, e.g. tables.
        It defines a single or double line from Start to End using the LeftWidth,
        Distance and RightWidth definitions.
        The LineStart/End overlap is defined by the Extend(Left|Right)(Start|End)
        definitions.
        */
        class DRAWINGLAYER_DLLPUBLIC BorderLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line definition
            basegfx::B2DPoint                               maStart;
            basegfx::B2DPoint                               maEnd;

            /// the single BorderLine style definition(s), one or three allowed (see constructors)
            std::vector< BorderLine >                       maBorderLines;

            bool                                            mbHasGapColor;

            /// common style definitions
            SvxBorderLineStyle                              mnStyle;
            double                                          mfPatternScale;

            // for view dependent decomposition in the case with distance (gap),
            // remember the last used concrete mfDistance, see get2DDecomposition
            // implementation
            double                                          mfDiscreteGapDistance;

            /// create local decomposition
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// simplified constructor for BorderLine with single edge
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                const BorderLine& rBorderLine,
                SvxBorderLineStyle nStyle,
                double fPatternScale = 1.0);

            /// constructor for full-fledged BorderLine with two edges and gap
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                const BorderLine& rLeft,
                const BorderLine& rGap,
                const BorderLine& rRight,
                bool bHasGapColor,
                SvxBorderLineStyle nStyle,
                double fPatternScale = 1.0);

            /// data read access
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            const std::vector< BorderLine >& getBorderLines() const { return maBorderLines; }
            bool hasGapColor() const { return mbHasGapColor; }
            SvxBorderLineStyle getStyle() const { return mnStyle; }
            double getPatternScale() const { return mfPatternScale; }

            /// helper to decide if AntiAliasing should be used
            bool isHorizontalOrVertical(const geometry::ViewInformation2D& rViewInformation) const;

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// Override standard getDecomposition to be view-dependent here
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
