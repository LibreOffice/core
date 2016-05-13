/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svgreader.hxx"
#include <xmloff/attrlist.hxx>
#include "gfxtypes.hxx"
#include "units.hxx"
#include "parserfragments.hxx"
#include "tokenmap.hxx"
#include "b2dellipse.hxx"

#include <rtl/math.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <comphelper/processfactory.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/tools/converter.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gradient.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/zcodec.hxx>

#include <map>

#define OASIS_STR "urn:oasis:names:tc:opendocument:xmlns:"

using namespace ::com::sun::star;

namespace svgi
{
    enum SvgiVisitorCaller {STYLE_ANNOTATOR, SHAPE_WRITER, STYLE_WRITER};
namespace
{

/** visits all children of the specified type with the given functor
 */
template<typename Func> void visitChildren(const Func& rFunc,
                                           const uno::Reference<xml::dom::XElement>& rElem,
                                           xml::dom::NodeType eChildType )
{
    uno::Reference<xml::dom::XNodeList> xChildren( rElem->getChildNodes() );
    const sal_Int32 nNumNodes( xChildren->getLength() );
    for( sal_Int32 i=0; i<nNumNodes; ++i )
    {
        SAL_INFO("svg", "node type: " << sal::static_int_cast<sal_uInt32>(xChildren->item(i)->getNodeType()) << " tag name " << xChildren->item(i)->getNodeName() << " value |" << xChildren->item(i)->getNodeValue() << "|");
        if( xChildren->item(i)->getNodeType() == eChildType )
            rFunc( *xChildren->item(i).get() );
    }
}

/** Visit all elements of the given tree (in-order traversal)

    Given functor is called for every element, and passed the
    element's attributes, if any
 */
template<typename Func> void visitElements(Func& rFunc,
                                           const uno::Reference<xml::dom::XElement>& rElem,
                                           SvgiVisitorCaller eCaller)
{
    if( rElem->hasAttributes() )
        rFunc(rElem, rElem->getAttributes());
    else
        rFunc(rElem);

    // notify children processing
    rFunc.push();

    if (eCaller == SHAPE_WRITER && rElem->getTagName() == "defs")
        return;

    // recurse over children
    uno::Reference<xml::dom::XNodeList> xChildren( rElem->getChildNodes() );
    const sal_Int32 nNumNodes( xChildren->getLength() );
    for( sal_Int32 i=0; i<nNumNodes; ++i )
    {
        if( xChildren->item(i)->getNodeType() == xml::dom::NodeType_ELEMENT_NODE ){
            visitElements( rFunc,
                uno::Reference<xml::dom::XElement>(
                xChildren->item(i),
                uno::UNO_QUERY_THROW),
                eCaller );
        }
    }

    // children processing done
    rFunc.pop();
}

template<typename value_type> value_type square(value_type v)
{
    return v*v;
}

double colorDiffSquared(const ARGBColor& rCol1, const ARGBColor& rCol2)
{
    return
        square(rCol1.a-rCol2.a)
        + square(rCol1.r-rCol2.r)
        + square(rCol1.g-rCol2.g)
        + square(rCol1.b-rCol2.b);
}

/**
    check whether a polypolygon contains both open and closed
    polygons
**/
bool PolyPolygonIsMixedOpenAndClosed( const basegfx::B2DPolyPolygon& rPoly )
{
    bool bRetval(false);
    bool bOpen(false);
    bool bClosed(false);

    // PolyPolygon is mixed open and closed if there is more than one
    // polygon and there are both closed and open polygons.
    for( sal_uInt32 a(0L); !bRetval && a < rPoly.count(); a++ )
    {
        if ( (rPoly.getB2DPolygon(a)).isClosed() )
        {
            bClosed = true;
        }
        else
        {
            bOpen = true;
        }

        bRetval = (bClosed && bOpen);
    }

    return bRetval;
}

typedef std::map<OUString,sal_Size> ElementRefMapType;

struct AnnotatingVisitor
{
    AnnotatingVisitor(StatePool&                                        rStatePool,
                      StateMap&                                         rStateMap,
                      const State&                                       rInitialState,
                      const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
                      std::vector< uno::Reference<xml::dom::XElement> >& rUseElementVector,
                      bool& rGradientNotFound) :
        mnCurrStateId(0),
        maCurrState(),
        maParentStates(),
        mrStates(rStatePool),
        mrStateMap(rStateMap),
        mxDocumentHandler(xDocumentHandler),
        maGradientVector(),
        maGradientStopVector(),
        maElementVector(),
        mrUseElementVector(rUseElementVector),
        mrGradientNotFound(rGradientNotFound)
    {
        maParentStates.push_back(rInitialState);
    }

