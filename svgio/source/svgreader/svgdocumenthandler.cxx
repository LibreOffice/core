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

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

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
                            if(pCharNode->getText().getLength())
                            {
                                if(pLast)
                                {
                                    // add in-between whitespace (single space) to last
                                    // known character node
                                    pLast->addGap();
                                }

                                // remember new last corected character node
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

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgDocHdl::SvgDocHdl(const rtl::OUString& aAbsolutePath)
        :   maDocument(aAbsolutePath),
            mpTarget(0),
            maCssContents()
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

        void SvgDocHdl::startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            OSL_ENSURE(!mpTarget, "Already a target at document start (!)");
            OSL_ENSURE(!maCssContents.size(), "SvgDocHdl startDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            OSL_ENSURE(!mpTarget, "Still a target at document end (!)");
            OSL_ENSURE(!maCssContents.size(), "SvgDocHdl endDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            if(aName.getLength())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName));

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
                        mpTarget->parseAttributes(xAttribs);

                        if(pNew->isTextCss())
                        {
                            maCssContents.push_back(rtl::OUString());
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

                    default:
                    {
                        /// invalid token, ignore
#ifdef DBG_UTIL
                        myAssert(
                            rtl::OUString::createFromAscii("Unknown Base SvgToken <") +
                            aName +
                            rtl::OUString::createFromAscii("> (!)"));
#endif
                        break;
                    }
                }
            }
        }

        void SvgDocHdl::endElement( const ::rtl::OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            if(aName.getLength())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName));
                SvgNode* pWhitespaceCheck(SVGTokenText == aSVGToken ? mpTarget : 0);
                SvgStyleNode* pCssStyle(SVGTokenStyle == aSVGToken ? static_cast< SvgStyleNode* >(mpTarget) : 0);
                SvgTitleDescNode* pSvgTitleDescNode(SVGTokenTitle == aSVGToken || SVGTokenDesc == aSVGToken ? static_cast< SvgTitleDescNode* >(mpTarget) : 0);

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
                    const rtl::OUString aText(pSvgTitleDescNode->getText());

                    if(aText.getLength())
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
                        pCssStyle->addCssStyleSheet(*(maCssContents.end() - 1));
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
                    whiteSpaceHandling(pWhitespaceCheck, 0);
                }
            }
        }

        void SvgDocHdl::characters( const ::rtl::OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException)
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
                        SvgCharacterNode* pTarget = 0;

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
                                const ::rtl::OUString aTrimmedChars(aChars.trim());

                                if(aTrimmedChars.getLength())
                                {
                                    std::vector< rtl::OUString >::iterator aString(maCssContents.end() - 1);
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

        void SvgDocHdl::ignorableWhitespace(const ::rtl::OUString& /*aWhitespaces*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }

        void SvgDocHdl::processingInstruction(const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }

        void SvgDocHdl::setDocumentLocator(const uno::Reference< xml::sax::XLocator >& /*xLocator*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
