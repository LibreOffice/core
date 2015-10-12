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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <svgio/svgreader/svgcharacternode.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgTextNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DHomMatrix*  mpaTransform;
            SvgTextPositions        maSvgTextPositions;

            /// local helpers
            void DecomposeChild(
                const SvgNode& rCandidate,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                SvgTextPosition& rSvgTextPosition) const;
            static void addTextPrimitives(
                const SvgNode& rCandidate,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                drawinglayer::primitive2d::Primitive2DSequence& rSource);

        public:
            SvgTextNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTextNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const override;

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

            /// access to SvgTextPositions
            const SvgTextPositions& getSvgTextPositions() const { return maSvgTextPositions; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGTEXTNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
