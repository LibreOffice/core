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

#include <o3tl/safeint.hxx>
#include "layoutatomvisitorbase.hxx"

#include <sal/log.hxx>

using namespace ::com::sun::star;

namespace oox::drawingml {

void LayoutAtomVisitorBase::defaultVisit(LayoutAtom const& rAtom)
{
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

std::vector<OUString> LayoutAtomVisitorBase::passNodesBelow(sal_Int32 nPointType) const
{
    std::vector<OUString> aResult;
    if (!mxCurrentNode.is())
        return aResult;

    // What the presentation below the current Point was built for, in the order the file lists
    // it. One pass of a loop belongs to each of those, of the type the loop runs over, so the
    // list says which Point each pass stands on.
    std::vector<std::pair<sal_Int32, OUString>> aBelow;
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             mrDgm.getData()->getConnections())
        if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_presParOf
            && rConnection->msSourceId == mxCurrentNode->msModelId)
            aBelow.emplace_back(rConnection->mnSourceOrder, rConnection->msDestId);

    std::sort(aBelow.begin(), aBelow.end());

    for (const auto& rOne : aBelow)
    {
        OUString aFor;
        for (const rtl::Reference<svx::diagram::Point>& rPoint : mrDgm.getData()->getPoints())
            if (rPoint.is() && rPoint->msModelId == rOne.second)
            {
                aFor = rPoint->getPresentation().msPresentationAssociationId;
                break;
            }

        if (aFor.isEmpty() || std::find(aResult.begin(), aResult.end(), aFor) != aResult.end())
            continue;

        if (nPointType != XML_all && nPointType != XML_none)
        {
            bool bRightKind = false;
            for (const rtl::Reference<svx::diagram::Point>& rPoint : mrDgm.getData()->getPoints())
                if (rPoint.is() && rPoint->msModelId == aFor)
                {
                    bRightKind = static_cast<sal_Int32>(rPoint->mnXMLType) == nPointType;
                    break;
                }

            if (!bRightKind)
                continue;
        }

        aResult.push_back(aFor);
    }

    return aResult;
}

rtl::Reference<svx::diagram::Point>
LayoutAtomVisitorBase::presentationForPass(const svx::diagram::Points& rNamed) const
{
    // Only what hangs under the presentation Point reached so far belongs to this layout node.
    svx::diagram::Points aUnder;
    for (const rtl::Reference<svx::diagram::Point>& rPoint : rNamed)
    {
        if (!rPoint.is() || !mxCurrentNode.is())
            continue;

        for (const rtl::Reference<svx::diagram::Connection>& rConnection :
                 mrDgm.getData()->getConnections())
            if (rConnection->msSourceId == mxCurrentNode->msModelId
                && rConnection->msDestId == rPoint->msModelId)
            {
                aUnder.push_back(rPoint);
                break;
            }
    }

    // The pass of the loop names a data Point, and what this pass draws is the presentation that
    // was built for that Point.
    if (!msPassNodeId.isEmpty())
        for (const rtl::Reference<svx::diagram::Point>& rPoint : aUnder)
            if (rPoint->getPresentation().msPresentationAssociationId == msPassNodeId)
                return rPoint;

    // A loop over the following siblings of a node stands on the next node, and what it draws is
    // the transition on the way there, the sibTrans of the node before it: so the presentation
    // built for that transition is the one, where none was built for the node itself.
    if (!msPassNodeId.isEmpty())
    {
        OUString aParent;
        sal_Int32 nOrder(-1);
        for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             mrDgm.getData()->getConnections())
            if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                && rConnection->msDestId == msPassNodeId)
            {
                aParent = rConnection->msSourceId;
                nOrder = rConnection->mnSourceOrder;
                break;
            }
        for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             mrDgm.getData()->getConnections())
            if (nOrder > 0 && rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                && rConnection->msSourceId == aParent
                && rConnection->mnSourceOrder == nOrder - 1
                && !rConnection->msSibTransId.isEmpty())
                for (const rtl::Reference<svx::diagram::Point>& rPoint : aUnder)
                    if (rPoint->getPresentation().msPresentationAssociationId
                        == rConnection->msSibTransId)
                        return rPoint;
    }

    // Failing that the place reached so far picks among them.
    if (mnCurrIdx >= 0 && o3tl::make_unsigned(mnCurrIdx) < aUnder.size())
        return aUnder[mnCurrIdx];

    // Outside a loop there is one presentation of a name under a Point, and that is this one.
    if (msPassNodeId.isEmpty() && aUnder.size() == 1)
        return aUnder.front();

    return rtl::Reference<svx::diagram::Point>();
}

