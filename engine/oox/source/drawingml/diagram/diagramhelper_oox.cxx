/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
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
#include <editeng/editobj.hxx>
#include <svx/svddef.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <svx/sdooitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <comphelper/processfactory.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
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
using namespace ::cpo;
using namespace svx::diagram;

namespace
{
// The box the shapes cover, against the origin of the Diagram, in EMU. A shape stands at a place
// relative to the shape above it, and a turned shape covers the box round it as it is turned.
// A group of its own draws nothing, what it covers is what the shapes in it cover.
void gatherCover(oox::drawingml::Shape& rShape, const basegfx::B2DPoint& rOrigin,
                 basegfx::B2DRange& rCover)
{
    const basegfx::B2DPoint aAt(rOrigin.getX() + rShape.getPosition().X,
                                rOrigin.getY() + rShape.getPosition().Y);
    const double fWidth(rShape.getSize().Width);
    const double fHeight(rShape.getSize().Height);
    if (rShape.getChildren().empty() && fWidth > 0.0 && fHeight > 0.0)
    {
        const double fAngle(basegfx::deg2rad(rShape.getRotation() / 60000.0));
        const double fAcross(std::abs(cos(fAngle)));
        const double fAlong(std::abs(sin(fAngle)));
        const double fHalfWidth((fAcross * fWidth + fAlong * fHeight) / 2.0);
        const double fHalfHeight((fAlong * fWidth + fAcross * fHeight) / 2.0);
        const basegfx::B2DPoint aMiddle(aAt.getX() + fWidth / 2.0, aAt.getY() + fHeight / 2.0);
        rCover.expand(basegfx::B2DRange(aMiddle.getX() - fHalfWidth, aMiddle.getY() - fHalfHeight,
                                        aMiddle.getX() + fHalfWidth,
                                        aMiddle.getY() + fHalfHeight));
    }
    for (const auto& rChild : rShape.getChildren())
        gatherCover(*rChild, aAt, rCover);
}
}

