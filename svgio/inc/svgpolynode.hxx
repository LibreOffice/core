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
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <optional>

namespace svgio::svgreader
    {
        class SvgPolyNode final : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            std::optional<basegfx::B2DPolygon>    mpPolygon;
            std::optional<basegfx::B2DHomMatrix>  mpaTransform;

            bool                        mbIsPolyline : 1; // true = polyline, false = polygon

        public:
            SvgPolyNode(
                SvgDocument& rDocument,
                SvgNode* pParent,
                bool bIsPolyline);
            virtual ~SvgPolyNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const override;

            /// Polygon content, set if found in current context
            void setPolygon(const std::optional<basegfx::B2DPolygon>& pPolygon) { mpPolygon = pPolygon; }

            /// transform content, set if found in current context
            const std::optional<basegfx::B2DHomMatrix>& getTransform() const { return mpaTransform; }
            void setTransform(const std::optional<basegfx::B2DHomMatrix>& pMatrix) { mpaTransform = pMatrix; }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
