/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
                            
                            svgio::svgreader::SvgCharacterNode* pCharNode = static_cast< svgio::svgreader::SvgCharacterNode* >(pCandidate);
                            pCharNode->whiteSpaceHandling();

                            
                            
                            if(!pCharNode->getText().isEmpty())
                            {
                                if(pLast)
                                {
                                    bool bAddGap(true);
                                    static bool bNoGapsForBaselineShift(true);

                                    if(bNoGapsForBaselineShift)
                                    {
                                        
                                        
                                        
                                        const svgio::svgreader::SvgStyleAttributes* pStyleLast = pLast->getSvgStyleAttributes();
                                        const svgio::svgreader::SvgStyleAttributes* pStyleCurrent = pCandidate->getSvgStyleAttributes();

                                        if(pStyleLast && pStyleCurrent && pStyleLast->getBaselineShift() != pStyleCurrent->getBaselineShift())
                                        {
                                            bAddGap = false;
                                        }
                                    }

                                    
                                    
                                    if(bAddGap)
                                    {
                                        pLast->addGap();
                                    }
                                }

                                
                                pLast = pCharNode;
                            }
                            break;
                        }
                        case svgio::svgreader::SVGTokenTspan:
                        case svgio::svgreader::SVGTokenTextPath:
                        case svgio::svgreader::SVGTokenTref:
                        {
                            
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

        void SvgDocHdl::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            if(!aName.isEmpty())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName));

                switch(aSVGToken)
                {
                    
                    case SVGTokenSymbol:
                    {
                        
                        
                        mpTarget = new SvgSymbolNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenDefs:
                    case SVGTokenG:
                    {
                        
                        mpTarget = new SvgGNode(aSVGToken, maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenSvg:
                    {
                        
                        mpTarget = new SvgSvgNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenUse:
                    {
                        
                        mpTarget = new SvgUseNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
                    case SVGTokenCircle:
                    {
                        
                        mpTarget = new SvgCircleNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenEllipse:
                    {
                        
                        mpTarget = new SvgEllipseNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenLine:
                    {
                        
                        mpTarget = new SvgLineNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPath:
                    {
                        
                        mpTarget = new SvgPathNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPolygon:
                    {
                        
                        mpTarget = new SvgPolyNode(maDocument, mpTarget, false);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenPolyline:
                    {
                        
                        mpTarget = new SvgPolyNode(maDocument, mpTarget, true);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenRect:
                    {
                        
                        mpTarget = new SvgRectNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenImage:
                    {
                        
                        mpTarget = new SvgImageNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
                    case SVGTokenTitle:
                    case SVGTokenDesc:
                    {
                        
                        mpTarget = new SvgTitleDescNode(aSVGToken, maDocument, mpTarget);
                        break;
                    }

                    
                    case SVGTokenLinearGradient:
                    case SVGTokenRadialGradient:
                    {
                        mpTarget = new SvgGradientNode(aSVGToken, maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
                    case SVGTokenStop:
                    {
                        mpTarget = new SvgGradientStopNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
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

                    
                    case SVGTokenStyle:
                    {
                        SvgStyleNode* pNew = new SvgStyleNode(maDocument, mpTarget);
                        mpTarget = pNew;
                        mpTarget->parseAttributes(xAttribs);

                        if(pNew->isTextCss())
                        {
                            maCssContents.push_back(OUString());
                        }
                        break;
                    }

                    
                    
                    case SVGTokenClipPathNode:
                    {
                        
                        mpTarget = new SvgClipPathNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }
                    case SVGTokenMask:
                    {
                        
                        mpTarget = new SvgMaskNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
                    case SVGTokenMarker:
                    {
                        
                        mpTarget = new SvgMarkerNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    
                    case SVGTokenPattern:
                    {
                        
                        mpTarget = new SvgPatternNode(maDocument, mpTarget);
                        mpTarget->parseAttributes(xAttribs);
                        break;
                    }

                    default:
                    {
                        
#ifdef DBG_UTIL
                        myAssert(
                            OUString("Unknown Base SvgToken <") +
                            aName +
                            OUString("> (!)") );
#endif
                        break;
                    }
                }
            }
        }

        void SvgDocHdl::endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException)
        {
            if(!aName.isEmpty())
            {
                const SVGToken aSVGToken(StrToSVGToken(aName));
                SvgNode* pWhitespaceCheck(SVGTokenText == aSVGToken ? mpTarget : 0);
                SvgStyleNode* pCssStyle(SVGTokenStyle == aSVGToken ? static_cast< SvgStyleNode* >(mpTarget) : 0);
                SvgTitleDescNode* pSvgTitleDescNode(SVGTokenTitle == aSVGToken || SVGTokenDesc == aSVGToken ? static_cast< SvgTitleDescNode* >(mpTarget) : 0);

                switch(aSVGToken)
                {
                    

                    
                    case SVGTokenDefs:
                    case SVGTokenG:
                    case SVGTokenSvg:
                    case SVGTokenSymbol:
                    case SVGTokenUse:

                    
                    case SVGTokenCircle:
                    case SVGTokenEllipse:
                    case SVGTokenLine:
                    case SVGTokenPath:
                    case SVGTokenPolygon:
                    case SVGTokenPolyline:
                    case SVGTokenRect:
                    case SVGTokenImage:

                    
                    case SVGTokenTitle:
                    case SVGTokenDesc:

                    
                    case SVGTokenLinearGradient:
                    case SVGTokenRadialGradient:

                    
                    case SVGTokenStop:

                    
                    case SVGTokenText:
                    case SVGTokenTspan:
                    case SVGTokenTextPath:
                    case SVGTokenTref:

                    
                    case SVGTokenStyle:

                    
                    case SVGTokenClipPathNode:
                    case SVGTokenMask:

                    
                    case SVGTokenMarker:

                    
                    case SVGTokenPattern:

                    
                    {
                        if(mpTarget)
                        {
                            if(!mpTarget->getParent())
                            {
                                
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
                        else 
                        {
                            mpTarget->parseAttribute(getStrDesc(), aSVGToken, aText);
                        }
                    }
                }

                if(pCssStyle && pCssStyle->isTextCss())
                {
                    
                    if(maCssContents.size())
                    {
                        
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
                    
                    whiteSpaceHandling(pWhitespaceCheck, 0);
                }
            }
        }

        void SvgDocHdl::characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException)
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
                            
                            pTarget->concatenate(aChars);
                        }
                        else
                        {
                            
                            
                            new SvgCharacterNode(maDocument, mpTarget, aChars);
                        }
                        break;
                    }
                    case SVGTokenStyle:
                    {
                        SvgStyleNode& rSvgStyleNode = static_cast< SvgStyleNode& >(*mpTarget);

                        if(rSvgStyleNode.isTextCss())
                        {
                            
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

                        
                        rSvgTitleDescNode.concatenate(aChars);
                        break;
                    }
                    default:
                    {
                        
                        break;
                    }
                }
            }
        }

        void SvgDocHdl::ignorableWhitespace(const OUString& /*aWhitespaces*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }

        void SvgDocHdl::processingInstruction(const OUString& /*aTarget*/, const OUString& /*aData*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }

        void SvgDocHdl::setDocumentLocator(const uno::Reference< xml::sax::XLocator >& /*xLocator*/) throw (xml::sax::SAXException, uno::RuntimeException)
        {
        }
    } 
} 




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
