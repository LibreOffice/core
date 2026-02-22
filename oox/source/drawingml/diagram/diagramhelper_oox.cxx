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

#include <oox/drawingml/diagram/diagramhelper_oox.hxx>
#include "diagram.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include <oox/ppt/pptimport.hxx>
#include <drawingml/fillproperties.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditer.hxx>
#include <svx/diagram/DomMapFlag.hxx>
#include <comphelper/processfactory.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace svx::diagram;

namespace oox::drawingml
{
bool DiagramHelper_oox::hasDiagramData() const { return mpDiagramPtr && mpDiagramPtr->getData(); }

DiagramHelper_oox::DiagramHelper_oox(std::shared_ptr<SmartArtDiagram> xDiagramPtr,
                                     std::shared_ptr<::oox::drawingml::Theme> xTheme,
                                     awt::Size aImportSize)
    : mpDiagramPtr(std::move(xDiagramPtr))
    , mpDiagramThemePtr(std::move(xTheme))
    , maDiagramImportSize(aImportSize)
    , msNewNodeId()
    , msNewNodeText()
{
}

DiagramHelper_oox::DiagramHelper_oox(DiagramHelper_oox const& rSource)
    : DiagramHelper_svx(rSource)
    , mpDiagramPtr(rSource.mpDiagramPtr ? new SmartArtDiagram(*rSource.mpDiagramPtr) : nullptr)
    , mpDiagramThemePtr(rSource.mpDiagramThemePtr)
    , maDiagramImportSize(rSource.maDiagramImportSize)
    , msNewNodeId()
    , msNewNodeText()
{
}

DiagramHelper_oox::~DiagramHelper_oox() {}

void DiagramHelper_oox::moveDiagramModelDataFromOldToNewXShape(
    const uno::Reference<drawing::XShape>& xOldShape,
    const uno::Reference<drawing::XShape>& xNewShape)
{
    SdrObject* pOldShape(SdrObject::getSdrObjectFromXShape(xOldShape));
    SdrObject* pNewShape(SdrObject::getSdrObjectFromXShape(xNewShape));

    if (nullptr == pOldShape || nullptr == pNewShape)
        return;

    // copy attributes
    pNewShape->SetMergedItemSet(pOldShape->GetMergedItemSet(), false, true);

    // copy Text/OutlinerParaObject
    OutlinerParaObject* pParaObject(pOldShape->GetOutlinerParaObject());
    if (nullptr != pParaObject)
        pNewShape->SetOutlinerParaObject(*pParaObject);

    // maybe copy more stuff...
}

uno::Reference<drawing::XShape>& DiagramHelper_oox::accessRootShape()
{
    assert(hasDiagramData() && "Access to XRootShape without DiagramData (!)");
    return mpDiagramPtr->getData()->accessRootShape();
}

void DiagramHelper_oox::reLayout()
{
    SdrObjGroup* pTarget(
        dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(accessRootShape())));
    if (nullptr == pTarget)
        return;

    // Rescue/remember geometric transformation of existing Diagram
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    pTarget->TRGetBaseGeometry(aTransformation, aPolyPolygon);

    // create temporary oox::Shape as target. No longer needed is to keep/remember
    // the original oox::Shape to do that. Use original Size and Pos from initial import
    // to get the same layout(s)
    oox::drawingml::ShapePtr pShapePtr = std::make_shared<Shape>("com.sun.star.drawing.GroupShape");
    pShapePtr->setDiagramType();
    pShapePtr->setSize(maDiagramImportSize);

    // remember existing DrawingLayerModelData. Do this before createShapeHierarchyFromModel
    // below, that will create a new BackgroundShapeModelID and the BGShape would
    // be missing.
    // Also important is to do this as XShapes, the content of the Group will delete the
    // SdrObjects in the process of re-creation, but the XShapes will survive
    std::vector<uno::Reference<drawing::XShape>> xOldXShapes;
    const bool bNewNodeMode(!msNewNodeId.isEmpty());
    uno::Reference<drawing::XShape> xOldShapeWithText;
    uno::Reference<drawing::XShape> xOldBGShape;
    {
        const OUString& rBGShapeID(mpDiagramPtr->getData()->getBackgroundShapeModelID());
        SdrObjListIter aIter(*pTarget, SdrIterMode::DeepNoGroups);
        while (aIter.IsMore())
        {
            SdrObject* pCandidate(aIter.Next());
            if (pCandidate->getDiagramDataModelID() == rBGShapeID)
                xOldBGShape = pCandidate->getUnoShape();
            else
            {
                uno::Reference<drawing::XShape> xCandidate(pCandidate->getUnoShape());
                xOldXShapes.push_back(xCandidate);

                if (bNewNodeMode && !xOldShapeWithText)
                {
                    uno::Reference<text::XText> xText(xCandidate, uno::UNO_QUERY);
                    if (xText && !xText->getString().isEmpty())
                        xOldShapeWithText = pCandidate->getUnoShape();
                }
            }
        }
    }

