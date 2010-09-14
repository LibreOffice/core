/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

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
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <boost/bind.hpp>
#include <hash_set>
#include <map>
#include <string.h>

#define USTR(x) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )
#define OASIS_STR "urn:oasis:names:tc:opendocument:xmlns:"

using namespace ::com::sun::star;

namespace svgi
{
namespace
{

/** visits all children of the specified type with the given functor
 */
template<typename Func> void visitChildren(const Func& rFunc,
                                           const uno::Reference<xml::dom::XElement> xElem,
                                           xml::dom::NodeType eChildType )
{
    uno::Reference<xml::dom::XNodeList> xChildren( xElem->getChildNodes() );
    const sal_Int32 nNumNodes( xChildren->getLength() );
    for( sal_Int32 i=0; i<nNumNodes; ++i )
    {
        if( xChildren->item(i)->getNodeType() == eChildType )
            rFunc( *xChildren->item(i).get() );
    }
}

/** Visit all elements of the given tree (in-order traversal)

    Given functor is called for every element, and passed the
    element's attributes, if any
 */
template<typename Func> void visitElements(Func& rFunc,
                                           const uno::Reference<xml::dom::XElement> xElem)
{
    if( xElem->hasAttributes() )
        rFunc(xElem,xElem->getAttributes());
    else
        rFunc(xElem);

    // notify children processing
    rFunc.push();

    // recurse over children
    uno::Reference<xml::dom::XNodeList> xChildren( xElem->getChildNodes() );
    const sal_Int32 nNumNodes( xChildren->getLength() );
    for( sal_Int32 i=0; i<nNumNodes; ++i )
    {
        if( xChildren->item(i)->getNodeType() == xml::dom::NodeType_ELEMENT_NODE )
            visitElements( rFunc,
                           uno::Reference<xml::dom::XElement>(
                               xChildren->item(i),
                               uno::UNO_QUERY_THROW) );
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

typedef std::map<rtl::OUString,sal_Size> ElementRefMapType;

struct AnnotatingVisitor
{
    AnnotatingVisitor(StatePool&                                        rStatePool,
                      StateMap&                                         rStateMap,
                      const State&                                       rInitialState,
                      const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler) :
        mnCurrStateId(0),
        maCurrState(),
        maParentStates(),
        mrStates(rStatePool),
        mrStateMap(rStateMap),
        mxDocumentHandler(xDocumentHandler),
        maGradientVector(),
        maGradientStopVector()
    {
        maParentStates.push_back(rInitialState);
    }

    void operator()( const uno::Reference<xml::dom::XElement>& )
    {}

    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& xAttributes )
    {
        const sal_Int32 nTagId(getTokenId(xElem->getTagName()));
        switch (nTagId)
        {
            case XML_LINEARGRADIENT:
            {
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                rtl::OUString sAttributeValue;
                maGradientVector.push_back(Gradient(Gradient::LINEAR));

                // do we have a reference to a parent gradient? parse
                // that first, as it sets our defaults here (manually
                // tracking default state on each Gradient variable is
                // much more overhead)
                uno::Reference<xml::dom::XNode> xNode(xAttributes->getNamedItem(USTR("href")));
                if(xNode.is())
                {
                    const rtl::OUString sValue(xNode->getNodeValue());
                    ElementRefMapType::iterator aFound=maGradientIdMap.end();
                    if (sValue.copy(0,1).equalsAscii("#"))
                        aFound = maGradientIdMap.find(sValue.copy(1));
                    else
                        aFound = maGradientIdMap.find(sValue);;

                    if( aFound != maGradientIdMap.end() )
                        maGradientVector.back() = maGradientVector[aFound->second];
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
                rtl::OUString sAttributeValue;
                maGradientVector.push_back(Gradient(Gradient::RADIAL));

                // do we have a reference to a parent gradient? parse
                // that first, as it sets our defaults here (manually
                // tracking default state on each Gradient variable is
                // much more overhead)
                uno::Reference<xml::dom::XNode> xNode(xAttributes->getNamedItem(USTR("href")));
                if(xNode.is())
                {
                    const rtl::OUString sValue(xNode->getNodeValue());
                    ElementRefMapType::iterator aFound=maGradientIdMap.end();
                    if (sValue.copy(0,1).equalsAscii("#"))
                        aFound = maGradientIdMap.find(sValue.copy(1));
                    else
                        aFound = maGradientIdMap.find(sValue);;

                    if( aFound != maGradientIdMap.end() )
                        maGradientVector.back() = maGradientVector[aFound->second];
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
            case XML_STOP:
            {
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                rtl::OUString sAttributeValue;
                maGradientStopVector.push_back(GradientStop());
                maGradientVector.back().maStops.push_back(maGradientStopVector.size()-1);
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    parseGradientStop( maGradientStopVector.back(),
                                       maGradientStopVector.size()-1,
                                       getTokenId(xAttributes->item(i)->getNodeName()),
                                       xAttributes->item(i)->getNodeValue() );
                }
                break;
            }
            default:
            {
                // init state. inherit defaults from parent.
                maCurrState = maParentStates.back();
                maCurrState.maTransform.identity();

                // scan for style info
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                rtl::OUString sAttributeValue;
                for( sal_Int32 i=0; i<nNumAttrs; ++i )
                {
                    sAttributeValue = xAttributes->item(i)->getNodeValue();
                    const sal_Int32 nTokenId(
                        getTokenId(xAttributes->item(i)->getNodeName()));
                    if( XML_STYLE == nTokenId )
                        parseStyle(sAttributeValue);
                    else
                        parseAttribute(nTokenId,
                                       sAttributeValue);
                }

                // all attributes parsed, can calc total CTM now
                basegfx::B2DHomMatrix aLocalTransform;
                if( !maCurrState.maViewBox.isEmpty() &&
                    maCurrState.maViewBox.getWidth() != 0.0 &&
                    maCurrState.maViewBox.getHeight() != 0.0 )
                {
                    // transform aViewBox into viewport, such that they
                    // coincide
                    aLocalTransform.translate(-maCurrState.maViewBox.getMinX(),
                                              -maCurrState.maViewBox.getMinY());
                    aLocalTransform.scale(maCurrState.maViewport.getWidth()/maCurrState.maViewBox.getWidth(),
                                          maCurrState.maViewport.getHeight()/maCurrState.maViewBox.getHeight());
                }
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

    rtl::OUString getStyleName( const char* sPrefix, sal_Int32 nId )
    {
        return rtl::OUString::createFromAscii(sPrefix)+rtl::OUString::valueOf(nId);
    }

    bool hasGradientOpacity( const Gradient& rGradient )
    {
        return
            maGradientStopVector[
                rGradient.maStops[0]].maStopColor.a != 1.0 ||
            maGradientStopVector[
                rGradient.maStops[1]].maStopColor.a != 1.0;
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
        std::vector<sal_Size> aNewStops(1,rGradient.maStops.front());
        for( sal_Size i=1; i<rGradient.maStops.size(); ++i )
        {
            if( maGradientStopVector[rGradient.maStops[i]].maStopColor !=
                maGradientStopVector[aNewStops.back()].maStopColor )
                aNewStops.push_back(rGradient.maStops[i]);
        }

        rGradient.maStops = aNewStops;

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

    sal_Int8 toByteColor( double val )
    {
        // TODO(Q3): duplicated from vcl::unotools
        return sal::static_int_cast<sal_Int8>(
            basegfx::fround(val*255.0));
    }

    rtl::OUString getOdfColor( const ARGBColor& rColor )
    {
        // TODO(Q3): duplicated from pdfimport
        rtl::OUStringBuffer aBuf( 7 );
        const sal_uInt8 nRed  ( toByteColor(rColor.r)   );
        const sal_uInt8 nGreen( toByteColor(rColor.g) );
        const sal_uInt8 nBlue ( toByteColor(rColor.b)  );
        aBuf.append( sal_Unicode('#') );
        if( nRed < 10 )
            aBuf.append( sal_Unicode('0') );
        aBuf.append( sal_Int32(nRed), 16 );
        if( nGreen < 10 )
            aBuf.append( sal_Unicode('0') );
        aBuf.append( sal_Int32(nGreen), 16 );
        if( nBlue < 10 )
            aBuf.append( sal_Unicode('0') );
        aBuf.append( sal_Int32(nBlue), 16 );

        // TODO(F3): respect alpha transparency (polygons etc.)
        OSL_ASSERT(rColor.a == 1.0);

        return aBuf.makeStringAndClear();
    }

    bool writeStyle(State& rState, const sal_Int32 nTagId)
    {
        rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
        uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

        std::pair<StatePool::iterator,
                  bool> aRes = mrStates.insert(rState);
        if( !aRes.second )
            return false; // not written

        ++mnCurrStateId;

        // mnStyleId does not take part in hashing/comparison
        const_cast<State&>(*aRes.first).mnStyleId = mnCurrStateId;
        mrStateMap.insert(std::make_pair(
                              mnCurrStateId,
                              rState));

        // find two representative stop colors (as odf only support
        // start&end color)
        optimizeGradientStops(rState.maFillGradient);

        // do we have a gradient fill? then write out gradient as well
        if( rState.meFillType == GRADIENT && rState.maFillGradient.maStops.size() > 1 )
        {
            // TODO(F3): ODF12 supposedly also groks svg:linear/radialGradient
            xAttrs->AddAttribute( USTR( "draw:name" ), getStyleName("svggradient", rState.maFillGradient.mnId) );
            if( rState.maFillGradient.meType == Gradient::LINEAR )
            {
                // should the optimizeGradientStops method decide that
                // this is a three-color gradient, it prolly wanted us
                // to take axial instead
                xAttrs->AddAttribute( USTR( "draw:style" ),
                                      rState.maFillGradient.maStops.size() == 3 ?
                                      USTR("axial") :
                                      USTR("linear") );
            }
            else
            {
                xAttrs->AddAttribute( USTR( "draw:style" ), USTR("ellipsoid") );
                xAttrs->AddAttribute( USTR( "draw:cx" ), USTR("50%") );
                xAttrs->AddAttribute( USTR( "draw:cy" ), USTR("50%") );
            }

            basegfx::B2DTuple rScale, rTranslate;
            double rRotate, rShearX;
            if( rState.maFillGradient.maTransform.decompose(rScale, rTranslate, rRotate, rShearX) )
                xAttrs->AddAttribute( USTR( "draw:angle" ),
                                      rtl::OUString::valueOf(rRotate*1800.0/M_PI ) );
            xAttrs->AddAttribute( USTR( "draw:start-color" ),
                                  getOdfColor(
                                      maGradientStopVector[
                                          rState.maFillGradient.maStops[0]].maStopColor) );
            xAttrs->AddAttribute( USTR( "draw:end-color" ),
                                  getOdfColor(
                                      maGradientStopVector[
                                          rState.maFillGradient.maStops[1]].maStopColor) );
            xAttrs->AddAttribute( USTR( "draw:border" ), USTR("0%") );
            mxDocumentHandler->startElement( USTR("draw:gradient"),
                                             xUnoAttrs );
            mxDocumentHandler->endElement( USTR("draw:gradient") );

            if( hasGradientOpacity(rState.maFillGradient) )
            {
                // need to write out opacity style as well
                xAttrs->Clear();
                xAttrs->AddAttribute( USTR( "draw:name" ), getStyleName("svgopacity", rState.maFillGradient.mnId) );
                if( rState.maFillGradient.meType == Gradient::LINEAR )
                {
                    xAttrs->AddAttribute( USTR( "draw:style" ), USTR("linear") );
                }
                else
                {
                    xAttrs->AddAttribute( USTR( "draw:style" ), USTR("ellipsoid") );
                    xAttrs->AddAttribute( USTR( "draw:cx" ), USTR("50%") );
                    xAttrs->AddAttribute( USTR( "draw:cy" ), USTR("50%") );
                }

                // modulate gradient opacity with overall fill opacity
                xAttrs->AddAttribute( USTR( "draw:end" ),
                                      rtl::OUString::valueOf(
                                          maGradientStopVector[
                                              rState.maFillGradient.maStops[0]].maStopColor.a*
                                          maCurrState.mnFillOpacity*100.0)+USTR("%" ) );
                xAttrs->AddAttribute( USTR( "draw:start" ),
                                      rtl::OUString::valueOf(
                                          maGradientStopVector[
                                              rState.maFillGradient.maStops[1]].maStopColor.a*
                                          maCurrState.mnFillOpacity*100.0)+USTR("%" ) );
                xAttrs->AddAttribute( USTR( "draw:border" ), USTR("0%") );
                mxDocumentHandler->startElement( USTR("draw:opacity"),
                                                 xUnoAttrs );
                mxDocumentHandler->endElement( USTR("draw:opacity") );
            }
        }

        // serialize to automatic-style section
        xAttrs->Clear();
        xAttrs->AddAttribute( USTR( "style:name" ), getStyleName("svggraphicstyle", mnCurrStateId) );
        xAttrs->AddAttribute( USTR( "style:family" ), USTR("graphic") );
        mxDocumentHandler->startElement( USTR("style:style"),
                                         xUnoAttrs );

        xAttrs->Clear();
        // text or shape? if the former, no use in processing any
        // graphic attributes except stroke color, ODF can do ~nothing
        // with text shapes
        if( nTagId == XML_TEXT )
        {
            //xAttrs->AddAttribute( USTR( "draw:auto-grow-height"), USTR("true"));
            xAttrs->AddAttribute( USTR( "draw:auto-grow-width"), USTR("true"));
            xAttrs->AddAttribute( USTR( "draw:textarea-horizontal-align"), USTR("left"));
            //xAttrs->AddAttribute( USTR( "draw:textarea-vertical-align"), USTR("top"));
            xAttrs->AddAttribute( USTR( "fo:min-height"), USTR("0cm"));

            xAttrs->AddAttribute( USTR( "fo:padding-top"), USTR("0cm"));
            xAttrs->AddAttribute( USTR( "fo:padding-left"), USTR("0cm"));
            xAttrs->AddAttribute( USTR( "fo:padding-right"), USTR("0cm"));
            xAttrs->AddAttribute( USTR( "fo:padding-bottom"), USTR("0cm"));

            // disable any background shape
            xAttrs->AddAttribute( USTR( "draw:stroke" ), USTR("none"));
            xAttrs->AddAttribute( USTR( "draw:fill" ), USTR("none"));
        }
        else
        {
            if( rState.meFillType != NONE )
            {
                if( rState.meFillType == GRADIENT )
                {
                    xAttrs->AddAttribute( USTR( "draw:fill" ), USTR("gradient"));
                    xAttrs->AddAttribute( USTR( "draw:fill-gradient-name" ),
                                          getStyleName("svggradient", rState.maFillGradient.mnId) );
                    if( hasGradientOpacity(rState.maFillGradient) )
                    {
                        // needs transparency gradient as well
                        xAttrs->AddAttribute( USTR( "draw:opacity-name" ),
                                              getStyleName("svgopacity", rState.maFillGradient.mnId) );
                    }
                    else if( maCurrState.mnFillOpacity != 1.0 )
                        xAttrs->AddAttribute( USTR( "draw:opacity" ),
                                              rtl::OUString::valueOf(100.0*maCurrState.mnFillOpacity)+USTR("%") );
                }
                else
                {
                    xAttrs->AddAttribute( USTR( "draw:fill" ), USTR("solid"));
                    xAttrs->AddAttribute( USTR( "draw:fill-color" ), getOdfColor(rState.maFillColor));
                    if( maCurrState.mnFillOpacity != 1.0 )
                        xAttrs->AddAttribute( USTR( "draw:opacity" ),
                                              rtl::OUString::valueOf(100.0*maCurrState.mnFillOpacity)+USTR("%") );
                }
            }
            else
                xAttrs->AddAttribute( USTR( "draw:fill" ), USTR("none"));

            if( rState.meStrokeType != NONE )
            {
                xAttrs->AddAttribute( USTR( "draw:stroke" ), USTR("solid"));
                xAttrs->AddAttribute( USTR( "svg:stroke-color" ), getOdfColor(rState.maStrokeColor));
            }
            else
                xAttrs->AddAttribute( USTR( "draw:stroke" ), USTR("none"));

            if( maCurrState.mnStrokeWidth != 0.0 )
            {
                ::basegfx::B2DVector aVec(maCurrState.mnStrokeWidth,0);
                aVec *= maCurrState.maCTM;
                xAttrs->AddAttribute( USTR("svg:stroke-width"), rtl::OUString::valueOf( pt2mm(aVec.getLength()) )+USTR("mm"));
            }
            if( maCurrState.meLineJoin == basegfx::B2DLINEJOIN_MITER )
                xAttrs->AddAttribute( USTR( "draw:stroke-linejoin"), USTR("miter"));
            else if( maCurrState.meLineJoin == basegfx::B2DLINEJOIN_ROUND )
                xAttrs->AddAttribute( USTR( "draw:stroke-linejoin"), USTR("round"));
            else if( maCurrState.meLineJoin == basegfx::B2DLINEJOIN_BEVEL )
                xAttrs->AddAttribute( USTR( "draw:stroke-linejoin"), USTR("bevel"));
            if( maCurrState.mnStrokeOpacity != 1.0 )
                xAttrs->AddAttribute( USTR("svg:stroke-opacity"),
                                      rtl::OUString::valueOf(100.0*maCurrState.mnStrokeOpacity)+USTR("%"));
        }

        mxDocumentHandler->startElement( USTR("style:graphic-properties"),
                                         xUnoAttrs );
        mxDocumentHandler->endElement( USTR("style:graphic-properties") );
        mxDocumentHandler->endElement( USTR("style:style") );

        return true; // newly written
    }

    void writeStyle(const uno::Reference<xml::dom::XElement>& xElem, const sal_Int32 nTagId)
    {
        sal_Int32 nEmulatedStyleId=0;
        if( maCurrState.maDashArray.size() &&
            maCurrState.meStrokeType != NONE )
        {
            // ODF dashing is severly borked - generate filled shape
            // instead (further down the road - here, we simply
            // emulate a filled style with the next id)

            // move all stroke attribs to fill, Clear stroking
            State aEmulatedStrokeState( maCurrState );
            aEmulatedStrokeState.meFillType = maCurrState.meStrokeType;
            aEmulatedStrokeState.mnFillOpacity = maCurrState.mnStrokeOpacity;
            aEmulatedStrokeState.maFillColor = maCurrState.maStrokeColor;
            aEmulatedStrokeState.maFillGradient = maCurrState.maStrokeGradient;
            aEmulatedStrokeState.meFillRule = EVEN_ODD;
            aEmulatedStrokeState.meStrokeType = NONE;

            if( writeStyle(aEmulatedStrokeState, nTagId) )
                nEmulatedStyleId = mnCurrStateId;
            else
                nEmulatedStyleId = mrStates.find(aEmulatedStrokeState)->mnStyleId;
        }

        sal_Int32 nStyleId=0;
        if( writeStyle(maCurrState, nTagId) )
            nStyleId = mnCurrStateId;
        else
            nStyleId = mrStates.find(maCurrState)->mnStyleId;

        xElem->setAttribute(USTR("internal-style-ref"),
                            rtl::OUString::valueOf(
                                nStyleId)
                            +USTR("$")
                            +rtl::OUString::valueOf(
                                nEmulatedStyleId));
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
                                  const rtl::OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_GRADIENTTRANSFORM:
            {
                rtl::OString aValueUtf8( sValue.getStr(),
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
                                  const rtl::OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_GRADIENTTRANSFORM:
            {
                rtl::OString aValueUtf8( sValue.getStr(),
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
                            const rtl::OUString& sValue )
    {
        switch(nTokenId)
        {
            case XML_HREF:
            {
                ElementRefMapType::iterator aFound=maStopIdMap.end();
                if (sValue.copy(0,1).equalsAscii("#"))
                    aFound = maStopIdMap.find(sValue.copy(1));
                else
                    aFound = maStopIdMap.find(sValue);;

                if( aFound != maStopIdMap.end() )
                    io_rGradientStop =  maGradientStopVector[aFound->second];
                break;
            }
            case XML_ID:
                maStopIdMap.insert(std::make_pair(sValue,nStopNumber));
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
                         const rtl::OUString& sValue )
    {
        rtl::OString aValueUtf8( sValue.getStr(),
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
                    aValueUtf8,
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
            case XML_FILL_OPACITY:
                if( aValueUtf8 == "inherit" )
                    maCurrState.mnFillOpacity = maParentStates.back().mnFillOpacity;
                else
                    maCurrState.mnFillOpacity = aValueUtf8.toDouble();
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
                    maCurrState.meLineJoin = basegfx::B2DLINEJOIN_MITER;
                else if( aValueUtf8 == "round" )
                    maCurrState.meLineJoin = basegfx::B2DLINEJOIN_ROUND;
                else if( aValueUtf8 == "bevel" )
                    maCurrState.meLineJoin = basegfx::B2DLINEJOIN_BEVEL;
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
                    maCurrState.maDashArray.clear();
                else if( aValueUtf8 == "inherit" )
                    maCurrState.maDashArray = maParentStates.back().maDashArray;
                else
                    parseDashArray(aValueUtf8.getStr(),
                                   maCurrState.maDashArray);
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
                maCurrState.mnFontSize=convLength(sValue,maCurrState,'v');
                break;
            case XML_FONT_STYLE:
                maCurrState.meFontStyle=STYLE_ITALIC; // TODO: sValue.toStyleId();
                break;
            case XML_FONT_WEIGHT:
                maCurrState.mnFontWeight=sValue.toDouble();
                break;
            case XML_FONT_VARIANT:
                maCurrState.meFontVariant=VARIANT_SMALLCAPS; // TODO: sValue.toDouble();
                break;
            case XML_STOP_COLOR:
                if( maGradientVector.empty() ||
                    maGradientVector.back().maStops.empty() )
                    break;
                parseColor( aValueUtf8,
                            maGradientStopVector[
                                maGradientVector.back().maStops.back()].maStopColor );
                break;
            case XML_STOP_OPACITY:
                if( maGradientVector.empty() ||
                    maGradientVector.back().maStops.empty() )
                    break;
                parseOpacity( aValueUtf8,
                              maGradientStopVector[
                                  maGradientVector.back().maStops.back()].maStopColor );
                break;
            default:
                OSL_TRACE("unhandled token %s", getTokenName(nTokenId));
                break;
        }
    }

    void parseStyle( const rtl::OUString& sValue )
    {
        // split individual style attributes
        sal_Int32 nIndex=0, nDummyIndex=0;
        rtl::OUString aCurrToken;
        do
        {
            aCurrToken=sValue.getToken(0,';',nIndex);

            if( aCurrToken.getLength() )
            {
                // split attrib & value
                nDummyIndex=0;
                rtl::OUString aCurrAttrib(
                    aCurrToken.getToken(0,':',nDummyIndex).trim());
                OSL_ASSERT(nDummyIndex!=-1);
                nDummyIndex=0;
                rtl::OUString aCurrValue(
                    aCurrToken.getToken(1,':',nDummyIndex).trim());
                OSL_ASSERT(nDummyIndex==-1);

                // recurse into normal attribute parsing
                parseAttribute( getTokenId(aCurrAttrib),
                                aCurrValue );
            }
        }
        while( nIndex != -1 );
    }

    void parsePaint( const rtl::OUString& rValue,
                     const char*      sValue,
                     PaintType&       rType,
                     ARGBColor&       rColor,
                     Gradient&        rGradient,
                     const PaintType& rInheritType,
                     const ARGBColor& rInheritColor,
                     const Gradient&  rInheritGradient )
    {
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
        else if( strncmp(sValue,"url(#",5) == 0 )
        {
            // assuming gradient. assumption does not hold generally
            if( rValue.getLength() > 5 )
            {
                ElementRefMapType::iterator aRes;
                if( (aRes=maGradientIdMap.find(rValue.copy(5,
                                                           rValue.getLength()-6))) != maGradientIdMap.end() )
                {
                    rGradient = maGradientVector[aRes->second];
                    rType = GRADIENT;
                }
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
    ElementRefMapType                          maGradientIdMap;
    ElementRefMapType                          maStopIdMap;
};

/// Annotate svg styles with unique references to state pool
static void annotateStyles( StatePool&                                        rStatePool,
                            StateMap&                                         rStateMap,
                            const State&                                       rInitialState,
                            const uno::Reference<xml::dom::XElement>          xElem,
                            const uno::Reference<xml::sax::XDocumentHandler>& xDocHdl )
{
    AnnotatingVisitor aVisitor(rStatePool,rStateMap,rInitialState,xDocHdl);
    visitElements(aVisitor, xElem);
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
        rtl::OUString sStyleId(
            xElem->getAttribute(
                USTR("internal-style-ref")).getToken(
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
                rtl::OUString sAttributeValue;
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

                rtl::OUString sLinePath = USTR("M")+rtl::OUString::valueOf(x1)+USTR(",")
                    +rtl::OUString::valueOf(y1)+USTR("L")+rtl::OUString::valueOf(x2)+USTR(",")
                    +rtl::OUString::valueOf(y2);
                basegfx::B2DPolyPolygon aPoly;
                basegfx::tools::importFromSvgD(aPoly, sLinePath);

                writePathShape(xAttrs,
                               xUnoAttrs,
                               xElem,
                               sStyleId,
                               basegfx::B2DPolyPolygon(aPoly));
                break;
            }
            case XML_POLYGON:
            case XML_POLYLINE:
            {
                rtl::OUString sPoints = xElem->hasAttribute(USTR("points")) ? xElem->getAttribute(USTR("points")) : USTR("");
                basegfx::B2DPolygon aPoly;
                basegfx::tools::importFromSvgPoints(aPoly, sPoints);
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
                rtl::OUString sAttributeValue;
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

                if( bRxSeen && !bRySeen )
                    ry = rx;
                else if( bRySeen && !bRxSeen )
                    rx = ry;

                basegfx::B2DPolygon aPoly;
                aPoly = basegfx::tools::createPolygonFromRect(
                    basegfx::B2DRange(x,y,x+width,y+height),
                    rx/width, ry/height );

                writePathShape(xAttrs,
                               xUnoAttrs,
                               xElem,
                               sStyleId,
                               basegfx::B2DPolyPolygon(aPoly));
                break;
            }
            case XML_PATH:
            {
                rtl::OUString sPath = xElem->hasAttribute(USTR("d")) ? xElem->getAttribute(USTR("d")) : USTR("");
                basegfx::B2DPolyPolygon aPoly;
                basegfx::tools::importFromSvgD(aPoly, sPath);

                writePathShape(xAttrs,
                               xUnoAttrs,
                               xElem,
                               sStyleId,
                               aPoly);
                break;
            }
            case XML_CIRCLE:
            {
                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                rtl::OUString sAttributeValue;
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
                        default:
                            // skip
                            break;
                    }
                }

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
                rtl::OUString sAttributeValue;
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
                        default:
                            // skip
                            break;
                    }
                }

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
                rtl::OUString sAttributeValue;
                double x=0.0,y=0.0,width=0.0,height=0.0;
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

                rtl::OUString sValue = xElem->hasAttribute(USTR("href")) ? xElem->getAttribute(USTR("href")) : USTR("");
                rtl::OString aValueUtf8( sValue.getStr(), sValue.getLength(), RTL_TEXTENCODING_UTF8 );
                std::string sLinkValue;
                parseXlinkHref(aValueUtf8.getStr(), sLinkValue);

                if (!sLinkValue.empty())
                    writeBinaryData(xAttrs, xUnoAttrs, xElem, basegfx::B2DRange(x,y,x+width,y+height), sLinkValue);
                break;
            }
            case XML_TEXT:
            {
                // collect text from all TEXT_NODE children into sText
                rtl::OUStringBuffer sText;
                visitChildren(boost::bind(
                                  (rtl::OUStringBuffer& (rtl::OUStringBuffer::*)(const sal_Unicode* str))&rtl::OUStringBuffer::append,
                                  boost::ref(sText),
                                  boost::bind(&xml::dom::XNode::getNodeValue,
                                              _1)),
                              xElem,
                              xml::dom::NodeType_TEXT_NODE);

                // collect attributes
                const sal_Int32 nNumAttrs( xAttributes->getLength() );
                rtl::OUString sAttributeValue;
                double x=0.0,y=0.0,width=0.0,height=0.0;
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

                // actually export text
                xAttrs->Clear();

                // extract basic transformations out of CTM
                basegfx::B2DTuple aScale, aTranslate;
                double fRotate, fShearX;
                ::rtl::OUString sTransformValue;
                if (maCurrState.maCTM.decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    rtl::OUString sTransform;
                    x += aTranslate.getX();
                    y += aTranslate.getY();

                    sTransform +=
                        USTR("scale(") +
                        rtl::OUString::valueOf(aScale.getX()) +
                        USTR(", ") +
                        rtl::OUString::valueOf(aScale.getX()) +
                        USTR(")");

                    if( fRotate )
                        sTransform += USTR(" rotate(") + rtl::OUString::valueOf(fRotate*180.0/M_PI) + USTR(")");

                    if( fShearX )
                        sTransform += USTR(" skewX(") + rtl::OUString::valueOf(fShearX*180.0/M_PI) + USTR(")");
                }

                xAttrs->AddAttribute( USTR( "svg:x" ), rtl::OUString::valueOf(pt2mm(x))+USTR("mm"));
                xAttrs->AddAttribute( USTR( "svg:y" ), rtl::OUString::valueOf(pt2mm(y))+USTR("mm"));
                xAttrs->AddAttribute( USTR( "draw:style-name" ), USTR("svggraphicstyle")+sStyleId );

                mxDocumentHandler->startElement(USTR("draw:frame"),xUnoAttrs);

                xAttrs->Clear();
                mxDocumentHandler->startElement(USTR("draw:text-box"),xUnoAttrs);
                // TODO: put text style in here
                mxDocumentHandler->startElement(USTR("text:p"),xUnoAttrs);
                mxDocumentHandler->characters(sText.makeStringAndClear());
                mxDocumentHandler->endElement(USTR("text:p"));
                mxDocumentHandler->endElement(USTR("draw:text-box"));
                mxDocumentHandler->endElement(USTR("draw:frame"));
                break;
            }
        }
    }

    void push()
    {}

    void pop()
    {}

    void writeBinaryData( rtl::Reference<SvXMLAttributeList>&           xAttrs,
                        const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                        const uno::Reference<xml::dom::XElement>&       /* xElem */,
                        const basegfx::B2DRange&                        rShapeBounds,
                        const std::string&                              data)
    {
        xAttrs->Clear();
        xAttrs->AddAttribute( USTR( "svg:x" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getMinX()))+USTR("mm"));
        xAttrs->AddAttribute( USTR( "svg:y" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getMinY()))+USTR("mm"));
        xAttrs->AddAttribute( USTR( "svg:width" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getWidth()))+USTR("mm"));
        xAttrs->AddAttribute( USTR( "svg:height" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getHeight()))+USTR("mm"));

        mxDocumentHandler->startElement(USTR("draw:frame"),xUnoAttrs);

        xAttrs->Clear();
        mxDocumentHandler->startElement(USTR("draw:image"),xUnoAttrs);

        mxDocumentHandler->startElement(USTR("office:binary-data"),xUnoAttrs);

        mxDocumentHandler->characters(rtl::OUString::createFromAscii(data.c_str()));

        mxDocumentHandler->endElement(USTR("office:binary-data"));

        mxDocumentHandler->endElement(USTR("draw:image"));

        mxDocumentHandler->endElement(USTR("draw:frame"));
    }


    void writeTransformAttribute(const basegfx::B2DHomMatrix rMatrix, rtl::Reference<SvXMLAttributeList>& xAttrs)
    {
        basegfx::B2DTuple rScale, rTranslate;
        double rRotate, rShearX;
        ::rtl::OUString sTransformValue;
        if (!rMatrix.decompose(rScale, rTranslate, rRotate, rShearX))
            return;
        if (rScale.getX() != 1.0 || rScale.getY() != 1.0)
            sTransformValue += USTR("scale(")+::rtl::OUString::valueOf(rScale.getX())+USTR(" ")
                 +::rtl::OUString::valueOf(rScale.getY())+USTR(") ");
        if (rTranslate.getX() != 0.0f || rTranslate.getY() != 0.0f)
            sTransformValue += USTR("translate(")+::rtl::OUString::valueOf(rTranslate.getX()/100.0f)+USTR("mm ")
                 +::rtl::OUString::valueOf(rTranslate.getY()/100.0f)+USTR("mm) ");
        if (rRotate != 0.0f)
            sTransformValue += USTR("rotate(")+::rtl::OUString::valueOf(rRotate)+USTR(") ");

        if (rShearX != 0.0f)
            sTransformValue += USTR("skewX(")+::rtl::OUString::valueOf(rShearX)+USTR(") ");
        if (!sTransformValue.getLength())
            return;
        xAttrs->AddAttribute( USTR("draw:transform"), sTransformValue);
    }

    void writeEllipseShape( rtl::Reference<SvXMLAttributeList>&          xAttrs,
                         const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                         const uno::Reference<xml::dom::XElement>&       xElem,
                         const rtl::OUString&                            rStyleId,
                         const basegfx::B2DEllipse&                      rEllipse)
    {
        State aState = maCurrState;
        rtl::OUString aStyleId(rStyleId);

        xAttrs->Clear();

        basegfx::B2DPolygon aPoly = basegfx::tools::createPolygonFromEllipse(rEllipse.getB2DEllipseCenter(),
            rEllipse.getB2DEllipseRadius().getX(), rEllipse.getB2DEllipseRadius().getY());
        writePathShape(xAttrs, xUnoAttrs, xElem, rStyleId, basegfx::B2DPolyPolygon(aPoly));

    }

    void writePathShape( rtl::Reference<SvXMLAttributeList>&             xAttrs,
                         const uno::Reference<xml::sax::XAttributeList>& xUnoAttrs,
                         const uno::Reference<xml::dom::XElement>&       xElem,
                         const rtl::OUString&                            rStyleId,
                         const basegfx::B2DPolyPolygon&                  rPoly )
    {
        // we might need to split up polypolygon into multiple path
        // shapes (e.g. when emulating line stroking)
        std::vector<basegfx::B2DPolyPolygon> aPolys(1,rPoly);
        State aState = maCurrState;
        rtl::OUString aStyleId(rStyleId);

        xAttrs->Clear();

        OSL_TRACE("writePath - the CTM is: %f %f %f %f %f %f",
                  maCurrState.maCTM.get(0,0),
                  maCurrState.maCTM.get(0,1),
                  maCurrState.maCTM.get(0,2),
                  maCurrState.maCTM.get(1,0),
                  maCurrState.maCTM.get(1,1),
                  maCurrState.maCTM.get(1,2));

        if( aState.meStrokeType != NONE && aState.maDashArray.size() )
        {
            // ODF dashing is severly borked - generate filled polygon instead
            aPolys.clear();
            for( sal_uInt32 i=0; i<rPoly.count(); ++i )
            {
                aPolys.push_back(
                    basegfx::tools::createAreaGeometryForPolygon(
                        rPoly.getB2DPolygon(i),
                        aState.mnStrokeWidth/2.0,
                        aState.meLineJoin));
                // TODO(F2): line ends
            }

            sal_Int32 nDummyIndex(0);
            aStyleId = xElem->getAttribute(
                USTR("internal-style-ref")).getToken(1,'$',nDummyIndex);
            StateMap::iterator pAlternateState=mrStateMap.find(aStyleId.toInt32());
            OSL_ASSERT(pAlternateState != mrStateMap.end());
            aState = pAlternateState->second;
            OSL_ENSURE( pAlternateState == mrStateMap.end(),
                        "Doh - where's my alternate style entry?!" );
        }

        // TODO(F2): separate out shear, rotate etc.
        // apply transformation to polygon, to keep draw
        // import in 100th mm
        std::for_each(aPolys.begin(),aPolys.end(),
                      boost::bind(&basegfx::B2DPolyPolygon::transform,
                                  _1,boost::cref(aState.maCTM)));

        for( sal_uInt32 i=0; i<aPolys.size(); ++i )
        {
            const basegfx::B2DRange aBounds(
                aPolys[i].areControlPointsUsed() ?
                basegfx::tools::getRange(
                    basegfx::tools::adaptiveSubdivideByAngle(aPolys[i])) :
                basegfx::tools::getRange(aPolys[i]));
            fillShapeProperties(xAttrs,
                                xElem,
                                aBounds,
                                USTR("svggraphicstyle")+aStyleId);

            // force path coordinates to 100th millimeter, after
            // putting polygon data at origin (odf viewbox
            // calculations largely untested codepaths, as OOo always
            // writes "0 0 w h" viewboxes)
            basegfx::B2DHomMatrix aNormalize;
            aNormalize.translate(-aBounds.getMinX(),-aBounds.getMinY());
            aNormalize.scale(2540.0/72.0,2540.0/72.0);
            aPolys[i].transform(aNormalize);

            xAttrs->AddAttribute( USTR( "svg:d" ), basegfx::tools::exportToSvgD(
                aPolys[i],
                false,   // no relative coords. causes rounding errors
                false )); // no quad bezier detection. crashes older versions.
            mxDocumentHandler->startElement(USTR("draw:path"),
                                            xUnoAttrs);
            mxDocumentHandler->endElement(USTR("draw:path"));
        }
    }

    void fillShapeProperties( rtl::Reference<SvXMLAttributeList>&       xAttrs,
                              const uno::Reference<xml::dom::XElement>& /* xElem */,
                              const basegfx::B2DRange&                  rShapeBounds,
                              const rtl::OUString&                      rStyleName )
    {
        xAttrs->AddAttribute( USTR( "draw:z-index" ), rtl::OUString::valueOf( mnShapeNum++ ));
        xAttrs->AddAttribute( USTR( "draw:style-name" ), rStyleName);
        xAttrs->AddAttribute( USTR( "svg:width" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getWidth()))+USTR("mm"));
        xAttrs->AddAttribute( USTR( "svg:height" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getHeight()))+USTR("mm"));

        // OOo expects the viewbox to be in 100th of mm
        xAttrs->AddAttribute( USTR( "svg:viewBox" ),
            USTR("0 0 ")
            + rtl::OUString::valueOf(
                basegfx::fround(pt100thmm(rShapeBounds.getWidth())) )
            + USTR(" ")
            + rtl::OUString::valueOf(
                basegfx::fround(pt100thmm(rShapeBounds.getHeight())) ));

        // TODO(F1): decompose transformation in calling code, and use
        // transform attribute here
        // writeTranslate(maCurrState.maCTM, xAttrs);
        xAttrs->AddAttribute( USTR( "svg:x" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getMinX()))+USTR("mm"));
        xAttrs->AddAttribute( USTR( "svg:y" ), rtl::OUString::valueOf(pt2mm(rShapeBounds.getMinY()))+USTR("mm"));
    }

    State                                      maCurrState;
    StateMap&                                  mrStateMap;
    uno::Reference<xml::sax::XDocumentHandler> mxDocumentHandler;
    sal_Int32                                  mnShapeNum;
};

/// Write out shapes from DOM tree
static void writeShapes( StatePool&                                        rStatePool,
                         StateMap&                                         rStateMap,
                         const uno::Reference<xml::dom::XElement>          xElem,
                         const uno::Reference<xml::sax::XDocumentHandler>& xDocHdl )
{
    ShapeWritingVisitor aVisitor(rStatePool,rStateMap,xDocHdl);
    visitElements(aVisitor, xElem);
}

#ifdef VERBOSE
struct DumpingVisitor
{
    void operator()( const uno::Reference<xml::dom::XElement>& xElem )
    {
        OSL_TRACE("name: %s",
                  rtl::OUStringToOString(
                      xElem->getTagName(),
                      RTL_TEXTENCODING_UTF8 ).getStr());
    }

    void operator()( const uno::Reference<xml::dom::XElement>&      xElem,
                     const uno::Reference<xml::dom::XNamedNodeMap>& xAttributes )
    {
        OSL_TRACE("name: %s",
                  rtl::OUStringToOString(
                      xElem->getTagName(),
                      RTL_TEXTENCODING_UTF8 ).getStr());
        const sal_Int32 nNumAttrs( xAttributes->getLength() );
        for( sal_Int32 i=0; i<nNumAttrs; ++i )
        {
            OSL_TRACE(" %s=%s",
                      rtl::OUStringToOString(
                          xAttributes->item(i)->getNodeName(),
                          RTL_TEXTENCODING_UTF8 ).getStr(),
                      rtl::OUStringToOString(
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
    visitElements(aVisitor, xElem);
}
#endif

} // namespace


SVGReader::SVGReader(const uno::Reference<lang::XMultiServiceFactory>&     xServiceFactory,
                     const uno::Reference<io::XInputStream>&           xInputStream,
                     const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler) :
    m_xServiceFactory( xServiceFactory ),
    m_xInputStream( xInputStream ),
    m_xDocumentHandler( xDocumentHandler )
{
}

sal_Bool SVGReader::parseAndConvert()
{
    uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(
        m_xServiceFactory->createInstance(
            rtl::OUString::createFromAscii("com.sun.star.xml.dom.DocumentBuilder")), uno::UNO_QUERY );

    uno::Reference<xml::dom::XDocument> xDom(
        xDomBuilder->parse(m_xInputStream),
        uno::UNO_QUERY_THROW );

    uno::Reference<xml::dom::XElement> xDocElem( xDom->getDocumentElement(),
                                                 uno::UNO_QUERY_THROW );

    // the root state for svg document
    State aInitialState;

    /////////////////////////////////////////////////////////////////
    // doc boilerplate
    /////////////////////////////////////////////////////////////////

    m_xDocumentHandler->startDocument();

    // get the document dimensions

    // if the "width" and "height" attributes are missing, inkscape fakes
    // A4 portrait for. Let's do the same.
    if (!xDocElem->hasAttribute(USTR("width")))
        xDocElem->setAttribute(USTR("width"), USTR("210mm"));
    if (!xDocElem->hasAttribute(USTR("height")))
        xDocElem->setAttribute(USTR("height"), USTR("297mm"));

    double fViewPortWidth( pt2mm(convLength(xDocElem->getAttribute(USTR("width")),aInitialState,'h')) );
    double fViewPortHeight( pt2mm(convLength(xDocElem->getAttribute(USTR("height")),aInitialState,'v')) );

    // document prolog
    rtl::Reference<SvXMLAttributeList> xAttrs( new SvXMLAttributeList() );
    uno::Reference<xml::sax::XAttributeList> xUnoAttrs( xAttrs.get() );

    xAttrs->AddAttribute( USTR( "xmlns:office" ), USTR( OASIS_STR "office:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:style" ), USTR( OASIS_STR "style:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:text" ), USTR( OASIS_STR "text:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:svg" ), USTR( OASIS_STR "svg-compatible:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:table" ), USTR( OASIS_STR "table:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:draw" ), USTR( OASIS_STR "drawing:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:fo" ), USTR( OASIS_STR "xsl-fo-compatible:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:xlink" ), USTR( "http://www.w3.org/1999/xlink" ));
    xAttrs->AddAttribute( USTR( "xmlns:dc" ), USTR( "http://purl.org/dc/elements/1.1/" ));
    xAttrs->AddAttribute( USTR( "xmlns:number" ), USTR( OASIS_STR "datastyle:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:presentation" ), USTR( OASIS_STR "presentation:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:math" ), USTR( "http://www.w3.org/1998/Math/MathML" ));
    xAttrs->AddAttribute( USTR( "xmlns:form" ), USTR( OASIS_STR "form:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:script" ), USTR( OASIS_STR "script:1.0" ));
    xAttrs->AddAttribute( USTR( "xmlns:dom" ), USTR( "http://www.w3.org/2001/xml-events" ));
    xAttrs->AddAttribute( USTR( "xmlns:xforms" ), USTR( "http://www.w3.org/2002/xforms" ));
    xAttrs->AddAttribute( USTR( "xmlns:xsd" ), USTR( "http://www.w3.org/2001/XMLSchema" ));
    xAttrs->AddAttribute( USTR( "xmlns:xsi" ), USTR( "http://www.w3.org/2001/XMLSchema-instance" ));
    xAttrs->AddAttribute( USTR( "office:version" ), USTR( "1.0" ));
    xAttrs->AddAttribute( USTR( "office:mimetype" ), USTR( "application/vnd.oasis.opendocument.graphics" ));

    m_xDocumentHandler->startElement( USTR("office:document"), xUnoAttrs );

    xAttrs->Clear();

    m_xDocumentHandler->startElement( USTR("office:settings"), xUnoAttrs);

    xAttrs->AddAttribute( USTR( "config:name" ), USTR( "ooo:view-settings" ));
    m_xDocumentHandler->startElement( USTR("config:config-item-set"), xUnoAttrs);

    xAttrs->Clear();

    xAttrs->AddAttribute( USTR( "config:name" ), USTR( "VisibleAreaTop" ));
    xAttrs->AddAttribute( USTR( "config:type" ), USTR( "int" ));
    m_xDocumentHandler->startElement( USTR( "config:config-item" ), xUnoAttrs);

    m_xDocumentHandler->characters( USTR( "0" ));

    m_xDocumentHandler->endElement( USTR( "config:config-item" ));

    xAttrs->Clear();

    xAttrs->AddAttribute( USTR( "config:name" ), USTR( "VisibleAreaLeft" ));
    xAttrs->AddAttribute( USTR( "config:type" ), USTR( "int" ));
    m_xDocumentHandler->startElement( USTR( "config:config-item" ), xUnoAttrs);

    m_xDocumentHandler->characters( USTR( "0" ));

    m_xDocumentHandler->endElement( USTR( "config:config-item" ));

    xAttrs->Clear();

    xAttrs->AddAttribute( USTR( "config:name" ), USTR( "VisibleAreaWidth" ));
    xAttrs->AddAttribute( USTR( "config:type" ), USTR( "int" ));
    m_xDocumentHandler->startElement( USTR( "config:config-item" ), xUnoAttrs);

    sal_Int64 iWidth = sal_Int64(fViewPortWidth);
    m_xDocumentHandler->characters( ::rtl::OUString::valueOf(iWidth) );

    m_xDocumentHandler->endElement( USTR( "config:config-item" ));

    xAttrs->Clear();

    xAttrs->AddAttribute( USTR( "config:name" ), USTR( "VisibleAreaHeight" ));
    xAttrs->AddAttribute( USTR( "config:type" ), USTR( "int" ));
    m_xDocumentHandler->startElement( USTR( "config:config-item" ), xUnoAttrs);

    sal_Int64 iHeight = sal_Int64(fViewPortHeight);
    m_xDocumentHandler->characters( ::rtl::OUString::valueOf(iHeight) );

    m_xDocumentHandler->endElement( USTR( "config:config-item" ));

    m_xDocumentHandler->endElement( USTR( "config:config-item-set" ));

    m_xDocumentHandler->endElement( USTR( "office:settings" ));

    xAttrs->Clear();

    m_xDocumentHandler->startElement( USTR("office:automatic-styles"),
                                      xUnoAttrs );

    xAttrs->AddAttribute( USTR( "style:name" ), USTR("pagelayout1"));
    m_xDocumentHandler->startElement( USTR("style:page-layout"),
                                      xUnoAttrs );
    // TODO(Q3): this is super-ugly. In-place container come to mind.
    xAttrs->Clear();

    // make page viewport-width times viewport-height mm large - add
    // 5% border at every side
    xAttrs->AddAttribute( USTR( "fo:margin-top" ), USTR("0mm"));
    xAttrs->AddAttribute( USTR( "fo:margin-bottom" ), USTR("0mm"));
    xAttrs->AddAttribute( USTR( "fo:margin-left" ), USTR("0mm"));
    xAttrs->AddAttribute( USTR( "fo:margin-right" ), USTR("0mm"));
    xAttrs->AddAttribute( USTR( "fo:page-width" ), rtl::OUString::valueOf(fViewPortWidth)+USTR("mm"));
    xAttrs->AddAttribute( USTR( "fo:page-height" ), rtl::OUString::valueOf(fViewPortHeight)+USTR("mm"));
    xAttrs->AddAttribute( USTR( "style:print-orientation" ),
        fViewPortWidth > fViewPortHeight ?
        USTR("landscape") :
        USTR("portrait"));
    m_xDocumentHandler->startElement( USTR("style:page-layout-properties"),
                                      xUnoAttrs );
    m_xDocumentHandler->endElement( USTR("style:page-layout-properties") );
    m_xDocumentHandler->endElement( USTR("style:page-layout") );

    xAttrs->Clear();
    xAttrs->AddAttribute( USTR( "style:name" ), USTR("pagestyle1"));
    xAttrs->AddAttribute( USTR( "style:family" ), USTR("drawing-page"));
    m_xDocumentHandler->startElement( USTR("style:style"),
                                      xUnoAttrs );

    xAttrs->Clear();
    xAttrs->AddAttribute( USTR( "draw:background-size" ), USTR("border"));
    xAttrs->AddAttribute( USTR( "draw:fill" ), USTR("none"));
    m_xDocumentHandler->startElement( USTR("style:drawing-page-properties"),
                                      xUnoAttrs );
    m_xDocumentHandler->endElement( USTR("style:drawing-page-properties") );
    m_xDocumentHandler->endElement( USTR("style:style") );

    StatePool aStatePool;
    StateMap  aStateMap;
    annotateStyles(aStatePool,aStateMap,aInitialState,
                   xDocElem,m_xDocumentHandler);

#ifdef VERBOSE
    dumpTree(xDocElem);
#endif

    m_xDocumentHandler->endElement( USTR("office:automatic-styles") );

    ////////////////////////////////////////////////////////////////////

    xAttrs->Clear();
    m_xDocumentHandler->startElement( USTR("office:styles"),
                                      xUnoAttrs);
    m_xDocumentHandler->endElement( USTR("office:styles") );

    ////////////////////////////////////////////////////////////////////

    m_xDocumentHandler->startElement( USTR("office:master-styles"),
                                      xUnoAttrs );
    xAttrs->Clear();
    xAttrs->AddAttribute( USTR( "style:name" ), USTR("Default"));
    xAttrs->AddAttribute( USTR( "style:page-layout-name" ), USTR("pagelayout1"));
    xAttrs->AddAttribute( USTR( "draw:style-name" ), USTR("pagestyle1"));
    m_xDocumentHandler->startElement( USTR("style:master-page"),
                                      xUnoAttrs );
    m_xDocumentHandler->endElement( USTR("style:master-page") );

    m_xDocumentHandler->endElement( USTR("office:master-styles") );

    ////////////////////////////////////////////////////////////////////

    xAttrs->Clear();
    m_xDocumentHandler->startElement( USTR("office:body"),
                                      xUnoAttrs );
    m_xDocumentHandler->startElement( USTR("office:drawing"),
                                      xUnoAttrs );

    xAttrs->Clear();
    xAttrs->AddAttribute( USTR( "draw:master-page-name" ), USTR("Default"));
    xAttrs->AddAttribute( USTR( "draw:style-name" ), USTR("pagestyle1"));
    m_xDocumentHandler->startElement(USTR("draw:page"),
                                     xUnoAttrs);

    // write out all shapes
    writeShapes(aStatePool,
                aStateMap,
                xDocElem,
                m_xDocumentHandler);

    m_xDocumentHandler->endElement( USTR("draw:page") );
    m_xDocumentHandler->endElement( USTR("office:drawing") );
    m_xDocumentHandler->endElement( USTR("office:body") );
    m_xDocumentHandler->endElement( USTR("office:document") );
    m_xDocumentHandler->endDocument();

    return sal_True;
}

} // namespace svgi
