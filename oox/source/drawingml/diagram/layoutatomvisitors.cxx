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

#include <functional>

#include <basegfx/numeric/ftools.hxx>

#include "drawingml/customshapeproperties.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace oox { namespace drawingml {

void ShapeCreationVisitor::defaultVisit(LayoutAtom& rAtom)
{
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShapeCreationVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // TODO: eval the constraints
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
    ShapePtr pCurrParent(mpParentShape);
    ShapePtr pCurrShape(rAtom.getShapeTemplate());

    if (rAtom.getExistingShape())
    {
        // reuse existing shape
        if (rAtom.setupShape(rAtom.getExistingShape(), mrDgm, mnCurrIdx))
            rAtom.getNodeShapes().push_back(rAtom.getExistingShape());
    }
    else if( pCurrShape )
    {
        SAL_INFO(
            "oox.drawingml",
            "processing shape type "
                << (pCurrShape->getCustomShapeProperties()
                    ->getShapePresetType()));

        // TODO(F3): cloned shape shares all properties by reference,
        // don't change them!
        ShapePtr pClonedShape(
            new Shape( pCurrShape ));

        if( rAtom.setupShape(pClonedShape, mrDgm, mnCurrIdx) )
        {
            pCurrParent->addChild(pClonedShape);
            pCurrParent = pClonedShape;
            rAtom.getNodeShapes().push_back(pClonedShape);
        }
    }
    else
    {
        SAL_WARN("oox.drawingml", "ShapeCreationVisitor::visit: no shape set while processing layoutnode named " << rAtom.getName() );
    }

    // set new parent for children
    ShapePtr pPreviousParent(mpParentShape);
    mpParentShape=pCurrParent;

    // process children
    defaultVisit(rAtom);

    // restore parent
    mpParentShape=pPreviousParent;

    // remove unneeded empty group shapes
    pCurrParent->getChildren().erase(
        std::remove_if(pCurrParent->getChildren().begin(), pCurrParent->getChildren().end(),
            [] (const ShapePtr & aChild) { return aChild->getServiceName() == "com.sun.star.drawing.GroupShape" && aChild->getChildren().empty(); }),
        pCurrParent->getChildren().end());
}

void ShapeLayoutingVisitor::defaultVisit(LayoutAtom& rAtom)
{
    // visit all children, one of them needs to be the layout algorithm
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void ShapeLayoutingVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeLayoutingVisitor::visit(AlgAtom& rAtom)
{
    if (mbLookForAlg && mpCurrentLayoutNode)
    {
        for (const auto& pShape : mpCurrentLayoutNode->getNodeShapes())
            rAtom.layoutShape(pShape, mpCurrentLayoutNode->getName());
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
    if (mbLookForAlg)
        return;

    LayoutNode* pPreviousLayoutNode = mpCurrentLayoutNode;
    mpCurrentLayoutNode = &rAtom;

    // process alg atoms first, nested layout nodes afterwards
    mbLookForAlg = true;
    defaultVisit(rAtom);
    mbLookForAlg = false;
    defaultVisit(rAtom);

    mpCurrentLayoutNode = pPreviousLayoutNode;
}

void ShallowPresNameVisitor::defaultVisit(LayoutAtom& rAtom)
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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
