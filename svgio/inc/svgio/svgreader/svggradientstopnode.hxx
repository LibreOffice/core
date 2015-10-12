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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGGRADIENTSTOPNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGGRADIENTSTOPNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgGradientStopNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// local attributes
            SvgNumber                   maOffset;

        public:
            SvgGradientStopNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgGradientStopNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;

            /// offset content
            const SvgNumber getOffset() const { return maOffset; }
            void setOffset(const SvgNumber& rOffset = SvgNumber()) { maOffset = rOffset; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGGRADIENTSTOPNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
