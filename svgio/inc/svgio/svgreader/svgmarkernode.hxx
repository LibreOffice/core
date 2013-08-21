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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGMARKERNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGMARKERNODE_HXX

#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgMarkerNode : public SvgNode
        {
        public:
            enum MarkerUnits
            {
                strokeWidth,
                userSpaceOnUse
            };

        private:
            /// buffered decomposition
            drawinglayer::primitive2d::Primitive2DSequence aPrimitives;

            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DRange*      mpViewBox;
            SvgAspectRatio          maSvgAspectRatio;
            SvgNumber               maRefX;
            SvgNumber               maRefY;
            MarkerUnits             maMarkerUnits;
            SvgNumber               maMarkerWidth;
            SvgNumber               maMarkerHeight;
            double                  mfAngle;

            /// bitfield
            bool                    mbOrientAuto : 1; // true == on, false == fAngle valid

        public:
            SvgMarkerNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgMarkerNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent);

            /// get marker primitives buffered, uses decomposeSvgNode internally
            const drawinglayer::primitive2d::Primitive2DSequence& getMarkerPrimitives() const;

            /// InfoProvider support for % values
            virtual const basegfx::B2DRange getCurrentViewPort() const;

            /// viewBox content
            const basegfx::B2DRange* getViewBox() const { return mpViewBox; }
            void setViewBox(const basegfx::B2DRange* pViewBox = 0) { if(mpViewBox) delete mpViewBox; mpViewBox = 0; if(pViewBox) mpViewBox = new basegfx::B2DRange(*pViewBox); }

            /// SvgAspectRatio content
            const SvgAspectRatio& getSvgAspectRatio() const { return maSvgAspectRatio; }
            void setSvgAspectRatio(const SvgAspectRatio& rSvgAspectRatio = SvgAspectRatio()) { maSvgAspectRatio = rSvgAspectRatio; }

            /// RefX content, set if found in current context
            const SvgNumber& getRefX() const { return maRefX; }
            void setRefX(const SvgNumber& rRefX = SvgNumber()) { maRefX = rRefX; }

            /// RefY content, set if found in current context
            const SvgNumber& getRefY() const { return maRefY; }
            void setRefY(const SvgNumber& rRefY = SvgNumber()) { maRefY = rRefY; }

            /// MarkerUnits content
            MarkerUnits getMarkerUnits() const { return maMarkerUnits; }
            void setMarkerUnits(const MarkerUnits aMarkerUnits) { maMarkerUnits = aMarkerUnits; }

            /// MarkerWidth content, set if found in current context
            const SvgNumber& getMarkerWidth() const { return maMarkerWidth; }
            void setMarkerWidth(const SvgNumber& rMarkerWidth = SvgNumber()) { maMarkerWidth = rMarkerWidth; }

            /// MarkerHeight content, set if found in current context
            const SvgNumber& getMarkerHeight() const { return maMarkerHeight; }
            void setMarkerHeight(const SvgNumber& rMarkerHeight = SvgNumber()) { maMarkerHeight = rMarkerHeight; }

            /// Angle content, set if found in current context
            double getAngle() const { return mfAngle; }
            void setAngle(double fAngle = 0.0) { mfAngle = fAngle; mbOrientAuto = false; }

            /// OrientAuto content, set if found in current context
            bool getOrientAuto() const { return mbOrientAuto; }
            void setOrientAuto(bool bOrientAuto = true) { mbOrientAuto = bOrientAuto; }

        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGMARKERNODE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