    void operator()( const uno::Reference<xml::dom::XElement>& xElem)
    {
        const sal_Int32 nTagId(getTokenId(xElem->getTagName()));
        if (nTagId != XML_TEXT && nTagId != XML_TSPAN)
            return;

        maCurrState = maParentStates.back();
        maCurrState.maTransform.identity();
        maCurrState.maViewBox.reset();
        // if necessary, serialize to automatic-style section
        writeStyle(xElem,nTagId);
    }

    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& xAttributes )
    {
        const sal_Int32 nTagId(getTokenId(xElem->getTagName()));
        switch (nTagId)
        {
            case XML_LINEARGRADIENT:
            {
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                maGradientVector.push_back(Gradient(Gradient::LINEAR));

                // do we have a reference to a parent gradient? parse
                // that first, as it sets our defaults here (manually
                // tracking default state on each Gradient variable is
                // much more overhead)
                uno::Reference<xml::dom::XNode> xNode(xAttributes->getNamedItem("href"));
                if(xNode.is())
                {
                    const OUString sValue(xNode->getNodeValue());
                    ElementRefMapType::iterator aFound=maGradientIdMap.end();
                    if ( sValue.copy(0,1) == "#" )
                        aFound = maGradientIdMap.find(sValue.copy(1));
                    else
                        aFound = maGradientIdMap.find(sValue);

                    if( aFound != maGradientIdMap.end() )
                        maGradientVector.back() = maGradientVector[aFound->second];
                    else
                    {
                        mrGradientNotFound = true;
                        maGradientVector.pop_back();
                        return;
                    }
                }

                // do that after dereferencing, to prevent hyperlinked
                // gradient to clobber our Id again
                maGradientVector.back().mnId = maGradientVector.size()-1;
                maGradientVector.back().meType = Gradient::LINEAR; // has been clobbered as well

                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    parseLinearGradientData( maGradientVector.back(),
                                             maGradientVector.size()-1,
                                             getTokenId(xAttributes->item(i)->getNodeName()),
                                             xAttributes->item(i)->getNodeValue() );
                }
                break;
            }
            case XML_RADIALGRADIENT:
            {
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                maGradientVector.push_back(Gradient(Gradient::RADIAL));

                // do we have a reference to a parent gradient? parse
                // that first, as it sets our defaults here (manually
                // tracking default state on each Gradient variable is
                // much more overhead)
                uno::Reference<xml::dom::XNode> xNode(xAttributes->getNamedItem("href"));
                if(xNode.is())
                {
                    const OUString sValue(xNode->getNodeValue());
                    ElementRefMapType::iterator aFound=maGradientIdMap.end();
                    if ( sValue.copy(0,1) == "#" )
                        aFound = maGradientIdMap.find(sValue.copy(1));
                    else
                        aFound = maGradientIdMap.find(sValue);

                    if( aFound != maGradientIdMap.end() )
                        maGradientVector.back() = maGradientVector[aFound->second];
                    else
                    {
                        mrGradientNotFound = true;
                        maGradientVector.pop_back();
                        return;
                    }
                }

                // do that after dereferencing, to prevent hyperlinked
                // gradient to clobber our Id again
                maGradientVector.back().mnId = maGradientVector.size()-1;
                maGradientVector.back().meType = Gradient::RADIAL; // has been clobbered as well

                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    parseRadialGradientData( maGradientVector.back(),
                                             maGradientVector.size()-1,
                                             getTokenId(xAttributes->item(i)->getNodeName()),
                                             xAttributes->item(i)->getNodeValue() );
                }
                break;
            }
            case XML_USE:
            {
                uno::Reference<xml::dom::XNode> xNode(xAttributes->getNamedItem("href"));
                if(xNode.is())
                {
                    OUString sValue(xNode->getNodeValue());
                    ElementRefMapType::iterator aFound=maElementIdMap.end();
                    if ( sValue.copy(0,1) == "#" )
                        sValue = sValue.copy(1);
                    aFound = maElementIdMap.find(sValue);
                    bool bFound = aFound != maElementIdMap.end();
                    if (bFound)
                    {
                        bool bSelfCycle = false;

                        uno::Reference<xml::dom::XNode> xParentNode(xElem->getParentNode());
                        if (xParentNode.is() && xParentNode->hasAttributes())
                        {
                            const uno::Reference<xml::dom::XNamedNodeMap> xParentAttributes = xParentNode->getAttributes();
                            const sal_Int32 nFooNumAttrs(xParentAttributes->getLength());
                            for (sal_Int32 i=0; i < nFooNumAttrs; ++i)
                            {
                                const sal_Int32 nTokenId(getTokenId(xParentAttributes->item(i)->getNodeName()));
                                if (XML_ID == nTokenId)
                                {
                                    OUString sParentID = xParentAttributes->item(i)->getNodeValue();
                                    bSelfCycle = sParentID == sValue;
                                    break;
                                }
                            }
                        }

                        if (bSelfCycle)
                        {
                            //drop this invalid self-referencing "use" node
                            maElementIdMap.erase(aFound);
                            bFound = false;
                        }
                    }

                    if (bFound)
                    {
                        uno::Reference<xml::dom::XElement> xRefElem(
                            maElementVector[aFound->second]->cloneNode(true), uno::UNO_QUERY);

                        xRefElem->removeAttribute("id");
                        uno::Reference<xml::dom::XNode> xAttrNode;

                        const sal_Int32 nNumAttrs( xAttributes->getLength() );
                        OUString sAttributeValue;
                        double x=0.0, y=0.0;
                        for( sal_Int32 i=0; i<nNumAttrs; ++i )
                        {
                            sAttributeValue = xAttributes->item(i)->getNodeValue();
                            const sal_Int32 nAttribId(
                                getTokenId(xAttributes->item(i)->getNodeName()));

                            switch(nAttribId)
                            {
                                case XML_ID:
                                    maElementVector.push_back(xElem);
                                    maElementIdMap.insert(std::make_pair(sAttributeValue,
                                        maElementVector.size() - 1));
                                    break;
                                case XML_X:
                                    x = convLength(sAttributeValue,maCurrState,'h');
                                    break;
                                case XML_Y:
                                    y = convLength(sAttributeValue,maCurrState,'v');
                                    break;
                                case XML_TRANSFORM:
                                    break;
                                default:
                                    OUString sAttributeName = xAttributes->item(i)->getNodeName();
                                    xRefElem->setAttribute(sAttributeName, sAttributeValue);
                                    break;
                            }
                        }
                        std::stringstream ssAttrValue;
                        ssAttrValue << xRefElem->getAttribute("transform");
                        ssAttrValue << xElem->getAttribute("transform");
                        ssAttrValue << " translate(" << x << "," << y << ")";

                        OUString attrValue(OUString::createFromAscii (ssAttrValue.str().c_str()));
                        xRefElem->setAttribute("transform", attrValue);

                        mrUseElementVector.push_back(xRefElem);

                        visitElements((*this), xRefElem, STYLE_ANNOTATOR);
                    }
                }
                break;
            }
            case XML_STOP:
            {
                if (!maGradientVector.empty())
                {
                    const sal_Int32 nNumAttrs( xAttributes->getLength() );
                    maGradientStopVector.push_back(GradientStop());
                    maGradientVector.back().maStops.push_back(maGradientStopVector.size()-1);

                    // first parse 'color' as 'stop-color' might depend on it
                    // if 'stop-color''s value is "currentColor" and parsed previously
                    uno::Reference<xml::dom::XNode> xNodeColor(xAttributes->getNamedItem("color"));
                    if(xNodeColor.is())
                        parseGradientStop( maGradientStopVector.back(),
                            maGradientStopVector.size()-1,
                            XML_STOP_COLOR,
                            xNodeColor->getNodeValue() );

                    //now, parse the rest of attributes
                    for( sal_Int32 i=0; i<nNumAttrs; ++i )
                    {
                        const sal_Int32 nTokenId(
                            getTokenId(xAttributes->item(i)->getNodeName()));
                        if ( nTokenId != XML_COLOR )
                            parseGradientStop( maGradientStopVector.back(),
                                               maGradientStopVector.size()-1,
                                               nTokenId,
                                               xAttributes->item(i)->getNodeValue() );
                    }
                }
                break;
            }
            default:
            {
                if ( !mrGradientNotFound )
                {
                    // init state. inherit defaults from parent.
                    maCurrState = maParentStates.back();
                    maCurrState.maTransform.identity();
                    maCurrState.maViewBox.reset();

                    // first parse 'color' and 'style' as 'fill' and 'stroke' might depend on them
                    // if their values are "currentColor" and parsed previously
                    uno::Reference<xml::dom::XNode> xNodeColor(xAttributes->getNamedItem("color"));
                    if(xNodeColor.is())
                        parseAttribute(XML_COLOR, xNodeColor->getNodeValue());

                    uno::Reference<xml::dom::XNode> xNodeStyle(xAttributes->getNamedItem("style"));
                    if(xNodeStyle.is())
                        parseStyle(xNodeStyle->getNodeValue());

                    const sal_Int32 nNumAttrs( xAttributes->getLength() );
                    OUString sAttributeValue;

                    //now, parse the rest of attributes
                    for( sal_Int32 i=0; i<nNumAttrs; ++i )
                    {
                        sAttributeValue = xAttributes->item(i)->getNodeValue();
                        const sal_Int32 nTokenId(
                            getTokenId(xAttributes->item(i)->getNodeName()));
                        if( XML_ID == nTokenId )
                        {
                            maElementVector.push_back(xElem);
                            maElementIdMap.insert(std::make_pair(sAttributeValue,
                                maElementVector.size() - 1));
                        }
                        else if (nTokenId != XML_COLOR && nTokenId != XML_STYLE)
                            parseAttribute(nTokenId,
                                sAttributeValue);
                    }

                    // all attributes parsed, can calc total CTM now
                    basegfx::B2DHomMatrix aLocalTransform;
                    if( !maCurrState.maViewBox.isEmpty() &&
                        maCurrState.maViewBox.getWidth() != 0.0 &&
                        maCurrState.maViewBox.getHeight() != 0.0 )
                    {
                        // transform aViewBox into viewport, keep aspect ratio
                        aLocalTransform.translate(-maCurrState.maViewBox.getMinX(),
                                                  -maCurrState.maViewBox.getMinY());
                        double scaleW = maCurrState.maViewport.getWidth()/maCurrState.maViewBox.getWidth();
                        double scaleH = maCurrState.maViewport.getHeight()/maCurrState.maViewBox.getHeight();
                        double scale = (scaleW < scaleH) ? scaleW : scaleH;
                        aLocalTransform.scale(scale,scale);
                    }
                    else if( !maParentStates.back().maViewBox.isEmpty() )
                                        maCurrState.maViewBox = maParentStates.back().maViewBox;

                    maCurrState.maCTM = maCurrState.maCTM*maCurrState.maTransform*aLocalTransform;

                    OSL_TRACE("annotateStyle - CTM is: %f %f %f %f %f %f",
                              maCurrState.maCTM.get(0,0),
                              maCurrState.maCTM.get(0,1),
                              maCurrState.maCTM.get(0,2),
                              maCurrState.maCTM.get(1,0),
                              maCurrState.maCTM.get(1,1),
                              maCurrState.maCTM.get(1,2));

                    // if necessary, serialize to automatic-style section
                    writeStyle(xElem,nTagId);
                }
            }
        }
    }

    static OUString getStyleName( const char* sPrefix, sal_Int32 nId )
    {
        return OUString::createFromAscii(sPrefix)+OUString::number(nId);
    }

    bool hasGradientOpacity( const Gradient& rGradient )
    {
        return
            (rGradient.maStops.size() > 1) &&
            (maGradientStopVector[
                 rGradient.maStops[0]].maStopColor.a != 1.0 ||
             maGradientStopVector[
                 rGradient.maStops[1]].maStopColor.a != 1.0);
    }

    struct StopSorter
    {
        explicit StopSorter( const std::vector< GradientStop >& rStopVec ) :
            mrStopVec(rStopVec)
        {}

        bool operator()( sal_Size rLHS, sal_Size rRHS )
        {
            return mrStopVec[rLHS].mnStopPosition < mrStopVec[rRHS].mnStopPosition;
        }

        const std::vector< GradientStop >& mrStopVec;
    };

    void optimizeGradientStops( Gradient& rGradient )
    {
        // sort for increasing stop position
        std::sort(rGradient.maStops.begin(),rGradient.maStops.end(),
                  StopSorter(maGradientStopVector));

        if( rGradient.maStops.size() < 3 )
            return; //easy! :-)

        // join similar colors
        std::vector<sal_Size> aNewStops { rGradient.maStops.front() };
        for( sal_Size i=1; i<rGradient.maStops.size(); ++i )
        {
            if( maGradientStopVector[rGradient.maStops[i]].maStopColor !=
                maGradientStopVector[aNewStops.back()].maStopColor )
                aNewStops.push_back(rGradient.maStops[i]);
        }

        rGradient.maStops = aNewStops;
        if (rGradient.maStops.size() < 2)
        {
            return; // can't optimize further...
        }

        // axial gradient, maybe?
        if( rGradient.meType == Gradient::LINEAR &&
            rGradient.maStops.size() == 3 &&
            maGradientStopVector[rGradient.maStops.front()].maStopColor ==
            maGradientStopVector[rGradient.maStops.back()].maStopColor )
        {
            // yep - keep it at that
            return;
        }

        // find out most significant color difference, and limit to
        // those two stops around this border (metric is
        // super-simplistic: take euclidean distance of colors, weigh
        // with stop distance)
        sal_Size nMaxIndex=0;
        double    fMaxDistance=0.0;
        for( sal_Size i=1; i<rGradient.maStops.size(); ++i )
        {
            const double fCurrDistance(
                colorDiffSquared(
                    maGradientStopVector[rGradient.maStops[i-1]].maStopColor,
                    maGradientStopVector[rGradient.maStops[i]].maStopColor) *
                (square(maGradientStopVector[rGradient.maStops[i-1]].mnStopPosition) +
                 square(maGradientStopVector[rGradient.maStops[i]].mnStopPosition)) );

            if( fCurrDistance > fMaxDistance )
            {
                nMaxIndex = i-1;
                fMaxDistance = fCurrDistance;
            }
        }
        rGradient.maStops[0] = rGradient.maStops[nMaxIndex];
        rGradient.maStops[1] = rGradient.maStops[nMaxIndex+1];
        rGradient.maStops.erase(rGradient.maStops.begin()+2,rGradient.maStops.end());
    }

    static sal_Int8 toByteColor( double val )
    {
        // TODO(Q3): duplicated from vcl::unotools
        return sal::static_int_cast<sal_Int8>(
            basegfx::fround(val*255.0));
    }

    static OUString getOdfColor( const ARGBColor& rColor )
    {
        // TODO(Q3): duplicated from pdfimport
        OUStringBuffer aBuf( 7 );
        const sal_uInt8 nRed  ( toByteColor(rColor.r)   );
        const sal_uInt8 nGreen( toByteColor(rColor.g) );
        const sal_uInt8 nBlue ( toByteColor(rColor.b)  );
        aBuf.append( '#' );
        if( nRed < 0x10 )
            aBuf.append( '0' );
        aBuf.append( sal_Int32(nRed), 16 );
        if( nGreen < 0x10 )
            aBuf.append( '0' );
        aBuf.append( sal_Int32(nGreen), 16 );
        if( nBlue < 0x10 )
            aBuf.append( '0' );
        aBuf.append( sal_Int32(nBlue), 16 );

        // TODO(F3): respect alpha transparency (polygons etc.)
        OSL_ASSERT(rColor.a == 1.0);

        return aBuf.makeStringAndClear();
    }

    static OUString getOdfAlign( TextAlign eAlign )
    {
        static const char aStart[] = "start";
        static const char aEnd[] = "end";
        static const char aCenter[] = "center";
        switch(eAlign)
        {
            default:
            case BEFORE:
                return OUString(aStart);
            case CENTER:
                return OUString(aCenter);
            case AFTER:
                return OUString(aEnd);
        }
    }

    bool writeStyle(State& rState, const sal_Int32 nTagId)
    {
        rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
        uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

        if (XML_TEXT == nTagId || XML_TSPAN == nTagId) {
            rState.mbIsText = true;
            basegfx::B2DTuple aScale, aTranslate;
            double fRotate, fShearX;
            if (rState.maCTM.decompose(aScale, aTranslate, fRotate, fShearX))
            {
                rState.mnFontSize *= aScale.getX();
            }
        }

        std::pair<StatePool::iterator,
                  bool> aRes = mrStates.insert(rState);
        SAL_INFO ("svg", "size " << mrStates.size() << "   id " <<  const_cast<State&>(*aRes.first).mnStyleId);

        if( !aRes.second )
            return false; // not written

        ++mnCurrStateId;

        // mnStyleId does not take part in hashing/comparison
        const_cast<State&>(*aRes.first).mnStyleId = mnCurrStateId;
        SAL_INFO ("svg", " --> " <<  const_cast<State&>(*aRes.first).mnStyleId);

        mrStateMap.insert(std::make_pair(
                              mnCurrStateId,
                              rState));

        // find two representative stop colors (as ODF only support
        // start&end color)
        optimizeGradientStops(rState.maFillGradient);

        if( !mxDocumentHandler.is() )
            return true; // cannot write style, svm import case

        // do we have a gradient fill? then write out gradient as well
        if( rState.meFillType == GRADIENT && rState.maFillGradient.maStops.size() > 0 )
        {
            // if only one stop-color is defined
            if( rState.maFillGradient.maStops.size() == 1 )
                rState.maFillGradient.maStops.push_back(rState.maFillGradient.maStops[0]);

            // TODO(F3): ODF12 supposedly also groks svg:linear/radialGradient. But CL says: nope.
            xAttrs->AddAttribute( "draw:name", getStyleName("svggradient", rState.maFillGradient.mnId) );
            if( rState.maFillGradient.meType == Gradient::LINEAR )
            {
                // should the optimizeGradientStops method decide that
                // this is a three-color gradient, it prolly wanted us
                // to take axial instead
                xAttrs->AddAttribute( "draw:style",
                                      rState.maFillGradient.maStops.size() == 3 ?
                                      OUString("axial") :
                                      OUString("linear") );
            }
            else
            {
                xAttrs->AddAttribute( "draw:style", "ellipsoid" );
                xAttrs->AddAttribute( "draw:cx", "50%" );
                xAttrs->AddAttribute( "draw:cy", "50%" );
            }

            basegfx::B2DTuple rScale, rTranslate;
            double rRotate, rShearX;
            if( rState.maFillGradient.maTransform.decompose(rScale, rTranslate, rRotate, rShearX) )
                xAttrs->AddAttribute( "draw:angle",
                                      OUString::number(rRotate*1800.0/M_PI ) );
            xAttrs->AddAttribute( "draw:start-color",
                                  getOdfColor(
                                      maGradientStopVector[
                                          rState.maFillGradient.maStops[0]].maStopColor) );
            xAttrs->AddAttribute( "draw:end-color",
                                  getOdfColor(
                                      maGradientStopVector[
                                          rState.maFillGradient.maStops[1]].maStopColor) );
            xAttrs->AddAttribute( "draw:border", "0%" );
            mxDocumentHandler->startElement( "draw:gradient", xUnoAttrs );
            mxDocumentHandler->endElement( "draw:gradient" );

            if( hasGradientOpacity(rState.maFillGradient) )
            {
                // need to write out opacity style as well
                xAttrs->Clear();
                xAttrs->AddAttribute( "draw:name", getStyleName("svgopacity", rState.maFillGradient.mnId) );
                if( rState.maFillGradient.meType == Gradient::LINEAR )
                {
                    xAttrs->AddAttribute( "draw:style", "linear" );
                }
                else
                {
                    xAttrs->AddAttribute( "draw:style", "ellipsoid" );
                    xAttrs->AddAttribute( "draw:cx", "50%" );
                    xAttrs->AddAttribute( "draw:cy", "50%" );
                }

                // modulate gradient opacity with overall fill opacity
                xAttrs->AddAttribute( "draw:end",
                                      OUString::number(
                                          maGradientStopVector[
                                              rState.maFillGradient.maStops[0]].maStopColor.a*
                                          maCurrState.mnFillOpacity*maCurrState.mnOpacity*100.0)+"%" );
                xAttrs->AddAttribute( "draw:start",
                                      OUString::number(
                                          maGradientStopVector[
                                              rState.maFillGradient.maStops[1]].maStopColor.a*
                                          maCurrState.mnFillOpacity*maCurrState.mnOpacity*100.0)+"%" );
                xAttrs->AddAttribute( "draw:border", "0%" );
                mxDocumentHandler->startElement( "draw:opacity", xUnoAttrs );
                mxDocumentHandler->endElement( "draw:opacity" );
            }
        }

        // serialize to automatic-style section
        if( nTagId == XML_TEXT || nTagId == XML_TSPAN)
        {
            // write paragraph style attributes
            xAttrs->Clear();

            xAttrs->AddAttribute( "style:name", getStyleName("svgparagraphstyle", mnCurrStateId) );
            xAttrs->AddAttribute( "style:family", "paragraph" );
            mxDocumentHandler->startElement( "style:style", xUnoAttrs );

            xAttrs->Clear();
            xAttrs->AddAttribute( "fo:text-align", getOdfAlign(rState.meTextAnchor));

            mxDocumentHandler->startElement( "style:paragraph-properties", xUnoAttrs );
            mxDocumentHandler->endElement( "style:paragraph-properties" );
            mxDocumentHandler->endElement( "style:style" );

            // write text style attributes
            xAttrs->Clear();

            xAttrs->AddAttribute( "style:name", getStyleName("svgtextstyle", mnCurrStateId) );
            xAttrs->AddAttribute( "style:family", "text" );
            mxDocumentHandler->startElement( "style:style", xUnoAttrs );
            xAttrs->Clear();
            xAttrs->AddAttribute( "fo:font-family", rState.maFontFamily);
            xAttrs->AddAttribute( "fo:font-size",
                                  OUString::number(pt2mm(rState.mnFontSize))+"mm");
            xAttrs->AddAttribute( "fo:font-style", rState.maFontStyle);
            xAttrs->AddAttribute( "fo:font-variant", rState.maFontVariant);
            xAttrs->AddAttribute( "fo:font-weight",
                                  OUString::number(rState.mnFontWeight));
            xAttrs->AddAttribute( "fo:color", getOdfColor(rState.maFillColor));

            mxDocumentHandler->startElement( "style:text-properties", xUnoAttrs );
            mxDocumentHandler->endElement( "style:text-properties" );
            mxDocumentHandler->endElement( "style:style" );
        }

        xAttrs->Clear();
        xAttrs->AddAttribute( "style:name" , getStyleName("svggraphicstyle", mnCurrStateId) );
        xAttrs->AddAttribute( "style:family", "graphic" );
        mxDocumentHandler->startElement( "style:style", xUnoAttrs );

        xAttrs->Clear();
        // text or shape? if the former, no use in processing any
        // graphic attributes except stroke color, ODF can do ~nothing
        // with text shapes
        if( nTagId == XML_TEXT || nTagId == XML_TSPAN )
        {
            //xAttrs->AddAttribute( "draw:auto-grow-height", "true");
            xAttrs->AddAttribute( "draw:auto-grow-width", "true");
            xAttrs->AddAttribute( "draw:textarea-horizontal-align", "left");
            //xAttrs->AddAttribute( "draw:textarea-vertical-align", "top");
            xAttrs->AddAttribute( "fo:min-height", "0cm");

            xAttrs->AddAttribute( "fo:padding-top", "0cm");
            xAttrs->AddAttribute( "fo:padding-left", "0cm");
            xAttrs->AddAttribute( "fo:padding-right", "0cm");
            xAttrs->AddAttribute( "fo:padding-bottom", "0cm");

            // disable any background shape
            xAttrs->AddAttribute( "draw:stroke", "none");
            xAttrs->AddAttribute( "draw:fill", "none");
        }
        else
        {
            if( rState.meFillType != NONE )
            {
                if( rState.meFillType == GRADIENT )
                {
                    // don't fill the gradient if there's no stop element present
                    if( rState.maFillGradient.maStops.size() == 0 )
                        xAttrs->AddAttribute( "draw:fill", "none" );
                    else
                    {
                        xAttrs->AddAttribute( "draw:fill", "gradient");
                        xAttrs->AddAttribute( "draw:fill-gradient-name",
                                              getStyleName("svggradient", rState.maFillGradient.mnId) );
                        if( hasGradientOpacity(rState.maFillGradient) )
                        {
                            // needs transparency gradient as well
                            xAttrs->AddAttribute( "draw:opacity-name",
                                                  getStyleName("svgopacity", rState.maFillGradient.mnId) );
                        }
                        else if( maCurrState.mnFillOpacity*maCurrState.mnOpacity != 1.0 )
                            xAttrs->AddAttribute( "draw:opacity",
                                                  OUString::number(100.0*maCurrState.mnFillOpacity*maCurrState.mnOpacity)+"%" );
                    }
                }
                else
                {
                    xAttrs->AddAttribute( "draw:fill", "solid");
                    xAttrs->AddAttribute( "draw:fill-color", getOdfColor(rState.maFillColor));
                    if( maCurrState.mnFillOpacity*maCurrState.mnOpacity != 1.0 )
                        xAttrs->AddAttribute( "draw:opacity",
                                              OUString::number(100.0*maCurrState.mnFillOpacity*maCurrState.mnOpacity)+"%" );
                }
            }
            else
                xAttrs->AddAttribute( "draw:fill", "none");

            if( rState.meStrokeType == SOLID )
            {
                xAttrs->AddAttribute( "draw:stroke", "solid");
                xAttrs->AddAttribute( "svg:stroke-color", getOdfColor(rState.maStrokeColor));
            }
            else if( rState.meStrokeType == DASH )
            {
                xAttrs->AddAttribute( "draw:stroke", "dash");
                xAttrs->AddAttribute( "draw:stroke-dash", "dash"+OUString::number(mnCurrStateId));
                xAttrs->AddAttribute( "svg:stroke-color", getOdfColor(rState.maStrokeColor));
            }
            else
                xAttrs->AddAttribute( "draw:stroke", "none");

            if( maCurrState.mnStrokeWidth != 0.0 )
            {
                ::basegfx::B2DVector aVec(maCurrState.mnStrokeWidth,0);
                aVec *= maCurrState.maCTM;
                xAttrs->AddAttribute( "svg:stroke-width", OUString::number( pt2mm(aVec.getLength()) )+"mm");
            }
            if( maCurrState.meLineJoin == basegfx::B2DLineJoin::Miter )
                xAttrs->AddAttribute( "draw:stroke-linejoin", "miter");
            else if( maCurrState.meLineJoin == basegfx::B2DLineJoin::Round )
                xAttrs->AddAttribute( "draw:stroke-linejoin", "round");
            else if( maCurrState.meLineJoin == basegfx::B2DLineJoin::Bevel )
                xAttrs->AddAttribute( "draw:stroke-linejoin", "bevel");
            if( maCurrState.mnStrokeOpacity*maCurrState.mnOpacity != 1.0 )
                xAttrs->AddAttribute( "svg:stroke-opacity",
                                      OUString::number(100.0*maCurrState.mnStrokeOpacity*maCurrState.mnOpacity)+"%");
        }

        mxDocumentHandler->startElement( "style:graphic-properties", xUnoAttrs );
        mxDocumentHandler->endElement( "style:graphic-properties" );
        mxDocumentHandler->endElement( "style:style" );

        return true; // newly written
    }

    void writeStyle(const uno::Reference<xml::dom::XElement>& xElem, const sal_Int32 nTagId)
    {
        SAL_INFO ("svg", "writeStyle xElem " << xElem->getTagName());

        sal_Int32 nStyleId=0;
        if( writeStyle(maCurrState, nTagId) )
            nStyleId = mnCurrStateId;
        else
            nStyleId = mrStates.find(maCurrState)->mnStyleId;

        xElem->setAttribute("internal-style-ref",
                            OUString::number(
                                nStyleId)
                            +"$0");
    }

    void push()
    {
        maParentStates.push_back(maCurrState);
    }

    void pop()
    {
        maParentStates.pop_back();
    }

    void parseLinearGradientData( Gradient& io_rCurrGradient,
                                  const sal_Int32 nGradientNumber,
                                  const sal_Int32 nTokenId,
                                  const OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_GRADIENTTRANSFORM:
            {
                OString aValueUtf8( sValue.getStr(),
                                         sValue.getLength(),
                                         RTL_TEXTENCODING_UTF8 );
                parseTransform(aValueUtf8.getStr(),io_rCurrGradient.maTransform);
                break;
            }
            case XML_X1:
                io_rCurrGradient.maCoords.linear.mfX1 = convLength(sValue,maCurrState,'h');
                break;
            case XML_X2:
                io_rCurrGradient.maCoords.linear.mfX2 = convLength(sValue,maCurrState,'h');
                break;
            case XML_Y1:
                io_rCurrGradient.maCoords.linear.mfY1 = convLength(sValue,maCurrState,'v');
                break;
            case XML_Y2:
                io_rCurrGradient.maCoords.linear.mfY2 = convLength(sValue,maCurrState,'v');
                break;
            case XML_ID:
                maGradientIdMap.insert(std::make_pair(sValue,nGradientNumber));
                break;
            case XML_GRADIENTUNITS:
                if (getTokenId(sValue) == XML_OBJECTBOUNDINGBOX)
                    io_rCurrGradient.mbBoundingBoxUnits = true;
                else
                    io_rCurrGradient.mbBoundingBoxUnits = false;
                break;
            default:
                break;
        }
    }

    void parseRadialGradientData( Gradient& io_rCurrGradient,
                                  const sal_Int32 nGradientNumber,
                                  const sal_Int32 nTokenId,
                                  const OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_GRADIENTTRANSFORM:
            {
                OString aValueUtf8( sValue.getStr(),
                                         sValue.getLength(),
                                         RTL_TEXTENCODING_UTF8 );
                parseTransform(aValueUtf8.getStr(),io_rCurrGradient.maTransform);
                break;
            }
            case XML_CX:
                io_rCurrGradient.maCoords.radial.mfCX = convLength(sValue,maCurrState,'h');
                break;
            case XML_CY:
                io_rCurrGradient.maCoords.radial.mfCY = convLength(sValue,maCurrState,'v');
                break;
            case XML_FX:
                io_rCurrGradient.maCoords.radial.mfFX = convLength(sValue,maCurrState,'h');
                break;
            case XML_FY:
                io_rCurrGradient.maCoords.radial.mfFY = convLength(sValue,maCurrState,'v');
                break;
            case XML_R:
                io_rCurrGradient.maCoords.radial.mfR = convLength(sValue,maCurrState,'r');
                break;
            case XML_ID:
                maGradientIdMap.insert(std::make_pair(sValue,nGradientNumber));
                break;
            case XML_GRADIENTUNITS:
                if (getTokenId(sValue) == XML_OBJECTBOUNDINGBOX)
                    io_rCurrGradient.mbBoundingBoxUnits = true;
                else
                    io_rCurrGradient.mbBoundingBoxUnits = false;
                break;
            default:
                break;
        }
    }

    void parseGradientStop( GradientStop& io_rGradientStop,
                            const sal_Int32 nStopNumber,
                            const sal_Int32 nTokenId,
                            const OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_HREF:
            {
                ElementRefMapType::iterator aFound=maStopIdMap.end();
                if ( sValue.copy(0,1) == "#" )
                    aFound = maStopIdMap.find(sValue.copy(1));
                else
                    aFound = maStopIdMap.find(sValue);

                if( aFound != maStopIdMap.end() )
                    io_rGradientStop =  maGradientStopVector[aFound->second];
                break;
            }
            case XML_ID:
                maStopIdMap.insert(std::make_pair(sValue,nStopNumber));
                break;
            case XML_STOP_COLOR:
                if( maGradientVector.empty() ||
                    maGradientVector.back().maStops.empty() )
                    break;
                parseColor( sValue.toUtf8().getStr(), maGradientStopVector[
                                  maGradientVector.back().maStops.back()].maStopColor );
                break;
            case XML_STOP_OPACITY:
                if( maGradientVector.empty() ||
                    maGradientVector.back().maStops.empty() )
                    break;
                parseOpacity( sValue.toUtf8().getStr(),
                              maGradientStopVector[
                                  maGradientVector.back().maStops.back()].maStopColor );
                break;
            case XML_OFFSET:
                io_rGradientStop.mnStopPosition = sValue.toDouble();
                break;
            case XML_STYLE:
                parseStyle( sValue );
                break;
            default:
                break;
        }
    }

    void parseAttribute( const sal_Int32      nTokenId,
                         const OUString& sValue )
    {
        OString aValueUtf8( sValue.getStr(),
                                 sValue.getLength(),
                                 RTL_TEXTENCODING_UTF8 );
        switch(nTokenId)
        {
            case XML_WIDTH:
            {
                const double fViewPortWidth(
                    convLength(sValue,maCurrState,'h'));

                maCurrState.maViewport.expand(
                    basegfx::B2DTuple(fViewPortWidth,0.0));
                break;
            }
            case XML_HEIGHT:
            {
                const double fViewPortHeight(
                    convLength(sValue,maCurrState,'v'));

                maCurrState.maViewport.expand(
                    basegfx::B2DTuple(0.0,fViewPortHeight));
                break;
            }
            case XML_VIEWBOX:
            {
                // TODO(F1): preserveAspectRatio
                parseViewBox(
                    aValueUtf8.getStr(),
                    maCurrState.maViewBox);
                break;
            }
            case XML_FILL_RULE:
            {
                if( aValueUtf8 == "evenodd" )
                    maCurrState.meFillRule = EVEN_ODD;
                else if( aValueUtf8 == "nonzero" )
                    maCurrState.meFillRule = NON_ZERO;
                else if( aValueUtf8 == "inherit" )
                    maCurrState.meFillRule = maParentStates.back().meFillRule;
                break;
            }
            case XML_OPACITY:
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnOpacity = maParentStates.back().mnOpacity;
                else
                    maCurrState.mnOpacity = aValueUtf8.toDouble();
                break;
            case XML_FILL_OPACITY:
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnFillOpacity = maParentStates.back().mnFillOpacity;
                else {
                    maCurrState.mnFillOpacity = aValueUtf8.toDouble();
                    if( maCurrState.mnFillOpacity > 1 )
                        maCurrState.mnFillOpacity = 1;
                }
                break;
            case XML_STROKE_WIDTH:
            {
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnStrokeWidth = maParentStates.back().mnStrokeWidth;
                else
                    maCurrState.mnStrokeWidth = convLength(sValue,maCurrState,'r');
                break;
            }
            case XML_STROKE_LINECAP:
            {
                if( aValueUtf8 == "butt" )
                    maCurrState.meLineCap = BUTT;
                else if( aValueUtf8 == "round" )
                    maCurrState.meLineCap = ROUND;
                else if( aValueUtf8 == "square" )
                    maCurrState.meLineCap = RECT;
                else if( aValueUtf8 == "inherit" )
                    maCurrState.meLineCap = maParentStates.back().meLineCap;
                break;
            }
            case XML_STROKE_LINEJOIN:
            {
                if( aValueUtf8 == "miter" )
                    maCurrState.meLineJoin = basegfx::B2DLineJoin::Miter;
                else if( aValueUtf8 == "round" )
                    maCurrState.meLineJoin = basegfx::B2DLineJoin::Round;
                else if( aValueUtf8 == "bevel" )
                    maCurrState.meLineJoin = basegfx::B2DLineJoin::Bevel;
                else if( aValueUtf8 == "inherit" )
                    maCurrState.meLineJoin = maParentStates.back().meLineJoin;
                break;
            }
            case XML_STROKE_MITERLIMIT:
            {
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnMiterLimit = maParentStates.back().mnMiterLimit;
                else
                    maCurrState.mnMiterLimit = aValueUtf8.toDouble();
                break;
            }
            case XML_STROKE_DASHOFFSET:
            {
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnDashOffset = maParentStates.back().mnDashOffset;
                else
                    maCurrState.mnDashOffset = convLength(sValue,maCurrState,'r');
                break;
            }
            case XML_STROKE_DASHARRAY:
            {
                if( aValueUtf8 == "none" )
                {
                    maCurrState.maDashArray.clear();
                    maCurrState.meStrokeType = SOLID;
                }
                else if( aValueUtf8 == "inherit" )
                    maCurrState.maDashArray = maParentStates.back().maDashArray;
                else
                {
                    if( parseDashArray(aValueUtf8.getStr(),
                                      maCurrState.maDashArray) )
                    {
                        maCurrState.meStrokeType = DASH;
                    }
                    else
                    {
                        maCurrState.meStrokeType = SOLID;
                    }
                }
                break;
            }
            case XML_STROKE_OPACITY:
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnStrokeOpacity = maParentStates.back().mnStrokeOpacity;
                else
                    maCurrState.mnStrokeOpacity = aValueUtf8.toDouble();
                break;
            case XML_FILL:
            {
                const State& rParent( maParentStates.back() );
                parsePaint( sValue,
                            aValueUtf8.getStr(),
                            maCurrState.meFillType,
                            maCurrState.maFillColor,
                            maCurrState.maFillGradient,
                            rParent.meFillType,
                            rParent.maFillColor,
                            rParent.maFillGradient );
                break;
            }
            case XML_STROKE:
            {
                const State& rParent( maParentStates.back() );
                parsePaint( sValue,
                            aValueUtf8.getStr(),
                            maCurrState.meStrokeType,
                            maCurrState.maStrokeColor,
                            maCurrState.maStrokeGradient,
                            rParent.meStrokeType,
                            rParent.maStrokeColor,
                            rParent.maStrokeGradient );
                break;
            }
            case XML_COLOR:
            {
                if( aValueUtf8 == "inherit" )
                    maCurrState.maCurrentColor = maParentStates.back().maCurrentColor;
                else
                    parseColor(aValueUtf8.getStr(), maCurrState.maCurrentColor);
                break;
            }
            case XML_TRANSFORM:
            {
                basegfx::B2DHomMatrix aTransform;
                parseTransform(aValueUtf8.getStr(),aTransform);
                maCurrState.maTransform = maCurrState.maTransform*aTransform;
                break;
            }
            case XML_FONT_FAMILY:
                maCurrState.maFontFamily=sValue;
                break;
            case XML_FONT_SIZE:
                maCurrState.mnParentFontSize=maParentStates.back().mnFontSize;
                maCurrState.mnFontSize=convLength(sValue,maCurrState,'v');
                break;
            case XML_FONT_STYLE:
                parseFontStyle(maCurrState,sValue,aValueUtf8.getStr());
                break;
            case XML_FONT_WEIGHT:
                maCurrState.mnFontWeight=sValue.toDouble();
                break;
            case XML_FONT_VARIANT:
                parseFontVariant(maCurrState,sValue,aValueUtf8.getStr());
                break;
            case XML_TEXT_ANCHOR:
                parseTextAlign(maCurrState,aValueUtf8.getStr());
                break;
            case XML_STOP_COLOR:
            case XML_STOP_OPACITY:
                parseGradientStop( maGradientStopVector.back(),
                                   maGradientStopVector.size()-1,
                                   nTokenId, sValue );
                break;
            case XML_TOKEN_INVALID:
                SAL_INFO("svg", "unhandled token");
                break;
            default:
                SAL_INFO("svg", "unhandled token " << getTokenName(nTokenId));
                break;
        }
    }

    void parseStyle( const OUString& sValue )
    {
        // split individual style attributes
        sal_Int32 nIndex=0, nDummyIndex=0;
        OUString aCurrToken;
        do
        {
            aCurrToken=sValue.getToken(0,';',nIndex);

            if( !aCurrToken.isEmpty() )
            {
                // split attrib & value
                nDummyIndex=0;
                OUString aCurrAttrib(
                    aCurrToken.getToken(0,':',nDummyIndex).trim());
                OSL_ASSERT(nDummyIndex!=-1);
                nDummyIndex=0;
                OUString aCurrValue(
                    aCurrToken.getToken(1,':',nDummyIndex).trim());
                OSL_ASSERT(nDummyIndex==-1);

                // recurse into normal attribute parsing
                parseAttribute( getTokenId(aCurrAttrib),
                                aCurrValue );
            }
        }
        while( nIndex != -1 );
    }

    static void parseFontStyle( State&               io_rInitialState,
                         const OUString& rValue,
                         const char*          sValue )
    {
        if( strcmp(sValue,"inherit") != 0 )
            io_rInitialState.maFontStyle = rValue;
    }

    static void parseFontVariant( State&               io_rInitialState,
                           const OUString& rValue,
                           const char*          sValue )
    {
        if( strcmp(sValue,"inherit") != 0 )
            io_rInitialState.maFontVariant = rValue;
    }

    static void parseTextAlign( State&      io_rInitialState,
                         const char* sValue )
    {
        if( strcmp(sValue,"start") == 0 )
            io_rInitialState.meTextAnchor = BEFORE;
        else if( strcmp(sValue,"middle") == 0 )
            io_rInitialState.meTextAnchor = CENTER;
        else if( strcmp(sValue,"end") == 0 )
            io_rInitialState.meTextAnchor = AFTER;
        // keep current val for sValue == "inherit"
    }

    void parsePaint( const OUString& rValue,
                     const char*      sValue,
                     PaintType&       rType,
                     ARGBColor&       rColor,
                     Gradient&        rGradient,
                     const PaintType& rInheritType,
                     const ARGBColor& rInheritColor,
                     const Gradient&  rInheritGradient )
    {
        std::pair<const char*,const char*> aPaintUri(nullptr,nullptr);
        std::pair<ARGBColor,bool>          aColor(maCurrState.maCurrentColor,
                                                  false);
        if( strcmp(sValue,"none") == 0 )
            rType = NONE;
        else if( strcmp(sValue,"currentColor") == 0 )
        {
            rType = SOLID;
            rColor = maCurrState.maCurrentColor;
        }
        else if( strcmp(sValue,"inherit") == 0)
        {
            rType = rInheritType;
            rColor = rInheritColor;
            rGradient = rInheritGradient;
        }
        else if( parsePaintUri(aPaintUri,aColor,sValue) )
        {
            if( aPaintUri.first != aPaintUri.second )
            {
                // assuming gradient. assumption does not hold generally
                if( strstr(sValue,")") && rValue.getLength() > 5 )
                {
                    ElementRefMapType::iterator aRes;
                    if( (aRes=maGradientIdMap.find(
                             rValue.copy(aPaintUri.first-sValue,
                                         aPaintUri.second-aPaintUri.first))) != maGradientIdMap.end() )
                    {
                        rGradient = maGradientVector[aRes->second];
                        rType = GRADIENT;
                    }
                }
            }
            else if( aColor.second )
            {
                rType = SOLID;
                rColor = aColor.first;
            }
            else
            {
                rType = NONE;
            }
        }
        else
        {
            rType = SOLID;
            parseColor(sValue,rColor);
        }
    }

    sal_Int32                                  mnCurrStateId;
    State                                      maCurrState;
    std::vector<State>                         maParentStates;
    StatePool&                                 mrStates;
    StateMap&                                  mrStateMap;
    uno::Reference<xml::sax::XDocumentHandler> mxDocumentHandler;
    std::vector< Gradient >                    maGradientVector;
    std::vector< GradientStop >                maGradientStopVector;
    std::vector< uno::Reference<xml::dom::XElement> > maElementVector;
    std::vector< uno::Reference<xml::dom::XElement> >& mrUseElementVector;
    ElementRefMapType                          maGradientIdMap;
    ElementRefMapType                          maStopIdMap;
    ElementRefMapType                          maElementIdMap;
    bool&                                      mrGradientNotFound;
};

