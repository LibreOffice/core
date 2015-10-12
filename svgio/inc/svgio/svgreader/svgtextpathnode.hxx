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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <svgio/svgreader/svgpathnode.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgTextPathNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// link to path content. If maXLink
            /// is set, the node can be fetched on demand
            OUString           maXLink;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber               maStartOffset;

            /// bitfield
            bool                    mbMethod : 1; // true = align, false = stretch
            bool                    mbSpacing : 1; // true = auto, false = exact

        public:
            SvgTextPathNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTextPathNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            void decomposePathNode(
                const drawinglayer::primitive2d::Primitive2DSequence& rPathContent,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const basegfx::B2DPoint& rTextStart) const;
            bool isValid() const;

            /// StartOffset content
            const SvgNumber& getStartOffset() const { return maStartOffset; }
            void setStartOffset(const SvgNumber& rStartOffset = SvgNumber()) { maStartOffset = rStartOffset; }

            /// Method content
            void setMethod(bool bNew) { mbMethod = bNew; }

            /// Spacing content
            void setSpacing(bool bNew) { mbSpacing = bNew; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
