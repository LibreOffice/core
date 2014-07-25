/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svgio.hxx"

#include <svgio/svgreader/svgnode.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <tools/urlobj.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        /// #125258#
        bool SvgNode::supportsParentStyle() const
        {
            return true;
        }

        const SvgStyleAttributes* SvgNode::getSvgStyleAttributes() const
        {
            return 0;
        }

        void SvgNode::fillCssStyleVector(const rtl::OUString& rClassStr)
        {
            OSL_ENSURE(!mbCssStyleVectorBuilt, "OOps, fillCssStyleVector called double ?!?");
            mbCssStyleVectorBuilt = true;

            // #125293# If we have CssStyles we need to buuild a linked list of SvgStyleAttributes
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
                maCssStyleVector.push_back(mpLocalCssStyle);
            }

            const SvgDocument& rDocument = getDocument();

            if(rDocument.hasSvgStyleAttributesById())
            {
                // check for 'id' references
                if(getId())
                {
                    // concatenate combined style name during search for CSS style equal to Id
                    // when travelling over node parents
                    rtl::OUString aConcatenatedStyleName;
                    const SvgNode* pCurrent = this;
                    const SvgStyleAttributes* pNew = 0;

                    while(!pNew && pCurrent)
                    {
                        if(pCurrent->getId())
                        {
                            aConcatenatedStyleName = *pCurrent->getId() + aConcatenatedStyleName;
                        }

                        if(aConcatenatedStyleName.getLength())
                        {
                            pNew = rDocument.findSvgStyleAttributesById(aConcatenatedStyleName);
                        }

                        pCurrent = pCurrent->getParent();
                    }

                    if(pNew)
                    {
                        maCssStyleVector.push_back(pNew);
                    }
                }

                // check for 'class' references
                if(getClass())
                {
                    // find all referenced CSS styles (a list of entries is allowed)
                    const rtl::OUString* pClassList = getClass();
                    const sal_Int32 nLen(pClassList->getLength());
                    sal_Int32 nPos(0);
                    const SvgStyleAttributes* pNew = 0;

                    skip_char(*pClassList, sal_Unicode(' '), nPos, nLen);

                    while(nPos < nLen)
                    {
                        rtl::OUStringBuffer aTokenValue;

                        copyToLimiter(*pClassList, sal_Unicode(' '), nPos, aTokenValue, nLen);
                        skip_char(*pClassList, sal_Unicode(' '), nPos, nLen);

                        rtl::OUString aId(rtl::OUString::createFromAscii("."));
                        const rtl::OUString aOUTokenValue(aTokenValue.makeStringAndClear());

                        // look for CSS style common to token
                        aId = aId + aOUTokenValue;
                        pNew = rDocument.findSvgStyleAttributesById(aId);

                        if(!pNew && rClassStr.getLength())
                        {
                            // look for CSS style common to class.token
                            aId = rClassStr + aId;

                            pNew = rDocument.findSvgStyleAttributesById(aId);
                        }

                        if(pNew)
                        {
                            maCssStyleVector.push_back(pNew);
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
                        maCssStyleVector.push_back(pNew);
                    }
                }
            }
        }

        const SvgStyleAttributes* SvgNode::checkForCssStyle(const rtl::OUString& rClassStr, const SvgStyleAttributes& rOriginal) const
        {
            if(!mbCssStyleVectorBuilt)
            {
                // build needed CssStyleVector for local node
                const_cast< SvgNode* >(this)->fillCssStyleVector(rClassStr);
            }

            if(maCssStyleVector.empty())
            {
                // return given original if no CssStlyes found
                return &rOriginal;
            }
            else
            {
                // #125293# rOriginal will be the last element in the linked list; use no CssStyleParent
                // there (reset it) to ensure that the parent hierarchy will be used when it's base
                // is referenced. This new chaning inserts the CssStyles before the original style,
                // this makes the whole process much safer since the original style when used will
                // be not different to the situation without CssStyles; thus loops which may be caused
                // by trying to use the parent hierarchy of the owner of the style will be avoided
                // already in this mechanism. It's still good to keep the supportsParentStyle
                // from #125258# in place, though.
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
            maCssStyleVector(),
            mpLocalCssStyle(0),
            mbCssStyleVectorBuilt(false)
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

            if(mpId)
            {
                delete mpId;
            }

            if(mpClass)
            {
                delete mpClass;
            }

            if(mpLocalCssStyle)
            {
                delete mpLocalCssStyle;
            }
        }

        void SvgNode::readLocalCssStyle(const rtl::OUString& aContent)
        {
            if(!mpLocalCssStyle)
            {
                // create LocalCssStyle if needed but not yet added
                mpLocalCssStyle = new SvgStyleAttributes(*this);
            }
            else
            {
                // 2nd fill would be an error
                OSL_ENSURE(false, "Svg node has two local CssStyles, this may lead to problems (!)");
            }

            if(mpLocalCssStyle)
            {
                // parse and set values to it
                mpLocalCssStyle->readStyle(aContent);
            }
            else
            {
                OSL_ENSURE(false, "Could not get/create a local CssStyle for a node (!)");
            }
        }

        void SvgNode::parseAttributes(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs)
        {
            // no longer need to pre-sort moving 'style' entries to the back so that
            // values get overwritten - that was the previous, not complete solution for
            // handling the priorities between svg and Css properties
            const sal_uInt32 nAttributes(xAttribs->getLength());

            for(sal_uInt32 a(0); a < nAttributes; a++)
            {
                const ::rtl::OUString aTokenName(xAttribs->getNameByIndex(a));
                const SVGToken aSVGToken(StrToSVGToken(aTokenName));

                parseAttribute(aTokenName, aSVGToken, xAttribs->getValueByIndex(a));
            }
        }

        Display getDisplayFromContent(const rtl::OUString& aContent)
        {
            if(aContent.getLength())
            {
                static rtl::OUString aStrInline(rtl::OUString::createFromAscii("inline"));
                static rtl::OUString aStrBlock(rtl::OUString::createFromAscii("block"));
                static rtl::OUString aStrList_item(rtl::OUString::createFromAscii("list-item"));
                static rtl::OUString aStrRun_in(rtl::OUString::createFromAscii("run-in"));
                static rtl::OUString aStrCompact(rtl::OUString::createFromAscii("compact"));
                static rtl::OUString aStrMarker(rtl::OUString::createFromAscii("marker"));
                static rtl::OUString aStrTable(rtl::OUString::createFromAscii("table"));
                static rtl::OUString aStrInline_table(rtl::OUString::createFromAscii("inline-table"));
                static rtl::OUString aStrTable_row_group(rtl::OUString::createFromAscii("table-row-group"));
                static rtl::OUString aStrTable_header_group(rtl::OUString::createFromAscii("table-header-group"));
                static rtl::OUString aStrTable_footer_group(rtl::OUString::createFromAscii("table-footer-group"));
                static rtl::OUString aStrTable_row(rtl::OUString::createFromAscii("table-row"));
                static rtl::OUString aStrTable_column_group(rtl::OUString::createFromAscii("table-column-group"));
                static rtl::OUString aStrTable_column(rtl::OUString::createFromAscii("table-column"));
                static rtl::OUString aStrTable_cell(rtl::OUString::createFromAscii("table-cell"));
                static rtl::OUString aStrTable_caption(rtl::OUString::createFromAscii("table-caption"));
                static rtl::OUString aStrNone(rtl::OUString::createFromAscii("none"));
                static rtl::OUString aStrInherit(rtl::OUString::createFromAscii("inherit"));

                if(aContent.match(aStrInline))
                {
                    return Display_inline;
                }
                else if(aContent.match(aStrNone))
                {
                    return Display_none;
                }
                else if(aContent.match(aStrInherit))
                {
                    return Display_inherit;
                }
                else if(aContent.match(aStrBlock))
                {
                    return Display_block;
                }
                else if(aContent.match(aStrList_item))
                {
                    return Display_list_item;
                }
                else if(aContent.match(aStrRun_in))
                {
                    return Display_run_in;
                }
                else if(aContent.match(aStrCompact))
                {
                    return Display_compact;
                }
                else if(aContent.match(aStrMarker))
                {
                    return Display_marker;
                }
                else if(aContent.match(aStrTable))
                {
                    return Display_table;
                }
                else if(aContent.match(aStrInline_table))
                {
                    return Display_inline_table;
                }
                else if(aContent.match(aStrTable_row_group))
                {
                    return Display_table_row_group;
                }
                else if(aContent.match(aStrTable_header_group))
                {
                    return Display_table_header_group;
                }
                else if(aContent.match(aStrTable_footer_group))
                {
                    return Display_table_footer_group;
                }
                else if(aContent.match(aStrTable_row))
                {
                    return Display_table_row;
                }
                else if(aContent.match(aStrTable_column_group))
                {
                    return Display_table_column_group;
                }
                else if(aContent.match(aStrTable_column))
                {
                    return Display_table_column;
                }
                else if(aContent.match(aStrTable_cell))
                {
                    return Display_table_cell;
                }
                else if(aContent.match(aStrTable_caption))
                {
                    return Display_table_caption;
                }
            }

            // return the default
            return Display_inline;
        }

        void SvgNode::parseAttribute(const rtl::OUString& /*rTokenName*/, SVGToken aSVGToken, const rtl::OUString& aContent)
        {
            switch(aSVGToken)
            {
                case SVGTokenId:
                {
                    if(aContent.getLength())
                    {
                        setId(&aContent);
                    }
                    break;
                }
                case SVGTokenClass:
                {
                    if(aContent.getLength())
                    {
                        setClass(&aContent);
                    }
                    break;
                }
                case SVGTokenXmlSpace:
                {
                    if(aContent.getLength())
                    {
                        static rtl::OUString aStrDefault(rtl::OUString::createFromAscii("default"));
                        static rtl::OUString aStrPreserve(rtl::OUString::createFromAscii("preserve"));

                        if(aContent.match(aStrDefault))
                        {
                            setXmlSpace(XmlSpace_default);
                        }
                        else if(aContent.match(aStrPreserve))
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

                    // #121656# also do not decompose nodes which have display="none" set
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
                        const rtl::OUString& rTitle = pStyles->getTitle();
                        const rtl::OUString& rDesc = pStyles->getDesc();

                        if(rTitle.getLength() || rDesc.getLength())
                        {
                            // default object name is empty
                            rtl::OUString aObjectName;

                            // use path as object name when outmost element
                            if(SVGTokenSvg == getType())
                            {
                                aObjectName = getDocument().getAbsolutePath();

                                if(aObjectName.getLength())
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

        double SvgNode::getCurrentFontSize() const
        {
            if(getSvgStyleAttributes())
            {
                return getSvgStyleAttributes()->getFontSize().solve(*this, xcoordinate);
            }
            else if(getParent())
            {
                return getParent()->getCurrentFontSize();
            }
            else
            {
                return 0.0;
            }
        }

        double SvgNode::getCurrentXHeight() const
        {
            if(getSvgStyleAttributes())
            {
                // for XHeight, use FontSize currently
                return getSvgStyleAttributes()->getFontSize().solve(*this, ycoordinate);
            }
            else if(getParent())
            {
                return getParent()->getCurrentXHeight();
            }
            else
            {
                return 0.0;
            }
        }

        void SvgNode::setId(const rtl::OUString* pfId)
        {
            if(mpId)
            {
                mrDocument.removeSvgNodeFromMapper(*mpId);
                delete mpId;
                mpId = 0;
            }

            if(pfId)
            {
                mpId = new rtl::OUString(*pfId);
                mrDocument.addSvgNodeToMapper(*mpId, *this);
            }
        }

        void SvgNode::setClass(const rtl::OUString* pfClass)
        {
            if(mpClass)
            {
                mrDocument.removeSvgNodeFromMapper(*mpClass);
                delete mpClass;
                mpClass = 0;
            }

            if(pfClass)
            {
                mpClass = new rtl::OUString(*pfClass);
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

//////////////////////////////////////////////////////////////////////////////
// eof
