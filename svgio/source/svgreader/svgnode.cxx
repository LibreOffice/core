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

        const SvgStyleAttributes* SvgNode::checkForCssStyle(const OUString& rClassStr, const SvgStyleAttributes& rOriginal) const
        {
            if(maCssStyleVector.empty()) // #120435# Evaluate for CSS styles only once, this cannot change
            {
                const SvgDocument& rDocument = getDocument();

                if(rDocument.hasSvgStyleAttributesById())
                {
                    // #i125293# If we have CssStyles we need to build a linked list of SvgStyleAttributes
                    // which represent this for the current object. There are various methods to
                    // specify CssStyles which need to be taken into account in a given order:
                    // - 'id' element
                    // - 'class' element(s)
                    // - type-dependent elements (e..g. 'rect' for all rect elements)
                    // - local direct attributes (rOriginal)
                    // - inherited attributes (up the hierarchy)
                    // The first three will be collected in maCssStyleVector for the current element
                    // (only once, this will not change) and be linked in the needed order using the
                    // get/setCssStyleParent at the SvgStyleAttributes which will be used preferred in
                    // member evaluation over the existing parent hierarchy

                    // check for 'id' references
                    if(getId())
                    {
                        // search for CSS style equal to Id
                        const SvgStyleAttributes* pNew = rDocument.findSvgStyleAttributesById(*getId());

                        if(pNew)
                        {
                            const_cast< SvgNode* >(this)->maCssStyleVector.push_back(pNew);
                        }
                    }

                    // check for 'class' references
                    if(getClass())
                    {
                        // find all referenced CSS styles, a list of entries is allowed
                        const OUString* pClassList = getClass();
                        const sal_Int32 nLen(pClassList->getLength());
                        sal_Int32 nPos(0);
                        const SvgStyleAttributes* pNew = 0;

                        skip_char(*pClassList, ' ', nPos, nLen);

                        while(nPos < nLen)
                        {
                            OUStringBuffer aTokenValue;

                            copyToLimiter(*pClassList, ' ', nPos, aTokenValue, nLen);
                            skip_char(*pClassList, ' ', nPos, nLen);

                            OUString aId(".");
                            const OUString aOUTokenValue(aTokenValue.makeStringAndClear());

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

                    // check for class-dependent references to CssStyles
                    if(rClassStr.getLength())
                    {
                        // search for CSS style equal to class type
                        const SvgStyleAttributes* pNew = rDocument.findSvgStyleAttributesById(rClassStr);

                        if(pNew)
                        {
                            const_cast< SvgNode* >(this)->maCssStyleVector.push_back(pNew);
                        }
                    }
                }
            }

            if(maCssStyleVector.empty())
            {
                // return original if no CssStlyes found
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
                // Resetting the CssStyleParent on rOriginal is probably not needeed
                // but simply safer to do.
                const_cast< SvgStyleAttributes& >(rOriginal).setCssStyleParent(0);

                // loop over the existing CssStyles and link them. There is a first one, take
                // as current
                SvgStyleAttributes* pCurrent = const_cast< SvgStyleAttributes* >(maCssStyleVector[0]);

                for(sal_uInt32 a(1); a < maCssStyleVector.size(); a++)
                {
                    SvgStyleAttributes* pNext = const_cast< SvgStyleAttributes* >(maCssStyleVector[a]);

                    pCurrent->setCssStyleParent(pNext);
                    pCurrent = pNext;
                }

                // pCurrent is the last used CssStyle, let it point to the original style
                pCurrent->setCssStyleParent(&rOriginal);

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
                const ::OUString aTokenName(xAttribs->getNameByIndex(nSVGTokenStyleIndex));

                parseAttribute(aTokenName, SVGTokenStyle, xAttribs->getValueByIndex(nSVGTokenStyleIndex));
            }
        }

        Display getDisplayFromContent(const OUString& aContent)
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
