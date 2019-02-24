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
#include <svgdocument.hxx>
#include <svgnode.hxx>
#include <svgstyleattributes.hxx>
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
            return nullptr;
        }

        void SvgNode::fillCssStyleVectorUsingHierarchyAndSelectors(
            const OUString& rClassStr,
            const SvgNode& rCurrent,
            const OUString& aConcatenated)
        {
            const SvgDocument& rDocument = getDocument();

            if(!rDocument.hasGlobalCssStyleAttributes())
                return;

            const SvgNode* pParent = rCurrent.getParent();

            // check for ID (highest priority)
            if(rCurrent.getId())
            {
                const OUString& rId = *rCurrent.getId();

                if(rId.getLength())
                {
                    const OUString aNewConcatenated(
                        "#" + rId + aConcatenated);

                    if(pParent)
                    {
                        // check for combined selectors at parent firstso that higher specificity will be in front
                        fillCssStyleVectorUsingHierarchyAndSelectors(rClassStr, *pParent, aNewConcatenated);
                    }

                    const SvgStyleAttributes* pNew = rDocument.findGlobalCssStyleAttributes(aNewConcatenated);

                    if(pNew)
                    {
                        // add CssStyle if found
                        maCssStyleVector.push_back(pNew);
                    }
                }
            }

            // check for 'class' references (a list of entries is allowed)
            if(rCurrent.getClass())
            {
                const OUString& rClassList = *rCurrent.getClass();
                const sal_Int32 nLen(rClassList.getLength());

                if(nLen)
                {
                    std::vector< OUString > aParts;
                    sal_Int32 nPos(0);
                    OUStringBuffer aToken;

                    while(nPos < nLen)
                    {
                        const sal_Int32 nInitPos(nPos);
                        copyToLimiter(rClassList, u' ', nPos, aToken, nLen);
                        skip_char(rClassList, u' ', nPos, nLen);
                        const OUString aPart(aToken.makeStringAndClear().trim());

                        if(aPart.getLength())
                        {
                            aParts.push_back(aPart);
                        }

                        if(nInitPos == nPos)
                        {
                            OSL_ENSURE(false, "Could not interpret on current position (!)");
                            nPos++;
                        }
                    }

                    for(size_t a(0); a < aParts.size(); a++)
                    {
                        const OUString aNewConcatenated(
                            "." + aParts[a] + aConcatenated);

                        if(pParent)
                        {
                            // check for combined selectors at parent firstso that higher specificity will be in front
                            fillCssStyleVectorUsingHierarchyAndSelectors(rClassStr, *pParent, aNewConcatenated);
                        }

                        const SvgStyleAttributes* pNew = rDocument.findGlobalCssStyleAttributes(aNewConcatenated);

                        if(pNew)
                        {
                            // add CssStyle if found
                            maCssStyleVector.push_back(pNew);
                        }
                    }
                }
            }

            // check for class-dependent references to CssStyles
            if(rClassStr.isEmpty())
                return;

            OUString aNewConcatenated(aConcatenated);

            if(!rCurrent.getId() && !rCurrent.getClass() && 0 == aConcatenated.indexOf(rClassStr))
            {
                // no new CssStyle Selector and already starts with rClassStr, do not concatenate;
                // we pass an 'empty' node (in the sense of CssStyle Selector)
            }
            else
            {
                aNewConcatenated = rClassStr + aConcatenated;
            }

            if(pParent)
            {
                // check for combined selectors at parent firstso that higher specificity will be in front
                fillCssStyleVectorUsingHierarchyAndSelectors(rClassStr, *pParent, aNewConcatenated);
            }

            const SvgStyleAttributes* pNew = rDocument.findGlobalCssStyleAttributes(aNewConcatenated);

            if(pNew)
            {
                // add CssStyle if found
                maCssStyleVector.push_back(pNew);
            }
        }

        void SvgNode::fillCssStyleVector(const OUString& rClassStr, const SvgStyleAttributes& rOriginal)
        {
            OSL_ENSURE(!mbCssStyleVectorBuilt, "OOps, fillCssStyleVector called double ?!?");
            mbCssStyleVectorBuilt = true;

            // #i125293# If we have CssStyles we need to build a linked list of SvgStyleAttributes
            // which represent this for the current object. There are various methods to
            // specify CssStyles which need to be taken into account in a given order:
            // - local CssStyle (independent from global CssStyles at SvgDocument)
            // - 'id' CssStyle
            // - 'class' CssStyle(s)
            // - type-dependent elements (e..g. 'rect' for all rect elements)
            // - local attributes (rOriginal)
            // - inherited attributes (up the hierarchy)
            // The first four will be collected in maCssStyleVector for the current element
            // (once, this will not change) and be linked in the needed order using the
            // get/setCssStyleParent at the SvgStyleAttributes which will be used preferred in
            // member evaluation over the existing parent hierarchy

            // check for local CssStyle with highest priority
            if(mpLocalCssStyle)
            {
                // if we have one, use as first entry
                maCssStyleVector.push_back(mpLocalCssStyle.get());
            }

            // check the hierarchy for concatenated patterns of Selectors
            fillCssStyleVectorUsingHierarchyAndSelectors(rClassStr, *this, OUString());

            // #i125329# find Css selector '*', add as last element if found
            const SvgStyleAttributes* pNew = getDocument().findGlobalCssStyleAttributes("*");

            if(pNew)
            {
                // add CssStyle for selector '*' if found
                maCssStyleVector.push_back(pNew);
            }

            //local attributes
            maCssStyleVector.push_back(&rOriginal);
        }

        const SvgStyleAttributes* SvgNode::checkForCssStyle(const OUString& rClassStr, const SvgStyleAttributes& rOriginal) const
        {
            if(!mbCssStyleVectorBuilt)
            {
                // build needed CssStyleVector for local node
                const_cast< SvgNode* >(this)->fillCssStyleVector(rClassStr, rOriginal);
            }

            if(maCssStyleVector.empty())
            {
                // return given original if no CssStyles found
                return &rOriginal;
            }
            else
            {
                // #i125293# rOriginal will be the last element in the linked list; use no CssStyleParent
                // there (reset it) to ensure that the parent hierarchy will be used when it's base
                // is referenced. This new chaining inserts the CssStyles before the original style,
                // this makes the whole process much safer since the original style when used will
                // be not different to the situation without CssStyles; thus loops which may be caused
                // by trying to use the parent hierarchy of the owner of the style will be avoided
                // already in this mechanism. It's still good to keep the supportsParentStyle
                // from #i125258# in place, though.
                // This chain building using pointers will be done every time when checkForCssStyle
                // is used (not the search, only the chaining). This is needed since the CssStyles
                // themselves will be potentially used multiple times. It is not expensive since it's
                // only changing some pointers.
                // The alternative would be to create the style hierarchy for every element (or even
                // for the element containing the hierarchy) in a vector of pointers and to use that.
                // Resetting the CssStyleParent on rOriginal is probably not needed
                // but simply safer to do.

                // loop over the existing CssStyles and link them. There is a first one, take
                // as current
                SvgStyleAttributes* pCurrent = const_cast< SvgStyleAttributes* >(maCssStyleVector[0]);

                for(size_t a(1); a < maCssStyleVector.size(); a++)
                {
                    SvgStyleAttributes* pNext = const_cast< SvgStyleAttributes* >(maCssStyleVector[a]);

                    pCurrent->setCssStyleParent(pNext);
                    pCurrent = pNext;
                }

                // return 1st CssStyle as style chain start element (only for the
                // local element, still no hierarchy used here)
                return maCssStyleVector[0];
            }
        }

        SvgNode::SvgNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   maType(aType),
            mrDocument(rDocument),
            mpParent(pParent),
            mpAlternativeParent(nullptr),
            maChildren(),
            maXmlSpace(XmlSpace_notset),
            maDisplay(Display_inline),
            maCssStyleVector(),
            mbDecomposing(false),
            mbCssStyleVectorBuilt(false)
        {
            OSL_ENSURE(SVGTokenUnknown != maType, "SvgNode with unknown type created (!)");

            if(pParent)
            {
                pParent->maChildren.emplace_back(this);
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
        }

        void SvgNode::readLocalCssStyle(const OUString& aContent)
        {
            if(!mpLocalCssStyle)
            {
                // create LocalCssStyle if needed but not yet added
                mpLocalCssStyle.reset(new SvgStyleAttributes(*this));
            }
            else
            {
                // 2nd fill would be an error
                OSL_ENSURE(false, "Svg node has two local CssStyles, this may lead to problems (!)");
            }

            if(mpLocalCssStyle)
            {
                // parse and set values to it
                mpLocalCssStyle->readCssStyle(aContent);
            }
            else
            {
                OSL_ENSURE(false, "Could not get/create a local CssStyle for a node (!)");
            }
        }

        void SvgNode::parseAttributes(const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs)
        {
            // no longer need to pre-sort moving 'style' entries to the back so that
            // values get overwritten - that was the previous, not complete solution for
            // handling the priorities between svg and Css properties
            const sal_uInt32 nAttributes(xAttribs->getLength());

            for(sal_uInt32 a(0); a < nAttributes; a++)
            {
                const OUString aTokenName(xAttribs->getNameByIndex(a));
                const SVGToken aSVGToken(StrToSVGToken(aTokenName, false));

                parseAttribute(aTokenName, aSVGToken, xAttribs->getValueByIndex(a));
            }
        }

        Display getDisplayFromContent(const OUString& aContent)
        {
            if(!aContent.isEmpty())
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
                        setId(aContent);
                    }
                    break;
                }
                case SVGTokenClass:
                {
                    if(!aContent.isEmpty())
                    {
                        setClass(aContent);
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
                    if(!aContent.isEmpty())
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

        void SvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            if (mbDecomposing) //guard against infinite recurse
                return;

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

                    // also not directly used are Markers and Patterns, only indirectly used
                    // by reference

                    // #i121656# also do not decompose nodes which have display="none" set
                    // as property
                    return;
                }
            }

            const auto& rChildren = getChildren();

            if(rChildren.empty())
                return;

            mbDecomposing = true;

            const sal_uInt32 nCount(rChildren.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                SvgNode* pCandidate = rChildren[a].get();

                if(pCandidate && Display_none != pCandidate->getDisplay())
                {
                    const auto& rGrandChildren = pCandidate->getChildren();
                    const SvgStyleAttributes* pChildStyles = pCandidate->getSvgStyleAttributes();
                    // decompose:
                    // - visible terminal nodes
                    // - all non-terminal nodes (might contain visible nodes down the hierarchy)
                    if( !rGrandChildren.empty() || ( pChildStyles && (Visibility_visible == pChildStyles->getVisibility())) )
                    {
                        drawinglayer::primitive2d::Primitive2DContainer aNewTarget;
                        pCandidate->decomposeSvgNode(aNewTarget, bReferenced);

                        if(!aNewTarget.empty())
                        {
                            rTarget.append(aNewTarget);
                        }
                    }
                }
                else if(!pCandidate)
                {
                    OSL_ENSURE(false, "Null-Pointer in child node list (!)");
                }
            }

            if(!rTarget.empty())
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
                                    INetURLObject::DecodeMechanism::WithCharset);
                            }
                        }

                        // pack in ObjectInfoPrimitive2D group
                        const drawinglayer::primitive2d::Primitive2DReference xRef(
                            new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                                rTarget,
                                aObjectName,
                                rTitle,
                                rDesc));

                        rTarget = drawinglayer::primitive2d::Primitive2DContainer { xRef };
                    }
                }
            }
            mbDecomposing = false;
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
                return getSvgStyleAttributes()->getFontSizeNumber().solve(*this, xcoordinate);

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
                return getSvgStyleAttributes()->getFontSizeNumber().solve(*this, ycoordinate);

            return getCurrentXHeightInherited();
        }

        void SvgNode::setId(OUString const & rId)
        {
            if(mpId)
            {
                mrDocument.removeSvgNodeFromMapper(*mpId);
                mpId.reset();
            }

            mpId = rId;
            mrDocument.addSvgNodeToMapper(*mpId, *this);
        }

        void SvgNode::setClass(OUString const & rClass)
        {
            if(mpClass)
            {
                mrDocument.removeSvgNodeFromMapper(*mpClass);
                mpClass.reset();
            }

            mpClass = rClass;
            mrDocument.addSvgNodeToMapper(*mpClass, *this);
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

        void SvgNode::accept(Visitor & rVisitor)
        {
            rVisitor.visit(*this);
        }
    } // end of namespace svgreader
} // end of namespace svgio


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
