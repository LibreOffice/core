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

#include <svgdocument.hxx>
#include <svgnode.hxx>
#include <svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>


namespace svgio::svgreader
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

        void SvgNode::addCssStyle(const SvgDocument& rDocument, const OUString& aConcatenated)
        {
            const SvgStyleAttributes* pNew = rDocument.findGlobalCssStyleAttributes(aConcatenated);

            if(pNew)
            {
                // add CssStyle if found
                maCssStyleVector.push_back(pNew);
            }
        }

namespace {
        std::vector< OUString > parseClass(const SvgNode& rNode)
        {
            std::vector< OUString > aParts;

            // check for 'class' references (a list of entries is allowed)
            if(rNode.getClass())
            {
                const OUString& rClassList = *rNode.getClass();
                const sal_Int32 nLen(rClassList.getLength());

                if(nLen)
                {
                    sal_Int32 nPos(0);
                    OUStringBuffer aToken;

                    while(nPos < nLen)
                    {
                        const sal_Int32 nInitPos(nPos);
                        copyToLimiter(rClassList, u' ', nPos, aToken, nLen);
                        skip_char(rClassList, u' ', nPos, nLen);
                        const OUString aPart(o3tl::trim(aToken));
                        aToken.setLength(0);

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
                }
            }

            return aParts;
        }
} //namespace

        void SvgNode::fillCssStyleVectorUsingHierarchyAndSelectors(
            const SvgNode& rCurrent,
            std::u16string_view aConcatenated)
        {
            const SvgDocument& rDocument = getDocument();

            if(!rDocument.hasGlobalCssStyleAttributes())
                return;

            const SvgNode* pParent = rCurrent.getParent();
            OUString sCurrentType(SVGTokenToStr(rCurrent.getType()));

            // check for ID (highest priority)
            if(rCurrent.getId())
            {
                const OUString& rId = *rCurrent.getId();

                if(rId.getLength())
                {
                    const OUString aNewConcatenated("#" + rId + aConcatenated);
                    addCssStyle(rDocument, aNewConcatenated);

                    if(!sCurrentType.isEmpty())
                        addCssStyle(rDocument, sCurrentType + aNewConcatenated);

                    if(pParent)
                    {
                        // check for combined selectors at parent first so that higher specificity will be in front
                        fillCssStyleVectorUsingHierarchyAndSelectors(*pParent, aNewConcatenated);
                    }
                }
            }

            std::vector <OUString> aClasses = parseClass(rCurrent);
            for(const auto &aClass : aClasses)
            {
                const OUString aNewConcatenated("." + aClass + aConcatenated);
                addCssStyle(rDocument, aNewConcatenated);

                if(!sCurrentType.isEmpty())
                    addCssStyle(rDocument, sCurrentType + aNewConcatenated);

                if(pParent)
                {
                    // check for combined selectors at parent first so that higher specificity will be in front
                    fillCssStyleVectorUsingHierarchyAndSelectors(*pParent, aNewConcatenated);
                }
            }

            if(!sCurrentType.isEmpty())
            {
                const OUString aNewConcatenated(sCurrentType + aConcatenated);
                addCssStyle(rDocument, aNewConcatenated);
            }

            OUString sType(SVGTokenToStr(getType()));

            // check for class-dependent references to CssStyles
            if(sType.isEmpty())
                return;

            if(pParent)
            {
                // check for combined selectors at parent first so that higher specificity will be in front
                fillCssStyleVectorUsingHierarchyAndSelectors(*pParent, sType);
            }
        }

