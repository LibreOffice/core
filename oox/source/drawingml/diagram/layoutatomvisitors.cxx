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

    const DiagramData::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end()
        || mnCurrIdx >= static_cast<sal_Int32>(aDataNode->second.size()))
        return;

    const svx::diagram::Point* pNewNode = aDataNode->second.at(mnCurrIdx);
    if (!mpCurrentNode || !pNewNode)
        return;

    bool bIsChild = false;
    for (const auto& aConnection : mrDgm.getData()->getConnections())
        if (aConnection.msSourceId == mpCurrentNode->msModelId
            && aConnection.msDestId == pNewNode->msModelId)
            bIsChild = true;

    if (!bIsChild)
        return;

    ShapePtr pCurrParent(mpParentShape);

    if (rAtom.getExistingShape())
    {
        // reuse existing shape
        ShapePtr pShape = rAtom.getExistingShape();
        if (rAtom.setupShape(pShape, pNewNode, mnCurrIdx))
        {
            pShape->setInternalName(rAtom.getName());
            rAtom.addNodeShape(pShape);
            mrDgm.getLayout()->getPresPointShapeMap()[pNewNode] = pShape;
        }
    }
    else
    {
        ShapeTemplateVisitor aTemplateVisitor(mrDgm, pNewNode);
        aTemplateVisitor.defaultVisit(rAtom);
        ShapePtr pShape = aTemplateVisitor.getShapeCopy();

        if (pShape)
        {
            SAL_INFO("oox.drawingml",
                     "processing shape type "
                         << (pShape->getCustomShapeProperties()->getShapePresetType()));

            if (rAtom.setupShape(pShape, pNewNode, mnCurrIdx))
            {
                pShape->setInternalName(rAtom.getName());
                pCurrParent->addChild(pShape);
                pCurrParent = pShape;
                rAtom.addNodeShape(pShape);
                mrDgm.getLayout()->getPresPointShapeMap()[pNewNode] = pShape;
            }
        }
        else
        {
            SAL_WARN("oox.drawingml",
                     "ShapeCreationVisitor::visit: no shape set while processing layoutnode named "
                         << rAtom.getName());
        }
    }

    const svx::diagram::Point* pPreviousNode = mpCurrentNode;
    mpCurrentNode = pNewNode;

    // set new parent for children
    ShapePtr xPreviousParent(mpParentShape);
    mpParentShape = pCurrParent;

    // process children
    meLookFor = LAYOUT_NODE;
    defaultVisit(rAtom);

    meLookFor = ALGORITHM;
    defaultVisit(rAtom);
    meLookFor = LAYOUT_NODE;

    // restore parent
    mpParentShape = std::move(xPreviousParent);
    mpCurrentNode = pPreviousNode;
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

    // add/set ModelID from current node to allow later association
    if (mpCurrentNode)
        mpShape->setDiagramDataModelID(mpCurrentNode->msModelId);
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
        const PresPointShapeMap aMap
            = rAtom.getLayoutNode().getDiagram().getLayout()->getPresPointShapeMap();
        auto pShape = aMap.find(mpCurrentNode);
        if (pShape != aMap.end())
            rAtom.layoutShape(pShape->second, maConstraints, maRules);
    }
}

void ShapeLayoutingVisitor::visit(LayoutNode& rAtom)
{
    if (meLookFor != LAYOUT_NODE)
        return;

    // stop processing if it's not a child of previous LayoutNode

    const DiagramData::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end()
        || mnCurrIdx >= static_cast<sal_Int32>(aDataNode->second.size()))
        return;

    const svx::diagram::Point* pNewNode = aDataNode->second.at(mnCurrIdx);
    if (!mpCurrentNode || !pNewNode)
        return;

    bool bIsChild = false;
    for (const auto& aConnection : mrDgm.getData()->getConnections())
        if (aConnection.msSourceId == mpCurrentNode->msModelId
            && aConnection.msDestId == pNewNode->msModelId)
            bIsChild = true;

    if (!bIsChild)
        return;

    size_t nParentConstraintsNumber = maConstraints.size();

    const svx::diagram::Point* pPreviousNode = mpCurrentNode;
    mpCurrentNode = pNewNode;

    // process alg atoms first, nested layout nodes afterwards
    meLookFor = CONSTRAINT;
    defaultVisit(rAtom);
    meLookFor = RULE;
    defaultVisit(rAtom);
    meLookFor = ALGORITHM;
    defaultVisit(rAtom);
    meLookFor = LAYOUT_NODE;
    defaultVisit(rAtom);

    mpCurrentNode = pPreviousNode;

    // delete added constraints, keep parent constraints
    maConstraints.erase(maConstraints.begin() + nParentConstraintsNumber, maConstraints.end());
}

void ShapeLayoutingVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
