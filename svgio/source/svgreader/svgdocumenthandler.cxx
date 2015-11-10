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

#include <svgio/svgreader/svgdocumenthandler.hxx>
#include <svgio/svgreader/svgtoken.hxx>
#include <svgio/svgreader/svgsvgnode.hxx>
#include <svgio/svgreader/svggnode.hxx>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgpathnode.hxx>
#include <svgio/svgreader/svgrectnode.hxx>
#include <svgio/svgreader/svggradientnode.hxx>
#include <svgio/svgreader/svggradientstopnode.hxx>
#include <svgio/svgreader/svgsymbolnode.hxx>
#include <svgio/svgreader/svgusenode.hxx>
#include <svgio/svgreader/svgcirclenode.hxx>
#include <svgio/svgreader/svgellipsenode.hxx>
#include <svgio/svgreader/svglinenode.hxx>
#include <svgio/svgreader/svgpolynode.hxx>
#include <svgio/svgreader/svgtextnode.hxx>
#include <svgio/svgreader/svgcharacternode.hxx>
#include <svgio/svgreader/svgtspannode.hxx>
#include <svgio/svgreader/svgtrefnode.hxx>
#include <svgio/svgreader/svgtextpathnode.hxx>
#include <svgio/svgreader/svgstylenode.hxx>
#include <svgio/svgreader/svgimagenode.hxx>
#include <svgio/svgreader/svgclippathnode.hxx>
#include <svgio/svgreader/svgmasknode.hxx>
#include <svgio/svgreader/svgmarkernode.hxx>
#include <svgio/svgreader/svgpatternnode.hxx>
#include <svgio/svgreader/svgtitledescnode.hxx>

using namespace com::sun::star;

namespace
{
    svgio::svgreader::SvgCharacterNode* whiteSpaceHandling(svgio::svgreader::SvgNode* pNode, svgio::svgreader::SvgCharacterNode* pLast)
    {
        if(pNode)
        {
            const svgio::svgreader::SvgNodeVector& rChilds = pNode->getChildren();
            const sal_uInt32 nCount(rChilds.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                svgio::svgreader::SvgNode* pCandidate = rChilds[a];

                if(pCandidate)
                {
                    switch(pCandidate->getType())
                    {
                        case svgio::svgreader::SVGTokenCharacter:
                        {
                            // clean whitespace in text span
                            svgio::svgreader::SvgCharacterNode* pCharNode = static_cast< svgio::svgreader::SvgCharacterNode* >(pCandidate);
                            pCharNode->whiteSpaceHandling();

                            // pCharNode may have lost all text. If that's the case, ignore
                            // as invalid character node
                            if(!pCharNode->getText().isEmpty())
                            {
                                if(pLast)
                                {
                                    bool bAddGap(true);
                                    static bool bNoGapsForBaselineShift(true);

                                    if(bNoGapsForBaselineShift)
                                    {
                                        // With this option a baseline shift between two char parts ('words')
                                        // will not add a space 'gap' to the end of the (non-last) word. This
                                        // seems to be the standard behaviour, see last bugdoc attached #122524#
                                        const svgio::svgreader::SvgStyleAttributes* pStyleLast = pLast->getSvgStyleAttributes();
                                        const svgio::svgreader::SvgStyleAttributes* pStyleCurrent = pCandidate->getSvgStyleAttributes();

                                        if(pStyleLast && pStyleCurrent && pStyleLast->getBaselineShift() != pStyleCurrent->getBaselineShift())
                                        {
                                            bAddGap = false;
                                        }
                                    }

                                    // add in-between whitespace (single space) to last
                                    // known character node
                                    if(bAddGap)
                                    {
                                        pLast->addGap();
                                    }
                                }

                                // remember new last corrected character node
                                pLast = pCharNode;
                            }
                            break;
                        }
                        case svgio::svgreader::SVGTokenTspan:
                        case svgio::svgreader::SVGTokenTextPath:
                        case svgio::svgreader::SVGTokenTref:
                        {
                            // recursively clean whitespaces in subhierarchy
                            pLast = whiteSpaceHandling(pCandidate, pLast);
                            break;
                        }
                        default:
                        {
                            OSL_ENSURE(false, "Unexpected token inside SVGTokenText (!)");
                            break;
                        }
                    }
                }
            }
        }

        return pLast;
    }
}



