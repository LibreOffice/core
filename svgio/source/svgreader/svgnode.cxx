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

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <tools/urlobj.hxx>


namespace svgio
{
    namespace svgreader
    {
        /// #i125258#
        bool SvgNode::supportsParentStyle() const
        {
            return true;
        }

        const SvgStyleAttributes* SvgNode::getSvgStyleAttributes() const
        {
            return 0;
        }

        const SvgStyleAttributes* SvgNode::checkForCssStyle(const rtl::OUString& rClassStr, const SvgStyleAttributes& rOriginal) const
        {
            if(maCssStyleVector.empty()) // #120435# Evaluate for CSS styles only once, this cannot change
            {
                const SvgDocument& rDocument = getDocument();

                if(rDocument.hasSvgStyleAttributesById())
                {
                    if(getClass())
                    {
                        // find all referenced CSS styles, a list of entries is allowed
                        const rtl::OUString* pClassList = getClass();
                        const sal_Int32 nLen(pClassList->getLength());
                        sal_Int32 nPos(0);
                        const SvgStyleAttributes* pNew = 0;

                        skip_char(*pClassList, ' ', nPos, nLen);

                        while(nPos < nLen)
                        {
                            rtl::OUStringBuffer aTokenValue;

                            copyToLimiter(*pClassList, ' ', nPos, aTokenValue, nLen);
                            skip_char(*pClassList, ' ', nPos, nLen);

                            rtl::OUString aId(".");
                            const rtl::OUString aOUTokenValue(aTokenValue.makeStringAndClear());

                            // look for CSS style common to token
                            aId += aOUTokenValue;
                            pNew = rDocument.findSvgStyleAttributesById(aId);

                            if(!pNew && !rClassStr.isEmpty())
                            {
                                // look for CSS style common to class.token
                                aId = rClassStr + aId;

                                pNew = rDocument.findSvgStyleAttributesById(aId);
                            }

                            if(pNew)
                            {
                                const_cast< SvgNode* >(this)->maCssStyleVector.push_back(pNew);
                            }
                        }
                    }

                    if(maCssStyleVector.empty() && getId())
                    {
                        // if none found, search for CSS style equal to Id
                        const SvgStyleAttributes* pNew = rDocument.findSvgStyleAttributesById(*getId());

                        if(pNew)
                        {
                            const_cast< SvgNode* >(this)->maCssStyleVector.push_back(pNew);
                        }
                    }

                    if(maCssStyleVector.empty() && !rClassStr.isEmpty())
                    {
                        // if none found, search for CSS style equal to class type
                        const SvgStyleAttributes* pNew = rDocument.findSvgStyleAttributesById(rClassStr);

                        if(pNew)
                        {
                            const_cast< SvgNode* >(this)->maCssStyleVector.push_back(pNew);
                        }
                    }
                }
            }

            if(!maCssStyleVector.empty())
            {
                // #i123510# if CSS styles were found, create a linked list with rOriginal as parent
                // and all CSS styles as linked children, so that the style attribute has
                // priority over the CSS style. If there is no style attribute this means that
                // no values are set at rOriginal, thus it is still correct to have that order.
                // Repeated style requests should only be issued from sub-Text nodes and I'm not
                // sure if in-between text nodes may build other chains (should not happen). But
                // it's only a re-chaining with pointers (cheap), so allow to do it every time.
                SvgStyleAttributes* pCurrent = const_cast< SvgStyleAttributes* >(&rOriginal);
                pCurrent->setCssStyleParent(0);

                for(sal_uInt32 a(0); a < maCssStyleVector.size(); a++)
                {
                    SvgStyleAttributes* pNext = const_cast< SvgStyleAttributes* >(maCssStyleVector[a]);

                    pCurrent->setCssStyleParent(pNext);
                    pCurrent = pNext;
                    pCurrent->setCssStyleParent(0);
                }
            }

            return &rOriginal;
        }

        SvgNode::SvgNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   maType(aType),
            mrDocument(rDocument),
            mpParent(pParent),
            mpAlternativeParent(0),
            maChildren(),
            mpId(0),
            mpClass(0),
            maXmlSpace(XmlSpace_notset),
            maDisplay(Display_inline),
            maCssStyleVector()
        {
            OSL_ENSURE(SVGTokenUnknown != maType, "SvgNode with unknown type created (!)");

            if(pParent)
            {
                pParent->maChildren.push_back(this);
            }
            else
            {
#ifdef DBG_UTIL
                if(SVGTokenSvg != getType())
                {
                    OSL_ENSURE(false, "No parent for this node (!)");
                }
#endif
            }
        }

        SvgNode::~SvgNode()
        {
            while(maChildren.size())
            {
                delete maChildren[maChildren.size() - 1];
                maChildren.pop_back();
            }

            if(mpId) delete mpId;
            if(mpClass) delete mpClass;
        }