namespace oox::drawingml
{
bool DiagramHelper_oox::hasDiagramData() const { return mpDiagramPtr && mpDiagramPtr->getData(); }

DiagramHelper_oox::DiagramHelper_oox(std::shared_ptr<SmartArtDiagram> xDiagramPtr,
                                     std::shared_ptr<::oox::drawingml::Theme> xTheme)
    : mpDiagramPtr(std::move(xDiagramPtr))
    , mpDiagramThemePtr(std::move(xTheme))
    , maFrameInGroup(0.0, 0.0, 1.0, 1.0)
    , msNewNodeId()
    , msNewNodeText()
    , msNewNodeTemplateId()
{
}

DiagramHelper_oox::DiagramHelper_oox(DiagramHelper_oox const& rSource)
    : DiagramHelper_svx(rSource)
    , mpDiagramPtr(rSource.mpDiagramPtr ? new SmartArtDiagram(*rSource.mpDiagramPtr) : nullptr)
    , mpDiagramThemePtr(rSource.mpDiagramThemePtr)
    , maFrameInGroup(rSource.maFrameInGroup)
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
    , maFrameInGroup(0.0, 0.0, 1.0, 1.0)
    , msNewNodeId()
    , msNewNodeText()
    , msNewNodeTemplateId()
{
}

DiagramHelper_oox::~DiagramHelper_oox() {}

namespace
{
// Hands the name, the title, the description and the decorative mark of rOldShape to rNewShape,
// the new version of the same shape. They are laid on with the undo recording off: the
// re-creation as a whole is the step that is undone, and undoing it runs the re-creation again.
void moveNamingFromOldToNewShape(SdrObject& rOldShape, SdrObject& rNewShape)
{
    SdrModel& rModel(rNewShape.getSdrModelFromSdrObject());
    const bool bUndoWasEnabled(rModel.IsUndoEnabled());
    rModel.EnableUndo(false);

    rNewShape.SetName(rOldShape.GetName());
    rNewShape.SetTitle(rOldShape.GetTitle());
    rNewShape.SetDescription(rOldShape.GetDescription());
    rNewShape.SetDecorative(rOldShape.IsDecorative());

    rModel.EnableUndo(bUndoWasEnabled);
}
}

// The size of the first run of the text of xShape, in points, 0 where it has no text.
static float firstRunHeightOf(const uno::Reference<drawing::XShape>& xShape)
{
    float fHeight(0.0);
    uno::Reference<container::XEnumerationAccess> xParagraphs(xShape, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnumeration(
        xParagraphs.is() ? xParagraphs->createEnumeration() : nullptr);
    if (xEnumeration.is() && xEnumeration->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xParagraph(xEnumeration->nextElement(),
                                                                 uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions(
            xParagraph.is() ? xParagraph->createEnumeration() : nullptr);
        if (xPortions.is() && xPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
            if (xPortion.is())
                xPortion->getPropertyValue(u"CharHeight"_ustr) >>= fHeight;
        }
    }
    return fHeight;
}

void DiagramHelper_oox::moveDiagramModelDataFromOldToNewXShape(
    const uno::Reference<drawing::XShape>& xOldShape,
    const uno::Reference<drawing::XShape>& xNewShape)
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

    // copy Text/OutlinerParaObject. The old text keeps what the user did to it, and its sizes:
    // the fit to the new box is the shape's own autofit, and the font sync over the nodes of
    // one name. Where the layout fitted a font for the node's group, the text is scaled so
    // that its first run has that font. A node with text of its own keeps its sizes.
    OutlinerParaObject* pParaObject(pOldShape->GetOutlinerParaObject());

    if (nullptr != pParaObject)
    {
        std::optional<double> oFitted;
        if (mpDiagramPtr && mpDiagramPtr->getData())
        {
            const rtl::Reference<svx::diagram::Point> xPoint(
                mpDiagramPtr->getData()->getPointByModelID(pNewShape->getDiagramDataModelID()));
            const rtl::Reference<svx::diagram::Point> xDataNode(
                mpDiagramPtr->getData()->getDataNodeForModelID(
                    pNewShape->getDiagramDataModelID()));
            if (xPoint.is() && !(xDataNode.is() && xDataNode->mbCustomText))
            {
                const auto aFitted = mpDiagramPtr->getFittedFontSizes().find(
                    xPoint->getPresentation().msPresentationLayoutName);
                if (aFitted != mpDiagramPtr->getFittedFontSizes().end())
                    oFitted = aFitted->second;
            }
        }
        const float fOldHeight(firstRunHeightOf(xOldShape));
        pNewShape->SetOutlinerParaObject(*pParaObject);
        if (oFitted && fOldHeight > 0.0)
        {
            const double fScale(*oFitted / fOldHeight);
            pNewShape->scaleText(fScale, fScale);
        }
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
    // correctly. The room the layout fills is the frame of the Diagram, which is kept as a
    // part of the group: the group covers what the shapes drew the last time, and a shape may
    // stick out of the frame, an arc round a list of nodes for one. Laying out into the whole
    // group would let such a shape grow the room with every layout.
    const basegfx::B2DHomMatrix aFrameTransformation(
        aTransformation
        * basegfx::utils::createScaleTranslateB2DHomMatrix(
              maFrameInGroup.getWidth(), maFrameInGroup.getHeight(), maFrameInGroup.getMinX(),
              maFrameInGroup.getMinY()));
    const basegfx::B2DRange aRootRange(
        aFrameTransformation * basegfx::B2DPoint(0, 0), // top-left
        aFrameTransformation * basegfx::B2DPoint(1, 1)); // bottom-right

    // also need to convert to Emu used by mso and thus in oox::Shape stuff
    // the frame is kept exact, so the room is not rounded to the hundredths of a millimetre
    pShapePtr->setSize(awt::Size(basegfx::fround(aRootRange.getWidth() * 360.0),
                                 basegfx::fround(aRootRange.getHeight() * 360.0)));

    // remember exsiting DrawingLayerModelData. Do this before createShapeHierarchyFromModel
    // below, that will create a new BackgroundShapeModelID and the BGShape would
    // be missing.
    // Also important is to do this as XShapes, the content of the Group will delete the
    // SdrObjects in the process of re-creation, but the XShapes will survive
    std::vector<uno::Reference<drawing::XShape>> xOldXShapes;

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

    // the layout measures the text, and the old shapes are where it is now
    mpDiagramPtr->getData()->fillTextBodiesFromXShapes();

    // also reset temporary buffered layout data - that might
    // still refer to changed oox::Shape data
    mpDiagramPtr->getLayout()->getPresPointShapeMap().clear();

    // the text font of the theme, the layout measures the text with it
    if (mpDiagramThemePtr && mpDiagramThemePtr->getTheme())
        mpDiagramPtr->setTextFont(
            mpDiagramThemePtr->getTheme()->getFontScheme().getMinorLatin().maTypeface);

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
    uno::Reference<cpo::uno::XInterface> const& rUnoModel(rModel.getUnoModel());
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

    // The shapes cover the room, or more where one sticks out of it, or less where the layout
    // leaves a part of the room empty. The group takes what they cover, put where that lies
    // against the frame, and the frame is remembered against the new group for the next time.
    basegfx::B2DHomMatrix aCoverTransformation(aTransformation);
    basegfx::B2DRange aCover;
    gatherCover(*pShapePtr, basegfx::B2DPoint(0.0, 0.0), aCover);
    // a shape may draw more than its box, a curved connector its path, so we cover what
    // the made shapes draw too; the room's origin is at the group's
    if (const tools::Rectangle aDrawn(pTarget->GetSnapRect()); !aDrawn.IsEmpty())
        aCover.expand(basegfx::B2DRange(oox::drawingml::convertHmmToEmu(aDrawn.Left()),
                                        oox::drawingml::convertHmmToEmu(aDrawn.Top()),
                                        oox::drawingml::convertHmmToEmu(aDrawn.Right()),
                                        oox::drawingml::convertHmmToEmu(aDrawn.Bottom())));
    const awt::Size& rRoom(pShapePtr->getSize());
    if (!aCover.isEmpty() && aCover.getWidth() > 0.0 && aCover.getHeight() > 0.0
        && rRoom.Width > 0 && rRoom.Height > 0)
    {
        aCoverTransformation
            = aFrameTransformation
              * basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aCover.getWidth() / rRoom.Width, aCover.getHeight() / rRoom.Height,
                    aCover.getMinX() / rRoom.Width, aCover.getMinY() / rRoom.Height);
        maFrameInGroup = basegfx::B2DRange(
            -aCover.getMinX() / aCover.getWidth(), -aCover.getMinY() / aCover.getHeight(),
            (rRoom.Width - aCover.getMinX()) / aCover.getWidth(),
            (rRoom.Height - aCover.getMinY()) / aCover.getHeight());
    }
    pTarget->TRSetBaseGeometry(aCoverTransformation, aPolyPolygon);

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

                        // A name belongs to one shape alone, so it goes from the old version
                        // to the new one, and a new node made after a template takes none.
                        moveNamingFromOldToNewShape(*pOldShape, *pNewShape);
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
                moveDiagramModelDataFromOldToNewXShape(xNewShapeTemplate, xNewShape);

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

namespace
{
// the paragraphs of the text of rObject, a string each
std::vector<OUString> paragraphsOf(const SdrObject& rObject)
{
    std::vector<OUString> aOut;
    if (const OutlinerParaObject* pText = rObject.GetOutlinerParaObject())
        for (sal_Int32 n = 0; n < pText->GetTextObject().GetParagraphCount(); ++n)
            aOut.push_back(pText->GetTextObject().GetText(n));
    return aOut;
}
}

void DiagramHelper_oox::textEditEnding(const SdrObject& rObject)
{
    mpTextEditStartState.reset();
    msTextEditModelId.clear();
    maTextEditParagraphs.clear();

    if (!hasDiagramData())
        return;

    msTextEditModelId = rObject.getDiagramDataModelID();
    maTextEditParagraphs = paragraphsOf(rObject);
    mpTextEditStartState = extractDiagramDataState();
}

void DiagramHelper_oox::textEditEnded(const SdrObject& rObject, SdrModel& rDrawModel, bool bUndo)
{
    DiagramDataStatePtr pStartState(std::move(mpTextEditStartState));
    const OUString aModelId(std::move(msTextEditModelId));
    const std::vector<OUString> aOld(std::move(maTextEditParagraphs));
    mpTextEditStartState.reset();
    msTextEditModelId.clear();
    maTextEditParagraphs.clear();

    if (!hasDiagramData() || aModelId.isEmpty() || aModelId != rObject.getDiagramDataModelID())
        return;

    svx::diagram::DiagramData_svx& rData(*mpDiagramPtr->getData());
    const rtl::Reference<svx::diagram::Point> xShown(rData.getPointByModelID(aModelId));
    if (!xShown.is() || xShown->maShows.empty())
        return;

    // the nodes the shape shows, in the order of their paragraphs
    std::vector<const svx::diagram::Connection*> aShows(xShown->maShows.begin(),
                                                        xShown->maShows.end());
    std::stable_sort(aShows.begin(), aShows.end(),
                     [](const svx::diagram::Connection* pA, const svx::diagram::Connection* pB) {
                         return pA->mnDestOrder < pB->mnDestOrder;
                     });
    std::vector<rtl::Reference<svx::diagram::Point>> aNodes;
    for (const svx::diagram::Connection* pShow : aShows)
    {
        if (nullptr == pShow || nullptr == pShow->mpSourcePoint
            || svx::diagram::TypeConstant::XML_node != pShow->mpSourcePoint->mnXMLType)
            return;
        aNodes.emplace_back(pShow->mpSourcePoint);
    }

    // a shape of a node of its own keeps all its paragraphs in that node
    if (aNodes.size() == 1
        && xShown->getPresentation().msPresentationAssociationId == aNodes[0]->msModelId)
        return;

    // the first paragraph of each node before the edit
    std::vector<size_t> aFirst;
    size_t nCount(0);
    for (const rtl::Reference<svx::diagram::Point>& rNode : aNodes)
    {
        aFirst.push_back(nCount);
        nCount += std::max<sal_Int32>(1, rNode->mnParagraphCount);
    }
    if (nCount != aOld.size())
    {
        SAL_WARN("oox", "textEditEnded: the paragraphs of the shape do not match its nodes");
        return;
    }

    // the paragraphs that stayed as they were at the start and at the end
    const std::vector<OUString> aNew(paragraphsOf(rObject));
    size_t nFront(0);
    while (nFront < aOld.size() && nFront < aNew.size() && aOld[nFront] == aNew[nFront])
        nFront++;
    size_t nBack(0);
    while (nBack < aOld.size() - nFront && nBack < aNew.size() - nFront
           && aOld[aOld.size() - 1 - nBack] == aNew[aNew.size() - 1 - nBack])
        nBack++;
    if (nFront == aOld.size() && nFront == aNew.size())
        return;

    // the nodes the changed paragraphs belonged to; with none, as for a paragraph only added,
    // the node before it, or the first node for one added at the start
    size_t nFirstNode(aNodes.size());
    size_t nEndNode(0);
    for (size_t n = 0; n < aNodes.size(); ++n)
    {
        const size_t nEnd(aFirst[n] + std::max<sal_Int32>(1, aNodes[n]->mnParagraphCount));
        if (aFirst[n] < aOld.size() - nBack && nEnd > nFront)
        {
            nFirstNode = std::min(nFirstNode, n);
            nEndNode = n + 1;
        }
    }
    bool bOnlyAdded(false);
    if (nFirstNode == aNodes.size())
    {
        if (nFront == 0)
        {
            nFirstNode = 0;
            nEndNode = 1;
        }
        else
        {
            for (size_t n = 0; n < aNodes.size(); ++n)
                if (aFirst[n] < nFront)
                    nFirstNode = n;
            nEndNode = nFirstNode + 1;
            bOnlyAdded = true;
        }
    }

    // the new paragraphs that take the place of the changed ones; the nodes keep as many as they
    // had while there are enough, and every paragraph left over is a node of its own, as typing
    // Enter in the shape makes it
    const size_t nOldFrom(
        aFirst[nFirstNode]
        + (bOnlyAdded ? std::max<sal_Int32>(1, aNodes[nFirstNode]->mnParagraphCount) : 0));
    const size_t nOldTo(aFirst[nEndNode - 1]
                        + std::max<sal_Int32>(1, aNodes[nEndNode - 1]->mnParagraphCount));
    size_t nLeft(aNew.size() - (aOld.size() - nOldTo) - nOldFrom);
    std::vector<OUString> aRemove;
    OUString aLastKept;
    bool bCountChanged(false);
    if (bOnlyAdded)
        aLastKept = aNodes[nFirstNode]->msModelId;
    else
        for (size_t n = nFirstNode; n < nEndNode; ++n)
        {
            const sal_Int32 nTake(static_cast<sal_Int32>(
                std::min<size_t>(std::max<sal_Int32>(1, aNodes[n]->mnParagraphCount), nLeft)));
            nLeft -= nTake;
            if (nTake == 0)
                aRemove.push_back(aNodes[n]->msModelId);
            else
            {
                bCountChanged |= aNodes[n]->mnParagraphCount != nTake;
                aNodes[n]->mnParagraphCount = nTake;
                aLastKept = aNodes[n]->msModelId;
            }
        }
    if (aRemove.empty() && nLeft == 0 && !bCountChanged)
        return;

    DomMapFlags aChangedParts;
    for (const OUString& rRemove : aRemove)
    {
        DomMapFlags aParts(rData.removeDiagramNode(rRemove, false));
        if (aParts.empty())
            SAL_WARN("oox", "textEditEnded: a node whose paragraph was taken out stays");
        aChangedParts.insert(aChangedParts.end(), aParts.begin(), aParts.end());
    }
    // a node that is added goes behind the one before it, and a node with no node before it
    // goes behind the first one that stays
    if (aLastKept.isEmpty() && nEndNode < aNodes.size())
        aLastKept = aNodes[nEndNode]->msModelId;
    for (; nLeft > 0 && !aLastKept.isEmpty(); nLeft--)
    {
        const svx::diagram::AddedDiagramNode aAdded(rData.addDiagramNode(aLastKept, false));
        if (aAdded.msNewNodeId.isEmpty())
        {
            SAL_WARN("oox", "textEditEnded: no node could be added for a paragraph");
            break;
        }
        aChangedParts.insert(aChangedParts.end(), aAdded.maChangedParts.begin(),
                             aAdded.maChangedParts.end());
        aLastKept = aAdded.msNewNodeId;
    }

    if (!aChangedParts.empty())
        mpDiagramPtr->resetOOXDomValues(std::move(aChangedParts));

    // the shape keeps the text as typed, the new layout takes it over
    reLayout();

    if (bUndo && pStartState)
    {
        SdrObject* pRootShape(SdrObject::getSdrObjectFromXShape(accessRootShape()));
        if (nullptr != pRootShape)
            rDrawModel.AddUndo(rDrawModel.GetSdrUndoFactory().CreateUndoDiagramModelData(
                *pRootShape, pStartState));
    }
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

void DiagramHelper_oox::doAnchor(uno::Reference<drawing::XShape>& rTarget,
                                 const awt::Point& rFramePosition, const awt::Size& rFrameSize)
{
    if (!mpDiagramPtr || !rTarget)
    {
        return;
    }

    // sync FontHeights
    mpDiagramPtr->syncDiagramFontHeights();

    // The group covers what the shapes drew, and the frame is the room the layout filled. The
    // frame is kept in parts of the group, so it goes along when the group is moved or resized.
    // Where the two do not go together at all the frame is taken to be the group.
    maFrameInGroup = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
    SdrObject* pGroup(SdrObject::getSdrObjectFromXShape(rTarget));
    if (pGroup && rFrameSize.Width > 0 && rFrameSize.Height > 0)
    {
        basegfx::B2DHomMatrix aGroupTransformation;
        basegfx::B2DPolyPolygon aPolyPolygon;
        pGroup->TRGetBaseGeometry(aGroupTransformation, aPolyPolygon);
        if (aGroupTransformation.isInvertible())
        {
            aGroupTransformation.invert();
            // in hundredths of a millimetre, not rounded
            const basegfx::B2DRange aFrame(
                aGroupTransformation
                    * basegfx::B2DPoint(rFramePosition.X / 360.0, rFramePosition.Y / 360.0),
                aGroupTransformation
                    * basegfx::B2DPoint((rFramePosition.X + rFrameSize.Width) / 360.0,
                                        (rFramePosition.Y + rFrameSize.Height) / 360.0));
            const bool bFits(aFrame.getWidth() > 0.2 && aFrame.getWidth() < 5.0
                             && aFrame.getHeight() > 0.2 && aFrame.getHeight() < 5.0
                             && aFrame.overlaps(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
            if (bFits)
                maFrameInGroup = aFrame;
        }
    }

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
