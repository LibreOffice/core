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

#include "diagramlayoutatoms.hxx"

#include <functional>

#include <basegfx/numeric/ftools.hxx>

#include "oox/helper/attributelist.hxx"
#include "drawingml/fillproperties.hxx"
#include "drawingml/lineproperties.hxx"
#include "drawingml/textbody.hxx"
#include "drawingml/textparagraph.hxx"
#include "drawingml/textrun.hxx"
#include "drawingml/customshapeproperties.hxx"
#include "layoutnodecontext.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace oox { namespace drawingml {

IteratorAttr::IteratorAttr( )
    : mnAxis( 0 )
    , mnCnt( -1 )
    , mbHideLastTrans( false )
    , mnPtType( 0 )
    , mnSt( 0 )
    , mnStep( 1 )
{
}

void IteratorAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    AttributeList attr( xAttr );
    mnAxis = xAttr->getOptionalValueToken( XML_axis, 0 );
    mnCnt = attr.getInteger( XML_cnt, -1 );
    mbHideLastTrans = attr.getBool( XML_hideLastTrans, false );
    mnPtType = xAttr->getOptionalValueToken( XML_ptType, 0 );
    mnSt = attr.getInteger( XML_st, 0 );
    mnStep = attr.getInteger( XML_step, 1 );
}

ConditionAttr::ConditionAttr()
    : mnFunc( 0 )
    , mnArg( 0 )
    , mnOp( 0 )
{

}

void ConditionAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    mnFunc = xAttr->getOptionalValueToken( XML_func, 0 );
    // mnArg will be -1 for "none" or any other unknown value
    mnArg = LayoutNodeContext::tagToVarIdx( xAttr->getOptionalValueToken( XML_arg, XML_none ) );
    mnOp = xAttr->getOptionalValueToken( XML_op, 0 );
    msVal = xAttr->getOptionalValue( XML_val );
}

void LayoutAtom::dump(int level)
{
    SAL_INFO("oox.drawingml",  "level = " << level << " - " << msName << " of type " << typeid(*this).name() );
    for (const auto& pAtom : getChildren())
        pAtom->dump(level + 1);
}

ForEachAtom::ForEachAtom(const Reference< XFastAttributeList >& xAttributes)
{
    maIter.loadFromXAttr(xAttributes);
}

void ForEachAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void ChooseAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

ConditionAtom::ConditionAtom(const Reference< XFastAttributeList >& xAttributes) :
    mbElse( false )
{
    maIter.loadFromXAttr( xAttributes );
    maCond.loadFromXAttr( xAttributes );
}

const std::vector<LayoutAtomPtr>& ConditionAtom::getChildren() const
{
    bool bDecisionVar=true;
    // HACK
    if( maCond.mnFunc == XML_var && maCond.mnArg == XML_dir && maCond.mnOp == XML_equ && maCond.msVal != "norm" )
        bDecisionVar=false;

    if( bDecisionVar )
        return mpChildNodes;
    else
        return mpElseChildNodes;
}

void ConditionAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void ConditionAtom::addChild( const LayoutAtomPtr & pNode )
{
    if( mbElse )
        mpElseChildNodes.push_back( pNode );
    else
        mpChildNodes.push_back( pNode );
}

void ConstraintAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void AlgAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void AlgAtom::layoutShape( const ShapePtr& rShape,
                           const OUString& rName ) const
{
    switch(mnType)
    {
        case XML_composite:
        {
            if( rShape->getChildren().empty() )
            {
                rShape->setSize(awt::Size(50,50));
                break;
            }

            // just put stuff below each other
            const sal_Int32 nIncX=0;
            const sal_Int32 nIncY=1;

            std::vector<ShapePtr>::const_iterator aCurrShape=rShape->getChildren().begin();
            const std::vector<ShapePtr>::const_iterator aLastShape=rShape->getChildren().end();

            // find biggest shape
            awt::Size aMaxSize;
            while( aCurrShape != aLastShape )
            {
                const awt::Size& sz=(*aCurrShape)->getSize();

                aMaxSize.Width = std::max(
                    aMaxSize.Width,
                    sz.Width);
                aMaxSize.Height = std::max(
                    aMaxSize.Height,
                    sz.Height);

                ++aCurrShape;
            }

            aCurrShape=rShape->getChildren().begin();
            const awt::Point aStartPos=(*aCurrShape)->getPosition();
            awt::Point aCurrPos=aStartPos;
            awt::Size  aTotalSize;
            aTotalSize.Width = aMaxSize.Width;
            while( aCurrShape != aLastShape )
            {
                const awt::Size& sz=(*aCurrShape)->getSize();
                (*aCurrShape)->setPosition(aCurrPos);
                (*aCurrShape)->setSize(
                    awt::Size(aMaxSize.Width,
                              sz.Height));

                aTotalSize.Height = std::max(
                    aTotalSize.Height,
                    aCurrPos.Y + sz.Height);

                aCurrPos.X += nIncX*sz.Width;
                aCurrPos.Y += nIncY*sz.Height;

                ++aCurrShape;
            }

            rShape->setSize(aTotalSize);
            break;
        }

        case XML_conn:
            break;

        case XML_cycle:
        {
            if( rShape->getChildren().empty() )
            {
                rShape->setSize(awt::Size(50,50));
                break;
            }

            const sal_Int32 nStartAngle=maMap.count(XML_stAng) ? maMap.find(XML_stAng)->second : 0;
            const sal_Int32 nSpanAngle=maMap.count(XML_spanAng) ? maMap.find(XML_spanAng)->second : 360;

            std::vector<ShapePtr>::const_iterator aCurrShape=rShape->getChildren().begin();
            const std::vector<ShapePtr>::const_iterator aLastShape=rShape->getChildren().end();
            const sal_Int32 nShapes=aLastShape-aCurrShape;

            // find biggest shape
            awt::Size aMaxSize;
            while( aCurrShape != aLastShape )
            {
                const awt::Size& sz=(*aCurrShape)->getSize();

                aMaxSize.Width = std::max(
                    aMaxSize.Width,
                    sz.Width);
                aMaxSize.Height = std::max(
                    aMaxSize.Height,
                    sz.Height);

                ++aCurrShape;
            }

            // layout shapes
            const sal_Int32 nMaxDim=std::max(aMaxSize.Width,aMaxSize.Height);
            awt::Size aTotalSize;
            aCurrShape=rShape->getChildren().begin();
            for( sal_Int32 i=0; i<nShapes; ++i, ++aCurrShape )
            {
                const awt::Size& sz=(*aCurrShape)->getSize();

                const double r=nShapes*nMaxDim/F_2PI * 360.0/nSpanAngle;
                const awt::Point aCurrPos(
                    r + r*sin( (double(i)*nSpanAngle/nShapes + nStartAngle)*F_PI180 ),
                    r - r*cos( (double(i)*nSpanAngle/nShapes + nStartAngle)*F_PI180 ) );
                (*aCurrShape)->setPosition(aCurrPos);

                aTotalSize.Width = std::max(
                    aTotalSize.Width,
                    aCurrPos.X + sz.Width);
                aTotalSize.Height = std::max(
                    aTotalSize.Height,
                    aCurrPos.Y + sz.Height);
            }

            rShape->setSize(aTotalSize);
            break;
        }

        case XML_hierChild:
        case XML_hierRoot:
            break;

        case XML_lin:
        case XML_snake:
        {
            if( rShape->getChildren().empty() )
            {
                rShape->setSize(awt::Size(50,50));
                break;
            }

            const sal_Int32 nDir=maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL;
            const sal_Int32 nIncX=nDir==XML_fromL ? 1 : (nDir==XML_fromR ? -1 : 0);
            const sal_Int32 nIncY=nDir==XML_fromT ? 1 : (nDir==XML_fromB ? -1 : 0);

            std::vector<ShapePtr>::const_iterator aCurrShape=rShape->getChildren().begin();
            const std::vector<ShapePtr>::const_iterator aLastShape=rShape->getChildren().end();
            const awt::Point aStartPos=(*aCurrShape)->getPosition();
            awt::Point aCurrPos=aStartPos;
            awt::Size  aTotalSize;
            while( aCurrShape != aLastShape )
            {
                const awt::Size& sz=(*aCurrShape)->getSize();
                (*aCurrShape)->setPosition(aCurrPos);

                aTotalSize.Width = std::max(
                    aTotalSize.Width,
                    aCurrPos.X + sz.Width);
                aTotalSize.Height = std::max(
                    aTotalSize.Height,
                    aCurrPos.Y + sz.Height);

                // HACK: the spacing is arbitrary
                aCurrPos.X += nIncX*(sz.Width+5);
                aCurrPos.Y += nIncY*(sz.Height+5);

                ++aCurrShape;
            }

            rShape->setSize(aTotalSize);
            break;
        }

        case XML_pyra:
            break;

        case XML_sp:
            // HACK. Handled one level higher. Or rather, planned to
            break;

        case XML_tx:
        {
            TextBodyPtr pTextBody=rShape->getTextBody();
            if( !pTextBody ||
                pTextBody->getParagraphs().empty() ||
                pTextBody->getParagraphs().front()->getRuns().empty() )
            {
                rShape->setSize(awt::Size(5,5));
                break;
            }

            // HACK - count chars & paragraphs to come up with *some*
            // notion of necessary size
            const sal_Int32 nHackyFontHeight=50;
            const sal_Int32 nHackyFontWidth=20;
            awt::Size aTotalSize;
            for( size_t nPara=0; nPara<pTextBody->getParagraphs().size(); ++nPara )
            {
                aTotalSize.Height += nHackyFontHeight;

                sal_Int32 nLocalWidth=0;
                for( size_t nRun=0; nRun<pTextBody->getParagraphs().at(nPara)->getRuns().size(); ++nRun )
                    nLocalWidth +=
                        pTextBody->getParagraphs().at(nPara)->getRuns().at(nRun)->getText().getLength()
                        * nHackyFontWidth;

                aTotalSize.Width = std::max(
                    aTotalSize.Width,
                    nLocalWidth);
            }

            rShape->setSize(aTotalSize);
            break;
        }

        default:
            break;
    }

    SAL_INFO(
        "oox.drawingml",
        "Layouting shape " << rName << ": (" << rShape->getPosition().X << ","
        << rShape->getPosition().Y << "," << rShape->getSize().Width << ","
        << rShape->getSize().Height << ")");
}

