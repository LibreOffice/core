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

#include <sal/config.h>
#include <rtl/ref.hxx>

#include <string_view>

#include "svgtextnode.hxx"
#include "svgtextposition.hxx"

namespace drawinglayer::primitive2d { class TextSimplePortionPrimitive2D; }

namespace svgio::svgreader
    {
        class SvgCharacterNode final : public SvgNode
        {
        private:
            /// the string data
            OUString           maText;

            // keep a copy of string data before space handling
            OUString           maTextBeforeSpaceHandling;

            SvgTextNode*        mpTextParent;

            /// local helpers
            rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> createSimpleTextPrimitive(
                SvgTextPosition& rSvgTextPosition,
                const SvgStyleAttributes& rSvgStyleAttributes) const;
            void decomposeTextWithStyle(
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                SvgTextPosition& rSvgTextPosition,
                const SvgStyleAttributes& rSvgStyleAttributes) const;

        public:
            SvgCharacterNode(
                SvgDocument& rDocument,
                SvgNode* pParent,
                OUString aText);
            virtual ~SvgCharacterNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;

            void decomposeText(drawinglayer::primitive2d::Primitive2DContainer& rTarget, SvgTextPosition& rSvgTextPosition) const;
            void whiteSpaceHandling();
            SvgCharacterNode* addGap(SvgCharacterNode* pPreviousCharacterNode);
            void concatenate(std::u16string_view rText);

            /// Text content
            const OUString& getText() const { return maText; }

            void setTextParent(SvgTextNode* pTextParent) { mpTextParent = pTextParent; }
        };

} // end of namespace svgio::svgreader


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