/// Annotate svg styles with unique references to state pool
void annotateStyles( StatePool&                                        rStatePool,
                            StateMap&                                         rStateMap,
                            const State&                                       rInitialState,
                            uno::Reference<xml::dom::XElement>&         rElem,
                            const uno::Reference<xml::sax::XDocumentHandler>& xDocHdl,
                            std::vector< uno::Reference<xml::dom::XElement> >& rUseElementVector )
{
    bool bGradientNotFound = false;
    AnnotatingVisitor aVisitor(rStatePool, rStateMap, rInitialState, xDocHdl, rUseElementVector, bGradientNotFound);
    visitElements(aVisitor, rElem, STYLE_ANNOTATOR);

    //Sometimes, xlink:href in gradients refers to another gradient which hasn't been parsed yet.
    // if that happens, we'll need to parse the styles again, so everything gets referred.
    if( bGradientNotFound )
    {
        visitElements(aVisitor, rElem, STYLE_ANNOTATOR);
    }
}

struct ShapeWritingVisitor
{
    ShapeWritingVisitor(StatePool&                                        /*rStatePool*/,
                        StateMap&                                         rStateMap,
                        const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler) :
        mrStateMap(rStateMap),
        mxDocumentHandler(xDocumentHandler),
        mnShapeNum(0)
    {}

