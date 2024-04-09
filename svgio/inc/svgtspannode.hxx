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

#include "svgnode.hxx"
#include "svgstyleattributes.hxx"

namespace svgio::svgreader
    {
        class SvgTspanNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            SvgNumberVector         maX;
            SvgNumberVector         maY;
            SvgNumberVector         maDx;
            SvgNumberVector         maDy;
            SvgNumberVector         maRotate;
            SvgNumber               maTextLength;

            bool                    mbLengthAdjust : 1; // true = spacing, false = spacingAndGlyphs

            // The text line width composed by the different SvgCharacterNode children
            // it will be used to calculate their alignment
            double mnTextLineWidth;

        public:
            SvgTspanNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTspanNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(SVGToken aSVGToken, const OUString& aContent) override;

            /// X content
            const SvgNumberVector& getX() const { return maX; }
            void setX(SvgNumberVector&& aX) { maX = std::move(aX); }

            /// Y content
            const SvgNumberVector& getY() const { return maY; }
            void setY(SvgNumberVector&& aY) { maY = std::move(aY); }

            /// Dx content
            const SvgNumberVector& getDx() const { return maDx; }
            void setDx(SvgNumberVector&& aDx) { maDx = std::move(aDx); }

            /// Dy content
            const SvgNumberVector& getDy() const { return maDy; }
            void setDy(SvgNumberVector&& aDy) { maDy = std::move(aDy); }

            /// Rotate content
            const SvgNumberVector& getRotate() const { return maRotate; }
            void setRotate(SvgNumberVector&& aRotate) { maRotate = std::move(aRotate); }

            /// TextLength content
            const SvgNumber& getTextLength() const { return maTextLength; }
            void setTextLength(const SvgNumber& rTextLength) { maTextLength = rTextLength; }

            /// LengthAdjust content
            bool getLengthAdjust() const { return mbLengthAdjust; }
            void setLengthAdjust(bool bNew) { mbLengthAdjust = bNew; }

            void concatenateTextLineWidth(double nWidth) {mnTextLineWidth += nWidth;}
            double getTextLineWidth() const { return mnTextLineWidth; }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
