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

#include "layoutatomvisitors.hxx"

#include <drawingml/customshapeproperties.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;

namespace oox::drawingml
{
void ShapeCreationVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeCreationVisitor::visit(RuleAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeCreationVisitor::visit(AlgAtom& rAtom)
{
    if (meLookFor == ALGORITHM)
    {
        mpParentShape->setAspectRatio(rAtom.getAspectRatio());
        mpParentShape->setVerticalShapesCount(rAtom.getVerticalShapesCount(mpParentShape));
    }
}

void ShapeCreationVisitor::visit(LayoutNode& rAtom)
{
    if (meLookFor != LAYOUT_NODE)
        return;

    // stop processing if it's not a child of previous LayoutNode

    const DiagramData_oox::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end())
        return;

    const rtl::Reference<svx::diagram::Point> rNewNode(
        presentationForPass(aDataNode->second));
    if (!mxCurrentNode.is() || !rNewNode.is())
        return;

    ShapePtr xCurrParent(mpParentShape);

    if (rAtom.getExistingShape())
    {
        // reuse existing shape
        ShapePtr pShape = rAtom.getExistingShape();
        if (rAtom.setupShape(mrDgm, pShape, rNewNode, mnCurrIdx))
        {
            pShape->setInternalName(rAtom.getName());
            rAtom.addNodeShape(pShape);
            mrDgm.getLayout()->getPresPointShapeMap()[rNewNode] = std::move(pShape);
        }
    }
    else
    {
        ShapeTemplateVisitor aTemplateVisitor(mrDgm, rNewNode);
        aTemplateVisitor.defaultVisit(rAtom);
        ShapePtr pShape = aTemplateVisitor.getShapeCopy();

        // A layout node need not state a shape. It still takes a place and a size and it still
        // holds what hangs below it, so it gets a group with nothing drawn in it. Without one
        // its children would hang off its own parent and a hierarchy would come out flat.
        if (!pShape)
            pShape = std::make_shared<Shape>(u"com.sun.star.drawing.GroupShape"_ustr);

        if (pShape)
        {
            SAL_INFO("oox.drawingml",
                     "processing shape type "
                         << (pShape->getCustomShapeProperties()->getShapePresetType()));

            if (rAtom.setupShape(mrDgm, pShape, rNewNode, mnCurrIdx))
            {
                pShape->setInternalName(rAtom.getName());
                xCurrParent->addChild(pShape);
                xCurrParent = pShape;
                rAtom.addNodeShape(pShape);
                mrDgm.getLayout()->getPresPointShapeMap()[rNewNode] = std::move(pShape);
            }
        }
        else
        {
            SAL_WARN("oox.drawingml",
                     "ShapeCreationVisitor::visit: no shape set while processing layoutnode named "
                         << rAtom.getName());
        }
    }

    const rtl::Reference<svx::diagram::Point> xPreviousNode(mxCurrentNode);
    mxCurrentNode = rNewNode;

    // From here the current Point is the one of this layout node, so a condition below asks
    // about that and wants no stand-in from the loop above.
    const OUString aHeldPassNodeId(msPassNodeId);
    msPassNodeId.clear();

    // set new parent for children
    ShapePtr xPreviousParent(mpParentShape);
    mpParentShape = std::move(xCurrParent);

    // process children
    meLookFor = LAYOUT_NODE;
    defaultVisit(rAtom);

    meLookFor = ALGORITHM;
    defaultVisit(rAtom);
    meLookFor = LAYOUT_NODE;

    // restore parent
    mpParentShape = std::move(xPreviousParent);
    msPassNodeId = aHeldPassNodeId;
    mxCurrentNode = xPreviousNode;
}

void ShapeCreationVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::visit(RuleAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::visit(AlgAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::visit(ForEachAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::visit(LayoutNode& /*rAtom*/)
{
    // stop processing - only traverse Condition/Choose atoms
}

void ShapeTemplateVisitor::visit(ShapeAtom& rAtom)
{
    if (mpShape)
    {
        SAL_WARN("oox.drawingml", "multiple shapes encountered inside LayoutNode");
        return;
    }

    const ShapePtr& pCurrShape(rAtom.getShapeTemplate());

    // TODO(F3): cloned shape shares all properties by reference,
    // don't change them!
    mpShape = std::make_shared<Shape>(pCurrShape);
    // Fill properties have to be changed as sometimes only the presentation node contains the blip
    // fill, unshare those.
    mpShape->cloneFillProperties();
    // The colour a Diagram gives a shape as a line goes on the line properties, and each shape
    // of a run takes a colour of its own, so unshare those as well.
    mpShape->cloneLineProperties();

    // add/set ModelID from current node to allow later association
    if (mxCurrentNode)
        mpShape->setDiagramDataModelID(mxCurrentNode->msModelId);
}

void ShapeLayoutingVisitor::visit(ConstraintAtom& rAtom)
{
    if (meLookFor == CONSTRAINT)
        rAtom.parseConstraint(maConstraints, /*bRequireForName=*/true);
}

void ShapeLayoutingVisitor::visit(RuleAtom& rAtom)
{
    if (meLookFor == RULE)
        rAtom.parseRule(maRules);
}

void ShapeLayoutingVisitor::visit(AlgAtom& rAtom)
{
    if (meLookFor == ALGORITHM)
    {
        const PresPointShapeMap aMap = mrDgm.getLayout()->getPresPointShapeMap();
        auto pShape = aMap.find(mxCurrentNode);
        if (pShape != aMap.end())
            rAtom.layoutShape(mrDgm, pShape->second, maConstraints, maRules);
    }
}

void ShapeLayoutingVisitor::visit(LayoutNode& rAtom)
{
    if (meLookFor != LAYOUT_NODE)
        return;

    // stop processing if it's not a child of previous LayoutNode

    const DiagramData_oox::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end())
        return;

    const rtl::Reference<svx::diagram::Point> rNewNode(
        presentationForPass(aDataNode->second));
    if (!mxCurrentNode.is() || !rNewNode.is())
        return;

    size_t nParentConstraintsNumber = maConstraints.size();

    const rtl::Reference<svx::diagram::Point> xPreviousNode(mxCurrentNode);
    mxCurrentNode = rNewNode;

    // From here the current Point is the one of this layout node, so a condition below asks
    // about that and wants no stand-in from the loop above.
    const OUString aHeldPassNodeId(msPassNodeId);
    msPassNodeId.clear();

    // process alg atoms first, nested layout nodes afterwards
    meLookFor = CONSTRAINT;
    defaultVisit(rAtom);
    meLookFor = RULE;
    defaultVisit(rAtom);
    meLookFor = ALGORITHM;
    defaultVisit(rAtom);
    meLookFor = LAYOUT_NODE;
    defaultVisit(rAtom);

    msPassNodeId = aHeldPassNodeId;
    mxCurrentNode = xPreviousNode;

    // delete added constraints, keep parent constraints
    maConstraints.erase(maConstraints.begin() + nParentConstraintsNumber, maConstraints.end());
}

void ShapeLayoutingVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
