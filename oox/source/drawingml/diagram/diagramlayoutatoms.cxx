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
            // all shapes fill parent

            for (auto & aCurrShape : rShape->getChildren())
            {
                aCurrShape->setSize(rShape->getSize());
                aCurrShape->setChildSize(rShape->getSize());
            }
            break;
        }

        case XML_conn:
            break;

        case XML_cycle:
        {
            if (rShape->getChildren().empty())
                break;

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
            aCurrShape=rShape->getChildren().begin();
            for( sal_Int32 i=0; i<nShapes; ++i, ++aCurrShape )
            {
                const double r=nShapes*nMaxDim/F_2PI * 360.0/nSpanAngle;
                const awt::Point aCurrPos(
                    r + r*sin( (double(i)*nSpanAngle/nShapes + nStartAngle)*F_PI180 ),
                    r - r*cos( (double(i)*nSpanAngle/nShapes + nStartAngle)*F_PI180 ) );
                (*aCurrShape)->setPosition(aCurrPos);
            }
            break;
        }

        case XML_hierChild:
        case XML_hierRoot:
            break;

        case XML_lin:
        {
            // spread children evenly across one axis, stretch across second

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            const sal_Int32 nDir = maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL;
            const sal_Int32 nIncX = nDir==XML_fromL ? 1 : (nDir==XML_fromR ? -1 : 0);
            const sal_Int32 nIncY = nDir==XML_fromT ? 1 : (nDir==XML_fromB ? -1 : 0);

            // TODO: get values from constraints
            sal_Int32 nCount = rShape->getChildren().size();
            double fSpace = 0.3;

            awt::Size aChildSize = rShape->getSize();
            if (nIncX)
                aChildSize.Width /= (nCount + (nCount-1)*fSpace);
            if (nIncY)
                aChildSize.Height /= (nCount + (nCount-1)*fSpace);

            awt::Point aCurrPos = rShape->getChildren().front()->getPosition();
            for (auto & aCurrShape : rShape->getChildren())
            {
                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);
                aCurrPos.X += nIncX * (aChildSize.Width + fSpace*aChildSize.Width);
                aCurrPos.Y += nIncY * (aChildSize.Height + fSpace*aChildSize.Height);
            }
            break;
        }

        case XML_pyra:
            break;

        case XML_snake:
        {
            // find optimal grid to layout children that have fixed aspect ratio

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            // TODO: get values from constraints
            sal_Int32 nCount = rShape->getChildren().size();
            double fSpace = 0.3;
            double fAspectRatio = 0.6;

            sal_Int32 nCol = 1;
            sal_Int32 nRow = 1;
            for ( ; nCol<nCount; nCol++)
            {
                nRow = (nCount+nCol-1) / nCol;
                if ((rShape->getSize().Height / nRow) / (rShape->getSize().Width / nCol) >= fAspectRatio)
                    break;
            }
            SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

            sal_Int32 nWidth = rShape->getSize().Width / (nCol + (nCol-1)*fSpace);
            const awt::Size aChildSize(nWidth, nWidth * fAspectRatio);

            awt::Point aStartPos = rShape->getChildren().front()->getPosition();
            awt::Point aCurrPos = aStartPos;
            sal_Int32 nColIdx = 0;

            for (auto & aCurrShape : rShape->getChildren())
            {
                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);
                aCurrPos.X += aChildSize.Width + fSpace*aChildSize.Width;
                if (++nColIdx == nCol)
                {
                    aStartPos.Y += aChildSize.Height + fSpace*aChildSize.Height;
                    aCurrPos = aStartPos;
                    nColIdx = 0;
                }
            }
            break;
        }

        case XML_sp:
            // HACK. Handled one level higher. Or rather, planned to
            break;

        case XML_tx:
            // TODO: adjust text size to fit shape
            break;

        default:
            break;
    }

    SAL_INFO(
        "oox.drawingml",
        "Layouting shape " << rName << ", alg type: " << mnType << ", ("
        << rShape->getPosition().X << "," << rShape->getPosition().Y << ","
        << rShape->getSize().Width << "," << rShape->getSize().Height << ")");
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
        const dgm::Point* aPresNode = aDataNode->second.at(nIdx);
        SAL_INFO(
            "oox.drawingml",
            "Filling content from " << nIdx << "th layout node named \""
                << msName << "\", modelId \""
                << aPresNode->msModelId << "\"");

        // got the presentation node - now, need the actual data node:
        const DiagramData::StringMap::const_iterator aNodeName=rDgm.getData()->getPresOfNameMap().find(
            aPresNode->msModelId);
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

        // TODO(Q1): apply styling & coloring - take presentation
        // point's presStyleLbl for both style & color
        // if not found use layout node's styleLbl
        // however, docs are a bit unclear on this
        OUString aStyleLabel = aPresNode->msPresentationLayoutStyleLabel;
        if (aStyleLabel.isEmpty())
            aStyleLabel = msStyleLabel;
        if( !aStyleLabel.isEmpty() )
        {
            const DiagramQStyleMap::const_iterator aStyle = rDgm.getStyles().find(aStyleLabel);
            if( aStyle != rDgm.getStyles().end() )
            {
                const DiagramStyle& rStyle = aStyle->second;
                rShape->getShapeStyleRefs()[XML_fillRef] = rStyle.maFillStyle;
                rShape->getShapeStyleRefs()[XML_lnRef] = rStyle.maLineStyle;
                rShape->getShapeStyleRefs()[XML_effectRef] = rStyle.maEffectStyle;
                rShape->getShapeStyleRefs()[XML_fontRef] = rStyle.maTextStyle;
            }
            else
            {
                SAL_WARN("oox.drawingml", "Style " << aStyleLabel << " not found");
            }

            const DiagramColorMap::const_iterator aColor = rDgm.getColors().find(aStyleLabel);
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
