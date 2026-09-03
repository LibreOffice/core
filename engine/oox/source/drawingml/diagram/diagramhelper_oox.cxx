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
#include <svx/svdundo.hxx>
#include <svx/sdasitm.hxx>
#include <editeng/outlobj.hxx>
#include <unordered_map>
#include <svx/svddef.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <svx/sdooitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <comphelper/processfactory.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <utility>

#ifdef DBG_UTIL
#include <osl/file.hxx>
#include <o3tl/environment.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#endif

using namespace ::com::sun::star;
using namespace svx::diagram;

namespace oox::drawingml
{
bool DiagramHelper_oox::hasDiagramData() const { return mpDiagramPtr && mpDiagramPtr->getData(); }

DiagramHelper_oox::DiagramHelper_oox(std::shared_ptr<SmartArtDiagram> xDiagramPtr,
                                     std::shared_ptr<::oox::drawingml::Theme> xTheme)
    : mpDiagramPtr(std::move(xDiagramPtr))
    , mpDiagramThemePtr(std::move(xTheme))
    , msNewNodeId()
    , msNewNodeText()
    , msNewNodeTemplateId()
{
}

DiagramHelper_oox::DiagramHelper_oox(DiagramHelper_oox const& rSource)
    : DiagramHelper_svx(rSource)
    , mpDiagramPtr(rSource.mpDiagramPtr ? new SmartArtDiagram(*rSource.mpDiagramPtr) : nullptr)
    , mpDiagramThemePtr(rSource.mpDiagramThemePtr)
    , msNewNodeId()
    , msNewNodeText()
    , msNewNodeTemplateId()
{
}

DiagramHelper_oox::DiagramHelper_oox(std::u16string_view rLayout, std::u16string_view rData,
                                     std::u16string_view rColors, std::u16string_view rQuickstyle)
    : DiagramHelper_svx()
    , mpDiagramPtr(new SmartArtDiagram(rLayout, rData, rColors, rQuickstyle))
    , mpDiagramThemePtr()
    , msNewNodeId()
    , msNewNodeText()
    , msNewNodeTemplateId()
{
}

DiagramHelper_oox::~DiagramHelper_oox() {}

void DiagramHelper_oox::moveDiagramModelDataFromOldToNewXShape(
    const uno::Reference<drawing::XShape>& xOldShape,
    const uno::Reference<drawing::XShape>& xNewShape, tools::Long nOldHeight)
{
    SdrObject* pOldShape(SdrObject::getSdrObjectFromXShape(xOldShape));
    SdrObject* pNewShape(SdrObject::getSdrObjectFromXShape(xNewShape));

    if (nullptr == pOldShape || nullptr == pNewShape)
        return;

    // copy attributes. We need to remove SdrCustomShapeGeometryItem here, this
    // carries geometry info and would destroy the just re-created geometry - a
    // bad trap.
    SfxItemSet aAttributes(pOldShape->GetMergedItemSet());
    aAttributes.ClearItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    pNewShape->SetMergedItemSet(aAttributes, false, true);

    // copy Text/OutlinerParaObject, with the sizes of the letters following the height that the
    // shape around them ends up with
    OutlinerParaObject* pParaObject(pOldShape->GetOutlinerParaObject());

    if (nullptr != pParaObject)
    {
        pNewShape->SetOutlinerParaObject(*pParaObject);

        const tools::Long nNewHeight(xNewShape->getSize().Height);

        // Now scale the text sizes
        if (nOldHeight > 0 && nNewHeight > 0)
            pNewShape->scaleText(static_cast<double>(nNewHeight) / static_cast<double>(nOldHeight),
                                 /*bUndo=*/false);
    }


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
    // the original oox::Shape to do that. Use local model data from Diagram root
    // shape to get the same layout(s)
    oox::drawingml::ShapePtr pShapePtr
        = std::make_shared<Shape>(u"com.sun.star.drawing.GroupShape"_ustr);
    pShapePtr->setDiagramType();

    // set the Size, this is important to let the layout mechanism work
    // correctly. Since we use the XShape/SdrObject hierarchy as part of
    // the model data, get size from there.
    // Create bounding range using unit coordinates and the object
    // transformation
    const basegfx::B2DRange aRootRange(aTransformation * basegfx::B2DPoint(0, 0), // top-left
                                       aTransformation * basegfx::B2DPoint(1, 1)); // bottom-right

    // also need to convert to Emu used by mso and thus in oox::Shape stuff
    pShapePtr->setSize(
        awt::Size(oox::drawingml::convertHmmToEmu(basegfx::fround(aRootRange.getWidth())),
                  oox::drawingml::convertHmmToEmu(basegfx::fround(aRootRange.getHeight()))));

    // remember exsiting DrawingLayerModelData. Do this before createShapeHierarchyFromModel
    // below, that will create a new BackgroundShapeModelID and the BGShape would
    // be missing.
    // Also important is to do this as XShapes, the content of the Group will delete the
    // SdrObjects in the process of re-creation, but the XShapes will survive
    std::vector<uno::Reference<drawing::XShape>> xOldXShapes;

    // Secure the height that each object covers to be able to later scale the text.
    // Text is currently linear scaled to the size changes of the hosting frame.
    std::unordered_map<OUString, tools::Long> aOldHeightForModelID;
    const bool bNewNodeMode(!msNewNodeId.isEmpty());
    uno::Reference<drawing::XShape> xNewShapeTemplate;

    // The first Object that represents one of the nodes of the Diagram. A layout also represents
    // shapes that belong to the Diagram as a whole, a background arrow for instance, and those
    // carry a representation of their own that a node must not take over.
    uno::Reference<drawing::XShape> xFirstDataNodeShape;
    uno::Reference<drawing::XShape> xOldBGShape;
    {
        SdrObjListIter aIter(*pTarget, SdrIterMode::DeepNoGroups);
        while (aIter.IsMore())
        {
            SdrObject* pCandidate(aIter.Next());
            if (pCandidate->getDiagramDataModelID() == EMPTY_OUSTRING)
                // empty string -> is BGShape
                xOldBGShape = pCandidate->getUnoShape();
            else
            {
                uno::Reference<drawing::XShape> xCandidate(pCandidate->getUnoShape());
                xOldXShapes.push_back(xCandidate);
                aOldHeightForModelID[pCandidate->getDiagramDataModelID()]
                    = xCandidate->getSize().Height;

                if (bNewNodeMode)
                {
                    const OUString& rCandidateId(pCandidate->getDiagramDataModelID());
                    const bool bDrawsDataNode(
                        mpDiagramPtr->getData()->isPresentationOfDataNode(rCandidateId));

                    if (bDrawsDataNode && !xFirstDataNodeShape)
                        xFirstDataNodeShape = xCandidate;

                    // grep the Object that the new node was added next to as template for
                    // split-model data copying
                    if (bDrawsDataNode && !xNewShapeTemplate && msNewNodeTemplateId == rCandidateId)
                        xNewShapeTemplate = xCandidate;
                }
            }
        }

        if (bNewNodeMode && !xNewShapeTemplate)
        {
            // No Object was named to copy from, because nothing was selected, or the one that was
            // named is not there any more or draws no node of the Diagram. The first Object that
            // draws a node hands over its look instead, and with the Diagram drawing no node at
            // all whatever it holds has to do.
            if (xFirstDataNodeShape)
                xNewShapeTemplate = xFirstDataNodeShape;
            else if (!xOldXShapes.empty())
                xNewShapeTemplate = xOldXShapes[0];
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
    const uno::Reference<cpo::uno::XComponentContext>& xContext(
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

    // new SdrObjects created, re-apply geometry change locks as needed
    // and reset SubSelection
    applyLocksToDiagramObjects(true);
    setSelectedModelID(EMPTY_OUSTRING);

    // extract newly created DrawingLayerModelData
    std::vector<uno::Reference<drawing::XShape>> xNewXShapes;
    uno::Reference<drawing::XShape> xNewShape;
    uno::Reference<drawing::XShape> xNewBGShape;
    {
        SdrObjListIter aIter(*pTarget, SdrIterMode::DeepNoGroups);
        while (aIter.IsMore())
        {
            SdrObject* pCandidate(aIter.Next());
            if (pCandidate->getDiagramDataModelID() == EMPTY_OUSTRING)
                // empty string -> is BGShape
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
        moveDiagramModelDataFromOldToNewXShape(xOldBGShape, xNewBGShape, 0);
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
                        const auto aOldHeight(aOldHeightForModelID.find(rNewModelID));
                        moveDiagramModelDataFromOldToNewXShape(
                            rOldShape, rNewShape,
                            aOldHeightForModelID.end() == aOldHeight ? 0 : aOldHeight->second);
                        break;
                    }
                }
            }
        }
    }

    if (bNewNodeMode)
    {
        if (xNewShapeTemplate && xNewShape)
        {
            // a shape was added in DomTree model and the model counter part in XShapes
            // is not filled yet
            SdrObject* pOldShape(SdrObject::getSdrObjectFromXShape(xNewShapeTemplate));

            if (nullptr != pOldShape)
            {
                // take the look of the template, text included
                const auto aTemplateHeight(aOldHeightForModelID.find(msNewNodeTemplateId));
                moveDiagramModelDataFromOldToNewXShape(
                    xNewShapeTemplate, xNewShape,
                    aOldHeightForModelID.end() == aTemplateHeight ? 0 : aTemplateHeight->second);

                // the text that came with the template gives way to the text of the new node
                if (nullptr != pOldShape->GetOutlinerParaObject())
                {
                    uno::Reference<text::XText> xText(xNewShape, uno::UNO_QUERY);
                    if (xText)
                        xText->setString(msNewNodeText);
                }
            }
        }

        msNewNodeId.clear();
        msNewNodeText.clear();
        msNewNodeTemplateId.clear();
    }

    // make DiagramNodes use FitSize in edit mode
    applyTextFitToSizeToDiagramNodes(accessRootShape(), /*bKeepFontScale*/false);
    mpDiagramPtr->syncDiagramFontHeights();
}

OUString DiagramHelper_oox::getDiagramString() const
{
    if (hasDiagramData())
    {
        return mpDiagramPtr->getData()->getDiagramString();
    }

    return EMPTY_OUSTRING;
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

bool DiagramHelper_oox::isChildNode(std::u16string_view rNodeId) const
{
    if (hasDiagramData())
    {
        return mpDiagramPtr->getData()->isChildNode(rNodeId);
    }

    return false;
}

bool DiagramHelper_oox::canHoldChildNode(std::u16string_view rNodeId) const
{
    return hasDiagramData() && mpDiagramPtr->getData()->canHoldChildNode(rNodeId);
}

namespace
{
// The presentation Point that represents a data node, which is what an XShape of the Diagram carries as
// its ModelId. A presOf Connection names it, and a node that no presOf Connection reaches is drawn
// by the presentation Points that name the node instead. Gives an empty string for a node that
// represents nothing.
OUString readShapeIdOfNode(const svx::diagram::DiagramData_svx& rData, std::u16string_view rNodeId)
{
    if (rNodeId.empty())
        return OUString();

    for (const rtl::Reference<svx::diagram::Connection>& rCxn : rData.getConnections())
        if (svx::diagram::TypeConstant::XML_presOf == rCxn->mnXMLType
            && rCxn->msSourceId == rNodeId)
            return rCxn->msDestId;

    for (const rtl::Reference<svx::diagram::Point>& rPoint : rData.getPoints())
        if (!rPoint->getPresentation().msPresentationAssociationId.isEmpty()
            && rPoint->getPresentation().msPresentationAssociationId == rNodeId)
            return rPoint->msModelId;

    return OUString();
}
}

namespace
{
// The node that a new node is relative to.
OUString readAnchorNode(const svx::diagram::DiagramData_svx& rData,
                        std::u16string_view rAnchorNode)
{
    if (rAnchorNode.empty())
        return OUString();

    if (rData.isPresentationOfDataNode(rAnchorNode))
        return OUString(rAnchorNode);

    const rtl::Reference<svx::diagram::Point> xPoint(rData.getPointByModelID(rAnchorNode));

    if (xPoint.is() && svx::diagram::TypeConstant::XML_node == xPoint->mnXMLType)
        return OUString(rAnchorNode);

    return OUString();
}
}

OUString DiagramHelper_oox::addDiagramNode(std::u16string_view rText, SdrModel& rDrawModel,
                                           std::u16string_view rAnchorNode, bool bAsChild)
{
    OUString aRetval;

    if (hasDiagramData())
    {
        const bool bUndo(rDrawModel.IsUndoEnabled());
        svx::diagram::DiagramDataStatePtr aStartState;

        if (bUndo)
        {
            // rescue all start state Diagram-defining data
            aStartState = extractDiagramDataState();
        }

        const OUString aTargetNode(readAnchorNode(*mpDiagramPtr->getData(), rAnchorNode));

        const svx::diagram::AddedDiagramNode aResult(
            mpDiagramPtr->getData()->addDiagramNode(aTargetNode, bAsChild));
        aRetval = aResult.msNewNodeId;

        if (!aRetval.isEmpty())
        {
            // reset Dom properties at DiagramData
            mpDiagramPtr->resetOOXDomValues(aResult.maChangedParts);

            // reset temporary buffered ModelData association lists & rebuild them
            // and the Diagram DataModel
            mpDiagramPtr->getData()->buildDiagramDataModel(true);

            // also reset temporary buffered layout data - that might
            // still refer to changed oox::Shape data
            mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();

            // aRetval represents the data node, the XShapes carry the ModelId of the presentation
            // Point that draws it
            msNewNodeId = readShapeIdOfNode(*mpDiagramPtr->getData(), aRetval);
            msNewNodeText = OUString(rText);

            // The Diagram defines which node handed over what the new node is represents with, and the
            // shape that represents
            msNewNodeTemplateId
                = readShapeIdOfNode(*mpDiagramPtr->getData(), aResult.msTemplateNodeId);

            if (bUndo)
            {
                // create undo action. That will internally secure the
                // current Diagram-defining data as end state
                SdrObject* pRootShape(SdrObject::getSdrObjectFromXShape(accessRootShape()));
                assert(nullptr != pRootShape && "Missing RootShape in DiagramHelper_oox (!)");
                rDrawModel.AddUndo(rDrawModel.GetSdrUndoFactory().CreateUndoDiagramModelData(
                    *pRootShape, aStartState));
            }
        }
    }

    return aRetval;
}

bool DiagramHelper_oox::removeDiagramNode(std::u16string_view rNodeId, SdrModel& rDrawModel)
{
    bool bRetval(false);

    if (hasDiagramData())
    {
        const bool bUndo(rDrawModel.IsUndoEnabled());
        svx::diagram::DiagramDataStatePtr aStartState;

        if (bUndo)
        {
            // rescue all start state Diagram-defining data
            aStartState = extractDiagramDataState();
        }

        DomMapFlags aResult(mpDiagramPtr->getData()->removeDiagramNode(rNodeId));

        if (!aResult.empty())
        {
            bRetval = true;

            // reset Dom properties at DiagramData
            mpDiagramPtr->resetOOXDomValues(std::move(aResult));

            // reset temporary buffered ModelData association lists & rebuild them
            // and the Diagram DataModel
            mpDiagramPtr->getData()->buildDiagramDataModel(true);

            // also reset temporary buffered layout data - that might
            // still refer to changed oox::Shape data
            mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();

            if (bUndo)
            {
                // create undo action. That will internally secure the
                // current Diagram-defining data as end state
                SdrObject* pRootShape(SdrObject::getSdrObjectFromXShape(accessRootShape()));
                assert(nullptr != pRootShape && "Missing RootShape in DiagramHelper_oox (!)");
                rDrawModel.AddUndo(rDrawModel.GetSdrUndoFactory().CreateUndoDiagramModelData(
                    *pRootShape, aStartState));
            }
        }
    }

    return bRetval;
}

void DiagramHelper_oox::ItemSetInformationChange(std::span<const SfxPoolItem* const> aChangedItems)
{
    bool bFillChanged(false);
    bool bLineChanged(false);
    bool bTextChanged(false);

    for (const SfxPoolItem* pItem : aChangedItems)
    {
        bFillChanged |= (pItem->Which() >= XATTR_FILL_FIRST && pItem->Which() <= XATTR_FILL_LAST);
        bLineChanged |= (pItem->Which() >= XATTR_LINE_FIRST && pItem->Which() <= XATTR_LINE_LAST);
        bTextChanged |= (pItem->Which() >= XATTR_TEXT_FIRST && pItem->Which() <= XATTR_TEXT_LAST);
    }

    if (bFillChanged || bLineChanged || bTextChanged)
    {
        // attributes of one of the contained shapes have
        // changed. Maybe other actions are needed in the
        // future, but for now do general reset like
        // TextInformationChange() does
        TextInformationChange();
    }
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
                                   cpo::uno::Any(cpo::uno::Sequence<beans::PropertyValue>()));
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

void DiagramHelper_oox::applyTextFitToSizeToDiagramNodes(
    const uno::Reference<drawing::XShape>& rTarget, bool bKeepFontScale)
{
    SdrObjGroup* pGroup(dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(rTarget)));

    if (nullptr == pGroup || !hasDiagramData())
        return;

    SdrObjListIter aIter(*pGroup, SdrIterMode::DeepNoGroups);

    while (aIter.IsMore())
    {
        SdrObject* pCandidate(aIter.Next());

        if (nullptr == pCandidate || nullptr == pCandidate->GetOutlinerParaObject())
            continue;

        const OUString& rModelID(pCandidate->getDiagramDataModelID());

        if (rModelID.isEmpty())
            continue;

        const rtl::Reference<svx::diagram::Point> xDataNode(
            mpDiagramPtr->getData()->getDataNodeForModelID(rModelID));

        // A node whose text the file marks as one of its own keeps what it came with
        if (!xDataNode.is() || xDataNode->mbCustomText)
            continue;

        // set AutoGrowHeight and TextFitToSize, need to rescue
        // some stuff when doing this
        uno::Reference<beans::XPropertySet> xShapeProperties(pCandidate->getUnoShape(),
                                                             uno::UNO_QUERY);
        double fFontScale(0.0);
        double fSpacingScale(0.0);

        if (bKeepFontScale && xShapeProperties.is())
        {
            xShapeProperties->getPropertyValue(u"TextFitToSizeFontScale"_ustr) >>= fFontScale;
            xShapeProperties->getPropertyValue(u"TextFitToSizeSpacingScale"_ustr) >>= fSpacingScale;
        }

        pCandidate->SetMergedItem(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false));
        pCandidate->SetMergedItem(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_AUTOFIT));

