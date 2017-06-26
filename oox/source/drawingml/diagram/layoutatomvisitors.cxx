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
    const std::vector<LayoutAtomPtr>& rChildren=rAtom.getChildren();
    std::for_each( rChildren.begin(), rChildren.end(),
        [this] (LayoutAtomPtr const& pAtom) { pAtom->accept(*this); } );
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
        std::for_each( rChildren.begin(), rChildren.end(),
            [&] (LayoutAtomPtr const& pAtom) { pAtom->accept(aVisitor); } );
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
        std::for_each( rChildren.begin(), rChildren.end(),
            [this] (LayoutAtomPtr const& pAtom) { pAtom->accept(*this); } );
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
    ShapePtr pCurrShape(rAtom.getShape());
    if( pCurrShape )
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

    // layout shapes - now all child shapes are created
    ShapeLayoutingVisitor aLayoutingVisitor(pCurrParent,
                                            rAtom.getName());
    aLayoutingVisitor.defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::defaultVisit(LayoutAtom& rAtom)
{
    // visit all children, one of them needs to be the layout algorithm
    const std::vector<LayoutAtomPtr>& rChildren=rAtom.getChildren();
    std::for_each( rChildren.begin(), rChildren.end(),
        [this] (LayoutAtomPtr const& pAtom) { pAtom->accept(*this); } );
}

void ShapeLayoutingVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeLayoutingVisitor::visit(AlgAtom& rAtom)
{
    rAtom.layoutShape(mpParentShape, maName);
}

void ShapeLayoutingVisitor::visit(ForEachAtom& /*rAtom*/)
{
    // stop processing
}

void ShapeLayoutingVisitor::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(ChooseAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShapeLayoutingVisitor::visit(LayoutNode& /*rAtom*/)
{
    // stop processing - only traverse Condition/Choose atoms
}

void ShallowPresNameVisitor::defaultVisit(LayoutAtom& rAtom)
{
    // visit all children, at least one of them needs to have proper
    // name set
    const std::vector<LayoutAtomPtr>& rChildren=rAtom.getChildren();
    std::for_each( rChildren.begin(), rChildren.end(),
        [this] (LayoutAtomPtr const& pAtom) { pAtom->accept(*this); } );
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