        void SvgNode::fillCssStyleVectorUsingParent(const SvgNode& rCurrent)
        {
            const SvgDocument& rDocument = getDocument();

            if(!rDocument.hasGlobalCssStyleAttributes())
                return;

            const SvgNode* pParent = rCurrent.getParent();

            if (!pParent)
                return;

            OUString sParentId;
            if (pParent->getId().has_value())
            {
                sParentId = pParent->getId().value();
            }
            std::vector <OUString> aParentClasses = parseClass(*pParent);
            OUString sParentType(SVGTokenToStr(pParent->getType()));

            if(rCurrent.getId())
            {
                const OUString& rId = *rCurrent.getId();

                if(!rId.isEmpty())
                {
                    if (!sParentId.isEmpty())
                    {
                        const OUString aConcatenated("#" + sParentId + ">#" + rId);
                        addCssStyle(rDocument, aConcatenated);
                    }

                    for(const auto &aParentClass : aParentClasses)
                    {
                        const OUString aConcatenated("." + aParentClass + ">#" + rId);
                        addCssStyle(rDocument, aConcatenated);
                    }

                    if (!sParentType.isEmpty())
                    {
                        const OUString aConcatenated(sParentType + ">#" + rId);
                        addCssStyle(rDocument, aConcatenated);
                    }
                }

            }

            std::vector <OUString> aClasses = parseClass(rCurrent);
            for(const auto &aClass : aClasses)
            {

                if (!sParentId.isEmpty())
                {
                    const OUString aConcatenated("#" + sParentId + ">." + aClass);
                    addCssStyle(rDocument, aConcatenated);
                }

                for(const auto &aParentClass : aParentClasses)
                {
                    const OUString aConcatenated("." + aParentClass + ">." + aClass);
                    addCssStyle(rDocument, aConcatenated);
                }

                if (!sParentType.isEmpty())
                {
                    const OUString aConcatenated(sParentType + ">." + aClass);
                    addCssStyle(rDocument, aConcatenated);
                }
            }

            OUString sCurrentType(SVGTokenToStr(getType()));

            if(!sCurrentType.isEmpty())
            {
                if (!sParentId.isEmpty())
                {
                    const OUString aConcatenated("#" + sParentId + ">" + sCurrentType);
                    addCssStyle(rDocument, aConcatenated);
                }

                for(const auto &aParentClass : aParentClasses)
                {
                    const OUString aConcatenated("." + aParentClass + ">" + sCurrentType);
                    addCssStyle(rDocument, aConcatenated);
                }

                if (!sParentType.isEmpty())
                {
                    const OUString aConcatenated(sParentType + ">" + sCurrentType);
                    addCssStyle(rDocument, aConcatenated);
                }
            }
        }

        void SvgNode::fillCssStyleVector(const SvgStyleAttributes& rOriginal)
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
            // - Css selector '*'
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

            // tdf#156038 check for child combinator
            fillCssStyleVectorUsingParent(*this);

            // check the hierarchy for concatenated patterns of Selectors
            fillCssStyleVectorUsingHierarchyAndSelectors(*this, std::u16string_view());


            // tdf#99115, Add css selector '*' style only if the element is on top of the hierarchy
            // meaning its parent is <svg>
            const SvgNode* pParent = this->getParent();

            if(pParent && pParent->getType() == SVGToken::Svg)
            {
                // #i125329# find Css selector '*', add as last element if found
                const SvgStyleAttributes* pNew = getDocument().findGlobalCssStyleAttributes("*");

                if(pNew)
                {
                    // add CssStyle for selector '*' if found
                    maCssStyleVector.push_back(pNew);
                }
            }