    // reset temporary buffered ModelData association lists & rebuild them
    // and the Diagram DataModel
    mpDiagramPtr->getData()->buildDiagramDataModel(true);

    // also reset temporary buffered layout data - that might
    // still refer to changed oox::Shape data
    mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();

    // Re-create the oox::Shapes for the diagram content
    mpDiagramPtr->createShapeHierarchyFromModel(pShapePtr, true);

    // Delete all existing shapes in that group to prepare re-creation
    pTarget->getChildrenOfSdrObject()->ClearSdrObjList();

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
    SdrModel& rModel(pTarget->getSdrModelFromSdrObject());
    uno::Reference<uno::XInterface> const& rUnoModel(rModel.getUnoModel());
    const uno::Reference<uno::XComponentContext>& xContext(
        comphelper::getProcessComponentContext());
    rtl::Reference<oox::shape::ShapeFilterBase> xFilter(new oox::shape::ShapeFilterBase(xContext));

    // set oox::Theme at Filter. All LineStyle/FillStyle/Colors/Attributes
    // will be taken from there
    if (UseDiagramThemeData())
        xFilter->setCurrentTheme(getOrCreateThemePtr(xFilter));

    uno::Reference<lang::XComponent> aComponentModel(rUnoModel, uno::UNO_QUERY);
    xFilter->setTargetDocument(aComponentModel);

    // set DiagramFontHeights
    xFilter->setDiagramFontHeights(&mpDiagramPtr->getDiagramFontHeights());

    // Prepare the target for the to-be-created XShapes
    uno::Reference<drawing::XShapes> xShapes(accessRootShape(), uno::UNO_QUERY_THROW);

    for (auto const& child : pShapePtr->getChildren())
    {
        // Create all sub-shapes. This will recursively create needed geometry using
        // filter-internal ::createShapes
        child->addShape(*xFilter, xFilter->getCurrentTheme(), xShapes, aTransformation,
                        pShapePtr->getFillProperties());
    }

    // Re-apply remembered geometry
    pTarget->TRSetBaseGeometry(aTransformation, aPolyPolygon);

    // extract newly created DrawingLayerModelData
    std::vector<uno::Reference<drawing::XShape>> xNewXShapes;
    uno::Reference<drawing::XShape> xNewShape;
    uno::Reference<drawing::XShape> xNewBGShape;
    {
        const OUString& rBGShapeID(mpDiagramPtr->getData()->getBackgroundShapeModelID());
        SdrObjListIter aIter(*pTarget, SdrIterMode::DeepNoGroups);
        while (aIter.IsMore())
        {
            SdrObject* pCandidate(aIter.Next());
            if (pCandidate->getDiagramDataModelID() == rBGShapeID)
                xNewBGShape = pCandidate->getUnoShape();
            else
            {
                xNewXShapes.push_back(pCandidate->getUnoShape());

                if (bNewNodeMode && !xNewShape
                    && msNewNodeId == pCandidate->getDiagramDataModelID())
                    xNewShape = pCandidate->getUnoShape();
            }
        }
    }

    if (xOldBGShape && xNewBGShape)
    {
        // we have old and new BGShapes, copy necessary data
        moveDiagramModelDataFromOldToNewXShape(xOldBGShape, xNewBGShape);
    }

    for (const auto& rNewShape : xNewXShapes)
    {
        if (!rNewShape)
            continue;

        SdrObject* pNewShape(SdrObject::getSdrObjectFromXShape(rNewShape));
        if (nullptr == pNewShape)
            continue;

        const OUString& rNewModelID(pNewShape->getDiagramDataModelID());
        if (rNewModelID.isEmpty())
            continue;

        for (const auto& rOldShape : xOldXShapes)
        {
            if (rOldShape)
            {
                SdrObject* pOldShape(SdrObject::getSdrObjectFromXShape(rOldShape));
                if (nullptr != pOldShape)
                {
                    if (rNewModelID == pOldShape->getDiagramDataModelID())
                    {
                        // we have old and new version of this shape, copy necessary data
                        moveDiagramModelDataFromOldToNewXShape(rOldShape, rNewShape);
                        break;
                    }
                }
            }
        }
    }

