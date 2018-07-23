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

#ifndef INCLUDED_SVGIO_INC_SVGPATTERNNODE_HXX
#define INCLUDED_SVGIO_INC_SVGPATTERNNODE_HXX

#include "svgnode.hxx"
#include "svgstyleattributes.hxx"
#include <memory>

namespace svgio
{
    namespace svgreader
    {
        class SvgPatternNode : public SvgNode
        {
        private:
            /// buffered decomposition
            drawinglayer::primitive2d::Primitive2DContainer aPrimitives;

            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            std::unique_ptr<basegfx::B2DRange>
                                    mpViewBox;
            SvgAspectRatio          maSvgAspectRatio;
            SvgNumber               maX;
            SvgNumber               maY;
            SvgNumber               maWidth;
            SvgNumber               maHeight;
            std::unique_ptr<SvgUnits>
                                    mpPatternUnits;
            std::unique_ptr<SvgUnits>
                                    mpPatternContentUnits;
            std::unique_ptr<basegfx::B2DHomMatrix>
                                    mpaPatternTransform;

            /// link to another pattern used as style. If maXLink
            /// is set, the node can be fetched on demand by using
            // tryToFindLink (buffered)
            mutable bool mbResolvingLink; // protect against infinite link recursion
            OUString           maXLink;
            const SvgPatternNode*   mpXLink;

            /// link on demand
            void tryToFindLink();

        public:
            SvgPatternNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgPatternNode() override;

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const override;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent) override;

            /// global helpers
            void getValuesRelative(double& rfX, double& rfY, double& rfW, double& rfH, const basegfx::B2DRange& rGeoRange, SvgNode const & rUser) const;

            /// get pattern primitives buffered, uses decomposeSvgNode internally
            const drawinglayer::primitive2d::Primitive2DContainer& getPatternPrimitives() const;

            /// InfoProvider support for % values
            virtual const basegfx::B2DRange getCurrentViewPort() const override;

            /// viewBox content
            const basegfx::B2DRange* getViewBox() const;
            void setViewBox(const basegfx::B2DRange* pViewBox) { mpViewBox.reset(); if(pViewBox) mpViewBox.reset(new basegfx::B2DRange(*pViewBox)); }

            /// SvgAspectRatio content
            const SvgAspectRatio& getSvgAspectRatio() const;

            /// X content, set if found in current context
            const SvgNumber& getX() const;

            /// Y content, set if found in current context
            const SvgNumber& getY() const;

            /// Width content, set if found in current context
            const SvgNumber& getWidth() const;

            /// Height content, set if found in current context
            const SvgNumber& getHeight() const;

            /// PatternUnits content
            const SvgUnits* getPatternUnits() const;
            void setPatternUnits(const SvgUnits aPatternUnits) { mpPatternUnits.reset( new SvgUnits(aPatternUnits) ); }

            /// PatternContentUnits content
            const SvgUnits* getPatternContentUnits() const;
            void setPatternContentUnits(const SvgUnits aPatternContentUnits) { mpPatternContentUnits.reset( new SvgUnits(aPatternContentUnits) ); }

            /// PatternTransform content
            const basegfx::B2DHomMatrix* getPatternTransform() const;
            void setPatternTransform(const basegfx::B2DHomMatrix* pMatrix) { mpaPatternTransform.reset(); if(pMatrix) mpaPatternTransform.reset(new basegfx::B2DHomMatrix(*pMatrix)); }

        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGPATTERNNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
