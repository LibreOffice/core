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

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_LAYOUTATOMVISITORS_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_LAYOUTATOMVISITORS_HXX

#include <memory>

#include <oox/drawingml/shape.hxx>
#include "diagram.hxx"
#include "diagramlayoutatoms.hxx"
#include "layoutatomvisitorbase.hxx"

namespace oox { namespace drawingml {

class ShapeCreationVisitor : public LayoutAtomVisitorBase
{
public:
    ShapeCreationVisitor(const Diagram& rDgm,
                         const dgm::Point* pRootPoint,
                         const ShapePtr& rParentShape) :
        LayoutAtomVisitorBase(rDgm, pRootPoint),
        mpParentShape(rParentShape)
    {}

    using LayoutAtomVisitorBase::visit;
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

private:
    ShapePtr mpParentShape;
};

class ShapeTemplateVisitor : public LayoutAtomVisitorBase
{
public:
    ShapeTemplateVisitor(const Diagram& rDgm, const dgm::Point* pRootPoint)
        : LayoutAtomVisitorBase(rDgm, pRootPoint)
    {}

    using LayoutAtomVisitorBase::visit;
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

    ShapePtr const & getShapeCopy() const
        { return mpShape; }

private:
    ShapePtr mpShape;
};

class ShapeLayoutingVisitor : public LayoutAtomVisitorBase
{
public:
    ShapeLayoutingVisitor(const Diagram& rDgm, const dgm::Point* pRootPoint) :
        LayoutAtomVisitorBase(rDgm, pRootPoint),
        meLookFor(LAYOUT_NODE)
    {}

    using LayoutAtomVisitorBase::visit;
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

private:
    std::vector<Constraint> maConstraints;
    enum {LAYOUT_NODE, CONSTRAINT, ALGORITHM} meLookFor;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