void LayoutAtomVisitorBase::visit(ChooseAtom& rAtom)
{
    for (const auto& pChild : rAtom.getChildren())
    {
        const ConditionAtomPtr pCond = std::dynamic_pointer_cast<ConditionAtom>(pChild);
        if (pCond && pCond->getDecision(mrDgm, mxCurrentNode, msPassNodeId))
        {
            SAL_INFO("oox.drawingml", "Entering if node: " << pCond->getName());
            pCond->accept(*this);
            break;
        }
    }
}

void LayoutAtomVisitorBase::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void LayoutAtomVisitorBase::visit(ForEachAtom& rAtom)
{
    if (!rAtom.getRef().isEmpty())
    {
        if (LayoutAtomPtr pRefAtom = rAtom.getRefAtom(mrDgm))
            pRefAtom->accept(*this);
        return;
    }

    const IteratorAttr& rIterator(rAtom.iterator());

    if (rIterator.mbHideLastTrans && !rIterator.maAxis.empty()
        && rIterator.maAxis[0] == XML_followSib)
    {
        // hideLastTrans hides the transition after the last node, so it is about loops over
        // transitions only, and the last node is the one with no node following it among its
        // siblings. Alternating_Flow walks its nodes two at a time and reaches the second of a
        // pair by followSib ptType node; counting the passes of the loops made the third node's
        // pass the last one and dropped the fourth node and every transition after a second.
        // Where there is no Point to ask, the passes of the loop around say it as before.
        const bool bOverTransitions(rIterator.mnPtType == XML_sibTrans
                                    || rIterator.mnPtType == XML_parTrans
                                    || rIterator.mnPtType == XML_all);
        const OUString aFrom(
            !msPassNodeId.isEmpty()
                ? msPassNodeId
                : (mxCurrentNode.is() ? mxCurrentNode->getPresentation().msPresentationAssociationId
                                      : OUString()));
        if (bOverTransitions && !aFrom.isEmpty())
        {
            IteratorAttr aNextNode;
            aNextNode.maAxis = { XML_followSib };
            aNextNode.maPtType = { XML_node };
            aNextNode.maStart = { 0 };
            aNextNode.maCount = { 0 };
            aNextNode.mnPtType = XML_node;
            if (pointsAlongAxis(mrDgm, aNextNode, aFrom, /*bLastStepWhole*/ true).empty())
                return;
        }
        else if (bOverTransitions && mnCurrIdx + mnCurrStep >= mnCurrCnt)
            return;
    }

    // cool#15967. Connectors are not independent but siblings of current node.
    // Only along the sibling axes: a loop over the transitions of the node's children, "axis ch
    // ptType sibTrans", walks the data like any other axis, and its passes are those children's
    // transitions. Pie_Process's negSibTrans stands on the first child's transition, so a node
    // with children has one and the last node's is not hidden.
    const bool bAlongSiblings(rIterator.maAxis.empty() || rIterator.maAxis[0] == XML_followSib
                              || rIterator.maAxis[0] == XML_precedSib
                              || rIterator.maAxis[0] == XML_self);
    if (rIterator.mnPtType == XML_sibTrans && bAlongSiblings)
    {
        // What the body draws belongs to the connector and not to the node an enclosing loop
        // stands on. Where that loop stands on a node, the connector is the sibTrans of that
        // node, and the pass stands on it: Alternating_Flow walks its nodes two at a time, and
        // the connector after the third node is the second presentation of its name, which the
        // place of the pass, 2, did not find. Without such a node the pass says nothing here.
        const OUString aHeldPassNodeId(msPassNodeId);
        OUString aTransition;
        if (!aHeldPassNodeId.isEmpty())
            for (const rtl::Reference<svx::diagram::Connection>& rConnection :
                 mrDgm.getData()->getConnections())
                if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                    && rConnection->msDestId == aHeldPassNodeId
                    && !rConnection->msSibTransId.isEmpty())
                {
                    aTransition = rConnection->msSibTransId;
                    break;
                }
        msPassNodeId = aTransition;
        defaultVisit(rAtom);
        msPassNodeId = aHeldPassNodeId;
        return;
    }

    // The self axis names the current data node, so the body applies once and keeps
    // the position that an enclosing loop has reached.
    if (rIterator.maAxis.size() == 1 && rIterator.maAxis[0] == XML_self)
    {
        defaultVisit(rAtom);
        return;
    }

    // Which Point each pass stands on. Only the conditions are told, the making of shapes is
    // left exactly as it was. The axis of the loop names those Points, and it is walked from the
    // Point the pass of an enclosing loop stands on, or from the data Point of the current
    // presentation where there is no such loop. Where the axis names none, the presentation below
    // the current Point says what it was built for and that stands in.
    const OUString aWalkFrom(
        !msPassNodeId.isEmpty()
            ? msPassNodeId
            : (mxCurrentNode.is() ? mxCurrentNode->getPresentation().msPresentationAssociationId
                                  : OUString()));
    std::vector<OUString> aPassNodes(
        pointsAlongAxis(mrDgm, rIterator, aWalkFrom, /*bLastStepWhole*/ true));
    // Walked from a Point and reaching none, the axis has said its piece: a node without
    // children makes no pass of a loop over its children, Circle_Accent_Timeline's third event
    // has no descriptions. Only where there was no Point to walk from does the presentation
    // below stand in.
    const bool bAxisSaid(!aWalkFrom.isEmpty());
    if (aPassNodes.empty() && !bAxisSaid)
        aPassNodes = passNodesBelow(rIterator.mnPtType);

    // Every other axis leads from the current data node to a set of data nodes, and the body
    // applies once per node of that set. The presentation Points that carry the layout node names
    // of the body reference for these nodes, so their number is the number of passes. The body is
    // read standing on the Point of the first pass, so a choose inside it takes the branch that
    // pass takes and what gets counted are the names that branch really carries.
    ShallowPresNameVisitor aVisitor(mrDgm, mxCurrentNode,
                                    aPassNodes.empty() ? msPassNodeId : aPassNodes[0]);
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(aVisitor);

    // The loop applies once per Point its axis picks out. Where the axis picks out none, the
    // layout node names of the body say how many presentations were built and that many passes
    // are made. A body that carries no layout node of its own applies once.
    const sal_Int32 nFound(static_cast<sal_Int32>(aVisitor.getCount()));
    const sal_Int32 nAlong(static_cast<sal_Int32>(aPassNodes.size()));
    const sal_Int32 nStep = rIterator.mnStep;

    // Where among what the axis reached the first pass stands. A loop can say it: a place
    // counted from one, or counted back from the end where it is written negative, -1 being
    // the last of them. Saying nothing starts at the near end, which for a loop that runs
    // backwards is the far one.
    // A loop over several steps, "axis ch ch st 1 1 cnt 1 0", states a start and a count for
    // each step; the walk applied those of the earlier steps, and the passes take the last
    // step's, a 0 meaning all. Nested_Target's oChild stands on the children of the first
    // child, all three of them, and the first step's count of 1 gave it one.
    sal_Int32 nStartWanted(rIterator.mnSt);
    sal_Int32 nCountWanted(rIterator.mnCnt);
    if (rIterator.maAxis.size() > 1)
    {
        if (rIterator.maStart.size() >= rIterator.maAxis.size())
            nStartWanted = rIterator.maStart.back();
        if (rIterator.maCount.size() >= rIterator.maAxis.size())
            nCountWanted = rIterator.maCount.back() == 0 ? -1 : rIterator.maCount.back();
    }
    sal_Int32 nFirstAlong(nStep < 0 ? nAlong - 1 : 0);
    if (nStartWanted > 0)
        nFirstAlong = nStartWanted - 1;
    else if (nStartWanted < 0)
        nFirstAlong = nAlong + nStartWanted;

    // How many passes there are room for from there, counting the way the loop runs.
    const sal_Int32 nRoom(nStep < 0 ? nFirstAlong + 1 : nAlong - nFirstAlong);
    const sal_Int32 nChildren(nAlong > 0     ? std::max<sal_Int32>(nRoom, 0)
                              : bAxisSaid    ? 0
                              : (nFound > 0) ? nFound
                                             : 1);

    const sal_Int32 nCnt = std::min(
        nChildren,
        nCountWanted==-1 ? nChildren : nCountWanted);

    const OUString aOldPassNodeId(msPassNodeId);

    const sal_Int32 nOldIdx = mnCurrIdx;
    const sal_Int32 nOldStep = mnCurrStep;
    const sal_Int32 nOldCnt = mnCurrCnt;
    mnCurrStep = nStep;
    mnCurrCnt = nCnt;
    // A loop may be told to run backwards, from the last of what it runs over to the first.
    // Starting at zero and adding a negative step never enters the body at all, so such a loop
    // drew nothing whatever.
    const sal_Int32 nFirstPass(nStep < 0 ? nCnt - 1 : 0);
    sal_Int32 nAlongIdx(nFirstAlong);
    for (mnCurrIdx = nFirstPass; nStep != 0 && mnCurrIdx >= 0 && mnCurrIdx < nCnt;
         mnCurrIdx += nStep, nAlongIdx += nStep)
    {
        msPassNodeId = nAlongIdx >= 0 && nAlongIdx < nAlong ? aPassNodes[nAlongIdx] : OUString();

        // TODO there is likely some conditions
        for (const auto& pAtom : rAtom.getChildren())
            pAtom->accept(*this);
    }

    msPassNodeId = aOldPassNodeId;

    // and restore idx
    mnCurrIdx = nOldIdx;
    mnCurrStep = nOldStep;
    mnCurrCnt = nOldCnt;
}

