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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGMASKNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGMASKNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgMaskNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber                   maX;
            SvgNumber                   maY;
            SvgNumber                   maWidth;
            SvgNumber                   maHeight;
            basegfx::B2DHomMatrix*      mpaTransform;
            SvgUnits                    maMaskUnits;
            SvgUnits                    maMaskContentUnits;

        public:
            SvgMaskNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgMaskNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const override;

            /// apply contained clipPath to given geometry #i124852# transform may be needed
            void apply(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const basegfx::B2DHomMatrix* pTransform) const;

            /// x content, set if found in current context
            const SvgNumber& getX() const { return maX; }
            void setX(const SvgNumber& rX = SvgNumber()) { maX = rX; }

            /// y content, set if found in current context
            const SvgNumber& getY() const { return maY; }
            void setY(const SvgNumber& rY = SvgNumber()) { maY = rY; }

            /// width content, set if found in current context
            const SvgNumber& getWidth() const { return maWidth; }
            void setWidth(const SvgNumber& rWidth = SvgNumber()) { maWidth = rWidth; }

            /// height content, set if found in current context
            const SvgNumber& getHeight() const { return maHeight; }
            void setHeight(const SvgNumber& rHeight = SvgNumber()) { maHeight = rHeight; }

            /// transform content
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

            /// MaskUnits content
            SvgUnits getMaskUnits() const { return maMaskUnits; }
            void setMaskUnits(const SvgUnits aMaskUnits) { maMaskUnits = aMaskUnits; }

            /// MaskContentUnits content
            SvgUnits getMaskContentUnits() const { return maMaskContentUnits; }
            void setMaskContentUnits(const SvgUnits aMaskContentUnits) { maMaskContentUnits = aMaskContentUnits; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGMASKNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
