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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGLINENODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGLINENODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgLineNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber               maX1;
            SvgNumber               maY1;
            SvgNumber               maX2;
            SvgNumber               maY2;
            basegfx::B2DHomMatrix*  mpaTransform;

        public:
            SvgLineNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgLineNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const override;

            /// X1 content, set if found in current context
            const SvgNumber& getX1() const { return maX1; }
            void setX1(const SvgNumber& rX1 = SvgNumber()) { maX1 = rX1; }

            /// Y1 content, set if found in current context
            const SvgNumber& getY1() const { return maY1; }
            void setY1(const SvgNumber& rY1 = SvgNumber()) { maY1 = rY1; }

            /// X2 content, set if found in current context
            const SvgNumber& getX2() const { return maX2; }
            void setX2(const SvgNumber& rX2 = SvgNumber()) { maX2 = rX2; }

            /// Y2 content, set if found in current context
            const SvgNumber& getY2() const { return maY2; }
            void setY2(const SvgNumber& rY2 = SvgNumber()) { maY2 = rY2; }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = nullptr) { if(mpaTransform) delete mpaTransform; mpaTransform = nullptr; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGLINENODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
