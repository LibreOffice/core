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
    if(rPoint.msTextBody && !rPoint.msTextBody->msText.isEmpty())
    {
        TextBodyPtr aNewTextBody(std::make_shared<TextBody>());
        rNewShape.setTextBody(aNewTextBody);
        TextRunPtr pTextRun = std::make_shared<TextRun>();
        pTextRun->getText() = rPoint.msTextBody->msText;
        aNewTextBody->addParagraph().addRun(pTextRun);

        if(rPoint.msTextBody->maTextProps.hasElements())
        {
            oox::PropertyMap& rTargetMap(aNewTextBody->getTextProperties().maPropertyMap);

            for (auto const& prop : rPoint.msTextBody->maTextProps)
            {
                sal_Int32 nPropId(oox::PropertyMap::getPropertyId(prop.Name));
                if(nPropId > 0)
                    rTargetMap.setAnyProperty(nPropId, prop.Value);
            }
        }
    }
}

void DiagramData::secureDataFromShapeToModelAfterDiagramImport()
{
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
                point.msTextBody = std::make_shared<svx::diagram::TextBody>();
                point.msTextBody->msText = pShapeCandidate->getTextBody()->toString();
                point.msTextBody->maTextProps = pShapeCandidate->getTextBody()->getTextProperties().maPropertyMap.makePropertyValueSequence();
            }

            // At this place a mechanism to find missing data should be added:
            // Create a Shape from so-far secured data & compare it with the
            // imported one. Report differences to allow extending the mechanism
            // more easily.
#ifdef DBG_UTIL
            // The oiginal is pShapeCandidate, re-create potential new oox::drawingml::Shape
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

DiagramData::DiagramData()
: svx::diagram::DiagramData()
, mpFillProperties( std::make_shared<FillProperties>() )
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