void LayoutAtomVisitorBase::visit(LayoutNode& rAtom)
{
    // TODO: deduplicate code in descendants

    // stop processing if it's not a child of previous LayoutNode

    const DiagramData_oox::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end()
        || mnCurrIdx >= static_cast<sal_Int32>(aDataNode->second.size()))
        return;

    const rtl::Reference<svx::diagram::Point>& rNewNode(aDataNode->second.at(mnCurrIdx));
    if (!mxCurrentNode.is() || !rNewNode.is())
        return;

    bool bIsChild = false;
    for (const rtl::Reference<svx::diagram::Connection>& aConnection :
             mrDgm.getData()->getConnections())
        if (aConnection->msSourceId == mxCurrentNode->msModelId
            && aConnection->msDestId == rNewNode->msModelId)
            bIsChild = true;

    if (!bIsChild)
        return;

    const rtl::Reference<svx::diagram::Point> xPreviousNode(mxCurrentNode);
    mxCurrentNode = rNewNode;

    // From here the current Point is the one of this layout node, so a condition below asks
    // about that and wants no stand-in from the loop above.
    const OUString aHeldPassNodeId(msPassNodeId);
    msPassNodeId.clear();

    defaultVisit(rAtom);

    msPassNodeId = aHeldPassNodeId;
    mxCurrentNode = xPreviousNode;
}

void ShallowPresNameVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(RuleAtom& /*rAtom*/)
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

void ShallowPresNameVisitor::visit(LayoutNode& rAtom)
{
    DiagramData_oox::PointsNameMap::const_iterator aDataNode =
        mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if( aDataNode != mrDgm.getData()->getPointsPresNameMap().end() )
        mnCnt = std::max(mnCnt,
                         aDataNode->second.size());
}

void ShallowPresNameVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
