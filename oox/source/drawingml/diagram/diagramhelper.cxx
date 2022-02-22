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

#include "diagramhelper.hxx"
#include "diagram.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include <oox/ppt/pptimport.hxx>
#include <svx/svdmodel.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace oox::drawingml {

bool AdvancedDiagramHelper::hasDiagramData() const
{
    return mpDiagramPtr && mpDiagramPtr->getData();
}

AdvancedDiagramHelper::AdvancedDiagramHelper(
    const std::shared_ptr< Diagram >& rDiagramPtr,
    const std::shared_ptr<::oox::drawingml::Theme>& rTheme)
: IDiagramHelper()
, mpDiagramPtr(rDiagramPtr)
, mpThemePtr(rTheme)
{
}

AdvancedDiagramHelper::~AdvancedDiagramHelper()
{
}

void AdvancedDiagramHelper::reLayout()
{
    if(!mpDiagramPtr)
    {
        return;
    }

    // Get the oox::Shape that represents the Diagram GraphicObject
    const ShapePtr & pParentShape = mpDiagramPtr->getShape();

    if(!pParentShape)
    {
        return;
    }

    // Remove it's children which represent the oox::Shapes created by
    // the layout process as preparation to re-creation. These should
    // already be cleared, but make sure.
    pParentShape->getChildren().clear();

    // Re-create the oox::Shapes for the diagram content
    mpDiagramPtr->addTo(pParentShape);

    // Access the GroupObject representing the SmartArt in DrawingLayer
    SdrObjGroup* pAnchorObj(dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(pParentShape->getXShape())));

    // Rescue/remember geometric transformation of existing Diagram
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    pAnchorObj->TRGetBaseGeometry(aTransformation, aPolyPolygon);

    // Delete all existing shapes in that group to prepare re-creation
    pAnchorObj->getChildrenOfSdrObject()->ClearSdrObjList();

    // For re-creation we need to use ::addShape functionality from the
    // oox import filter since currently Shape import is very tightly
    // coupled to Shape creation. It converts a oox::Shape representation
    // combined with an oox::Theme to incarrnated XShapes representing the
    // Diagram.
    // To use that functionality, we have to create a temporary filter
    // (based on ShapeFilterBase). Problems are that this needs to know
    // the oox:Theme and a ComponentModel from TargetDocument.
    // The DiagramHelper holds/delivers the oox::Theme to use, so
    // it does not need to be re-imported from oox repeatedly.
    // The ComponentModel can be derived from the existing XShape/GroupShape
    // when knowing where to get it from, making it independent from app.
    //
    // NOTE: Using another (buffered) oox::Theme would allow to re-create
    //       using another theming in the future.
    // NOTE: The incarnation of import filter (ShapeFilterBase) is only
    //       used for XShape creation, no xml snippets/data gets imported
    //       here. XShape creation may be isolated in the future.
    SdrModel& rModel(pAnchorObj->getSdrModelFromSdrObject());
    uno::Reference< uno::XInterface > const & rUnoModel(rModel.getUnoModel());
    css::uno::Reference<css::uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    rtl::Reference<oox::shape::ShapeFilterBase> xFilter(new oox::shape::ShapeFilterBase(xContext));
    xFilter->setCurrentTheme(mpThemePtr);
    css::uno::Reference< css::lang::XComponent > aComponentModel( rUnoModel, uno::UNO_QUERY );
    xFilter->setTargetDocument(aComponentModel);

    // Prepare the target for the to-be-created XShapes
    uno::Reference<drawing::XShapes> xShapes(pParentShape->getXShape(), uno::UNO_QUERY_THROW);

    for (auto const& child : pParentShape->getChildren())
    {
        // Create all sub-shapes. This will recursively create needed geometry using
        // filter-internal ::createShapes
        child->addShape(
            *xFilter,
            xFilter->getCurrentTheme(),
            xShapes,
            aTransformation,
            pParentShape->getFillProperties());
    }

    // Re-apply remembered geometry
    pAnchorObj->TRSetBaseGeometry(aTransformation, aPolyPolygon);

    // Delete oox::Shapes that represented the content of the
    // diagram. These were needed for creating the XShapes/SdrObjects
    // (created by ::addTo above) but are no longer needed, so free
    // the memory
    pParentShape->getChildren().clear();
}

OUString AdvancedDiagramHelper::getString() const
{
    if(hasDiagramData())
    {
        return mpDiagramPtr->getData()->getString();
    }

    return OUString();
}

std::vector<std::pair<OUString, OUString>> AdvancedDiagramHelper::getChildren(const OUString& rParentId) const
{
    if(hasDiagramData())
    {
        return mpDiagramPtr->getData()->getChildren(rParentId);
    }

    return std::vector<std::pair<OUString, OUString>>();
}

OUString AdvancedDiagramHelper::addNode(const OUString& rText)
{
    if(hasDiagramData())
    {
        return mpDiagramPtr->getData()->addNode(rText);
    }

    return OUString();
}

bool AdvancedDiagramHelper::removeNode(const OUString& rNodeId)
{
    if(hasDiagramData())
    {
        return mpDiagramPtr->getData()->removeNode(rNodeId);
    }

    return false;
}

void AdvancedDiagramHelper::doAnchor(SdrObjGroup& rTarget)
{
    if(!mpDiagramPtr)
    {
        return;
    }

    const ShapePtr& pParentShape(mpDiagramPtr->getShape());

    if(pParentShape)
    {
        // The oox::Shapes childs are not needed for holding the original data,
        // free that memory
        pParentShape->getChildren().clear();
    }

    anchorToSdrObjGroup(rTarget);
}

void AdvancedDiagramHelper::newTargetShape(ShapePtr& pTarget)
{
    if(!mpDiagramPtr)
    {
        return;
    }

    mpDiagramPtr->newTargetShape(pTarget);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
