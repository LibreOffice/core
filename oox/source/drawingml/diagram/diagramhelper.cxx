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
#include <drawingml/fillproperties.hxx>
#include <svx/svdmodel.hxx>
#include <comphelper/processfactory.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <utility>

using namespace ::com::sun::star;

namespace oox::drawingml {

bool AdvancedDiagramHelper::hasDiagramData() const
{
    return mpDiagramPtr && mpDiagramPtr->getData();
}

AdvancedDiagramHelper::AdvancedDiagramHelper(
    std::shared_ptr< Diagram > xDiagramPtr,
    std::shared_ptr<::oox::drawingml::Theme> xTheme,
    css::awt::Size aImportSize,
    bool bSelfCreated)
: svx::diagram::IDiagramHelper(bSelfCreated)
, mpDiagramPtr(std::move(xDiagramPtr))
, mpThemePtr(std::move(xTheme))
, maImportSize(aImportSize)
{
}

AdvancedDiagramHelper::~AdvancedDiagramHelper()
{
}

void AdvancedDiagramHelper::reLayout(SdrObjGroup& rTarget)
{
    if(!mpDiagramPtr)
    {
        return;
    }

    // Rescue/remember geometric transformation of existing Diagram
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    rTarget.TRGetBaseGeometry(aTransformation, aPolyPolygon);

    // create temporary oox::Shape as target. No longer needed is to keep/remember
    // the original oox::Shape to do that. Use original Size and Pos from initial import
    // to get the same layout(s)
    oox::drawingml::ShapePtr pShapePtr = std::make_shared<Shape>( "com.sun.star.drawing.GroupShape" );
    pShapePtr->setDiagramType();
    pShapePtr->setSize(maImportSize);

    // Re-create the oox::Shapes for the diagram content
    mpDiagramPtr->addTo(pShapePtr, true);

    // Delete all existing shapes in that group to prepare re-creation
    rTarget.getChildrenOfSdrObject()->ClearSdrObjList();

    // For re-creation we need to use ::addShape functionality from the
    // oox import filter since currently Shape import is very tightly
    // coupled to Shape creation. It converts a oox::Shape representation
    // combined with an oox::Theme to incarnated XShapes representing the
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
    SdrModel& rModel(rTarget.getSdrModelFromSdrObject());
    uno::Reference< uno::XInterface > const & rUnoModel(rModel.getUnoModel());
    css::uno::Reference<css::uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    rtl::Reference<oox::shape::ShapeFilterBase> xFilter(new oox::shape::ShapeFilterBase(xContext));

    // set oox::Theme at Filter. All LineStyle/FillStyle/Colors/Attributes
    // will be taken from there
    // also need to use theme when geometry gets self-created the first time
    if(UseDiagramThemeData() || !isSelfCreated())
        xFilter->setCurrentTheme(getOrCreateThemePtr(xFilter));

    css::uno::Reference< css::lang::XComponent > aComponentModel( rUnoModel, uno::UNO_QUERY );
    xFilter->setTargetDocument(aComponentModel);

    // set DiagramFontHeights
    xFilter->setDiagramFontHeights(&mpDiagramPtr->getDiagramFontHeights());

    // Prepare the target for the to-be-created XShapes
    uno::Reference<drawing::XShapes> xShapes(rTarget.getUnoShape(), uno::UNO_QUERY_THROW);

    for (auto const& child : pShapePtr->getChildren())
    {
        // Create all sub-shapes. This will recursively create needed geometry using
        // filter-internal ::createShapes
        child->addShape(
            *xFilter,
            xFilter->getCurrentTheme(),
            xShapes,
            aTransformation,
            pShapePtr->getFillProperties());
    }

    // sync FontHeights
    mpDiagramPtr->syncDiagramFontHeights();

    if (isSelfCreated())
    {
        // already secured at import, re-apply secured data from ModelData
        if(UseDiagramModelData())
            mpDiagramPtr->getData()->restoreDataFromShapeToModelAfterDiagramImport(*pShapePtr);
    }
    else
    {
        // secure data from ModelData for the 1st time for shapes except BackgroundShape
        if(UseDiagramModelData())
            mpDiagramPtr->getData()->secureDataFromShapeToModelAfterDiagramImport(*pShapePtr);

        // note that shapes are now self-created
        setSelfCreated();
    }

    // Re-apply remembered geometry
    rTarget.TRSetBaseGeometry(aTransformation, aPolyPolygon);
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
    OUString aRetval;

    if(hasDiagramData())
    {
        aRetval = mpDiagramPtr->getData()->addNode(rText);

        // reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel
        mpDiagramPtr->getData()->buildDiagramDataModel(true);

        // also reset temporary buffered layout data - that might
        // still refer to changed oox::Shape data
        mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();
    }

    return aRetval;
}

bool AdvancedDiagramHelper::removeNode(const OUString& rNodeId)
{
    bool bRetval(false);

    if(hasDiagramData())
    {
        bRetval = mpDiagramPtr->getData()->removeNode(rNodeId);

        // reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel
        mpDiagramPtr->getData()->buildDiagramDataModel(true);

        // also reset temporary buffered layout data - that might
        // still refer to changed oox::Shape data
        mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();
    }

    return bRetval;
}

svx::diagram::DiagramDataStatePtr AdvancedDiagramHelper::extractDiagramDataState() const
{
    if(!mpDiagramPtr)
    {
        return svx::diagram::DiagramDataStatePtr();
    }

    return mpDiagramPtr->getData()->extractDiagramDataState();
}

void AdvancedDiagramHelper::applyDiagramDataState(const svx::diagram::DiagramDataStatePtr& rState)
{
    if(!mpDiagramPtr)
    {
        return;
    }

    mpDiagramPtr->getData()->applyDiagramDataState(rState);
}

void AdvancedDiagramHelper::doAnchor(SdrObjGroup& rTarget, ::oox::drawingml::Shape& rRootShape)
{
    if(!mpDiagramPtr)
    {
        return;
    }

    mpDiagramPtr->syncDiagramFontHeights();

    // After Diagram import, parts of the Diagram ModelData is at the
    // oox::drawingml::Shape. Since these objects are temporary helpers,
    // secure that data at the Diagram ModelData by copying.
    mpDiagramPtr->getData()->secureDataFromShapeToModelAfterDiagramImport(rRootShape);

    anchorToSdrObjGroup(rTarget);
}

const std::shared_ptr< ::oox::drawingml::Theme >& AdvancedDiagramHelper::getOrCreateThemePtr(
    rtl::Reference< oox::shape::ShapeFilterBase >& rxFilter) const
{
    // (Re-)Use already existing Theme if existing/imported if possible.
    // If not, re-import Theme if data is available and thus possible
    if(hasDiagramData() && (ForceThemePtrRecreation() || !mpThemePtr))
    {
        // get the originally imported dom::XDocument
        const uno::Reference< css::xml::dom::XDocument >& xThemeDocument(mpDiagramPtr->getData()->getThemeDocument());

        if(xThemeDocument)
        {
            // reset local Theme ModelData *always* to get rid of former data that would
            // else be added additionally
            const_cast<AdvancedDiagramHelper*>(this)->mpThemePtr = std::make_shared<oox::drawingml::Theme>();
            auto pTheme = std::make_shared<model::Theme>();
            mpThemePtr->setTheme(pTheme);

            // import Theme ModelData
            rxFilter->importFragment(
                new ThemeFragmentHandler(*rxFilter, OUString(), *mpThemePtr, *pTheme),
                uno::Reference< css::xml::sax::XFastSAXSerializable >(
                    xThemeDocument,
                    uno::UNO_QUERY_THROW));
        }
    }

    return mpThemePtr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
