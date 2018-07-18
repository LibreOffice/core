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

#ifndef INCLUDED_SVGIO_INC_SVGGRADIENTNODE_HXX
#define INCLUDED_SVGIO_INC_SVGGRADIENTNODE_HXX

#include "svgnode.hxx"
#include "svgstyleattributes.hxx"
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <memory>

namespace svgio
{
    namespace svgreader
    {
        class SvgGradientNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// linear gradient values
            SvgNumber                   maX1;
            SvgNumber                   maY1;
            SvgNumber                   maX2;
            SvgNumber                   maY2;

            /// radial gradient values
            SvgNumber                   maCx;
            SvgNumber                   maCy;
            SvgNumber                   maR;
            SvgNumber                   maFx;
            SvgNumber                   maFy;

            /// variable scan values, dependent of given XAttributeList
            SvgUnits                    maGradientUnits;
            drawinglayer::primitive2d::SpreadMethod   maSpreadMethod;
            std::unique_ptr<basegfx::B2DHomMatrix>    mpaGradientTransform;

            /// link to another gradient used as style. If maXLink
            /// is set, the node can be fetched on demand by using
            // tryToFindLink (buffered)
            mutable bool mbResolvingLink; // protect against infinite link recursion
            OUString               maXLink;
            const SvgGradientNode*      mpXLink;

            /// link on demand
            void tryToFindLink();

        public:
            SvgGradientNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgGradientNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;

            /// collect gradient stop entries
            void collectGradientEntries(drawinglayer::primitive2d::SvgGradientEntryVector& aVector) const;

            /// x1 content
            const SvgNumber getX1() const;

            /// y1 content
            const SvgNumber getY1() const;

            /// x2 content
            const SvgNumber getX2() const;

            /// y2 content
            const SvgNumber getY2() const;

            /// Cx content
            const SvgNumber getCx() const;

            /// Cy content
            const SvgNumber getCy() const;

            /// R content
            const SvgNumber getR() const;

            /// Fx content
            const SvgNumber* getFx() const;

            /// Fy content
            const SvgNumber* getFy() const;

            /// gradientUnits content
            SvgUnits getGradientUnits() const { return maGradientUnits; }
            void setGradientUnits(const SvgUnits aGradientUnits) { maGradientUnits = aGradientUnits; }

            /// SpreadMethod content
            drawinglayer::primitive2d::SpreadMethod getSpreadMethod() const { return maSpreadMethod; }
            void setSpreadMethod(const drawinglayer::primitive2d::SpreadMethod aSpreadMethod) { maSpreadMethod = aSpreadMethod; }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getGradientTransform() const;
            void setGradientTransform(const basegfx::B2DHomMatrix* pMatrix);
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGGRADIENTNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