    if (bNewNodeMode)
    {
        if (xOldShapeWithText && xNewShape)
        {
            // a shape was added in DomTree model and the model counter part in XShapes
            // is not filled yet
            SdrObject* pOldShape(SdrObject::getSdrObjectFromXShape(xOldShapeWithText));
            SdrObject* pNewShape(SdrObject::getSdrObjectFromXShape(xNewShape));

            if (nullptr != pOldShape && nullptr != pNewShape)
            {
                // copy attributes
                pNewShape->SetMergedItemSet(pOldShape->GetMergedItemSet(), false, true);

                OutlinerParaObject* pParaObject(pOldShape->GetOutlinerParaObject());
                if (nullptr != pParaObject)
                {
                    pNewShape->SetOutlinerParaObject(*pParaObject);
                    uno::Reference<text::XText> xText(xNewShape, uno::UNO_QUERY);
                    if (xText)
                        xText->insertString(xText->getStart(), msNewNodeText, true);
                }
            }
        }

        msNewNodeId.clear();
        msNewNodeText.clear();
    }
}

OUString DiagramHelper_oox::getDiagramString() const
{
    if (hasDiagramData())
    {
        return mpDiagramPtr->getData()->getDiagramString();
    }

    return OUString();
}

std::vector<std::pair<OUString, OUString>>
DiagramHelper_oox::getDiagramChildren(const OUString& rParentId) const
{
    if (hasDiagramData())
    {
        return mpDiagramPtr->getData()->getDiagramChildren(rParentId);
    }

    return std::vector<std::pair<OUString, OUString>>();
}

OUString DiagramHelper_oox::addDiagramNode(const OUString& rText)
{
    OUString aRetval;

    if (hasDiagramData())
    {
        const std::pair<OUString, DomMapFlags> aResult = mpDiagramPtr->getData()->addDiagramNode();
        aRetval = aResult.first;

        // reset Dom properties at DiagramData
        mpDiagramPtr->resetOOXDomValues(aResult.second);

        // reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel
        mpDiagramPtr->getData()->buildDiagramDataModel(true);

        // also reset temporary buffered layout data - that might
        // still refer to changed oox::Shape data
        mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();

        // we have the text node in aRetval, but we need the ModelID
        // of the node referring that one, that is the one that will be used
        // as ModelID in the XShape/SdrObject. Loop and look for it
        for (const auto& rCandidate : mpDiagramPtr->getData()->getPoints())
        {
            if (!rCandidate.msPresentationAssociationId.isEmpty()
                && rCandidate.msPresentationAssociationId == aRetval)
            {
                msNewNodeId = rCandidate.msModelId;
                break;
            }
        }

        // msNewNodeId = aRetval;
        msNewNodeText = rText;
    }

    return aRetval;
}

bool DiagramHelper_oox::removeDiagramNode(const OUString& rNodeId)
{
    bool bRetval(false);

    if (hasDiagramData())
    {
        DomMapFlags aResult = mpDiagramPtr->getData()->removeDiagramNode(rNodeId);
        bRetval = !aResult.empty();

        // reset Dom properties at DiagramData
        mpDiagramPtr->resetOOXDomValues(std::move(aResult));

        // reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel
        mpDiagramPtr->getData()->buildDiagramDataModel(true);

        // also reset temporary buffered layout data - that might
        // still refer to changed oox::Shape data
        mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();
    }

    return bRetval;
}

void DiagramHelper_oox::TextInformationChange()
{
    if (!mpDiagramPtr)
        return;

    DomMapFlags aFlags;
    aFlags.push_back(DomMapFlag::OOXData);
    aFlags.push_back(DomMapFlag::OOXDrawing);
    aFlags.push_back(DomMapFlag::OOXDataImageRels);
    aFlags.push_back(DomMapFlag::OOXDataHlinkRels);
    aFlags.push_back(DomMapFlag::OOXDrawingImageRels);
    aFlags.push_back(DomMapFlag::OOXDrawingHlinkRels);
    mpDiagramPtr->resetOOXDomValues(std::move(aFlags));

    // still reset GrabBag at Associated SdrObjGroup object. There are no "OOX.*"
    // entries anymore, but others like "mso-rotation-angle" and others
    uno::Reference<beans::XPropertySet> xPropSet(accessRootShape(), uno::UNO_QUERY);
    if (xPropSet->getPropertySetInfo()->hasPropertyByName(u"InteropGrabBag"_ustr))
        xPropSet->setPropertyValue(u"InteropGrabBag"_ustr,
                                   uno::Any(uno::Sequence<beans::PropertyValue>()));
}

