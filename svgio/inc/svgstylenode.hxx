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
        class SvgStyleNode final : public SvgNode
        {
        private:
            /// use styles
            std::vector< SvgStyleAttributes* >      maSvgStyleAttributes;

            bool                                    mbTextCss : 1; // true == type is 'text/css'

        public:
            SvgStyleNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgStyleNode() override;

            /// #i125258# tell if this node is allowed to have a parent style (e.g. defs do not)
            virtual bool supportsParentStyle() const override;

            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;

            /// CssStyleSheet add helpers
            void addCssStyleSheet(const OUString& aSelectors, const SvgStyleAttributes& rNewStyle);
            void addCssStyleSheet(const OUString& aSelectors, const OUString& aContent);
            void addCssStyleSheet(const OUString& aSelectorsAndContent);

            /// textCss access
            bool isTextCss() const { return mbTextCss; }
            void setTextCss(bool bNew) { mbTextCss = bNew; }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
