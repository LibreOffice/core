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

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
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

                    if(maCssStyleVector.empty() && rClassStr.getLength())
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

            if(maCssStyleVector.empty())
            {
                return &rOriginal;
            }
            else
            {
                // set CssStyleParent at maCssStyleVector members to hang them in front of
                // the existing style. Build a style chain, reset parent of original for security.
                // Repeated style requests should only be issued from sub-Text nodes and I'm not
                // sure if in-between text nodes may build other chains (should not happen). But
                // it's only a re-chaining with pointers (cheap), so allow to do it every time.
                SvgStyleAttributes* pCurrent = const_cast< SvgStyleAttributes* >(&rOriginal);
                pCurrent->setCssStyleParent(0);

                for(sal_uInt32 a(0); a < maCssStyleVector.size(); a++)
                {
                    SvgStyleAttributes* pCandidate = const_cast< SvgStyleAttributes* >(maCssStyleVector[maCssStyleVector.size() - a - 1]);

                    pCandidate->setCssStyleParent(pCurrent);
                    pCurrent = pCandidate;
                }

                return pCurrent;
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

            for(sal_uInt32 a(0); a < nAttributes; a++)
            {
                const OUString aTokenName(xAttribs->getNameByIndex(a));

                parseAttribute(aTokenName, StrToSVGToken(aTokenName), xAttribs->getValueByIndex(a));
            }
        }

        void SvgNode::parseAttribute(const OUString& /*rTokenName*/, SVGToken aSVGToken, const OUString& aContent)
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
                        static OUString aStrDefault(OUString::createFromAscii("default"));
                        static OUString aStrPreserve(OUString::createFromAscii("preserve"));

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
                default:
                {
                    break;
                }
            }
        }

        void SvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
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

                    if(pCandidate)
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

                        if(rTitle.getLength() || rDesc.getLength())
                        {
                            // default object name is empty
                            OUString aObjectName;

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

        const basegfx::B2DRange* SvgNode::getCurrentViewPort() const
        {
            if(getParent())
            {
                return getParent()->getCurrentViewPort();
            }
            else
            {
                return 0;
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
