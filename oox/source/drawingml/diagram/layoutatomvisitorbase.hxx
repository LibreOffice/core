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

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_LAYOUTATOMVISITORBASE_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_LAYOUTATOMVISITORBASE_HXX

#include <vector>
#include <algorithm>
#include "diagram.hxx"
#include "diagramlayoutatoms.hxx"

namespace oox::drawingml {

struct LayoutAtomVisitor
{
    virtual ~LayoutAtomVisitor() {}
    virtual void visit(ConstraintAtom& rAtom) = 0;
    virtual void visit(RuleAtom& rAtom) = 0;
    virtual void visit(AlgAtom& rAtom) = 0;
    virtual void visit(ForEachAtom& rAtom) = 0;
    virtual void visit(ConditionAtom& rAtom) = 0;
    virtual void visit(ChooseAtom& rAtom) = 0;
    virtual void visit(LayoutNode& rAtom) = 0;
    virtual void visit(ShapeAtom& rAtom) = 0;
};

// basic visitor implementation that follows if/else and for-each nodes
// and keeps track of current position in data tree
class LayoutAtomVisitorBase : public LayoutAtomVisitor
{
public:
    LayoutAtomVisitorBase(const SmartArtDiagram& rDgm,
                          const rtl::Reference<svx::diagram::Point>& rRootPoint) :
        mrDgm(rDgm),
        mxCurrentNode(rRootPoint),
        mnCurrIdx(0),
        mnCurrStep(0),
        mnCurrCnt(0),
        meLookFor(LAYOUT_NODE)
    {}

    void defaultVisit(LayoutAtom const& rAtom);

    /// The data Points the presentation below the current one was built for, of nPointType, in
    /// the order the file lists them. One pass of a loop belongs to each.
    std::vector<OUString> passNodesBelow(sal_Int32 nPointType) const;

    /// The presentation Point of rNamed that was built for the Point the pass of the loop stands
    /// on. Where the pass stands on nothing, or nothing was built for it, the place of the pass
    /// among rNamed decides, and an empty reference comes back where there is no such place.
    rtl::Reference<svx::diagram::Point>
    presentationForPass(const svx::diagram::Points& rNamed) const;

    using LayoutAtomVisitor::visit;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(ConditionAtom& rAtom) override;
    virtual void visit(ChooseAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;

protected:
    const SmartArtDiagram& mrDgm;
    rtl::Reference<svx::diagram::Point> mxCurrentNode;
    sal_Int32 mnCurrIdx;
    sal_Int32 mnCurrStep;
    sal_Int32 mnCurrCnt;
    enum {LAYOUT_NODE, CONSTRAINT, ALGORITHM, RULE} meLookFor;

    // The modelId of the Point the pass of the loop stands on, for the conditions to ask about.
    // Empty outside a loop.
    OUString msPassNodeId;
};

class ShallowPresNameVisitor : public LayoutAtomVisitorBase
{
public:
    explicit ShallowPresNameVisitor(const SmartArtDiagram& rDgm,
                                    const rtl::Reference<svx::diagram::Point>& rRootPoint,
                                    const OUString& rPassNodeId) :
        LayoutAtomVisitorBase(rDgm, rRootPoint),
        mnCnt(0)
    {
        msPassNodeId = rPassNodeId;
    }

    using LayoutAtomVisitorBase::visit;
    virtual void visit(ConstraintAtom& rAtom) override;
    virtual void visit(RuleAtom& rAtom) override;
    virtual void visit(AlgAtom& rAtom) override;
    virtual void visit(ForEachAtom& rAtom) override;
    virtual void visit(LayoutNode& rAtom) override;
    virtual void visit(ShapeAtom& rAtom) override;

    size_t getCount() const { return mnCnt; }

private:
    size_t mnCnt;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