        void SvgNode::parseAttributes(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs)
        {
            const sal_uInt32 nAttributes(xAttribs->getLength());
            // #i122522# SVG defines that 'In general, this means that the presentation attributes have
            // lower priority than other CSS style rules specified in author style sheets or style
            // attributes.' in http://www.w3.org/TR/SVG/styling.html#UsingPresentationAttributes
            // (6.4 Specifying properties using the presentation attributes SVG 1.1). That means that
            // e.g. font-size will appear as presentation attribute and CSS style attribute. In these
            // cases, CSS style attributes need to have precedence. To do so it is possible to create
            // a proirity system for all properties of a shape, but it will also work to parse the
            // presentation attributes of type 'style' last, so they will overwrite the less-prioritized
            // already interpreted ones. Thus, remember SVGTokenStyle entries and parse them last.
            // To make this work it is required that parseAttribute is only called by parseAttributes
            // which is the case.
            std::vector< sal_uInt32 > aSVGTokenStyleIndexes;

            for(sal_uInt32 a(0); a < nAttributes; a++)
            {
                const OUString aTokenName(xAttribs->getNameByIndex(a));
                const SVGToken aSVGToken(StrToSVGToken(aTokenName));

                if(SVGTokenStyle == aSVGToken)
                {
                    // #i122522# remember SVGTokenStyle entry
                    aSVGTokenStyleIndexes.push_back(a);
                }
                else
                {
                    parseAttribute(aTokenName, aSVGToken, xAttribs->getValueByIndex(a));
                }
            }

            // #i122522# parse SVGTokenStyle entries last to override already interpreted
            // 'presentation attributes' of potenially the same type
            for(sal_uInt32 b(0); b < aSVGTokenStyleIndexes.size(); b++)
            {
                const sal_uInt32 nSVGTokenStyleIndex(aSVGTokenStyleIndexes[b]);
                const ::rtl::OUString aTokenName(xAttribs->getNameByIndex(nSVGTokenStyleIndex));

                parseAttribute(aTokenName, SVGTokenStyle, xAttribs->getValueByIndex(nSVGTokenStyleIndex));
            }
        }

        Display getDisplayFromContent(const rtl::OUString& aContent)
        {
            if(aContent.getLength())
            {
                if(aContent.startsWith("inline"))
                {
                    return Display_inline;
                }
                else if(aContent.startsWith("none"))
                {
                    return Display_none;
                }
                else if(aContent.startsWith("inherit"))
                {
                    return Display_inherit;
                }
                else if(aContent.startsWith("block"))
                {
                    return Display_block;
                }
                else if(aContent.startsWith("list-item"))
                {
                    return Display_list_item;
                }
                else if(aContent.startsWith("run-in"))
                {
                    return Display_run_in;
                }
                else if(aContent.startsWith("compact"))
                {
                    return Display_compact;
                }
                else if(aContent.startsWith("marker"))
                {
                    return Display_marker;
                }
                else if(aContent.startsWith("table"))
                {
                    return Display_table;
                }
                else if(aContent.startsWith("inline-table"))
                {
                    return Display_inline_table;
                }
                else if(aContent.startsWith("table-row-group"))
                {
                    return Display_table_row_group;
                }
                else if(aContent.startsWith("table-header-group"))
                {
                    return Display_table_header_group;
                }
                else if(aContent.startsWith("table-footer-group"))
                {
                    return Display_table_footer_group;
                }
                else if(aContent.startsWith("table-row"))
                {
                    return Display_table_row;
                }
                else if(aContent.startsWith("table-column-group"))
                {
                    return Display_table_column_group;
                }
                else if(aContent.startsWith("table-column"))
                {
                    return Display_table_column;
                }
                else if(aContent.startsWith("table-cell"))
                {
                    return Display_table_cell;
                }
                else if(aContent.startsWith("table-caption"))
                {
                    return Display_table_caption;
                }
            }

            // return the default
            return Display_inline;
        }