    void operator()( const uno::Reference<xml::dom::XElement>& )
    {
    }

    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& xAttributes )
    {
        rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
        uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

        sal_Int32 nDummyIndex(0);
        OUString sStyleId(
            xElem->getAttribute("internal-style-ref").getToken(
                    0,'$',nDummyIndex));
        StateMap::iterator pOrigState=mrStateMap.find(
            sStyleId.toInt32());

        if( pOrigState == mrStateMap.end() )
            return; // non-exportable element, e.g. linearGradient

        maCurrState = pOrigState->second;

        const sal_Int32 nTokenId(getTokenId(xElem->getNodeName()));
        switch(nTokenId)
        {
            case XML_LINE:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                double x1=0.0,y1=0.0,x2=0.0,y2=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                        case XML_X1:
                            x1= convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_X2:
                            x2 = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_Y1:
                            y1 = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        case XML_Y2:
                            y2 = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        default:
                            // skip
                            break;
                    }
                }

                if ( x1 != x2 || y1 != y2 ) {
                    OUString sLinePath = "M"+OUString::number(x1)+","
                        +OUString::number(y1)+"L"+OUString::number(x2)+","
                        +OUString::number(y2);
                    basegfx::B2DPolyPolygon aPoly;
                    basegfx::tools::importFromSvgD(aPoly, sLinePath, false, nullptr);

                    writePathShape(xAttrs,
                                   xUnoAttrs,
                                   xElem,
                                   sStyleId,
                                   basegfx::B2DPolyPolygon(aPoly));
                }

                break;
            }
            case XML_POLYGON:
            case XML_POLYLINE:
            {
                OUString sPoints = xElem->hasAttribute("points") ? xElem->getAttribute("points") : "";
                basegfx::B2DPolygon aPoly;
                (void)basegfx::tools::importFromSvgPoints(aPoly, sPoints);
                if( nTokenId == XML_POLYGON || maCurrState.meFillType != NONE )
                    aPoly.setClosed(true);

                writePathShape(xAttrs,
                               xUnoAttrs,
                               xElem,
                               sStyleId,
                               basegfx::B2DPolyPolygon(aPoly));
                break;
            }
            case XML_RECT:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                bool bRxSeen=false, bRySeen=false;
                double x=0.0,y=0.0,width=0.0,height=0.0,rx=0.0,ry=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                                              getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                    case XML_X:
                        x = convLength(sAttributeValue,maCurrState,'h');
                        break;
                    case XML_Y:
                        y = convLength(sAttributeValue,maCurrState,'v');
                        break;
                    case XML_WIDTH:
                        width = convLength(sAttributeValue,maCurrState,'h');
                        break;
                    case XML_HEIGHT:
                        height = convLength(sAttributeValue,maCurrState,'v');
                        break;
                    case XML_RX:
                        rx = convLength(sAttributeValue,maCurrState,'h');
                        bRxSeen=true;
                        break;
                    case XML_RY:
                        ry = convLength(sAttributeValue,maCurrState,'v');
                        bRySeen=true;
                        break;
                    default:
                        // skip
                        break;
                    }
                }

                if ( (width > 0) && (height > 0) ) {
                    if( bRxSeen && !bRySeen )
                        ry = rx;
                    else if( bRySeen && !bRxSeen )
                        rx = ry;

                    basegfx::B2DPolygon aPoly;
                    aPoly = basegfx::tools::createPolygonFromRect(
                                                                  basegfx::B2DRange(x,y,x+width,y+height),
                                                                  rx/(0.5*width), ry/(0.5*height) );

                    writePathShape(xAttrs,
                                   xUnoAttrs,
                                   xElem,
                                   sStyleId,
                                   basegfx::B2DPolyPolygon(aPoly));
                }
                break;
            }
            case XML_PATH:
            {
                OUString sPath = xElem->hasAttribute("d") ? xElem->getAttribute("d") : "";
                basegfx::B2DPolyPolygon aPoly;
                basegfx::tools::importFromSvgD(aPoly, sPath, false, nullptr);

                if ((maCurrState.meStrokeType == NONE) &&
                    (maCurrState.meFillType != NONE) &&
                    !aPoly.isClosed())
                {
                    aPoly.setClosed(true);
                }

                // tdf#51165: rendering of paths with open and closed polygons is not implemented
                // split mixed polypolygons into single polygons and add them one by one
                if( PolyPolygonIsMixedOpenAndClosed(aPoly) )
                {
                    for( sal_uInt32 i(0L); i<aPoly.count(); ++i ) {
                        writePathShape(xAttrs,
                                       xUnoAttrs,
                                       xElem,
                                       sStyleId,
                                       basegfx::B2DPolyPolygon(aPoly.getB2DPolygon(i)));
                    }
                }
                else
                {
                    writePathShape(xAttrs,
                                   xUnoAttrs,
                                   xElem,
                                   sStyleId,
                                   aPoly);
                }
                break;
            }
            case XML_CIRCLE:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                double cx=0.0,cy=0.0,r=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                        case XML_CX:
                            cx = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_CY:
                            cy = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        case XML_R:
                            r = convLength(sAttributeValue,maCurrState,'r');
                            break;
                        default:
                            // skip
                            break;
                    }
                }

                if ( r > 0 )
                    writeEllipseShape(xAttrs,
                                      xUnoAttrs,
                                      xElem,
                                      sStyleId,
                                      basegfx::B2DEllipse(basegfx::B2DPoint(cx, cy), basegfx::B2DTuple(r,r)));
                break;
            }
            case XML_ELLIPSE:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                double cx=0.0,cy=0.0,rx=0.0, ry=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                        case XML_CX:
                            cx = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_CY:
                            cy = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        case XML_RX:
                            rx = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_RY:
                            ry = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        default:
                            // skip
                            break;
                    }
                }

                if ( rx > 0 && ry > 0 )
                    writeEllipseShape(xAttrs,
                                      xUnoAttrs,
                                      xElem,
                                      sStyleId,
                                      basegfx::B2DEllipse(basegfx::B2DPoint(cx, cy), basegfx::B2DTuple(rx,ry)));
                break;
            }
            case XML_IMAGE:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                double x=0.0, y=0.0, width=0.0, height=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                        case XML_X:
                            x = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_Y:
                            y = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        case XML_WIDTH:
                            width = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_HEIGHT:
                            height = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        default:
                            // skip
                            break;
                    }
                }
                // extract basic transformations out of CTM
                basegfx::B2DTuple aScale, aTranslate;
                double fRotate, fShearX;
                if (maCurrState.maCTM.decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    // apply transform
                    x *= aScale.getX();
                    width *= aScale.getX();
                    y *= aScale.getY();
                    height *= aScale.getY();
                    x += aTranslate.getX();
                    y += aTranslate.getY();
                    //TODO: Rotate
                }

                OUString sValue = xElem->hasAttribute("href") ? xElem->getAttribute("href") : "";
                OString aValueUtf8( sValue.getStr(), sValue.getLength(), RTL_TEXTENCODING_UTF8 );
                OUString sLinkValue;
                parseXlinkHref(aValueUtf8.getStr(), sLinkValue);

                if (!sLinkValue.isEmpty())
                    writeBinaryData(xAttrs, xUnoAttrs, xElem, basegfx::B2DRange(x,y,x+width,y+height), sLinkValue);
                break;
            }
            case XML_TSPAN:
            case XML_TEXT:
            {
                // collect text from all TEXT_NODE children into sText
                OUStringBuffer sText;
                visitChildren(
                    [&sText] (xml::dom::XNode & rNode) {
                        return sText.append(rNode.getNodeValue());
                    },
                              xElem,
                              xml::dom::NodeType_TEXT_NODE);

                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                OUString sAttributeValue;
                double x=0.0,y=0.0;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nAttribId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    switch(nAttribId)
                    {
                        case XML_X:
                            x = convLength(sAttributeValue,maCurrState,'h');
                            break;
                        case XML_Y:
                            y = convLength(sAttributeValue,maCurrState,'v');
                            break;
                        default:
                            // skip
                            break;
                    }
                }

                // actually export text
                xAttrs->Clear();


                // extract basic transformations out of CTM
                basegfx::B2DTuple aScale, aTranslate;
                double fRotate, fShearX;
                if (maCurrState.maCTM.decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    // some heuristic attempts to have text output
                    // baseline-relative
                    y -= 2.0*maCurrState.mnFontSize/aScale.getX()/3.0;
                    // apply transform
                    x *= aScale.getX();
                    y *= aScale.getY();
                    x += aTranslate.getX();
                    y += aTranslate.getY();
                    //TODO: Rotate
                }
                else {
                    // some heuristic attempts to have text output
                    // baseline-relative
                    y -= 2.0*maCurrState.mnFontSize/3.0;
                }

                xAttrs->AddAttribute( "svg:x", OUString::number(pt2mm(x))+"mm");
                xAttrs->AddAttribute( "svg:y", OUString::number(pt2mm(y))+"mm");
                xAttrs->AddAttribute( "draw:style-name", "svggraphicstyle"+sStyleId );

                mxDocumentHandler->startElement("draw:frame", xUnoAttrs);

                xAttrs->Clear();
                mxDocumentHandler->startElement("draw:text-box", xUnoAttrs);
                xAttrs->AddAttribute( "text:style-name", "svgparagraphstyle"+sStyleId);
                mxDocumentHandler->startElement("text:p", xUnoAttrs);

                xAttrs->Clear();
                xAttrs->AddAttribute( "text:style-name", "svgtextstyle"+sStyleId);
                mxDocumentHandler->startElement("text:span", xUnoAttrs);

                xAttrs->Clear();
                mxDocumentHandler->characters(sText.makeStringAndClear());
                mxDocumentHandler->endElement("text:span");
                mxDocumentHandler->endElement("text:p");
                mxDocumentHandler->endElement("draw:text-box");
                mxDocumentHandler->endElement("draw:frame");
                break;
            }
        }
    }

    static void push()
    {}

    static void pop()
    {}

    void writeBinaryData( rtl::Reference<SvXMLAttributeList>&           xAttrs,
                        const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                        const uno::Reference<xml::dom::XElement>&       /* xElem */,
                        const basegfx::B2DRange&                        rShapeBounds,
                        const OUString&                                 data)
    {
        xAttrs->Clear();
        xAttrs->AddAttribute( "svg:x", OUString::number(pt2mm(rShapeBounds.getMinX()))+"mm");
        xAttrs->AddAttribute( "svg:y", OUString::number(pt2mm(rShapeBounds.getMinY()))+"mm");
        xAttrs->AddAttribute( "svg:width", OUString::number(pt2mm(rShapeBounds.getWidth()))+"mm");
        xAttrs->AddAttribute( "svg:height", OUString::number(pt2mm(rShapeBounds.getHeight()))+"mm");

        mxDocumentHandler->startElement("draw:frame", xUnoAttrs);

        xAttrs->Clear();
        mxDocumentHandler->startElement("draw:image", xUnoAttrs);

        mxDocumentHandler->startElement("office:binary-data", xUnoAttrs);

        mxDocumentHandler->characters(data);

        mxDocumentHandler->endElement("office:binary-data");

        mxDocumentHandler->endElement("draw:image");

        mxDocumentHandler->endElement("draw:frame");
    }


    void writeEllipseShape( rtl::Reference<SvXMLAttributeList>&          xAttrs,
                         const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                         const uno::Reference<xml::dom::XElement>&       xElem,
                         const OUString&                            rStyleId,
                         const basegfx::B2DEllipse&                      rEllipse)
    {
        State aState = maCurrState;

        xAttrs->Clear();

        basegfx::B2DPolygon aPoly = basegfx::tools::createPolygonFromEllipse(rEllipse.getB2DEllipseCenter(),
            rEllipse.getB2DEllipseRadius().getX(), rEllipse.getB2DEllipseRadius().getY());
        writePathShape(xAttrs, xUnoAttrs, xElem, rStyleId, basegfx::B2DPolyPolygon(aPoly));

    }

    void writePathShape( rtl::Reference<SvXMLAttributeList>&             xAttrs,
                         const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                         const uno::Reference<xml::dom::XElement>&       xElem,
                         const OUString&                                 rStyleId,
                         const basegfx::B2DPolyPolygon&                  rPoly )
    {
        // we might need to split up polypolygon into multiple path
        // shapes (e.g. when emulating line stroking)
        std::vector<basegfx::B2DPolyPolygon> aPolys(1,rPoly);
        State aState = maCurrState;

        xAttrs->Clear();

        OSL_TRACE("writePath - the CTM is: %f %f %f %f %f %f",
                  maCurrState.maCTM.get(0,0),
                  maCurrState.maCTM.get(0,1),
                  maCurrState.maCTM.get(0,2),
                  maCurrState.maCTM.get(1,0),
                  maCurrState.maCTM.get(1,1),
                  maCurrState.maCTM.get(1,2));

        // TODO(F2): separate out shear, rotate etc.
        // apply transformation to polygon, to keep draw
        // import in 100th mm
        for (basegfx::B2DPolyPolygon & aPoly : aPolys)
        {
            aPoly.transform(aState.maCTM);
        }

        for(basegfx::B2DPolyPolygon & aPoly : aPolys)
        {
            const basegfx::B2DRange aBounds(
                aPoly.areControlPointsUsed() ?
                basegfx::tools::getRange(
                    basegfx::tools::adaptiveSubdivideByAngle(aPoly)) :
                basegfx::tools::getRange(aPoly));
            fillShapeProperties(xAttrs,
                                xElem,
                                aBounds,
                                "svggraphicstyle"+rStyleId);

            // force path coordinates to 100th millimeter, after
            // putting polygon data at origin (ODF viewbox
            // calculations largely untested codepaths, as OOo always
            // writes "0 0 w h" viewboxes)
            basegfx::B2DHomMatrix aNormalize;
            aNormalize.translate(-aBounds.getMinX(),-aBounds.getMinY());
            aNormalize.scale(2540.0/72.0,2540.0/72.0);
            aPoly.transform(aNormalize);

            xAttrs->AddAttribute( "svg:d", basegfx::tools::exportToSvgD(
                aPoly,
                false,   // no relative coords. causes rounding errors
                false,   // no quad bezier detection. crashes older versions.
                false ));
            mxDocumentHandler->startElement("draw:path", xUnoAttrs);
            mxDocumentHandler->endElement("draw:path");
        }
    }

    void fillShapeProperties( rtl::Reference<SvXMLAttributeList>&       xAttrs,
                              const uno::Reference<xml::dom::XElement>& /* xElem */,
                              const basegfx::B2DRange&                  rShapeBounds,
                              const OUString&                      rStyleName )
    {
        xAttrs->AddAttribute( "draw:z-index", OUString::number( mnShapeNum++ ));
        xAttrs->AddAttribute( "draw:style-name", rStyleName);
        xAttrs->AddAttribute( "svg:width", OUString::number(pt2mm(rShapeBounds.getWidth()))+"mm");
        xAttrs->AddAttribute( "svg:height", OUString::number(pt2mm(rShapeBounds.getHeight()))+"mm");

        // OOo expects the viewbox to be in 100th of mm
        xAttrs->AddAttribute( "svg:viewBox",
            "0 0 "
            + OUString::number(
                basegfx::fround(pt100thmm(rShapeBounds.getWidth())) )
            + " "
            + OUString::number(
                basegfx::fround(pt100thmm(rShapeBounds.getHeight())) ));

        // TODO(F1): decompose transformation in calling code, and use
        // transform attribute here
        // writeTranslate(maCurrState.maCTM, xAttrs);
        xAttrs->AddAttribute( "svg:x", OUString::number(pt2mm(rShapeBounds.getMinX()))+"mm");
        xAttrs->AddAttribute( "svg:y", OUString::number(pt2mm(rShapeBounds.getMinY()))+"mm");
    }

    State                                      maCurrState;
    StateMap&                                  mrStateMap;
    uno::Reference<xml::sax::XDocumentHandler> mxDocumentHandler;
    sal_Int32                                  mnShapeNum;
};