namespace svgio
{
    namespace svgreader
    {
        SvgDocHdl::SvgDocHdl(const OUString& aAbsolutePath)
        :   maDocument(aAbsolutePath),
            mpTarget(nullptr),
            maCssContents(),
            bSkip(false)
        {
        }

        SvgDocHdl::~SvgDocHdl()
        {
#ifdef DBG_UTIL
            if(mpTarget)
            {
                OSL_ENSURE(false, "SvgDocHdl destructed with active target (!)");
                delete mpTarget;
            }
            OSL_ENSURE(!maCssContents.size(), "SvgDocHdl destructed with active css style stack entry (!)");
#endif
        }

        void SvgDocHdl::startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
            OSL_ENSURE(!mpTarget, "Already a target at document start (!)");
            OSL_ENSURE(!maCssContents.size(), "SvgDocHdl startDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
            OSL_ENSURE(!mpTarget, "Still a target at document end (!)");
            OSL_ENSURE(!maCssContents.size(), "SvgDocHdl endDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
            if (bSkip)
                return;
            if(!aName.isEmpty())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName, false));

                switch(aSVGToken)
                {
                    /// structural elements
                    case SVGTokenSymbol:
                    {
                        /// new basic node for Symbol. Content gets scanned, but
                        /// will not be decomposed (see SvgNode::decomposeSvgNode and bReferenced)
                        mpTarget = new SvgSymbolNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenDefs:
                    case SVGTokenG:
                    {
                        /// new node for Defs/G
                        mpTarget = new SvgGNode(aSVGToken, maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenSvg:
                    {
                        /// new node for Svg
                        mpTarget = new SvgSvgNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenUse:
                    {
                        /// new node for Use
                        mpTarget = new SvgUseNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// shape elements
                    case SVGTokenCircle:
                    {
                        /// new node for Circle
                        mpTarget = new SvgCircleNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenEllipse:
                    {
                        /// new node for Ellipse
                        mpTarget = new SvgEllipseNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenLine:
                    {
                        /// new node for Line
                        mpTarget = new SvgLineNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPath:
                    {
                        /// new node for Path
                        mpTarget = new SvgPathNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPolygon:
                    {
                        /// new node for Polygon
                        mpTarget = new SvgPolyNode(maDocument, mpTarget, false);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPolyline:
                    {
                        /// new node for Polyline
                        mpTarget = new SvgPolyNode(maDocument, mpTarget, true);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenRect:
                    {
                        /// new node for Rect
                        mpTarget = new SvgRectNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenImage:
                    {
                        /// new node for Image
                        mpTarget = new SvgImageNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// title and description
                    case SVGTokenTitle:
                    case SVGTokenDesc:
                    {
                        /// new node for Title and/or Desc
                        mpTarget = new SvgTitleDescNode(aSVGToken, maDocument, mpTarget);
                        break;
                    }

                    /// gradients
                    case SVGTokenLinearGradient:
                    case SVGTokenRadialGradient:
                    {
                        mpTarget = new SvgGradientNode(aSVGToken, maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// gradient stops
                    case SVGTokenStop:
                    {
                        mpTarget = new SvgGradientStopNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// text
                    case SVGTokenText:
                    {
                        mpTarget = new SvgTextNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenTspan:
                    {
                        mpTarget = new SvgTspanNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenTref:
                    {
                        mpTarget = new SvgTrefNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenTextPath:
                    {
                        mpTarget = new SvgTextPathNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// styles (as stylesheets)
                    case SVGTokenStyle:
                    {
                        SvgStyleNode* pNew = new SvgStyleNode(maDocument, mpTarget);
                        mpTarget = pNew;
                        const sal_uInt32 nAttributes(xAttribs->getLength());

                        if(0 == nAttributes)
                        {
                            // #i125326# no attributes, thus also no type="text/css". This is allowed to be missing,
                            // thus do mark this style as CssStyle. This is required to read the contained
                            // text (which defines the css style)
                            pNew->setTextCss(true);
                        }
                        else
                        {
                            // #i125326# there are attributes, read them. This will set isTextCss to true if
                            // a type="text/css" is contained as exact match, else not
                            mpTarget->parseAttributes(xAttribs);
                        }

                        if(pNew->isTextCss())
                        {
                            // if it is a Css style, allow reading text between the start and end tag (see
                            // SvgDocHdl::characters for details)
                            maCssContents.push_back(OUString());
                        }
                        break;
                    }

                    /// structural elements clip-path and mask. Content gets scanned, but
                    /// will not be decomposed (see SvgNode::decomposeSvgNode and bReferenced)
                    case SVGTokenClipPathNode:
                    {
                        /// new node for ClipPath
                        mpTarget = new SvgClipPathNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenMask:
                    {
                        /// new node for Mask
                        mpTarget = new SvgMaskNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// structural element marker
                    case SVGTokenMarker:
                    {
                        /// new node for marker
                        mpTarget = new SvgMarkerNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    /// structural element pattern
                    case SVGTokenPattern:
                    {
                        /// new node for pattern
                        mpTarget = new SvgPatternNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    // ignore FlowRoot and child nodes
                    case SVGTokenFlowRoot:
                    {
                        bSkip = true;
                        break;
                    }

                    default:
                    {
                        /// invalid token, ignore
#ifdef DBG_UTIL
                        myAssert( "Unknown Base SvgToken <" + aName + "> (!)" );
#endif
                        break;
                    }
                }
            }
        }

        void SvgDocHdl::endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
            if(!aName.isEmpty())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName, false));
                SvgNode* pWhitespaceCheck(SVGTokenText == aSVGToken ? mpTarget : nullptr);
                SvgStyleNode* pCssStyle(SVGTokenStyle == aSVGToken ? static_cast< SvgStyleNode* >(mpTarget) : nullptr);
                SvgTitleDescNode* pSvgTitleDescNode(SVGTokenTitle == aSVGToken || SVGTokenDesc == aSVGToken ? static_cast< SvgTitleDescNode* >(mpTarget) : nullptr);

                // if we are in skipping mode and we reach the flowRoot end tag: stop skipping mode
                if(bSkip && aSVGToken == SVGTokenFlowRoot)
                    bSkip = false;
                // we are in skipping mode: do nothing until we found the flowRoot end tag
                else if(bSkip)
                    return;

                switch(aSVGToken)
                {
                    /// valid tokens for which a new one was created

                    /// structural elements
                    case SVGTokenDefs:
                    case SVGTokenG:
                    case SVGTokenSvg:
                    case SVGTokenSymbol:
                    case SVGTokenUse:

                    /// shape elements
                    case SVGTokenCircle:
                    case SVGTokenEllipse:
                    case SVGTokenLine:
                    case SVGTokenPath:
                    case SVGTokenPolygon:
                    case SVGTokenPolyline:
                    case SVGTokenRect:
                    case SVGTokenImage:

                    /// title and description
                    case SVGTokenTitle:
                    case SVGTokenDesc:

                    /// gradients
                    case SVGTokenLinearGradient:
                    case SVGTokenRadialGradient:

                    /// gradient stops
                    case SVGTokenStop:

                    /// text
                    case SVGTokenText:
                    case SVGTokenTspan:
                    case SVGTokenTextPath:
                    case SVGTokenTref:

                    /// styles (as stylesheets)
                    case SVGTokenStyle:

                    /// structural elements clip-path and mask
                    case SVGTokenClipPathNode:
                    case SVGTokenMask:

                    /// structural element marker
                    case SVGTokenMarker:

                    /// structural element pattern
                    case SVGTokenPattern:

                    /// content handling after parsing
                    {
                        if(mpTarget)
                        {
                            if(!mpTarget->getParent())
                            {
                                // last element closing, save this tree
                                maDocument.appendNode(mpTarget);
                            }

                            mpTarget = const_cast< SvgNode* >(mpTarget->getParent());
                        }
                        else
                        {
                            OSL_ENSURE(false, "Closing token, but no context (!)");
                        }
                        break;
                    }
                    default:
                    {
                        /// invalid token, ignore
                    }
                }

                if(pSvgTitleDescNode && mpTarget)
                {
                    const OUString aText(pSvgTitleDescNode->getText());

                    if(!aText.isEmpty())
                    {
                        if(SVGTokenTitle == aSVGToken)
                        {
                            mpTarget->parseAttribute(getStrTitle(), aSVGToken, aText);
                        }
                        else // if(SVGTokenDesc == aSVGToken)
                        {
                            mpTarget->parseAttribute(getStrDesc(), aSVGToken, aText);
                        }
                    }
                }

                if(pCssStyle && pCssStyle->isTextCss())
                {
                    // css style parsing
                    if(maCssContents.size())
                    {
                        // need to interpret css styles and remember them as StyleSheets
                        // #125325# Caution! the Css content may contain block comments
                        // (see http://www.w3.org/wiki/CSS_basics#CSS_comments). These need
                        // to be removed first
                        const OUString aCommentFreeSource(removeBlockComments(*(maCssContents.end() - 1)));

                        if(aCommentFreeSource.getLength())
                        {
                            pCssStyle->addCssStyleSheet(aCommentFreeSource);
                        }

                        maCssContents.pop_back();
                    }
                    else
                    {
                        OSL_ENSURE(false, "Closing CssStyle, but no collector string on stack (!)");
                    }
                }

                if(pWhitespaceCheck)
                {
                    // cleanup read strings
                    whiteSpaceHandling(pWhitespaceCheck, nullptr);
                }
            }
        }

        void SvgDocHdl::characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
            const sal_uInt32 nLength(aChars.getLength());

            if(mpTarget && nLength)
            {
                switch(mpTarget->getType())
                {
                    case SVGTokenText:
                    case SVGTokenTspan:
                    case SVGTokenTextPath:
                    {
                        const SvgNodeVector& rChilds = mpTarget->getChildren();
                        SvgCharacterNode* pTarget = nullptr;

                        if(rChilds.size())
                        {
                            pTarget = dynamic_cast< SvgCharacterNode* >(rChilds[rChilds.size() - 1]);
                        }

                        if(pTarget)
                        {
                            // concatenate to current character span
                            pTarget->concatenate(aChars);
                        }
                        else
                        {
                            // add character span as simplified tspan (no arguments)
                            // as direct child of SvgTextNode/SvgTspanNode/SvgTextPathNode
                            new SvgCharacterNode(maDocument, mpTarget, aChars);
                        }
                        break;
                    }
                    case SVGTokenStyle:
                    {
                        SvgStyleNode& rSvgStyleNode = static_cast< SvgStyleNode& >(*mpTarget);

                        if(rSvgStyleNode.isTextCss())
                        {
                            // collect characters for css style
                            if(maCssContents.size())
                            {
                                const OUString aTrimmedChars(aChars.trim());

                                if(!aTrimmedChars.isEmpty())
                                {
                                    std::vector< OUString >::iterator aString(maCssContents.end() - 1);
                                    (*aString) += aTrimmedChars;
                                }
                            }
                            else
                            {
                                OSL_ENSURE(false, "Closing CssStyle, but no collector string on stack (!)");
                            }
                        }
                        break;
                    }
                    case SVGTokenTitle:
                    case SVGTokenDesc:
                    {
                        SvgTitleDescNode& rSvgTitleDescNode = static_cast< SvgTitleDescNode& >(*mpTarget);

                        // add text directly to SvgTitleDescNode
                        rSvgTitleDescNode.concatenate(aChars);
                        break;
                    }
                    default:
                    {
                        // characters not used by a known node
                        break;
                    }
                }
            }
        }

        void SvgDocHdl::ignorableWhitespace(const OUString& /*aWhitespaces*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
        }

        void SvgDocHdl::processingInstruction(const OUString& /*aTarget*/, const OUString& /*aData*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
        }

        void SvgDocHdl::setDocumentLocator(const uno::Reference< xml::sax::XLocator >& /*xLocator*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
        {
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