void LayoutNode::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

bool LayoutNode::setupShape( const ShapePtr& rShape, const Diagram& rDgm, sal_uInt32 nIdx ) const
{
    // find the data node to grab text from
    DiagramData::PointsNameMap::const_iterator aDataNode=rDgm.getData()->getPointsPresNameMap().find(msName);
    if( aDataNode != rDgm.getData()->getPointsPresNameMap().end() &&
        aDataNode->second.size() > nIdx )
    {
        SAL_INFO(
            "oox.drawingml",
            "Filling content from " << nIdx << "th layout node named \""
                << msName << "\", modelId \""
                << aDataNode->second.at(nIdx)->msModelId << "\"");

        // got the presentation node - now, need the actual data node:
        const DiagramData::StringMap::const_iterator aNodeName=rDgm.getData()->getPresOfNameMap().find(
            aDataNode->second.at(nIdx)->msModelId);
        if( aNodeName != rDgm.getData()->getPresOfNameMap().end() )
        {
            DiagramData::StringMap::value_type::second_type::const_iterator aVecIter=aNodeName->second.begin();
            const DiagramData::StringMap::value_type::second_type::const_iterator aVecEnd=aNodeName->second.end();
            while( aVecIter != aVecEnd )
            {
                DiagramData::PointNameMap& rMap = rDgm.getData()->getPointNameMap();
                DiagramData::PointNameMap::const_iterator aDataNode2 = rMap.find(aVecIter->first);
                if (aDataNode2 == rMap.end())
                {
                    //busted, skip it
                    ++aVecIter;
                    continue;
                }

                if( aVecIter->second == 0 )
                {
                    // grab shape attr from topmost element(s)
                    rShape->getShapeProperties() = aDataNode2->second->mpShape->getShapeProperties();
                    rShape->getLineProperties() = aDataNode2->second->mpShape->getLineProperties();
                    rShape->getFillProperties() = aDataNode2->second->mpShape->getFillProperties();
                    rShape->getCustomShapeProperties() = aDataNode2->second->mpShape->getCustomShapeProperties();
                    rShape->setMasterTextListStyle( aDataNode2->second->mpShape->getMasterTextListStyle() );

                    SAL_INFO(
                        "oox.drawingml",
                        "Custom shape with preset type "
                            << (rShape->getCustomShapeProperties()
                                ->getShapePresetType())
                            << " added for layout node named \"" << msName
                            << "\"");
                }

                // append text with right outline level
                if( aDataNode2->second->mpShape->getTextBody() &&
                    !aDataNode2->second->mpShape->getTextBody()->getParagraphs().empty() &&
                    !aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getRuns().empty() )
                {
                    TextBodyPtr pTextBody=rShape->getTextBody();
                    if( !pTextBody )
                    {
                        pTextBody.reset( new TextBody() );

                        // also copy text attrs
                        pTextBody->getTextListStyle() =
                            aDataNode2->second->mpShape->getTextBody()->getTextListStyle();
                        pTextBody->getTextProperties() =
                            aDataNode2->second->mpShape->getTextBody()->getTextProperties();

                        rShape->setTextBody(pTextBody);
                    }

                    TextParagraph& rPara=pTextBody->addParagraph();
                    if( aVecIter->second != -1 )
                        rPara.getProperties().setLevel(aVecIter->second);

                    rPara.addRun(
                        aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getRuns().front());
                    rPara.getProperties().apply(
                        aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getProperties());
                }

                ++aVecIter;
            }
        }
        else
        {
            SAL_INFO(
                "oox.drawingml",
                "ShapeCreationVisitor::visit: no data node name found while"
                    " processing shape type "
                    << rShape->getCustomShapeProperties()->getShapePresetType()
                    << " for layout node named \"" << msName << "\"");
        }

        // TODO(Q1): apply styling & coloring - taking
        // layout node's styleLbl for both style & color
        // now, but docs are a bit unclear on this
        if( !msStyleLabel.isEmpty() )
        {
            const DiagramQStyleMap::const_iterator aStyle=rDgm.getStyles().find(msStyleLabel);
            if( aStyle != rDgm.getStyles().end() )
            {
                rShape->getShapeStyleRefs()[XML_fillRef] = aStyle->second.maFillStyle;
                rShape->getShapeStyleRefs()[XML_lnRef] = aStyle->second.maLineStyle;
                rShape->getShapeStyleRefs()[XML_effectRef] = aStyle->second.maEffectStyle;
                rShape->getShapeStyleRefs()[XML_fontRef] = aStyle->second.maTextStyle;
                Color aColor=aStyle->second.maTextStyle.maPhClr;
            }

            const DiagramColorMap::const_iterator aColor=rDgm.getColors().find(msStyleLabel);
            if( aColor != rDgm.getColors().end() )
            {
                const DiagramColor& rColor=aColor->second;
                if( rColor.maFillColor.isUsed() )
                    rShape->getShapeStyleRefs()[XML_fillRef].maPhClr = rColor.maFillColor;
                if( rColor.maLineColor.isUsed() )
                    rShape->getShapeStyleRefs()[XML_lnRef].maPhClr = rColor.maLineColor;
                if( rColor.maEffectColor.isUsed() )
                    rShape->getShapeStyleRefs()[XML_effectRef].maPhClr = rColor.maEffectColor;
                if( rColor.maTextFillColor.isUsed() )
                    rShape->getShapeStyleRefs()[XML_fontRef].maPhClr = rColor.maTextFillColor;
            }
        }
        else
        {
            // if no style label apply at least some fill color
            rShape->getShapeStyleRefs()[XML_fillRef].maPhClr.setScrgbClr(0, 0, 0);
            rShape->getShapeStyleRefs()[XML_fillRef].mnThemedIdx = 2;
        }

        // even if no data node found, successful anyway. it's
        // contained at the layoutnode
        return true;
    }
    else
    {
        SAL_INFO(
            "oox.drawingml",
            "no text found while processing shape type "
                << rShape->getCustomShapeProperties()->getShapePresetType()
                << " for layout node named \"" << msName << "\"");
    }

    return false;
}

void ShapeAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
