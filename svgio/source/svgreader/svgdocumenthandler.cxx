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

#include <svgdocumenthandler.hxx>
#include <svgtoken.hxx>
#include <svgsvgnode.hxx>
#include <svggnode.hxx>
#include <svganode.hxx>
#include <svgnode.hxx>
#include <svgpathnode.hxx>
#include <svgrectnode.hxx>
#include <svggradientnode.hxx>
#include <svggradientstopnode.hxx>
#include <svgswitchnode.hxx>
#include <svgsymbolnode.hxx>
#include <svgusenode.hxx>
#include <svgcirclenode.hxx>
#include <svgellipsenode.hxx>
#include <svglinenode.hxx>
#include <svgpolynode.hxx>
#include <svgtextnode.hxx>
#include <svgcharacternode.hxx>
#include <svgtspannode.hxx>
#include <svgtrefnode.hxx>
#include <svgtextpathnode.hxx>
#include <svgstylenode.hxx>
#include <svgimagenode.hxx>
#include <svgclippathnode.hxx>
#include <svgfecolormatrixnode.hxx>
#include <svgfedropshadownode.hxx>
#include <svgfefloodnode.hxx>
#include <svgfeimagenode.hxx>
#include <svgfegaussianblurnode.hxx>
#include <svgfeoffsetnode.hxx>
#include <svgfilternode.hxx>
#include <svgmasknode.hxx>
#include <svgmarkernode.hxx>
#include <svgpatternnode.hxx>
#include <svgtitledescnode.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/lang/Locale.hpp>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>

using namespace com::sun::star;