/// Write out shapes from DOM tree
void writeShapes( StatePool&                                        rStatePool,
                         StateMap&                                         rStateMap,
                         const uno::Reference<xml::dom::XElement>&         rElem,
                         const uno::Reference<xml::sax::XDocumentHandler>& xDocHdl,
                         std::vector< uno::Reference<xml::dom::XElement> >& rUseElementVector )
{
    ShapeWritingVisitor aVisitor(rStatePool,rStateMap,xDocHdl);
    visitElements(aVisitor, rElem, SHAPE_WRITER);

    std::vector< uno::Reference<xml::dom::XElement> >::iterator it;
    for ( it = rUseElementVector.begin() ; it != rUseElementVector.end(); ++it)
    {
        visitElements(aVisitor, *it, SHAPE_WRITER);
    }
}

} // namespace

struct OfficeStylesWritingVisitor
{
    OfficeStylesWritingVisitor( StateMap&                                         rStateMap,
                                const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler) :
        mrStateMap(rStateMap),
        mxDocumentHandler(xDocumentHandler)
    {}
    void operator()( const uno::Reference<xml::dom::XElement>& /*xElem*/ )
    {
    }
    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& /*xAttributes*/ )
    {
        rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
        uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

        sal_Int32 nDummyIndex(0);
        OUString sStyleId(
            xElem->getAttribute("internal-style-ref").getToken(
                    0,'$',nDummyIndex));
        StateMap::iterator pOrigState=mrStateMap.find(
            sStyleId.toInt32());

        if( pOrigState == mrStateMap.end() )
            return; // non-exportable element, e.g. linearGradient

        maCurrState = pOrigState->second;

        if( maCurrState.meStrokeType == DASH )
        {
            sal_Int32 dots1, dots2;
            double dots1_length, dots2_length, dash_distance;
            SvgDashArray2Odf( &dots1, &dots1_length, &dots2, &dots2_length, &dash_distance );

            xAttrs->Clear();
            xAttrs->AddAttribute( "draw:name", "dash"+sStyleId );
            xAttrs->AddAttribute( "draw:display-name", "dash"+sStyleId );
            xAttrs->AddAttribute( "draw:style", "rect" );
            if ( dots1>0 ) {
                xAttrs->AddAttribute( "draw:dots1", OUString::number(dots1) );
                xAttrs->AddAttribute( "draw:dots1-length", OUString::number(pt2mm(convLength( OUString::number(dots1_length), maCurrState, 'h' )))+"mm" );
            }
            xAttrs->AddAttribute( "draw:distance", OUString::number(pt2mm(convLength( OUString::number(dash_distance), maCurrState, 'h' )))+"mm" );
            if ( dots2>0 ) {
                xAttrs->AddAttribute( "draw:dots2", OUString::number(dots2) );
                xAttrs->AddAttribute( "draw:dots2-length", OUString::number(pt2mm(convLength( OUString::number(dots2_length), maCurrState, 'h' )))+"mm" );
            }

            mxDocumentHandler->startElement( "draw:stroke-dash", xUnoAttrs);
            mxDocumentHandler->endElement( "draw:stroke-dash" );
        }
    }

    void SvgDashArray2Odf( sal_Int32 *dots1, double *dots1_length, sal_Int32 *dots2, double *dots2_length, double *dash_distance )
    {
        *dots1 = 0;
        *dots1_length = 0;
        *dots2 = 0;
        *dots2_length = 0;
        *dash_distance = 0;

        if( maCurrState.maDashArray.empty() ) {
            return;
        }

        double effective_dasharray_size = maCurrState.maDashArray.size();
        if( maCurrState.maDashArray.size() % 2 == 1 )
            effective_dasharray_size = maCurrState.maDashArray.size()*2;

        *dash_distance = maCurrState.maDashArray[1%maCurrState.maDashArray.size()];
        sal_Int32 dist_count = 1;
        for( int i=3; i<effective_dasharray_size; i+=2 ) {
            *dash_distance = ((dist_count * *dash_distance) + maCurrState.maDashArray[i%maCurrState.maDashArray.size()])/(dist_count+1);
            ++dist_count;
        }

        *dots1 = 1;
        *dots1_length = maCurrState.maDashArray[0];
        int i=2;
        while( ( i<effective_dasharray_size ) && ( maCurrState.maDashArray[i%maCurrState.maDashArray.size()] == *dots1_length ) ) {
            ++(*dots1);
            i += 2;
        }
        if( i<effective_dasharray_size ) {
            *dots2 = 1;
            *dots2_length = maCurrState.maDashArray[i];
            i+=2;
            while( ( i<effective_dasharray_size ) && ( maCurrState.maDashArray[i%maCurrState.maDashArray.size()] == *dots2_length ) ) {
                ++(*dots2);
                i += 2;
            }
        }

        SAL_INFO("svg", "SvgDashArray2Odf " << *dash_distance << " " << *dots1 << " " << *dots1_length << " " << *dots2 << " " << *dots2_length );

        return;
    }

    static void push() {}
    static void pop()  {}

    State                                      maCurrState;
    StateMap&                                  mrStateMap;
    uno::Reference<xml::sax::XDocumentHandler> mxDocumentHandler;
};

