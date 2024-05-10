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

#include "datamodel.hxx"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <oox/drawingml/shape.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <editeng/unoprnms.hxx>

#include <unordered_set>

using namespace ::com::sun::star;

namespace oox::drawingml {

Shape* DiagramData::getOrCreateAssociatedShape(const svx::diagram::Point& rPoint, bool bCreateOnDemand) const
{
    if(maPointShapeMap.end() == maPointShapeMap.find(rPoint.msModelId))
    {
        const_cast<DiagramData*>(this)->maPointShapeMap[rPoint.msModelId] = ShapePtr();
    }

    const ShapePtr& rShapePtr = maPointShapeMap.find(rPoint.msModelId)->second;

    if(!rShapePtr && bCreateOnDemand)
    {
        const_cast<ShapePtr&>(rShapePtr) = std::make_shared<Shape>();

        // If we did create a new oox::drawingml::Shape, directly apply
        // available data from the Diagram ModelData to it as preparation
        restoreDataFromModelToShapeAfterReCreation(rPoint, *rShapePtr);
    }

    return rShapePtr.get();
}

void DiagramData::restoreDataFromModelToShapeAfterReCreation(const svx::diagram::Point& rPoint, Shape& rNewShape)
{
    // If we did create a new oox::drawingml::Shape, directly apply
    // available data from the Diagram ModelData to it as preparation

    // This is e.g. the Text, but may get more (styles?)
    if(!rPoint.msTextBody->msText.isEmpty())
    {
        TextBodyPtr aNewTextBody(std::make_shared<TextBody>());
        rNewShape.setTextBody(aNewTextBody);
        TextRunPtr pTextRun = std::make_shared<TextRun>();
        pTextRun->getText() = rPoint.msTextBody->msText;
        aNewTextBody->addParagraph().addRun(pTextRun);

        if(!rPoint.msTextBody->maTextProps.empty())
        {
            oox::PropertyMap& rTargetMap(aNewTextBody->getTextProperties().maPropertyMap);

            for (auto const& prop : rPoint.msTextBody->maTextProps)
            {
                const sal_Int32 nPropId(oox::PropertyMap::getPropertyId(prop.first));
                if(nPropId > 0)
                    rTargetMap.setAnyProperty(nPropId, prop.second);
            }
        }
    }
}

static void addProperty(const OUString& rName,
    const css::uno::Reference< css::beans::XPropertySetInfo >& xInfo,
    std::vector< std::pair< OUString, css::uno::Any >>& rTarget,
    const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
{
    if(xInfo->hasPropertyByName(rName))
            rTarget.push_back(std::pair(OUString(rName), xPropSet->getPropertyValue(rName)));
}

void DiagramData::secureStyleDataFromShapeToModel(::oox::drawingml::Shape& rShape)
{
    const std::vector< ShapePtr >& rChildren(rShape.getChildren());

    if(!rChildren.empty())
    {
        // group shape
        for (auto& child : rChildren)
        {
            secureStyleDataFromShapeToModel(*child);
        }

        // if group shape we are done. Do not secure properties for group shapes
        return;
    }

    // we need a XShape
    const css::uno::Reference< css::drawing::XShape > &rXShape(rShape.getXShape());
    if(!rXShape)
        return;

    // we need a ModelID for association
    if(rShape.getDiagramDataModelID().isEmpty())
        return;

    // define target to save to
    svx::diagram::PointStyle* pTarget(nullptr);
    const bool bIsBackgroundShape(rShape.getDiagramDataModelID() == msBackgroundShapeModelID);

    if(bIsBackgroundShape)
    {
        // if BackgroundShape, create properties & set as target
        if(!maBackgroundShapeStyle)
            maBackgroundShapeStyle = std::make_shared< svx::diagram::PointStyle >();
        pTarget = maBackgroundShapeStyle.get();
    }
    else
    {
        // if Shape, seek association
        for (auto & point : maPoints)
        {
            if(point.msModelId == rShape.getDiagramDataModelID())
            {
                // found - create properties & set as target
                pTarget = point.msPointStylePtr.get();

                // we are done, there is no 2nd shape with the same ModelID by definition
                break;
            }
        }
    }

    // no target -> nothing to do
    if(nullptr == pTarget)
        return;

#ifdef DBG_UTIL
    // to easier decide which additional properties may/should be preserved,
    // create a full list of set properties to browse/decide (in debugger)
    const css::uno::Reference< css::beans::XPropertyState > xAllPropStates(rXShape, css::uno::UNO_QUERY);
    const css::uno::Reference< css::beans::XPropertySet > xAllPropSet( rXShape, css::uno::UNO_QUERY );
    const css::uno::Sequence< css::beans::Property > allSequence(xAllPropSet->getPropertySetInfo()->getProperties());
    std::vector< std::pair< OUString, css::uno::Any >> allSetProps;
    for (auto& rProp : allSequence)
    {
        try
        {
            if (xAllPropStates->getPropertyState(rProp.Name) == css::beans::PropertyState::PropertyState_DIRECT_VALUE)
            {
                css::uno::Any aValue(xAllPropSet->getPropertyValue(rProp.Name));
                if(aValue.hasValue())
                    allSetProps.push_back(std::pair(rProp.Name, aValue));
            }
        }
        catch (...)
        {
        }
    }
#endif

    const css::uno::Reference< css::beans::XPropertySet > xPropSet( rXShape, css::uno::UNO_QUERY );
    if(!xPropSet)
        return;

    const css::uno::Reference< css::lang::XServiceInfo > xServiceInfo( rXShape, css::uno::UNO_QUERY );
    if(!xServiceInfo)
        return;

    const css::uno::Reference< css::beans::XPropertySetInfo > xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    // Note: The Text may also be secured here, so it may also be possible to
    // secure/store it at PointStyle instead of at TextBody, same maybe evaluated
    // for the text attributes - where when securing here the attributes would be
    // in our UNO API format already.
    // if(xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    // {
    //     css::uno::Reference< css::text::XText > xText(rXShape, css::uno::UNO_QUERY);
    //     const OUString aText(xText->getString());
    //
    //     if(!aText.isEmpty())
    //     {
    //     }
    // }

    // Add all kinds of properties that are needed to re-create the XShape.
    // For now this is a minimal example-selection, it will need to be extended
    // over time for all kind of cases/properties

    // text properties
    if(!bIsBackgroundShape
        && xServiceInfo->supportsService(u"com.sun.star.drawing.TextProperties"_ustr))
    {
        addProperty(UNO_NAME_CHAR_COLOR, xInfo, pTarget->maProperties, xPropSet);
        addProperty(UNO_NAME_CHAR_HEIGHT, xInfo, pTarget->maProperties, xPropSet);
        addProperty(UNO_NAME_CHAR_SHADOWED, xInfo, pTarget->maProperties, xPropSet);
        addProperty(UNO_NAME_CHAR_WEIGHT, xInfo, pTarget->maProperties, xPropSet);
    }

    // fill properties
    if(xServiceInfo->supportsService(u"com.sun.star.drawing.FillProperties"_ustr))
    {
        css::drawing::FillStyle eFillStyle(css::drawing::FillStyle_NONE);
        if (xInfo->hasPropertyByName(UNO_NAME_FILLSTYLE))
            xPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= eFillStyle;

        if(css::drawing::FillStyle_NONE != eFillStyle)
        {
            addProperty(UNO_NAME_FILLSTYLE, xInfo, pTarget->maProperties, xPropSet);

            switch(eFillStyle)
            {
                case css::drawing::FillStyle_SOLID:
                {
                    addProperty(UNO_NAME_FILLCOLOR, xInfo, pTarget->maProperties, xPropSet);
                    break;
                }
                default:
                case css::drawing::FillStyle_NONE:
                case css::drawing::FillStyle_GRADIENT:
                case css::drawing::FillStyle_HATCH:
                case css::drawing::FillStyle_BITMAP:
                    break;
            }
        }
    }

    // line properties
    if(!bIsBackgroundShape
        && xServiceInfo->supportsService(u"com.sun.star.drawing.LineProperties"_ustr))
    {
        css::drawing::LineStyle eLineStyle(css::drawing::LineStyle_NONE);
        if (xInfo->hasPropertyByName(UNO_NAME_LINESTYLE))
            xPropSet->getPropertyValue(UNO_NAME_LINESTYLE) >>= eLineStyle;

        if(css::drawing::LineStyle_NONE != eLineStyle)
        {
            addProperty(UNO_NAME_LINESTYLE, xInfo, pTarget->maProperties, xPropSet);
            addProperty(UNO_NAME_LINECOLOR, xInfo, pTarget->maProperties, xPropSet);
            addProperty(UNO_NAME_LINEWIDTH, xInfo, pTarget->maProperties, xPropSet);

            switch(eLineStyle)
            {
                case css::drawing::LineStyle_SOLID:
                    break;
                default:
                case css::drawing::LineStyle_NONE:
                case css::drawing::LineStyle_DASH:
                    break;
            }
        }
    }
}

void DiagramData::secureDataFromShapeToModelAfterDiagramImport(::oox::drawingml::Shape& rRootShape)
{
    const std::vector< ShapePtr >& rChildren(rRootShape.getChildren());

    for (auto& child : rChildren)
    {
        secureStyleDataFromShapeToModel(*child);
    }

    // After Diagram import, parts of the Diagram ModelData is at the
    // oox::drawingml::Shape. Since these objects are temporary helpers,
    // secure that data at the Diagram ModelData by copying.

    // This is currently mainly the Text, but may get more (styles?)
    for (auto & point : maPoints)
    {
        Shape* pShapeCandidate(getOrCreateAssociatedShape(point));

        if(nullptr != pShapeCandidate)
        {
            if(pShapeCandidate->getTextBody() && !pShapeCandidate->getTextBody()->isEmpty())
            {
                point.msTextBody->msText = pShapeCandidate->getTextBody()->toString();

                const uno::Sequence< beans::PropertyValue > aTextProps(
                    pShapeCandidate->getTextBody()->getTextProperties().maPropertyMap.makePropertyValueSequence());

                for (auto const& prop : aTextProps)
                    point.msTextBody->maTextProps.push_back(std::pair(prop.Name, prop.Value));
            }

            // At this place a mechanism to find missing data should be added:
            // Create a Shape from so-far secured data & compare it with the
            // imported one. Report differences to allow extending the mechanism
            // more easily.
#ifdef DBG_UTIL
            // The original is pShapeCandidate, re-create potential new oox::drawingml::Shape
            // as aNew to be able to compare these
            ShapePtr aNew(std::make_shared<Shape>());
            restoreDataFromModelToShapeAfterReCreation(point, *aNew);

            // Unfortunately oox::drawingml::Shape has no operator==. I tried to add
            // one, but that is too expensive. I stopped at oox::drawingml::Color.
            // To compare it is necessary to use the debugger, or for single aspects
            // of the oox data it might be possible to call local dump() methods at
            // both instances to compare them/their output

            // bool bSame(aNew.get() == pShapeCandidate);
#endif
        }
    }
}

void DiagramData::restoreStyleDataFromShapeToModel(::oox::drawingml::Shape& rShape)
{
    const std::vector< ShapePtr >& rChildren(rShape.getChildren());

    if(!rChildren.empty())
    {
        // group shape
        for (auto& child : rChildren)
        {
            restoreStyleDataFromShapeToModel(*child);
        }

        // if group shape we are done. Do not restore properties for group shapes
        return;
    }

    // we need a XShape
    const css::uno::Reference< css::drawing::XShape > &rXShape(rShape.getXShape());
    if(!rXShape)
        return;

    // we need a ModelID for association
    if(rShape.getDiagramDataModelID().isEmpty())
        return;

    // define source to save to
    svx::diagram::PointStyle* pSource(nullptr);

    if(rShape.getDiagramDataModelID() == msBackgroundShapeModelID)
    {
        // if BackgroundShape, set BackgroundShapeStyle as source
        if(maBackgroundShapeStyle)
            pSource = maBackgroundShapeStyle.get();
    }
    else
    {
        // if Shape, seek association
        for (auto & point : maPoints)
        {
            if(point.msModelId == rShape.getDiagramDataModelID())
            {
                // found - create properties & set as source
                pSource = point.msPointStylePtr.get();

                // we are done, there is no 2nd shape with the same ModelID by definition
                break;
            }
        }
    }

    // no source -> nothing to do
    if(nullptr == pSource)
        return;

    // get target PropertySet of new XShape
    css::uno::Reference<css::beans::XPropertySet> xPropSet(rXShape, css::uno::UNO_QUERY);
    if(!xPropSet)
        return;

    // apply properties
    for (auto const& prop : pSource->maProperties)
    {
        xPropSet->setPropertyValue(prop.first, prop.second);
    }
}

void DiagramData::restoreDataFromShapeToModelAfterDiagramImport(::oox::drawingml::Shape& rRootShape)
{
    const std::vector< ShapePtr >& rChildren(rRootShape.getChildren());

    for (auto& child : rChildren)
    {
        restoreStyleDataFromShapeToModel(*child);
    }
}

DiagramData::DiagramData()
: svx::diagram::DiagramData()
, mpBackgroundShapeFillProperties( std::make_shared<FillProperties>() )
{
}

DiagramData::~DiagramData()
{
}

static void Connection_dump(const svx::diagram::Connection& rConnection)
{
    SAL_INFO(
        "oox.drawingml",
        "cnx modelId " << rConnection.msModelId << ", srcId " << rConnection.msSourceId << ", dstId "
            << rConnection.msDestId << ", parTransId " << rConnection.msParTransId << ", presId "
            << rConnection.msPresId << ", sibTransId " << rConnection.msSibTransId << ", srcOrd "
            << rConnection.mnSourceOrder << ", dstOrd " << rConnection.mnDestOrder);
}

static void Point_dump(const svx::diagram::Point& rPoint, const Shape* pShape)
{
    SAL_INFO(
        "oox.drawingml",
        "pt text " << pShape << ", cnxId " << rPoint.msCnxId << ", modelId "
            << rPoint.msModelId << ", type " << rPoint.mnXMLType);
}

void DiagramData::dump() const
{
    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of cnx: " << maConnections.size() );
    for (const auto& rConnection : maConnections)
        Connection_dump(rConnection);

    SAL_INFO("oox.drawingml", "Dgm: DiagramData # of pt: " << maPoints.size() );
    for (const auto& rPoint : maPoints)
        Point_dump(rPoint, getOrCreateAssociatedShape(rPoint));
}

void DiagramData::buildDiagramDataModel(bool bClearOoxShapes)
{
    if(bClearOoxShapes)
    {
        // Delete/remove all existing oox::drawingml::Shape
        maPointShapeMap.clear();
    }

    // call parent
    svx::diagram::DiagramData::buildDiagramDataModel(bClearOoxShapes);

    if(bClearOoxShapes)
    {
        // re-create all existing oox::drawingml::Shape
        svx::diagram::Points& rPoints = getPoints();

        for (auto & point : rPoints)
        {
            // Create/get shape. Re-create here, that may also set needed
            // and available data from the Diagram ModelData at the Shape
            getOrCreateAssociatedShape(point, true);
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
