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

#include "oox/drawingml/shape.hxx"
#include "diagram.hxx"
#include "diagramlayoutatoms.hxx"

namespace oox { namespace drawingml {

class ShapeCreationVisitor : public LayoutAtomVisitor
{
    ShapePtr mpParentShape;
    const Diagram& mrDgm;
    sal_Int32 mnCurrIdx;

    void defaultVisit(LayoutAtom& rAtom);
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(ConditionAtom& rAtom) override;
    virtual void visit(ChooseAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

public:
    ShapeCreationVisitor(const ShapePtr& rParentShape,
                         const Diagram& rDgm) :
        mpParentShape(rParentShape),
        mrDgm(rDgm),
        mnCurrIdx(0)
    {}
};

class ShapeTemplateVisitor : public LayoutAtomVisitor
{
    ShapePtr mpShape;

    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(ConditionAtom& rAtom) override;
    virtual void visit(ChooseAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

public:
    void defaultVisit(LayoutAtom& rAtom);
    ShapePtr getShapeCopy() const
        { return mpShape; }
};

class ShapeLayoutingVisitor : public LayoutAtomVisitor
{
    ShapePtr mpParentShape;
    OUString maName;

    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(ConditionAtom& rAtom) override;
    virtual void visit(ChooseAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

public:
    ShapeLayoutingVisitor(const ShapePtr& rParentShape,
                          const OUString& rName) :
        mpParentShape(rParentShape),
        maName(rName)
    {}

    void defaultVisit(LayoutAtom& rAtom);
};

class ShallowPresNameVisitor : public LayoutAtomVisitor
{
    const Diagram& mrDgm;
    size_t mnCnt;

    void defaultVisit(LayoutAtom& rAtom);
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(ConditionAtom& rAtom) override;
    virtual void visit(ChooseAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

public:
    explicit ShallowPresNameVisitor(const Diagram& rDgm) :
        mrDgm(rDgm),
        mnCnt(0)
    {}

    size_t getCount() const
        { return mnCnt; }
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