static void writeOfficeStyles(  StateMap&                                         rStateMap,
                                const uno::Reference<xml::dom::XElement>&         rElem,
                                const uno::Reference<xml::sax::XDocumentHandler>& xDocHdl )
{
    OfficeStylesWritingVisitor aVisitor( rStateMap, xDocHdl );
    visitElements( aVisitor, rElem, STYLE_WRITER );

}

#ifdef DEBUG_FILTER_SVGREADER
struct DumpingVisitor
{
    void operator()( const uno::Reference<xml::dom::XElement>& xElem )
    {
        OSL_TRACE("name: %s",
                  OUStringToOString(
                      xElem->getTagName(),
                      RTL_TEXTENCODING_UTF8 ).getStr());
    }

    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& xAttributes )
    {
        OSL_TRACE("name: %s",
                  OUStringToOString(
                      xElem->getTagName(),
                      RTL_TEXTENCODING_UTF8 ).getStr());
        const sal_Int32 nNumAttrs( xAttributes->getLength() );
        for( sal_Int32 i=0; i<nNumAttrs; ++i )
        {
            OSL_TRACE(" %s=%s",
                      OUStringToOString(
                          xAttributes->item(i)->getNodeName(),
                          RTL_TEXTENCODING_UTF8 ).getStr(),
                      OUStringToOString(
                          xAttributes->item(i)->getNodeValue(),
                          RTL_TEXTENCODING_UTF8 ).getStr());
        }
    }

    void push() {}
    void pop()  {}
};

