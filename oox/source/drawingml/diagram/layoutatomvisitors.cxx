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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace oox { namespace drawingml {

void ShapeCreationVisitor::defaultVisit(LayoutAtom const & rAtom)
{
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShapeCreationVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeCreationVisitor::visit(AlgAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeCreationVisitor::visit(ForEachAtom& rAtom)
{
    const std::vector<LayoutAtomPtr>& rChildren=rAtom.getChildren();

    sal_Int32 nChildren=1;
    if( rAtom.iterator().mnPtType == XML_node )
    {
        // cound child data nodes - check all child Atoms for "name"
        // attribute that is contained in diagram's
        // getPointsPresNameMap()
        ShallowPresNameVisitor aVisitor(mrDgm);
        for (const auto& pAtom : rChildren)
            pAtom->accept(aVisitor);
        nChildren = aVisitor.getCount();
    }

    const sal_Int32 nCnt = std::min(
        nChildren,
        rAtom.iterator().mnCnt==-1 ? nChildren : rAtom.iterator().mnCnt);

    const sal_Int32 nOldIdx=mnCurrIdx;
    const sal_Int32 nStep=rAtom.iterator().mnStep;
    for( mnCurrIdx=0; mnCurrIdx<nCnt && nStep>0; mnCurrIdx+=nStep )
    {
        // TODO there is likely some conditions
        for (const auto& pAtom : rChildren)
            pAtom->accept(*this);
    }

    // and restore idx
    mnCurrIdx = nOldIdx;
}

void ShapeCreationVisitor::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeCreationVisitor::visit(ChooseAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeCreationVisitor::visit(LayoutNode& rAtom)
{
    // stop processing if it's not a child of previous LayoutNode

    const DiagramData::PointsNameMap::const_iterator aDataNode = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end() || mnCurrIdx >= static_cast<sal_Int32>(aDataNode->second.size()))
        return;

    const dgm::Point* pNewNode = aDataNode->second.at(mnCurrIdx);
    if (!mpCurrentNode || !pNewNode)
        return;

    bool bIsChild = false;
    for (const auto & aConnection : mrDgm.getData()->getConnections())
        if (aConnection.msSourceId == mpCurrentNode->msModelId && aConnection.msDestId == pNewNode->msModelId)
            bIsChild = true;

    if (!bIsChild)
        return;

    ShapePtr pCurrParent(mpParentShape);

    if (rAtom.getExistingShape())
    {
        // reuse existing shape
        ShapePtr pShape = rAtom.getExistingShape();
        if (rAtom.setupShape(pShape, pNewNode))
        {
            pShape->setInternalName(rAtom.getName());
            rAtom.addNodeShape(pShape);
        }
    }
    else
    {
        ShapeTemplateVisitor aTemplateVisitor;
        aTemplateVisitor.defaultVisit(rAtom);
        ShapePtr pShape = aTemplateVisitor.getShapeCopy();

        if (pShape)
        {
            SAL_INFO(
                "oox.drawingml",
                "processing shape type "
                    << (pShape->getCustomShapeProperties()
                        ->getShapePresetType()));

            if (rAtom.setupShape(pShape, pNewNode))
            {
                pShape->setInternalName(rAtom.getName());
                pCurrParent->addChild(pShape);
                pCurrParent = pShape;
                rAtom.addNodeShape(pShape);
            }
        }
        else
        {
            SAL_WARN("oox.drawingml", "ShapeCreationVisitor::visit: no shape set while processing layoutnode named " << rAtom.getName());
        }
    }

    const dgm::Point* pPreviousNode = mpCurrentNode;
    mpCurrentNode = pNewNode;

    // set new parent for children
    ShapePtr pPreviousParent(mpParentShape);
    mpParentShape=pCurrParent;

    // process children
    defaultVisit(rAtom);

    // restore parent
    mpParentShape=pPreviousParent;
    mpCurrentNode = pPreviousNode;

    // remove unneeded empty group shapes
    pCurrParent->getChildren().erase(
        std::remove_if(pCurrParent->getChildren().begin(), pCurrParent->getChildren().end(),
            [] (const ShapePtr & aChild) { return aChild->getServiceName() == "com.sun.star.drawing.GroupShape" && aChild->getChildren().empty(); }),
        pCurrParent->getChildren().end());
}

void ShapeCreationVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeTemplateVisitor::defaultVisit(LayoutAtom const & rAtom)
{
    // visit all children, one of them needs to be the layout algorithm
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShapeTemplateVisitor::visit(ConstraintAtom& /*rAtom*/)
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

void ShapeTemplateVisitor::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeTemplateVisitor::visit(ChooseAtom& rAtom)
{
    defaultVisit(rAtom);
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

    ShapePtr pCurrShape(rAtom.getShapeTemplate());

    // TODO(F3): cloned shape shares all properties by reference,
    // don't change them!
    mpShape.reset(new Shape(pCurrShape));
}

void ShapeLayoutingVisitor::defaultVisit(LayoutAtom const & rAtom)
{
    // visit all children, one of them needs to be the layout algorithm
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShapeLayoutingVisitor::visit(ConstraintAtom& rAtom)
{
    if (meLookFor == CONSTRAINT)
        rAtom.parseConstraint(maConstraints);
}

void ShapeLayoutingVisitor::visit(AlgAtom& rAtom)
{
    if (meLookFor == ALGORITHM)
    {
        for (const auto& pShape : rAtom.getLayoutNode().getNodeShapes())
            rAtom.layoutShape(pShape, maConstraints);
    }
}

void ShapeLayoutingVisitor::visit(ForEachAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(ChooseAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(LayoutNode& rAtom)
{
    if (meLookFor != LAYOUT_NODE)
        return;

    // process alg atoms first, nested layout nodes afterwards
    meLookFor = CONSTRAINT;
    defaultVisit(rAtom);
    meLookFor = ALGORITHM;
    defaultVisit(rAtom);
    maConstraints.clear();
    meLookFor = LAYOUT_NODE;
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::defaultVisit(LayoutAtom const & rAtom)
{
    // visit all children, at least one of them needs to have proper
    // name set
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShallowPresNameVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(AlgAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(ForEachAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShallowPresNameVisitor::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShallowPresNameVisitor::visit(ChooseAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShallowPresNameVisitor::visit(LayoutNode& rAtom)
{
    DiagramData::PointsNameMap::const_iterator aDataNode=
        mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if( aDataNode != mrDgm.getData()->getPointsPresNameMap().end() )
        mnCnt = std::max(mnCnt,
                         aDataNode->second.size());
}

void ShallowPresNameVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
