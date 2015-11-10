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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGPOLYNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGPOLYNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

namespace svgio
{
    namespace svgreader
    {
        class SvgPolyNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DPolygon*        mpPolygon;
            basegfx::B2DHomMatrix*      mpaTransform;

            /// bitfield
            bool                        mbIsPolyline : 1; // true = polyline, false = polygon

        public:
            SvgPolyNode(
                SvgDocument& rDocument,
                SvgNode* pParent,
                bool bIsPolyline);
            virtual ~SvgPolyNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const override;

            /// type read access
            bool isPolyline() const { return mbIsPolyline; }

            /// Polygon content, set if found in current context
            const basegfx::B2DPolygon* getPolygon() const { return mpPolygon; }
            void setPolygon(const basegfx::B2DPolygon* pPolygon = nullptr) { if(mpPolygon) delete mpPolygon; mpPolygon = nullptr; if(pPolygon) mpPolygon = new basegfx::B2DPolygon(*pPolygon); }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = nullptr) { if(mpaTransform) delete mpaTransform; mpaTransform = nullptr; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGPOLYNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