        void SvgNode::parseAttribute(const OUString& /*rTokenName*/, SVGToken aSVGToken, const OUString& aContent)
        {
            switch(aSVGToken)
            {
                case SVGTokenId:
                {
                    if(!aContent.isEmpty())
                    {
                        setId(&aContent);
                    }
                    break;
                }
                case SVGTokenClass:
                {
                    if(!aContent.isEmpty())
                    {
                        setClass(&aContent);
                    }
                    break;
                }
                case SVGTokenXmlSpace:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("default"))
                        {
                            setXmlSpace(XmlSpace_default);
                        }
                        else if(aContent.startsWith("preserve"))
                        {
                            setXmlSpace(XmlSpace_preserve);
                        }
                    }
                    break;
                }
                case SVGTokenDisplay:
                {
                    if(aContent.getLength())
                    {
                        setDisplay(getDisplayFromContent(aContent));
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            if(Display_none == getDisplay())
            {
                return;
            }

            if(!bReferenced)
            {
                if(SVGTokenDefs == getType() ||
                    SVGTokenSymbol == getType() ||
                    SVGTokenClipPathNode == getType() ||
                    SVGTokenMask == getType() ||
                    SVGTokenMarker == getType() ||
                    SVGTokenPattern == getType())
                {
                    // do not decompose defs or symbol nodes (these hold only style-like
                    // objects which may be used by referencing them) except when doing
                    // so controlled referenced

                    // also do not decompose ClipPaths and Masks. These should be embedded
                    // in a defs node (which gets not decomposed by itself), but you never
                    // know

                    // also not directly used are Markers and Patterns, only indirecty used
                    // by reference

                    // #i121656# also do not decompose nodes which have display="none" set
                    // as property
                    return;
                }
            }

            const SvgNodeVector& rChildren = getChildren();

            if(!rChildren.empty())
            {
                const sal_uInt32 nCount(rChildren.size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    SvgNode* pCandidate = rChildren[a];

                    if(pCandidate && Display_none != pCandidate->getDisplay())
                    {
                        drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                        pCandidate->decomposeSvgNode(aNewTarget, bReferenced);

                        if(aNewTarget.hasElements())
                        {
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewTarget);
                        }
                    }
                    else
                    {
                        OSL_ENSURE(false, "Null-Pointer in child node list (!)");
                    }
                }

                if(rTarget.hasElements())
                {
                    const SvgStyleAttributes* pStyles = getSvgStyleAttributes();

                    if(pStyles)
                    {
                        // check if we have Title or Desc
                        const OUString& rTitle = pStyles->getTitle();
                        const OUString& rDesc = pStyles->getDesc();

                        if(!rTitle.isEmpty() || !rDesc.isEmpty())
                        {
                            // default object name is empty
                            OUString aObjectName;

                            // use path as object name when outmost element
                            if(SVGTokenSvg == getType())
                            {
                                aObjectName = getDocument().getAbsolutePath();

                                if(!aObjectName.isEmpty())
                                {
                                    INetURLObject aURL(aObjectName);

                                    aObjectName = aURL.getName(
                                        INetURLObject::LAST_SEGMENT,
                                        true,
                                        INetURLObject::DECODE_WITH_CHARSET);
                                }
                            }

                            // pack in ObjectInfoPrimitive2D group
                            const drawinglayer::primitive2d::Primitive2DReference xRef(
                                new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                                    rTarget,
                                    aObjectName,
                                    rTitle,
                                    rDesc));

                            rTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                        }
                    }
                }
            }
        }

        const basegfx::B2DRange SvgNode::getCurrentViewPort() const
        {
            if(getParent())
            {
                return getParent()->getCurrentViewPort();
            }
            else
            {
                return basegfx::B2DRange(); // return empty B2DRange
            }
        }

        double SvgNode::getCurrentFontSizeInherited() const
        {
            if(getParent())
            {
                return getParent()->getCurrentFontSize();
            }
            else
            {
                return 0.0;
            }
        }

        double SvgNode::getCurrentFontSize() const
        {
            if(getSvgStyleAttributes())
                return getSvgStyleAttributes()->getFontSize().solve(*this, xcoordinate);

            return getCurrentFontSizeInherited();
        }

        double SvgNode::getCurrentXHeightInherited() const
        {
            if(getParent())
            {
                return getParent()->getCurrentXHeight();
            }
            else
            {
                return 0.0;
            }
        }

        double SvgNode::getCurrentXHeight() const
        {
            if(getSvgStyleAttributes())
                // for XHeight, use FontSize currently
                return getSvgStyleAttributes()->getFontSize().solve(*this, ycoordinate);

            return getCurrentXHeightInherited();
        }

        void SvgNode::setId(const OUString* pfId)
        {
            if(mpId)
            {
                mrDocument.removeSvgNodeFromMapper(*mpId);
                delete mpId;
                mpId = 0;
            }

            if(pfId)
            {
                mpId = new OUString(*pfId);
                mrDocument.addSvgNodeToMapper(*mpId, *this);
            }
        }

        void SvgNode::setClass(const OUString* pfClass)
        {
            if(mpClass)
            {
                mrDocument.removeSvgNodeFromMapper(*mpClass);
                delete mpClass;
                mpClass = 0;
            }

            if(pfClass)
            {
                mpClass = new OUString(*pfClass);
                mrDocument.addSvgNodeToMapper(*mpClass, *this);
            }
        }

        XmlSpace SvgNode::getXmlSpace() const
        {
            if(maXmlSpace != XmlSpace_notset)
            {
                return maXmlSpace;
            }

            if(getParent())
            {
                return getParent()->getXmlSpace();
            }

            // default is XmlSpace_default
            return XmlSpace_default;
        }

    } // end of namespace svgreader
} // end of namespace svgio


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