        if (fFontScale > 0.0 && fSpacingScale > 0.0)
        {
            xShapeProperties->setPropertyValue(u"TextFitToSizeFontScale"_ustr,
                                               cpo::uno::Any(fFontScale));
            xShapeProperties->setPropertyValue(u"TextFitToSizeSpacingScale"_ustr,
                                               cpo::uno::Any(fSpacingScale));
        }
    }
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
    connectToSdrObjGroup(rTarget, nullptr);
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

void DiagramHelper_oox::setOOXDomValue(DomMapFlag aDomMapFlag, const cpo::uno::Any& rValue)
{
    if (mpDiagramPtr)
        mpDiagramPtr->setOOXDomValue(aDomMapFlag, rValue);
}

cpo::uno::Any DiagramHelper_oox::getOOXDomValue(DomMapFlag aDomMapFlag) const
{
    if (mpDiagramPtr)
        return mpDiagramPtr->getOOXDomValue(aDomMapFlag);

    return cpo::uno::Any();
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

void DiagramHelper_oox::writeDiagramReducedOOXData(
    uno::Reference<io::XOutputStream>& xOutputStream) const
{
    if (!mpDiagramPtr)
        return;

    mpDiagramPtr->writeDiagramReducedOOXData(xOutputStream);
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

bool DiagramHelper_oox::isTextNodeModelID(const OUString& rModelID) const
{
    if (!mpDiagramPtr || rModelID.isEmpty())
        return false;

    for (const rtl::Reference<svx::diagram::Point>& rCandidate :
             mpDiagramPtr->getData()->getPoints())
    {
        if (rCandidate->msModelId == rModelID
            && rCandidate->getPresentation().msPresentationLayoutName == u"textNode"_ustr)
            return true;
    }

    return false;
}

OUString DiagramHelper_oox::getDiagramModelData(svx::diagram::DomMapFlag aId) const
{
    if (!checkMinimalDataDoms())
        // check if we have needed DomTrees (OOXLayout, OOXStyle and OOXColor)
        return EMPTY_OUSTRING;

    if (svx::diagram::DomMapFlag::OOXData == aId)
    {
        // prepare target XOutputStream
        SvMemoryStream aStream(1024, 1024);
        uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));

        // write reduced data.xml
        writeDiagramReducedOOXData(xOutputStream);

        // this call is *important*, without it xDocBuilder->parse below fails and some strange
        // and wrong assertion gets thrown in ~FastSerializerHelper that  shall get called
        xOutputStream->closeOutput();

#ifdef DBG_UTIL
        const OUString env(o3tl::getEnvironment(u"DIAGRAM_DUMP_PATH"_ustr));
        if (!env.isEmpty())
        {
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            OUString url;
            ::osl::FileBase::getFileURLFromSystemPath(env, url);
            SvFileStream aOutStream(url + "data_Reduced.xml",
                                    StreamMode::WRITE | StreamMode::TRUNC);
            uno::Reference<io::XStream> xOutStream(new utl::OStreamWrapper(aOutStream));
            uno::Reference<io::XStream> xInStream(xOutputStream, uno::UNO_QUERY);
            comphelper::OStorageHelper::CopyInputToOutput(xInStream->getInputStream(),
                                                          xOutStream->getOutputStream());
        }
#endif

        // create & return as string
        return OUString(static_cast<const char*>(aStream.GetData()), aStream.TellEnd(),
                        RTL_TEXTENCODING_UTF8);
    }

    // get DomTree
    uno::Reference<xml::dom::XDocument> aDomTree;
    getOOXDomValue(aId) >>= aDomTree;

    if (aDomTree)
    {
        // serialize existing DomTree to a MemoryStream
        SvMemoryStream aStream(1024, 1024);
        rtl::Reference<utl::OStreamWrapper> pStreamWrapper = new utl::OStreamWrapper(aStream);
        uno::Reference<xml::sax::XSAXSerializable> serializer;
        uno::Reference<xml::sax::XWriter> writer
            = xml::sax::Writer::create(comphelper::getProcessComponentContext());
        serializer.set(aDomTree, uno::UNO_QUERY);
        writer->setOutputStream(pStreamWrapper->getOutputStream());
        serializer->serialize(
            uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
            cpo::uno::Sequence<beans::StringPair>());

        // create & return as string
        return OUString(static_cast<const char*>(aStream.GetData()), aStream.TellEnd(),
                        RTL_TEXTENCODING_UTF8);
    }

    return EMPTY_OUSTRING;
}