namespace svgio::svgreader
{

namespace
{
    svgio::svgreader::SvgCharacterNode* whiteSpaceHandling(svgio::svgreader::SvgNode const * pNode, svgio::svgreader::SvgTspanNode* pParentLine, svgio::svgreader::SvgCharacterNode* pLast)
    {
        if(pNode)
        {
            const auto& rChilds = pNode->getChildren();
            const sal_uInt32 nCount(rChilds.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                svgio::svgreader::SvgNode* pCandidate = rChilds[a].get();

                if(pCandidate)
                {
                    switch(pCandidate->getType())
                    {
                        case SVGToken::Character:
                        {
                            // clean whitespace in text span
                            svgio::svgreader::SvgCharacterNode* pCharNode = static_cast< svgio::svgreader::SvgCharacterNode* >(pCandidate);

                            pCharNode->setParentLine(pParentLine);

                            pCharNode->whiteSpaceHandling();
                            pLast = pCharNode->addGap(pLast);

                            double fTextWidth(0.0);

                            const SvgStyleAttributes* pSvgStyleAttributes = pCharNode->getSvgStyleAttributes();

                            if(pSvgStyleAttributes)
                            {
                                const drawinglayer::attribute::FontAttribute aFontAttribute(
                                        svgio::svgreader::SvgCharacterNode::getFontAttribute(*pSvgStyleAttributes));

                                double fFontWidth(pSvgStyleAttributes->getFontSizeNumber().solve(*pCharNode));
                                double fFontHeight(fFontWidth);

                                css::lang::Locale aLocale;

                                drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                                aTextLayouterDevice.setFontAttribute(aFontAttribute, fFontWidth, fFontHeight, aLocale);
                                fTextWidth = aTextLayouterDevice.getTextWidth(pCharNode->getText(), 0.0, pCharNode->getText().getLength());
                            }

                            pParentLine->concatenateTextLineWidth(fTextWidth);
                            break;
                        }
                        case SVGToken::Tspan:
                        {
                            svgio::svgreader::SvgTspanNode* pTspanNode = static_cast< svgio::svgreader::SvgTspanNode* >(pCandidate);

                            // If x or y exist it means it's a new line of text
                            if(!pTspanNode->getX().empty() || !pTspanNode->getY().empty())
                                pParentLine = pTspanNode;

                            // recursively clean whitespaces in subhierarchy
                            pLast = whiteSpaceHandling(pCandidate, pParentLine, pLast);
                            break;
                        }
                        case SVGToken::TextPath:
                        case SVGToken::Tref:
                        {
                            // recursively clean whitespaces in subhierarchy
                            pLast = whiteSpaceHandling(pCandidate, pParentLine, pLast);
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

} // end anonymous namespace

        SvgDocHdl::SvgDocHdl(const OUString& aAbsolutePath)
        :   maDocument(aAbsolutePath),
            mpTarget(nullptr)
        {
        }

        SvgDocHdl::~SvgDocHdl()
        {
            if (mpTarget)
            {
                OSL_ENSURE(false, "SvgDocHdl destructed with active target (!)");

                while (mpTarget->getParent())
                    mpTarget = const_cast< SvgNode* >(mpTarget->getParent());

                const SvgNodeVector& rOwnedTopLevels = maDocument.getSvgNodeVector();
                if (std::none_of(rOwnedTopLevels.begin(), rOwnedTopLevels.end(),
                                [&](std::unique_ptr<SvgNode> const & p) { return p.get() == mpTarget; }))
                    delete mpTarget;
            }
            OSL_ENSURE(maCssContents.empty(), "SvgDocHdl destructed with active css style stack entry (!)");
        }

        void SvgDocHdl::startDocument(  )
        {
            OSL_ENSURE(!mpTarget, "Already a target at document start (!)");
            OSL_ENSURE(maCssContents.empty(), "SvgDocHdl startDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::endDocument(  )
        {
            OSL_ENSURE(!mpTarget, "Still a target at document end (!)");
            OSL_ENSURE(maCssContents.empty(), "SvgDocHdl endDocument with active css style stack entry (!)");
        }

        void SvgDocHdl::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        {
            if(aName.isEmpty())
                return;

            const SVGToken aSVGToken(StrToSVGToken(aName, false));

            switch (aSVGToken)
            {
                /// structural elements
                case SVGToken::Symbol:
                {
                    /// new basic node for Symbol. Content gets scanned, but
                    /// will not be decomposed (see SvgNode::decomposeSvgNode and bReferenced)
                    mpTarget = new SvgSymbolNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Switch:
                {
                    /// new node for Switch
                    mpTarget = new SvgSwitchNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Defs:
                case SVGToken::G:
                {
                    /// new node for Defs/G
                    mpTarget = new SvgGNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Svg:
                {
                    /// new node for Svg
                    mpTarget = new SvgSvgNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Use:
                {
                    /// new node for Use
                    mpTarget = new SvgUseNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::A:
                {
                    /// new node for A
                    mpTarget = new SvgANode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// shape elements
                case SVGToken::Circle:
                {
                    /// new node for Circle
                    mpTarget = new SvgCircleNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Ellipse:
                {
                    /// new node for Ellipse
                    mpTarget = new SvgEllipseNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Line:
                {
                    /// new node for Line
                    mpTarget = new SvgLineNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Path:
                {
                    /// new node for Path
                    mpTarget = new SvgPathNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Polygon:
                {
                    /// new node for Polygon
                    mpTarget = new SvgPolyNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Polyline:
                {
                    /// new node for Polyline
                    mpTarget = new SvgPolyNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Rect:
                {
                    /// new node for Rect
                    mpTarget = new SvgRectNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Image:
                {
                    /// new node for Image
                    mpTarget = new SvgImageNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// title and description
                case SVGToken::Title:
                case SVGToken::Desc:
                {
                    /// new node for Title and/or Desc
                    mpTarget = new SvgTitleDescNode(aSVGToken, maDocument, mpTarget);
                    break;
                }

                /// gradients
                case SVGToken::LinearGradient:
                case SVGToken::RadialGradient:
                {
                    mpTarget = new SvgGradientNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// gradient stops
                case SVGToken::Stop:
                {
                    mpTarget = new SvgGradientStopNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// text
                case SVGToken::Text:
                {
                    mpTarget = new SvgTextNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Tspan:
                {
                    mpTarget = new SvgTspanNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Tref:
                {
                    mpTarget = new SvgTrefNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::TextPath:
                {
                    mpTarget = new SvgTextPathNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// styles (as stylesheets)
                case SVGToken::Style:
                {
                    SvgStyleNode* pNew = new SvgStyleNode(maDocument, mpTarget);
                    mpTarget = pNew;

                    // #i125326# there are attributes, read them. This will set isTextCss to false if
                    // type attribute is different to "text/css"
                    mpTarget->parseAttributes(xAttribs);

                    if(pNew->isTextCss())
                    {
                        // if it is a Css style, allow reading text between the start and end tag (see
                        // SvgDocHdl::characters for details)
                        maCssContents.emplace_back();
                    }
                    break;
                }

                /// structural elements clip-path and mask. Content gets scanned, but
                /// will not be decomposed (see SvgNode::decomposeSvgNode and bReferenced)
                case SVGToken::ClipPathNode:
                {
                    /// new node for ClipPath
                    mpTarget = new SvgClipPathNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Mask:
                {
                    /// new node for Mask
                    mpTarget = new SvgMaskNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeColorMatrix:
                {
                    /// new node for feColorMatrix
                    mpTarget = new SvgFeColorMatrixNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeDropShadow:
                {
                    /// new node for feDropShadow
                    mpTarget = new SvgFeDropShadowNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeFlood:
                {
                    /// new node for feFlood
                    mpTarget = new SvgFeFloodNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeImage:
                {
                    /// new node for feImage
                    mpTarget = new SvgFeImageNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeGaussianBlur:
                {
                    /// new node for feGaussianBlur
                    mpTarget = new SvgFeGaussianBlurNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::FeOffset:
                {
                    /// new node for feOffset
                    mpTarget = new SvgFeOffsetNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }
                case SVGToken::Filter:
                {
                    /// new node for Filter
                    mpTarget = new SvgFilterNode(aSVGToken, maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// structural element marker
                case SVGToken::Marker:
                {
                    /// new node for marker
                    mpTarget = new SvgMarkerNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                /// structural element pattern
                case SVGToken::Pattern:
                {
                    /// new node for pattern
                    mpTarget = new SvgPatternNode(maDocument, mpTarget);
                    mpTarget->parseAttributes(xAttribs);
                    break;
                }

                default:
                {
                    mpTarget = new SvgNode(SVGToken::Unknown, maDocument, mpTarget);
                    break;
                }
            }
        }

        void SvgDocHdl::endElement( const OUString& aName )
        {
            if(aName.isEmpty())
                return;

            if(!mpTarget)
                return;

            const SVGToken aSVGToken(StrToSVGToken(aName, false));
            SvgNode* pTextNode(SVGToken::Text == aSVGToken ? mpTarget : nullptr);
            SvgStyleNode* pCssStyle(SVGToken::Style == aSVGToken ? static_cast< SvgStyleNode* >(mpTarget) : nullptr);
            SvgTitleDescNode* pSvgTitleDescNode(SVGToken::Title == aSVGToken || SVGToken::Desc == aSVGToken ? static_cast< SvgTitleDescNode* >(mpTarget) : nullptr);

            if(!mpTarget->getParent())
            {
                // last element closing, save this tree
                maDocument.appendNode(std::unique_ptr<SvgNode>(mpTarget));
            }

            mpTarget = const_cast< SvgNode* >(mpTarget->getParent());

            if (pSvgTitleDescNode && mpTarget)
            {
                const OUString& aText(pSvgTitleDescNode->getText());

                if(!aText.isEmpty())
                {
                    mpTarget->parseAttribute(aSVGToken, aText);
                }
            }

            if(pCssStyle && pCssStyle->isTextCss())
            {
                // css style parsing
                if(!maCssContents.empty())
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

            if(pTextNode)
            {
                // cleanup read strings
                whiteSpaceHandling(pTextNode, static_cast< SvgTspanNode*>(pTextNode), nullptr);
            }
        }

        void SvgDocHdl::characters( const OUString& aChars )
        {
            const sal_uInt32 nLength(aChars.getLength());

            if(!(mpTarget && nLength))
                return;

            switch(mpTarget->getType())
            {
                case SVGToken::Text:
                case SVGToken::Tspan:
                case SVGToken::TextPath:
                {
                    const auto& rChilds = mpTarget->getChildren();

                    if(!rChilds.empty())
                    {
                        SvgNode* pChild = rChilds[rChilds.size() - 1].get();
                        if ( pChild->getType() == SVGToken::Character )
                        {
                            SvgCharacterNode& rSvgCharacterNode = static_cast< SvgCharacterNode& >(*pChild);

                            // concatenate to current character span
                            rSvgCharacterNode.concatenate(aChars);
                            break;
                        }
                    }

                    // add character span as simplified tspan (no arguments)
                    // as direct child of SvgTextNode/SvgTspanNode/SvgTextPathNode
                    new SvgCharacterNode(maDocument, mpTarget, aChars);
                    break;
                }
                case SVGToken::Style:
                {
                    SvgStyleNode& rSvgStyleNode = static_cast< SvgStyleNode& >(*mpTarget);

                    if(rSvgStyleNode.isTextCss())
                    {
                        // collect characters for css style
                        if(!maCssContents.empty())
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
                case SVGToken::Title:
                case SVGToken::Desc:
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

        void SvgDocHdl::ignorableWhitespace(const OUString& /*aWhitespaces*/)
        {
        }

        void SvgDocHdl::processingInstruction(const OUString& /*aTarget*/, const OUString& /*aData*/)
        {
        }

        void SvgDocHdl::setDocumentLocator(const uno::Reference< xml::sax::XLocator >& /*xLocator*/)
        {
        }
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