            //local attributes
            maCssStyleVector.push_back(&rOriginal);
        }

        const SvgStyleAttributes* SvgNode::checkForCssStyle(const SvgStyleAttributes& rOriginal) const
        {
            if(!mbCssStyleVectorBuilt)
            {
                // build needed CssStyleVector for local node
                const_cast< SvgNode* >(this)->fillCssStyleVector(rOriginal);
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
            maXmlSpace(XmlSpace::NotSet),
            maDisplay(maType == SVGToken::Unknown ? Display::None : Display::Inline), // tdf#150124: do not display unknown nodes
            mbDecomposing(false),
            mbCssStyleVectorBuilt(false)
        {
            if (pParent)
            {
                pParent->maChildren.emplace_back(this);
            }
        }

        SvgNode::~SvgNode()
        {
        }

        void SvgNode::readLocalCssStyle(std::u16string_view aContent)
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

                parseAttribute(aSVGToken, xAttribs->getValueByIndex(a));
            }
        }

        Display getDisplayFromContent(std::u16string_view aContent)
        {
            if(!aContent.empty())
            {
                if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"inline"))
                {
                    return Display::Inline;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"none"))
                {
                    return Display::None;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"inherit"))
                {
                    return Display::Inherit;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"block"))
                {
                    return Display::Block;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"list-item"))
                {
                    return Display::ListItem;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"run-in"))
                {
                    return Display::RunIn;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"compact"))
                {
                    return Display::Compact;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"marker"))
                {
                    return Display::Marker;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table"))
                {
                    return Display::Table;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"inline-table"))
                {
                    return Display::InlineTable;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-row-group"))
                {
                    return Display::TableRowGroup;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-header-group"))
                {
                    return Display::TableHeaderGroup;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-footer-group"))
                {
                    return Display::TableFooterGroup;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-row"))
                {
                    return Display::TableRow;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-column-group"))
                {
                    return Display::TableColumnGroup;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-column"))
                {
                    return Display::TableColumn;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-cell"))
                {
                    return Display::TableCell;
                }
                else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"table-caption"))
                {
                    return Display::TableCaption;
                }
            }

            // return the default
            return Display::Inline;
        }

        void SvgNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
        {
            switch(aSVGToken)
            {
                case SVGToken::Id:
                {
                    if(!aContent.isEmpty())
                    {
                        setId(aContent);
                    }
                    break;
                }
                case SVGToken::Class:
                {
                    if(!aContent.isEmpty())
                    {
                        setClass(aContent);
                    }
                    break;
                }
                case SVGToken::SystemLanguage:
                {
                    if(!aContent.isEmpty())
                    {
                        setSystemLanguage(aContent);
                    }
                    break;
                }
                case SVGToken::XmlSpace:
                {
                    if(!aContent.isEmpty())
                    {
                        if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"default"))
                        {
                            setXmlSpace(XmlSpace::Default);
                        }
                        else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"preserve"))
                        {
                            setXmlSpace(XmlSpace::Preserve);
                        }
                    }
                    break;
                }
                case SVGToken::Display:
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

            if(Display::None == getDisplay())
            {
                return;
            }

            if(!bReferenced)
            {
                if(SVGToken::Defs == getType() ||
                    SVGToken::Symbol == getType() ||
                    SVGToken::ClipPathNode == getType() ||
                    SVGToken::Mask == getType() ||
                    SVGToken::Marker == getType() ||
                    SVGToken::Pattern == getType())
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

                if(pCandidate && Display::None != pCandidate->getDisplay())
                {
                    const auto& rGrandChildren = pCandidate->getChildren();
                    const SvgStyleAttributes* pChildStyles = pCandidate->getSvgStyleAttributes();
                    // decompose:
                    // - visible terminal nodes
                    // - all non-terminal nodes (might contain visible nodes down the hierarchy)
                    if( !rGrandChildren.empty() || ( pChildStyles && (Visibility::visible == pChildStyles->getVisibility())) )
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
                        if (SVGToken::Svg == getType())
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
                        drawinglayer::primitive2d::Primitive2DReference xRef(
                            new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                                std::move(rTarget),
                                aObjectName,
                                rTitle,
                                rDesc));

                        rTarget = drawinglayer::primitive2d::Primitive2DContainer { xRef };
                    }
                }
            }
            mbDecomposing = false;
        }

        basegfx::B2DRange SvgNode::getCurrentViewPort() const
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
                return getSvgStyleAttributes()->getFontSizeNumber().solve(*this, NumberType::xcoordinate);

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
                return getSvgStyleAttributes()->getFontSizeNumber().solve(*this, NumberType::ycoordinate);

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

        void SvgNode::setSystemLanguage(OUString const & rSystemClass)
        {
            const sal_Int32 nLen(rSystemClass.getLength());
            sal_Int32 nPos(0);
            OUStringBuffer aToken;

            // split into single tokens (currently only comma separator)
            while(nPos < nLen)
            {
                const sal_Int32 nInitPos(nPos);
                copyToLimiter(rSystemClass, u',', nPos, aToken, nLen);
                skip_char(rSystemClass, u',', nPos, nLen);
                const OUString aLang(o3tl::trim(aToken));
                aToken.setLength(0);

                if(!aLang.isEmpty())
                {
                    maSystemLanguage.push_back(aLang);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }
        }

        XmlSpace SvgNode::getXmlSpace() const
        {
            if(maXmlSpace != XmlSpace::NotSet)
            {
                return maXmlSpace;
            }

            if(getParent())
            {
                return getParent()->getXmlSpace();
            }

            // default is XmlSpace::Default
            return XmlSpace::Default;
        }

        void SvgNode::accept(Visitor & rVisitor)
        {
            rVisitor.visit(*this);
        }
} // end of namespace svgio


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