DiagramHelperFactory_oox::DiagramHelperFactory_oox()
    : DiagramHelperFactory_svx()
{
    // make clear that this is supposed to happen only once
    assert(nullptr == pSingleGlobalDiagramHelperFactory_svx
           && "DiagramHelperFactory initialized multiple times (!)");

    // directly assign created instance to global pointer in svx for access from there
    if (nullptr == pSingleGlobalDiagramHelperFactory_svx)
        pSingleGlobalDiagramHelperFactory_svx = this;
}

DiagramHelperFactory_oox::~DiagramHelperFactory_oox()
{
    if (this == pSingleGlobalDiagramHelperFactory_svx)
        pSingleGlobalDiagramHelperFactory_svx = nullptr;
}

std::shared_ptr<svx::diagram::DiagramHelper_svx> DiagramHelperFactory_oox::createDiagramHelper_svx(
    std::u16string_view rLayout, std::u16string_view rData, std::u16string_view rColors,
    std::u16string_view rQuickstyle) const
{
    // from here we can instantiate DiagramHelper_oox and return
    return std::make_shared<DiagramHelper_oox>(rLayout, rData, rColors, rQuickstyle);
}

// this is the global needed anchor in oox module for an instantiation of
// DiagramHelperFactory_oox and thus for DiagramHelperFactory_svx. This
// is needed since in svx where a Diagram has to be imported an instance
// of DiagramHelper_oox needs to be instantiated, but svx cannot access oox
static DiagramHelperFactory_oox aSingleGlobalDiagramHelperFactory_oox;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
