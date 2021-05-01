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

#include "svgstyleattributes.hxx"
#include <memory>

namespace svgio::svgreader
    {
        class SvgSvgNode final : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            std::unique_ptr<basegfx::B2DRange>
                                    mpViewBox;
            SvgAspectRatio          maSvgAspectRatio;
            SvgNumber               maX;
            SvgNumber               maY;
            SvgNumber               maWidth;
            SvgNumber               maHeight;
            SvgNumber               maVersion;

            /// #i125258# bitfield
            bool                    mbStyleAttributesInitialized : 1;

            // #i125258# on-demand init hard attributes when this is the outmost svg element
            // and more (see implementation)
            void initializeStyleAttributes();

        public:
            SvgSvgNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgSvgNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const override;

            /// Seeks width and height of viewport, which is current before the new viewport is set.
            // needed for percentage unit in x, y, width or height
            void seekReferenceWidth(double& fWidth, bool& bHasFound) const;
            void seekReferenceHeight(double& fHeight, bool& bHasFound) const;

            /// InfoProvider support for % values in children
            // The returned 'CurrentViewPort' is the viewport as it is set by this svg element
            // and as it is needed to resolve relative values in children
            // The method does not check for invalid width and height
            virtual basegfx::B2DRange getCurrentViewPort() const override;

            /// viewBox content
            const basegfx::B2DRange* getViewBox() const { return mpViewBox.get(); }
            void setViewBox(const basegfx::B2DRange* pViewBox) { mpViewBox.reset(); if(pViewBox) mpViewBox.reset( new basegfx::B2DRange(*pViewBox) ); }

            /// SvgAspectRatio content
            const SvgAspectRatio& getSvgAspectRatio() const { return maSvgAspectRatio; }

            /// x content
            const SvgNumber& getX() const { return maX; }

            /// y content
            const SvgNumber& getY() const { return maY; }

            /// width content
            const SvgNumber& getWidth() const { return maWidth; }

            /// height content
            const SvgNumber& getHeight() const { return maHeight; }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
