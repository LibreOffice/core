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
#include <memory>

namespace svgio::svgreader
    {
        class SvgEllipseNode final : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber               maCx;
            SvgNumber               maCy;
            SvgNumber               maRx;
            SvgNumber               maRy;
            std::unique_ptr<basegfx::B2DHomMatrix>  mpaTransform;

        public:
            SvgEllipseNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgEllipseNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const override;

            /// Cx content, set if found in current context
            const SvgNumber& getCx() const { return maCx; }

            /// Cy content, set if found in current context
            const SvgNumber& getCy() const { return maCy; }

            /// Rx content, set if found in current context
            const SvgNumber& getRx() const { return maRx; }

            /// Ry content, set if found in current context
            const SvgNumber& getRy() const { return maRy; }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform.get(); }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix) { mpaTransform.reset(); if(pMatrix) mpaTransform.reset( new basegfx::B2DHomMatrix(*pMatrix) ); }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