DiagramDataStatePtr DiagramHelper_oox::extractDiagramDataState() const
{
    if (!mpDiagramPtr)
    {
        return DiagramDataStatePtr();
    }

    return mpDiagramPtr->getData()->extractDiagramDataState();
}

void DiagramHelper_oox::applyDiagramDataState(const DiagramDataStatePtr& rState)
{
    if (!mpDiagramPtr)
    {
        return;
    }

    mpDiagramPtr->getData()->applyDiagramDataState(rState);
}

void DiagramHelper_oox::doAnchor(uno::Reference<drawing::XShape>& rTarget)
{
    if (!mpDiagramPtr || !rTarget)
    {
        return;
    }

    // sync FontHeights
    mpDiagramPtr->syncDiagramFontHeights();

    // initialize connection to GroupObject
    connectToSdrObjGroup(rTarget);
}

const std::shared_ptr<::oox::drawingml::Theme>& DiagramHelper_oox::getOrCreateThemePtr(
    const rtl::Reference<oox::shape::ShapeFilterBase>& rxFilter) const
{
    // (Re-)Use already existing Theme if existing/imported if possible.
    // If not, re-import Theme if data is available and thus possible
    if (hasDiagramData() && (ForceThemePtrRecreation() || !mpDiagramThemePtr))
    {
        // get the originally imported dom::XDocument
        const uno::Reference<xml::dom::XDocument>& xThemeDocument(
            mpDiagramPtr->getData()->getThemeDocument());

        if (xThemeDocument)
        {
            // reset local Theme ModelData *always* to get rid of former data that would
            // else be added additionally
            const_cast<DiagramHelper_oox*>(this)->mpDiagramThemePtr
                = std::make_shared<oox::drawingml::Theme>();
            auto pTheme = std::make_shared<model::Theme>();
            mpDiagramThemePtr->setTheme(pTheme);

            // import Theme ModelData
            rxFilter->importFragment(
                new ThemeFragmentHandler(*rxFilter, OUString(), *mpDiagramThemePtr, *pTheme),
                uno::Reference<xml::sax::XFastSAXSerializable>(xThemeDocument,
                                                               uno::UNO_QUERY_THROW));
        }
    }

    return mpDiagramThemePtr;
}

void DiagramHelper_oox::setOOXDomValue(DomMapFlag aDomMapFlag, const uno::Any& rValue)
{
    if (mpDiagramPtr)
        mpDiagramPtr->setOOXDomValue(aDomMapFlag, rValue);
}

uno::Any DiagramHelper_oox::getOOXDomValue(DomMapFlag aDomMapFlag) const
{
    if (mpDiagramPtr)
        return mpDiagramPtr->getOOXDomValue(aDomMapFlag);

    return uno::Any();
}

bool DiagramHelper_oox::checkMinimalDataDoms() const
{
    if (!mpDiagramPtr)
        return false;

    return mpDiagramPtr->checkMinimalDataDoms();
}

void DiagramHelper_oox::writeDiagramOOXData(DrawingML& rOriginalDrawingML,
                                            uno::Reference<io::XOutputStream>& xOutputStream,
                                            std::u16string_view rDrawingRelId) const
{
    if (!mpDiagramPtr)
        return;

    mpDiagramPtr->writeDiagramOOXData(rOriginalDrawingML, xOutputStream, rDrawingRelId);
}

void DiagramHelper_oox::writeDiagramOOXDrawing(
    DrawingML& rOriginalDrawingML, uno::Reference<io::XOutputStream>& xOutputStream) const
{
    if (!mpDiagramPtr)
        return;

    mpDiagramPtr->writeDiagramOOXDrawing(rOriginalDrawingML, xOutputStream);
}

DiagramHelper_oox* DiagramHelper_oox::clone() const
{
    if (!mpDiagramPtr)
        return nullptr;

    return new DiagramHelper_oox(*this);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