static void dumpTree( const uno::Reference<xml::dom::XElement> xElem )
{
    DumpingVisitor aVisitor;
    visitElements(aVisitor, xElem, STYLE_ANNOTATOR);
}
#endif


SVGReader::SVGReader(const uno::Reference<uno::XComponentContext>&     xContext,
                     const uno::Reference<io::XInputStream>&           xInputStream,
                     const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler) :
    m_xContext( xContext ),
    m_xInputStream( xInputStream ),
    m_xDocumentHandler( xDocumentHandler )
{
}

bool SVGReader::parseAndConvert()
{
    uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder = xml::dom::DocumentBuilder::create(m_xContext);

    uno::Reference<xml::dom::XDocument> xDom(
        xDomBuilder->parse(m_xInputStream),
        uno::UNO_QUERY_THROW );

    uno::Reference<xml::dom::XElement> xDocElem( xDom->getDocumentElement(),
                                                 uno::UNO_QUERY_THROW );

    // the root state for svg document
    State aInitialState;


    // doc boilerplate


    m_xDocumentHandler->startDocument();

    // get the document dimensions

    // if the "width" and "height" attributes are missing, inkscape fakes
    // A4 portrait for. Let's do the same.
    if (!xDocElem->hasAttribute("width"))
        xDocElem->setAttribute("width", "210mm");
    if (!xDocElem->hasAttribute("height"))
        xDocElem->setAttribute("height", "297mm");

    double fViewPortWidth( pt2mm(convLength(xDocElem->getAttribute("width"),aInitialState,'h')) );
    double fViewPortHeight( pt2mm(convLength(xDocElem->getAttribute("height"),aInitialState,'v')) );

    // document prolog
    rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
    uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

    xAttrs->AddAttribute( "xmlns:office", OASIS_STR "office:1.0" );
    xAttrs->AddAttribute( "xmlns:style", OASIS_STR "style:1.0" );
    xAttrs->AddAttribute( "xmlns:text", OASIS_STR "text:1.0" );
    xAttrs->AddAttribute( "xmlns:svg", OASIS_STR "svg-compatible:1.0" );
    xAttrs->AddAttribute( "xmlns:table", OASIS_STR "table:1.0" );
    xAttrs->AddAttribute( "xmlns:draw", OASIS_STR "drawing:1.0" );
    xAttrs->AddAttribute( "xmlns:fo", OASIS_STR "xsl-fo-compatible:1.0" );
    xAttrs->AddAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink");
    xAttrs->AddAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/");
    xAttrs->AddAttribute( "xmlns:number", OASIS_STR "datastyle:1.0" );
    xAttrs->AddAttribute( "xmlns:presentation", OASIS_STR "presentation:1.0" );
    xAttrs->AddAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML");
    xAttrs->AddAttribute( "xmlns:form", OASIS_STR "form:1.0" );
    xAttrs->AddAttribute( "xmlns:script", OASIS_STR "script:1.0" );
    xAttrs->AddAttribute( "xmlns:config", OASIS_STR "config:1.0" );
    xAttrs->AddAttribute( "xmlns:dom", "http://www.w3.org/2001/xml-events");
    xAttrs->AddAttribute( "xmlns:xforms", "http://www.w3.org/2002/xforms");
    xAttrs->AddAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
    xAttrs->AddAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    xAttrs->AddAttribute( "office:version", "1.0");
    xAttrs->AddAttribute( "office:mimetype", "application/vnd.oasis.opendocument.graphics");

    m_xDocumentHandler->startElement( "office:document", xUnoAttrs );

    xAttrs->Clear();

    m_xDocumentHandler->startElement( "office:settings", xUnoAttrs);

    xAttrs->AddAttribute( "config:name", "ooo:view-settings");
    m_xDocumentHandler->startElement( "config:config-item-set", xUnoAttrs);

    xAttrs->Clear();

    xAttrs->AddAttribute( "config:name", "VisibleAreaTop");
    xAttrs->AddAttribute( "config:type", "int");
    m_xDocumentHandler->startElement( "config:config-item", xUnoAttrs);

    m_xDocumentHandler->characters( "0" );

    m_xDocumentHandler->endElement( "config:config-item" );

    xAttrs->Clear();

    xAttrs->AddAttribute( "config:name", "VisibleAreaLeft" );
    xAttrs->AddAttribute( "config:type", "int" );
    m_xDocumentHandler->startElement( "config:config-item" , xUnoAttrs);

    m_xDocumentHandler->characters( "0" );

    m_xDocumentHandler->endElement( "config:config-item" );

    xAttrs->Clear();

    xAttrs->AddAttribute( "config:name" , "VisibleAreaWidth" );
    xAttrs->AddAttribute( "config:type" , "int" );
    m_xDocumentHandler->startElement( "config:config-item" , xUnoAttrs);

    sal_Int64 iWidth = sal_Int64(fViewPortWidth);
    m_xDocumentHandler->characters( OUString::number(iWidth) );

    m_xDocumentHandler->endElement( "config:config-item" );

    xAttrs->Clear();

    xAttrs->AddAttribute( "config:name", "VisibleAreaHeight" );
    xAttrs->AddAttribute( "config:type", "int" );
    m_xDocumentHandler->startElement( "config:config-item", xUnoAttrs);

    sal_Int64 iHeight = sal_Int64(fViewPortHeight);
    m_xDocumentHandler->characters( OUString::number(iHeight) );

    m_xDocumentHandler->endElement( "config:config-item" );

    m_xDocumentHandler->endElement( "config:config-item-set" );

    m_xDocumentHandler->endElement( "office:settings" );

    xAttrs->Clear();

    m_xDocumentHandler->startElement( "office:automatic-styles",
                                      xUnoAttrs );

    xAttrs->AddAttribute( "style:name", "pagelayout1");
    m_xDocumentHandler->startElement( "style:page-layout", xUnoAttrs );
    // TODO(Q3): this is super-ugly. In-place container come to mind.
    xAttrs->Clear();

    // make page viewport-width times viewport-height mm large - add
    // 5% border at every side
    xAttrs->AddAttribute( "fo:margin-top", "0mm");
    xAttrs->AddAttribute( "fo:margin-bottom", "0mm");
    xAttrs->AddAttribute( "fo:margin-left", "0mm");
    xAttrs->AddAttribute( "fo:margin-right", "0mm");
    xAttrs->AddAttribute( "fo:page-width", OUString::number(fViewPortWidth)+"mm");
    xAttrs->AddAttribute( "fo:page-height", OUString::number(fViewPortHeight)+"mm");
    xAttrs->AddAttribute( "style:print-orientation",
        fViewPortWidth > fViewPortHeight ? OUString("landscape") : OUString("portrait") );
    m_xDocumentHandler->startElement( "style:page-layout-properties", xUnoAttrs );
    m_xDocumentHandler->endElement( "style:page-layout-properties" );
    m_xDocumentHandler->endElement( "style:page-layout" );

    xAttrs->Clear();
    xAttrs->AddAttribute( "style:name", "pagestyle1" );
    xAttrs->AddAttribute( "style:family", "drawing-page" );
    m_xDocumentHandler->startElement( "style:style", xUnoAttrs );

    xAttrs->Clear();
    xAttrs->AddAttribute( "draw:background-size", "border");
    xAttrs->AddAttribute( "draw:fill", "none");
    m_xDocumentHandler->startElement( "style:drawing-page-properties", xUnoAttrs );
    m_xDocumentHandler->endElement( "style:drawing-page-properties" );
    m_xDocumentHandler->endElement( "style:style" );

    StatePool aStatePool;
    StateMap  aStateMap;
    std::vector< uno::Reference<xml::dom::XElement> > aUseElementVector;

    annotateStyles(aStatePool,aStateMap,aInitialState,
                   xDocElem,m_xDocumentHandler,aUseElementVector);

#ifdef DEBUG_FILTER_SVGREADER
    dumpTree(xDocElem);
#endif

    m_xDocumentHandler->endElement( "office:automatic-styles" );

    xAttrs->Clear();
    m_xDocumentHandler->startElement( "office:styles", xUnoAttrs);
    writeOfficeStyles( aStateMap,
                       xDocElem,
                       m_xDocumentHandler);
    m_xDocumentHandler->endElement( "office:styles" );


    m_xDocumentHandler->startElement( "office:master-styles", xUnoAttrs );
    xAttrs->Clear();
    xAttrs->AddAttribute( "style:name", "Default");
    xAttrs->AddAttribute( "style:page-layout-name", "pagelayout1");
    xAttrs->AddAttribute( "draw:style-name", "pagestyle1");
    m_xDocumentHandler->startElement( "style:master-page", xUnoAttrs );
    m_xDocumentHandler->endElement( "style:master-page" );

    m_xDocumentHandler->endElement( "office:master-styles" );


    xAttrs->Clear();
    m_xDocumentHandler->startElement( "office:body", xUnoAttrs );
    m_xDocumentHandler->startElement( "office:drawing", xUnoAttrs );

    xAttrs->Clear();
    xAttrs->AddAttribute( "draw:master-page-name", "Default");
    xAttrs->AddAttribute( "draw:style-name", "pagestyle1");
    m_xDocumentHandler->startElement("draw:page", xUnoAttrs);

    // write out all shapes
    writeShapes(aStatePool,
                aStateMap,
                xDocElem,
                m_xDocumentHandler,
                aUseElementVector);

    m_xDocumentHandler->endElement( "draw:page" );
    m_xDocumentHandler->endElement( "office:drawing" );
    m_xDocumentHandler->endElement( "office:body" );
    m_xDocumentHandler->endElement( "office:document" );
    m_xDocumentHandler->endDocument();

    return true;
}

} // namespace svgi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
