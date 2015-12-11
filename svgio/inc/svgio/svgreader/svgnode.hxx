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

#ifndef INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGNODE_HXX
#define INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGNODE_HXX

#include <svgio/svgreader/svgtools.hxx>
#include <svgio/svgreader/svgtoken.hxx>
#include <svgio/svgreader/svgpaint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <vector>
#include <boost/noncopyable.hpp>

// predefines
namespace svgio
{
    namespace svgreader
    {
        class SvgNode;
        class SvgDocument;
        class SvgStyleAttributes;
    }
}



namespace svgio
{
    namespace svgreader
    {
        typedef ::std::vector< SvgNode* > SvgNodeVector;
        typedef ::std::vector< const SvgStyleAttributes* > SvgStyleAttributeVector;

        enum XmlSpace
        {
            XmlSpace_notset,
            XmlSpace_default,
            XmlSpace_preserve
        };

        // display property (see SVG 1.1. 11.5), not inheritable
        enum Display // #i121656#
        {
            Display_inline, // the default
            Display_block,
            Display_list_item,
            Display_run_in,
            Display_compact,
            Display_marker,
            Display_table,
            Display_inline_table,
            Display_table_row_group,
            Display_table_header_group,
            Display_table_footer_group,
            Display_table_row,
            Display_table_column_group,
            Display_table_column,
            Display_table_cell,
            Display_table_caption,
            Display_none,
            Display_inherit
        };

        // helper to convert a string associated with a token of type SVGTokenDisplay
        // to the enum Display. Empty strings return the default 'Display_inline' with
        // which members should be initialized
        Display getDisplayFromContent(const OUString& aContent);

        class SvgNode : private boost::noncopyable, public InfoProvider
        {
        private:
            /// basic data, Type, document we belong to and parent (if not root)
            SVGToken                    maType;
            SvgDocument&                mrDocument;
            const SvgNode*              mpParent;
            const SvgNode*              mpAlternativeParent;

            /// sub hierarchy
            SvgNodeVector               maChildren;

            /// Id svan value
            OUString*              mpId;

            /// Class svan value
            OUString*              mpClass;

            /// XmlSpace value
            XmlSpace                    maXmlSpace;

            /// Display value #i121656#
            Display                     maDisplay;

            // CSS style vector chain, used in decompose phase and built up once per node.
            // It contains the StyleHierarchy for the local node. INdependent from the
            // node hierarchy itself which also needs to be used in style entry solving
            SvgStyleAttributeVector     maCssStyleVector;

            /// possible local CssStyle, e.g. style="fill:red; stroke:red;"
            SvgStyleAttributes*         mpLocalCssStyle;

            /// bitfield
            // flag if maCssStyleVector is already computed (done only once)
            bool                        mbCssStyleVectorBuilt : 1;

        protected:
            /// helper to evtl. link to css style
            const SvgStyleAttributes* checkForCssStyle(const OUString& rClassStr, const SvgStyleAttributes& rOriginal) const;

            /// helper for filling the CssStyle vector once dependent on mbCssStyleVectorBuilt
            void fillCssStyleVector(const OUString& rClassStr);
            void fillCssStyleVectorUsingHierarchyAndSelectors(
                const OUString& rClassStr,
                const SvgNode& rCurrent,
                const OUString& aConcatenated);

        public:
            SvgNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgNode();

            /// scan helper to read and interpret a local CssStyle to mpLocalCssStyle
            void readLocalCssStyle(const OUString& aContent);

            /// style helpers
            void parseAttributes(const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs);
            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const;

            /// #i125258# tell if this node is allowed to have a parent style (e.g. defs do not)
            virtual bool supportsParentStyle() const;

            /// basic data read access
            SVGToken getType() const { return maType; }
            const SvgDocument& getDocument() const { return mrDocument; }
            const SvgNode* getParent() const { if(mpAlternativeParent) return mpAlternativeParent; return mpParent; }
            const SvgNodeVector& getChildren() const { return maChildren; }

            /// InfoProvider support for %, em and ex values
            virtual const basegfx::B2DRange getCurrentViewPort() const override;
            virtual double getCurrentFontSizeInherited() const override;
            virtual double getCurrentXHeightInherited() const override;

            double getCurrentFontSize() const;
            double getCurrentXHeight() const;

            /// Id access
            const OUString* getId() const { return mpId; }
            void setId(const OUString* pfId = nullptr);

            /// Class access
            const OUString* getClass() const { return mpClass; }
            void setClass(const OUString* pfClass = nullptr);

            /// XmlSpace access
            XmlSpace getXmlSpace() const;
            void setXmlSpace(XmlSpace eXmlSpace = XmlSpace_notset) { maXmlSpace = eXmlSpace; }

            /// Display access #i121656#
            Display getDisplay() const { return maDisplay; }
            void setDisplay(Display eDisplay = Display_inherit) { maDisplay = eDisplay; }

            /// alternative parent
            void setAlternativeParent(const SvgNode* pAlternativeParent = nullptr) { mpAlternativeParent = pAlternativeParent; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGIO_SVGREADER_SVGNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
